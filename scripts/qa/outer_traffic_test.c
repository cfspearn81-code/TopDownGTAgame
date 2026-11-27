/* Seven regional cars use the same body, E/heat, parking and collision code
 * as the city. Synthetic state only, no player career writes. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <string.h>
static void seed(void){
 init_city();starter_save();scene=1;menu=-1;mapOn=0;end_job();clear_input();vd_reset();
 sv.intro=4;sv.dayClock=600;p=(Player){.x=2080,.y=900,.car=-1,.hp=100};
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
}
static void tick_lane(int frames){while(frames--)outer_life_tick(1/60.f);}
static void all_lanes(void){
 int bodies=0;
 for(int phase=0;phase<2;phase++){
  seed();sv.dayClock=phase?404:600;int stock=nc;
  Car cityBefore[STREET_CITY_CAR_LIMIT];cp(cityBefore,cars,sizeof cityBefore);
  tick_lane(1);float moved[OUT_TRAFFIC_COUNT]={0};
  for(int frame=0;frame<10800;frame++){
   Car old[OUT_TRAFFIC_COUNT];cp(old,cars+STREET_CITY_CAR_LIMIT,sizeof old);
   anim+=1/60.f;tick_lane(1);
   for(int i=0;i<OUT_TRAFFIC_COUNT;i++){
    int id=STREET_CITY_CAR_LIMIT+i;Car*c=cars+id;
    float delta=sq(d2(old[i].x-c->x,old[i].y-c->y));moved[i]+=delta;
    assert(street_car_exists(id)&&!street_hits_wall(c)&&delta<.80f);
    if(phase)assert(!night_phone_lane_stop(old[i].x,old[i].y,c->x,c->y,17));
    for(int j=street_next_car(-1);j<LEN(cars);j=street_next_car(j))
     if(j!=id)assert(!street_overlap(cars+j,c));
    bodies++;
   }
  }
  for(int i=0;i<OUT_TRAFFIC_COUNT;i++)assert(moved[i]>=4000.f);
  assert(nc==stock&&!memcmp(cityBefore,cars,sizeof cityBefore));
 }
 printf("Outer traffic: %d collision/road/smooth-step samples, all seven cars moved over four km per simulation, open highway and detour.\n",bodies);
}
static void parkway_yield(void){
 seed();const int id=STREET_CITY_CAR_LIMIT;
 outer_life_tick(1/60.f);
 /* A straight approach from the west in the eastbound lane. */
 OutLane*l=outLane;outDriver[0]=(OutDriver){.route=0,.next=1};
 cars[id]=(Car){2380,1033,0,39,1,0,100};
 p.x=2415;p.y=1033;int cityCount=nc;
 tick_lane(180);assert(street_foot_clear(p.x,p.y,-1)&&cars[id].s<.01f);
 float stopped=cars[id].x;assert(stopped<2398&&stopped>=2380);
 p.y=1057;tick_lane(90);assert(cars[id].x>stopped+30);
 /* A closure can start with a driver already approaching the barricade.
  * The car stops there, then advances normally once it opens again. */
 sv.dayClock=360;tick_lane(180);float barrier=cars[id].x;
 assert(barrier<NP_PARKWAY_X-16&&barrier>NP_PARKWAY_X-40&&cars[id].s<.01f);
 sv.dayClock=480;tick_lane(195);assert(cars[id].x>NP_PARKWAY_X+40&&nc==cityCount);
 /* Parked and wrecked chassis are obstructions, not only other NPC drivers. */
 float from=cars[id].x;
 cars[1]=(Car){from+58,1033,0,0,0,0,0};
 tick_lane(180);assert(cars[id].x>from+8&&cars[id].s<.01&&!street_overlap(cars+1,cars+id));
 float queued=cars[id].x;
 cars[1].y+=32.f;tick_lane(90);assert(cars[id].x>queued+30);
 puts("Outer traffic: driver stops/resumes for a pedestrian, closed Parkway and a parked wreck.");
}
static void board_and_park(void){
 seed();const int id=STREET_CITY_CAR_LIMIT;
 tick_lane(1);int stock=nc;int contract=nc-1;
 sv.owned=1;sv.carCond[0]=100;
 Car owned=cars[0];Car jobBefore=cars[contract];
 /* Isolate one street ride, keeping both protected slots occupied. */
 cars[id]=(Car){2280,1033,0,22,1,0,100};outDriver[0]=(OutDriver){.driven=1};
 p.x=2280;p.y=1053;p.a=-1.57f;
 assert(street_near_car()==id&&!street_foot_car_clear(2280,1033,-1,7));
 assert(street_car_fraction(2280,1000,2280,1060,1,-1)<1.f);
 Car other={2248,1033,0,118,3,1,100},next=other;next.x+=4;
 assert(street_car_clear(0,0,&other)&&!street_car_clear(0,&other,&next));
 tick(USE);assert(p.car==id&&cars[id].reported&&p.heat>=38.f&&!cars[id].owner);
 clear_input();tick(CAMERA);assert(ch_enabled());tick(0);tick(CAMERA);assert(!ch_enabled());
 for(int i=0;i<30;i++)tick(BRAKE);tick(USE);assert(p.car<0&&street_foot_clear(p.x,p.y,-1));
 float x=cars[id].x,y=cars[id].y;
 tick_lane(200);assert(cars[id].x==x&&cars[id].y==y&&street_near_car()==id);
 clear_input();tick(USE);assert(p.car==id&&cars[id].reported);
 clear_input();tick(USE);assert(p.car<0);
 assert(nc==stock&&!memcmp(&owned,cars,sizeof owned)&&!memcmp(&jobBefore,cars+contract,sizeof jobBefore));
 /* Profile/city reset never saves this transient unregistered ride. */
 outer_life_reset();assert(!street_car_exists(id)&&nc==stock);
 puts("Outer traffic: solid on foot and as cover; true E theft/dispatch, C chase, park/reboard, unchanged owned/job slots and transient cleanup.");
}
int main(void){all_lanes();parkway_yield();board_and_park();return 0;}
