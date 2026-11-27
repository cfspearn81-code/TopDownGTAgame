/* Open Contact composition tests: normal keys, not replacement UI callbacks. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void key(U k){tick(k);tick(0);}
static void scene_at(float x,float y){
 init_city();starter_save();sv.intro=4;sv.cash=500;sv.dayClock=730;
 scene=1;menu=atmOpen=-1;mapOn=0;helpBack=0;
 end_job();npc_reset();vd_reset();ng_reset();ncr_reset();harbor_boats_reset();
 p=(Player){.x=x,.y=y,.a=0,.hp=100,.car=-1};
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=3000};
 for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};
 nc=1;outCrowdCount=0;clear_input();
 assert(street_foot_clear(x,y,-1));
}
static void local_passer(void){
 static const float angle[]={0,1.5707963f,3.1415926f,4.712389f};
 for(int t=0;t<LEN(angle);t++){
  float a=angle[t],x=p.x+cs(a)*19,y=p.y+sn(a)*19;
  if(street_foot_clear(x,y,-1)&&npc_clear(p.x,p.y,x,y)){
   peds[0]=(Ped){.x=x,.y=y,.a=a,.hp=3,.cool=1000};return;
  }
 }
 assert(0&&"fixture needs a nearby, reachable passer");
}
static void shade_and_cinema(void){
 scene_at(OC_GEAR_X,OC_GEAR_Y);local_passer();
 NpcRef r;assert(npc_near_ref(&r,0)&&r.id==N_ACT_STREET&&sg_clear());
 key(HELP);assert(scene==2&&helpBack==1);paint();
 int binds=0;for(int i=0;i<LEN(guideStreet);i++)binds|=!strcmp(guideStreet[i].key,"SHIFT + E");
 assert(binds);key(ESC);assert(scene==1);
 key(USE|BRAKE);assert(npc_holds(N_ACT_STREET)&&npc_gate_street_use()&&!sg_clear());
 key(ESC);assert(!npc_gate_street_use()&&sg_clear());
 npcEnc=(NpcEncounter){.actor=r,.phase=NPC_RECOVER,.ticks=90,.room=1};
 key(ENTER);assert(scene==1&&npcEnc.phase==NPC_RECOVER&&!sg_clear());
 for(int i=0;i<90;i++)tick(0);
 assert(sg_clear());
 key(ENTER);assert(scene==OPEN_CITY_GEAR_SCENE&&!npc_gate_street_use());
 key(ENTER);key(ENTER);assert(sv.cash==410&&night_gear_mask());
 key(ESC);assert(scene==1);
 const int cinema=13;const int cx=ordinaryDoorX[cinema],cy=ordinaryDoorY[cinema];
 scene_at(cx,cy);local_passer();
 assert(npc_near_ref(&r,0)&&r.id==N_ACT_STREET);
 assert(street_use_target(placeNear(),carNear()).kind==STREET_USE_SHOP);
 key(USE);assert(scene==4&&walkin_active==cinema&&!npc_gate_street_use());
 puts("PASS Shade Enter with nearby passer, explicit Shift+E, active-contact guard, Field Guide resume, unchanged main Cinema E.");
}
static void ranger(void){
 const OuterPoint at=woodline_sites[WOOD_RANGER];
 scene_at(at.x,at.y);NpcRef r;
 assert(npc_near_ref(&r,0)&&r.id==N_ACT_OUT_RANGER);
 key(USE|BRAKE);assert(npc_holds(N_ACT_OUT_RANGER)&&scene==1);key(ESC);
 key(USE);assert(scene==NIGHT_FOREST_SCENE);key(ESC);assert(scene==1);
 puts("PASS both advertised controls at a single Woodline position: Shift+E talks to Rowan; E opens the desk.");
}
static void signal(void){
 scene_at(OC_GEAR_X,OC_GEAR_Y);
 key(PHONE);assert(scene==NIGHT_PHONE_SCENE&&np_app==NP_INBOX);
 key(LEFT);assert(np_app==NP_PREF);key(UP);assert(np_row==4);
 key(ENTER);assert(navManualLoc==NAV_GEAR);key(ESC);assert(scene==1);
 key(ENTER);key(ENTER);key(ENTER);assert(night_gear_mask());key(ESC);
 /* Fixture sets previously accepted fictional commission and actor recovery
  * timer only. An actual E performs the two handoff attempts. */
 const int j=1,id=N_ACT_OUT_SIGNAL+j-1;
 nv.phone[NP_JOB]=j+1;nv.phone[NP_BAG]=np_brief[j].qty;
 int stop=np_job_dest();p.x=loc[stop].x;p.y=loc[stop].y;
 assert(street_foot_clear(p.x,p.y,-1));
 int paid=sv.dirty,stock=np_get(NP_BAG,99),heat=night_phone_heat_at(p.x,p.y);
 streetExtraHealth[id].ticks=7;
 key(USE);assert(np_job()==j&&sv.dirty==paid&&np_get(NP_BAG,99)==stock&&sg_left()==0);
 while(street_extra_down(id))tick(0);
 key(USE);assert(np_job()<0&&sv.dirty==paid+np_brief[j].pay&&!np_get(NP_BAG,99)&&sg_left()>0);
 assert(night_phone_heat_at(p.x,p.y)==heat);
 key(USE);assert(sv.dirty==paid+np_brief[j].pay);
 puts("PASS live Signal recovery and one payout: blocked while recovering, successful House handoff and exactly one bounded Witness charge after recovery.");
}
int main(void){shade_and_cinema();ranger();signal();return 0;}
