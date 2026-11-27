/* Deterministic light/physics regression with unchanged real play functions. */
#define DC_TEST
#include "game.c"
#include <assert.h>

static void fresh(void){
 init_city();starter_save();vd_reset();end_job();menu=-1;mapOn=0;scene=1;fastGfx=0;
 clear_input();night_world_reset();outer_life_reset();
 sv.intro=4;sv.dayClock=1260;sv.owned=8;sv.selection=3;sv.carCond[3]=100;sv.hp=100;
 z(streetDrivers,sizeof streetDrivers);nc=1;
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
 cars[0]=(Car){970,1024,0,0,3,1,100};p=(Player){.x=970,.y=1024,.hp=100,.car=0};
 cx=700;cy=810;
}
static int probe_lamp(int x,int y){return headlamps_energy(x,y);}
static void world_fixture(void){
 fresh(); /* Blank room for controlled ray tests, unrelated city art untouched. */
 z(roofMask,sizeof roofMask);
 cars[0].x=p.x=1024;cars[0].y=p.y=1024;cars[0].a=0;
 headlamps_build(0,940,940,1170,1120);
 assert(hlCount>0&&probe_lamp(1040,1018)>25&&probe_lamp(1040,1030)>25);
 for(int x=1041;x<1090;x++){
  int y=(int)(1024+5+(x-1040)*.138f+.5f);
  assert(probe_lamp(x,y)>10&&probe_lamp(x,2047-y)>10);
  assert(!probe_lamp(x,1024)&&!probe_lamp(x,1023));
 }
 assert(!probe_lamp(1010,1018)&&!probe_lamp(1010,1030)&&!probe_lamp(1136,1038));
 /* Co/si rotation and reverse velocity do not change fixture orientation. */
 cars[0].a=1.570796f;cars[0].s=-28.f;
 headlamps_build(0,940,940,1170,1120);
 assert(probe_lamp(1016,1060)>20&&probe_lamp(1031,1060)>20&&!probe_lamp(1024,1060)&&!probe_lamp(1027,990));
 cars[0].a=0;cars[0].s=0.f;
 for(int y=1028;y<1050;y++)roofMask[y*ROOM+1055]=1;
 headlamps_build(0,940,940,1170,1120);
 assert(!probe_lamp(1065,1033)&&probe_lamp(1065,1015)>10&&!probe_lamp(1055,1031));
 for(int y=1028;y<1050;y++)roofMask[y*ROOM+1055]=0;
 /* Traffic at a stop signal stays lit; a wreck / removed vehicle does not. */
 p.car=-1;nc=2;cars[1]=(Car){1024,1024,0,0,1,0,100};streetDrivers[1].live=1;
 headlamps_build(0,940,940,1170,1120);
 assert(probe_lamp(1065,1033)&&probe_lamp(1065,1014)&&!probe_lamp(1065,1024));
 cars[1].cond=0;headlamps_build(0,940,940,1170,1120);assert(!hlCount);
 nc=1;int regional=STREET_CITY_CAR_LIMIT;
 cars[regional]=(Car){1024,1024,0,0,2,0,100};outDriver[0].driven=0;
 assert(street_next_car(0)==regional);
 headlamps_build(0,940,940,1170,1120);
 assert(probe_lamp(1065,1033)&&probe_lamp(1065,1014)&&!probe_lamp(1065,1024));
 outDriver[0].driven=1;headlamps_build(0,940,940,1170,1120);assert(!hlCount);
 p.car=regional;headlamps_build(1,940,940,1170,1120);
 assert(probe_lamp(1065,1033)&&probe_lamp(1065,1014)&&!probe_lamp(1065,1024));
 fastGfx=1;headlamps_build(0,940,940,1170,1120);assert(!hlCount&&!probe_lamp(1065,1033));
}
static void write_frame(const char*dir,const char*name){
 if(!dir)return;
 char f[600];snprintf(f,sizeof f,"%s/%s.ppm",dir,name);FILE*out=fopen(f,"wb");assert(out);
 fprintf(out,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc(c>>16,out);fputc(c>>8,out);fputc(c,out);}assert(!fclose(out));
}
static U sample(float xd,float yd){
 ChVec p0=ch_local((ChVec){xd,yd,0});assert(p0.z>9);ChPlot d=ch_screen(p0);
 assert(d.x>=0&&d.x<W&&d.y>=0&&d.y<H);return pixels[d.y*W+d.x];
}
static void renderer_fixture(const char*dir){
 fresh();ngCamAngle=cars[0].a;ngCamRide=0;nwWet=.72f;sv.cash=420;
 paint();write_frame(dir,"01-overhead-twin");
 int front=1030,sep=11; /* at 60 from the vehicle, after the hood */
 assert(((pixels[sy(1024+sep)*W+sx(front)]>>16)&255)>((pixels[sy(1024)*W+sx(front)]>>16)&255)+15);
 fastGfx=1;paint();write_frame(dir,"02-overhead-low-fx");assert(!hlCount);
 fastGfx=0;nv.garage[60]=1;
 paint();write_frame(dir,"03-chase-twin");assert(chProjected);
 const float near=1005,side=7.8f;
 U l=sample(near,1024-side),c=sample(near,1024),r=sample(near,1024+side);
 assert(((l>>16)&255)>((c>>16)&255)+35&&((r>>16)&255)>((c>>16)&255)+35);
 cars[0].a=.48f;ngCamAngle=.15f;
 paint();write_frame(dir,"04-chase-steering");
 Car final[2];float heat0[2];
 for(int low=0;low<2;low++){
  fresh();fastGfx=low;nv.garage[60]=!low;
  for(int i=0;i<110;i++){tick(i<45?UP:i<72?UP|BRAKE|RIGHT:0);paint();}
  final[low]=cars[0];heat0[low]=p.heat;
 }
 assert(final[0].x==final[1].x&&final[0].y==final[1].y&&final[0].a==final[1].a
  &&final[0].s==final[1].s&&final[0].cond==final[1].cond&&heat0[0]==heat0[1]);
}
static void woodline_shoulders(void){
 fresh();
 /* Harbour sweep: the lamps reach the walkable open land across its
    shoulder, rather than stopping at an invisible asphalt boundary. */
 cars[0].x=p.x=2416;cars[0].y=p.y=1663;cars[0].a=1.570796f;
 assert(outer_onroad(p.x,p.y));
 headlamps_build(0,2320,1625,2520,1810);
 int offroad=0;
 for(int y=1708;y<=1735;y++)for(int x=2385;x<=2447;x++)
  offroad+=!outer_onroad(x,y)&&outer_walkable(x,y,0)&&probe_lamp(x,y)>8;
 assert(offroad>20&&beam_block(492,2636)); /* Mirror Tarn remains opaque. */
}
static void native_setup(const char*name){
 fresh();starter_save();sv.intro=4;sv.owned=1;sv.selection=0;sv.carCond[0]=100;sv.hp=100;sv.cash=420;
 sv.x=284;sv.y=506;sv.dayClock=1240;sv.hash=savehash();
 FILE*f=fopen(name,"wb");assert(f&&fwrite(&sv,sizeof sv,1,f)==1&&!fclose(f));
}
int main(int argc,char**argv){
 if(argc==3){native_setup(argv[2]);return 0;}
 world_fixture();renderer_fixture(argc>1?argv[1]:NULL);woodline_shoulders();
 puts("Twin beams: two live front origins, persistent dark center, rotate with heading/reverse, independent wall cutoff, parked traffic/open shoulders without water bleed, F2 and unmodified driving passed.");
 return 0;
}
