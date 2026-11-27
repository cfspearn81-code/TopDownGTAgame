/* Real fixed-step controls against a quiet open street, without writing a save.
 * Run: cc -std=c11 -O2 scripts/qa/chase_drive_motion_test.c -o build/chase-drive-motion */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>

enum{DRIVE_TICKS=414};
static const struct{int count;U keys;} phase[]={
 {1,0},{95,UP},{18,UP|BRAKE|RIGHT},{48,UP|LEFT},{85,DOWN},
 {160,BRAKE},{7,0}
};
typedef struct {float x,y,a,s,heat;int cond,car;} RunPose;
static RunPose poses[DRIVE_TICKS];
static void fresh(int perspective){
 starter_save();end_job();vd_reset();npc_reset();clear_input();ng_cam_reset();
 harbor_boats_reset();bank_ui_reset();night_world_reset();outer_life_reset();
 menu=-1;mapOn=fastGfx=0;scene=1;sv.owned=1;sv.intro=4;sv.hp=100;sv.carCond[0]=100;
 for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100.f,.y=-100.f};
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
 z(streetDrivers,sizeof streetDrivers);
 /* Preserve surveyed block/camera geometry, quiet non-player simulation,
  * and leave grid start and street limits intact. Native tests drive the
  * unchanged map below separately, without this controlled roof mask. */
 z(roofMask,sizeof roofMask);
 nc=1;p=(Player){.x=956,.y=1024,.a=0,.hp=100,.car=0};
 cars[0]=(Car){956,1024,0,0,0,1,100};
 nv.garage[60]=perspective;
}
static void assert_mount(float*peakStep,float*peakHeight,float*peakOrbit,float*nearLook,float*farLook,float*compression){
 ChMount q=ch_mount();
 assert(q.x==cars[0].x&&q.y==cars[0].y&&q.subject==0);
 float exact=ch_clearance(q,cs(q.a),sn(q.a));ch_frame_rig();
 assert(chFit<=exact+.001f&&chFit>=1&&ch.h>=31.f&&ch.h<=83.f);
 if(absf(chDrive.orbit)>*peakOrbit)*peakOrbit=absf(chDrive.orbit);
 if(chDrive.lead<*nearLook)*nearLook=chDrive.lead;if(chDrive.lead>*farLook)*farLook=chDrive.lead;
 if(chDrive.brake>*compression)*compression=chDrive.brake;
 static float prev=79,prevHeight=38;
 if(absf(ch.range-prev)>*peakStep)*peakStep=absf(ch.range-prev);
 if(absf(ch.height-prevHeight)>*peakHeight)*peakHeight=absf(ch.height-prevHeight);
 prev=ch.range;prevHeight=ch.height;
 if(chFit>15){
  ChVec in=ch_local((ChVec){q.x,q.y,4});ChPlot s=ch_screen(in);
  assert(s.z>0&&s.x>=W/2-2&&s.x<=W/2+2&&s.y>245&&s.y<432);
 }
}
static void sequence(int perspective){
 fresh(perspective);int frame=0;
 float peakStep=0,peakHeight=0,peakOrbit=0,nearLook=0,farLook=0,compression=0,peakYaw=0,lastYaw=0;
 for(int i=0;i<LEN(phase);i++)for(int j=0;j<phase[i].count;j++,frame++){
  assert(frame<DRIVE_TICKS);
  U k=phase[i].keys;
  tick(k);
  const Car*c=cars;RunPose now={c->x,c->y,c->a,c->s,p.heat,c->cond,p.car};
  assert(p.car==0&&c->cond>0);
  if(!perspective){poses[frame]=now;continue;}
  assert(!memcmp(&poses[frame],&now,sizeof now));
  assert_mount(&peakStep,&peakHeight,&peakOrbit,&nearLook,&farLook,&compression);
  float yaw=ch_mount().a,delta=absf(wrap(yaw-lastYaw));if(frame&&delta>peakYaw)peakYaw=delta;
  lastYaw=yaw;
  if(frame==0){assert(ch.range==79&&ch.height==38&&chDrive.lead==0&&chDrive.orbit==0);}
  if(frame%48==0){
   float look=chDrive.lead,h=ch.h,x=ch.x,y=ch.y;
   for(int j=0;j<3;j++){paint();assert(chProjected&&chDrive.lead==look&&ch.h==h&&ch.x==x&&ch.y==y);}
  }
 }
 assert(frame==DRIVE_TICKS);
 if(!perspective)return;
 assert(peakStep<=1.321f&&peakHeight<=.741f&&peakOrbit>.025f&&peakOrbit<=.073f&&peakYaw<.081f);
 assert(nearLook< -3.f&&farLook>7.f&&compression>.12f);
 assert(absf(chDrive.lead)<.07f&&absf(chDrive.orbit)<.002f&&absf(ch.range-79.f)<.2f);
 printf("Road camera: %d identical car/heat poses; signed sightline %.1f .. %.1f units; corner orbit %.3f rad (max yaw step %.3f); brake pulse %.2f; max boom/height rise %.2f / %.2f per tick.\n",
  frame,nearLook,farLook,peakOrbit,peakYaw,compression,peakStep,peakHeight);
 tick(GFX);paint();assert(ch_pref()&&!chProjected);tick(0);tick(GFX);paint();
 assert(chProjected&&chDrive.orbit==0.f&&absf(chDrive.lead)<.01f);
 tick(0);tick(CAMERA);paint();assert(!ch_pref()&&!chProjected);
 tick(0);tick(CAMERA);paint();assert(ch_pref()&&chProjected);
 tick(0);tick(USE);paint();assert(p.car<0&&ch_subject()==CH_FOOT&&chProjected);
 puts("Parked F2, C twice, and E disembark return to their exact presentation modes.");
}
int main(void){init_city();sequence(0);sequence(1);}
