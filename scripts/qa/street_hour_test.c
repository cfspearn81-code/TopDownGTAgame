/* Arcade, synthetic careers. Launch identical scripted input with -I pointing
   to the archived or repaired game. No saves or OS state are consulted. */
#define DC_TEST
#include <game.c>
#include <stdio.h>
#include <assert.h>
#include <string.h>
static void key(U k){tick(k);tick(0);}
static void frames(int n){while(n--)tick(0);}
static void duel(void){
 starter_save();clear_input();npc_reset();vd_reset();bank_ui_reset();end_job();scene=1;menu=-1;mapOn=0;
 sv.intro=4;sv.gear=0;sv.dirty=0;
 p=(Player){.x=1024,.y=1024,.hp=100,.car=-1};wheelFists=0;punchFlash=0;walking=0;
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=9999};
 for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};nc=2;
 taxi=(Taxi){0};peds[0]=(Ped){.x=1052,.y=1024,.hp=3};
 streetWalks[0]=(StreetWalk){.mx=1052,.my=1024,.pause=60};set_session_seed(2);
 key(USE);assert(npcTalkOn);key(DOWN);key(ENTER);assert(npcEnc.phase==NPC_DRAW);
 frames(30);assert(npcEnc.phase==NPC_FIGHT);
}
int main(int argc,char **argv){
 int mism=0;
 init_city();U modals[]={HELP,PHONE,SLOT};
 for(unsigned i=0;i<3;i++)for(int dropped=0;dropped<2;dropped++){
  duel();
  if(dropped){for(int j=0;j<130&&npcEnc.phase==NPC_FIGHT;j++)tick(FIRE);assert(npcEnc.phase==NPC_DROP);tick(0);}
  int prior=npcEnc.phase,prize=npcEnc.prize,elapsed=npcEnc.ticks;key(modals[i]);
  int overlay=scene;assert(overlay!=1);frames(12);key(ESC);
  printf("key=%u scene=%d state=%d->%d cash=%d ticks=%d->%d prize=%d\n",modals[i],overlay,prior,npcEnc.phase,sv.dirty,elapsed,npcEnc.ticks,prize);
  if(npcEnc.phase!=prior||npcEnc.prize!=prize||scene!=1||sv.dirty!=0)mism++;
  if(!mism&&dropped){
   p.x=npcEnc.actor.x;p.y=npcEnc.actor.y;frames(70);assert(sv.dirty==prize);frames(70);assert(sv.dirty==prize);
  }
 }
 printf("modal encounter transitions lost: %d/6\n",mism);return mism!=0;
}
