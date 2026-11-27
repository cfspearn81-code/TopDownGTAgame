/* Test-only physical input + real Linux save-shutdown/reload fixture. */
#include <stdio.h>
#include "game.c"
#include "linux_save.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
static void wf_key(U k){tick(0);tick(k);tick(0);}
static void wf_walk(U k,int ticks){for(int i=0;i<ticks;i++)tick(k);tick(0);}
static void wf_stand(OuterPoint pt){
 assert(!solid(pt.x,pt.y,8));scene=1;p.car=-1;menu=-1;mapOn=0;job.kind=-1;
 assert(!p.dead);p.x=pt.x;p.y=pt.y;
}
static void wf_restart(void){
 request_save();assert(saveFlash>0);linux_disk_shutdown();
 assert(linux_disk_init());assert(linux_disk_lock());disk_slot(0);
 init_city();assert(wason);scene=1;
}
static void wf_screen(const char*name){
 if(!name)return;FILE*f=fopen(name,"wb");assert(f);
 paint();fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U a=pixels[i];fputc(a>>16,f);fputc(a>>8,f);fputc(a,f);}
 fclose(f);
}
static void wf_to_ranger(void){
 wf_stand(woodline_sites[WOOD_RANGER]);wf_key(USE);
 assert(scene==NIGHT_FOREST_SCENE&&wf_mode==WF_RANGER&&wf_row==0);
}
static void wf_to_shop(int shop){
 int j=shop==0?4:7;wf_stand((OuterPoint){loc[j].x,loc[j].y});wf_key(USE);
 assert(scene==3&&interior_active_loc==j&&!interior_subroom);
 wf_walk(RIGHT,38);wf_walk(UP,19);assert(wf_bench_near());
 wf_key(ENTER);assert(scene==NIGHT_FOREST_SCENE&&wf_mode==WF_PLANTER&&wf_shop==shop&&wf_row==0);
}
static void wf_row_to(int row){while(wf_row!=row)wf_key(DOWN);}
static void wf_check_geography(void){
 for(int i=0;i<WOOD_N_SITES;i++)assert(!solid(woodline_sites[i].x,woodline_sites[i].y,8));
 for(int i=0;i<LEN(wf_patches);i++){OuterPoint q=wf_point(wf_patches+i);if(solid(q.x,q.y,8))fprintf(stderr,"Blocked patch %d (%d,%d)\n",i,q.x,q.y);assert(!solid(q.x,q.y,8));}
 for(int i=0;i<LEN(wf_chests);i++){OuterPoint q=wf_point(wf_chests+i);assert(!solid(q.x,q.y,8));}
 enum{NX=123,NY=116};static unsigned char seen[NX*NY];static int stack[NX*NY];
 /* Collision-checked route through the historic southwest seawall. */
 const int start=(1992-1960)/8*NX+520/8;int front=0,tail=0;stack[tail++]=start;seen[start]=1;
 assert(!solid(520,1992,8));
 while(front<tail){
  int t=stack[front++],x=t%NX,y=t/NX;
  const int off[]={-1,1,-NX,NX};
  for(int j=0;j<4;j++){if((j==0&&x==0)||(j==1&&x==NX-1)||(j==2&&y==0)||(j==3&&y==NY-1))continue;
   int n=t+off[j],nx=n%NX*8,ny=n/NX*8+1960;
   if(!seen[n]&&!solid(nx,ny,8)&&!solid((nx+x*8)*.5f,(ny+y*8+1960)*.5f,8)){seen[n]=1;stack[tail++]=n;}}
 }
 for(int i=0;i<WOOD_N_SITES;i++){OuterPoint q=woodline_sites[i];int ok=0;
  for(int dx=-2;dx<=2;dx++)for(int dy=-2;dy<=2;dy++){int x=q.x/8+dx,y=(q.y-1960)/8+dy;
   if(seen[y*NX+x]&&d2(q.x-x*8,q.y-(1960+y*8))<23*23)ok=1;}
  if(!ok)fprintf(stderr,"No Forest walking route to site %d\n",i);
  assert(ok);}
}
static void wf_full_quest(void){
 int cash=sv.cash;
 wf_stand(wf_point(wf_chests));wf_key(USE);assert(!wf_has(WF_SUPPLY)&&sv.cash==cash);
 wf_stand(woodline_sites[wf_stamps[0]]);wf_key(USE);assert(!wf_stamps_done());
 wf_to_ranger();wf_key(USE);assert(wf_has(WF_SURVEY_BEGIN));
 wf_key(MAP);assert(navHasManual&&navManual.x==woodline_sites[WOOD_FERN].x);wf_key(ESC);
 for(int i=0;i<LEN(wf_stamps);i++){wf_stand(woodline_sites[wf_stamps[i]]);wf_key(USE);}
 wf_restart();assert(wf_stamps_done()==7u&&wf_has(WF_SURVEY_BEGIN));
 wf_to_ranger();wf_key(USE);assert(wf_has(WF_SURVEY_PAID)&&sv.cash==cash+90);
 wf_key(USE);assert(sv.cash==cash+90);
 wf_key(DOWN);wf_key(ENTER);assert(wf_has(WF_CATALOG_BEGIN));wf_key(ESC);
 for(int i=0;i<LEN(wf_patches);i++){wf_stand(wf_point(wf_patches+i));wf_key(USE);wf_key(USE);}
 assert(wf_species()==4&&wf_gathered()==255u);for(int i=0;i<4;i++)assert(wf_inv(i)==2);
 wf_to_ranger();wf_key(DOWN);wf_key(ENTER);assert(wf_has(WF_CATALOG_PAID)&&sv.cash==cash+210);
 for(int i=0;i<4;i++)assert(wf_inv(i)==1);
 wf_key(ENTER);assert(sv.cash==cash+210);
 wf_key(DOWN);wf_key(ENTER);assert(wf_has(WF_SALVAGE_BEGIN));wf_key(ESC);
 wf_stand(wf_point(wf_chests+1));wf_key(USE);assert(!wf_has(WF_RUIN));
 wf_stand(wf_point(wf_chests));wf_key(USE);wf_key(USE);assert(wf_has(WF_SUPPLY)&&sv.cash==cash+238);
 wf_stand(wf_point(wf_chests+1));wf_key(USE);wf_key(USE);assert(wf_has(WF_RUIN)&&sv.cash==cash+283);
 wf_restart();assert(wf_has(WF_SUPPLY|WF_RUIN)&&!wf_has(WF_SALVAGE_PAID));
 wf_to_ranger();wf_row_to(2);wf_key(ENTER);assert(wf_has(WF_SALVAGE_PAID)&&sv.cash==cash+448);
 wf_key(MAP);assert(navManual.x==wf_point(wf_chests+2).x);wf_key(ESC);
 wf_stand(wf_point(wf_chests+2));sv.cash=WALLET_LIMIT;wf_key(USE);assert(!wf_has(WF_FINAL));sv.cash=cash+448;
 wf_key(USE);wf_key(USE);assert(wf_has(WF_FINAL)&&sv.cash==cash+533&&wf_inv(3)==2);
 wf_restart();wf_stand(wf_point(wf_chests+2));wf_key(USE);assert(wf_has(WF_FINAL)&&sv.cash==cash+533&&wf_inv(3)==2);
}
static void wf_test_planters(const char*preview){
 sv.b[0].own=0;int cash=sv.cash;
 wf_to_shop(0);wf_row_to(4);wf_key(ENTER);assert(wf_bed_crop(0,0)<0&&sv.cash==cash);wf_key(ESC);
 sv.b[0]=(Business){1,0,1,0,0,0,0};
 nv.phone[NP_MODULES]=3;
 for(int shop=0;shop<2;shop++)for(int crop=0;crop<LEN(wf_ticks);crop++){
  int bed=crop%2,before=crop<4?wf_inv(crop):np_get(NP_BAG+(crop-4)*3,99);
  wf_to_shop(shop);if(bed)wf_key(RIGHT);wf_row_to(crop);
  int prior=sv.cash;wf_key(ENTER);assert(wf_bed_crop(shop,bed)==crop&&wf_bed_left(shop,bed)==wf_ticks[crop]&&sv.cash==prior-wf_fees[crop]);
  wf_key(ENTER);assert(wf_bed_left(shop,bed)==wf_ticks[crop]&&sv.cash==prior-wf_fees[crop]);
  if(crop==0&&shop==0)wf_screen(preview);
  wf_key(ESC);assert(scene==3);
  if(crop==3){int left=wf_bed_left(shop,bed);wf_restart();assert(wf_bed_crop(shop,bed)==crop&&wf_bed_left(shop,bed)==left);}
  for(int i=0;i<wf_ticks[crop];i++)woodline_world_tick(1.f/60.f);
  assert(!wf_bed_left(shop,bed));
  wf_to_shop(shop);if(bed)wf_key(RIGHT);
  if(crop==3&&shop==1){wf_inventory(3,31);wf_key(ENTER);assert(wf_bed_crop(shop,bed)==crop);wf_inventory(3,before);}
  wf_key(ENTER);
  assert(wf_bed_crop(shop,bed)<0);
  assert((crop<4?wf_inv(crop):np_get(NP_BAG+(crop-4)*3,99))==before+1);
  wf_key(ESC);
 }
 /* All four independent bed halves may be active at once. Reload must
  * preserve each code/time without borrowing a neighboring shop's bits. */
 for(int shop=0;shop<2;shop++){wf_to_shop(shop);wf_row_to(shop*2);wf_key(ENTER);
  wf_key(RIGHT);wf_row_to(shop*2+1);wf_key(ENTER);wf_key(ESC);}
 U packed0=nv.garage[WF_PLANT_LANTERN],packed1=nv.garage[WF_PLANT_KIOSK];
 wf_restart();assert(nv.garage[WF_PLANT_LANTERN]==packed0&&nv.garage[WF_PLANT_KIOSK]==packed1);
 for(int shop=0;shop<2;shop++)for(int b=0;b<2;b++)
  assert(wf_bed_crop(shop,b)==shop*2+b&&wf_bed_left(shop,b)>0);
 for(int t=0;t<wf_ticks[3];t++)woodline_world_tick(1.f/60.f);
 for(int shop=0;shop<2;shop++){wf_to_shop(shop);wf_key(ENTER);wf_key(RIGHT);wf_key(ENTER);wf_key(ESC);}
 wf_restart();assert(nv.garage[WF_PLANT_LANTERN]==0&&nv.garage[WF_PLANT_KIOSK]==0);
 for(int k=0;k<3;k++)assert(np_get(NP_BAG+k*3,99)==2);
}
int main(int argc,char**argv){
 assert(getenv("XDG_DATA_HOME"));assert(linux_disk_init()&&linux_disk_lock());disk_slot(0);init_city();
 scene=1;p.dead=0;p.car=-1;p.hp=100;sv.cash=2000;sv.intro=4;menu=-1;
 if(argc==3&&!strcmp(argv[1],"--seed")){
  const char*s=argv[2];int item=atoi(s);
  if(*s=='p'){int shop=s[1]=='1',j=shop?7:4;p.x=loc[j].x;p.y=loc[j].y;nv.garage[WF_FLAGS]=0xffu<<WF_PATCH_SHIFT;}
  else if(item==-2){p.x=520;p.y=1992;sv.dayClock=610.f;}
  else if(item<0){p.x=woodline_sites[WOOD_RANGER].x;p.y=woodline_sites[WOOD_RANGER].y;}
  else{OuterPoint q=wf_point(wf_patches+cl(item,0,7));p.x=q.x;p.y=q.y;}
  request_save();assert(saveFlash>0);linux_disk_shutdown();return 0;
 }
 wf_check_geography();
 wf_stand((OuterPoint){loc[12].x,loc[12].y});wf_walk(0,14);request_save();assert(saveFlash>0);
 for(int i=49;i<=52;i++)assert(!nv.garage[i]);
 assert(disk_read(&sv,sizeof sv));
 wf_to_ranger();wf_screen(argc>1?argv[1]:NULL);wf_key(ESC);
 wf_full_quest();assert(woodline_post_status()==3);
 wf_test_planters(argc>2?argv[2]:NULL);
 linux_disk_shutdown();
 puts("PASS Woodline: collision-accessible forest, 3 full quests, four species/eight patches, three one-shot chests, all seven crops in both walk-in shops, cross-reload and zero-state legacy save.");
 return 0;
}
