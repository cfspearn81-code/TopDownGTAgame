/* Simulation-safe island/ribbon/atlas fixture against the real world renderer.
   gcc -O1 -std=c11 scripts/qa/harbor_geometry_test.c -o build/harbor-geometry
   build/harbor-geometry [existing output directory for PPMs] */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <string.h>
static const char*out;
static void capture(const char*n){
 paint();if(!out)return;
 char name[384];snprintf(name,sizeof name,"%s/%s.ppm",out,n);
 FILE*f=fopen(name,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc((c>>16)&255,f);fputc((c>>8)&255,f);fputc(c&255,f);}
 assert(!fclose(f));
}
static void hit(U key){tick(key);tick(0);}
static int marine_line(OuterPoint a,OuterPoint b,int r){
 for(int i=0;i<=100;i++){
  float t=i*.01f;
  if(!harbor_nav_water(a.x+(b.x-a.x)*t,a.y+(b.y-a.y)*t,r))return 0;
 }
 return 1;
}
static void approach(NavPoint start,NavPoint end,int inCar){
 p.car=inCar?0:-1;
 nav_solve(start,end);assert(navCount>=2&&navDist>5);
 assert(navNodes[navCount-1].x==end.x&&navNodes[navCount-1].y==end.y);
 for(int i=1;i<navCount;i++)assert(nav_traversable(navNodes[i-1],navNodes[i],inCar?6:3));
}
static void road_hulls(void){
 for(int i=OUT_ROADS_ORIGINAL;i<OUT_ROADS_WITH_QUAY;i++){
  const OuterRoad*r=outerRoads+i;
  OuterPoint a=outerNodes[r->a],b=outerNodes[r->b];float dx=b.x-a.x,dy=b.y-a.y;
  Car c={.type=0,.cond=100,.a=look(dx,dy)};
  for(int k=0;k<=120;k++){
   c.x=a.x+dx*k/120;c.y=a.y+dy*k/120;
   assert(!street_hits_wall(&c)&&outer_walkable(c.x,c.y,10));
  }
 }
}
int main(int argc,char**argv){
 out=argc>1?argv[1]:0;init_city();scene=1;sv.intro=4;sv.dayClock=720;job.kind=-1;
 NavPoint oldCity={1792,1792};
 assert(OUT_HQ_TIDE==65&&OUT_NP_PASS==66&&LEN(outerStops)==10&&LEN(outerNodes)==74);
 assert(!strcmp(outerStops[3].label,"HARBOR PIER")&&woodline_is_grove(560,2100));
 assert(!harbor_nav_water(492,2636,4)&&!harbor_nav_water(3040,2572,2)&&!harbor_nav_water(2400,2400,-1));
 for(int i=0;i<LEN(harborPieces);i++)assert(harborPiecePlaced[i]);
 assert(!harbor_dock_point(-1,0,0));
 road_hulls();
 for(int i=0;i<HARBOR_N_DOCK_POINTS;i+=2){
  int x,y,u,v;assert(harbor_dock_point(i,&x,&y)&&harbor_dock_point(i+1,&u,&v));
  assert(outer_walkable(x,y,10)&&!outer_area(x,y,10)&&!harbor_nav_water(x,y,1));
  assert(solid(u,v,10)&&harbor_nav_water(u,v,17));
  approach(oldCity,(NavPoint){x,y},0);
  approach(oldCity,nav_outer_point(outerStops[6+i/2].node),1);
  p.x=x;p.y=y;p.car=-1;
  for(int j=0;j<40;j++)street_player_step(0,2.5f); /* The visible pier stays dry. */
  assert(p.y>=y&&p.y<(float)v-12.f&&!harbor_nav_water(p.x,p.y,0));
 }
 static const OuterPoint seaPings[]={{2840,2420},{2718,2390},{2635,2440}};
 for(int i=0;i<LEN(seaPings);i++){
  assert(harbor_nav_water(seaPings[i].x,seaPings[i].y,17));
  assert(marine_line(harborDockPoints[HARBOR_PUBLIC_BOAT],seaPings[i],17));
  const OuterPoint highSea={seaPings[i].x,2650},tideApproach={2478,2650};
  assert(marine_line(seaPings[i],highSea,17)&&marine_line(highSea,tideApproach,17));
  assert(marine_line(tideApproach,harborDockPoints[HARBOR_TIDE_BOAT],17));
 }
 assert(outer_walkable(HARBOR_OFFICE_X,HARBOR_OFFICE_Y,10));
 assert(outer_area(HARBOR_YARD_X,HARBOR_YARD_Y,10)&&outer_area(2824,2192,10));
 Car loading={.x=2824,.y=2185,.cond=100,.a=1.570796f};
 for(int y=2155;y<=2214;y++){assert(outer_walkable(2852,y,10));assert(street_foot_car_depth(&loading,2852,y,10)<=0);}
 for(int x=2824;x<=2852;x++){assert(outer_walkable(x,2214,10));assert(street_foot_car_depth(&loading,x,2214,10)<=0);}
 assert(!harbor_nav_water(__builtin_nanf(""),2244,4));
 /* Signed public stop in the actual F8 inspector. */
 p.x=2856;p.y=2054;p.car=-1;mapOn=0;menu=-1;clear_input();hit(MAP);assert(mapOn&&nav_span()==ROOM);
 NavPoint q=nav_outer_point(outerStops[6].node);
 int ax=NAV_MAP_X+q.x*NAV_MAP_SIZE/nav_span(),ay=NAV_MAP_Y+q.y*NAV_MAP_SIZE/nav_span();
 pointer_input(ax,ay,1);assert(navHasManual&&navManualLoc==NAV_OUT_BASE+6&&navManual.x==2950);
 capture("harbor-atlas");hit(ESC);assert(!mapOn);nav_clear();p.noticeT=0;
 p.x=2896;p.y=2127;
 capture("harbor-quay-day");
 sv.dayClock=60;anim=2.4f;capture("harbor-quay-night");
 p.x=2478;p.y=2510;capture("harbor-tide-signal");
 p.x=2856;p.y=2054;p.car=0;nc=1;sv.owned=1;nv.garage[60]=1;
 cars[0]=(Car){.x=p.x,.y=p.y,.a=.84f,.owner=1,.cond=100};ngCamRide=-1;
 capture("harbor-chase");assert(chProjected);
 puts("PASS: unchanged old pier/woods, 1,089 new road hull samples, 3 foot slips, 3 r17 marine links, terrain separation, 6 old-city routes, signed F8 pin, overhead/chase snapshots.");
}
