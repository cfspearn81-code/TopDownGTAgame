/* Arcade close-range interaction and gear-wheel integration. No disk access. */
#define DC_TEST
#include <game.c>
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void hit(U key){tick(key);tick(0);}
static void init_range(int dx,int dy){
 scene=1;menu=-1;mapOn=0;p=(Player){.x=1024,.y=1024,.hp=100,.car=-1};
 sv.intro=4;sv.gear=0;sv.cash=70;sv.dirty=0;clear_input();
 npc_reset();pulse_guard_reset();vd_reset();bank_ui_reset();end_job();wheelFists=1;punchFlash=0;
 for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};nc=2;
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
 peds[0]=(Ped){.x=p.x+dx,.y=p.y+dy,.hp=3};
 streetWalks[0]=(StreetWalk){.mx=peds[0].x,.my=peds[0].y,.pause=30};
}
static void request(void){hit(USE);assert(npcTalkOn);hit(DOWN);set_session_seed(2);hit(ENTER);}
static unsigned region(int x,int y,int w,int h){
 U v=2166136261u;for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)v=(v^pixels[yy*W+xx])*16777619u;return v;
}
int main(void){
 init_city();
 init_range(12,0);request();
 printf("Close-range selected target: phase=%d (draw=%d), notice=%s\n",npcEnc.phase,NPC_DRAW,notice);fflush(stdout);
 assert(npcEnc.phase==NPC_DRAW&&npcEnc.actor.id==0&&p.trail<=0&&p.heat==0);
 for(int j=0;j<31;j++)tick(0);assert(npcEnc.phase==NPC_FIGHT);
 tick(FIRE);assert(npcEnc.hp<3&&p.trail>0);
 for(int face=0;face<8;face++){
  float a=face*.785398f;init_range((int)(12*cs(a)),(int)(12*sn(a)));
  request();assert(npcEnc.phase==NPC_DRAW&&npcEnc.actor.id==0);
 }
 init_range(1,0);request();assert(npcEnc.phase==NPC_IDLE&&!npcSpent[0]&&sv.dirty==0);
 init_range(20,0);NpcRef r;assert(npc_ref(0,&r));cars[1]=(Car){1034,1024,0,0,1,0,100};assert(!npc_begin(r)&&!npcSpent[0]);
 init_range(40,0);wheelFists=0;nvk_cycle_frame(0);pointer_input(WW_CX,WW_CY-WW_DIST,0);
 tick(WHEEL);paint();unsigned old=region(WW_CX-91,WW_CY-34,182,67);tick(0);
 unsigned variants[3];
 for(int frame=1;frame<=3;frame++){
  nvk_cycle_frame(frame);nvk_set(NVK_LOAD_BIT,15,3);nvk_put(0,17);
  NightSave snap=nv;tick(WHEEL);paint();
  unsigned current=region(WW_CX-91,WW_CY-34,182,67);variants[frame-1]=current;
  assert(old!=current&&!memcmp(&snap,&nv,sizeof nv)&&wheelOpen);
  tick(0);nvk_set(NVK_LOAD_BIT,15,2);tick(WHEEL);paint();
  assert(current!=region(WW_CX-91,WW_CY-34,182,67));tick(0);
 }
 assert(variants[0]!=variants[1]&&variants[0]!=variants[2]&&variants[1]!=variants[2]);
 puts("PASS: eight close headings, covered/overlap guard, shared draw and shot; all three fitted wheel labels, live charge count, no resource mutation.");
}
