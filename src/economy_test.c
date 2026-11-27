/* Headless game-code economy checks with an in-memory disk. */
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned char stored[sizeof(Save11)];
static unsigned stored_n;
static int failWrite;
int disk_write(const void*p,unsigned n){assert(n==sizeof(Save)||n==sizeof(Save11));if(failWrite)return 0;cp(stored,p,n);stored_n=n;return 1;}
int disk_read(void*p,unsigned n){if(n!=stored_n)return 0;cp(p,stored,n);return 1;}
static void historical_save(void){
 /* Construct the bytes an old build wrote, not the current save writer. */
 sv.x=p.x;sv.y=p.y;sv.hp=p.hp;sv.hash=savehash();assert(disk_write(&sv,sizeof sv));
}
static void select_action(int m,int a){active(m);makeMenu();doOpt(a);}
static void check_revenue_receipt(int kind,int clean,int dirty){
 RevenueReceipt r=revenue_seizure_notice();assert(r.kind==kind&&r.clean==clean&&r.dirty==dirty);
}
/* Loss paths matter as much as an ideal completion. These scenarios use
 * the production contract, GPS and checkout code, plus round-trip saves. */
static void test_daily_spend(void){
 init_city();scene=1;starter_save();sv.cash=650;sv.dirty=1000;sv.dayClock=75.f;
 assert(bank_deposit(600)==600&&bank_balance()==600&&sv.cash==50&&sv.dirtySpent==0);
 select_action(9,70); /* $550 cost: the exact $500 dirty portion is safe. */
 assert(sv.home&&sv.cash==0&&sv.dirty==500&&sv.dirtySpent==500&&!sv.charges);check_revenue_receipt(REVENUE_NO_RECEIPT,0,0);
 load_save();assert(wason&&sv.rev==SAVE_REV&&sv.home&&sv.dirtySpent==500&&sv.dayClock==75.f&&sv.bank==600);
 sv.pending=37;sv.clearT=10.f;
 sv.legit[0]=(LegalBusiness){.owned=1,.pending=165,.cooldown=400.f};
 sv.legit[1]=(LegalBusiness){.owned=1,.pending=240};
 credit_wallet(1,44); /* $45 supply crate costs $1 dirty. */
 int units=sv.units;
 select_action(2,20);
 assert(sv.charges&&sv.dirty==0&&sv.dirtySpent==501&&sv.units==units+3);
 assert(!sv.pending&&!sv.clearT&&!sv.legit[0].pending&&!sv.legit[0].cooldown&&!sv.legit[1].pending);
 assert(sv.bank==600&&sv.cash==0&&sv.sweepTicks==REVENUE_SWEEP_TICKS);check_revenue_receipt(REVENUE_THRESHOLD,0,499);check_revenue_receipt(REVENUE_NO_RECEIPT,0,0);
 load_save();assert(wason&&sv.dirtySpent==501&&sv.charges&&sv.bank==600&&sv.units==units+3);
 assert(bank_withdraw(180)==180&&bank_balance()==420&&sv.dirtySpent==501);
 select_action(11,80);assert(!sv.charges&&sv.cash==0);
 credit_wallet(0,100);select_action(2,20);
 assert(!sv.charges&&!sv.sweepTicks&&sv.dirty==55&&sv.dirtySpent==546);check_revenue_receipt(REVENUE_NO_RECEIPT,0,0);
 sv.dayClock=(DAY_SECONDS*60-1)/60.f;request_save();load_save();
 assert(wason&&sv.dirtySpent==546&&sv.dayClock>DAY_SECONDS-1.f);
 production_tick(1.f/60);assert(!sv.dirtySpent&&sv.dayClock==0.f&&sv.dirty==55&&sv.bank==420);
 load_save();assert(wason&&sv.dirtySpent==0&&sv.dayClock==0.f);
 /* A day with no remaining balance still raises the case only on >$500. */
 sv.dirtySpent=499;sv.dirty=45;select_action(2,20);assert(sv.charges&&!sv.dirty&&sv.dirtySpent==544);check_revenue_receipt(REVENUE_THRESHOLD,0,0);
 sv.restitution=2;sv.sweepTicks=11234;sv.dayClock=(DAY_SECONDS*60-1)/60.f;production_tick(1.f/60);
 sv.dirty=800;for(int i=0;i<12;i++)select_action(2,20);
 assert(sv.charges&&sv.restitution==2&&sv.dirtySpent==540&&sv.sweepTicks==11234);check_revenue_receipt(REVENUE_THRESHOLD,0,260);
}
static void test_bank_transactions(void){
 init_city();scene=1;starter_save();sv.cash=112;sv.dirty=10000;sv.dirtySpent=33;
 assert(!bank_deposit(113)&&!bank_deposit(0)&&!bank_deposit(-1)&&!bank_withdraw(1));
 assert(!bank_balance()&&sv.cash==112&&sv.dirty==10000&&sv.dirtySpent==33);
 failWrite=1;assert(!bank_deposit(12)&&saveFlash<0&&sv.cash==112&&!bank_balance());failWrite=0;
 assert(bank_deposit(112)==112&&bank_balance()==112&&sv.cash==0&&sv.dirtySpent==33);
 load_save();assert(wason&&bank_balance()==112&&sv.cash==0&&sv.dirty==10000&&sv.dirtySpent==33);
 failWrite=1;assert(!bank_withdraw(12)&&saveFlash<0&&!sv.cash&&bank_balance()==112);failWrite=0;
 revenue_open_case();assert(bank_withdraw(12)==12&&bank_balance()==100&&sv.cash==12);
 assert(bank_deposit(12)==12&&bank_balance()==112&&sv.charges);
 sv.cash=WALLET_LIMIT;assert(!bank_withdraw(1)&&bank_balance()==112&&sv.cash==WALLET_LIMIT);
 sv.bank=WALLET_LIMIT;assert(!bank_deposit(1)&&bank_balance()==WALLET_LIMIT&&sv.cash==WALLET_LIMIT);
 /* Denied and clean purchases never add to the meter with big dirty balances. */
 starter_save();sv.cash=1000;sv.dirty=10000;
 select_action(9,70);assert(sv.home&&sv.cash==450&&sv.dirty==10000&&!sv.dirtySpent&&!sv.charges);
 sv.cash=0;sv.dirty=40;select_action(2,20);assert(!sv.units&&sv.dirty==40&&!sv.dirtySpent&&!sv.charges);
 /* Clean labor/trading/rear-desk deposits are distinct from checkout. */
 sv.dirty=670;sv.legit[0].owned=1;
 select_action(19,LEGAL_DEPOSIT_FIRST);assert(sv.dirty==450&&sv.legit[0].pending==165&&!sv.dirtySpent&&!sv.charges);
 sv.stocks[0]=10;select_action(7,60);assert(sv.dirty==670&&!sv.dirtySpent&&!sv.charges);
}
static void test_paycheck_gps(void){
 init_city();scene=1;starter_save();sv.intro=2;sv.owned=0;sv.legal=0;sv.cash=0;
 navHasManual=0;job=(Job){.kind=JOB_COURIER,.step=2,.total=3,.price=115};
 advanceJob();assert(job.kind<0&&sv.cash==140&&sv.intro==3);
 load_save();NavPoint goal;int venue;
 assert(wason&&nav_objective(&goal,&venue)==1&&venue==3&&nav_same(goal,nav_door(3)));
 navHasManual=1;navManualLoc=2;assert(nav_objective(&goal,&venue)==3&&venue==2);
 navHasManual=0;beginJob(JOB_RECOVERY);assert(job.kind==JOB_RECOVERY&&nav_objective(&goal,&venue)==4);
 end_job();assert(nav_objective(&goal,&venue)==1&&venue==3);
 sv.owned=1;assert(!nav_objective(&goal,&venue));
}
static void test_licensed_jobs(void){
 init_city();scene=1;sv.intro=4;sv.owned=0;p.car=-1;end_job();
 int fare=sv.taxiFares,legal=sv.legal,cash=sv.cash;
 const int kinds[]={JOB_TAXI,JOB_SPRINT};
 for(int j=0;j<2;j++){
  int k=kinds[j];beginJob(k);assert(job.kind==k&&jobRide==nc-1);
  cars[0].cond=cars[jobRide].cond=0;
  job_clock_tick(1.f/60);assert(job.kind<0&&jobRide<0);
  assert(sv.taxiFares==(U)fare&&sv.legal==legal&&sv.cash==cash);
  beginJob(k);assert(job.kind==k&&jobRide==nc-1&&cars[jobRide].cond==100);
  cars[0]=(Car){256,512,0,0,0,1,51};cars[jobRide].cond=0;
  job_clock_tick(1.f/60);assert(job.kind==k&&job_vehicle_target()==0);
  p.car=0;assert(job_vehicle_target()<0);end_job();p.car=-1;
 }
 sv.taxiFares=1;p.car=0;next_fare();assert(job.kind==JOB_TAXI);
 p.car=-1;assert(job_vehicle_target()==0); /* T fare: walk back to parked car. */
 taxi.phase=TAXI_RIDING;taxi.ride=0;job.step=1;
 taxi_tick(0);assert(job.kind<0&&jobRide<0&&sv.cash==cash);
}
static void restitution_courier(void){
 select_action(0,10);assert(job.kind==JOB_COURIER);
 while(job.kind==JOB_COURIER){int venue=job_venue();assert(venue>=0&&interior_job_drop(venue));}
}
static void test_restitution_service(void){
 init_city();scene=1;starter_save();sv.intro=1;revenue_open_case();sv.cash=64;sv.dirty=61;
 active(11);makeMenu();assert(strstr(opt[1].note,"pay $0"));
 active(0);makeMenu();assert(strstr(opt[0].n,"UNPAID")&&strstr(opt[0].note,"$0"));
 active(10);makeMenu();assert(strstr(opt[0].n,"UNPAID")&&strstr(opt[0].note,"$0"));
 restitution_courier();
 assert(sv.legal==1&&sv.intro==2&&sv.restitution==1&&sv.charges&&sv.cash==64&&sv.dirty==61);
 assert(strstr(notice,"1/3")&&strstr(notice,"paid $0")&&notice_color()==GOLD);
 load_save();assert(wason&&sv.rev==SAVE_REV&&sv.charges&&sv.restitution==1&&sv.cash==64);
 /* Unrelated jobs still pay and do not advance a case. */
 job=(Job){.kind=JOB_TAXI,.step=1,.total=2,.price=100}; taxi=(Taxi){0};
 advanceJob();assert(sv.cash==190&&sv.restitution==1&&sv.charges&&sv.taxiFares==1);
 job=(Job){.kind=JOB_VAULT,.step=1,.total=2,.price=260};
 advanceJob();assert(sv.dirty==321&&sv.restitution==1&&sv.charges);
 /* Abandoning doesn't pay or count, even after a parcel handoff. */
 beginJob(JOB_COURIER);assert(interior_job_drop(job_venue()));
 select_action(PAUSE_MENU,92);assert(job.kind<0&&sv.restitution==1&&sv.cash==190);
 restitution_courier();assert(sv.charges&&sv.restitution==2&&sv.cash==190);
 sv.sweepTicks=1;restitution_courier();revenue_tick();assert(!sv.charges&&!sv.restitution&&!sv.sweepTicks&&sv.cash==190&&sv.dirty==321);
 assert(strstr(notice,"cleared")&&strstr(notice,"paid $0"));
 load_save();assert(wason&&!sv.charges&&!sv.restitution&&sv.cash==190);
 active(0);makeMenu();assert(!strstr(opt[0].n,"UNPAID"));
 active(10);makeMenu();assert(!strstr(opt[0].n,"UNPAID"));
 restitution_courier();assert(sv.cash>190&&sv.dirty==321&&!sv.restitution);
 /* With no case, the real opening bonus still pays. */
 starter_save();sv.intro=1;
 restitution_courier();assert(sv.cash==140&&sv.intro==3&&!sv.charges&&!sv.restitution);
 /* The $0 shift you accepted remains unpaid even if you settle the case
  * at the Annex before the last handoff; the following assignment pays. */
 starter_save();sv.intro=4;sv.cash=200;revenue_open_case();sv.sweepTicks=1;
 beginJob(JOB_COURIER);assert(job.unpaid&&job_is_service(job.kind));select_action(11,80);
 revenue_tick();assert(sv.cash==20&&!sv.charges&&!sv.sweepTicks&&!job_is_service(job.kind)&&job.unpaid);
 while(job.kind==JOB_COURIER)assert(interior_job_drop(job_venue()));
 assert(sv.cash==20&&!sv.restitution&&strstr(notice,"already closed"));
 restitution_courier();assert(sv.cash>20);
 /* Crossing the current day's cap during a paid assignment converts it. */
 starter_save();sv.intro=4;sv.dirty=700;sv.dirtySpent=499;
 beginJob(JOB_COURIER);assert(!job.unpaid);
 select_action(2,20);assert(sv.charges&&job.unpaid&&strstr(notice,"now counts as unpaid"));
 while(job.kind==JOB_COURIER)assert(interior_job_drop(job_venue()));
 assert(sv.cash==0&&sv.restitution==1&&sv.charges);
 puts("Passed: unpaid courier and recovery listings, service progress through reload, final shift pays $0, regular jobs still pay, and no advance on abandoning.");
}
static void gameplay_frames(int n){for(int i=0;i<n;i++)tick(0);}
static void test_recurring_sweeps(void){
 init_city();scene=1;starter_save();sv.intro=4;
 sv.dayClock=DAY_SECONDS-450.f;sv.cash=945;assert(bank_deposit(945)==945);sv.dirty=845;sv.dirtySpent=499;
 open_interior(2);select_action(2,20);assert(sv.charges&&sv.sweepTicks==18000);
 check_revenue_receipt(REVENUE_THRESHOLD,0,800);assert(sv.bank==945&&!sv.cash&&!sv.dirty);
 /* Prepare some on-hand balances and prove only the wallets are swept. */
 assert(bank_withdraw(145)==145);sv.dirty=34;
 sv.pending=18;sv.clearT=17.f;sv.legit[0]=(LegalBusiness){.owned=1,.pending=90,.cooldown=100.f};
 /* A real franchise still operates and pays before the collection
  * deadline; record its earned cash when testing the confiscation. */
 gameplay_frames(17999);assert(sv.sweepTicks==1&&sv.cash==193&&sv.dirty==34&&!assetAlert);
 assert(sv.pending==18&&sv.legit[0].pending==90);
 char timer[16];revenue_time(timer);assert(!strcmp(timer,"0:01"));
 request_save();load_save();assert(wason&&sv.sweepTicks==1&&sv.cash==193&&sv.bank==800);
 active(PAUSE_MENU);gameplay_frames(180);assert(sv.sweepTicks==1&&sv.cash==193);
 scene=2;menu=-1;gameplay_frames(180);assert(sv.sweepTicks==1&&sv.cash==193);
 scene=3;active(2);p.dead=1;gameplay_frames(180);assert(sv.sweepTicks==1&&sv.cash==193);p.dead=0;
 gameplay_frames(1);
 assert(!sv.cash&&!sv.dirty&&sv.bank==800&&sv.charges&&sv.sweepTicks==18000);
 assert(assetAlert&&assetSeizure.kind==REVENUE_SWEEP&&assetSeizure.clean==193&&assetSeizure.dirty==34);
 assert(sv.pending==18&&sv.legit[0].pending==90);
 load_save();assert(wason&&sv.sweepTicks==18000&&!sv.cash&&!sv.dirty&&sv.bank==800);
 /* The second deadline spans the 24-minute daily rollover; retain cadence. */
 credit_wallet(1,33);credit_wallet(0,99);sv.dirtySpent=400;
 gameplay_frames(17999);assert(sv.cash==81&&sv.dirty==99&&sv.sweepTicks==1&&!sv.dirtySpent);
 gameplay_frames(1);
 assert(!sv.cash&&!sv.dirty&&sv.bank==800&&sv.sweepTicks==18000&&assetSeizure.clean==105&&assetSeizure.dirty==99);
 /* An empty sweep advances/persists its schedule without a false alert. */
 sv.legit[0].owned=0;gameplay_frames(18000);
 assert(sv.sweepTicks==18000&&!sv.cash&&!sv.dirty&&!assetAlert&&sv.bank==800);
 load_save();assert(wason&&sv.sweepTicks==18000&&!sv.cash&&!sv.dirty);
 /* Atlas and ATM are live contexts just like storefront menus. */
 scene=1;menu=-1;mapOn=1;gameplay_frames(5);assert(sv.sweepTicks==17995);
 mapOn=0;atm_open(0);gameplay_frames(5);assert(sv.sweepTicks==17990);bank_ui_reset();
 puts("Passed: exact 5/10-minute wallet sweeps, empty wallets, protected bank and ledger, save/reload, pause/guide/downed, live atlas/ATM and independent day rollover.");
}
static void show_day(const char*expected){char t[6];day_clock_label(t);assert(!strcmp(t,expected));}
static void test_day_clock(void){
 init_city();scene=1;starter_save();sv.intro=4;sv.dirtySpent=500;
 open_interior(2);active(2);
 show_day("00:00");gameplay_frames(59);show_day("00:00");
 gameplay_frames(1);show_day("00:01");
 gameplay_frames(3539);show_day("00:59");
 gameplay_frames(1);show_day("01:00");
 gameplay_frames(25200);show_day("08:00");assert(sv.dirtySpent==500);
 gameplay_frames(57599);show_day("23:59");assert(DAY_SECONDS==1440&&sv.dirtySpent==500&&sv.playedTicks==86399);
 request_save();load_save();assert(wason&&sv.rev==SAVE_REV&&sv.dirtySpent==500);show_day("23:59");
 active(PAUSE_MENU);gameplay_frames(180);show_day("23:59");assert(sv.playedTicks==86399&&sv.dirtySpent==500);
 scene=2;menu=-1;gameplay_frames(60);show_day("23:59");assert(sv.dirtySpent==500);
 scene=3;active(2);gameplay_frames(1);show_day("00:00");
 assert(sv.dayClock==0.f&&sv.playedTicks==86400&&!sv.dirtySpent);
 load_save();assert(wason&&sv.playedTicks==86400&&!sv.dirtySpent);show_day("00:00");
 puts("Passed: HH:MM from real minutes/seconds, no 08:00 reset, midnight at precisely 86,400 active ticks, save/reload and paused clock.");
}
static void test_old_day_clock_save(void){
 init_city();scene=1;starter_save();sv.rev=9;sv.intro=4;sv.dayClock=470.f;
 sv.bank=777;sv.dirty=654;sv.cash=743;sv.dirtySpent=402;
 revenue_open_case();sv.sweepTicks=137;sv.restitution=2;
 sv.legit[0]=(LegalBusiness){.owned=1,.pending=23,.cooldown=10.f};
 sv.legit[1]=(LegalBusiness){.owned=1,.pending=45,.cooldown=490.f};
 sv.legit[2]=(LegalBusiness){.owned=1,.pending=67};
 historical_save();load_save();
 assert(wason&&sv.rev==SAVE_REV&&sv.dayClock==470.f&&sv.bank==777&&sv.dirty==654&&sv.cash==743&&sv.dirtySpent==402);
 assert(sv.charges&&sv.restitution==2&&sv.sweepTicks==137);
 assert(sv.legit[0].pending==23&&sv.legit[0].cooldown==970.f);
 assert(sv.legit[1].pending==45&&sv.legit[1].cooldown==2410.f);
 assert(sv.legit[2].pending==67&&!sv.legit[2].cooldown);
 show_day("07:50");
 production_tick(970.f);
 assert(sv.legit[0].pending==23&&!sv.legit[0].cooldown&&sv.legit[1].cooldown==1440.f);
 load_save();assert(wason&&sv.legit[2].pending==67&&!sv.legit[2].cooldown);
 puts("Passed: Rev9 retains phase, wallets and Revenue timer; normal, deferred and ready franchise deposits each keep their promised closing.");
}
static void test_lantern_operator_only(void){
 init_city();scene=1;starter_save();sv.intro=4;
 sv.cash=900;sv.dirty=47;sv.bank=410;sv.protect=1;sv.units=1;sv.sales=12;
 sv.b[0]=(Business){.own=1,.crew=2,.tier=3,.product=1,.durationMs=120000,.batches=1,.timer=25.f};
 sv.b[1]=(Business){.own=1,.crew=2,.tier=1,.product=2};
 sv.stocks[0]=40;sv.stocks[1]=20;sv.stocks[2]=60;
 const Save prior=sv;
 /* Test the actual deserializer across each loaded-save acceptance branch. */
 for(int rev=8;rev<=SAVE_REV;rev++){
  sv=prior;sv.rev=(U)rev;
  if(rev==8){
   sv.sweepTicks=(int)hashbytes(LEGACY8_SIZE-4);cp(stored,&sv,LEGACY8_SIZE);stored_n=LEGACY8_SIZE;
  }else historical_save();
  load_save();
  Business expected=prior.b[0];expected.crew=1;
  assert(wason&&sv.rev==SAVE_REV&&sv.hash==savehash()&&!memcmp(&sv.b[0],&expected,sizeof expected));
  assert(!memcmp(sv.stocks,prior.stocks,sizeof sv.stocks)&&!memcmp(&sv.b[1],&prior.b[1],sizeof prior.b[1]));
  assert(sv.cash==prior.cash&&sv.dirty==prior.dirty&&sv.bank==prior.bank&&sv.units==prior.units&&sv.sales==prior.sales);
 }
 /* No direct sale; grandfather exactly this harvest. Even the next manual
    start and automatic start must now reserve space inside the normal cap. */
 production_tick(24.5f);assert(sv.b[0].timer==.5f&&warehouse_units(0)==60);
 production_tick(.5f);
 assert(warehouse_units(0)==76&&sv.stocks[1]==36&&sv.b[0].timer==0.f&&sv.b[0].batches==2&&sv.units==1);
 assert(sv.cash==prior.cash&&sv.dirty==prior.dirty&&sv.bank==prior.bank&&sv.sales==prior.sales);
 load_save();assert(wason&&sv.b[0].crew==1&&warehouse_units(0)==76&&sv.b[1].crew==2);
 open_interior(4);int_switch_room(4,1);active(4);makeMenu();
 for(int j=0;j<nopt;j++)assert(opt[j].action!=50&&!strstr(opt[j].n,"DISTRIBUTOR"));
 doOpt(50);assert(sv.cash==prior.cash&&sv.b[0].crew==1);
 select_action(4,44);assert(sv.units==1&&!sv.b[0].timer);
 sell_stock(1);production_tick(1.f);assert(warehouse_units(0)==56&&sv.units==1&&!sv.b[0].timer);
 sell_stock(1);production_tick(1.f);assert(warehouse_units(0)==40&&!sv.units&&sv.b[0].timer==120.f);

 /* Cinder's purchase and direct sale still work, even while its stock shelf
    is full; the neighboring Lantern batch goes to storage instead. */
 sv.b[1].crew=1;active(5);makeMenu();int found=0;
 for(int j=0;j<nopt;j++)if(opt[j].action==51&&strstr(opt[j].n,"HIRE DISTRIBUTOR"))found=1;
 assert(found);int clean=sv.cash;doOpt(51);
 assert(sv.cash==clean-210&&sv.b[1].crew==2);
 sv.units=2;int dirty=sv.dirty,sales=sv.sales;
 production_tick(0.5f);assert(sv.b[1].timer==120.f&&!sv.units);
 production_tick(120.f);
 assert(warehouse_units(0)==56&&sv.stocks[2]==60&&sv.b[1].batches==1&&sv.b[1].timer==0.f);
 assert(sv.dirty==dirty+6*prod[2].price-5&&sv.sales==sales+6);

 /* Retain even a synthetic valid import at the serializer's upper bound
    rather than producing a save that cannot be loaded after harvest. */
 sv.b[0].timer=.01f;sv.stocks[1]=SAVE_STOCK_LIMIT;
 production_tick(1.f/60);assert(sv.stocks[1]==SAVE_STOCK_LIMIT&&sv.b[0].timer>0.f&&valid_save_core());
 sv.stocks[1]-=16;production_tick(1.f/60);
 assert(sv.b[0].timer==0.f&&sv.stocks[1]==SAVE_STOCK_LIMIT);
 load_save();assert(wason&&sv.stocks[1]==SAVE_STOCK_LIMIT);
 puts("Passed: Lantern-only operator, Rev8–10 crew migration, full legacy harvest, safe overflow and resume, blocked old purchase and unchanged Cinder distributor.");
}
static void stress_closing_windows(void){
 /* Every minute/second in a full day, at all three different counters.
  * Test the final frame before closing, the closing itself, reloading both
  * states, and spamming claim. Offline time is never advanced. */
 init_city();scene=1;sv.audit=0;revenue_close_case();
 for(int i=0;i<LEN(legal_specs);i++)for(int sec=0;sec<DAY_SECONDS;sec++){
  z(sv.legit,sizeof sv.legit);sv.legit[i].owned=1;
  sv.cash=0;sv.dirty=600;sv.dayClock=(float)sec;
  int until=DAY_SECONDS-sec;if(until<LEGAL_MIN_WINDOW)until+=DAY_SECONDS;
  int amount=legal_specs[i].limit,net=legal_net(amount);
  select_action(LEGAL_REAR_FIRST+i,LEGAL_DEPOSIT_FIRST+i);
  assert(sv.legit[i].pending==net&&sv.dirty==600-amount&&sv.legit[i].cooldown==(float)until);
  legal_tick((float)until-1.f/60);
  int before=sv.cash;
  select_action(LEGAL_REAR_FIRST+i,LEGAL_CLAIM_FIRST+i);
  assert(sv.cash==before&&sv.legit[i].pending==net);
  request_save();load_save();assert(wason&&sv.legit[i].cooldown>0);
  legal_tick(1.f/60);assert(sv.legit[i].cooldown==0);
  request_save();load_save();assert(wason&&sv.legit[i].pending==net);
  before=sv.cash;select_action(LEGAL_REAR_FIRST+i,LEGAL_CLAIM_FIRST+i);
  select_action(LEGAL_REAR_FIRST+i,LEGAL_CLAIM_FIRST+i);
  assert(sv.cash==before+net&&sv.legit[i].pending==0);
 }
 printf("Passed: %d timing/desk combinations with last-frame refusals, settled claims, duplicate claims and persisted balances.\n",LEN(legal_specs)*DAY_SECONDS);
}
int main(void){
 init_city();scene=1;sv.intro=4;sv.cash=4000;sv.dirty=900;
 for(int i=0;i<3;i++){
  int before=sv.cash;select_action(16+i,LEGAL_BUY_FIRST+i);
  assert(sv.legit[i].owned&&sv.cash==before-legal_specs[i].cost&&sv.legit[i].pending==0);
 }
 int paid=sv.cash;production_tick(119);assert(sv.cash==paid);
 production_tick(1);assert(sv.cash==paid+24+36+49);
 sv.dayClock=DAY_SECONDS-90;
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
 sv.dayClock=DAY_SECONDS-10;select_action(20,LEGAL_DEPOSIT_FIRST+1);assert(sv.legit[1].cooldown==DAY_SECONDS+10);
 production_tick(12);assert(sv.legit[1].cooldown==DAY_SECONDS-2);
 sv.cash=0;sv.dirtySpent=480;select_action(2,20);
 assert(sv.charges&&sv.dirty==0&&sv.legit[1].pending==0);
 revenue_close_case();sv.cash=25000;
 select_action(4,40);select_action(5,41);assert(sv.cash==12000&&sv.b[0].own&&sv.b[1].own);
 select_action(4,48);select_action(4,48);assert(sv.b[0].tier==3&&sv.cash==10200);
 select_action(5,49);select_action(5,49);assert(sv.b[1].tier==3&&sv.cash==5550);
 sv.units=3;select_action(4,44);assert(sv.units==3);
 open_interior(4);int_switch_room(4,1);select_action(4,44);
 open_interior(5);select_action(5,45);assert(sv.units==0);
 assert(sv.b[0].timer==120&&sv.b[1].timer==120);
 production_tick(119);assert(sv.stocks[0]==0&&sv.stocks[2]==0);
 production_tick(1);assert(sv.stocks[0]==17&&sv.stocks[2]==18);
 sv.stocks[0]=30;sv.stocks[1]=10;sv.stocks[2]=35;sv.sales=0;
 paid=sv.dirty;select_action(7,60);
 assert(sv.stocks[0]==7&&sv.stocks[1]==10&&sv.stocks[2]==35&&sv.sales==20&&sv.dirty==paid+440);
 assert(p.heat==0&&peds[40].hp==0);
 select_action(5,52);assert(warehouse_secured(1)&&!warehouse_secured(0));
 select_action(7,62);assert(sv.stocks[2]==15&&sv.stocks[0]==7);
 sv.protect=1;assert(warehouse_secured(0)&&warehouse_secured(1)); /* Legacy universal lockboxes. */
 sv.stocks[0]=60;sv.stocks[1]=0;sv.stocks[2]=0;sv.units=3;
 sv.b[0].crew=1;sv.b[1].crew=1;production_tick(1.f);
 assert(sv.b[0].timer==0&&sv.b[1].timer==120.f&&sv.units==1);
 test_daily_spend();test_bank_transactions();test_paycheck_gps();test_licensed_jobs();test_restitution_service();test_recurring_sweeps();test_day_clock();test_old_day_clock_save();test_lantern_operator_only();stress_closing_windows();
 puts("Passed: exact $500 safe, signed checkout completes before $501 seizure, daily reset/reload, protected transfers and failed-write rollback; paycheck GPS and licensed job recovery.");
 puts("Passed: all three franchises, clean income, end-of-day delay, no auto payout, correct desk only, persistence, no duplicate claim, late deposit, audit, exact facility prices, upgrades and 120-second batches.");
}
