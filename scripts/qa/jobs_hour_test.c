/* Deterministic session-only job and listing UI regressions. Uses the same
 * tick/controls as the game. No persistent files or actual player saves. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
static void press(U k){tick(k);tick(0);}
static void reset(void){
 init_city();starter_save();profiles_activate();sv.intro=4;sv.cash=40;sv.dirty=19;sv.owned=1;sv.taxiFares=1;
 scene=1;menu=-1;mapOn=0;clear_input();end_job();bank_ui_reset();set_session_seed(136);
 p.x=2530;p.y=1035;p.hp=100;p.heat=0;p.dead=0;p.car=0;cars[0]=(Car){.x=p.x,.y=p.y,.owner=1,.cond=100};
}
static void listing_pages(void){
 reset();press(ACTIVITIES);assert(vdPanel==1&&vdSelected==0);
 /* Left from issue one should show the first entry of the last issue. */
 press(LEFT);assert(vdSelected==12);press(RIGHT);assert(vdSelected==0);
 for(int first=0;first<VA_COUNT;first++){
  vdSelected=first;press(RIGHT);int expected=(first/6+1)%((VA_COUNT+5)/6)*6+first%6;
  assert(vdSelected==min(expected,VA_COUNT-1));
  vdSelected=first;press(LEFT);expected=(first/6+(VA_COUNT+5)/6-1)%((VA_COUNT+5)/6)*6+first%6;
  assert(vdSelected==min(expected,VA_COUNT-1));
 }
 for(int i=0;i<VA_COUNT;i++){vdSelected=i;press(DOWN);assert(vdSelected==(i+1)%VA_COUNT);}
}
static void shift_start(void){
 reset();taxi_record_ride(100,26,0,126,0);press(TAXI_RECEIPT);press(ENTER);assert(taxiReceipt.ended);press(ESC);
 press(TAXI_QUOTE);assert(taxiRadio==TR_QUOTE);press(ESC);assert(taxiReceipt.trips==1&&taxiReceipt.ended);
 press(TAXI_QUOTE);press(USE);assert(job.kind==JOB_TAXI);press(TAXI_RECEIPT);assert(taxiRadio==TR_RECEIPT);
 assert(!taxiReceipt.ended&&!taxiReceipt.trips&&!taxiReceipt.credit);press(ESC);end_job();
 for(int i=0;i<3;i++){
  taxi_record_ride(60,10,0,70,1);press(TAXI_RECEIPT);press(ENTER);press(ESC);
  if(i==0)press(TAXI_NEXT);
  else {cabRequested=i==2?1:0;beginJob(JOB_TAXI);cabRequested=0;}
  assert(job.kind==JOB_TAXI&&!taxiReceipt.ended&&!taxiReceipt.trips&&sv.cash==40);end_job();
 }
}
static void partial_credit(void){
 reset();sv.cash=WALLET_LIMIT-2;next_fare();assert(job.kind==JOB_TAXI);
 job.step=1;taxi.ride=0;taxi.phase=TAXI_RIDING;p.x=cars[0].x=job.xs[1];p.y=cars[0].y=job.ys[1];
 for(int i=0;i<120;i++)taxi_tick(1.f/60);
 assert(job.kind<0&&taxiReceipt.credit==2&&sv.cash==WALLET_LIMIT);
 assert(strstr(notice,"$2 credited")&&strstr(notice,"wallet limit"));assert(sv.dirty==19);
 next_fare();assert(job.kind==JOB_TAXI);job.step=1;taxi.ride=0;taxi.phase=TAXI_RIDING;
 p.x=cars[0].x=job.xs[1];p.y=cars[0].y=job.ys[1];for(int i=0;i<120;i++)taxi_tick(1.f/60);
 assert(job.kind<0&&strstr(notice,"$0 credited"));
}
static void pedestrian_impact(void){
 for(int aboard=0;aboard<=1;aboard++){
 reset();next_fare();assert(job.kind==JOB_TAXI);
 if(aboard){job.step=1;taxi.ride=0;taxi.phase=TAXI_RIDING;}
 p.x=cars[0].x=768;p.y=cars[0].y=1038;cars[0].s=72;cars[0].a=0;
 for(int i=1;i<nc;i++){cars[i].x=cars[i].y=-100;streetDrivers[i].live=0;}
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){0};
 int ped=STREET_CIVILIANS;peds[ped]=(Ped){.hp=1,.x=p.x+7,.y=p.y};
 tick(UP);assert(!peds[ped].hp&&p.heat>0);
 int normalTip=taxi_tip_amount(job.price,0,0,0);job.step=1;advanceJob();
 assert(taxiReceipt.bumps==aboard&&taxiReceipt.tip==(aboard?0:normalTip));
 }
}
static void regional_routes(void){init_city();starter_save();profiles_activate();scene=1;sv.cash=370;sv.taxiFares=24;sv.intro=4;
 int stops=0,inner=0,cross=0;
 for(int i=0;i<144;i++){
  set_session_seed(100+i);sv.dayClock=i*10.f;lastTaxiPickup=lastTaxiDrop=lastTaxiRider=-1;
  p.x=768;p.y=1038;
  for(int j=0;j<4;j++){TaxiOffer f;assert(taxi_draft(&f));taxi_commit_call(&f);assert(taxi.pickup<LEN(loc));
   if(taxi.dropoff>=LEN(loc)){stops|=1<<taxi_outer_index(taxi.dropoff);cross++;assert(taxi.meters==taxi_dispatch_distance(job.xs[0],job.ys[0],job.xs[1],job.ys[1]));}
   else inner++;end_job();}
 }
 assert(stops==(1<<TAXI_OUTER_STOPS)-1&&inner>350&&cross>40);
 printf("%d downtown draws across %d times/seeds: local %d, outbound %d, outer destinations mask %x/%x\n",144*4,144,inner,cross,stops,(1<<TAXI_OUTER_STOPS)-1);
 /* First-time Dispatch callers never have to leave town to license. */
 sv.taxiFares=0;for(int i=0;i<48;i++){set_session_seed(i);TaxiOffer a;assert(taxi_draft(&a)&&a.taxi.pickup<LEN(loc)&&a.taxi.dropoff<LEN(loc));}
}
int main(int argc,char**argv){
 const char*mode=argc>1?argv[1]:"all";int all=!strcmp(mode,"all");
 if(all||!strcmp(mode,"page"))listing_pages();if(all||!strcmp(mode,"shift"))shift_start();
 if(all||!strcmp(mode,"limit"))partial_credit();if(all||!strcmp(mode,"impact"))pedestrian_impact();
 if(all||!strcmp(mode,"regional"))regional_routes();
 puts("PASS: City listing issue navigation; taxi next-shift rollover, partial fare notification, passenger-impact feedback.");return 0;
}
