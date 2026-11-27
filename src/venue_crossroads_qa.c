/* Reproduce main a545bd8 venue failures using normal engine key ticks.
   Only the starter location, clock and inventory are test fixtures. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void key(U k){tick(k);tick(0);}
static void street_at(int x,int y){p.x=x;p.y=y;p.car=-1;p.dead=0;scene=1;menu=-1;mapOn=0;job.kind=-1;clear_input();tick(0);}
static void card(int app,int row){key(PHONE);assert(scene==NIGHT_PHONE_SCENE);
 while(np_app!=app)key(RIGHT);while(np_row!=row)key(DOWN);key(ENTER);key(ESC);}
static U patchhash(int x,int y,int w,int h){U r=2166136261u;for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)r=(r^pixels[j*W+i])*16777619u;return r;}
int main(int argc,char**argv){
 init_city();profiles_activate();sv.intro=4;
 int before=argc>1&&!strcmp(argv[1],"--observe-baseline"),wrong=0;
 for(int fair=1;fair<=2;fair++){
  int v=np_fair_location(fair),slot=fair==2?NP_BAG+3:NP_BAG;
  sv.dayClock=fair==2?1360:730;
  street_at(loc[v].x,loc[v].y);nv.phone[NP_POPSTAMP]=0;nv.phone[slot]=99;card(NP_WIRE,3);assert(nv.phone[NP_CARD]==(U)fair);
  for(int j=0;j<4&&scene==1;j++)key(USE);
  printf("full bag, reserved pop-up %d: scene %d, stock %u, RSVP %u\n",fair,scene,nv.phone[slot],nv.phone[NP_CARD]);
  wrong+=scene!=3;
  if(!before)assert(scene==3&&interior_active_loc==v&&nv.phone[slot]==99&&nv.phone[NP_CARD]==(U)fair);
  street_at(loc[v].x,loc[v].y);nv.phone[slot]=98;key(USE);assert(scene==1&&nv.phone[slot]==99&&!nv.phone[NP_CARD]);
  key(USE);assert(scene==3&&interior_active_loc==v);
 }
 /* Check retention of a simultaneous legal proof handoff. */
 sv.dayClock=750;street_at(loc[2].x,loc[2].y);nv.phone[NP_POPSTAMP]=0;nv.phone[NP_BAG]=99;card(NP_WIRE,3);card(NP_INBOX,0);
 key(USE);printf("prepaid proof with full reserved sample: %u\n",nv.phone[NP_SAMPLE]);wrong+=!nv.phone[NP_SAMPLE];
 if(!before)assert(scene==1&&np_job()==0&&nv.phone[NP_SAMPLE]&&nv.phone[NP_CARD]==1);
 z(nv.phone,sizeof nv.phone);
 /* Keep a menu up across each 2.5h route change or the 05:00 closure. */
 static const int hours[]={1290,1440,150,300};
 for(int i=0;i<4;i++){int x,y;sv.dayClock=(float)hours[i]-.12f;assert(night_venue_vendor_pos(&x,&y));
  street_at(x,y);key(USE);assert(scene==NIGHT_VENUES_SCENE&&nvs_room==NVR_CART);
  sv.cash=500;sv.gear=1;for(int n=0;n<20;n++)tick(0);
  int actual=scene;key(ENTER);printf("cart edge %d: scene %d, cash %d\n",hours[i],actual,sv.cash);wrong+=actual!=1;
  if(!before)assert(actual==1&&sv.cash==500);
  nv.world[NVK_BUYS]=0;
 }
 /* Reach the public aquarium from its street-side door, bypassing only the already-tested restoration campaign. */
 street_at(ordinaryDoorX[74],ordinaryDoorY[74]);nv.world[NVS_STAGE]=5;
 key(USE);assert(scene==4);key(ENTER);assert(scene==NIGHT_VENUES_SCENE);key(FIRE);assert(nvs_mode==4);
 for(int i=0;i<110;i++)tick(RIGHT);tick(0);int at=nvs_position;paint();
 U keeper=patchhash(138,303,28,31),hero=patchhash(at-14,309,29,31);
 sv.look.part[HAIR]=6;sv.look.part[TOP_TONE]=8;sv.look.part[SKIN]=7;paint();
 int clone=keeper!=patchhash(138,303,28,31);assert(hero!=patchhash(at-14,309,29,31));
 printf("Inez changes with player's hairstyle/colors: %d\n",clone);wrong+=clone;
 if(!before)assert(!clone);
 if(!before){
  for(int i=0;i<30;i++)tick(0);
  paint();U idle=patchhash(at-14,309,29,31);
  for(int i=0;i<8;i++){tick(0);paint();assert(idle==patchhash(at-14,309,29,31));}
  for(int i=0;i<12;i++)tick(LEFT);tick(0);int l=nvs_position;paint();U left=patchhash(l-14,309,29,31);
  for(int i=0;i<12;i++)tick(RIGHT);tick(0);paint();U right=patchhash(nvs_position-14,309,29,31);
  assert(left!=right&&nvs_position==at);
 }
 open_interior(12);key(ENTER);key(LEFT);assert(nvs_tab==4);key(MAP);
 assert(navHasManual&&navManualLoc==LEN(loc)+NVH_GLASS_CROWN);
 int misnamed=!strstr(nvs_log,walkin_name(NVH_GLASS_CROWN));wrong+=misnamed;
 printf("residence map card names the pinned public address (%s): %d\n",walkin_name(NVH_GLASS_CROWN),!misnamed);
 if(!before)assert(!misnamed);
 if(before){printf("baseline mismatch cases: %d/9\n",wrong);return wrong==9?0:1;}
 /* Stability across repeated arrivals, not just four cherry-picked ticks.
    The clock brackets exact boundaries with 4..30 input ticks of lead time. */
 int edgepasses=0,doorpasses=0;
 for(int sample=0;sample<32;sample++)for(int i=0;i<4;i++){
  sv.dayClock=hours[i]-.10f-(sample%27)/60.f;
  int x,y;assert(night_venue_vendor_pos(&x,&y));street_at(x,y);key(USE);assert(scene==NIGHT_VENUES_SCENE);
  for(int n=0;n<46;n++)tick(0);assert(scene==1);edgepasses++;
 }
 for(int sample=0;sample<50;sample++)for(int fair=1;fair<=2;fair++){
  int v=np_fair_location(fair),slot=fair==2?NP_BAG+3:NP_BAG;
  sv.dayClock=fair==2?1360:730;nv.phone[NP_POPSTAMP]=0;nv.phone[slot]=99;
  street_at(loc[v].x,loc[v].y);card(NP_WIRE,3);key(USE);
  assert(scene==3&&interior_active_loc==v&&nv.phone[slot]==99&&nv.phone[NP_CARD]==(U)fair);doorpasses++;
 }
 printf("PASS: %d scheduled cart transitions, %d saturated-stock door visits.\n",edgepasses,doorpasses);
 puts("PASS: independent guide appearance, 7 event/route edges, normal E doors, reserve preserved, no out-of-range purchases.");return 0;
}
