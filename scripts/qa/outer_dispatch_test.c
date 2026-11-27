/* Hold a sustained incident at a physical Outer Loop curb. Synthetic world
   only, no teleporting or spawning response units. No ambient vehicle blocks. */
#define DC_TEST
#include <game.c>
#include <stdio.h>
#include <assert.h>
#ifdef DC_AUDIT_DISPATCH
#include <string.h>
static void live_traffic(void){
 starter_save();sv.intro=4;
 for(int shift=0;shift<2;shift++)for(int site=0;site<6;site++){
  clear_input();npc_reset();vd_reset();end_job();spawnCars();dispatch_reset();outer_life_reset();
  scene=1;sv.dayClock=shift?17*60.f:11*60.f;menu=-1;mapOn=0;
  OuterPoint pt=outerNodes[outerStops[site].node];p=(Player){.x=pt.x,.y=pt.y,.car=-1,.hp=100,.heat=150.f};
  float best=1e8f;int first=-1,penetrate=0;unsigned arrivals=0;
  for(int k=0;k<9000;k++){
   anim+=1.f/60;outer_life_tick(1.f/60);street_tick_traffic(1.f/60);
   npcTick(1.f/60);p.hp=100;
   if(first<0&&dispatchSaw)first=k;
   for(int j=0;j<DISPATCH_UNITS;j++){Ped*q=peds+DISPATCH_FIRST+j;if(q->hp<=0)continue;
    float f=d2(q->x-p.x,q->y-p.y);if(f<best)best=f;
    if(f<177*177&&street_line_open(q->x,q->y,p.x,p.y,2))arrivals|=1u<<j;
    penetrate+=solid(q->x,q->y,3);
   }
  }
  assert(first>=0&&arrivals&&penetrate==0);printf("shift=%d %s first=%d nearest=%.1f seenMask=%x wall=%d\n",shift,outerStops[site].label,first,sq(best),arrivals,penetrate);fflush(stdout);
 }
}

static void extras(void){
 int passes=0,closed=0;
 /* All planned street legs are physical feet sweeps, not snapped positions. */
 for(int shift=0;shift<2;shift++)for(int i=0;i<LEN(outerStops);i++)for(int j=0;j<LEN(outerStops);j++)if(i!=j){
  sv.dayClock=shift?17*60.f:11*60.f;OuterPoint a=outerNodes[outerStops[i].node],b=outerNodes[outerStops[j].node];
  DispatchPoint route[D_ROUTE_CAP];navNodes[0]=(NavPoint){1911,611};navNodes[1]=(NavPoint){1999,560};navCount=2;navDist=15011;
  int n=dispatch_regional_plan((DispatchPoint){a.x,a.y},(DispatchPoint){b.x,b.y},route,D_ROUTE_CAP);
  assert(navCount==2&&navDist==15011&&navNodes[0].x==1911&&navNodes[1].y==560&&n>1&&n<=D_ROUTE_CAP);
  for(int k=1;k<n;k++)assert(dispatch_region_clear(route[k-1],route[k]));
  passes++;closed+=!!np_closed();
 }
 assert(closed==30&&passes==60);
 /* Keep one real assigned unit, then end the call and see it return home. */
 dispatch_reset();sv.dayClock=660;scene=1;streetWalks[DISPATCH_FIRST].pause=0;
 p=(Player){.x=2819.34f,.y=1025.64f,.heat=90,.hp=100,.car=-1};
 /* These stopped chassis were observed in the actual Parkway keyboard run. */
 cars[0]=(Car){2217.4f,1033.f,-.4f,0,0,0,100};cars[1]=(Car){2201,1072,-1.6f,0,0,0,100};nc=2;
 for(int i=1;i<DISPATCH_UNITS;i++)peds[DISPATCH_FIRST+i].hp=0;
 int first=-1,post=-1;
 for(int k=0;k<22000;k++){
  if(first>=0)p.heat=0;
  dispatch_tick(1.f/60.f);if(first<0&&dispatchSaw)first=k;
  if(first>=0&&dispatchUnits[0].state==D_POST&&!dispatchRoute[0].used){post=k;break;}
 }
 assert(first>0&&post>first&&d2(peds[DISPATCH_FIRST].x-dispatchUnits[0].home.x,peds[DISPATCH_FIRST].y-dispatchUnits[0].home.y)<2.f);
 printf("PASS: %d surveyed scene pairs, %d closure routes, atlas scratch preserved; original responder contacted at %d, returned to post at %d.\n",passes,closed,first,post);
}
#endif
int main(void){
 init_city();starter_save();scene=1;menu=-1;mapOn=0;sv.intro=4;
 int missed=0;
 for(int site=0;site<LEN(outerStops);site++){
  dispatch_reset();end_job();npc_reset();clear_input();
  for(int j=0;j<LEN(cars);j++)cars[j]=(Car){.x=-100,.y=-100};nc=2;
  OuterPoint goal=outerNodes[outerStops[site].node];
  p=(Player){.x=goal.x,.y=goal.y,.hp=100,.car=-1,.heat=150.f,.quiet=8.f};
  float closest=1e8f,maxstep=0;int first=-1,stationed=0;
  for(int k=0;k<14500;k++){ /* 241.67 simulated seconds of an uninterrupted incident */
   float old[DISPATCH_UNITS][2];for(int i=0;i<DISPATCH_UNITS;i++){Ped*q=peds+DISPATCH_FIRST+i;old[i][0]=q->x;old[i][1]=q->y;}
   dispatch_tick(1.f/60.f);p.hp=100; /* survey response, not defeat/respawn */
   for(int i=0;i<DISPATCH_UNITS;i++){
    Ped*q=peds+DISPATCH_FIRST+i;float ds=d2(old[i][0]-q->x,old[i][1]-q->y);
    if(ds>maxstep)maxstep=ds;if(q->hp<=0)continue;
    float d=d2(q->x-p.x,q->y-p.y);if(d<closest)closest=d;
   }
   if(dispatchSaw&&first<0)first=k;
  }
  printf("%s sight=%d closest=%.1f largest-step=%.2f active=%d\n",outerStops[site].label,first,sq(closest),sq(maxstep),dispatch_engaged());
  missed+=first<0;assert(sq(maxstep)<4.f);
 }
 #ifdef DC_AUDIT_DISPATCH
 extras();live_traffic();
#endif
 printf("regions with no officer arrival: %d/%d\n",missed,LEN(outerStops));return !!missed;
}
