/* Physical input and Linux disk regression. All sessions use a caller-provided
 * temporary XDG_DATA_HOME and never touch actual player profiles. */
#include <stdio.h>
#include "game.c"
#include "linux_save.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
static void hj_key(U k){tick(0);tick(k);tick(0);}
static void hj_hold(U k,int count){for(int i=0;i<count;i++)tick(k);tick(0);}
static void hj_stand(NavPoint q){assert(!solid(q.x,q.y,8));scene=1;p.x=q.x;p.y=q.y;p.car=-1;menu=-1;mapOn=0;p.dead=0;p.heat=0;}
static void hj_capture(const char*path){
 if(!path)return;FILE*f=fopen(path,"wb");assert(f);
 paint();fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U z0=pixels[i];fputc(z0>>16,f);fputc(z0>>8,f);fputc(z0,f);}fclose(f);
}
static void hj_restart(void){
 request_save();assert(saveFlash>0);
 linux_disk_shutdown();assert(linux_disk_init()&&linux_disk_lock());disk_slot(0);
 init_city();assert(wason);scene=1;assert(!harbor_boat_active());
}
static void hj_accept_phone(int row){
 hj_key(PHONE);assert(scene==NIGHT_PHONE_SCENE);
 hj_key(RIGHT);hj_key(RIGHT);assert(np_app==NP_WIRE);
 while(np_row!=14+row)hj_key(DOWN);
 int before=hj_phase();hj_key(ENTER);assert(hj_phase()==before);hj_key(ENTER);hj_key(ESC);
}
static void hj_auto_walk(NavPoint q,int near){
 for(int t=0;t<480&&!hj_near(q,near);t++){
  unsigned k=0;
  if(absf(p.x-q.x)>2.f)k|=p.x<q.x?RIGHT:LEFT;
  if(absf(p.y-q.y)>2.f)k|=p.y<q.y?DOWN:UP;
  tick(k);
 }
 if(!hj_near(q,near))fprintf(stderr,"Walk stuck (%.1f,%.1f) -> (%d,%d) at phase %d\n",p.x,p.y,q.x,q.y,hj_phase());
 assert(hj_near(q,near));tick(0);
}
static void hj_harbor_walk(NavPoint goal,int radius){
 enum{NX=64,NY=67,X0=2758,Y0=2002,STEP=4,GRID=NX*NY};
 int start=cl(((int)p.x-X0+2)/STEP,0,NX-1)+cl(((int)p.y-Y0+2)/STEP,0,NY-1)*NX;
 int parent[GRID],queue[GRID],from=0,to=1,found=-1;
 for(int i=0;i<GRID;i++)parent[i]=-1;
 parent[start]=start;queue[0]=start;
 while(from<to){
  int i=queue[from++],ix=i%NX,iy=i/NX,x=X0+STEP*ix,y=Y0+STEP*iy;
  if(d2(x-goal.x,y-goal.y)<(radius-4)*(radius-4)){found=i;break;}
  static const int off[]={-1,1,-NX,NX};
  for(int k=0;k<4;k++){
   if((k==0&&!ix)||(k==1&&ix==NX-1)||(k==2&&!iy)||(k==3&&iy==NY-1))continue;
   int j=i+off[k],qx=X0+STEP*(j%NX),qy=Y0+STEP*(j/NX);
   if(parent[j]!=-1||solid(qx,qy,10)||!street_foot_car_clear(qx,qy,-1,7))continue;
   parent[j]=i;queue[to++]=j;
  }
 }
 if(found<0)fprintf(stderr,"No ordinary walk from %.1f,%.1f to %d,%d (car parked %.1f,%.1f)\n",p.x,p.y,goal.x,goal.y,cars[0].x,cars[0].y);
 assert(found>=0);
 int steps=0;for(int i=found;i!=start;i=parent[i])queue[steps++]=i;
 for(int i=steps-1;i>=0;i--){int a=queue[i];hj_auto_walk((NavPoint){X0+STEP*(a%NX),Y0+STEP*(a/NX)},3);}
 assert(hj_near(goal,radius));
}
static void hj_to_dock(int dock){NavPoint d=hj_dock_walk(dock*2);hj_stand(d);hj_key(USE);assert(harbor_boat_active());if(getenv("DC_HARBOR_3D")){paint();assert(chProjected);}}
static void hj_helm(NavPoint q,int still,int radius){
 assert(harbor_boat_active());
 for(int i=0;i<1850;i++){
  float vx=q.x-p.x,vy=q.y-p.y,d=sq(d2(vx,vy)),spd=harbor_boat_speed();
  if(d<radius&&(!still||(absf(spd)<9.f&&d2(hb.hull.vx,hb.hull.vy)<100.f)))return;
  float turn=wrap(look(vx,vy)-hb.hull.a);U k=0;
  if(absf(turn)>.13f)k|=turn<0?LEFT:RIGHT;
  if(still&&d<radius+17&&(absf(spd)>9.f||d2(hb.hull.vx,hb.hull.vy)>100.f))k|=BRAKE;
  else if(d>=radius&&(absf(turn)<.46f||absf(spd)<10.f))k|=UP;
  else if(absf(turn)>.7f&&spd>48.f)k|=BRAKE;
  tick(k);
  if(!harbor_boat_active())break;
 }
 fprintf(stderr,"Skiff stuck (%.1f,%.1f) -> (%d,%d), spd %.1f, stage %d\n",p.x,p.y,q.x,q.y,harbor_boat_speed(),hj_phase());
 assert(0);
}
static void hj_map_points(void){
 assert(!solid(hj_office.x,hj_office.y,8)&&!solid(hj_alley.x,hj_alley.y,8));
 assert(!solid(hj_cue.x,hj_cue.y,8)&&!solid(hj_quay.x,hj_quay.y,8));
 for(int j=0;j<3;j++){NavPoint b=hj_water(j);assert(harbor_nav_water(b.x,b.y,17));}
 NavPoint spot=hj_dock_walk(2);assert(!solid(spot.x,spot.y,8));assert(nav_traversable(hj_quay,spot,8));
}
static void hj_try_cancel(void){
 hj_stand(hj_office);hj_key(USE);assert(harbor_jobs_panel_active());
 hj_key(DOWN);hj_key(ENTER);hj_key(ENTER);assert(hj_phase()==HJ_BLUE_CLUE);hj_key(ESC);assert(!harbor_jobs_panel_active());
 hj_accept_phone(HJ_CANCEL);assert(hj_phase()==HJ_IDLE&&nv.phone[HJ_FLOAT]==0&&nv.phone[HJ_DONE]==0);
}
static void hj_redo_after_car_change(void){
 assert(hj_phase()==HJ_RELAY_QUAY);int backup=cars[0].type;
 cars[0].type=(backup+1)%LEN(specs);hj_stand(hj_quay);hj_key(USE);assert(hj_phase()==HJ_RELAY_QUAY);
 hj_stand(hj_alley);hj_key(USE);assert(hj_phase()==HJ_RELAY_LIFT&&nv.phone[HJ_FLOAT]==0);cars[0].type=backup;
 hj_key(USE);assert(hj_phase()==HJ_RELAY_TRUNK);
}
static void hj_play_relay(void){
 int initial=sv.cash;
 hj_stand(hj_office);hj_accept_phone(HJ_RELAY);assert(hj_phase()==HJ_IDLE);
 sv.owned=1;sv.selection=0;sv.carCond[0]=sv.vehicleCond=100;cars[0]=(Car){2824,2164,1.5707963f,0,0,1,100,0};
 hj_accept_phone(HJ_RELAY);assert(hj_phase()==HJ_RELAY_TAG);
 hj_stand(hj_alley);
 int hp=p.hp;hj_hold(0,145);assert(hj_interference>0&&hj_phase()==HJ_RELAY_TAG);
 hj_key(USE);assert(hj_phase()==HJ_RELAY_TAG&&p.hp==hp);
 hj_hold(UP,55);assert(hj_interference==0&&hj_attention==0&&p.hp==hp&&p.heat==0.f);
 hj_auto_walk(hj_alley,12);hj_key(USE);assert(hj_phase()==HJ_RELAY_LIFT);hj_key(USE);assert(hj_phase()==HJ_RELAY_TRUNK);
 NavPoint r=hj_rear();hj_auto_walk(r,12);hj_key(USE);assert(hj_phase()==HJ_RELAY_QUAY&&nv.phone[HJ_FLOAT]==1);
 hj_redo_after_car_change();hj_auto_walk(r,12);hj_key(USE);assert(hj_phase()==HJ_RELAY_QUAY);
 hj_restart();assert(hj_phase()==HJ_RELAY_QUAY&&nv.phone[HJ_FLOAT]==1&&sv.cash==initial);
 /* The real loader stages the registered ride at a safe curb near where
  * the fixture last stood. For a deterministic route, stage it in the
  * neighboring marked cargo lane, then board and drive via normal input. */
 cars[0]=(Car){2845,2078,1.5707963f,0,0,1,100,0};
 hj_stand(hj_rear());hj_key(USE);assert(p.car==0);
 hj_hold(UP,43);hj_hold(BRAKE,40);assert(cars[0].y>2107&&cars[0].y<2148);
 hj_key(USE);assert(p.car<0);hj_harbor_walk(hj_quay,24);hj_key(USE);assert(hj_phase()==HJ_RELAY_BUOY);
 hj_harbor_walk(hj_dock_walk(2),12);hj_key(USE);assert(harbor_boat_active());
 int x,y;assert(harbor_jobs_route(&x,&y)&&harbor_nav_water(x,y,14));
 hj_helm(hj_water(HJ_RELAY),0,29);hj_key(USE);assert(hj_complete(HJ_RELAY)&&hj_phase()==HJ_IDLE&&sv.cash==initial+185);
 hj_restart();assert(hj_complete(HJ_RELAY)&&sv.cash==initial+185);
 assert(!harbor_boat_active()&&hj_near(hj_dock_walk(2),8));
}
static void hj_play_blue(void){
 int initial=sv.cash;hj_stand(hj_office);hj_key(USE);assert(harbor_jobs_panel_active());
 hj_key(DOWN);hj_key(ENTER);hj_key(ENTER);assert(hj_phase()==HJ_BLUE_CLUE);hj_key(ESC);
 hj_stand(hj_cue);hj_key(USE);assert(hj_phase()==HJ_BLUE_FIND);
 hj_to_dock(1);
 hj_helm(hj_water(HJ_BLUE),0,29);hj_key(USE);assert(hj_phase()==HJ_BLUE_RETURN);
 hj_restart();assert(hj_phase()==HJ_BLUE_RETURN&&hj_near(hj_dock_walk(2),8));
 hj_to_dock(1);hj_helm(hj_dock_walk(1),1,24);hj_key(USE);assert(!harbor_boat_active());
 hj_stand(hj_office);hj_key(USE);assert(hj_complete(HJ_BLUE)&&sv.cash==initial+150);
}
static void hj_play_bump(int collect_chip){
 int initial=sv.cash;
 hj_stand(hj_office);hj_accept_phone(HJ_BUMP);assert(hj_phase()==HJ_BUMP_TAG);
 hj_to_dock(1);
 for(int pass=0;pass<4&&hj_phase()==HJ_BUMP_TAG;pass++){
  NavPoint q=hj_trainer();hj_helm(q,0,6);hj_hold(UP,1);
 }
 assert(hj_phase()==HJ_BUMP_LOOT&&nv.phone[HJ_FLOAT]!=0);
 U drift=nv.phone[HJ_FLOAT];hj_restart();assert(hj_phase()==HJ_BUMP_LOOT&&nv.phone[HJ_FLOAT]==drift);
 if(collect_chip){
  hj_to_dock(1);hj_helm(hj_drift(),0,28);hj_key(USE);assert(hj_phase()==HJ_BUMP_RETURN);
  hj_restart();assert(hj_phase()==HJ_BUMP_RETURN);
 }
 int expected=initial+(collect_chip?150:105);
 hj_stand(hj_office);hj_key(USE);assert(hj_complete(HJ_BUMP)&&sv.cash==expected);
 hj_key(USE);assert(harbor_jobs_panel_active());hj_key(ESC);assert(sv.cash==expected);
}
int main(int argc,char**argv){
 assert(getenv("XDG_DATA_HOME"));assert(linux_disk_init()&&linux_disk_lock());disk_slot(0);init_city();scene=1;sv.intro=4;sv.cash=500;p.hp=100;
 if(argc>1&&!strcmp(argv[1],"--seed")){
  if(argc<3)return 1;
  hj_stand(hj_office);
  if(!strcmp(argv[2],"stages")){nv.phone[HJ_PHASE]=HJ_RELAY_TAG;hj_stand(hj_alley);}
  if(!strcmp(argv[2],"afloat")){nv.phone[HJ_PHASE]=HJ_RELAY_BUOY;hj_stand(hj_dock_walk(2));}
  request_save();assert(saveFlash>0);linux_disk_shutdown();return 0;
 }
 if(argc==2&&!strcmp(argv[1],"--bump-base")){
  hj_play_bump(0);linux_disk_shutdown();puts("PASS optional harbor chip can be skipped, only base payout granted.");return 0;
 }
 if(getenv("DC_HARBOR_3D")){hj_key(CAMERA);paint();assert(ch_pref()&&chProjected);}
 hj_map_points();
 hj_stand((NavPoint){loc[12].x,loc[12].y});hj_hold(0,20);
 assert(nv.phone[HJ_PHASE]==0&&nv.phone[HJ_DONE]==0&&nv.phone[HJ_FLOAT]==0);
 request_save();
 if(getenv("DC_HARBOR_3D")){Save11 full;assert(disk_read(&full,sizeof full)&&full.night.garage[60]==1);}
 else assert(disk_read(&sv,sizeof sv));
 hj_stand(hj_office);hj_key(USE);assert(harbor_jobs_panel_active());
 hj_capture(argc>1?argv[1]:NULL);hj_key(ESC);
 nv.phone[NP_BAG+2]=7;
 hj_try_cancel();hj_play_relay();hj_play_blue();hj_play_bump(1);
 hj_restart();assert((nv.phone[HJ_DONE]&7u)==7u&&sv.cash==985&&hj_phase()==HJ_IDLE&&nv.phone[NP_BAG+2]==7);

 linux_disk_shutdown();puts("PASS Harbor Office: three complete commissions, owned car and quay, skiff/buoy movement, optional moving target, save/relaunch and one-time payouts.");return 0;
}
