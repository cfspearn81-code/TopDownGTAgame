/* Prove authored floor cues, upright actors/text and true wheel-contact
 * strips appear in C mode without advancing jobs or ground on repaint. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static U baseline[W*H];
static void jobs_fixture(float x,float y){
 init_city();scene=1;menu=-1;mapOn=0;end_job();vd_reset();z(streetDrivers,sizeof streetDrivers);
 sv.intro=4;sv.dayClock=600;sv.owned=8;sv.selection=3;sv.carCond[3]=100;sv.hp=100;
 for(int j=0;j<LEN(peds);j++)peds[j]=(Ped){.life=900.f};p=(Player){.x=x,.y=y,.car=-1,.hp=100};
 clear_input();fastGfx=0;tick(CAMERA);tick(0);assert(ch_pref());
}
static int colors(void){int count=0;for(int k=0;k<W*H;k++)if(chJobMap[k])count++;return count;}
static void drawn_jobs(void){
 jobs_fixture(SW_PICK_X,SW_Y+30);assert(vd_launch(VA_B5));paint();
 assert(chDecals&&chLabelN==1&&strstr(chLabel[0].words,"CASSETTE")&&colors()>80);
 int cash=sv.cash,t=swTape.clock,num=colors();memcpy(baseline,pixels,sizeof baseline);paint();
 assert(!memcmp(baseline,pixels,sizeof baseline)&&sv.cash==cash&&swTape.clock==t&&num==colors());
 vd_cancel();p.x=CR_X+28;p.y=CR_Y+50;assert(vd_launch(VA_B6A));cr.locked=1;cr.mode=CR_KEEPER;cr.workerY=CR_Y;paint();
 assert(chLabelN>=2&&chActorN>=1&&colors()>100);
 int actors=chActorN,labels=chLabelN;vd_cancel();assert(vd_launch(VA_A2));
 p.x=VD_FILM_X+40;p.y=VD_FILM_Y+95;dvPhase=1;paint();
 assert(chLabelN>=2&&colors()>70);vd_cancel();paint();assert(!chDecals&&chLabelN==0&&!chActorN);
 printf("Switchboard cabinet, staffed signal crossing (%d actors/%d labels), dual camera bay footprints and world text survive C. Repaint unchanged.\n",actors,labels);
}
static void tire_marks(void){
 jobs_fixture(902,1024);nc=1;cars[0]=(Car){902,1024,0,0,3,1,100};p.car=0;
 for(int i=0;i<44;i++)tick(UP);for(int i=0;i<7;i++)tick(UP|BRAKE|RIGHT);
 assert(roadCount>2);int count=roadCount;paint();memcpy(baseline,pixels,sizeof baseline);
 roadCount=0;paint();int marked=0;for(int k=0;k<W*H;k++)if(baseline[k]!=pixels[k])marked++;
 assert(marked>3);roadCount=count;
 for(int t=0;t<ROAD_MARK_LIFE;t++)roadFX_tick();assert(roadCount==0);
 printf("Held throttle/handbrake produced %d live paired skid segments and %d visible changed pixels. Queue expired at 300 ticks.\n",count,marked);
}
int main(void){drawn_jobs();tire_marks();return 0;}
