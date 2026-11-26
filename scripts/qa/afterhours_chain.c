/* Cross-feature in-engine input playtest in an isolated Linux career.
 * Fixture steps position the player near verified entrances / an open street
 * and start with a registered Mote + credits. Selections, driving, deliveries
 * and business transactions below use the same tick path as SDL. */
#define _POSIX_C_SOURCE 200809L
#define DC_LINUX_NATIVE
#include "../../src/game.c"
#include "../../src/linux_save.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void hit(U button){tick(button);tick(0);}
static void hits(U button,int n){while(n--)hit(button);}
static void frames(U keys,int n){while(n--)tick(keys);tick(0);}
static void street_fixture(float x,float y){p.car=-1;p.x=x;p.y=y;menu=-1;mapOn=0;scene=1;clear_input();tick(0);}
static void phone(int app,int row){
 assert(scene!=NIGHT_PHONE_SCENE);hit(PHONE);assert(scene==NIGHT_PHONE_SCENE);
 for(int i=0;i<NP_NAPP&&np_app!=app;i++)hit(RIGHT);
 assert(np_app==app);
 for(int i=0;i<16&&np_row!=row;i++)hit(DOWN);
 assert(np_row==row);
}
static void courtyard_position(float x,float y){
 for(int i=0;i<500&&absf(ncX-x)>2.6f;i++)tick(ncX>x?LEFT:RIGHT);
 for(int i=0;i<500&&absf(ncY-y)>2.6f;i++)tick(ncY>y?UP:DOWN);
 tick(0);
 if(absf(ncX-x)>=4.f||absf(ncY-y)>=4.f)fprintf(stderr,"Club fixture movement: scene %d room %d from %.1f,%.1f to %.1f,%.1f target %d\n",scene,ncRoom,ncX,ncY,x,y,nc_target());
 assert(absf(ncX-x)<4.f&&absf(ncY-y)<4.f);
}
static void reach_office(void){
 street_fixture(ordinaryDoorX[NC_ID],ordinaryDoorY[NC_ID]);
 hit(USE);assert(scene==4&&walkin_active==NC_ID);hit(ENTER);
 assert(scene==NIGHT_CLUB_SCENE&&ncRoom==NC_FLOOR);
 courtyard_position(164,361);
 hit(USE);assert(ncRoom==NC_OFFICE);
 courtyard_position(480,267);
 hit(USE);assert(ncPanel==NC_DESK);
}
static void reach_garage(void){
 street_fixture(loc[3].x,loc[3].y);
 hit(USE);
 assert(scene==3&&interior_active_loc==3);
 /* Skips the already-regression-tested indoor walk to the service desk. */
 interior_x=interior_desk_x;interior_y=interior_desk_y;
 hit(USE);assert(menu==3);
 hits(DOWN,6);assert(opt[sel].action==184);
 hit(ENTER);assert(scene==NIGHT_GARAGE_SCENE);
}
int main(int argc,char **argv){
 assert(argc==2&&setenv("XDG_DATA_HOME",argv[1],1)==0&&linux_disk_init()&&linux_disk_lock());
 init_city();
 /* No private save: start a dedicated fixture with enough seed capital. */
 starter_save();sv.cash=4095;sv.intro=4;sv.owned=1;sv.selection=0;
 sv.dayClock=250.f;spawnCars();spawnPlayer();
 street_fixture(loc[4].x,loc[4].y);

 phone(NP_LOFT,0);hits(ENTER,2);assert(sv.b[0].own);
 hits(DOWN,3);hit(ENTER);assert(np_active_batch()==0&&np_active_left()>0);
 hit(ESC);frames(0,1100);assert(np_active_left()==0&&night_world_weather()==NIGHT_RAIN&&night_world_is_night());
 phone(NP_LOFT,4);hit(ENTER);assert(np_get(NP_SHELF,99)==3);
 hit(RIGHT);assert(np_app==NP_STOCK);hit(ENTER);assert(np_get(NP_BAG,99)==3);
 hit(ESC);

 phone(NP_INBOX,1);hit(ENTER);assert(np_job()==1&&np_job_dest()==12);
 hit(ESC);
 reach_garage();
 hit(ENTER);assert(nv.garage[0]==1);
 hit(DOWN);hit(ENTER);assert(ng.tab==NG_BUILD);
 hits(DOWN,3);hit(RIGHT);hit(ENTER);
 assert(nv.garage[4]==1&&ng_installed(0).tune==1);
 hit(ESC);if(scene==NIGHT_GARAGE_SCENE)hit(ESC);
 assert(scene==3);

 /* Test-yard fixture is one clear road segment from the actual apartment.
  * E boards the customized Mote; fixed-step controls move it in the rain. */
 scene=1;menu=-1;
 /* Move the two pre-existing local traffic actors out of the isolated
  * test track so a traffic jam cannot mask weather/handling differences. */
 for(int i=1;i<nc;i++)if(absf(cars[i].x-256)<100&&cars[i].y>538&&cars[i].y<868)
  street_stage_car(i,(Car){.x=-100,.y=-100});
 street_stage_car(0,(Car){.x=256,.y=750,.a=-1.5707963f,.type=0,.owner=1,.cond=100});
 street_fixture(278,750);
 hit(USE);assert(p.car==0&&ng_power(cars)==1);
 hit(CAMERA);assert(ch_pref());paint();assert(chProjected);
 float initial= p.y;
 frames(UP,34);assert(p.y<initial-25&&night_world_brake()<.94f&&ngCamZoom>0.f);
 frames(BRAKE,65);
 assert(p.y<initial-47&&cars[0].cond>0);
 hit(USE);assert(p.car<0);
 int attempts=0;
 while(d2(p.x-loc[12].x,p.y-loc[12].y)>=42*42&&attempts++<90){
   float dx=loc[12].x-p.x,dy=loc[12].y-p.y;
   U walk=absf(dx)>4.f?(dx>0?RIGHT:LEFT):0;
   if(absf(dx)<35.f&&absf(dy)>4.f)walk|=dy>0?DOWN:UP;
   tick(walk);
 }
 tick(0);assert(d2(p.x-loc[12].x,p.y-loc[12].y)<43*43);
 hit(USE);
 assert(np_job()<0&&np_get(NP_BAG,99)==1&&sv.dirty==154&&nv.phone[NP_DONE]==1);

 reach_office();
 hit(ENTER);assert(nv.club[NC_OWN]==1&&sv.cash<95);
 hits(DOWN,5);hit(ENTER);assert(nv.club[NC_DECOR]==0); /* Not affordable without the new income. */
 hits(UP,4);hits(ENTER,2);assert(nv.club[NC_COVER]==0);
 hits(DOWN,5);hit(ENTER);assert(sv.dirty==114&&nv.club[NC_EXCHANGE]==30);
 frames(0,NC_SHOW+1);assert(nv.club[NC_TILL]>30&&nv.club[NC_EXCHANGE]==0);
 assert(sv.cash+nv.club[NC_LAST]<95&&sv.cash+nv.club[NC_TILL]>=95);
 hits(UP,6);hit(ENTER);assert(nv.club[NC_TILL]==0&&sv.cash>=95);
 hits(DOWN,5);hit(ENTER);assert(nv.club[NC_DECOR]==1);
 int balance=sv.cash;
 request_save();assert(saveFlash>0);
 Save11 artifact;assert(disk_read(&artifact,sizeof artifact)&&artifact.base.rev==11);
 starter_save();load_save();
 assert(wason&&sv.cash==balance&&sv.dirty==114&&sv.b[0].own&&ch_pref());
 assert(nv.phone[NP_DONE]==1&&nv.phone[NP_BAG]==1&&nv.garage[4]==1&&nv.club[NC_DECOR]==1);
 linux_disk_shutdown();
 puts("Afterhours integrated playtest: Lantern -> matching edition -> Signal request -> Motor Works tune -> 3D chase and real wet-night driving and handoff -> decisive box-office credit -> mirror installation -> Rev11 reload.");
 return 0;
}
