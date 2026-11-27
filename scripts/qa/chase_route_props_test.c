/* Real sprint/detour/cart rules with render-only perspective markers. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <string.h>
static void key(U k){tick(0);tick(k);tick(0);}
static void frame(const char*dir,const char*name){
 paint();
 if(!dir)return;
 char path[512];snprintf(path,sizeof path,"%s/%s.ppm",dir,name);
 FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int j=0;j<W*H;j++){U c=pixels[j];fputc(c>>16,f);fputc(c>>8,f);fputc(c,f);}
 assert(!fclose(f));
}
static void live(void){
 set_session_seed(31);init_city();starter_save();scene=1;menu=-1;mapOn=0;sv.intro=4;sv.dayClock=797;
 p.car=-1;p.hp=100;p.dead=0;end_job();vd_reset();harbor_boats_reset();
 clear_input();nv.garage[60]=0;fastGfx=0;
}
static Car sprint(int perspective,const char*out){
 live();sv.owned=1;nc=1;p.car=0;p.x=950;p.y=1024;p.a=0;
 cars[0]=(Car){.x=p.x,.y=p.y,.a=0,.type=0,.owner=1,.cond=100};
 job=(Job){.kind=JOB_SPRINT,.total=3,.xs={1024,1280,1280},.ys={1024,1024,1280},.time=95.f};
 key(perspective?CAMERA:0);
 frame(out,perspective?"01-sprint-road-gate":"00-sprint-overhead");
 assert(chProjected==perspective);
 if(perspective){
  assert(ch_job_prop_visible(1024,1024)&&!ch_job_prop_visible(1280,1024));
  p.a=cars[0].a=ngCamAngle=3.14159f;paint();assert(!ch_job_prop_visible(1024,1024));
  p.a=cars[0].a=ngCamAngle=0;paint();
 }
 for(int i=0;i<48;i++)tick(UP);
 assert(job.step==1&&jobRide==0&&cars[0].cond==100);
 return cars[0];
}
static void vendor(const char*out){
 live();sv.dayClock=1145.f;
 int x,y;assert(night_venue_vendor_pos(&x,&y));
 assert(outer_walkable(x,y+24,7));
 p.x=x;p.y=y+24;p.a=-1.5707963f;
 key(CAMERA);frame(out,"02-night-signal-cart");
 assert(chProjected&&scene==1);key(USE);assert(scene==NIGHT_VENUES_SCENE&&nvs_room==NVR_CART);
 key(ESC);assert(scene==1&&ch_pref());frame(out,"03-return-to-cart");
}
static void destinations(const char*out){
 live();
 int x=taxi_copper_x(),y=taxi_copper_y()+22;
 p.x=x+8;p.y=y-2;p.a=-1.5707963f;
 job=(Job){.kind=JOB_TAXI,.total=2,.step=1};
 taxi=(Taxi){.phase=TAXI_RIDING,.copper=CAB_VIA};
 key(CAMERA);frame(out,"04-copper-playhouse");assert(chProjected);
 p.x=outerNodes[outerStops[6].node].x+8;p.y=outerNodes[outerStops[6].node].y+58;frame(out,"05-lantern-quay-sign");
}
static void fixture(const char*path){init_city();starter_save();scene=1;
 sv.intro=4;sv.dayClock=1145;sv.hp=100;
 int x,y;assert(night_venue_vendor_pos(&x,&y));p.x=x;p.y=y+24;assert(outer_walkable(p.x,p.y,7));
 Save11 b={0};save_outer_position();b.base=sv;b.night=nv;outer_safe_anchor(&b.base);
 b.base.rev=NIGHT_SAVE_REV;b.night.world[NW_SAVE_X]|=0x80000000u;
 b.base.hash=night_hash(&b.base,sizeof b.base-4);b.hash=night_hash(&b,sizeof b-4);
 FILE*f=fopen(path,"wb");assert(f&&fwrite(&b,sizeof b,1,f)==1&&!fclose(f));
}

int main(int argc,char**argv){
 if(argc==3&&!strcmp(argv[1],"--vendor-fixture")){fixture(argv[2]);return 0;}
 char*dir=argc>1?argv[1]:0;
 Car a=sprint(0,dir),b=sprint(1,dir);
 assert(a.x==b.x&&a.y==b.y&&a.s==b.s&&a.a==b.a&&a.wear==b.wear&&a.cond==b.cond);
 vendor(dir);destinations(dir);
 puts("PASS: 3D gate follows scored position, rear fallback unchanged, exact overhead driving, real scheduled Night Signal entry/exit, Copper Playhouse and Lantern Quay camera fixtures.");
}
