/* Presentation-only test. Steer actual chassis on open road and verify fixed
 * input parity, steering/suspension visual time, and deterministic redraw. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <string.h>
typedef struct{Car c;float x,y,a;int cash,dirty,heat,hp;} Trip;
static void key(U k){tick(k);tick(0);}
static void frame(const char*folder,const char*name){
 paint();if(!folder)return;
 char path[512];snprintf(path,sizeof path,"%s/%s.ppm",folder,name);
 FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U v=pixels[i];fputc(v>>16,f);fputc(v>>8,f);fputc(v,f);}assert(!fclose(f));
}
static void prep(void){
 set_session_seed(72);init_city();scene=1;menu=-1;mapOn=0;end_job();vd_reset();z(streetDrivers,sizeof streetDrivers);
 sv.intro=4;sv.dayClock=790;sv.owned=8;sv.selection=3;sv.carCond[3]=100;sv.cash=180;sv.hp=100;
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
 nc=1;cars[0]=(Car){902,1024,0,0,3,1,100};p.car=0;p.x=cars[0].x;p.y=cars[0].y;
 ngCamRide=-1;fastGfx=0;clear_input();nv.garage[60]=0;
}
static Trip course(int perspective,const char*folder){
 prep();key(perspective?CAMERA:0);
 for(int i=0;i<99;i++)tick(0);
 if(perspective)frame(folder,"01-coachwork-idle");
 CvMotion idle={0};if(perspective){const CvMotion*m=cv_motion(cars);assert(m);idle=*m;assert(absf(idle.phase)<.001f);}
 for(int i=0;i<43;i++)tick(UP);
 if(perspective){
  const CvMotion*m=cv_motion(cars);assert(m&&absf(m->phase-idle.phase)>.09f&&m->pitch<-.007f&&absf(m->steer)<.02f);
  frame(folder,"02-coachwork-throttle");
 }
 float roll=0,steer=0;
 for(int i=0;i<27;i++){
  tick(UP|BRAKE|RIGHT);
  if(perspective){
   const CvMotion*m=cv_motion(cars);assert(m);if(m->roll>roll)roll=m->roll;if(m->steer>steer)steer=m->steer;
   if(i==14)frame(folder,"03-coachwork-steering");
  }
 }
 if(perspective){
  const CvMotion*m=cv_motion(cars);assert(m&&roll>.010f&&steer>.19f);
  U snapshot=night_hash(pixels,sizeof pixels);CvMotion pose=*m;
  paint();snapshot=night_hash(pixels,sizeof pixels);
  paint();assert(!memcmp(&pose,m,sizeof pose)&&night_hash(pixels,sizeof pixels)==snapshot);
 }
 for(int i=0;i<133;i++)tick(BRAKE);
 if(perspective){
  const CvMotion*m=cv_motion(cars);assert(m&&absf(m->heave)<.030f&&absf(m->pitch)<.009f&&absf(m->roll)<.008f&&absf(m->steer)<.009f);
  key(MAP);assert(mapOn);CvMotion phase=*m;
  for(int i=0;i<86;i++){tick(0);if(i%20==0)paint();}
  assert(!memcmp(&phase,m,sizeof phase));key(MAP);assert(!mapOn);key(CAMERA);
 }else{
  key(MAP);for(int i=0;i<86;i++)tick(0);key(MAP);key(0);
 }
 Trip s={0};s.c=cars[0];s.x=p.x;s.y=p.y;s.a=p.a;s.cash=sv.cash;s.dirty=sv.dirty;s.heat=(int)p.heat;s.hp=p.hp;return s;
}
int main(int argc,char**argv){
 Trip x=course(0,0),y=course(1,argc>1?argv[1]:0);
 assert(x.c.x==y.c.x&&x.c.y==y.c.y&&x.c.a==y.c.a&&x.c.s==y.c.s&&x.c.type==y.c.type&&x.c.cond==y.c.cond&&x.c.wear==y.c.wear);
 assert(x.x==y.x&&x.y==y.y&&x.a==y.a&&x.cash==y.cash&&x.dirty==y.dirty&&x.heat==y.heat&&x.hp==y.hp);
 puts("PASS: real acceleration, brake and turn; independent wheel travel/load, redraw & atlas freeze, exact physics/cash/health parity.");
}
