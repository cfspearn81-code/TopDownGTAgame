/* Exercise agreements between the combined simulation and render views.
 * Uses no real career or replacement gameplay source. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
static void cam_fixture(void){
 init_city();starter_save();scene=1;sv.intro=4;sv.dayClock=1220;sv.owned=1;sv.selection=3;nc=1;
 z(peds,sizeof peds);z(streetDrivers,sizeof streetDrivers);outer_life_reset();
 p=(Player){.x=952,.y=1024,.car=0,.hp=100};cars[0]=(Car){.x=p.x,.y=p.y,.a=0,.type=3,.owner=1,.cond=100};
 nv.garage[60]=1;ngCamAngle=0;ngCamRide=0;fastGfx=0;
}
static int rider_pixels(int x,int y){
 int n=0;for(int yy=max(0,y-9);yy<min(H,y+11);yy++)for(int xx=max(0,x-11);xx<min(W,x+12);xx++)n+=pixels[yy*W+xx]==0xf18acb;
 return n;
}
static void ride_fixture(void){
 cam_fixture();
 ncr.phase=NCR_WAIT;ncr.x=1052;ncr.y=1024;ncr.a=.9f;
 StreetPerson sp;int marker;
 assert(street_night_rider_pose(&sp,&marker)&&marker&&sp.x==1052&&sp.y==1024&&sp.a==.9f&&sp.step==0);
 U start=night_hash(&ncr,sizeof ncr);
 paint();
 ChPlot rider=ch_screen(ch_local((ChVec){ncr.x,ncr.y,16.5f}));
 assert(chProjected&&rider_pixels(rider.x,rider.y)>4&&night_hash(&ncr,sizeof ncr)==start);
 ncr.phase=NCR_DRIVE;paint();
 assert(chProjected&&rider_pixels(rider.x,rider.y)==0&&!street_night_rider_pose(&sp,&marker));
 p.car=-1;nv.garage[60]=0;ncr.phase=NCR_WAIT;paint();assert(!chProjected&&rider_pixels(sx(ncr.x),sy(ncr.y)+5)>4);
 ncr.phase=NCR_LEAVE;assert(street_night_rider_pose(&sp,&marker)&&!marker);
}
static void hold_fixture(void){
 cam_fixture();p.car=-1;p.x=2708;p.y=435;night_world_reset();outer_life_tick(1.f/60.f);
 assert(outer_walk_count()==24);
 NpcRef first;int found=-1;
 for(int i=0;i<SP_OUT_MAX;i++){
  StreetPerson pose;NpcRef raw;
  if(!street_outer_pose(i,&pose)||!npc_ref(N_ACT_OUTER+i,&raw))continue;
  if(absf(pose.x-p.x)>145.f||absf(pose.y-p.y)>145.f)continue;
  assert(pose.x==raw.x&&pose.y==raw.y);first=raw;found=i;break;
 }
 assert(found>=0);
 p.x=first.x+12;p.y=first.y;assert(npc_start_talk(first));
 for(int n=0;n<90;n++){
  StreetPerson pose;NpcRef raw;
  outer_life_tick(1.f/60.f);
  assert(street_outer_pose(found,&pose)&&npc_ref(N_ACT_OUTER+found,&raw));
  assert(pose.x==first.x&&pose.y==first.y&&pose.step==0&&pose.x==raw.x&&pose.y==raw.y&&pose.a==raw.a);
 }
 npc_end_talk(1);
}
int main(void){
 ride_fixture();hold_fixture();puts("Render merge passes: real Afterimage pickup/alight role in both cameras; regional rendered feet match held and selectable actor pose.");return 0;
}
