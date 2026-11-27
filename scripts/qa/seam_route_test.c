/* Regional path joins must not tell players to reverse along exactly the
 * same open road, nor include that mileage on an outbound or return fare. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
static int backtrack(NavPoint a,NavPoint b,NavPoint c){
 int dx=b.x-a.x,dy=b.y-a.y,ex=c.x-b.x,ey=c.y-b.y;
 return dx*ey==dy*ex&&dx*ex+dy*ey<0;
}
static int route(int from,int to,int hour,int trace){
 sv.dayClock=hour*60;int a,b,c,x,y;
 assert(taxi_curb(from,&a,&b,&c)&&taxi_curb(to,&x,&y,&c));
 nav_solve((NavPoint){a,b},(NavPoint){x,y});
 assert(navCount>=2&&navNodes[0].x==a&&navNodes[0].y==b&&navNodes[navCount-1].x==x&&navNodes[navCount-1].y==y);
 int actual=0,backs=0;for(int i=1;i<navCount;i++){
  NavPoint p=navNodes[i-1],q=navNodes[i];
  actual+=nav_length(p,q);assert(!night_phone_lane_stop(p.x,p.y,q.x,q.y,4));
  if(i>=2&&backtrack(navNodes[i-2],p,q))backs++;
 }
 if(trace)printf("%s to %s %02d:00 / %d m, %d immediate retraces\n",taxi_stop_label(from),taxi_stop_label(to),hour,actual,backs);
 assert(actual==navDist&&taxi_dispatch_distance(a,b,x,y)==actual);
 return backs;
}
int main(int argc,char **argv){
 int survey=argc>1;init_city();int bad=0,run=0;
 for(int h=0;h<24;h++)for(int c=0;c<LEN(loc);c++)for(int k=0;k<LEN(outerStops);k++){
  int verbose=h==15&&c==10&&(k==2||k==3);
  bad+=route(c,LEN(loc)+k,h,verbose);bad+=route(LEN(loc)+k,c,h,verbose);run+=2;
 }
 printf("%d city/outer itineraries: %d spurious immediate reversals.\n",run,bad);
 if(!survey)assert(!bad);
}
