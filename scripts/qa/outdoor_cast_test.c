/* Production key, route, draw and damage paths. The QA fixture supplies
   legal starting curbs and unlocked contact hours, not gameplay results. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>

static void setup(void){
 init_city();starter_save();sv.intro=4;sv.dayClock=730;scene=1;menu=atmOpen=-1;mapOn=0;
 end_job();npc_reset();vd_reset();ng_reset();ncr_reset();harbor_boats_reset();outCrowdCount=0;
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=3000};
 for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};
 nc=1;z(streetDrivers,sizeof streetDrivers);
 clear_input();
 p=(Player){.x=1000,.y=1000,.a=-1.5707963f,.car=-1,.hp=100};
}
static void key(U k){tick(k);tick(0);}
static void contact_at(NpcRef a,int want){
 for(int dist=19;dist<=32;dist+=3)for(int t=0;t<8;t++){
  float b=t*.7853982f;p.x=a.x+cs(b)*dist;p.y=a.y+sn(b)*dist;p.a=wrap(look(a.x-p.x,a.y-p.y));
  NpcRef r;
  if(street_foot_clear(p.x,p.y,-1)&&npc_near_ref(&r,0)&&(!want||r.id==a.id))return;
 }
 fprintf(stderr,"No public approach to %s at %.1f,%.1f\n",a.name,a.x,a.y);assert(0);
}
static void contact(int id,int checkDamage){
 NpcRef r;StreetPerson s;assert(npc_ref(id,&r)&&street_extra_pose(id,&s));
 assert(r.x==s.x&&r.y==s.y&&r.coat==s.coat);
 contact_at(r,1);
 key(CAMERA);assert(ch_mode_active());paint();assert(chProjected);
 key(USE|BRAKE);assert(npc_holds(id));
 for(int k=0;k<28;k++)tick(UP);
 assert(npc_ref(id,&r)&&street_extra_pose(id,&s)&&s.x==r.x&&s.y==r.y&&!s.step);
 key(ESC);assert(!npc_holds(id));key(CAMERA);assert(!ch_mode_active());paint();assert(!chProjected);
 if(checkDamage){
  assert(npc_ref(id,&r));contact_at(r,1);
  for(int i=0;i<3;i++){p.shot=0;punch();}
  assert(street_extra_down(id)&&!npc_ref(id,&r)&&!street_extra_pose(id,&s)&&street_extra_fallen(id,&s));
  int ticks=streetExtraHealth[id].ticks;
  while(ticks--)street_extra_tick();
  assert(npc_ref(id,&r)&&street_extra_pose(id,&s)&&!street_extra_fallen(id,&s));
 }
}
int main(void){
 setup();
 sv.dayClock=1250;int count=0,id=-1;
 for(int i=N_ACT_OUT_QUEUE;i<N_ACT_OUT_SIGNAL;i++){NpcRef r;
  if(npc_ref(i,&r)){if(id<0)id=i;count++;}}
 assert(count>0);contact(id,1);
 printf("%d Afterimage queue patrons: same poses in each lens, Shift+E, exact hold, direct fists and recovery.\n",count);

 NpcRef a;setup();
 assert(npc_ref(N_ACT_OUT_RANGER,&a));
 contact_at(a,1);
 const OuterPoint desk=woodline_sites[WOOD_RANGER];p.x=desk.x;p.y=desk.y;key(USE);
 assert(scene==NIGHT_FOREST_SCENE);key(ESC);assert(scene==1);
 contact(N_ACT_OUT_RANGER,1);
 contact(N_ACT_OUT_CLERK,1);

 setup();contact(N_ACT_OUT_HIKER,0);
 setup();ncr.phase=NCR_WAIT;ncr.x=988;ncr.y=1056;ncr.a=1.57f;ncr.pick=(NavPoint){988,1082};
 contact(N_ACT_OUT_VOCAL,1);

 setup();int tested=0;
 for(int j=1;j<10;j++){
  nv.phone[NP_JOB]=j+1;int id=N_ACT_OUT_SIGNAL+j-1;
  assert(npc_ref(id,&a)&&a.name==np_brief[j].name);
  contact(id,0);tested++;
 }
 printf("All %d Signal customers and the ranger, clerk, hiker and Jules respond to input.\n",tested);

 setup();sv.gear=1;nv.phone[NP_JOB]=3; int j=2;
 nv.phone[NP_BAG+np_brief[j].id]=np_brief[j].qty;
 assert(npc_ref(N_ACT_OUT_SIGNAL+j-1,&a));contact_at(a,1);
 float aim=look(a.x-p.x,a.y-p.y);p.a=aim;p.shot=0;shoot();
 assert(streetExtraHealth[a.id].damage==2);
 int stop=np_job_dest();p.x=loc[stop].x;p.y=loc[stop].y;p.heat=0;
 int paid=sv.dirty;key(USE);
 assert(np_job()<0&&sv.dirty>paid);
 int once=sv.dirty;key(USE);assert(sv.dirty==once);
 puts("Direct pulse hits the real Signal customer; normal E completes the original handoff and pays exactly once.");
 setup();sv.owned=1;nc=2;
 cars[0]=(Car){.x=314,.y=728,.owner=1,.cond=100};p.x=cars[0].x;p.y=cars[0].y;p.car=0;
 beginJob(JOB_TAXI);assert(job.kind==JOB_TAXI&&taxi.phase==TAXI_WAIT);
 p.car=-1;int fare=N_ACT_RIDER+taxi.rider;
 assert(npc_ref(fare,&a));contact_at(a,1);
 for(int i=0;i<3;i++){p.shot=0;punch();}
 assert(street_extra_down(fare)&&!npc_ref(fare,&a));
 p.car=0;cars[0].x=p.x=job.xs[0];cars[0].y=p.y=job.ys[0];cars[0].s=0;
 int money=sv.cash;
 for(int i=0;i<30;i++){taxi_tick(1.f/60.f);street_extra_tick();}
 assert(taxi.phase==TAXI_WAIT&&sv.cash==money);
 while(street_extra_down(fare))street_extra_tick();
 for(int i=0;i<170&&taxi.phase!=TAXI_RIDING;i++)taxi_tick(1.f/60.f);
 assert(taxi.phase==TAXI_RIDING&&job.step==1&&sv.cash==money);
 puts("Taxi can't board or pay while its passenger is down; after recovery, the original ride starts without double pay.");

 setup();assert(vd_launch(VA_B7));nc=2;cars[1]=(Car){.x=944,.y=1262,.owner=2,.cond=100};vm.newsPhase=2;vm.van=1;vm.phaseT=120;
 for(int event=0;event<2;event++){
  vm.event=event;
  int first=event?N_ACT_OUT_MEDIA+2:N_ACT_OUT_MEDIA,n=event?1:2;
  for(int i=first;i<first+n;i++)contact(i,0);
 }
 puts("Three live camera subjects are also matched to physical dialogue and pose IDs.");
 return 0;
}
