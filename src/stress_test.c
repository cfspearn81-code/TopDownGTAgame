/* Deterministic sanitized soak, complementary to the keyboard playtests. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdint.h>
#include <math.h>
static U state=0x8eaf35a1u;
static U draw(void){state^=state<<13;state^=state>>17;state^=state<<5;return state;}
static void assert_state(void){
 assert(scene>=0&&scene<=5&&nc>=1&&nc<=LEN(cars)&&nb<=LEN(buildings));assert(p.hp>=0&&p.hp<=100);
 assert(p.car>=-1&&p.car<nc&&p.x>=35&&p.x<=ROOM-36&&p.y>=35&&p.y<=ROOM-36);
 assert(isfinite(p.heat)&&p.heat>=0&&p.heat<=300);
 assert(sv.cash>=0&&sv.dirty>=0&&sv.units>=0&&sv.armor>=0&&sv.armor<=90);
 if(job.kind>=0){assert(job.kind<LEN(jobNames)&&job.total>0&&job.total<=6&&job.step>=0&&job.step<job.total&&isfinite(job.time));}
 for(int i=0;i<2;i++)assert(isfinite(sv.b[i].timer)&&sv.b[i].timer>=0&&sv.b[i].timer<=120);
 for(int i=0;i<nc;i++)assert(isfinite(cars[i].x)&&isfinite(cars[i].y)&&isfinite(cars[i].s)&&cars[i].cond>=0&&cars[i].cond<=100);
}
static void seed_round(int round){
 init_city();scene=1;sv.intro=4;sv.cash=35000;sv.dirty=5000;sv.gear=round%3-1;sv.units=60;
 for(int j=0;j<3;j++)sv.legit[j]=(LegalBusiness){.owned=1,.elapsed=(float)(j*31)};
 for(int j=0;j<2;j++){sv.b[j]=(Business){1,round%3,round%3+1,j?2:round%2,0,0,0};start_batch(sv.b+j,120);}
 sv.taxiFares=1;sv.owned=1;sv.vehicleCond=sv.carCond[0]=100;
 p.x=256+(round%7)*256;p.y=256+((round*3)%7)*256;
 if(round%4==0){p.car=0;cars[0]=(Car){p.x,p.y,0,0,0,1,100};}
 else if(round%4==1){open_interior(round%LEN(loc));}
 else if(round%4==2){for(int i=0;i<nb;i++)if(ordinaryDoorY[i]){walkin_open(i);break;}}
}
int main(void){
 static const U actions[]={0,UP,LEFT,DOWN,RIGHT,UP|BRAKE,LEFT|BRAKE,DOWN|RIGHT,UP|LEFT,UP|FIRE,UP|RIGHT|FIRE,FIRE,PUNCH,USE,ENTER,SAVE,HELP,ESC,MAP,GFX,TAXI_NEXT,SLOT,NEW_SLOT,USE|ENTER,LEFT|RIGHT|BRAKE};
 unsigned frames=0,renders=0;
 for(int seed=0;seed<18;seed++){
   seed_round(seed);U hold=0;int dur=0;
   for(int f=0;f<10000;f++){
    if(--dur<=0){
      unsigned r=draw();hold=actions[r%LEN(actions)];
      dur=(r>>8)%48+1;if(hold&(USE|ENTER|SAVE|HELP|ESC|MAP|GFX|TAXI_NEXT|SLOT|NEW_SLOT))dur=1;
    }
    if(p.dead){hold=0;if(f%60==0){hold=ENTER;dur=1;}}
    if(mapOn&&f%19==0)pointer_input((int)(draw()%1200)-120,(int)(draw()%650)-60,(draw()%5==0)?2:1);
    tick(hold);
    if(f%83==0){paint();renders++;}
    assert_state();frames++;
   }
 }
 printf("Sanitized soak: %u fixed-step gameplay ticks, %u render passes, 18 starting contexts/seeds.\n",frames,renders);
 return 0;
}
