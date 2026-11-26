#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
static void wait_ticks(int n){for(int i=0;i<n;i++)tick(0);}
static void tap(U k){tick(k);tick(0);}
static void setup(void){
 starter_save();scene=1;menu=-1;mapOn=0;p=(Player){.x=1024,.y=1024,.hp=100,.car=-1};
 sv.intro=4;sv.gear=0;clear_input();npc_reset();pulse_guard_reset();bank_ui_reset();vd_reset();
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
 for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};
 nc=2;job.kind=-1;taxi=(Taxi){0};walking=0;punchFlash=0;
}
static NpcRef ref(int id){
 NpcRef r;assert(npc_ref(id,&r));return r;
}
static void still(NpcRef started){
 float x=-1,y=-1,a=-1;int step=-1;
 assert(npc_holds(started.id)&&npc_present_pose(started.id,&x,&y,&a,&step)==1);
 assert(x==started.x&&y==started.y&&step==0);
 NpcRef current=ref(started.id);
 assert(current.x==x&&current.y==y&&current.a==a);
}
int main(void){
 init_city();setup();
 peds[0]=(Ped){.x=1054,.y=1024,.a=0,.s=28,.hp=3};
 peds[1]=(Ped){.x=1050,.y=1054,.a=0,.s=28,.hp=3};
 streetWalks[0]=(StreetWalk){.mx=1120,.my=1024,.leg=STREET_LEAVE};
 streetWalks[1]=(StreetWalk){.mx=1024,.my=1054,.leg=STREET_LEAVE};
 NpcRef original=ref(0);float other=peds[1].x;U clock=sv.playedTicks;
 tap(USE);assert(npcTalkOn&&npcTalk.id==0);still(original);
 wait_ticks(75);still(original);
 assert(peds[0].x==original.x&&peds[0].y==original.y&&peds[0].a==original.a);
 assert(absf(peds[1].x-other)>10&&sv.playedTicks>clock+74);
 tap(ESC);assert(!npcTalkOn&&peds[0].x>original.x&&!npc_holds(0));
 tap(USE);NpcRef quiet=ref(0);wait_ticks(10);still(quiet);
 tap(ENTER);assert(!npcTalkOn&&peds[0].x>quiet.x&&!npcLetters[0]);
 tap(USE);NpcRef hello=ref(0);wait_ticks(10);still(hello);
 tap(UP);tap(ENTER);assert(!npcTalkOn&&peds[0].x>hello.x&&npcLetters[0]==1);
 /* Exact still stance is handed directly into the normal draw/response. */
 tap(USE);NpcRef draw=ref(0);
 tap(DOWN);wait_ticks(12);still(draw);
 set_session_seed(2);tap(ENTER);
 assert(!npcTalkOn&&npcEnc.phase==NPC_DRAW&&npcEnc.actor.id==0);
 assert(npcEnc.actor.x==draw.x&&npcEnc.actor.y==draw.y&&p.trail<=0);
 wait_ticks(25);assert(npcEnc.phase==NPC_FIGHT&&!npc_holds(0));

 setup();
 taxi=(Taxi){.rider=1,.phase=TAXI_WALKING,.linger=2.5f,.dropoff=0,.x=1052,.y=1024};
 tap(USE);assert(npcTalkOn&&npcTalk.id==N_ACT_RIDER+1);
 float until=taxi.linger,tx=taxi.x,ty=taxi.y;
 wait_ticks(62);
 assert(taxi.linger==until&&taxi.x==tx&&taxi.y==ty);
 tap(ESC);assert(!npcTalkOn&&taxi.linger<until&&d2(taxi.x-tx,taxi.y-ty)>0);

 setup();open_interior(1);
 interior_x=V39_PLAYER_X;interior_y=V39_PLAYER_Y;v39.tick=19;v39.patron=0;
 tap(USE);assert(v39.requested);
 tap(BRAKE|USE);assert(npcTalkOn&&npcTalk.id==N_ACT_MIKA);
 original=ref(N_ACT_MIKA);NpcRef dancer=ref(N_ACT_DANCE);
 wait_ticks(73);still(original);
 NpcRef raw;assert(npc_raw_ref(N_ACT_MIKA,&raw)&&absf(original.x-raw.x)>14);
 NpcRef moving=ref(N_ACT_DANCE);assert(d2(moving.x-dancer.x,moving.y-dancer.y)>5);
 assert(v39.tick>=93&&v39.serves==0);
 tick(ESC);assert(!npcTalkOn&&!npc_holds(N_ACT_MIKA));
 NpcRef resume=ref(N_ACT_MIKA);
 assert(absf(resume.x-original.x)<=1.751f&&absf(resume.x-original.x)>.5f);
 for(int i=0;i<165;i++){
  NpcRef before=ref(N_ACT_MIKA);tick(0);NpcRef after=ref(N_ACT_MIKA);
  if(npc_rejoin_for(N_ACT_MIKA)->room)assert(d2(after.x-before.x,after.y-before.y)<=1.76f*1.76f);
 }
 assert(!npc_rejoin_for(N_ACT_MIKA)->room&&v39.serves>=1);
 /* Patio patrons/dancers don't share the bartender's hold state. */
 NpcRef patron=ref(N_ACT_PATRON+1);
 assert(npc_start_talk(patron));interior_x=patron.x-18;interior_y=patron.y+2;
 wait_ticks(50);still(patron);tap(ESC);assert(!npcTalkOn);

 setup();open_interior(4);sv.b[0].own=1;sv.b[0].crew=1;
 int_switch_room(4,1);start_batch(sv.b,120.f);
 NpcRef grow=ref(N_ACT_GROW);interior_x=grow.x+12;interior_y=grow.y;
 assert(npc_start_talk(grow));float timer=sv.b[0].timer;
 wait_ticks(76);still(grow);assert(sv.b[0].timer<timer);
 tap(ESC);wait_ticks(295);assert(!npc_rejoin_for(N_ACT_GROW)->room&&!npcTalkOn);

 setup();vdActive=VA_B6A;cr.mode=CR_KEEPER;cr.locked=1;cr.worker=1;cr.workerY=CR_Y-30;
 p.x=CR_X+45;p.y=cr.workerY;
 NpcRef worker=ref(N_ACT_CROSS);assert(npc_start_talk(worker));
 int crtime=cr.ticks;for(int i=0;i<45;i++)cr_move();
 assert(cr.workerY==worker.y&&cr.ticks==crtime+45&&npc_holds(N_ACT_CROSS));
 npc_end_talk(1);cr_move();assert(!npc_holds(N_ACT_CROSS)&&cr.workerY>worker.y);
 puts("Conversation hold: selected street/rider/Vault/grow/crossing NPCs still, unrelated traffic/people/timers live; all exits and draw/fight resume.");
}
