/* Synthetic careers, same game functions and fixed-tick passenger animation.
 * No runtime hooks or player files. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <string.h>
static void stop(int stage,int car){
 p.car=car;cars[car]=(Car){.x=job.xs[stage],.y=job.ys[stage],.owner=1,.cond=100};
 p.x=cars[car].x;p.y=cars[car].y;
 for(int i=0;i<100;i++)taxi_tick(1.f/60.f);
}
int main(void){
 init_city();starter_save();spawnCars();spawnPlayer();end_job();scene=1;menu=-1;vd_reset();
 sv.intro=1;beginJob(JOB_RECOVERY);assert(job.kind==JOB_RECOVERY);advanceJob();advanceJob();
 assert(sv.intro==1&&sv.cash==160&&nav_intro_venue()==0&&!vd_launch(VA_A1));
 beginJob(JOB_COURIER);assert(sv.intro==2&&courierRoute==0);
 while(job.kind==JOB_COURIER)assert(interior_job_drop(job_venue()));
 assert(sv.cash==300&&sv.intro==3&&nav_intro_venue()==3);
 puts("Recovery before opening courier: pays $160; Mara's opening route, $140 paycheck and unlock are preserved.");
 int pickups=0,drops=0,city=0,expansion=0;int fares=0;
 const int hours[]={2,6,10,16};set_session_seed(17);p.car=0;job.kind=-1;sv.taxiFares=1;
 for(int h=0;h<LEN(hours);h++)for(int i=0;i<LEN(outerStops);i++){
  sv.dayClock=(float)hours[h]*60;OuterPoint origin=outerNodes[outerStops[i].node];
  for(int m=0;m<12;m++){
   p.x=origin.x;p.y=origin.y;p.car=0;cars[0].owner=cars[0].cond=1;
   int prevp=lastTaxiPickup,prevd=lastTaxiDrop,prevr=lastTaxiRider;int cash=sv.cash;
   next_fare();assert(job.kind==JOB_TAXI&&taxi_outer_index(taxi.pickup)>=0);
   assert(taxi.rider!=prevr&&taxi.pickup!=prevp&&taxi.pickup!=prevd&&taxi.dropoff!=prevp&&taxi.dropoff!=prevd);
   assert(taxi_dispatch_distance(origin.x,origin.y,job.xs[0],job.ys[0])>0);
   for(int j=0;j<2;j++){int x,y,curb;
    assert(taxi_curb(j?taxi.dropoff:taxi.pickup,&x,&y,&curb)&&!solid(x,y,12)&&!solid(x,curb,6));
    assert(job.xs[j]==x&&job.ys[j]==y);
   }
   int expected=60+taxi_dispatch_distance(job.xs[0],job.ys[0],job.xs[1],job.ys[1])/16;
   assert(expected==job.price&&expected>60);
   assert(!strcmp(taxi_route_name(),taxi_stop_label(taxi.pickup)));
   pickups|=1<<taxi_outer_index(taxi.pickup);int k=taxi_outer_index(taxi.dropoff);
   if(k>=0){expansion++;drops|=1<<k;}else city++;
   stop(0,0);assert(job.step==1&&taxi.phase==TAXI_RIDING);
   assert(!strcmp(taxi_route_name(),taxi_stop_label(taxi.dropoff)));
   stop(1,0);assert(job.kind<0&&taxi.phase==TAXI_WALKING&&sv.cash==cash+expected+taxi_tip_amount(expected,0,0,0));
   int before=sv.cash;for(int j=0;j<300;j++)taxi_tick(1.f/60);assert(sv.cash==before&&!taxi.linger);
   fares++;
  }
 }
 assert(pickups==(1<<LEN(outerStops))-1&&drops==pickups&&city&&expansion);
 printf("Completed %d priced and tipped trips: six outer pickup/drop neighborhoods, %d inner / %d outer drop-offs, four road-closure times. No repeats between successive call identities or duplicate payouts.\n",fares,city,expansion);
}
