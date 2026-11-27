/* Stateful black-box-at-the-engine-boundary fuzz of ordinary venue controls.
 * Seeds alter doors, menu choices, movement, hold duration, traffic clock and
 * camera-quality toggles. Initial careers/door coordinates are staged; every
 * subsequent movement, shop decision, menu visit and rendered frame is real.
 * cc -O1 -std=c11 -Isrc scripts/qa/venue_controls_soak.c -o venue_controls
 */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static U sseed;static int ticks,paints,steps,visits,scenes[12],pauses;
static U next_rand(void){sseed=sseed*1664525u+1013904223u;return sseed;}
static int span(int n){return (next_rand()>>8)%(U)n;}
static void step(U key){
 tick(key);ticks++;if(ticks%83==0){paint();paints++;}
 assert(scene>=0&&scene<12);assert(p.hp>=0&&p.hp<=100);assert(sv.cash>=0&&sv.dirty>=0);
 scenes[scene]++;
 if(scene==NIGHT_CLUB_SCENE){assert(!nc_solid(ncX,ncY));if(ncPanel!=NC_NONE&&ncPanel!=NC_DANCE_PLAY)assert(ncChoice>=0&&ncChoice<nc_items());}
 if(scene==NIGHT_TRAP_SCENE){assert(nt_x>112&&nt_x<847&&nt_y>227&&nt_y<450);assert(nt_stock_total()<=nt_capacity());}
 if(scene==NIGHT_VENUES_SCENE&&nvs_room==NVR_GLASS){assert(nvs_tab>=0&&nvs_tab<4&&nvs_stage()<=5);}
 if(scene==3){assert(!int_solid(interior_x,interior_y));}
 if(scene==4){assert(!wi_solid(wi_x,wi_y));}
}
static void press(U k){step(0);step(k);step(0);}
static void start(int kind){
 /* Visits are independent player arrivals at a doorstep, not teleports in
  * the reported input routes. Preserve inventory/city clock across visits. */
 clear_input();npc_reset();scene=1;mapOn=0;menu=-1;p.car=-1;p.dead=0;p.hp=100;
 p.heat=0;job.kind=-1;vdActive=-1;vdPanel=0;atmOpen=-1;wheelOpen=0;
 if(kind==0){int b=night_club_listing();p.x=ordinaryDoorX[b];p.y=ordinaryDoorY[b];press(USE);assert(scene==NIGHT_CLUB_SCENE);}
 else if(kind==1){p.x=NT_DOOR_X;p.y=NT_DOOR_Y;press(ENTER);assert(scene==NIGHT_TRAP_SCENE);}
 else if(kind==2){int b=nvs_aquaria[span(4)];walkin_open(b);press(ENTER);assert(scene==NIGHT_VENUES_SCENE);}
 else if(kind==3){open_interior(12);press(ENTER);assert(scene==NIGHT_VENUES_SCENE);}
 else if(kind==4){open_interior(1);assert(scene==3);}
 else{int b;do{b=span(nb);}while(!ordinaryDoorY[b]||b==NC_ID||b==ncFrontId);walkin_open(b);}
}
int main(int argc,char**argv){
 sseed=argc>1?(U)strtoul(argv[1],0,0):201u;set_session_seed(sseed?sseed:1);
 init_city();profiles_activate();sv.intro=4;sv.cash=2200;sv.gear=1;sv.home=1;sv.dayClock=span(1440);
 static const U dirs[]={UP,DOWN,LEFT,RIGHT,UP|LEFT,DOWN|LEFT,UP|RIGHT,DOWN|RIGHT};
 static const U taps[]={USE,ENTER,FIRE,LEFT,RIGHT,UP,DOWN,ESC,PHONE,GFX,HELP,SAVE,MAP,TAXI_NEXT,BRAKE|USE};
 for(int v=0;v<24;v++){
  start(v%6);visits++;
  for(int a=0;a<92;a++){
   if(scene==1||scene<1||scene==5)break;
   if(scene==NIGHT_PHONE_SCENE){press(span(5)?ESC:RIGHT);continue;}
   if(scene==2){press(ESC);continue;}
   if(menu==PAUSE_MENU){pauses++;press(ESC);continue;}
   if(span(3)){U dir=dirs[span(8)]|(span(2)?BRAKE:0);int dur=1+span(38);for(int j=0;j<dur;j++){step(dir);steps++;}}
   else press(taps[span(LEN(taps))]);
  }
 }
 printf("Venue controls: %d staged visits, %d key ticks, %d movement-input ticks, %d frames painted, %d pause exits, scenes %d/%d/%d/%d/%d/%d.\n",visits,ticks,steps,paints,pauses,scenes[3],scenes[4],scenes[NIGHT_CLUB_SCENE],scenes[NIGHT_TRAP_SCENE],scenes[NIGHT_VENUES_SCENE],scenes[NIGHT_PHONE_SCENE]);
}
