/* Deterministic city/country movement and actual dialogue input test.
 * Build: gcc -std=c11 -O2 src/street_motion_test.c -o build/street_motion_test */
#define DC_TEST
#include "game.c"
#include <assert.h>
static void cycle(void){
 init_city();starter_save();scene=1;menu=-1;mapOn=0;sv.intro=4;clear_input();npc_reset();
 sv.dayClock=570; p=(Player){.x=284,.y=666,.hp=100,.car=-1};
}
static int pros_density(void){
 int count=0,live=0;
 for(int i=0;i<STREET_CIVILIANS;i++)if(peds[i].hp>0){
  live++;assert(!solid(peds[i].x,peds[i].y,3));
  for(int j=i+1;j<STREET_CIVILIANS;j++)if(peds[j].hp>0&&d2(peds[i].x-peds[j].x,peds[i].y-peds[j].y)<11.f*11.f)count++;
 }
 assert(live==STREET_CIVILIANS);return count;
}
static void oldtown(void){
 cycle();assert(pros_density()==0);
 float dist[STREET_CIVILIANS]={0};int maxPair=0,crosses=0,moving=0;
 for(int t=0;t<14400;t++){
  float before[STREET_CIVILIANS][2];int any=0;
  for(int i=0;i<STREET_CIVILIANS;i++){before[i][0]=peds[i].x;before[i][1]=peds[i].y;}
  tick(0);
  for(int i=0;i<STREET_CIVILIANS;i++){
   Ped*q=peds+i;float delta=sq(d2(q->x-before[i][0],q->y-before[i][1]));
   assert(delta<1.55f&&q->hp>0);dist[i]+=delta;if(q->s>1)moving++;
   if(streetWalks[i].cross==1&&q->s>1)any=1;
  }
  crosses+=any;if(t%60==0)maxPair=max(maxPair,pros_density());
 }
 for(int i=0;i<STREET_CIVILIANS;i++)assert(dist[i]>256.f);
 assert(maxPair==0&&crosses>100&&moving>100000);
 printf("Old town: %d walkers each covered a block in four minutes; worst sampled co-located pairs = %d; crossings still active.\n",STREET_CIVILIANS,maxPair);
}
static void outer_walks(void){
 cycle();p.x=2800;p.y=1024;tick(0);
 assert(outer_walk_count()==24&&outCrowdCount==24);
 float walked[OUT_WALK_COUNT]={0};
 for(int t=0;t<6900;t++){
  if(t==1200)sv.dayClock=1220; /* no night crowd replacement */
  if(t==2400){p.x=600;p.y=660;}
  if(t==4200){p.x=2690;p.y=390;}
  float orig[OUT_WALK_COUNT][2];
  for(int i=0;i<OUT_WALK_COUNT;i++){orig[i][0]=outWalkers[i].x;orig[i][1]=outWalkers[i].y;}
  tick(0);
  assert(outer_walk_count()==24&&outCrowdCount==24);
  for(int i=0;i<OUT_WALK_COUNT;i++){
   float x,y,a;int step;U coat;assert(outer_walk_pose(i,&x,&y,&a,&step,&coat)&&outer_walk_clear(x,y)&&coat);
   float move=sq(d2(x-orig[i][0],y-orig[i][1]));assert(move<.54f);walked[i]+=move;
   if(t%45==0)for(int j=i+1;j<OUT_WALK_COUNT;j++)assert(d2(x-outWalkers[j].x,y-outWalkers[j].y)>=11.f*11.f-.01f);
  }
 }
 for(int i=0;i<OUT_WALK_COUNT;i++)assert(walked[i]>90.f);
 puts("Outer loop: 24 continuously simulated identities; legal shoulder positions, spacing and gradual foot movement across time-of-day and viewport changes.");
}
static void wait_frame(void){
 cycle();
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
 for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};
 p=(Player){.x=1024,.y=1024,.hp=100,.car=-1};nc=2;job.kind=-1;
 peds[0]=(Ped){.x=1054,.y=1024,.s=28,.hp=3};
 peds[1]=(Ped){.x=1050,.y=1054,.s=28,.hp=3};
 streetWalks[0]=(StreetWalk){.mx=1120,.my=1024,.leg=STREET_LEAVE};
 streetWalks[1]=(StreetWalk){.mx=1024,.my=1054,.leg=STREET_LEAVE};
 tick(USE);assert(npc_holds(N_ACT_STREET)&&npcTalk.id==N_ACT_STREET);
 float x=peds[0].x,y=peds[0].y,neighbor=peds[1].x,px=p.x,py=p.y;
 for(int i=0;i<38;i++)tick(UP);
 assert(p.x==px&&p.y==py&&peds[0].x==x&&peds[0].y==y&&absf(peds[1].x-neighbor)>10.f);
 tick(ESC);
 assert(!npc_holds(N_ACT_STREET));for(int i=0;i<15;i++)tick(RIGHT);
 assert(p.x>px+12&&peds[0].x>x+2.f);
 puts("Held input: selected dialogue actor stays exactly fixed, another pedestrian continues, and Esc resumes the same NPC trajectory.");
}
static void parked_car(void){
 cycle();for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
 for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};
 nc=2;z(streetDrivers,sizeof streetDrivers);
 peds[0]=(Ped){.x=333,.y=550,.a=0,.hp=2};
 cars[1]=(Car){.x=355,.y=550,.a=0,.cond=100};
 streetWalks[0]=(StreetWalk){.mx=474,.my=550,.fromX=294,.fromY=550,.tx=474,.ty=550,.ax=474,.ay=550,.leg=STREET_EN_ROUTE};
 int replanned=0,started=0;
 for(int i=0;i<950;i++){
  tick(0);assert(street_foot_car_clear(peds[0].x,peds[0].y,-1,3.f));assert(!solid(peds[0].x,peds[0].y,3.f));
  if(streetWalks[0].detour)started=1;
  if(streetWalks[0].visits&&d2(peds[0].x-333.f,peds[0].y-550.f)>42.f*42.f)replanned=1;
 }
 assert(started&&replanned);
 puts("Blocked sidewalk: waited, backed to safe curb and took a different route around a real parked car.");
}
int main(void){oldtown();outer_walks();wait_frame();parked_car();return 0;}
