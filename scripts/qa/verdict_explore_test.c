/* End-to-end native engine test. Compile alongside the shared verdict core:
   cc -std=c11 -O2 -I src scripts/qa/verdict_explore_test.c -o /tmp/ex-test
   /tmp/ex-test [existing frame-output-directory]
   Only initial career/location is staged. All movement, accepted listings,
   physical visits, incorrect/correct actions and outcomes use game input. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static int frames,visits,paths,replans;static const char *dir,*planDir;static FILE *tape,*probes;
static void check(int ok,const char*what){
 if(ok)return;
 fprintf(stderr,"FAIL %s / scene %d bldg %d outdoor %.2f,%.2f indoor %.2f,%.2f active %d @ %d: %s\n",
  what,scene,walkin_active,p.x,p.y,wi_x,wi_y,vdActive,frames,notice);exit(1);
}
static void k(U m){if(tape)fprintf(tape,"%u\n",m);tick(m);frames++;check(!p.dead,"survived walk");}
static void hit(U m){k(m);k(0);}
static void shot(const char*name){
 paint();
 if(probes){
  static const int regions[][4]={{10,208,205,20},{549,288,60,48},{373,332,190,18},{666,116,252,153},{666,116,252,153},{675,118,247,170},{675,118,247,170},{16,213,285,140},{16,213,285,140}};
  const int*r=regions[atoi(name)-1];U hash=2166136261u;
  for(int y=r[1];y<r[1]+r[3];y++)for(int x=r[0];x<r[0]+r[2];x++)hash=(hash^(pixels[y*W+x]&0xffffffu))*16777619u;
  fprintf(probes,"%d %d %d %d %d %08x %s\n",frames,r[0],r[1],r[2],r[3],hash,name);
 }
 if(!dir)return;char path[512];snprintf(path,sizeof path,"%s/%s.ppm",dir,name);
 FILE*f=fopen(path,"wb");check(!!f,"frame output");fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc(c>>16,f);fputc(c>>8,f);fputc(c,f);}fclose(f);
}
/* Sample a coarse graph, then replay cardinal sprint keys through actual
   movement/collision. When traffic moves into the road, plan anew. */
enum {EX_GRID=490, EX_CAP=EX_GRID*EX_GRID};
static int parent[EX_CAP],qbuf[EX_CAP];static unsigned char moves[EX_CAP];
static void go(int tx,int ty,int rad){
 static const U km[]={RIGHT,DOWN,LEFT,UP};paths++;
 for(int retry=0;retry<100;retry++){
  int inside=scene==4,mul=inside?2:4;float stride=inside?WALKIN_RUN_STEP:FOOT_SPRINT_SPEED/60.f;
  float ax=inside?wi_x:p.x,ay=inside?wi_y:p.y,ds=mul*stride;
  if(d2(ax-tx,ay-ty)<=rad*rad)return;
  int ox=(int)(ax/ds),oy=(int)(ay/ds),nx=(int)((inside?W:ROOM)/ds)+1,ny=(int)((inside?H:ROOM)/ds)+1;
  int root=oy*nx+ox,found=-1,rd=0,n=0;check(nx*ny<=EX_CAP,"graph cap");
  memset(parent,0xff,(size_t)nx*ny*sizeof *parent);qbuf[n++]=root;parent[root]=-2;
  int adj[]={1,nx,-1,-nx};
  while(rd<n){
   int id=qbuf[rd++],ix=id%nx,iy=id/nx;float xx=ax+(ix-ox)*ds,yy=ay+(iy-oy)*ds;
   if(d2(xx-tx,yy-ty)<rad*rad){found=id;break;}
   for(int j=0;j<4;j++){
    if((j==0&&ix==nx-1)||(j==1&&iy==ny-1)||(j==2&&ix==0)||(j==3&&iy==0))continue;
    int h=id+adj[j];if(parent[h]!=-1)continue;
    float vx=ax+(h%nx-ox)*ds,vy=ay+(h/nx-oy)*ds;
    if(inside?wi_solid(vx,vy):!street_foot_clear(vx,vy,-1)){parent[h]=-3;continue;}
    parent[h]=j;qbuf[n++]=h;
   }
  }
  check(found!=-1,"approachable target");int cnt=0;
  for(int at=found;at!=root;){int j=parent[at];moves[cnt++]=(unsigned char)j;at-=adj[j];}
  int blocked=0;
  for(int ix=cnt-1;ix>=0&&!blocked;ix--)for(int it=0;it<mul;it++){
   int j=moves[ix];float bx=inside?wi_x:p.x,by=inside?wi_y:p.y;k(km[j]|BRAKE);
   float nx=inside?wi_x:p.x,ny=inside?wi_y:p.y;
   if(absf(nx-bx)+absf(ny-by)<stride*.7f){blocked=1;break;}
  }
  if(blocked){replans++;for(int it=0;it<8;it++)k(0);}else k(0);
 }
 check(0,"too many traffic retries");
}
static void room(int b){
 check(scene==1,"approach from street");go(ordinaryDoorX[b],ordinaryDoorY[b],12);
 hit(USE);check(ex_room(b),"entered actual door");visits++;
}
static void fixture(int i){const InteriorProp*f=wi_f+i;go(f->x+f->w/2,f->y+f->h+14,8);check(ex_fixture_near(walkin_active,i),"at correct fixture");}
static void leave(void){go(W/2,421,12);hit(USE);check(scene==1,"left actual door");}
static void board(int id){
 hit(ACTIVITIES);check(vdPanel==1,"open listings");
 while(vdSelected!=id)hit(DOWN);hit(ENTER);check(vdActive==id,"choose listing");
}
static void study(int b){room(b);go(wi_plaque_x(),414,10);hit(USE);check(!strcmp(notice,wiPlaque[b]),"ordinary plaque text retained");leave();}
static void all(void){
 if(planDir)set_session_seed(8043);
 init_city();sv.intro=3;sv.cash=sv.dirty=sv.bank=sv.owned=0;
 if(planDir){
  char file[512];snprintf(file,sizeof file,"%s/start.dat",planDir);FILE*f=fopen(file,"wb");check(!!f,"fixture output");
  sv.x=ordinaryDoorX[EX_HOME];sv.y=ordinaryDoorY[EX_HOME];sv.hash=savehash();check(fwrite(&sv,1,sizeof sv,f)==sizeof sv,"write fixture");fclose(f);
  snprintf(file,sizeof file,"%s/keys",planDir);tape=fopen(file,"w");snprintf(file,sizeof file,"%s/art",planDir);probes=fopen(file,"w");check(tape&&probes,"open replay files");
  fputs("0\n1024\n",tape);frames=2;profiles_activate();k(0);k(0);k(0);
 }else{p.x=ordinaryDoorX[EX_HOME];p.y=ordinaryDoorY[EX_HOME];p.car=-1;scene=1;menu=-1;job.kind=-1;mapOn=0;k(0);}
 hit(GFX);check(fastGfx,"Low FX");hit(MAP);if(planDir){hit(RIGHT);hit(ENTER);}else pointer_input(676,90,1);check(navHasManual,"set independent personal pin");hit(MAP);
 NavPoint pin=navManual;
 board(VA_A4);check(vdGoalX==ordinaryDoorX[EX_HOME],"start has lobby route");room(EX_HOME);fixture(2);hit(ENTER);
 check(ex_supper_step==1&&vdGoalX<0,"note clears only listing goal");leave();
 go(ordinaryDoorX[EX_OVEN]+28,ordinaryDoorY[EX_OVEN]+25,10);shot("01-bakery-sign");room(EX_OVEN);
 fixture(0);int before=wi_sw[EX_OVEN];hit(USE);check(wi_sw[EX_OVEN]!=before,"oven bell still usable");hit(ENTER);check(ex_supper_step==2,"baker gets note");leave();
 room(EX_HOME);fixture(2);shot("02-place-set");hit(ENTER);check(ex_supper_done&&vdActive<0,"supper finish");leave();

 board(VA_B3A);room(EX_PAPER);fixture(3);hit(ENTER);check(ex_photo_step==1,"received camera");leave();
 room(EX_KELP);go(ex_mark_x(),ex_mark_y(),7);hit(ENTER);
 check(!ex_photo_done&&strstr(notice,"dark"),"lamp needed");
 fixture(0);hit(USE);check(wi_sw[EX_KELP],"night lamp on");go(ex_mark_x(),ex_mark_y(),7);
 hit(DOWN);hit(ENTER);check(!ex_photo_done&&strstr(notice,"Turn"),"camera facing required");
 hit(UP);shot("03-tank-framing");hit(ENTER);check(ex_photo_done&&vdActive<0,"still filed");shot("04-news-still");leave();
 room(EX_PAPER);fixture(3);check(ex_photo_done,"page remains on rack");shot("05-rack");leave();

 board(VA_B3B);room(EX_PROOF);fixture(3);shot("06-uncorrected-proof");hit(ENTER);hit(ENTER);
 check(!vdPanel&&!ex_bill_clues,"must read clue first");leave();
 study(EX_CIRCLE);check(ex_bill_clues==2,"negative clue accepted");
 room(EX_PROOF);fixture(3);hit(ENTER);hit(DOWN);hit(ENTER);
 check(vdActive==VA_B3B&&!ex_bill_done&&strstr(notice,"straight row"),"wrong venue explains and permits retry");
 leave();study(EX_FOOT);check(ex_bill_clues==3,"correct plaque also accepted");
 room(EX_PROOF);fixture(3);hit(ENTER);hit(ENTER);
 check(ex_bill_done&&vdActive<0,"proof corrected");shot("07-corrected-proof");leave();

 board(VA_B3C);room(EX_DESK);fixture(6);hit(ENTER);check(ex_dial_step==1&&vdGoalX<0,"receiver acquired");leave();
 check(street_foot_clear(EX_SOURCE_X,EX_SOURCE_Y,-1),"source footing clear");
 go(EX_SOURCE_X,EX_SOURCE_Y,14);hit(ENTER);check(!ex_dial_done,"untriangulated cabinet cannot finish");
 int order[]={2,0,0,1};for(int j=0;j<4;j++){
  int i=order[j];go(ex_post_x(i),ex_post_y(i),14);hit(ENTER);
  check(ex_dial_mask&(1<<i),"sample logged");
  if(j==1||j==2)check(ex_dial_mask==5,"no duplicate sample credit");
 }
 check(ex_dial_mask==7,"three individual block readings");shot("08-directional-meter");go(EX_SOURCE_X,EX_SOURCE_Y,12);shot("09-source");hit(ENTER);
 check(ex_dial_done&&vdActive<0&&!ex_dial_step,"source found");
 check(navHasManual&&nav_same(pin,navManual),"player pin preserved for all routes");
 check(!sv.cash&&!sv.bank&&!sv.owned&&!sv.dirty&&sizeof(Save)==304,"no save or economic changes");
 room(EX_DESK);fixture(6);hit(ENTER);check(vdActive==VA_B3C&&ex_dial_mask==0,"replay resets readings");
 hit(ACTIVITIES);hit(DOWN);hit(ENTER);check(vdActive<0&&!ex_dial_step,"ordinary cancellation");
 if(planDir){hit(SAVE);k(0);k(0);fclose(tape);tape=0;fclose(probes);probes=0;}
 vd_reset();check(!ex_bill_done&&!ex_dial_done&&!ex_supper_done&&!ex_photo_done,"session artifacts reset");
 printf("PASS: %d real input ticks; %d indoor visits, %d navigated legs, %d replans. Four activities, Low FX, manual pin, money, replay/reset.\n",frames,visits,paths,replans);
}
int main(int argc,char**argv){if(argc>1)dir=argv[1];planDir=getenv("DC_EX_PLAN_DIR");all();return 0;}
