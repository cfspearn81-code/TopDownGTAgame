/* Real fixed-step on-foot input and support-foot mechanics, no save disk. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
static void press(U k){tick(k);tick(0);}
static void setup(int perspective){
 starter_save();end_job();harbor_boats_reset();vd_reset();npc_reset();clear_input();
 scene=1;menu=-1;mapOn=0;fastGfx=0;sv.intro=4;p=(Player){.x=820,.y=1024,.a=0,.hp=100,.car=-1};
 nv.garage[60]=perspective;nc=0;z(streetDrivers,sizeof streetDrivers);z(peds,sizeof peds);
}
static float route(int pref,int run,int fallback){
 setup(pref);float x=p.x,y=p.y;
 U key=RIGHT|(run?BRAKE:0);if(fallback)tick(GFX);
 float first=0,faster=0;for(int i=0;i<40;i++){float old=p.x;tick(key);if(i==0)first=p.x-old;if(i==15)faster=p.x-old;}
 if(pref)assert(first>0&&first<faster*.8f);
 assert(p.y==y&&p.x>x+25);
 if(pref){
  GaitPose s=gait_pose(GAIT_STREET,p.x,p.y,p.a);assert(s.valid&&s.lean>0&&s.foot[0].lift+s.foot[1].lift>0);
  float phase=gait.phase,last=p.x;
  tick(key|GFX);assert(p.x-last>first&&gait.phase!=phase);
  tick(key);last=p.x;
  for(int i=0;i<16;i++){tick(0);paint();assert(p.x==last&&p.y==y);}
  GaitPose at=gait_pose(GAIT_STREET,p.x,p.y,p.a);assert(!at.lean&&!at.foot[0].lift&&!at.foot[1].lift);
  return last-x;
 }
 return p.x-x;
}
static void shift_speed(void){
 setup(1);for(int i=0;i<14;i++)tick(RIGHT);
 float x=p.x;tick(RIGHT);float walk=p.x-x;
 x=p.x;tick(RIGHT|BRAKE);float push=p.x-x;
 for(int i=0;i<19;i++)tick(RIGHT|BRAKE);
 x=p.x;tick(RIGHT|BRAKE);float sprint=p.x-x;
 x=p.x;tick(RIGHT);float pull=p.x-x;
 assert(walk<push&&push<sprint&&walk<pull&&pull<sprint&&push-walk<.45f*(sprint-walk));
}
static void feet(void){
 setup(1);float x=p.x;int held=0,landed=0;
 for(int i=0;i<70;i++){
  GaitPose a=gait_pose(GAIT_STREET,p.x,p.y,p.a);float old=p.x;
  tick(RIGHT);GaitPose b=gait_pose(GAIT_STREET,p.x,p.y,p.a);
  if(i>40&&gait.weight==1.f)for(int j=0;j<2;j++){
   if(a.foot[j].planted&&b.foot[j].planted&&absf(a.foot[j].fore-b.foot[j].fore)<2.2f){
    assert(absf(old+a.foot[j].fore-p.x-b.foot[j].fore)<.065f);held++;
   }
   if(!a.foot[j].planted&&b.foot[j].planted)landed++;
  }
 }
 assert(held>=8&&landed>=1&&p.x>x+60);
 int solidSteps=0;for(int i=0;i<240;i++){float bx=p.x,by=p.y;tick(UP);if(p.x==bx&&p.y==by)solidSteps++;}
 assert(solidSteps>15);GaitPose m=gait_pose(GAIT_STREET,p.x,p.y,p.a);assert(!m.lean&&!m.foot[0].lift&&!m.foot[1].lift);
}
static void actual_room(void){
 setup(1);p.x=loc[1].x;p.y=loc[1].y;tick(0);press(USE);assert(scene==3&&interior_active_loc==1);
 float home=interior_x,oy=interior_y;
 for(int i=0;i<24;i++)tick(RIGHT);
 assert(interior_x>home+19&&interior_x<home+32&&interior_y==oy&&gait.mode==GAIT_NAMED);
 for(int i=0;i<90&&!int_at_door();i++)tick(interior_x>W/2?LEFT:RIGHT);
 assert(int_at_door());tick(0);press(USE);assert(scene==1&&p.x==loc[1].x&&p.y==loc[1].y);
}
int main(void){
 init_city();float w=route(0,0,0),r=route(0,1,0),x=route(1,0,0),y=route(1,1,0),z0=route(1,1,1);
 assert(x<w*.74f&&x>w*.59f&&y<r*.79f&&y>r*.65f&&y>x*1.44f&&absf(z0-y)<.004f);
 shift_speed();feet();actual_room();printf("PASS gait street 3D %.2f/%.2f vs overhead %.2f/%.2f; F2 gait match; grounded contact, no wall treadmill and real room entry/exit.\n",x,y,w,r);
}
