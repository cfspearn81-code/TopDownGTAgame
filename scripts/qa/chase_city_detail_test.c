/* Repeatable native renderer views of authored urban assets. Usage:
 * cc -O2 -DC_CITY_TRIM -Isrc scripts/qa/chase_city_detail_test.c -o /tmp/trim
 * Different baseline can be compared using -Ipath/to/baseline/src without
 * DC_CITY_TRIM; both runs measure identical angles and output PPM frames. */
#define _POSIX_C_SOURCE 200809L
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <time.h>
static double cpu_ms(void){
 struct timespec t;clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t);
 return 1e3*t.tv_sec+1e-6*t.tv_nsec;
}
static int compare(const void*a,const void*b){
 return (*(const double*)a>*(const double*)b)-(*(const double*)a<*(const double*)b);
}
static void shot(const char*dir,const char*name){
 if(!dir)return;
 char f[500];assert(snprintf(f,sizeof f,"%s/%s.ppm",dir,name)<(int)sizeof f);
 FILE*out=fopen(f,"wb");assert(out);fprintf(out,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc(c>>16,out);fputc(c>>8,out);fputc(c,out);}
 assert(fclose(out)==0);
}
static void draw_at(float x,float y,float h,float a,float b){
 ch.x=x;ch.y=y;ch.h=h;ch.co=cs(a);ch.si=sn(a);chCos=cs(b);chSin=sn(b);
 ch_ground();ch_buildings();ch_props();
}
int main(int argc,char**argv){
 const char*dir=argc>1?argv[1]:0;set_session_seed(289);init_city();
 U career=savehash(),seed=rng;
 chDark=.87f;chFog=.16f;nwWet=.65f;
 struct {int id;float offset,angle;const char *name;} shots[]={
  {ncFrontId,10,-1.64f,"afterimage"}, {CAB_COPPER_BUILDING,-8,-1.50f,"copper"},
  {zone_idx[0],-13,-1.37f,"mercury"},{zone_idx[1],-14,-1.46f,"vault"},
  {zone_idx[2],12,-1.78f,"ferry-market"}
 };
 for(int i=0;i<LEN(shots);i++){
  if(i==2||i==4){chDark=.12f;chFog=.04f;nwWet=.03f;}
  const Building*b=buildings+shots[i].id;
  draw_at(b->x+b->w*.5f+shots[i].offset,b->y+b->h+80,25,shots[i].angle,.12f);
  shot(dir,shots[i].name);
 }
 StreetLamp l;assert(street_lamp_legacy(3,2,0,&l));
 chDark=.73f;chFog=.06f;
 draw_at(l.x+17,l.y+72,23,-1.73f,.11f);shot(dir,"crossing-lamp");
 #ifdef DC_CITY_TRIM
  assert(cfRoadInkReady);
  assert(cf_road_sample(0x9d9b71,768+55,512-29)!=0x9d9b71);
  assert(cf_road_sample(0x9d9b71,768+88,512+17)==0x9d9b71);
  const ChVec a={275,295,0},b={395,295,0};
  float light=cf_facade_distance(a,b);ch.si=1;ch.co=0;
  assert(light==cf_facade_distance(a,b)); /* heading alone cannot relight a wall */
 #endif
 for(int i=0;i<72;i++){
  float a=.049f*i;ChVec c={502.f+i*.6f,757.f+i*.5f,0};
  draw_at(c.x,c.y,17,a,.13f);
 }
 assert(savehash()==career&&rng==seed);
 double t[120];
 for(int i=0;i<LEN(t);i++){
  double a=cpu_ms();draw_at(256+11.f*i,1024+12.f*sn(i*.35f),39,.18f*sn(i*.19f),.25f);t[i]=cpu_ms()-a;
 }
 qsort(t,LEN(t),sizeof(*t),compare);
 printf("PASS six address/lamp views and 72-point near corner sweep; no RNG or save writes. 120 urban frames CPU median %.3f ms / p95 %.3f ms\n",t[59],t[113]);return 0;
}
