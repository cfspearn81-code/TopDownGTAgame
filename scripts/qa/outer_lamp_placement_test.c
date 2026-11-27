/* Independent geometry/atlas coverage of every visible outer municipal light.
 * Optional output directory receives actual renderer frames at two headings
 * in both perspectives. Existing slots and cameras remain untouched.
 * Test: cc -O2 -Isrc scripts/qa/outer_lamp_placement_test.c -o /tmp/outer-lamps
 * Compile against an old source with -DDC_EXPECT_LEGACY_LAMPS for comparison. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
static int clear_foot(const StreetLamp*s,int detail){
 int x=s->x,y=s->y,clear=1;
 if(x<35||x>ROOM-36||y<35||y>ROOM-36)return 0;
 for(int i=0;i<LEN(outerRoads);i++){
  const OuterRoad*r=&outerRoads[i];const OuterPoint a=outerNodes[r->a],b=outerNodes[r->b];
  float dx=b.x-a.x,dy=b.y-a.y,t=((x-a.x)*dx+(y-a.y)*dy)/(dx*dx+dy*dy);
  t=t<0?0:t>1?1:t;
  float w=r->half+detail;
  if(d2(x-a.x-dx*t,y-a.y-dy*t)<w*w)clear=0;
 }
 return clear;
}
static int graphic_clear(int x,int y,int w,int h){
 if(x<35||x+w>ROOM-35||y<35||y+h>ROOM-35)return 0;
 for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++){
  int j=yy*ROOM+xx;
  if(outerLane[j]||roofMask[j]||!outer_walkable(xx,yy,0))return 0;
 }
 return 1;
}
static void record(const char*dir,const char*filename){
 if(!dir)return;
 char f[450];assert(snprintf(f,sizeof f,"%s/%s.ppm",dir,filename)<(int)sizeof f);
 FILE*out=fopen(f,"wb");assert(out);fprintf(out,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc(c>>16,out);fputc(c>>8,out);fputc(c,out);}assert(fclose(out)==0);
}
static void evaluate(const char*dir){
 /* A real parked owned ride, front wheels about to pass through the old
  * unsafe x=2215,y=1007 anchor on Freight Parkway's westbound carriageway. */
 const struct {int x,y;float angle;const char*chase,*top;} tuples[]={
  {2158,1008,.03f,"parkway-c","parkway-over"},
  {2594,1006,.01f,"freight-c","freight-over"},
  {2120,1783,-.40f,"shore-c","shore-over"},
  {2429,2310,1.50f,"pier-c","pier-over"},
 };
 scene=1;menu=-1;mapOn=fastGfx=0;sv.dayClock=1248;
 sv.intro=4;sv.hp=p.hp=100;sv.owned=1;sv.carCond[0]=100;
 for(int i=0;i<LEN(tuples);i++){
  p.x=tuples[i].x;p.y=tuples[i].y;p.a=tuples[i].angle;p.car=0;
  cars[0]=(Car){p.x,p.y,p.a,0,0,1,100};ngCamRide=0;ngCamAngle=p.a;ch.ride=-1;
  nv.garage[60]=1;paint();assert(chProjected);record(dir,tuples[i].chase);
  nv.garage[60]=0;paint();assert(!chProjected);record(dir,tuples[i].top);
 }
}
int main(int argc,char**argv){
 set_session_seed(273);init_city();
 int old=0,represented=0,problem=0,valid=0,harbor=0;
 for(int i=0;i<LEN(outerNodes);i+=2){
  const OuterPoint q=outerNodes[i];
  if(!outer_is_new(q.x,q.y))continue;
  StreetLamp begin=street_lamp_style(q.x+23,q.y-17,q.y<810?0x845987:q.y<1520?0x8d7352:0x388b8c,0),s;
  old++;problem+=!clear_foot(&begin,0);
  if(!street_lamp_outer(i,&s))continue;
  represented++;int geometry=clear_foot(&s,8),
      graphic=graphic_clear(s.x+s.poleX,s.y+s.poleY,s.poleW,s.poleH)&&
              graphic_clear(s.x+s.lensX,s.y+s.lensY,s.lensW,s.lensH);
  valid+=geometry&&graphic;
  printf("node %2d (%d,%d) -> (%d,%d) %s\n",i,begin.x,begin.y,s.x,s.y,geometry&&graphic?"clear":"on road or blocked");
  assert(s.radius==begin.radius&&s.weight==begin.weight&&s.pool==begin.pool&&s.pole==begin.pole&&s.lens==begin.lens);
  assert(s.poleW==begin.poleW&&s.poleH==begin.poleH&&s.lensW==begin.lensW&&s.lensH==begin.lensH);
 #ifndef DC_EXPECT_LEGACY_LAMPS
  int close=0;
  for(int j=0;j<LEN(outerRoads);j++){
   const OuterRoad *r=outerRoads+j;OuterPoint a=outerNodes[r->a],b=outerNodes[r->b];float t=out_proj(a,b,s.x,s.y);
   float margin=r->half+26.f;
   if(d2(s.x-a.x-(b.x-a.x)*t,s.y-a.y-(b.y-a.y)*t)<margin*margin)close=1;
  }
  assert(close&&geometry&&graphic);
  for(int y=0;y<s.lensH;y++)for(int x=0;x<s.lensW;x++)assert(city[(s.y+s.lensY+y)*ROOM+s.x+s.lensX+x]==s.lens);
 #endif
 }
 for(int i=0;i<HARBOR_LAMPS;i++){
  StreetLamp s;
  if(!street_lamp_harbor(i,&s))continue;
  harbor++;
  printf("pier %d (%d,%d) %s\n",i,s.x,s.y,clear_foot(&s,8)?"clear":"on road");
 #ifndef DC_EXPECT_LEGACY_LAMPS
  assert(clear_foot(&s,8)&&graphic_clear(s.x+s.poleX,s.y+s.poleY,s.poleW,s.poleH)
        &&graphic_clear(s.x+s.lensX,s.y+s.lensY,s.lensW,s.lensH));
 #endif
 }
 printf("%d/%d original road nodes placed lights into roads; %d/%d fixture positions clear now, %d dry pier lights.\n",problem,old,valid,represented,harbor);
 #ifndef DC_EXPECT_LEGACY_LAMPS
  assert(problem>20&&represented>=24&&represented==valid&&harbor==3);
  assert(street_lamp_outer(16,&(StreetLamp){0})&&street_lamp_outer(18,&(StreetLamp){0}));
  StreetLamp first[LEN(outerNodes)];unsigned char placed[LEN(outerNodes)];
  cp(first,outerLamp,sizeof first);cp(placed,outerLampPlaced,sizeof placed);
  U oldRng=rng,oldSave=savehash();
  street_lamp_place_outside();
  for(int i=0;i<LEN(outerNodes);i++){
   assert(placed[i]==outerLampPlaced[i]);
   if(placed[i])assert(first[i].x==outerLamp[i].x&&first[i].y==outerLamp[i].y);
  }
  assert(rng==oldRng&&oldSave==savehash());
 #endif
 if(argc>1)evaluate(argv[1]);
 return 0;
}
