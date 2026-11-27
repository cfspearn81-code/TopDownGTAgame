/* Press the same keys the player uses, with real world collision. No saves. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
static void press(U bits){tick(0);tick(bits);tick(0);}
static void fresh(void){init_city();scene=1;menu=-1;mapOn=fastGfx=0;sv.intro=4;p.hp=100;p.car=-1;p.dead=0;clear_input();}
static void panels(void){
 fresh();p.x=p.y=1024;
 press(CAMERA);assert(ch_pref()&&ch_enabled());paint();assert(chProjected&&ch_subject()==CH_FOOT);
 float x=p.x,y=p.y,front=ch.co;for(int i=0;i<8;i++)tick(RIGHT);
 assert(p.x>x+1&&p.y==y);paint();assert(absf(ch.co-front)<.00001f&&ch.si<-.99f);
 press(MAP);assert(mapOn);press(CAMERA);assert(ch_pref());paint();assert(!chProjected);press(ESC);
 press(HELP);assert(scene==2);press(CAMERA);assert(ch_pref());press(ESC);assert(scene==1);
 press(PHONE);assert(scene==NIGHT_PHONE_SCENE);press(CAMERA);assert(ch_pref());press(ESC);assert(scene==1);
 press(WHEEL);/* release closes; while held C must not trigger. */
 tick(WHEEL);tick(WHEEL|CAMERA);assert(ch_pref());tick(0);
 press(GFX);assert(!ch_enabled()&&ch_pref());paint();assert(!chProjected);press(GFX);paint();assert(chProjected);
 open_interior(12);assert(scene==3);press(CAMERA);assert(!ch_pref());press(CAMERA);assert(ch_pref());
 press(ESC);assert(menu>=0);press(CAMERA);assert(ch_pref());press(ESC);assert(menu<0);int_leave();
 p.x=2948;p.y=2206;
 press(USE);assert(harbor_boat_active()&&ch_subject()==CH_HELM&&ch_enabled());
 for(int i=0;i<18;i++)tick(UP);paint();assert(chProjected&&chFit>75);
 for(int i=0;i<50;i++)tick(BRAKE);
 press(USE);assert(!harbor_boat_active()&&ch_subject()==CH_FOOT&&ch_enabled());
 printf("C transitions: foot, map/guide/phone/wheel, F2 fallback, room/pause, helm + moor checked.\n");
}
static void inverse(void){
 fresh();p.x=p.y=1024;press(CAMERA);ch_frame_rig();
 float worst=0,oldWorst=0;int n=0;
 for(int k=0;k<3;k++){
  if(k==1){chSin=.25f;chCos=sq(1-chSin*chSin);ch.h=38;}
  if(k==2){chSin=.89f;chCos=sq(1-chSin*chSin);ch.h=66;}
  for(int yy=15;yy<H;yy+=7){
   float ray=(CH_MY-yy)*(1.f/CH_F),denom=chSin-ray*chCos;if(denom<.049f)continue;
   float f=ch_floor_run(ray,denom);
   float back=CH_MY-CH_F*(f*chSin-ch.h*chCos)/(f*chCos+ch.h*chSin);
   float wrong=ch.h*(chCos-ray*chSin)/denom;
   float old=CH_MY-CH_F*(wrong*chSin-ch.h*chCos)/(wrong*chCos+ch.h*chSin);
   if(absf(back-yy)>worst)worst=absf(back-yy);
   if(absf(old-yy)>oldWorst)oldWorst=absf(old-yy);n++;
  }
 }
 assert(n>=174&&worst<.0002f&&oldWorst>18);
 printf("Floor-to-geometry reprojection: %d row/pitch probes, max %.5f pixels (old formula %.1f).\n",n,worst,oldWorst);
}
static void boom(void){
 fresh();nc=1;sv.owned=1;nv.garage[60]=1;float maxRise=0;int found=0;
 for(int i=0;i<nb&&!found;i++){
  StreetBounds b=building_bounds(buildings+i);
  float x=b.x+b.w+22,y=b.y+b.h*.5f;if(solid(x,y,12)||!solid(x-45,y,4))continue;
  cars[0]=(Car){x,y,0,0,0,1,100};p.x=x;p.y=y;p.car=0;ch_reset_spring();
  ngCamRide=0;ngCamAngle=0;ch.ride=0;ch.range=93;ch.height=40;ch_fixed_step();float start=chSpring.boom;
  if(start>36)continue;
  /* Sweep a view past the edge of this real wall, maintaining real caster
     bounds. It must contract as needed and recover on ticks, not renders. */
  int clear=0;float oldjump=0,prev=ch_clearance(ch_mount(),1,0);
  for(int j=0;j<210;j++){
   ngCamAngle=wrap((j>80?80:j)*.037f);ChMount m=ch_mount();
   float safe=ch_clearance(m,cs(m.a),sn(m.a));if(safe-prev>oldjump)oldjump=safe-prev;prev=safe;
   float before=chSpring.boom;ch_fixed_step();float step=chSpring.boom-before;
   if(step>maxRise)maxRise=step;assert(step<=1.501f&&chSpring.boom<=safe+.05f);
   ch_frame_rig();float cached=chFit;for(int repeat=0;repeat<6;repeat++){ch_frame_rig();assert(chFit==cached);}
   if(chFit>start+25)clear=1;
  }
  if(clear&&oldjump>10){found=1;printf("Wall recovery: unsprung release %.1f units in one tick; new maximum %.2f. Six repeat paints/step never advanced the boom.\n",oldjump,maxRise);}
 }
 assert(found);
}
int main(void){panels();inverse();boom();return 0;}
