/* Independent Vault 39 QA planner. Adapted from the project's R2 key-route
 * harness; only creates a synthetic Rev10 career. Observation/route finding
 * use room collisions and restore coordinates before replaying any input.
 * For exact launcher verification see vault_keyboard_replay.py. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static U moves[18000]; static int count;
static void play(U mask){assert(count<18000);moves[count++]=mask;tick(mask);}
static void press(U mask){play(0);play(mask);play(0);}
static void waypoint(float tx,float ty){
 for(int i=0;i<700;i++){
   float dx=tx-p.x,dy=ty-p.y;
   if(absf(dx)<3&&absf(dy)<3)return;
   U k=BRAKE;
   if(absf(dx)>1.5f)k|=dx>0?RIGHT:LEFT;
   if(absf(dy)>1.5f)k|=dy>0?DOWN:UP;
   play(k);
 }
 fprintf(stderr,"stuck target %g,%g player %g,%g f%d\n",tx,ty,p.x,p.y,count);exit(2);
}
static void station(void){ /* Discretize the game's own walkable interior at its 2px single-tick stride. */
 enum{NX=480,NY=270,N=NX*NY};static int queue[N];static signed char parent[N];static U path[N];
 int xy[]={1,NX,-1,-NX};U masks[]={RIGHT,DOWN,LEFT,UP};int x0=(int)wi_x,y0=(int)wi_y,s=(y0/2)*NX+x0/2,n=1,w=0;
 queue[0]=s;memset(parent,-1,sizeof(parent));parent[s]=4;int found=-1;
 while(w<n){int i=queue[w++];float x=(i%NX)*2.f+x0%2,y=(i/NX)*2.f+y0%2;wi_x=x;wi_y=y;
  if(wi_at_station()&&!wi_at_door()){found=i;break;}
  for(int d=0;d<4;d++){int j=i+xy[d];if(j<0||j>=N||parent[j]>=0||wi_solid((j%NX)*2.f+x0%2,(j/NX)*2.f+y0%2))continue;
  parent[j]=d;queue[n++]=j;}
 }
 wi_x=x0;wi_y=y0;assert(found>=0);int m=0;while(found!=s){int d=parent[found];path[m++]=masks[d];found-=xy[d];}
 while(m)play(path[--m]);play(0);assert(wi_at_station()&&!wi_at_door());
}
static void state(const char*label){fprintf(stderr,"%-18s frame=%d scene=%d hall=%d cash=%d job=%d notice=[%s] player=%g/%g\n",label,count,scene,walkin_active,sv.cash,job.kind,notice,p.x,p.y);}
static void export(void){printf("%d 0\n",count);for(int i=0;i<count;){int j=i+1;while(j<count&&moves[i]==moves[j])j++;printf("%d %d\n",j-i,moves[i]);i=j;}}

/* Plan a keyboard route through the current simulated geometry. Queries only:
   the shipped binary receives the resulting keyboard plan, never game state. */
enum{GS=8,GX=256,GN=GX*GX};
#define POSX(u) (((u)%GX)*GS)
#define POSY(u) (((u)/GX)*GS)
static int safe(float x,float y){return !solid(x,y,11)&&street_foot_car_clear(x,y,-1,7);}
static void overland(int tx,int ty){
 static int q[GN],pre[GN],seen[GN];static int epoch;
 static int mx[]={-1,1,0,0,-1,-1,1,1},my[]={0,0,-1,1,-1,1,-1,1};
 for(int re=0;re<1300;re++){
 int u=cl((int)((p.x+4)/8),1,254)+GX*cl((int)((p.y+4)/8),1,254),sn=u;
 int tar=cl((tx+4)/8,1,254)+GX*cl((ty+4)/8,1,254);
 if(d2(p.x-tx,p.y-ty)<5*5)return;
 if(sn==tar){waypoint(tx,ty);return;}
 int tail=1,head=0;seen[u]=++epoch;q[0]=u;pre[u]=-1;int result=-1;
 while(head<tail&&result<0){u=q[head++];
 for(int dir=0;dir<8;dir++){
  int v=u+mx[dir]+GX*my[dir];if((unsigned)v>=GN||seen[v]==epoch||abs(POSX(v)-POSX(u))>GS)continue;
  float x=POSX(v),y=POSY(v);
  if(!safe(x,y))continue;
  if(dir>3&&(!safe(x,POSY(u))||!safe(POSX(u),y)))continue;
  seen[v]=epoch;pre[v]=u;q[tail++]=v;if(v==tar){result=v;break;}
 }}
 if(result<0){for(int k=0;k<20;k++)play(0);continue;}
 int best=result;
 while(pre[best]!=sn)best=pre[best];
 int bx=POSX(best),by=POSY(best);
 for(int k=0;k<8;k++){float dx=bx-p.x,dy=by-p.y;
 U m=BRAKE;if(absf(dx)>1.5)m|=dx>0?RIGHT:LEFT;if(absf(dy)>1.5)m|=dy>0?DOWN:UP;
 if(m==BRAKE)break;play(m);}
 }
 fprintf(stderr,"overland route stalled player=%g,%g\n",p.x,p.y);exit(2);
}

enum {want_gate_x=V39_STAFF_X,want_gate_y=V39_STAFF_Y};
static int named_target(int where){
 if(where==2)return !int_at_door()&&!int_at_desk()&&d2(interior_x-want_gate_x,interior_y-want_gate_y)<15*15;
 return where==1?int_at_door():int_at_desk()&&!int_at_door();
}
static void named_inside(int where){
 enum{NX=512,NY=300,N=NX*NY};static float xx[NX],yy[NY];static int q[N];static signed char pre[N];static U steps[N];
 float sx=interior_x,sy=interior_y,step=NAMED_WALK_STEP;int ix=(int)(sx/step),iy=(int)(sy/step),root=iy*NX+ix;
 xx[ix]=sx; yy[iy]=sy;
 for(int i=ix+1;i<NX;i++)xx[i]=xx[i-1]+step;for(int i=ix-1;i>=0;i--)xx[i]=xx[i+1]-step;
 for(int i=iy+1;i<NY;i++)yy[i]=yy[i-1]+step;for(int i=iy-1;i>=0;i--)yy[i]=yy[i+1]-step;
 q[0]=root;memset(pre,-1,sizeof pre);pre[root]=4;int n=1,h=0,found=-1;
 int delta[]={1,NX,-1,-NX};U masks[]={RIGHT,DOWN,LEFT,UP};
 while(h<n){int i=q[h++];interior_x=xx[i%NX];interior_y=yy[i/NX];
 if(named_target(where)){found=i;break;}
 for(int d=0;d<4;d++){int j=i+delta[d];if(j<0||j>=N||abs(j%NX-i%NX)>1||pre[j]>=0||int_solid(xx[j%NX],yy[j/NX]))continue;pre[j]=d;q[n++]=j;}}
 interior_x=sx;interior_y=sy;assert(found>=0);int m=0;while(found!=root){int d=pre[found];steps[m++]=masks[d];found-=delta[d];}
 while(m)play(steps[--m]);play(0);assert(named_target(where));
}
static void leave_puzzle(void){ /* Reverse directions across the same accessible station route, then interact. */
 enum{NX=480,NY=270,N=NX*NY};static int q[N];static signed char pre[N];static U moves2[N];
 const int x0=wi_x,y0=wi_y,root=(y0/2)*NX+x0/2;int n=1,h=0,end=-1;
 int delta[]={1,NX,-1,-NX};U masks[]={RIGHT,DOWN,LEFT,UP};q[0]=root;memset(pre,-1,sizeof pre);pre[root]=4;
 while(h<n){int i=q[h++];wi_x=(i%NX)*2+x0%2;wi_y=(i/NX)*2+y0%2;
 if(wi_at_door()){end=i;break;}
 for(int d=0;d<4;d++){int j=i+delta[d];if(j<0||j>=N||pre[j]>=0||wi_solid((j%NX)*2+x0%2,(j/NX)*2+y0%2))continue;pre[j]=d;q[n++]=j;}}
 wi_x=x0;wi_y=y0;assert(end>=0);int m=0;while(end!=root){int d=pre[end];moves2[m++]=masks[d];end-=delta[d];}
 while(m)play(moves2[--m]);press(USE);assert(scene==1);
}
static void note(const char *tag){state(tag);fprintf(stderr,"    heat=%.2f quiet=%.2f p.hp=%d dirty=%d cash=%d seen=%d engaged=%d\n",p.heat,p.quiet,p.hp,sv.dirty,sv.cash,dispatch_visible(),dispatch_engaged());}
static void frame(const char*tag){
 note(tag);
 const char*probe=getenv("DC_VAULT_PROBES");
 if(probe){
  FILE *fp=fopen(probe,"a");assert(fp);paint();
  /* Pixel hashes for the in-world viewport plus subtitle only, without
     the day clock, wallet or Save flash. */
  int x=20,y=488,w=800,h=45;
  if(tag[0]=='A'){x=515;y=145;w=200;h=155;} /* service and cup motions */
  else if(tag[1]=='3'){x=565;y=100;w=310;h=104;} /* Employees Only / doorway */
  else if(tag[1]=='4'){x=370;y=166;w=180;h=120;} /* visible Ivo + booth */
  else if(tag[1]=='5'){x=169;y=200;w=620;h=170;} /* Ivo's contract list */
  U hsh=2166136261u;for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)hsh=(hsh^(pixels[j*W+i]&0xffffffu))*16777619u;
  fprintf(fp,"%d %d %d %d %d %08x %s\n",count,x,y,w,h,hsh,tag);fclose(fp);
 }
 const char*path=getenv("DC_VAULT_CAP");if(!path)return;
 char fn[512];snprintf(fn,sizeof fn,"%s/%s.ppm",path,tag);FILE*f=fopen(fn,"wb");assert(f);
 paint();fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int j=0;j<W*H;j++){U c=pixels[j];fputc(c>>16,f);fputc(c>>8,f);fputc(c,f);}fclose(f);
}
static void enter_front(void){
 assert(scene==1);overland(loc[1].x,loc[1].y);press(USE);
 assert(scene==3&&interior_active_loc==1&&!interior_subroom);
}
static void leave_vault(void){
 if(menu>=0)press(ESC);
 if(interior_subroom){
  named_inside(1);press(USE);
  assert(scene==3&&interior_active_loc==1&&!interior_subroom);
 }
 named_inside(1);press(USE);assert(scene==1&&p.x==loc[1].x&&p.y==loc[1].y);
}
static void enter_rear(int hold){
 named_inside(2);
 if(hold){for(int i=0;i<27;i++)play(USE);play(0);}else press(USE);
 assert(scene==3&&interior_active_loc==1&&interior_subroom&&menu<0);
 assert(p.noticeT<=0||strncmp(notice,"Mika",4));
}
static void expect_options(void){
 assert(nopt>=4&&opt[0].action==14&&opt[1].action==15&&opt[2].action==16&&opt[3].action==17);
}
int main(int argc,char**argv){
 assert(argc==2&&"usage: plan starter.dat");
 set_session_seed(17);init_city();starter_save();sv.x=loc[1].x;sv.y=loc[1].y;
 sv.intro=3;sv.cash=70;sv.hp=83;sv.hash=savehash();
 FILE*f=fopen(argv[1],"wb");assert(f&&fwrite(&sv,1,sizeof sv,f)==sizeof sv);fclose(f);
 moves[count++]=0;moves[count++]=ENTER;profiles_activate();play(0);play(0);play(0);
 enter_front();press(GFX);
 assert(fastGfx);named_inside(0);frame("01-public-bar");press(USE);frame("02-bar-response");
 assert(scene==3&&!interior_subroom&&job.kind<0&&sv.dirty==0&&menu<0&&(v39.requested||v39.playerTick));
 press(USE);assert(v39.requested||v39.playerTick);assert(strstr(notice,"have your order"));
 for(int i=0;i<240&&!(v39.tick>=125&&v39.tick<140);i++)play(0);
 assert(v39.serves==1&&v39.tick>=125&&v39.tick<140);frame("A1-patron-served");
 for(int i=0;i<310&&v39.playerTick<68;i++)play(0);
 assert(v39.playerTick>=68);frame("A2-soda-handoff");
 for(int i=0;i<120;i++)play(0);
 assert(v39.playerServed==1&&menu<0&&sv.cash==70&&p.hp==83);frame("A3-player-served");
 press(USE);assert(!v39.playerServed&&!v39.requested&&!v39.playerTick&&menu<0&&sv.cash==70&&strstr(notice,"Enjoy"));
 if(menu>=0)press(ESC);named_inside(2);frame("03-employees-door");
 enter_rear(1);frame("04-hideout");
 named_inside(0);press(USE);expect_options();frame("05-ivo-contracts");
 for(int i=0;i<4;i++)press(DOWN);assert(opt[sel].action==97);press(ENTER);
 assert(menu==1&&job.kind<0&&!strncmp(notice,"Ivo:",4));
 for(int i=0;i<4;i++)press(UP);
 press(ENTER);assert(job.kind==JOB_VAULT&&job.step==0);leave_vault();
 const int hall=job_room();assert(hall>=0);
 overland(ordinaryDoorX[hall],ordinaryDoorY[hall]);press(USE);assert(scene==4&&walkin_active==hall);
 station();press(USE);assert(job.kind==JOB_VAULT&&job.step==1&&job_venue()==1&&p.heat==105);
 leave_puzzle();enter_front();named_inside(0);press(USE);
 assert(job.kind==JOB_VAULT&&job.step==1&&sv.dirty==0&&menu!=1);
 if(menu>=0)press(ESC);
 enter_rear(0);named_inside(0);assert(p.heat>0);frame("06-hot-prompt");
 press(USE);assert(menu<0&&p.heat>0&&job.step==1&&!sv.dirty);frame("07-hot-refusal");
 const float old_heat=p.heat;for(int i=0;i<120;i++)play(0);assert(p.heat==old_heat);
 leave_vault();overland(550,767);
 for(int i=0;i<1300&&(p.heat>0||dispatch_engaged());i++){assert(!p.dead);play(0);}
 assert(p.heat==0);enter_front();named_inside(0);press(USE);
 assert(job.kind==JOB_VAULT&&job.step==1&&sv.dirty==0);
 if(menu>=0)press(ESC);enter_rear(0);named_inside(0);frame("08-ready-prompt");
 int wallet=sv.cash;press(USE);assert(job.kind<0&&sv.dirty==260&&sv.cash==wallet&&sv.crime==1&&sizeof(Save)==304);
 frame("09-payout");press(USE);expect_options();assert(sv.dirty==260&&sv.crime==1);
 press(ESC);press(SAVE);leave_vault();press(SAVE);export();return 0;
}
