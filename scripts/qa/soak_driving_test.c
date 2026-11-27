/* Synthetic test careers. All gameplay state uses game.c's own input,
   camera, transaction and collision paths; no access to host save slots. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int changed(void){static U frame[W*H];static int first=1;
 int pixelsChanged=0;
 if(!first)for(int i=0;i<W*H;i++)if(pixels[i]!=frame[i])pixelsChanged++;
 cp(frame,pixels,sizeof frame);first=0;return pixelsChanged;
}
static void empty(void){
 starter_save();scene=1;menu=-1;mapOn=0;end_job();vd_reset();ng_reset();ncr_reset();npc_reset();
 p=(Player){.x=1024,.y=1024,.hp=100,.car=-1};sv.intro=4;
 sv.owned=1;sv.selection=0;sv.carCond[0]=100;sv.dayClock=600;
 nc=2;for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900};
 z(streetDrivers,sizeof streetDrivers);outer_life_reset();clear_input();
}
static void chase_curb(void){
 empty();nv.garage[60]=1;p.car=0;p.x=2750;p.y=1034;outCrowdCount=4;
 NpcRef q;assert(outer_person_ref(N_ACT_OUTER+4,&q,0));
 cars[0]=(Car){p.x,p.y,look(q.x-p.x,q.y-p.y),0,0,1,100};
 ch_draw();changed();outCrowdCount=0;
 ch_draw();int crowd=changed();assert(crowd>30);
 /* A rider outside her ride is solid world art in either camera; one in
    the car must not appear on the pavement. Compare framebuffer pixels. */
 ncr.x=q.x;ncr.y=q.y;ncr.phase=NCR_WAIT;ch_draw();int singer=changed();assert(singer>15);
 ncr.phase=NCR_DRIVE;ch_draw();assert(changed()==singer);
 printf("Chase roadside actors: %d crowd pixels, %d arriving performer pixels, no rider duplicate once aboard.\n",crowd,singer);
}
static void lights(void){
 empty();sv.dayClock=1230;nv.garage[60]=1;p.car=0;p.x=1024;p.y=1024;cars[0]=(Car){p.x,p.y,0,0,0,1,100};
 ngCamRide=-1;assert(ch_draw()&&chDark>.6f);
 static U ground[W*H];ch_ground();cp(ground,pixels,sizeof ground);
 ChPlot near=ch_screen(ch_local((ChVec){1054,1024,0})),far=ch_screen(ch_local((ChVec){1080,1024,0}));
 int nearI=near.y*W+near.x,farI=far.y*W+far.x;
 ch_headlights(cars);assert(pixels[nearI]>ground[nearI]&&pixels[farI]>ground[farI]);
 int count=0;for(int i=0;i<W*H;i++)if(ground[i]!=pixels[i])count++;
 assert(count>75&&count<W*H/8);
 /* Place a controlled barrier into the exact roof mask both renderers query.
    Before opaque scene props draw, no illuminated road remains behind it. */
 unsigned char old[40];for(int i=0;i<40;i++){int id=(1004+i)*ROOM+1062;old[i]=roofMask[id];roofMask[id]=1;}
 ch_ground();ch_headlights(cars);assert(pixels[nearI]>ground[nearI]&&pixels[farI]==ground[farI]);
 for(int i=0;i<40;i++)roofMask[(1004+i)*ROOM+1062]=(unsigned char)old[i];
 ch_ground();chDark=0;ch_headlights(cars);assert(!memcmp(ground,pixels,sizeof ground));
 chDark=1;cars[0].cond=0;ch_headlights(cars);assert(!memcmp(ground,pixels,sizeof ground));
 printf("Chase lights: %d lit pavement pixels at night, no spill behind the barrier or light from a wreck/daytime.\n",count);
}
static void gate(void){
 for(int side=-1;side<=1;side+=2){
  empty();sv.dayClock=400;int x=NP_PARKWAY_X,y=NP_PARKWAY_Y;
  p.x=x-15*side;p.y=y;const int id=1;
  cars[id]=(Car){x+17.5f*side,y,side>0?1.5708f:-1.5708f,0,2,0,100};
  assert(!street_hits_wall(cars+id)&&!solid(p.x,p.y,10));
  assert(night_phone_lane_stop(p.x,p.y,cars[id].x,y,2));
  assert(street_near_car()!=id&&!street_enter_car(id)&&p.car<0);
  sv.dayClock=481;assert(street_near_car()==id&&street_enter_car(id)==1);
  sv.dayClock=400;assert(street_exit_car()&&p.car<0&&(p.x-x)*side>17);
  assert(street_near_car()==id&&street_enter_car(id)==1);
  sv.dayClock=481;assert(street_exit_car()&&p.car<0&&(p.x-x)*side<0);
 }
 puts("Freight Parkway both sides: closed barrier blocks seat-swap crossings; near-side exit/entry and reopened gate stay available.");
}
static void ghost(void){
 empty();ng.event=NG_RACE;float peakD=0,peakA=0;int samples=0;
 for(int r=0;r<ng_courses();r++){
  ng.bookedCourse=r;Car prev={0};
  for(int t=0;t<=ngRun[r].seconds*60;t++){
   ng.frames=t;Car c;assert(ng_ghost_car(&c)&&!solid(c.x,c.y,4));
   if(t){float d=sq(d2(c.x-prev.x,c.y-prev.y)),a=absf(wrap(c.a-prev.a));assert(d<2&&a<.16f);
    if(d>peakD)peakD=d;if(a>peakA)peakA=a;}
   if(!r&&!t){/* First gate runs north, right of course is east. */assert(c.x>ngRun[r].gate[0].x+6);}
   prev=c;samples++;
  }
 }
 printf("Night race ghosts: %d clear frame poses across all five tracks, largest step %.3f units / %.3f radians.\n",samples,peakD,peakA);
}
static void outer_impact(void){
 empty();p.x=2820;p.y=1047;outCrowdCount=4;NpcRef who;const int id=N_ACT_OUTER+N_OUTER_SEATS;
 assert(outer_person_ref(id,&who,0));
 int roster=nc;for(int i=0;i<OUT_TRAFFIC_COUNT;i++)outDriver[i]=(OutDriver){.driven=1,.outOfSight=-10000};
 p.x=who.x-5;p.y=who.y;p.car=1;cars[1]=(Car){p.x,p.y,0,65,0,1,100};
 tick(UP);assert(p.car==1&&p.heat>35&&cars[1].s<45&&outBody[id-N_ACT_OUTER].damage==3);
 assert(!outer_person_ref(id,&who,0)&&nc==roster);
 puts("Physical street input: an Outer Loop curb impact slows the car and triggers the same patrol response as downtown.");
}
static void dock_routes(void){
 for(int east=0;east<2;east++){
  empty();ng.event=NG_FETCH;ng.stageType=2;ng.target=1;ng.frames=9000;
  int x=east?1792:1024,y=east?1088:890;
  cars[1]=(Car){x,y,0,0,2,0,100};p.car=1;p.x=x;p.y=y;
  night_garage_street(0);int gx,gy;assert(ng.event==NG_DROP&&ng_route_goal(&gx,&gy));
  assert(gx==(east?NG_ALT_DOCK_X:NG_DOCK_X)&&gy==(east?NG_ALT_DOCK_Y:NG_DOCK_Y));
  /* Neither GPS nor the contract penalizes using the other intake. */
  p.x=cars[1].x=east?NG_DOCK_X:NG_ALT_DOCK_X;p.y=cars[1].y=east?NG_DOCK_Y:NG_ALT_DOCK_Y;
  assert(ng_route_goal(&gx,&gy)&&gx==(east?NG_ALT_DOCK_X:NG_DOCK_X));
  night_garage_street(ENTER);assert(scene==8&&ng.tab==NG_RETURN);
 }
}
static void wreck_exit(void){
 const int off[][2]={{0,21},{0,-21},{32,0},{-34,0}};
 for(int k=0;k<2;k++){
  empty();sv.owned=0;sv.dayClock=400;nc=6;
  int x=k?NP_PARKWAY_X-22:1024,y=k?NP_PARKWAY_Y:800;
  p.car=1;p.x=x;p.y=y;cars[1]=(Car){x,y,0,125,0,1,1};vehicle_set_health(cars+1,1);
  for(int i=0;i<4;i++){cars[i+2]=(Car){x+off[i][0],y+off[i][1],0,0,1,1,100};assert(!street_overlap(cars+1,cars+i+2)&&!street_hits_wall(cars+i+2));}
  tick(UP);assert(vehicle_hp(cars+1)==0&&p.car<0&&p.hp==73&&street_foot_clear(p.x,p.y,-1));
  assert(!night_phone_lane_stop(x,y,p.x,p.y,7)&&street_line_open(x,y,p.x,p.y,7));
  float px=p.x,py=p.y;U xdir=px>x?RIGHT:LEFT,ydir=py>y?DOWN:UP;
  for(int i=0;i<8;i++)tick(xdir|ydir|BRAKE);
  assert(d2(p.x-px,p.y-py)>8);
 }
 puts("Boxed crash: a destroyed car lands its driver on walkable ground, including next to the live Parkway barrier; regular door restrictions remain.");
}
static void blocked_handoff(void){
 empty();sv.owned=0;ng.event=NG_DROP;ng.frames=9000;ng.target=1;ng.stageType=2;
 p.car=1;int x=NG_DOCK_X,y=NG_DOCK_Y;cars[1]=(Car){x,y,0,0,2,0,100};p.x=x;p.y=y;
 nc=6;const int off[][2]={{0,21},{0,-21},{34,0},{-34,0}};
 for(int i=0;i<4;i++){cars[i+2]=(Car){x+off[i][0],y+off[i][1],0,0,1,1,100};assert(!street_overlap(cars+1,cars+i+2));}
 night_garage_street(ENTER);ng_ui_accept();assert(ng.tab==NG_SALVAGE);
 for(int i=0;i<3;i++){ng.idx=ng.targetToken;ng_ui_accept();}
 assert(ng.tokens==2&&p.car==1&&strstr(ng.toast,"blocked"));
 cars[2].y+=55;ng_ui_accept();assert(ng.event==NG_IDLE&&p.car==-1&&cars[1].x<0&&nv.garage[38]==3);
}
static void intake(void){
 empty();sv.cash=WALLET_LIMIT;sv.dirty=WALLET_LIMIT-20;ng.event=NG_DROP;ng.frames=9000;ng.target=1;ng.stageType=2;
 p.car=1;p.x=cars[1].x=NG_DOCK_X;p.y=cars[1].y=NG_DOCK_Y;cars[1]=(Car){p.x,p.y,0,0,2,0,100};
 nv.garage[38]=19;night_garage_street(ENTER);assert(scene==8&&ng.tab==NG_RETURN);
 ng_ui_accept();assert(ng.tab==NG_SALVAGE);
 for(int i=0;i<3;i++){ng.idx=ng.targetToken;ng_ui_accept();}
 assert(scene==8&&ng.tab==NG_SALVAGE&&ng.tokens==2&&cars[1].cond==100&&p.car==1&&nv.garage[38]==19&&sv.dirty==WALLET_LIMIT-20&&strstr(ng.toast,"wallet"));
 sv.dirty-=22;ng_ui_accept();
 assert(scene==1&&ng.event==NG_IDLE&&p.car==-1&&sv.dirty==WALLET_LIMIT&&cars[1].x<0&&nv.garage[38]==20&&strstr(notice,"+1 salvage credit."));
 sv.cash=WALLET_LIMIT-7;ng.bookedCourse=0;ng.paid=1;ng.frames=377;ng_finish_run();
 assert(sv.cash==WALLET_LIMIT&&strstr(notice,"$7 credited")&&nv.garage[37]);
 cars[0]=(Car){1024,1024,0,0,0,1,100};p.car=0;nv.garage[0]=1;nv.garage[1]=2;nv.garage[36]=nv.garage[39]=0;
 ng_show();assert(!nv.garage[36]&&!nv.garage[39]);
 sv.cash-=32;ng_show();assert(sv.cash==WALLET_LIMIT&&nv.garage[36]&&nv.garage[39]==1);
 puts("Motor Works: capacity-safe fee / one remaining salvage credit, recoverable final stamp, exact race receipt, showcase retained until payout fits.");
}
int main(void){setvbuf(stdout,0,_IOLBF,0);init_city();chase_curb();lights();gate();ghost();outer_impact();dock_routes();wreck_exit();blocked_handoff();intake();return 0;}
