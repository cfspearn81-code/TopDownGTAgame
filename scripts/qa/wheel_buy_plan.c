/* Synthetic career outside Ferry Market, followed by genuine controls. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static U sent[2048];static int count;
static void step(U k){assert(count<LEN(sent));sent[count++]=k;tick(k);}
static void hit(U k){step(k);step(0);}
enum{NX=640,NY=360};
static int queue[NX*NY];static signed char back[NX*NY];static U path[NX*NY];
static float xs[NX],ys[NY];
static void front(void){
 float s=NAMED_WALK_STEP;int wx=(int)(interior_x/s),wy=(int)(interior_y/s);
 xs[wx]=interior_x;ys[wy]=interior_y;
 for(int i=wx-1;i>=0;--i)xs[i]=xs[i+1]-s;
 for(int i=wx+1;i<NX;++i)xs[i]=xs[i-1]+s;
 for(int i=wy-1;i>=0;--i)ys[i]=ys[i+1]-s;
 for(int i=wy+1;i<NY;++i)ys[i]=ys[i-1]+s;
 int start=wy*NX+wx,head=0,end=0;queue[end++]=start;memset(back,-1,sizeof back);back[start]=0;
 int dst=-1,m[]={1,NX,-1,-NX};U ms[]={RIGHT,DOWN,LEFT,UP};
 while(head<end){
  int id=queue[head++],x=id%NX,y=id/NX;
  if(d2(xs[x]-interior_desk_x,ys[y]-interior_desk_y)<40*40){dst=id;break;}
  for(int d=0;d<4;d++){
   int q=id+m[d];if(q<0||q>=NX*NY||abs(q%NX-x)>1||back[q]!=-1)continue;
   if(int_solid(xs[q%NX],ys[q/NX])){back[q]=-2;continue;}
   back[q]=d+1;queue[end++]=q;
  }
 }
 assert(dst>=0);int n=0;while(dst!=start){int d=back[dst]-1;path[n++]=ms[d];dst-=m[d];}
 while(n)step(path[--n]);step(0);assert(int_at_desk());
}
int main(int argc,char**argv){
 assert(argc==2);set_session_seed(17);init_city();starter_save();sv.intro=4;sv.cash=150;sv.dirty=30;sv.bank=87;sv.x=loc[2].x;sv.y=loc[2].y;sv.hash=savehash();
 FILE*f=fopen(argv[1],"wb");assert(f&&fwrite(&sv,sizeof sv,1,f)==1);fclose(f);
 profiles_activate();sent[count++]=0;sent[count++]=ENTER;
 for(int i=0;i<4;i++)step(0);
 hit(USE);assert(scene==3&&interior_active_loc==2);front();hit(USE);
 assert(menu==2);hit(DOWN);hit(ENTER);hit(ENTER);
 assert(sv.medPatches==2&&sv.cash==100&&sv.rev==11);hit(ESC);
 step(WHEEL|USE_PATCH);assert(wheelOpen&&wheelFeedback==WW_FULL_HP&&sv.medPatches==2);
 for(int i=0;i<24;i++)step(WHEEL|USE_PATCH);step(0);hit(SAVE);step(0);
 printf("%d %08x\n",count,sv.hash);
 for(int i=0;i<count;){int j=i+1;while(j<count&&sent[j]==sent[i])j++;printf("%d %u\n",j-i,sent[i]);i=j;}
}
