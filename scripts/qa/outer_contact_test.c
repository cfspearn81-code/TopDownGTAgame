/* Compare against the shipped source with: cc -O1 -I old/src ... */
#define DC_TEST
#include <game.c>
#include <stdio.h>
#include <assert.h>
static void setup(void){
 init_city();starter_save();scene=1;menu=atmOpen=-1;mapOn=0;sv.intro=4;sv.gear=1;
 sv.dayClock=690;anim=0;wheelFists=0;clear_input();npc_reset();outer_life_reset();vd_reset();end_job();
 for(int j=0;j<LEN(peds);j++)peds[j]=(Ped){.life=3000};
 for(int j=0;j<LEN(cars);j++)cars[j]=(Car){.x=-100,.y=-100};nc=2;
 OuterPoint site=outerNodes[outerStops[1].node];p=(Player){.x=site.x,.y=site.y,.hp=100,.car=-1};
 outCrowdCount=4;
}
#ifdef DC_NEW_CONTACT
static void edges(void){
 setup();int id=N_ACT_OUTER+N_OUTER_SEATS;NpcRef rr,u;assert(npc_ref(id,&rr));
 /* A city pedestrian or a parked body takes precedence at the same edge. */
 p.x=rr.x-54;p.y=rr.y;p.a=0;peds[0]=(Ped){.x=rr.x-25,.y=rr.y,.hp=3};
 shoot();assert(peds[0].hp==1&&!outBody[id-N_ACT_OUTER].damage&&trailEndX==peds[0].x);
 peds[0].hp=0;cars[1]=(Car){rr.x-26,rr.y,1.57f,0,0,0,100};
 p.shot=0;shoot();assert(!outBody[id-N_ACT_OUTER].damage);
 cars[1].x=cars[1].y=-100;
 p.shot=0;shoot();assert(outBody[id-N_ACT_OUTER].damage==2&&npc_ref(id,&u));
 /* A lone combat hit cannot touch another person or pay a robbery prize. */
 assert(sv.dirty==0&&outBody[id+1-N_ACT_OUTER].damage==0);
 p.shot=0;shoot();assert(!npc_ref(id,&u)&&outBody[id-N_ACT_OUTER].damage==3);
 for(int k=0;k<2000;k++)outer_condition_tick(1.f/60.f);
 assert(!npc_ref(id,&u));p.x=p.y=512;cx=cy=200;outer_condition_tick(1.f/60.f);
 p.x=rr.x-12;p.y=rr.y;assert(npc_ref(id,&u));
 /* One fast chassis impact matches one person. A selected conversation
    target is protected; do not count an impact on a knocked-down body. */
 int j=0,collisions=0;
 for(int shift=0;shift<2;shift++)for(int site=0;site<6;site++){
  setup();OuterPoint at=outerNodes[outerStops[site].node];p.x=at.x;p.y=at.y;
  sv.dayClock=shift?1200:690;outCrowdCount=shift?8:4;
  for(int seat=0;seat<4;seat++){
   int index=N_ACT_OUTER+4*site+seat;NpcRef a;
   if(!npc_ref(index,&a))continue;j++;
   /* Work through a whole crowd as their feet can overlap. */
   while(outer_car_hit(a.x,a.y,1,0))collisions++;
   assert(!npc_ref(index,&a));
  }
 }
 assert(j>=12&&collisions>=30);
 setup();assert(npc_ref(id,&rr));assert(npc_start_talk(rr));
 outer_car_hit(rr.x,rr.y,1,0);assert(npc_ref(id,&u)&&!outBody[id-N_ACT_OUTER].damage);
 puts("PASS: cover, nearest city/region ordering, no cash, off-camera recovery, 12 site/shift groups, single-hit footprint, conversation hold.");
}
#endif
int main(void){
 int fail=0;for(int mode=0;mode<3;mode++){
  setup(); NpcRef r;int id=N_ACT_OUTER+N_OUTER_SEATS;assert(npc_ref(id,&r));
  float d=mode==0?21.f:mode==1?52.f:8.f;
  p.x=r.x-d;p.y=r.y;p.a=0;assert(street_reach_open(p.x,p.y,r.x,r.y,1,-1));
  for(int hit=0;hit<(mode==0?3:2);hit++){p.shot=0;if(mode==0)punch();else shoot();}
  NpcRef after;int present=npc_ref(id,&after);
  printf("mode %d actor=%s at %.0f,%.0f, standing after %s: %d, heat %.0f, trace end %.1f,%.1f\n",mode,r.name,r.x,r.y,mode==0?"3 punches":"2 amplified pulses",present,p.heat,trailEndX,trailEndY);
  fail+=present;assert(!sv.dirty);
 }
#ifdef DC_NEW_CONTACT
 edges();
#endif
 return fail!=0;
}
