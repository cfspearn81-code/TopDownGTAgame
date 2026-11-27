/* Native shared-engine fixtures. This file is also the synthetic prior-slot
 * writer for harbor_boats_host.py. Never reads a real player's career. */
#define DC_TEST
#include "game.c"
#include <assert.h>
static void input(U k,int frames){for(int i=0;i<frames;i++)tick(k);}
static void press(U k){tick(k);tick(0);}
static void prep(int dock){
 init_city();starter_save();sv.intro=4;sv.dayClock=1270;scene=1;menu=-1;mapOn=0;
 harbor_boats_reset();bank_ui_reset();npc_reset();clear_input();vd_reset();
 int x,y;assert(harbor_dock_point(dock*2,&x,&y));
 p=(Player){.x=x,.y=y,.car=-1,.hp=100,.a=1.57f};
 assert(street_foot_clear(p.x,p.y,-1));
}
static void fixture(const char*destination){
 prep(0);Save11 save={0};
 save.base=sv;save.base.x=1792;save.base.y=1792;save.base.rev=NIGHT_SAVE_REV;
 save.night.world[NW_SAVE_X]=(U)(p.x*NW_SAVE_SCALE+.5f)|0x80000000u;
 save.night.world[NW_SAVE_Y]=(U)(p.y*NW_SAVE_SCALE+.5f);
 save.base.hash=night_hash(&save.base,sizeof save.base-4);
 save.hash=night_hash(&save,sizeof save-4);
 FILE*f=fopen(destination,"wb");assert(f&&fwrite(&save,sizeof save,1,f)==1&&!fclose(f));
}
static void frame(const char *folder,const char*label){
 if(!folder)return;char name[525];snprintf(name,sizeof name,"%s/%s.ppm",folder,label);
 paint();FILE*f=fopen(name,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc(c>>16,f);fputc(c>>8,f);fputc(c,f);}assert(!fclose(f));
}
static void physical_course(const char*folder){
 prep(0);float homeX=p.x,homeY=p.y;
 frame(folder,"01-landing");
 press(USE);assert(harbor_boat_active()&&p.car<0&&harbor_boat_nav_clear(p.x,p.y));
 const float origin=p.y;
 assert(harbor_boat_speed()==0&&!harbor_boat_hit(p.x,p.y,8,0));
 int contact=0;
 for(int i=0;i<85;i++){
  tick(UP);assert(harbor_boat_clear(p.x,p.y,hb.hull.a));
  if(harbor_boat_hit(homeX,origin+48,8,38.f))contact=1;
  assert(!harbor_boat_hit(homeX-100,p.y,8,1));
 }
 assert(p.y>origin+75.f&&harbor_boat_speed()>115.f&&contact);
 frame(folder,"02-launch-wake");
 float traveled=p.y;press(HELP);assert(scene==2&&harbor_boat_active()&&p.y==traveled);
 press(ESC);assert(scene==1&&harbor_boat_active());
 press(PHONE);assert(scene==NIGHT_PHONE_SCENE&&harbor_boat_active());
 press(PHONE);assert(scene==1&&harbor_boat_active());
 press(MAP);assert(mapOn&&harbor_boat_active());press(MAP);assert(!mapOn&&harbor_boat_active());
 press(SAVE);assert(sv.x==homeX&&sv.y==homeY&&p.y>origin+75.f);
 press(USE);assert(harbor_boat_active()); /* Never exit over deep water. */
 float a=hb.hull.a,x0=p.x;input(UP|RIGHT,30);
 assert(wrap(hb.hull.a-a)>.44f&&p.x<x0-4.f);
 a=hb.hull.a;input(UP|LEFT,25);assert(wrap(hb.hull.a-a)<-.36f);
 input(BRAKE,64);
 assert(absf(harbor_boat_speed())<.20f&&hb.condition==100&&p.hp==100);
 frame(folder,"03-harbor-water");
 /* Exact two-point turn: steer toward an authored cargo berth and arrive
  * on the physical hull, not by altering the coordinates. */
 int bx,by,lx,ly;assert(hb_dock(1,&lx,&ly,&bx,&by));
 float targetX=bx,targetY=by;
 int reached=0;
 for(int i=0;i<1600;i++){
  float dx=targetX-p.x,dy=targetY-p.y,n=sq(d2(dx,dy)),e=wrap(look(dx,dy)-hb.hull.a);
  if(n<15.f){
   input(BRAKE,45);if(hb_near_dock(p.x,p.y,1)==1&&d2(hb.hull.vx,hb.hull.vy)<17.f*17.f){press(USE);reached=!hb.active;break;}
  }
  U k=0;if(absf(e)>.065f)k|=e>0?RIGHT:LEFT;
  if(absf(e)<.47f&&n>=15.f)k|=UP;
  if(absf(e)>.55f||n<44||harbor_boat_speed()>92)k|=BRAKE;
  tick(k);assert(harbor_boat_clear(p.x,p.y,hb.hull.a));
 }
 if(!reached)fprintf(stderr,"Cargo transfer unreached; at %f, %f, hull angle %.2f, speed %.1f.\n",p.x,p.y,hb.hull.a,hb.hull.s);
 assert(reached&&street_foot_clear(p.x,p.y,-1)&&d2(p.x-lx,p.y-ly)<30*30);
 frame(folder,"04-cargo-arrival");
 press(USE);assert(harbor_boat_active());press(USE);assert(!harbor_boat_active());
 puts("E launch, thrust/sliding steering, hull masking, one-frame contact, F1/P/M while aboard, F5 dock anchor, physical deposit at a second quay, reusable launch: passed.");
}
static void shore_and_traffic(void){
 for(int i=0;i<HB_DOCKS;i++){
  prep(i);press(USE);assert(harbor_boat_active());press(USE);assert(!harbor_boat_active()&&street_foot_clear(p.x,p.y,-1));
 }
 prep(0);press(USE);int wx,wy,lx,ly;assert(hb_dock(0,&lx,&ly,&wx,&wy));
 input(UP,76);input(BRAKE,70);
 hb.condition=1;hb.hull.a=-1.570796f;hb.hull.s=146;
 hb.hull.vx=0;hb.hull.vy=-146;
 for(int i=0;i<210&&hb.active;i++)tick(UP);
 assert(!hb.active&&p.car<0&&p.hp==100&&street_foot_clear(p.x,p.y,-1)&&d2(p.x-lx,p.y-ly)<32*32);
 prep(0);press(USE);input(UP,170);input(BRAKE,70);
 /* Make the world clock advance while remaining afloat. Repeat braking
  * at rest, enough to trigger the ordinary scheduled patrol appearance. */
 int civilians=0,patrol=0;float moved[HB_AMBIENT]={0};
 for(int t=0;t<2200;t++){
   HarborTraffic before[HB_AMBIENT];cp(before,hbTraffic,sizeof before);
   tick(0);
   for(int i=0;i<HB_AMBIENT;i++)if(hbTraffic[i].live){
    const HarborHull*h=&hbTraffic[i].hull;assert(harbor_boat_clear(h->x,h->y,h->a));
    float distance=sq(d2(h->x-before[i].hull.x,h->y-before[i].hull.y));assert(distance<2.f);moved[i]+=distance;
    if(h->kind==HB_PATROL)patrol++;else civilians++;
   }
 }
 assert(civilians>5000&&patrol>100&&moved[HB_AMBIENT-1]>25.f);
 puts("All three docks, actual water boundary collision/tow bailout, several distinct compliant ambient boats, occasional moving patrol: passed.");
}
static void far_south_helm(void){
 /* The public channel connects to open southern water well west of port.
  * Leaving the harbor traffic update rectangle must not erase the helm. */
 prep(0);press(USE);assert(hb.active);
 assert(hb_lane_open(hb.hull.x,hb.hull.y,hb.hull.x,2920.f));
 assert(hb_lane_open(hb.hull.x,2920.f,1540.f,2920.f));
 p.x=hb.hull.x=1540; p.y=hb.hull.y=2920; p.a=hb.hull.a=3.14f;
 sv.dayClock=750;menu=-1;mapOn=0;fastGfx=0;tick(0);paint();
 int n=0;for(int i=0;i<W*H;i++)if(pixels[i]==0xdcf6df)n++;
 assert(n>2);
 press(CAMERA);paint();assert(chProjected);
 ChVec pilot=ch_local((ChVec){hb.hull.x-2.65f*cs(hb.hull.a),hb.hull.y-2.65f*sn(hb.hull.a),9});
 ChPlot mark=ch_screen(pilot);
 assert(mark.x>1&&mark.y>1&&mark.x<W-1&&mark.y<H-1);
 int depth=0;for(int y=mark.y-1;y<=mark.y+1;y++)for(int x=mark.x-1;x<=mark.x+1;x++)depth=max(depth,chZ[y*W+x]);
 assert(depth>(int)(CH_ZS/(pilot.z+3.4f)));
 puts("Southern open-water channel stays visible outside the port, overhead and in C view.");
}
int main(int argc,char**argv){
 if(argc==3){fixture(argv[2]);return 0;}
 physical_course(argc>1?argv[1]:0);shore_and_traffic();far_south_helm();return 0;
}
