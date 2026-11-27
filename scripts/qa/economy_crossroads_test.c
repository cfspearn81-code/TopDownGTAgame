/* Public single-player state, throwaway values to exercise hard-to-reach edges.
   Run in normal engine input/update, no real player save or operating system. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void reset(void){starter_save();spawnPlayer();scene=1;p.dead=0;p.car=-1;menu=-1;job.kind=-1;vdActive=-1;}
int main(int argc,char**argv){
 int baseline=argc>1&&!strcmp(argv[1],"--baseline");init_city();reset();
 sv.cash=2100;sv.home=1;sv.gear=0;revenue_open_case();nt_open(NT_VISIT);nt_panel_use();nt_panel_use();
 int deed=night_trap_owned();nvs_lease_tick(USE);int crown=nvs_penthouse();nvs_tab=1;nvs_sel=2;nvs_cart_tick(USE);int frame=nvk_owned(3)!=0;
 printf("Open case: $480 Bell Lane deed %d, $640 penthouse %d, $310 Halo purchase %d, clean cash %d\n",deed,crown,frame,sv.cash);
 if(baseline)assert(deed&&crown&&frame);else{assert(!deed&&!crown&&!frame&&sv.cash==2100);nvs_sel=0;nvs_cart_tick(USE);assert(nvk_owned(1)&&sv.cash==1925);
 revenue_close_case();nt_open(NT_VISIT);nt_panel_use();nt_panel_use();assert(night_trap_owned());nvs_lease_tick(USE);assert(nvs_penthouse());}
 reset();sv.cash=WALLET_LIMIT-3;sv.legit[0]=(LegalBusiness){.owned=1,.pending=90};active(LEGAL_REAR_FIRST);doOpt(LEGAL_CLAIM_FIRST);
 printf("Settled franchise $90, clean wallet had $3 room: remaining %d\n",sv.legit[0].pending);
 if(baseline)assert(!sv.legit[0].pending);else{assert(sv.legit[0].pending==87&&sv.cash==WALLET_LIMIT);doOpt(LEGAL_CLAIM_FIRST);assert(sv.legit[0].pending==87);
 sv.cash-=100;doOpt(LEGAL_CLAIM_FIRST);assert(!sv.legit[0].pending&&sv.cash==WALLET_LIMIT-13);doOpt(LEGAL_CLAIM_FIRST);assert(sv.cash==WALLET_LIMIT-13);}
 reset();sv.dayClock=1380.f;sv.playedTicks=420;nv.phone[NP_POPSTAMP]=nv.garage[36]=np_stamp();night_phone_heat_add((int)p.x,(int)p.y,90);
 nvs_tab=2;nvs_sel=0;nvs_home_tick(USE);int h=night_phone_heat_at((int)p.x,(int)p.y);
 printf("2h sleep over midnight: clock %.0f, previous daily gift/show %u/%u, district meter %d (from 90)\n",sv.dayClock,nv.phone[NP_POPSTAMP],nv.garage[36],h);
 if(baseline)assert(sv.dayClock==60.f&&nv.phone[NP_POPSTAMP]&&nv.garage[36]&&h==78);
 else{assert(sv.dayClock==60.f&&!nv.phone[NP_POPSTAMP]&&!nv.garage[36]&&h==84);
   sv.dayClock=430;sv.playedTicks=DAY_SECONDS*60u-1;nv.phone[NP_POPSTAMP]=nv.garage[36]=np_stamp();
   tick(0);assert(nv.phone[NP_POPSTAMP]==np_stamp()&&nv.garage[36]==np_stamp());}
 reset();sv.b[0]=(Business){.own=1,.product=0,.tier=1,.timer=19.f,.durationMs=120000};
 open_interior(4);int_switch_room(4,1);active(4);for(int i=0;i<1200;i++)tick(0);
 printf("Open Lantern counter 1,200 updates: batch remaining %.1f, inventory %d\n",sv.b[0].timer,sv.stocks[0]);
 assert(sv.b[0].timer==0&&sv.stocks[0]>0);
}
