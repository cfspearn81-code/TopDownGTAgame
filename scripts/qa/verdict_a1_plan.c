#define DC_TEST
#include "../../src/game.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

enum { NX=920, NY=920, N=NX*NY };
static int q[N], seen[N];
static unsigned char came[N], out[N];
static float xs[NX],ys[NY];
static int len=0;

static void key(U mask){assert(len<12000);tick(mask);printf("%u\n",mask);len++;}
static void press(U m){key(m);key(0);}
static void held(U m,int n){for(int i=0;i<n;i++)key(m);}
static void record(const char *label){fprintf(stderr,"%s %d scene=%d job=%d cash=%d x=%.1f y=%.1f notice=%s\n",label,len+1,scene,job.kind,sv.cash,p.x,p.y,notice);}
enum { DESK, DOOR, BLUE, TRACK, RADIO, RIDE, CIRCUIT, WEXIT, WPLAQUE, JPOINT, CDOOR };
static int goal(int g,float x,float y){
 if(g==99)return d2(x-(wi_f[0].x+wi_f[0].w/2),y-a1_rack_y())<25.f*25.f;
 if(g==DESK)return d2(interior_desk_x-x,interior_desk_y-y)<38*38&&!int_at_staff_gate();
 if(g==DOOR)return d2(x-480,y-428)<38*38;
 if(g==BLUE)return d2(x-1411,y-983)<19*19;
 if(g==RIDE){float ox=p.x,oy=p.y;p.x=x;p.y=y;int which=street_near_car();int venue=placeNear(),shop=ordinary_door_near(),atm=atm_near();p.x=ox;p.y=oy;return which==jobRide&&venue<0&&shop<0&&atm<0;}
 if(g==CDOOR){int b=job_room();return b>=0&&d2(x-ordinaryDoorX[b],y-ordinaryDoorY[b])<20*20&&street_line_open(x,y,ordinaryDoorX[b],ordinaryDoorY[b],2);}
 if(g==JPOINT)return d2(x-job.xs[job.step],y-job.ys[job.step])<31*31&&street_line_open(x,y,cars[jobRide].x,cars[jobRide].y,2);
 if(g==CIRCUIT)return d2(x-loc[6].x,y-loc[6].y)<20*20;
 if(g==RADIO)return d2(x-431,y-1742)<19*19&&street_line_open(x,y,431,1742,2);
 if(g==WEXIT)return d2(x-480,y-430)<31*31;
 if(g==WPLAQUE)return d2(x-wi_plaque_x(),y-414)<17*17;
 if(g==TRACK){const InteriorProp*f=wi_f;float dx=x<f->x?f->x-x:x>f->x+f->w?x-f->x-f->w:0,dy=y<f->y?f->y-y:y>f->y+f->h?y-f->y-f->h:0;return d2(dx,dy)<32*32;}
 return 0;
}
static int pass(float x,float y){if(scene==4)return !wi_solid(x,y);if(scene==3)return !int_solid(x,y);return street_foot_clear(x,y,-1);}
static int path(int goalname){
 int room=scene==3||scene==4;float x=scene==3?interior_x:scene==4?wi_x:p.x,y=scene==3?interior_y:scene==4?wi_y:p.y,s=scene==3?NAMED_RUN_STEP:scene==4?WALKIN_RUN_STEP:FOOT_SPRINT_SPEED*(1.f/60);
 int ia=(int)(x/s),ib=(int)(y/s);assert(ia>=0&&ia<NX&&ib>=0&&ib<NY);
 xs[ia]=x;ys[ib]=y;for(int i=ia-1;i>=0;i--)xs[i]=xs[i+1]-s;for(int i=ia+1;i<NX;i++)xs[i]=xs[i-1]+s;for(int i=ib-1;i>=0;i--)ys[i]=ys[i+1]-s;for(int i=ib+1;i<NY;i++)ys[i]=ys[i-1]+s;
 int a=ib*NX+ia,to=-1,n=0,front=0,end=1,off[]={1,-NX,-1,NX},k[]={RIGHT,UP,LEFT,DOWN};
 memset(seen,-1,sizeof seen);q[0]=a;seen[a]=a;
 while(front<end){int u=q[front++];float x=xs[u%NX],y=ys[u/NX];if(goal(goalname,x,y)){to=u;break;}
 for(int i=0;i<4;i++){int v=u+off[i];if(v<=0||v>=N||abs(v%NX-u%NX)>1||seen[v]!=-1)continue;float x=xs[v%NX],y=ys[v/NX];if(x<0||x>ROOM||y<0||y>ROOM||(room&&(x>W||y>H))||!pass(x,y)){seen[v]=-2;continue;}
 q[end++]=v;seen[v]=u;came[v]=BRAKE|k[i];}}
 if(to<0)return -1;
 while(to!=a){out[n++]=came[to];to=seen[to];}
 return n;
}
static void travel(int g){
 for(int i=0;i<180;i++){
  int n=path(g);if(n==0)return;if(n<0){held(0,5);continue;}
  for(int j=0;j<n;j++){
   float x=p.x,y=p.y;key(out[n-1-j]);if(scene==1&&p.x==x&&p.y==y){held(0,7);break;}
  }
 }
 assert(0);
}
int main(int argc,char**argv){
 assert(argc==5);int cash=atoi(argv[1]),track=atoi(argv[2]),late=atoi(argv[3]),rail=cash==5;
 set_session_seed(8043);init_city();starter_save();sv.x=ordinaryDoorX[31];sv.y=ordinaryDoorY[31];sv.intro=4;sv.cash=cash;sv.dirty=18;sv.bank=73;sv.hash=savehash();
 FILE*f=fopen(argv[4],"wb");assert(f&&fwrite(&sv,1,sizeof sv,f)==sizeof sv);fclose(f);
 /* Mirror the launcher: first presented frame precedes "select career". */
 printf("0\n1024\n");len=2;profiles_activate();held(0,3);press(USE);assert(scene==4&&walkin_active==31);
 travel(99);press(USE);assert(vdPanel==2);record("sleeves");if(track==2)press(DOWN);press(ENTER);assert(a1Track==track);
 travel(TRACK);record("eligibility");held(ENTER,4);key(0);assert(walkin_active==(rail?69:31)&&sv.cash==cash-(rail?5:0));
 travel(WEXIT);press(USE);assert(scene==1);travel(RADIO);press(USE);assert(scene==4&&walkin_active==57);travel(TRACK);
 if(late)held(0,A1_SLOT+3);press(USE);assert(!a1Track&&a1Headline==track&&a1Late==late);record("onair");press(USE);assert(sv.cash==cash+95-(rail?5:0));
 travel(WEXIT);press(USE);assert(scene==1&&a1StreetFrames);record("headline");press(SAVE);held(0,3);
}
