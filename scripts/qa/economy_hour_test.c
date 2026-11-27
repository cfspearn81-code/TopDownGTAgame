/* Actual career/menu/ledger functions on an in-memory disposable disk.
 * Each independent seed varies balances, stock and source of full-wallet pressure.
 * --before documents the old outcomes, not assertions on the repaired build. */
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned char recorded[sizeof(Save11)];static unsigned length,writes;
int disk_write(const void*b,unsigned n){assert(n<=sizeof recorded);memcpy(recorded,b,n);length=n;writes++;return 1;}
int disk_read(void*b,unsigned n){if(n!=length)return 0;memcpy(b,recorded,n);return 1;}
static unsigned seed=1;
static unsigned next(unsigned n){seed=seed*1664525u+1013904223u;return (seed>>8)%n;}
static void fresh(void){starter_save();sv.intro=4;scene=1;p.dead=0;p.car=-1;p.hp=100;p.x=388;p.y=736;menu=-1;mapOn=0;job.kind=-1;assetAlert=0;}
static void roundtrip(void){request_save();assert(saveFlash>0);load_save();assert(wason);}
/* An independent integer ledger follows ordinary mixed street purchases,
 * legal/marked pay, fees, bank transfers, closing and sweeps. The game's
 * state is never used to construct the expected transaction result. */
static void mixed_ledger(void){
 fresh();sv.cash=880;sv.dirty=1400;sv.bank=400;sv.dayClock=1435.f;
 long long c=880,d=1400,bank=400,spent=0;int charge=0,sweep=0,tm=1435*60;
 for(int n=0;n<1200;n++){
  int op=(int)next(9),v=1+(int)next(520);
  if(op<2){assert(credit_wallet(op==0,v)==v);if(op==0)c+=v;else d+=v;}
  else if(op==2||op==3){
   int deposit=op==2;long long from=deposit?c:bank;
   int ok=from>=v;assert((deposit?bank_deposit(v):bank_withdraw(v))==(ok?v:0));
   if(ok){if(deposit)c-=v,bank+=v;else c+=v,bank-=v;}
  }else if(op<7){
   int ok=!(charge&&v>=240)&&c+d>=v;
   assert(canDirtyPurchase(v)==ok);
   if(ok){
    long long clean=c<v?c:v,marked=v-clean,prev=spent;
    c-=clean;d-=marked;spent+=marked;
    if(prev<=500&&spent>500){d=0;if(!charge)sweep=18000;charge=1;}
    finish_purchase();request_save();
   }
  }else if(op==7&&charge&&n%17==0){
   active(11);doOpt(80);if(c>=180){c-=180;charge=0;sweep=0;}
  }else if(op==8&&n%5==0)roundtrip();
  int steps=1+(int)next(56);
  while(steps--){
   if(++tm==86400){tm=0;spent=0;}
   if(charge&&!--sweep){sweep=18000;c=d=0;}
   production_tick(1.f/60);revenue_tick();
  }
  assert(sv.cash==c&&sv.dirty==d&&sv.bank==bank&&sv.dirtySpent==spent&&sv.charges==charge&&sv.sweepTicks==sweep);
  assert((int)(sv.dayClock*60+.5f)==tm);
 }
}
static void run(int before){
 fresh();p.hp=7;roundtrip();spawnPlayer();
 if(before)assert(p.hp==15);else assert(p.hp==7);
 /* Even imported max-unsigned counters must keep dispatch radio unlocked. */
 fresh();sv.taxiFares=~0u;job=(Job){.kind=JOB_TAXI,.step=1,.total=2,.price=25};advanceJob();roundtrip();
 if(before)assert(!sv.taxiFares);else assert(sv.taxiFares==~0u);
 fresh();sv.cash=9;atm_open(0);assert(atm_amount()==9);bank_ui_tick(LEFT);
 if(before)assert(atm_amount()==9);else assert(atm_amount()==1);
 fresh();sv.dirty=WALLET_LIMIT-3;sv.stocks[0]=30;sv.b[0]=(Business){.own=1,.tier=1};sell_stock(0);
 if(before){assert(sv.sales==20&&sv.stocks[0]==7&&sv.dirty==WALLET_LIMIT);}
 else {assert(!sv.sales&&sv.stocks[0]==30&&sv.dirty==WALLET_LIMIT-3);sv.dirty-=22; sell_stock(0);
  assert(sv.sales==1&&sv.stocks[0]==29&&sv.dirty==WALLET_LIMIT-3);}
 fresh();sv.dirty=WALLET_LIMIT-3;sv.b[1]=(Business){.own=1,.crew=2,.tier=1,.product=2,.durationMs=120000,.timer=.01f};production_tick(1.f/60.f);
 if(before)assert(sv.b[1].timer==0&&sv.sales==6&&sv.dirty==WALLET_LIMIT);
 else {assert(sv.b[1].timer>0&&!sv.sales&&sv.dirty==WALLET_LIMIT-3&&!sv.b[1].batches);
   roundtrip();unsigned oldWrites=writes;for(int i=0;i<90;i++)production_tick(1.f/60);
   assert(writes==oldWrites);sv.dirty-=400;production_tick(1.f/60);
   assert(sv.b[1].timer==0&&sv.sales==6&&sv.b[1].batches==1&&sv.dirty==WALLET_LIMIT-403+271);
 }
 fresh();sv.cash=WALLET_LIMIT-3;sv.pending=79;sv.clearT=.01f;production_tick(1.f/60);assert(sv.cash==WALLET_LIMIT);
 if(before)assert(!sv.pending);else {
  assert(sv.pending==76&&sv.clearT<0);roundtrip();unsigned oldWrites=writes;
  production_tick(1.f);assert(writes==oldWrites&&sv.pending==76);assert(bank_deposit(100)==100);
  production_tick(1.f/60);assert(!sv.pending&&sv.cash==WALLET_LIMIT-24);roundtrip();production_tick(1.f);
  assert(!sv.pending&&sv.cash==WALLET_LIMIT-24&&sv.bank==100);
 }
 fresh();sv.units=1000000000-1;sv.cash=45;doOpt(20);
 if(before){assert(sv.units==1000000002);load_save();assert(!wason);}
 else{assert(sv.units==999999999&&sv.cash==45);roundtrip();}
 if(!before){
  for(int k=0;k<800;k++){
   fresh();int stock=1+(int)next(59),item=(int)next(3),headroom=(int)next(1120),price=prod[item].price;
   sv.stocks[item]=stock;sv.dirty=WALLET_LIMIT-headroom;sv.sales=1000000000-1;sv.b[prod[item].site].own=(int)next(2);
   sv.b[prod[item].site].tier=1;sv.protect=next(4)?1:0;
   int units=min(stock,min(20,headroom/price)),lost=units>=15&&stock-units>=3&&sv.b[prod[item].site].own&&!sv.protect?3:0;
   active(7);makeMenu();assert(opt[item].action==60+item);
   if(units){char quote[24];fmt(quote,"= $",units*price);assert(strstr(opt[item].note,quote));
    assert(!!strstr(opt[item].note,"3 more")==!!lost);}
   else assert(strstr(opt[item].note,"No sale"));
   sell_stock(item);assert(sv.stocks[item]==stock-units-lost&&sv.dirty==WALLET_LIMIT-headroom+units*price&&sv.sales==min(1000000000,999999999+units));
   roundtrip();
   int hand=1+(int)next(34000),bank=(int)next(500),old=0;sv.cash=hand;sv.bank=bank;atm_open(0);
   for(int j=0;j<20;j++){old=atm_amount();atm_adjust(1);assert(old<hand?atm_amount()>old:atm_amount()==hand);}
   for(int j=0;j<20;j++){old=atm_amount();atm_adjust(-1);assert(old>1?atm_amount()<old:atm_amount()==1);}
   int claim=1+(int)next(480),space=(int)next(479);sv.pending=claim;sv.cash=WALLET_LIMIT-space;sv.clearT=.005f;
   production_tick(1.f/60);assert(sv.cash==WALLET_LIMIT-space+min(claim,space)&&sv.pending==max(0,claim-space));
   roundtrip();
  }
  mixed_ledger();
  fresh();sv.cash=3702;sv.pending=32;sv.clearT=0;production_tick(1.f);
  assert(sv.cash==3702&&sv.pending==32&&sv.clearT==0);roundtrip();
  /* A valid imported Cinder rack at the previous save reader's ceiling
   * remains loadable and releases the already-paid harvest after a sale. */
  fresh();sv.stocks[2]=500000000;sv.b[1]=(Business){.own=1,.tier=1,.product=2,.durationMs=120000,.timer=.01f};
  production_tick(1.f);assert(sv.stocks[2]==500000000&&sv.b[1].timer>0);roundtrip();
  sell_stock(2);production_tick(1.f);assert(!sv.b[1].timer&&sv.stocks[2]==499999983);roundtrip();
 }
}
int main(int argc,char**argv){int before=argc>1&&!strcmp(argv[1],"--before");if(argc>1&&!before)seed=(unsigned)strtoul(argv[1],0,10);
 init_city();run(before);printf("%s economy: %u disposable writes; %s\n",before?"BASE":"PASS",writes,before?"low-HP restore, ATM no-op arrow, Kiosk and distributor stock loss, early settlement write-off and invalid supply save reproduced":"800 randomized stock/ATM/settlement cases and 1200 independently modeled transactions");}
