/* Repeatable varied careers with real route generation, ride state transitions,
 * input negotiation and payout code. Test drivers stage the car at the two
 * curbs; road driving itself is covered by the separate retail keyboard tapes. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
static void skey(U k){tick(k);tick(0);}
static U pr;
static U draw(void){pr=1664525u*pr+1013904223u;return (pr>>8) ^ (pr<<17);}
static void stage(int where){
 assert(job.kind==JOB_TAXI);
 p.x=cars[0].x=job.xs[where];p.y=cars[0].y=job.ys[where];p.car=0;cars[0].owner=1;cars[0].s=0;
 for(int i=0;i<100;i++)taxi_tick(1.f/60.f);
}
int main(int argc,char**argv){
 U seed=argc>1?strtoul(argv[1],0,10):1;pr=seed;set_session_seed(seed);
 init_city();starter_save();profiles_activate();scene=1;menu=-1;sv.intro=4;sv.owned=1;sv.taxiFares=1;sv.cash=233;sv.dirty=77;
 int abandoned=0,complete=0,declined=0,outer=0,cap=0,close=0,regional=0;
 for(int n=0;n<128;n++){
  int c=draw()%5;const OuterPoint o=outerNodes[outerStops[draw()%LEN(outerStops)].node];
  int fromCity=c<3;int startId=draw()%LEN(loc);int x,y,curb;
  if(!taxi_curb(startId,&x,&y,&curb)){x=768;y=1038;}
  p.x=cars[0].x=fromCity?x:o.x;p.y=cars[0].y=fromCity?y:o.y;p.car=0;p.hp=100;p.heat=0;
  cars[0].cond=100;cars[0].owner=1;cars[0].s=0;sv.dayClock=(float)(draw()%1440);
  int money=sv.cash;U fares=sv.taxiFares;
  if(draw()%4==0){skey(TAXI_QUOTE);assert(taxiRadio==TR_QUOTE);skey(ESC);declined++;assert(job.kind<0&&sv.cash==money&&sv.taxiFares==fares);}
  if(draw()%2){skey(TAXI_QUOTE);assert(taxiRadio==TR_QUOTE);TaxiOffer a=taxiQuote;skey((draw()%2)?USE:ENTER);assert(job.kind==JOB_TAXI&&taxi.rider==a.taxi.rider&&job.price==a.job.price);}
  else skey(TAXI_NEXT);
  assert(job.kind==JOB_TAXI&&taxi.meters>0);
  if(!fromCity)outer++;
  if(fromCity&&taxi_outer_index(taxi.dropoff)>=0)regional++;
  int raw=60+taxi.meters/16;assert(job.price==raw);
  stage(0);assert(taxi.phase==TAXI_RIDING);
  if(draw()%9==0){p.car=-1;taxi_tick(1.f/60);abandoned++;assert(job.kind<0&&sv.taxiFares==fares&&money==sv.cash);continue;}
  int collisions=draw()%4;
  for(int j=0;j<collisions;j++){taxi.contactSeconds=0;taxi_quality(1.f/60,70,0,2+draw()%17);}
  assert(taxi.crashes==collisions);
  int tip=taxi_tip_amount(raw,taxi.crashes,taxi.damage,taxi.rough);
  if(draw()%16==0){sv.cash=WALLET_LIMIT-(int)(draw()%10);cap++;}
  int before=sv.cash,expected=min(WALLET_LIMIT-before,raw+tip);
  stage(1);complete++;assert(job.kind<0&&sv.taxiFares==fares+1&&sv.cash==before+expected&&sv.dirty==77);
  int final=sv.cash;for(int i=0;i<13;i++)taxi_tick(1.f/60);
  assert(sv.cash==final);if(sv.cash>WALLET_LIMIT-1000)sv.cash=500;
  if(draw()%7==0){skey(TAXI_RECEIPT);assert(taxiRadio==TR_RECEIPT);skey(ENTER);assert(taxiReceipt.ended);skey(ESC);close++;}
 }
 printf("seed=%u fare=128 paid=%d abandoned=%d declined=%d outer_origins=%d city_to_outer=%d wallet_caps=%d shift_ends=%d\n",seed,complete,abandoned,declined,outer,regional,cap,close);
}
