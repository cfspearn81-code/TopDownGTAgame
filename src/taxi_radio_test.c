/* Engine-level deterministic menu/transaction regressions; the reusable
 * outer_taxi_replay.py --radio runs the same controls through each host. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <string.h>
static void tap(U k){tick(k);tick(0);}
static void park(float x,float y){
 p.car=0;p.x=x;p.y=y;
 cars[0]=(Car){.x=x,.y=y,.owner=1,.cond=100};
}
static void finish(int pay,int crashes,int damage,int rough){
 job=(Job){.kind=JOB_TAXI,.step=1,.total=2,.price=pay,.time=140.f};
 taxi=(Taxi){.phase=TAXI_EXITING,.ride=0,.crashes=crashes,.damage=damage,.rough=rough};
 advanceJob();
}
static void art(const char *path){
 paint();if(!path)return;
 FILE *out=fopen(path,"wb");assert(out);
 fprintf(out,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc(c>>16,out);fputc(c>>8,out);fputc(c,out);}
 fclose(out);
}
int main(int argc,char **argv){
 init_city();profiles_activate();scene=1;sv.intro=4;sv.taxiFares=1;sv.cash=50;sv.dirty=12;
 park(2530,1035);set_session_seed(136);navHasManual=1;navManualLoc=0;navManual=nav_door(0);nav_slot();
 navHasManual=1;navManualLoc=0;navManual=nav_door(0);nav_update();
 int n=navCount,dist=navDist;NavPoint nodes[LEN(navNodes)];cp(nodes,navNodes,sizeof nodes);
 TaxiOffer target;assert(taxi_draft(&target));U seed=taxiSeed, ticks=sv.playedTicks;
 tap(TAXI_QUOTE);
 assert(taxiRadio==TR_QUOTE&&taxiToPickup>0&&taxiQuote.job.price==target.job.price);
 assert(job.kind<0&&taxiSeed==seed&&sv.cash==50&&sv.dirty==12&&sv.playedTicks==ticks);
 assert(navHasManual&&navManualLoc==0&&n==navCount&&dist==navDist&&!memcmp(nodes,navNodes,sizeof nodes));
 tap(ACTIVITIES);tap(MAP);tap(WHEEL);tap(UP);tap(TAXI_RECEIPT);
 assert(taxiRadio==TR_QUOTE&&!vdPanel&&!mapOn&&!wheelOpen&&p.x==2530&&sv.playedTicks==ticks);
 art(argc>1?argv[1]:0);
 tap(ESC);
 assert(!taxiRadio&&job.kind<0&&lastTaxiPickup<0&&lastTaxiDrop<0&&lastTaxiRider<0&&sv.cash==50);
 assert(taxiSeed==target.after&&navHasManual&&navManualLoc==0);
 tap(TAXI_QUOTE);target=taxiQuote;tap(USE);
 assert(!taxiRadio&&job.kind==JOB_TAXI&&jobRide==p.car&&taxi.rider==target.taxi.rider);
 assert(taxi.pickup==target.taxi.pickup&&taxi.dropoff==target.taxi.dropoff&&job.price==target.job.price);
 tap(TAXI_QUOTE);assert(job.kind==JOB_TAXI&&!taxiRadio);
 tap(TAXI_RECEIPT);assert(taxiRadio==TR_RECEIPT);tap(ENTER);assert(!taxiReceipt.ended&&job.kind==JOB_TAXI);tap(ESC);
 end_job();
 assert(!taxiReceipt.trips);tap(TAXI_NEXT);assert(job.kind==JOB_TAXI);
 end_job();
 finish(100,1,9,1);int tip1=taxi_tip_amount(100,1,9,1);
 assert(sv.cash==150+tip1&&taxiReceipt.trips==1&&taxiReceipt.base==100&&taxiReceipt.tip==tip1&&taxiReceipt.bumps==1&&taxiReceipt.credit==100+tip1);
 cabRequested=1;assert(taxi_offer());cabRequested=0;taxi.copper=CAB_SEEN;job.step=1;int base=job.price,tip=taxi_tip_amount(base,0,0,0);advanceJob();
 assert(taxiReceipt.trips==2&&taxiReceipt.base==100+base&&taxiReceipt.tip==tip1+tip&&taxiReceipt.bonus==20&&taxiReceipt.bumps==1);
 int cash=sv.cash;tap(TAXI_RECEIPT);assert(taxiRadio==TR_RECEIPT);
 art(argc>2?argv[2]:0);
 tap(ENTER);tap(ENTER);assert(taxiReceipt.ended&&taxiReceipt.trips==2&&sv.cash==cash&&sv.dirty==12);tap(TAXI_RECEIPT);assert(!taxiRadio);
 sv.cash=WALLET_LIMIT-3;finish(200,0,0,0);
 assert(taxiReceipt.trips==1&&!taxiReceipt.ended&&taxiReceipt.credit==3&&taxiReceipt.base==200&&sv.cash==WALLET_LIMIT);
 profiles_activate();assert(!taxiRadio&&!taxiReceipt.trips);
 puts("Taxi quote/receipt: preview is nonmutating; decline doesn't start a fare; exact quote accepts; T stays quick; end shift pays $0; new shift/career and wallet cap handled.");
}
