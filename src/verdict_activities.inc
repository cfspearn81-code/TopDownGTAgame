/* Each optional translation-unit fragment has its own prefix and owns its
 * transient state. The stubs let the individual modules be tested in isolation. */
#if __has_include("verdict_a1.inc")
#include "verdict_a1.inc"
#endif
#if __has_include("verdict_driving.inc")
#include "verdict_driving.inc"
#endif
#if __has_include("verdict_taxi.inc")
#include "verdict_taxi.inc"
#endif
#if __has_include("verdict_explore.inc")
#include "verdict_explore.inc"
#endif
#if __has_include("verdict_media.inc")
#include "verdict_media.inc"
#endif
#if __has_include("verdict_street.inc")
#include "verdict_street.inc"
#endif
typedef int (*VdHandler)(int,U,U);
#ifndef VD_HAS_A1
#define vd_a1 ((VdHandler)0)
#endif
#ifndef VD_HAS_A2
#define vd_a2 ((VdHandler)0)
#endif
#ifndef VD_HAS_A3
#define vd_a3 ((VdHandler)0)
#endif
#ifndef VD_HAS_A4
#define vd_a4 ((VdHandler)0)
#endif
#ifndef VD_HAS_B1
#define vd_b1 ((VdHandler)0)
#endif
#ifndef VD_HAS_B2A
#define vd_b2a ((VdHandler)0)
#endif
#ifndef VD_HAS_B2B
#define vd_b2b ((VdHandler)0)
#endif
#ifndef VD_HAS_B3A
#define vd_b3a ((VdHandler)0)
#endif
#ifndef VD_HAS_B3B
#define vd_b3b ((VdHandler)0)
#endif
#ifndef VD_HAS_B3C
#define vd_b3c ((VdHandler)0)
#endif
#ifndef VD_HAS_B4
#define vd_b4 ((VdHandler)0)
#endif
#ifndef VD_HAS_B5
#define vd_b5 ((VdHandler)0)
#endif
#ifndef VD_HAS_B6A
#define vd_b6a ((VdHandler)0)
#endif
#ifndef VD_HAS_B6B
#define vd_b6b ((VdHandler)0)
#endif
#ifndef VD_HAS_B7
#define vd_b7 ((VdHandler)0)
#endif
#ifndef VD_HAS_B8
#define vd_b8 ((VdHandler)0)
#endif
#ifndef VD_HAS_B9
#define vd_b9 ((VdHandler)0)
#endif
static VdHandler const vdHandlers[]={vd_a1,vd_a2,vd_a3,vd_a4,vd_b1,vd_b2a,vd_b2b,vd_b3a,vd_b3b,vd_b3c,vd_b4,vd_b5,vd_b6a,vd_b6b,vd_b7,vd_b8,vd_b9};
static const char*const vdDeck[VA_COUNT]={
 "Blue Hour Tram Hall / Bring a record to Harbourlight Radio. Walk for free or pay $5 clean on the tram.",
 "Film set / Find your line in a loaner. Stop facing the camera on either of two painted marks.",
 "Passenger request / Go straight to HQ or give your rider a look at the Copper Playhouse lights.",
 "Neighborhood favor / Carry a dinner invitation to the baker who makes the braided loaf.",
 "Taxi desk / Compare two quoted fares and their distances before you commit.",
 "Parking shift / Take the customer's keys, park, get out and collect the next set.",
 "Parking puzzle / Work out the order of a compact car and a van on one apron.",
 "Picture desk / Find the intended scene, arrange the shot, and make the still.",
 "Arts desk / Match the picture and plaque clues to the real venue.",
 "Signal desk / A captioned meter helps you find an unmarked transmitter.",
 "Show bill / Pick what gets announced. Revisit the block to see who showed up.",
 "Street contract / Carry a tape during pursuit. Choose a handoff by its readiness.",
 "Freight desk / Release two queues while keeping the pedestrian crossing clear.",
 "Road notice / Obey the crossing signals or discover the signed detour.",
 "Local affairs / Keep a moving subject in the studio van's frame before you go live.",
 "Bluebird Print Shop / Align two moving plates and see your result in the shop window.",
 "Glass Mile / Race your recorded last run and decide which lap becomes your rival."
};
static void vd_copy(char*out,int cap,const char*s){int n=0;if(s)while(n+1<cap&&s[n]){out[n]=s[n];n++;}out[n]=0;}
static void vd_status(const char*s){vd_copy(vdLine,sizeof vdLine,s);}
static void vd_goal(int x,int y,const char*s){vdGoalX=x;vdGoalY=y;vd_copy(vdGoalName,sizeof vdGoalName,s);}
static void vd_end(void){vdActive=-1;vdChoice=0;vdPanel=0;vd_goal(-1,-1,0);vd_status(0);}
static void vd_cancel(void){if(vdActive>=0&&vdHandlers[vdActive])vdHandlers[vdActive](VE_CANCEL,0,0);vd_end();}
static void vd_reset(void){
 vd_end();vdSelected=vdCursor=0;
 for(int i=0;i<VA_COUNT;i++)if(vdHandlers[i])vdHandlers[i](VE_RESET,0,0);
}
static int vd_locked(void){return vdActive>=0;}
static int vd_launch(int id){
 if(id<0||id>=VA_COUNT||!vdHandlers[id])return 0;
 if(vdActive>=0||job.kind>=0||ncr_active()){tell("Finish your current work before starting another listing.");return 0;}
 if(sv.intro<3){tell("First, finish Mara's opening courier shift at Mercury Dispatch.");return 0;}
 vdPanel=0;vdActive=id;vd_goal(-1,-1,0);vd_status(0);vdChoice=0;
 vdHandlers[id](VE_START,0,0);return vdActive==id;
}
static void vd_choose(const char*title,const char*a,const char*b){
 vd_copy(vdTitle,sizeof vdTitle,title);vd_copy(vdOptions[0],sizeof vdOptions[0],a);vd_copy(vdOptions[1],sizeof vdOptions[1],b);
 vdPanel=2;vdCursor=0;vdChoice=0;
}
static int vd_input(U tap){
 if(vdPanel==2){
  if(tap&ESC){vd_cancel();tell("Listing declined. No charge.");}
  else if(tap&(LEFT|RIGHT|UP|DOWN))vdCursor^=1;
  else if(tap&(ENTER|USE)){vdChoice=1+vdCursor;vdPanel=0;}
  return 1;
 }
 if(vdPanel==1){
  if(tap&(ESC|ACTIVITIES)){vdPanel=0;return 1;}
  if(vdActive>=0){
   if(tap&(UP|DOWN|LEFT|RIGHT))vdCursor^=1;
   if(tap&(ENTER|USE)){vdPanel=0;if(vdCursor){vd_cancel();tell("Listing closed. Free roam continues.");}}
   return 1;
  }
  if(tap&UP)vdSelected=(vdSelected+VA_COUNT-1)%VA_COUNT;
  if(tap&DOWN)vdSelected=(vdSelected+1)%VA_COUNT;
  if(tap&(LEFT|RIGHT)){
   const int pages=(VA_COUNT+5)/6,row=vdSelected%6;
   int page=(vdSelected/6+((tap&RIGHT)?1:pages-1))%pages;
   vdSelected=min(page*6+row,VA_COUNT-1);
  }
  if(tap&(ENTER|USE)){vdPanel=0;vd_launch(vdSelected);}
  return 1;
 }
 if((tap&ACTIVITIES)&&!p.dead&&(scene==1||scene==3||scene==4)&&menu<0&&!mapOn&&atmOpen<0){
  vdPanel=1;vdCursor=0;return 1;
 }
 return 0;
}
static int vd_frame(U keys,U tap){
 if(p.dead)return 0;
 int consume=0,was=vdActive;
 for(int i=0;i<VA_COUNT;i++)if(vdHandlers[i])consume|=vdHandlers[i](VE_AMBIENT,keys,consume?tap&~(USE|ENTER):tap);
 if(was>=0&&vdActive==was&&vdHandlers[was])consume|=vdHandlers[was](VE_TICK,keys,consume?tap&~(USE|ENTER):tap);
 vdChoice=0;return consume||was!=vdActive||vdPanel;
}
static void vd_world(void){if(vdActive>=0&&vdHandlers[vdActive])vdHandlers[vdActive](VE_WORLD,0,0);}
static void vd_draw(void){
 if(p.dead||mapOn||scene==2||menu>=0||atmOpen>=0)return;
 if(!vdPanel){
  if(scene!=1&&vdActive>=0&&vdLine[0]){char row[130];const char*s=vdLine;
   int rows=0;while(*s){s=hud_line(s,892,row);rows++;}
   box(17,77,926,3+21*rows,DARK,0x415766);s=vdLine;
   for(int j=0;*s;j++){s=hud_line(s,892,row);textat(29,77+j*21,row,TEAL,0);}
  }
  for(int i=0;i<VA_COUNT;i++)if(vdHandlers[i])vdHandlers[i](VE_DECOR,0,0);
  if(vdActive>=0&&vdHandlers[vdActive])vdHandlers[vdActive](VE_DRAW,0,0);
  return;
 }
 if(vdPanel==2){
  modal(88,78,784,381,GOLD);textat(116,99,"PORT VANTA   /   PLAYER CHOICE",GOLD,0);
  nav_wrap(117,132,724,vdTitle,PAPER,4);
  for(int i=0;i<2;i++){
   int y=217+81*i;box(115,y,728,73,i==vdCursor?0x29373e:0x15232b,i==vdCursor?GOLD:0x425560);
   textat(128,y+12,i?"02":"01",i==vdCursor?GOLD:MUTED,0);nav_wrap(169,y+7,vdActive==VA_A1?611:657,vdOptions[i],PAPER,3);
  }
  #ifdef VD_HAS_A1
  if(vdActive==VA_A1)a1_draw_choice();
#endif
  textat(119,412,"ARROWS: CHOOSE       ENTER / E: CONFIRM       ESC: DECLINE",MUTED,0);return;
 }
 modal(64,39,832,461,TEAL);textat(94,60,"PORT VANTA     /     AFTER HOURS",GOLD,0);
 textat(93,86,"CITY LISTINGS",PAPER,1);
 if(vdActive>=0){
  textat(96,156,vdNames[vdActive],TEAL,0);nav_wrap(96,197,756,vdLine,PAPER,5);
  for(int i=0;i<2;i++){int y=305+i*43;box(94,y,764,35,i==vdCursor?0x293b40:0x15242b,i==vdCursor?TEAL:0x45575b);textat(108,y+5,i?"Close this activity without payout":"Resume activity",PAPER,0);}
  textat(98,449,"UP / DOWN: CHOOSE     ENTER: SELECT     ESC: RESUME",MUTED,0);return;
 }
 int first=(vdSelected/6)*6;
 for(int j=first;j<min(first+6,VA_COUNT);j++){
  int y=150+(j-first)*41;box(93,y,354,35,j==vdSelected?0x284047:0x16262d,j==vdSelected?TEAL:0x394d55);
  textat(108,y+4,vdNames[j],j==vdSelected?PAPER:MUTED,0);
 }
 char page[40];char*q=number(append(page,"ISSUE   "),first/6+1);append(q," / 3");
 textat(477,151,page,GOLD,0);nav_wrap(478,194,372,vdDeck[vdSelected],PAPER,6);
 if(sv.intro<3)wrapped(477,312,"Opens after your first courier shift.",GOLD,40);
 else if(job.kind>=0)wrapped(477,312,"Finish or abandon your regular contract first.",GOLD,40);
 else textat(478,317,"ENTER / EXPLORE THIS LISTING",TEAL,0);
 textat(96,450,"UP / DOWN: BROWSE      LEFT / RIGHT: PAGE      ENTER: OPEN      ESC: RETURN",MUTED,0);
}
