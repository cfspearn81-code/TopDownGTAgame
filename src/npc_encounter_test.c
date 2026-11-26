#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void step(U k){tick(k);}
static void tap(U k){tick(k);tick(0);}
static void reset(void){
 starter_save();scene=1;menu=-1;mapOn=0;p=(Player){.x=1024,.y=1024,.hp=100,.car=-1};
 sv.intro=4;sv.gear=0;sv.cash=70;sv.dirty=0;bank_ui_reset();vd_reset();clear_input();npc_reset();
 pulse_guard_reset();job.kind=-1;taxi=(Taxi){0};walking=0;punchFlash=0;
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
 for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};nc=2;
}
static void hook(int i){peds[i]=(Ped){.x=1054,.y=1024,.hp=3,.role=i>=DISPATCH_FIRST?1:i>=STREET_CIVILIANS?2:0};}
static void near_job_sector(void){by=-1;near=-1;}
static void select_rob(void){tap(USE);assert(npcTalkOn);tap(DOWN);step(ENTER);}
static void choose_seed(int want){for(U i=0;;i++){set_session_seed(i);npc_roll_bound(141);if((npc_roll_bound(100)<35)==want){set_session_seed(i);return;}}}
static void prove_statistics(void){
 unsigned freq[141]={0},yielded=0;
 set_session_seed(21183);
 for(int i=0;i<40000;i++){
  U amount=npc_roll_bound(141),roll=npc_roll_bound(100);
  assert(amount<141&&roll<100);freq[amount]++;yielded+=roll<35;
 }
 assert(yielded>=13600&&yielded<=14400);
 for(int i=0;i<141;i++)assert(freq[i]>211&&freq[i]<357);
 printf("40,000 attempts: %u surrendered (%.3f%%), 141 occupied payout values, endpoints $15 and $155.\n",yielded,yielded/400.0);
}
int main(void){
 init_city();reset();hook(0);near_job_sector();
 NpcRef r;assert(npc_ref(0,&r)&&npc_near_ref(&r,0)&&r.id==0);
 /* Quiet and greeting menu stay live; no repeat on held E or Enter. */
 tap(USE);assert(npcTalkOn&&npcChoice==1);
 U tp=sv.playedTicks;for(int i=0;i<20;i++)step(0);assert(sv.playedTicks>tp&&npcTalkOn);
 tap(ENTER);assert(!npcTalkOn&&!npcLetters[0]&&!sv.dirty);
 char spoken[50][112];
 for(int i=0;i<50;i++){
  peds[0].x=1054;peds[0].y=1024;tap(USE);assert(npcTalkOn);tap(UP);step(ENTER);
  assert(!npcTalkOn);strcpy(spoken[i],notice);
  for(int j=0;j<i;j++)assert(strcmp(spoken[j],spoken[i]));
  for(int t=0;t<2;t++)step(ENTER);
  step(0);
 }
 assert(!npcLetters[0]&&!sv.dirty);
 reset();hook(0);sv.gear=-1;select_rob();assert(npcEnc.phase==NPC_IDLE&&!sv.dirty&&!p.heat);
 reset();hook(0);choose_seed(1);wheelFists=1;
 select_rob();assert(npcEnc.phase==NPC_DRAW&&pistol_equipped()&&p.trail<=0&&p.heat==0);
 int prize=npcEnc.prize;assert(prize>=15&&prize<=155);
 for(int i=0;i<25;i++)step(ENTER|FIRE);
 assert(npcEnc.phase==NPC_YIELD&&sv.dirty==prize&&sv.cash==70&&p.trail<=0);
 for(int i=0;i<50;i++)step(ENTER);step(0);
 assert(npcEnc.phase==NPC_IDLE&&sv.dirty==prize&&npcSpent[0]);
 peds[0].x=1054;peds[0].y=1024;select_rob();assert(npcEnc.phase==NPC_IDLE&&sv.dirty==prize);
 reset();hook(0);choose_seed(0);select_rob();prize=npcEnc.prize;
 for(int i=0;i<25;i++)step(0);
 assert(npcEnc.phase==NPC_FIGHT&&!sv.dirty);
 for(int i=0;i<48;i++)step(0);
 assert(p.hp<100&&npcEnc.strike!=0); /* They attacked before the player shot. */
 for(int i=0;i<100&&npcEnc.phase==NPC_FIGHT;i++)step(FIRE);
 assert(npcEnc.phase==NPC_DROP&&!sv.dirty&&npcEnc.prize==prize);
 for(int i=0;i<22;i++)step(0);
 for(int i=0;i<5;i++)step(RIGHT);
 assert(sv.dirty==prize&&npcEnc.phase==NPC_RECOVER&&!npcEnc.prize);
 for(int i=0;i<100;i++)step(0);
 assert(sv.dirty==prize);
 reset();hook(0);choose_seed(0);select_rob();
 for(int i=0;i<25;i++)step(0);
 assert(npcEnc.phase==NPC_FIGHT);
 for(int i=0;i<125;i++)step(LEFT|BRAKE);
 assert(npcEnc.phase==NPC_IDLE&&!sv.dirty);
 /* Trading/bar routes retain E. Shift+E and the personal menu open the
    correct named voice even across the collision footprint of a desk. */
 reset();open_interior(1);interior_x=V39_PLAYER_X;interior_y=V39_PLAYER_Y;
 tap(USE);assert(v39.requested&&!npcTalkOn);
 tap(USE|BRAKE);assert(npcTalkOn&&npcTalk.id==N_ACT_MIKA);tap(ESC);assert(!npcTalkOn);
 int people=0;for(int i=0;i<N_ACT_MAX;i++)people+=npc_ref(i,&r);
 assert(people==7);
 open_interior(0);interior_x=interior_desk_x;interior_y=interior_desk_y;
 tap(USE);assert(menu==0&&npc_front_option());
 doOpt(N_ACT_OPEN);assert(npcTalkOn&&npcTalk.id==N_ACT_FRONT&&menu<0);
 tap(ESC);assert(!npcTalkOn);
 /* A real shop counter supplies its staged clear-side actor. Defeat,
    collect, and reopening the underlying counter cannot grant cash again. */
 choose_seed(0);tap(USE);assert(menu==0);
 doOpt(N_ACT_OPEN);tap(DOWN);step(ENTER);assert(npcEnc.phase==NPC_DRAW&&npcEnc.room>=32);
 int insidePrize=npcEnc.prize;
 for(int i=0;i<25;i++)step(0);
 assert(npcEnc.phase==NPC_FIGHT);
 for(int i=0;i<100&&npcEnc.phase==NPC_FIGHT;i++)step(FIRE);
 assert(npcEnc.phase==NPC_DROP&&sv.dirty==0&&npcEnc.prize==insidePrize);
 assert(npcTraceTicks>0);
 for(int i=0;i<20;i++)step(0);
 for(int i=0;i<5;i++){
  U k=absf(npcEnc.actor.x-interior_x)>absf(npcEnc.actor.y-interior_y)
      ?npcEnc.actor.x>interior_x?RIGHT:LEFT:npcEnc.actor.y>interior_y?DOWN:UP;
  step(k);
 }
 assert(sv.dirty==insidePrize&&npcEnc.phase==NPC_RECOVER);
 for(int i=0;i<100;i++)step(0);
 assert(npc_ref(N_ACT_FRONT,&r)&&sv.dirty==insidePrize);
 open_interior(14);assert(npc_ref(N_ACT_FRONT+14,&r)&&npc_ref(N_ACT_CHIEF,&r)&&npc_ref(N_ACT_HQ1,&r)&&npc_ref(N_ACT_HQ2,&r));
 open_interior(4);sv.b[0].own=1;sv.b[0].crew=1;int_switch_room(4,1);assert(npc_ref(N_ACT_GROW,&r));
 open_interior(1);int_switch_room(1,1);assert(npc_ref(N_ACT_REAR+1,&r));
 /* Vehicle and death cleanup never pay out a pending attempt. */
 reset();hook(0);choose_seed(0);select_rob();p.car=1;step(0);assert(npcEnc.phase==NPC_IDLE&&!sv.dirty);
 reset();hook(0);choose_seed(0);select_rob();spawnPlayer();assert(npcEnc.phase==NPC_IDLE&&!sv.dirty&&!npcSpent[0]);
 prove_statistics();
 puts("Hello/quiet/hold, owned pistol, surrender, attack/defeat/drop once, flee, bar and counter access, protected roles and reset passed.");
}
