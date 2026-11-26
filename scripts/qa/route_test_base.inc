#include "route_prefix.inc"
#include <stdint.h>
#include <math.h>
enum {NXST=512,NN=NXST*NXST};
static signed char ab[NN];
static int streetq[NN],chosen[NN],pred[NN],path[NN];
static const int stepX[]={-1,1,0,0,1,-1,1,-1};
static const int stepY[]={0,0,-1,1,1,1,-1,-1};
static int at(int xx,int yy){return xx>=0&&xx<NXST&&yy>=0&&yy<NXST?xx+yy*NXST:-1;}
static float xof(int i){return (float)(i%NXST*4);}
static float yof(int i){return (float)(i/NXST*4);}
static int clear_at(int id){
 if(id<0)return 0;
 if(!ab[id])ab[id]=solid(xof(id),yof(id),12)||!street_foot_car_clear(xof(id),yof(id),-1,9.f)?-1:1;
 return ab[id]>0;
}
static int board_test(float x,float y){
 float ox=p.x,oy=p.y;p.x=x;p.y=y;StreetUse u=street_use_target(placeNear(),carNear());p.x=ox;p.y=oy;
 return u.kind==STREET_USE_CAR&&u.id==vm.van;
}
static int street_path_xy(int gx,int gy,int door){
 memset(ab,0,NN);memset(pred,-1,sizeof pred);
 int src=at((int)(p.x/4+.5f),(int)(p.y/4+.5f));int rd=0,wr=0,finish=-1;
 pred[src]=src;streetq[wr++]=src;
 while(rd<wr){
  int c=streetq[rd++],x=c%NXST,y=c/NXST;
  if(d2(xof(c)-gx,yof(c)-gy)<=(door?18*18:36*36)){
   if(door ? street_line_open(xof(c),yof(c),gx,gy,3.f) : board_test(xof(c),yof(c))){finish=c;break;}
  }
  for(int k=0;k<8;k++){
   int xx=x+stepX[k],yy=y+stepY[k],n=at(xx,yy);
   if(n<0||pred[n]!=-1)continue;
   if(!clear_at(n))continue;
   if(k>=4&&(!clear_at(at(xx,y))||!clear_at(at(x,yy))))continue;
   pred[n]=c;streetq[wr++]=n;
  }
 }
 if(finish<0){fprintf(stderr,"reachable %d nodes from %g,%g to %d,%d\n",wr,p.x,p.y,gx,gy);return 0;}
 int n=0;
 for(int c=finish;c!=src;c=pred[c])chosen[n++]=c;
 for(int i=0;i<n;i++)path[i]=chosen[n-1-i];
 return n;
}
static void street_to(int b){
 int bottom=total,prev_stuck=0,rd=0,n=0;
 while(ordinary_door_near()!=b && total-bottom<7500){
  if(rd>=n||prev_stuck>12){rd=0;n=street_path_xy(ordinaryDoorX[b],ordinaryDoorY[b],1);prev_stuck=0;if(!n){record(0);continue;}}
  int furthest=rd;
  for(int i=rd+1;i<n&&i<rd+26;i++){
   if(!street_reach_open(p.x,p.y,xof(path[i]),yof(path[i]),11.f,-1))break;
   furthest=i;
  }
  rd=furthest;
  float x=xof(path[rd]),y=yof(path[rd]);
  float dx=x-p.x,dy=y-p.y,ox=p.x,oy=p.y;
  if(d2(dx,dy)<3*3){rd++;continue;}
  U k=BRAKE;
  if(absf(dx)>.46f*absf(dy))k|=dx<0?LEFT:RIGHT;
  if(absf(dy)>.46f*absf(dx))k|=dy<0?UP:DOWN;
  record(k);if(d2(ox-p.x,oy-p.y)<.01f)prev_stuck++;else prev_stuck=0;
 }
 fprintf(stderr,"Reached %s [%d] at %g,%g in %d ticks; HP %d\n",walkin_name(b),b,p.x,p.y,total-bottom,p.hp);
 ck(ordinary_door_near()==b,"street path didn't get there");
}
static void r4mark(const char*s){fprintf(stderr,"Frame %d / %s / cash %d / %g,%g / %s\n",total,s,sv.cash,p.x,p.y,notice);}
static void station(int b,const char *label,int leave){
 if(ordinary_door_near()!=b)street_to(b);
 press_key(USE);ck(scene==4&&walkin_active==b,"wrong building");
 go(STATION_G);press_key(USE);
 r4mark(label);
 if(leave){go(EXIT_G);press_key(USE);ck(scene==1,"station exit");}
}
