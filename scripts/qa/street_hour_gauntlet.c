/* Change the seed on every invocation. Synthetic NPCs/starting positions
   only; from the initial state forward all gameplay changes use tick(). */
#define DC_TEST
#include <game.c>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
static U stream;
static U draw32(void){stream^=stream<<13;stream^=stream>>17;stream^=stream<<5;return stream;}
static int frames,renders,seen[6],encounters;
static void step(U k){
 tick(k);frames++;
 if(frames%239==0){paint();renders++;}
 assert(isfinite(p.x)&&isfinite(p.y)&&isfinite(p.heat)&&p.hp>=0&&p.hp<=100);
 assert(sv.dirty>=0&&sv.medPatches>=0&&sv.medPatches<=99&&p.heat<=300.01f);
 assert(npcEnc.phase>=NPC_IDLE&&npcEnc.phase<=NPC_RECOVER);seen[npcEnc.phase]++;
 if(npcTalkOn)assert(npcTalk.id>=0&&npcTalk.id<N_ACT_MAX);
 if(npcEnc.phase!=NPC_IDLE)assert(npcEnc.prize>=0&&npcEnc.prize<=155&&isfinite(npcEnc.actor.x));
}
static void tap(U k){step(k);step(0);}
static void setup(int round){
 init_city();starter_save();sv.cash=70;sv.dirty=0;sv.intro=4;sv.gear=round%2?0:1;sv.medPatches=5;
 clear_input();npc_reset();vd_reset();bank_ui_reset();end_job();mapOn=0;scene=1;menu=-1;walking=0;
 float x=256*(2+draw32()%5),y=256*(2+draw32()%5),a=draw32()%8*.7853982f,range=13+draw32()%20;
 p=(Player){.x=x,.y=y,.a=a,.hp=100,.car=-1};pulse_guard_reset();wheelFists=0;
 /* Isolate the target; leave the genuinely finite patrols in their own station slots. */
 for(int i=0;i<DISPATCH_FIRST;i++)peds[i]=(Ped){.life=3000.f};
 int id=draw32()%STREET_CIVILIANS;
 peds[id]=(Ped){.x=x+range*cs(a),.y=y+range*sn(a),.hp=3,.a=a+1.5f};
 streetWalks[id]=(StreetWalk){.mx=peds[id].x,.my=peds[id].y,.pause=25};
 for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};nc=2;
 set_session_seed(draw32());
}
int main(int argc,char**argv){
 U seed=(argc==2?strtoul(argv[1],0,10):1)+0x7add631;stream=seed;int hpLost=0,loot=0;
 U directions[]={UP,DOWN,LEFT,RIGHT,UP|LEFT,UP|RIGHT,DOWN|LEFT,DOWN|RIGHT};
 for(int round=0;round<12;round++){
  setup(round);tap(USE|BRAKE);assert(npcTalkOn);
  if(round%6==5){tap(UP);tap(ENTER);assert(!npcTalkOn);tap(USE|BRAKE);assert(npcTalkOn);}
  tap(DOWN);tap(ENTER);assert(npcEnc.phase==NPC_DRAW);encounters++;
  for(int j=0;j<390;j++){
   U k=0;U v=draw32();
   if(npcEnc.phase==NPC_FIGHT){
    int mode=round%6;
    if(mode==0)k=FIRE;
    else if(mode==1)k=LEFT|BRAKE;
    else if(mode==2)k=PUNCH;
    else if(mode==3){
     if(j%37<=3){pointer_input(480,70,0);k=WHEEL|(j%37==2?USE_PATCH:0);}else k=FIRE;
    }else if(mode==4)k=FIRE|(j%31<3?directions[v%8]:0);
    else if(j%55==0){U modal=(v&1)?HELP:PHONE;int prize=npcEnc.prize;tap(modal);assert(scene!=1);tap(ESC);assert(scene==1&&npcEnc.phase==NPC_FIGHT&&npcEnc.prize==prize);}
    else if(j>75)k=FIRE;
   }else if(npcEnc.phase==NPC_DROP){
    float dx=npcEnc.actor.x-p.x,dy=npcEnc.actor.y-p.y;
    if(absf(dx)>2)k|=dx>0?RIGHT:LEFT;if(absf(dy)>2)k|=dy>0?DOWN:UP;
   }else if(npcEnc.phase==NPC_IDLE&&j>100){
    k=directions[(j/25+round)%8]|(v%3?BRAKE:0);
    if(v%23==0)k|=PUNCH;
   }
   step(k);if(p.dead)break;
  }
  hpLost+=100-p.hp;loot+=sv.dirty;
 }
 assert(encounters==12&&seen[NPC_DRAW]>0);
 printf("seed=%u encounters=%d ticks=%d renders=%d health-lost=%d street-cash=%d seen=%d/%d/%d/%d/%d/%d\n",seed,encounters,frames,renders,hpLost,loot,seen[0],seen[1],seen[2],seen[3],seen[4],seen[5]);
}
