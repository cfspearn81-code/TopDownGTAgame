/* Geometric survey plus real pedestrian movement across the old edge. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
enum{SURVEY_STEP=3,SURVEY_SIDE=ROOM/SURVEY_STEP,SURVEY_N=SURVEY_SIDE*SURVEY_SIDE};
static unsigned char reached[SURVEY_N],clear[SURVEY_N];
static int frontier[SURVEY_N];
static int sample_xy(int x,int y){return cl(y/SURVEY_STEP,0,SURVEY_SIDE-1)*SURVEY_SIDE+cl(x/SURVEY_STEP,0,SURVEY_SIDE-1);}
static int mid_x(int i){return i%SURVEY_SIDE*SURVEY_STEP+SURVEY_STEP/2;}
static int mid_y(int i){return i/SURVEY_SIDE*SURVEY_STEP+SURVEY_STEP/2;}
static void flood_walk(void){
 int n=0,e=0,k=sample_xy(520,1997);
 for(int i=0;i<SURVEY_N;i++)clear[i]=!solid(mid_x(i),mid_y(i),10);
 for(int i=0;i<SURVEY_N;i++){int x=mid_x(i),y=mid_y(i);if(clear[i]&&y==mid_y(k)&&x<2005){reached[i]=1;frontier[e++]=i;}}
 while(n<e){
  int id=frontier[n++],x=id%SURVEY_SIDE,y=id/SURVEY_SIDE;
  static const int d[]={-1,1,-SURVEY_SIDE,SURVEY_SIDE};
  for(int i=0;i<4;i++){
   int to=id+d[i];
   if((unsigned)to>=SURVEY_N||(i==0&&!x)||(i==1&&x>=SURVEY_SIDE-1))continue;
   if(reached[to]||!clear[to])continue;
   if(!street_line_open(mid_x(id),mid_y(id),mid_x(to),mid_y(to),10))continue;
   reached[to]=1;frontier[e++]=to;
  }
 }
 int land=0,visited=0,deep=0;
 for(int i=0;i<SURVEY_N;i++)if(outer_is_new(mid_x(i),mid_y(i))&&clear[i]){
  land++;visited+=reached[i]!=0;
  if(mid_y(i)>2750&&mid_x(i)<980)deep+=reached[i]!=0;
 }
 printf("Open land flood fill: %d/%d legal exterior samples, including %d bottom-left forest samples.\n",visited,land,deep);
 assert(visited>300000&&visited*1000>=land*997&&deep>3000);
 /* At this resolution each cell is only 3x3 world units. Remaining
  * isolated slivers must be tiny edge scraps, not a secondary region. */
 int pocketMax=0,pocketX=0,pocketY=0;
 for(int i=0;i<SURVEY_N;i++)if(!reached[i]&&clear[i]&&outer_is_new(mid_x(i),mid_y(i))){
  int a=0,b=0;reached[i]=2;frontier[b++]=i;
  while(a<b){
   int id=frontier[a++],x=id%SURVEY_SIDE;
   static const int d[]={-1,1,-SURVEY_SIDE,SURVEY_SIDE};
   for(int k=0;k<4;k++){
    int q=id+d[k];
    if((unsigned)q>=SURVEY_N||(k==0&&!x)||(k==1&&x==SURVEY_SIDE-1))continue;
    if(reached[q]||!clear[q]||!outer_is_new(mid_x(q),mid_y(q)))continue;
    reached[q]=2;frontier[b++]=q;
   }
  }
  if(b>pocketMax){pocketMax=b;pocketX=mid_x(i);pocketY=mid_y(i);}
 }
 printf("Largest sub-sample of stranded edge pixels: %d near %d,%d.\n",pocketMax,pocketX,pocketY);
 assert(pocketMax<=24);
 for(int i=0;i<WOOD_N_SITES;i++){
  OuterPoint q=woodline_sites[i];assert(reached[sample_xy(q.x,q.y)]&&outer_walkable(q.x,q.y,18));
 }
}
/* Cross perpendicularly from a legal road center into the dirt, then
 * back, at safe land sides of every curved road class. */
static void survey_edges(void){
 /* The existing Outer Loop roads have dry shoulders at these probes.
    The nine new working quays intentionally end beside water; their
    independent fixture tests all hull directions and the foot decks. */
 int counts[5]={0},scanned=0,actual=0,sea=0;
 const int oldRoads=LEN(outerRoads)-9;
 for(int i=0;i<oldRoads;i++){
  const OuterRoad*r=outerRoads+i;OuterPoint a=outerNodes[r->a],b=outerNodes[r->b];
  float dx=b.x-a.x,dy=b.y-a.y,len=sq(d2(dx,dy)),xx=-dy/len,yy=dx/len;
  int side[2]={0,0};
  static const float fractions[]={.22f,.4f,.6f,.8f,.92f,.976f};
  for(int sign=-1;sign<=1;sign+=2)for(int j=0;j<LEN(fractions);j++){
   float t=fractions[j],x=a.x+dx*t,y=a.y+dy*t,d=(r->half+24)*sign;
   float u=x+xx*d,v=y+yy*d;
   if(!outer_is_new(x,y)||!outer_is_new(u,v)||!outer_area(x,y,10)||outer_area(u,v,8))continue;
   scanned++;
   if(!outer_walkable(u,v,10)){sea++;continue;}
   if(!street_line_open(x,y,u,v,10))continue;
   p.x=x;p.y=y;
   for(int step=0;step<48;step++)street_player_step((u-x)/48,(v-y)/48);
   if(d2(p.x-u,p.y-v)>4.f)continue;
   actual++;
   side[(sign+1)/2]=1;
   Car c={.x=u,.y=v,.a=look(dx,dy),.cond=100};
   assert(street_hits_wall(&c));
   for(int step=0;step<48;step++)street_player_step((x-u)/48,(y-v)/48);
   assert(d2(p.x-x,p.y-y)<4.f);
   break;
  }
  counts[r->kind]+=side[0]+side[1];
 }
 printf("Physical lane-to-land steps: %d/%d surveyed side samples, %d true water/obstacle edges, classes %d hill / %d freeway / %d shore / %d yards / %d country.\n",
  actual,scanned,sea,counts[0],counts[1],counts[2],counts[3],counts[4]);
 assert(actual==2*oldRoads);
 assert(counts[0]>19&&counts[1]>6&&counts[2]>22&&counts[3]>20&&counts[4]>8);
}
static void actual_walk(void){
 /* A live fixed-tick walk over the pre-Woodline fence, from the city
  * pavement through both the world and view-origin boundaries. */
 scene=1;p.x=520;p.y=1995;p.car=-1;sv.intro=4;mapOn=0;nc=0;job.kind=-1;p.heat=0;menu=-1;
 clear_input();for(int i=0;i<84;i++)tick(DOWN);tick(0);
 printf("Live seam walker: %.1f, %.1f.\n",p.x,p.y);
 assert(p.y>2070&&outer_is_new(p.x,p.y)&&!outer_area(p.x,p.y,10));
 for(int i=0;i<108;i++){
  float n=1998.f+i*.3f;
  assert(!solid(520,n,10));assert(!solid(n,1024,10));
 }
 NavPoint from={(int)p.x,(int)p.y};
 for(int i=0;i<WOOD_N_SITES;i++){
  OuterPoint q=woodline_sites[i];NavPoint to={q.x,q.y};
  nav_solve(from,to);assert(navCount>=2&&nav_same(navNodes[navCount-1],to));
  for(int j=1;j<navCount;j++)assert(street_line_open(navNodes[j-1].x,navNodes[j-1].y,navNodes[j].x,navNodes[j].y,10));
 }
 p.x=492;p.y=2636;assert(solid(p.x,p.y,4));assert(solid(2922,2712,10));
 assert(outer_tile(492,2636)==TILE_TARN&&outer_tile(2922,2712)==TILE_TIDE);
 assert(!outer_walkable(492,2636,0)&&!outer_walkable(2922,2712,0));
 nav_slot();mapOn=0;outerAtlas=1;p.x=520;p.y=1996;p.car=-1;
 navigation_pointer(NAV_MAP_X+woodline_sites[0].x*NAV_MAP_SIZE/ROOM,NAV_MAP_Y+woodline_sites[0].y*NAV_MAP_SIZE/ROOM,1);
 assert(navHasManual&&navManualLoc==NAV_WOOD_BASE+WOOD_RANGER&&navManual.y==2100);
 p.car=0;cars[0].x=512;cars[0].y=1792;nav_update();assert(navLeg&&navCurb.y<=2012);
 puts("Live cross-seam walk, obstacle-aware foot routes to eight sites, pins, park-and-walk and hard water all pass.");
}
int main(void){
 init_city();profiles_activate();sv.intro=4;sv.owned=0;nc=0;sv.dayClock=800;
 flood_walk();survey_edges();actual_walk();
}
