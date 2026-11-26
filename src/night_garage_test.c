/* Shared-module verification. Build at root: gcc -std=c11 -O2
 * src/night_garage_test.c -o build/night_garage_test && build/night_garage_test
 * Optional first argument: an existing directory for frames of actual input. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <string.h>
static const char*dir;
static void frame(const char*id){
 if(!dir)return;char path[768];snprintf(path,sizeof path,"%s/%s.ppm",dir,id);
 paint();FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U v=pixels[i];fputc(v>>16,f);fputc(v>>8,f);fputc(v,f);}fclose(f);
}
static void tap_ui(U k){assert(scene==8);night_garage_tick(k,k);}
static void fresh(void){
 init_city();scene=1;menu=-1;mapOn=0;starter_save();ng_reset();z(&nv.garage,sizeof nv.garage);vd_reset();
 sv.intro=4;sv.owned=1;sv.selection=0;sv.carCond[0]=100;sv.cash=2000;sv.dirty=0;
 p=(Player){.x=1024,.y=1024,.car=-1,.hp=100};clear_input();end_job();bank_ui_reset();
 cars[0]=(Car){1024,1024,0,0,0,1,100};
}
static void workshops(void){
 fresh();open_interior(3);active(3);makeMenu();int found=0;for(int i=0;i<nopt;i++)if(opt[i].action==184)found=1;assert(found);
 doOpt(184);assert(scene==8&&ng.tab==NG_MAIN);int balance=sv.cash;tap_ui(ENTER);assert(ng_read(0,1)&&sv.cash==balance-NG_DEED);
 tap_ui(DOWN);tap_ui(ENTER);assert(ng.tab==NG_BUILD&&ng.idx==0);tap_ui(RIGHT);assert(ng.val==1&&!ng_installed(0).paint);
 tap_ui(ENTER);assert(ng_installed(0).paint==1);int curt=sv.cash;tap_ui(ENTER);assert(sv.cash==curt);tap_ui(DOWN);tap_ui(RIGHT);tap_ui(ENTER);
 tap_ui(DOWN);tap_ui(RIGHT);tap_ui(RIGHT);tap_ui(ENTER);assert(ng_installed(0).kit==2);
 tap_ui(DOWN);tap_ui(RIGHT);tap_ui(ENTER);assert(ng_power(cars)==1&&!ng_power(cars+1));tap_ui(DOWN);tap_ui(RIGHT);tap_ui(ENTER);
 tap_ui(DOWN);tap_ui(RIGHT);tap_ui(ENTER);assert(ng_installed(0).sub==1&&!ng_installed(1).sub);
 float a=ng.angle;for(int i=0;i<35;i++)night_garage_tick(BRAKE|RIGHT|UP,0);assert(absf(wrap(ng.angle-a))>.9f&&ng.zoom>1.2f);
 frame("01-lift");tap_ui(ESC);assert(ng.tab==NG_MAIN);tap_ui(ESC);assert(scene==3&&interior_active_loc==3);
 scene=1;p.x=cars[0].x;p.y=cars[0].y;p.car=0;assert(night_garage_audio()==1);p.car=-1;assert(night_garage_audio()==2);p.x+=150;assert(!night_garage_audio());
 nv.garage[1]=~0u;assert(ng_installed(0).paint==0);
}
static void road_camera(void){
 fresh();end_job();job.kind=-1;nc=1;z(streetDrivers,sizeof streetDrivers);
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
 sv.selection=3;sv.owned=8;sv.carCond[3]=100;
 cars[0]=(Car){902,1024,0,0,3,1,100};p.car=0;p.x=cars[0].x;p.y=cars[0].y;fastGfx=0;
 for(int i=0;i<160;i++)tick(0);
 float rest=ngCamZoom;
 assert(rest>.14f&&rest<.15f);frame("02-idle");
 for(int i=0;i<43;i++)tick(UP);
 float ramp=ngCamZoom;
 assert(ngCamBurst>.7f&&ramp<rest-.055f);frame("03-throttle");
 float biggest=0,gap=0;
 for(int i=0;i<27;i++){
  tick(BRAKE|RIGHT|UP);if(absf(ngCamRoll)>biggest)biggest=absf(ngCamRoll);
  float g=absf(wrap(cars[0].a-ngCamAngle));if(g>gap)gap=g;
  assert(absf(ngCamRoll)<.0371f&&absf(cameraLeadX)<=108&&absf(cameraLeadY)<=76);
  if(i==12)frame("04-apex");
 }
 assert(biggest>.002f&&gap>.25f&&cars[0].a>1.2f&&cars[0].cond==100);
 for(int i=0;i<75;i++)tick(UP);frame("05-exit");
 assert(absf(ngCamRoll)<.001f&&absf(wrap(cars[0].a-ngCamAngle))<.003f);
 for(int i=0;i<80;i++)tick(BRAKE);assert(ngCamZoom>ramp+.035f);
 printf("camera: idle crop %.3f -> under-throttle %.3f; max corner lean %.4frad, heading lag %.3frad, coast/stop %.3f.\n",rest,ramp,biggest,gap,ngCamZoom);
 for(int y=0;y<H;y++)for(int x=0;x<W;x++)pixels[W*y+x]=x+1024*y;
 ngCamRoll=.037f;ngCamZoom=.08f;fastGfx=1;U px=pixels[17];ng_cam_compose();assert(pixels[17]==px);fastGfx=0;ng_cam_compose();assert(px!=pixels[17]);
}
static void bookings(void){
 fresh();sv.dayClock=1200;p.x=cars[0].x=NG_MEET_X;p.y=cars[0].y=NG_MEET_Y;p.car=0;night_garage_street(ENTER);assert(scene==8&&ng.tab==NG_MEET);
 tap_ui(RIGHT);assert(ng.rival==1);frame("06-switchyard");
 nv.garage[0]=nv.garage[1]=1;tap_ui(DOWN);int before=sv.cash;tap_ui(ENTER);assert(sv.cash==before+32);
 before=sv.cash;tap_ui(ENTER);assert(sv.cash==before);
 tap_ui(DOWN);tap_ui(ENTER);assert(ng.tab==NG_RUNS&&ng.route==1);tap_ui(DOWN);tap_ui(RIGHT);tap_ui(DOWN);tap_ui(ENTER);
 assert(ng.tab==NG_CONFIRM&&ng.idx==0);tap_ui(ENTER);assert(ng.event==NG_IDLE&&sv.cash==before);
 ng.idx=2;tap_ui(ENTER);tap_ui(DOWN);tap_ui(ENTER);assert(ng.event==NG_STAGE&&sv.cash==before-75);
 night_garage_street(0);assert(ng.event==NG_COUNT);for(int i=0;i<180;i++)night_garage_street(0);
 assert(ng.event==NG_RACE);int x,y;assert(ng_route_goal(&x,&y)&&x==1536&&y==768);
 for(int g=1;g<=4;g++){p.x=cars[0].x=ngRun[1].gate[g].x;p.y=cars[0].y=ngRun[1].gate[g].y;night_garage_street(0);}
 assert(ng.event==NG_IDLE&&sv.cash==before-75+185&&ng_read(33,36000)==4&&ng_read(37,1));
 /* Title stake may never take the driven or only car. */
 ng_enter_panel(NG_RUNS,1);ng.meet=1;ng.stake=2;assert(!ng_begin_race()&&sv.owned==1);
 sv.owned|=2;sv.carCond[1]=100;assert(ng_begin_race()&&sv.owned==1&&ng.spare==1);ng_stop("test forfeit");assert(sv.owned==1);
 scene=1;p.car=0;sv.dayClock=600;ng_clear_event();p.x=NG_MEET_X;p.y=NG_MEET_Y;night_garage_street(ENTER);assert(scene==1);
}
static void recovery(void){
 fresh();nv.garage[0]=1;ng_enter_panel(NG_MAIN,1);assert(ng_begin_recovery());assert(ng.event==NG_FETCH&&ng.target>0&&ng.target<nc);
 int id=ng.target,cond=cars[id].cond,t=cars[id].type;p.car=id;p.x=cars[id].x;p.y=cars[id].y;night_garage_street(0);assert(ng.event==NG_DROP);
 p.x=cars[id].x=NG_DOCK_X;p.y=cars[id].y=NG_DOCK_Y;cars[id].s=0;night_garage_street(ENTER);assert(scene==8&&ng.tab==NG_RETURN);
 int old=sv.cash;assert(!(sv.owned&(1<<t)));assert(ng_settle(1));assert((sv.owned&(1<<t))&&sv.carCond[t]==cond&&sv.cash==old-specs[t].price*2/3&&cars[id].x<0&&p.car<0);
 ng_enter_panel(NG_MAIN,1);assert(ng_begin_recovery());id=ng.target;ng.frames=1;night_garage_street(0);assert(ng.event==NG_IDLE&&cars[id].x>0);
}
int main(int argc,char**argv){dir=argc>1?argv[1]:NULL;workshops();bookings();recovery();road_camera();puts("Garage, preview, audio context, three-class race, salvage and camera checks passed.");return 0;}
