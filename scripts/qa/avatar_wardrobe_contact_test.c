/* Regression on live game-state advance and the genuine perspective renderer.
 * Reusable with exact composed src, no writes to career files. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static U region(int x,int y,int w,int h){
 U k=2166136261u;for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)k=(k^pixels[j*W+i])*16777619u;return k;
}
static U gait_state(void){
 U out=night_hash(&p,sizeof p);out=(out^gaitFrame)*16777619u;out=(out^night_hash(&gait,sizeof gait))*16777619u;
 out=(out^night_hash(peds,sizeof peds))*16777619u;return (out^night_hash(cars,sizeof cars))*16777619u;
}
static void shot(const char*dir,const char*file){
 if(!dir)return;char path[512];snprintf(path,sizeof path,"%s/%s.ppm",dir,file);
 FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc(c>>16,f);fputc(c>>8,f);fputc(c,f);}assert(!fclose(f));
}
static U outfit(void){
 U a=ward_set(0,CW_HAT,4);a=ward_set(a,CW_HAT_TONE,2);a=ward_set(a,CW_TRIM,5);
 a=ward_set(a,CW_OUTERWEAR,1);a=ward_set(a,CW_ACCESSORY,6);a=ward_set(a,CW_FOOT_TONE,2);
 return ward_set(ward_set(a,CW_HAIR,1),CW_FACE,3);
}
static void fixture(void){
 init_city();starter_save();end_job();harbor_boats_reset();vd_reset();npc_reset();clear_input();
 scene=1;menu=-1;mapOn=0;fastGfx=0;sv.intro=4;p=(Player){.x=820,.y=1024,.a=0,.hp=100,.car=-1};
 nv.garage[60]=1;ch_reset_spring();nc=0;z(streetDrivers,sizeof streetDrivers);z(peds,sizeof peds);
}
int main(int argc,char**argv){
 const char*d=argc==2?argv[1]:NULL;
 fixture();assert(ch_mode_active());int airborne=0,stand=0,left=0;U bare;
 for(int i=0;i<61;i++){
  tick(i<43?RIGHT:(i<53?UP|BRAKE:0));
  GaitPose gp=gait_pose(GAIT_STREET,p.x,p.y,p.a);
  assert(gp.valid);
  if(gp.foot[0].lift+gp.foot[1].lift>.11f)airborne++;
  if(gp.foot[0].planted||gp.foot[1].planted)stand++;
  if(absf(gp.turn)>.05f)left++;
  if(i==28||i==43||i==60){
   U state=gait_state();ward_store(0);paint();assert(chProjected);bare=region(335,115,275,290);
   ward_store(outfit());paint();U styled=region(335,115,275,290);
   assert(bare!=styled);assert(gait_state()==state);
   paint();assert(region(335,115,275,290)==styled&&gait_state()==state);
   if(i==43)shot(d,"02-worn-style-turn");
   if(i==28)shot(d,"01-worn-style-walk");
  }
 }
 printf("Ground states: airborne=%d contact=%d turn=%d X=%.2f Y=%.2f\n",airborne,stand,left,p.x,p.y);fflush(stdout);assert(airborne>25&&stand>12&&left>0);
 /* The exact same head is drawn behind real room depth, not overlayed. */
 open_interior(12);assert(scene==3);for(int i=0;i<18;i++)tick(RIGHT);
 int beforeMode=gait.mode;U state=gait_state();ward_store(0);paint();bare=region(315,100,335,345);
 ward_store(outfit());paint();U styled=region(315,100,335,345);
 assert(gait.mode==beforeMode&&gait_state()==state&&bare!=styled);
 shot(d,"03-worn-style-room");
 /* The character uses both packed layers simultaneously. */
 nv.world[4]|=3u<<8;assert(night_gear_mask()&&ward_hat(ward_live())==4);
 paint();
 assert(styled!=region(315,100,335,345)&&ward_live()==outfit()&&gait_state()==state);
 shot(d,"04-mask-under-hat-room");
 puts("Wardrobe/Street Contact: worn hat/coat/gloves/sole and mask passed at turn, on planted/airborne stride, indoors on scaled rig, and repeated paints are state-pure.");
}
