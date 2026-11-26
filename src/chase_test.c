/* Real control replay for angle/boom, perspective projection, obstacle sweep,
 * occlusion and fallback. Optional argument: directory for unretouched frames. */
#define DC_TEST
#include "game.c"
#include <assert.h>
static char *out;
static void frame(const char*name){
 paint();if(!out)return;char path[400];snprintf(path,sizeof path,"%s/%s.ppm",out,name);
 FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U v=pixels[i];fputc(v>>16,f);fputc(v>>8,f);fputc(v,f);}fclose(f);
}
static void setup(float heading){
 init_city();scene=1;menu=-1;mapOn=0;end_job();vd_reset();z(streetDrivers,sizeof streetDrivers);
 sv.intro=4;sv.dayClock=1243;sv.owned=8;sv.selection=3;sv.carCond[3]=100;sv.cash=180;sv.hp=100;
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
 float co=cs(heading),si=sn(heading);
 nc=1;cars[0]=(Car){1024-co*122,1024-si*122,heading,0,3,1,100};p.car=0;p.x=cars[0].x;p.y=cars[0].y;
 ngCamRide=-1;fastGfx=0;clear_input();
}
static int car_marks(void){
 int n=0;for(int y=H/2;y<H-100;y++)for(int x=W/2-110;x<W/2+110;x++)if(pixels[y*W+x]==0xc84b44||pixels[y*W+x]==0xff5148)n++;
 return n;
}
static void corner(int sign){
 setup(sign>0?0.f:1.5707963f);
 tick(CAMERA);assert(ch_pref());tick(0);
 for(int i=0;i<99;i++)tick(0);
 float idle=ch.range;assert(idle>78&&idle<80);frame(sign>0?"01-chase-idle":"06-left-idle");assert(chProjected&&car_marks()>24);
 int x=(int)cars[0].x-cx+130*cs(cars[0].a),y=(int)cars[0].y-cy+130*sn(cars[0].a);ch_project(&x,&y);
 assert(absf(x-W/2)<6&&y>125&&y<245); /* in front is straight above own bumper */
 for(int i=0;i<43;i++)tick(UP);
 float speed=ch.range;assert(speed>idle+15);frame(sign>0?"02-chase-throttle":"07-left-throttle");
 float lag=0;for(int i=0;i<27;i++){tick(UP|BRAKE|(sign>0?RIGHT:LEFT));float q=absf(wrap(cars[0].a-ngCamAngle));if(q>lag)lag=q;
  if(i==14){frame(sign>0?"03-chase-right":"08-chase-left");assert(chProjected&&car_marks()>12);}}
 assert(lag>.26f&&lag<.7f&&p.car==0&&cars[0].cond==100);
 for(int i=0;i<74;i++)tick(UP);
 assert(absf(wrap(cars[0].a-ngCamAngle))<.003f);
 frame(sign>0?"04-chase-straight":"09-left-exit");assert(chProjected&&car_marks()>20);
 for(int i=0;i<92;i++)tick(BRAKE);
 assert(ch.range>78.0f&&ch.range<80.5f);
 printf("chase %s: range %.1f idle, %.1f throttle, %.1f stopped; heading lag %.3f rad.\n",sign>0?"right":"left",idle,speed,ch.range,lag);
 tick(GFX);frame(sign>0?"05-low-fx":"10-left-low-fx");assert(!chProjected&&ch_pref());
 tick(0);tick(GFX);paint();assert(chProjected);
 tick(0);tick(CAMERA);paint();assert(!chProjected&&!ch_pref());
}
static void depth(void){
 setup(0);tick(CAMERA);p.x=cars[0].x=975;p.y=cars[0].y=1008;
 cars[0].a=.7f;ngCamAngle=.7f;ngCamRide=0;ch.range=126.f;ch.ride=0;ch.height=39;
 frame("11-wall-corner");assert(chProjected&&car_marks()>4);
 /* Boom clamps against an actual building envelope; the car still renders. */
 int occluded=0;
 for(int i=0;i<nb&&!occluded;i++){
  StreetBounds b=building_bounds(buildings+i);
  float x=b.x+b.w+22,y=b.y+b.h*.55f;if(solid(x,y,12)||!solid(x-45,y,4))continue;
  p.x=cars[0].x=x;p.y=cars[0].y=y;cars[0].a=ngCamAngle=0;ch.range=100;ch.height=38;
  frame("12-wall-sweep");int stripe=0;for(int j=0;j<W*H;j++)if(pixels[j]==0xf7ddb6)stripe++;
  assert(cars[0].x-ch.x<55&&cars[0].x-ch.x>12&&ch.h>=42&&stripe>12);occluded=1;
 }
 assert(occluded);
}
int main(int argc,char **argv){if(argc>1)out=argv[1];corner(1);corner(-1);depth();puts("Perspective chase tracking/near clips, spring distance, line of sight, and reversible C/F2 passed.");return 0;}
