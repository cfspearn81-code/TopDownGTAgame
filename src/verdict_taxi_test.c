/* Staged streets, ordinary keyboard flow. Cold-start retail ELF and PE tapes
 * separately cover both full drives without moving the player in test code. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <string.h>
static void keytap(U key){tick(key);tick(0);}
static void idle(int n){while(n--)tick(0);}
static void reset_street(void){
 init_city();starter_save();spawnCars();spawnPlayer();end_job();vd_reset();
 scene=1;menu=-1;mapOn=0;clear_input();bank_ui_reset();
 sv.intro=4;sv.owned=1;sv.cash=140;sv.dirty=0;sv.legal=1;sv.taxiFares=0;
 p.hp=100;p.x=loc[0].x;p.y=loc[0].y;p.car=-1;
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900};
}
static void browse(int id){
 assert(vdActive<0);keytap(ACTIVITIES);assert(vdPanel==1);
 while(vdSelected!=id)keytap(DOWN);
 keytap(ENTER);
}
static void seat(int k,int index){
 p.car=k;cars[k]=(Car){job.xs[index],job.ys[index],0,0,0,2,100};
 p.x=cars[k].x;p.y=cars[k].y;
}
static void get_rider(void){
 int cab=jobRide;assert(cab>=0&&job.kind==JOB_TAXI);
 seat(cab,0);idle(80);assert(taxi.phase==TAXI_RIDING&&p.car==cab&&job.step==1);
}
static void driveby(void){
 assert(p.car>=0);
 cars[p.car].x=taxi_copper_x();cars[p.car].y=taxi_copper_y()+13;cars[p.car].s=22;
 p.x=cars[p.car].x;p.y=cars[p.car].y;tick(0);
}
int main(void){
 reset_street();nav_slot();navHasManual=1;navManualLoc=10;navManual=nav_door(10);
 navSelected=10;navCursor=navManual;nav_update();assert(navGoalKind==3);
 U seed=taxiSeed;int last=lastTaxiPickup,oldDist=navDist,oldNode=navCount;
 NavPoint pin=navManual,origin=navOrigin,goal=navGoal,nodes[LEN(navNodes)];
 cp(nodes,navNodes,sizeof nodes);
 browse(VA_B1);assert(vdPanel==2&&job.kind<0);
 assert(strstr(vdOptions[0],"APARTMENT 4")&&strstr(vdOptions[1],"FERRY MARKET")&&strstr(vdOptions[0],"trip total"));
 assert(strstr(vdOptions[0],"$141")&&strstr(vdOptions[1],"$86")&&strstr(vdOptions[0],"+$20"));
 keytap(DOWN);keytap(ESC);
 assert(vdActive<0&&job.kind<0&&sv.cash==140&&sv.taxiFares==0&&taxiSeed==seed&&lastTaxiPickup==last);
 assert(navHasManual&&navManualLoc==10&&navSelected==10&&nav_same(navManual,pin)&&nav_same(navGoal,goal));
 assert(nav_same(navOrigin,origin)&&navDist==oldDist&&navCount==oldNode&&!memcmp(nodes,navNodes,sizeof nodes));
 browse(VA_B1);keytap(DOWN);keytap(ENTER);assert(taxi.rider==4&&taxi.dropoff==13&&taxi.copper==CAB_NONE);
 get_rider();seat(p.car,1);idle(90);assert(job.kind<0&&sv.cash==140+86+22&&sv.taxiFares==1);
 puts("Cab Rank: compare, decline without touching nav state or the random fare generator; second call paid $86 + $22.");
 reset_street();browse(VA_A3);get_rider();int base=job.price;assert(base==141);
 int cash=sv.cash;keytap(TAXI_NEXT);assert(taxi_via_copper());
 navHasManual=1;navManualLoc=8;navManual=nav_door(8);nav_update();assert(navGoalKind==3);
 assert(!strcmp(taxi_route_name(),"COPPER PLAYHOUSE")&&nav_job_goal().x==taxi_copper_x());
 driveby();assert(taxi.copper==CAB_SEEN&&sv.cash==cash);driveby();driveby();assert(taxi_detour_bonus()==20);
 assert(navHasManual&&navManualLoc==8&&nav_job_goal().x==job.xs[1]&&!strcmp(taxi_route_name(),"VANTA POLICE HQ"));
 seat(p.car,1);idle(100);assert(job.kind<0&&sv.cash==cash+base+37+20&&sv.taxiFares==1&&sv.dirty==0);
 idle(120);assert(sv.cash==cash+198&&sv.taxiFares==1);
 keytap(TAXI_NEXT);assert(job.kind==JOB_TAXI&&taxi.copper==CAB_NONE);end_job();
 puts("Copper: 3 passes, one deferred $20 bonus, kept the pin; following T fare has no detour or carry-over.");
 reset_street();browse(VA_A3);get_rider();keytap(TAXI_NEXT);keytap(TAXI_NEXT);assert(taxi.copper==CAB_DIRECT);
 driveby();seat(p.car,1);idle(90);assert(sv.cash==318&&sv.taxiFares==1);
 reset_street();browse(VA_A3);get_rider();keytap(TAXI_NEXT);seat(p.car,1);idle(90);assert(sv.cash==318&&sv.taxiFares==1);
 reset_street();browse(VA_A3);get_rider();keytap(ENTER);assert(taxi.copper==CAB_DIRECT);driveby();seat(p.car,1);idle(90);
 assert(sv.cash==318&&sv.taxiFares==1);
 reset_street();browse(VA_A3);get_rider();keytap(TAXI_NEXT);driveby();assert(taxi.copper==CAB_SEEN);
 cars[p.car].s=0;keytap(USE);assert(p.car<0&&job.kind<0&&sv.cash==140&&sv.taxiFares==0);
 reset_street();browse(VA_A3);get_rider();keytap(TAXI_NEXT);driveby();job.time=.001f;idle(2);assert(job.kind<0&&sv.cash==140&&sv.taxiFares==0);
 puts("Straight, detour skip and early return keep $141+$37. Exit or deadline after seeing marquee pays $0.");
 reset_street();browse(VA_A3);get_rider();float a=cars[p.car].a;
 for(int n=0;n<8;n++){taxi_quality(1.f/60,0,a,1);idle(4);}
 assert(taxi.crashes==1&&taxi.damage==8);idle(25);taxi_quality(1.f/60,0,a,7);
 assert(taxi.crashes==2&&taxi.damage==15);
 p.x+=36;taxi_quality(1.f/60,0,a,2);assert(taxi.crashes==3&&taxi.damage==17);
 assert(sizeof(Save)==304&&SAVE_REV==10);
 puts("Contact scoring: one localized contact, actual accumulated damage, fresh hits after separation. Save stays Rev10 / 304 bytes.");
}
