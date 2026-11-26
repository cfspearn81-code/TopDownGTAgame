/* Headless game-code economy checks with an in-memory disk. */
#include "game.c"
#include <assert.h>
#include <stdio.h>
static unsigned char stored[sizeof(Save)];
static unsigned stored_n;
int disk_write(const void*p,unsigned n){assert(n==sizeof(Save));cp(stored,p,n);stored_n=n;return 1;}
int disk_read(void*p,unsigned n){if(n>stored_n)return 0;cp(p,stored,n);return 1;}
static void select_action(int m,int a){active(m);makeMenu();doOpt(a);}
int main(void){
 init_city();scene=1;sv.intro=4;sv.cash=4000;sv.dirty=900;
 for(int i=0;i<3;i++){
  int before=sv.cash;select_action(16+i,LEGAL_BUY_FIRST+i);
  assert(sv.legit[i].owned&&sv.cash==before-legal_specs[i].cost&&sv.legit[i].pending==0);
 }
 int paid=sv.cash;production_tick(119);assert(sv.cash==paid);
 production_tick(1);assert(sv.cash==paid+24+36+49);
 sv.dayClock=390;
 select_action(16,LEGAL_DEPOSIT_FIRST);assert(sv.dirty==900&&!sv.legit[0].pending);
 select_action(19,LEGAL_DEPOSIT_FIRST);assert(sv.dirty==680&&sv.legit[0].pending==165&&sv.legit[0].cooldown==90);
 load_save();assert(wason&&sv.legit[0].pending==165&&sv.legit[0].cooldown==90);
 paid=sv.cash;select_action(19,LEGAL_CLAIM_FIRST);assert(sv.cash==paid);
 production_tick(89);assert(sv.legit[0].cooldown>0);
 production_tick(1);assert(sv.legit[0].cooldown==0&&sv.cash==paid);
 load_save();assert(wason&&sv.legit[0].pending==165&&sv.legit[0].cooldown==0);
 select_action(20,LEGAL_CLAIM_FIRST+1);assert(sv.cash==paid);
 select_action(19,LEGAL_CLAIM_FIRST);assert(sv.cash==paid+165&&!sv.legit[0].pending);
 load_save();assert(wason&&sv.cash==paid+165&&!sv.legit[0].pending);
 select_action(19,LEGAL_CLAIM_FIRST);assert(sv.cash==paid+165);
 sv.dayClock=470;select_action(20,LEGAL_DEPOSIT_FIRST+1);assert(sv.legit[1].cooldown==490);
 production_tick(12);assert(sv.legit[1].cooldown==478);
 sv.audit=1;sv.auditT=.02f;businessTick(.03f);
 assert(sv.charges&&sv.dirty==0&&sv.legit[1].pending==0);
 sv.charges=0;sv.cash=25000;
 select_action(4,40);select_action(5,41);assert(sv.cash==12000&&sv.b[0].own&&sv.b[1].own);
 select_action(4,48);select_action(4,48);assert(sv.b[0].tier==3&&sv.cash==10200);
 select_action(5,49);select_action(5,49);assert(sv.b[1].tier==3&&sv.cash==5550);
 sv.units=3;select_action(4,44);select_action(5,45);assert(sv.units==0);
 assert(sv.b[0].timer==120&&sv.b[1].timer==120);
 production_tick(119);assert(sv.stocks[0]==0&&sv.stocks[2]==0);
 production_tick(1);assert(sv.stocks[0]==17&&sv.stocks[2]==18);
 puts("Passed: all three franchises, clean income, end-of-day delay, no auto payout, correct desk only, persistence, no duplicate claim, late deposit, audit, exact facility prices, upgrades and 120-second batches.");
}
