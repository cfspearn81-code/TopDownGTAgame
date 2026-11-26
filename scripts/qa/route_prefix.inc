/* Deterministic native indoor playthrough. The street-side setup selects an
   address; actual USE, cardinal movement and USE again cover the entire visit.
   Compile: cc -std=c11 -O2 src/interior_routes_test.c -o build/room_routes
   Optional argument: existing capture directory, with 82 PPMs and rooms.tsv. */
#define DC_TEST
#include "game.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static U akeys[35000];static int total;static void record(U k){akeys[total++]=k; tick(k);} 
enum Goal { EXIT_G, DESK_G, FEATURE_G, GATE_G, CHIEF_G, CELLS_G, STATION_G, PLAQUE_G };
static const char *goals[]={"exit","counter","signature fixture","staff gate","chief","custody","shop station","visitor plaque"};
static const char *room_name;
static int g_kind, doors_seen, rears_seen, walkins_seen, route_count, steps_taken;
static FILE *manifest;
static const char *outdir;

static void fail(const char *why){
 fprintf(stderr,"FAIL [%s] %s (scene %d, rear %d, menu %d, player %.2f/%.2f)\n",room_name?room_name:"city",why,scene,interior_subroom,menu,
 (double)(scene==4?wi_x:interior_x),(double)(scene==4?wi_y:interior_y));exit(1);
}
static void ck(int ok,const char *why){if(!ok)fail(why);}
static void press_key(U key){record(key);record(0);}

static int matches(enum Goal g){
 if(scene==4){
#ifdef DC_STREET_ADDRESS_PLAQUES
  if(g==PLAQUE_G)return !wi_at_door()&&!wi_at_station()&&wi_at_plaque();
#endif
  return g==EXIT_G?wi_at_door():wi_at_station()&&!wi_at_door();
 }
 int hasgate=g_kind==4||legal_index_for_kind(g_kind)>=0;
 int atdoor=int_at_door(),atgate=hasgate&&int_at_staff_gate();
 if(g==EXIT_G)return atdoor;
 if(atdoor)return 0;
 if(g==GATE_G)return atgate;
 if(atgate)return 0;
 if(g==CHIEF_G)return int_at_chief();
 if(g==CELLS_G)return !int_at_chief()&&int_at_custody();
 if(g_kind==15&&(int_at_chief()||int_at_custody()))return 0;
 if(g==FEATURE_G)return int_feature_near(g_kind)&&!int_at_desk();
 return g==DESK_G&&int_at_desk();
}
/* A graph vertex is one actual cardinal walking tick from its neighbour.
   Generate float coordinates by repeated addition with the same step as the
   input code. A plan never moves the actor: restore after proximity probes,
   then replay every genuine key tick to reach and validate the destination. */
/* Capacity for cardinal walking steps down to 1.5 pixels per tick. */
enum{NX=W*2/3+2,NY=H*2/3+2,NODES=NX*NY};
static int qnode[NODES];
static signed char came[NODES]; /* -1 unvisited; -2 obstructed; 0 spawn; 1..4 */
static unsigned char rec[NODES];
static float pxs[NX],pys[NY];
static int axis(float *a,int cap,float start,float size,int end,int *origin){
 int i=(int)(start/size);ck(i>=0&&i<cap,"invalid entry coordinate");*origin=i;a[i]=start;
 for(int j=i;j>0;j--)a[j-1]=a[j]-size;
 for(int j=i+1;j<cap;j++){a[j]=a[j-1]+size;if(a[j]>end)return j;}
 fail("test grid exceeded framebuffer");return 0;
}
static void go(enum Goal g){
 int shop=scene==4;float *cx=shop?&wi_x:&interior_x,*cy=shop?&wi_y:&interior_y;
 float sx=*cx,sy=*cy,v=shop?WALKIN_WALK_STEP:NAMED_WALK_STEP;int ix,iy;
 int ww=axis(pxs,NX,sx,v,W,&ix),hh=axis(pys,NY,sy,v,H,&iy),start=iy*ww+ix;
 int delta[]={1,ww,-1,-ww};U key[]={RIGHT,DOWN,LEFT,UP};
 memset(came,-1,(size_t)(ww*hh));came[start]=0;qnode[0]=start;
 int read=0,n=1,found=-1;
 ck(!(shop?wi_solid(sx,sy):int_solid(sx,sy)),"actor spawned inside a solid!");
 while(read<n){
  int id=qnode[read++],x=id%ww,y=id/ww;
  *cx=pxs[x];*cy=pys[y];
  if(matches(g)){found=id;break;}
  for(int d=0;d<4;d++){
   if((d==0&&x+1==ww)||(d==1&&y+1==hh)||(d==2&&x==0)||(d==3&&y==0))continue;
   int nid=id+delta[d];if(came[nid]!=-1)continue;
   float nx=pxs[nid%ww],ny=pys[nid/ww];
   if(shop?wi_solid(nx,ny):int_solid(nx,ny)){came[nid]=-2;continue;}
   came[nid]=(signed char)(d+1);qnode[n++]=nid;
  }
 }
 *cx=sx;*cy=sy;
 if(found<0){fprintf(stderr,"No path to %s; %d reachable nodes examined.\n",goals[g],read);fail("blocked interaction");}
 int count=0;for(int cur=found;cur!=start;){int d=came[cur]-1;rec[count++]=(unsigned char)d;cur-=delta[d];}
 for(int i=count;i>0;i--)record(key[rec[i-1]]);
 record(0);steps_taken+=count;route_count++;
 if(!matches(g)){fprintf(stderr,"replay did not arrive at %s after %d key ticks\n",goals[g],count);fail("physics / route mismatch");}
}


