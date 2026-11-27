/* Physical fixture catalog, rotated chassis, normal impact/durability,
 * old-career recovery, and old-crossing civilian detours. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
static Ped walker(float x,float y){return (Ped){.x=x,.y=y,.hp=100};}
static void street_lamp_empty_fixture(void){
 scene=1;menu=-1;mapOn=fastGfx=0;sv.intro=4;sv.dayClock=630;sv.owned=1;sv.cash=274;sv.hp=100;
 nc=2;cars[1].x=-200;cars[1].y=-200;end_job();vd_reset();
 z(streetDrivers,sizeof streetDrivers);z(peds,sizeof peds);z(streetWalks,sizeof streetWalks);
 clear_input();p=(Player){.car=-1,.hp=100};
}
static void foot_contact(StreetLamp s,int c){
 street_lamp_empty_fixture();cars[0].x=285;cars[0].y=528;p.x=s.x-16;p.y=s.y;
 nv.garage[60]=(unsigned)c;float begin=p.x;
 for(int i=0;i<100;i++){tick(RIGHT);assert(!street_lamp_foot(p.x,p.y,10));}
 assert(p.x>begin+10&&p.x>s.x-5&&p.x<s.x-3.7f);
 for(int i=0;i<17;i++)tick(DOWN);
 for(int i=0;i<100;i++)tick(RIGHT);
 assert(p.x>s.x+16&&p.y>s.y+4); /* no huge gravity/lighting collision radius */
}
static void driven_hit(StreetLamp s,int c){
 street_lamp_empty_fixture();nv.garage[60]=(unsigned)c;lampBumpRide=-1;
 cars[0]=(Car){s.x-24.f,s.y,0,170,0,1,100};p.x=cars[0].x;p.y=cars[0].y;p.car=0;
 int hurt=0;for(int i=0;i<170;i++){tick(UP);assert(!street_hits_wall(cars));
  if(cars[0].cond<100&&!hurt)hurt=cars[0].cond;}
 assert(hurt>0&&hurt<100&&cars[0].cond==hurt&&lampBumpRide==0);
 assert(cars[0].x>s.x-22&&cars[0].x<s.x-16.8f); /* nose, not center, reached the pole */
 /* Leave the contact area and do a genuinely new approach. */
 for(int i=0;i<58;i++)tick(DOWN);
 assert(cars[0].x<s.x-31&&lampBumpRide<0);
 for(int i=0;i<88;i++)tick(UP);
 assert(cars[0].cond<hurt);
 /* A fast diagonal sweep must not allow the rotated front corner through. */
 cars[0]=(Car){s.x-15.f,s.y-15.f,.785398f,0,0,1,100};
 assert(!street_hits_wall(cars));street_drive_car(0,4.1f,4.1f);
 assert(!street_hits_wall(cars)&&d2(cars[0].x-s.x,cars[0].y-s.y)>290.f);
}
static void civilian_bend(StreetLamp s){
 street_lamp_empty_fixture();cars[0].x=285;cars[0].y=528;
 Ped*q=peds;StreetWalk*w=streetWalks;*q=walker(s.x-9,s.y);
 *w=(StreetWalk){.leg=STREET_EN_ROUTE,.mx=s.x+42,.my=s.y,.tx=s.x+42,.ty=s.y,.goal=3};
 int took=0;for(int i=0;i<240&&q->x<s.x+30;i++){
  street_tick_civilian(0,1.f/60);assert(!street_lamp_foot(q->x,q->y,4));
  took|=w->yield==STREET_YIELD_LAMP_SIDE;
 }
 assert(took&&q->x>s.x+30&&absf(q->y-s.y)<5.f);
}
int main(void){
 init_city();
 int north=0;StreetLamp s;
 for(int i=OUT_NP_PASS;i<LEN(outerNodes);i++)if(street_lamp_outer(i,&s)){
  assert(street_lamp_clear(&s));
  north++;
 }
 assert(north==4&&streetLampFootCount-north==107);
 for(int i=1;i<=streetLampFootCount;i++){
  StreetLampFoot f=streetLampFoot[i];
  assert(solid(f.x,f.y,0)&&street_lamp_foot(f.x+1.f,f.y,0));
  assert(!street_lamp_foot(f.x+4.1f,f.y,10)&&!street_lamp_foot(f.x,f.y+5,10));
  Car a={f.x,f.y,.63f,0,0,1,100};
  assert(street_lamp_car(&a,cs(a.a),sn(a.a),18,16)>0);
 }
 StreetLamp primary,secondary;assert(street_lamp_legacy(2,2,0,&primary)&&street_lamp_legacy(2,2,1,&secondary));
 /* No pole is a broad wall for the separate chase sightline. */
 ChMount m={primary.x+17.f,primary.y,0,39,38,4,.303f,0,0};
 assert(ch_clearance(m,1,0)==m.dist);
 street_lamp_empty_fixture();cars[0].x=285;cars[0].y=528;
 sv.x=primary.x;sv.y=primary.y;unsigned old=savehash();spawnPlayer();
 assert(d2(p.x-sv.x,p.y-sv.y)<20.f*20.f&&street_foot_clear(p.x,p.y,-1)&&old==savehash()&&p.hp==100);
 foot_contact(primary,0);foot_contact(primary,1);driven_hit(primary,0);driven_hit(primary,1);civilian_bend(secondary);
 printf("PASS: all %d fixture bases (%d new Northpoint), small-foot contact (overhead/C), rotated car sweep and one damage event per impact, civilian bypass, old-save rescue and unblocked chase boom.\n",streetLampFootCount,north);
}
