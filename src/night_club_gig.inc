/* An optional Afterimage ride. This is a separate job from Dispatch's taxis,
 * with authored curb goals and the same normal player/vehicle physics. */
enum { NCR_OFF,NCR_WAIT,NCR_BOARD,NCR_DRIVE,NCR_ALIGHT,NCR_LEAVE };
#include "venue_shelf_ledger.inc"
enum { NCR_VOCALIST=0,NCR_PHOTOGRAPHER,NCR_PROMOTER };
typedef struct{int x,y,model,hue;U coat;const char*name,*role;} NcRegular;
static const NcRegular ncr_cast[]={
 {265,286,MODEL_CIVILIAN,2,0x9c88c5,"Jules Vale","VOCALIST"},
 {696,313,MODEL_CIVILIAN,6,0xc78c5f,"Remi Lens","STAGE PHOTOGRAPHER"},
 {273,371,MODEL_STAFF,5,0x87bec0,"Bryn Calder","PROMOTER"}
};
static const struct{int shop,fee;const char*shortName;} ncr_shows[]={
 {28,115,"Copper Playhouse"},{31,145,"Blue Hour Tram Hall"},{47,165,"West Wind Playhouse"}
};
static struct{int phase,show,ride,still,walk;float x,y,a;NavPoint pick,drop,side,door;} ncr;
static int ncr_contact=-1,ncr_quote;
static unsigned ncr_heard[LEN(ncr_cast)];
static int ncr_active(void){return ncr.phase>=NCR_WAIT&&ncr.phase<=NCR_ALIGHT;}
static int ncr_owed(void){
 int token=venue_fee_token();return token>=1&&token<=LEN(ncr_shows)?ncr_shows[token-1].fee:0;
}
static void ncr_reset(void){
 ncr.phase=NCR_OFF;ncr_contact=-1;z(ncr_heard,sizeof ncr_heard);
}
static int ncr_offer(void){return cl((int)sv.dayClock/480,0,LEN(ncr_shows)-1);}
static int ncr_curbs(int show,NavPoint *pick,NavPoint *drop,NavPoint *door){
 int b=night_club_listing(),d=ncr_shows[show].shop;
 NavPoint base={ordinaryDoorX[b],ordinaryDoorY[b]};
 *door=(NavPoint){ordinaryDoorX[d],ordinaryDoorY[d]};
 return nav_park(base,pick)&&nav_park(*door,drop);
}
static void ncr_begin(void){
 if(ownedJob())return;
 int x,y;
 if(ng_route_goal(&x,&y)){tell("Bryn: Finish your Motor Works event before taking another booking.");return;}
 int show=ncr_quote;NavPoint pick,drop,door;
 if(!ncr_curbs(show,&pick,&drop,&door)){tell("Bryn: That show's curb is unavailable. Check back later.");return;}
 ncr.phase=NCR_WAIT;ncr.show=show;ncr.ride=-1;ncr.still=ncr.walk=0;
 ncr.pick=pick;ncr.drop=drop;ncr.door=door;
 ncr.x=pick.x;ncr.y=pick.y-26;ncr.a=1.570796f;
 navGoalKind=0;  /* Personal pins deliberately keep their existing priority. */
 ncPanel=NC_NONE;ncr_contact=-1;
 tell("Bryn: Ride booked. Bring an owned or licensed car to Jules at the marked curb.");
}
static int ncr_pay(void){
 int fee=ncr_owed();if(!fee)return 0;
 if(sv.cash>WALLET_LIMIT-fee){tell("Clean wallet full. Bryn will keep your fee until you make room.");return 0;}
 credit_wallet(1,fee);venue_split_write(venue_stock(),0);
 char msg[90];number(append(msg,"SHOW RIDE COMPLETE  /  CLEAN CASH +$"),fee);
 tell(msg);request_save();return 1;
}
static void ncr_complete(void){
 ncr.phase=NCR_LEAVE;ncr.walk=180;ncr.still=0;navGoalKind=0;
 venue_split_write(venue_stock(),ncr.show+1);
 if(!ncr_pay())request_save();
}
static int ncr_target(int*x,int*y){
 if(!ncr_active())return 0;
 NavPoint q=ncr.phase>=NCR_DRIVE?ncr.drop:ncr.pick;
 *x=q.x;*y=q.y;return 1;
}
static const char *ncr_stage_text(void){
 static char line[126];char*q;
 switch(ncr.phase){
 case NCR_WAIT:return "Pick up Jules outside The Afterimage. Stop by the marked curb in an owned or licensed car.";
 case NCR_BOARD:return "Jules is boarding. Hold still.";
 case NCR_DRIVE:q=append(line,"Drive Jules to ");append(append(q,ncr_shows[ncr.show].shortName),". Stop at the marked curb.");return line;
 case NCR_ALIGHT:return "Jules is walking to the sidewalk. Hold still.";
 default:return "";
 }
}
static const char *ncr_stop_name(void){
 return ncr.phase>=NCR_DRIVE?ncr_shows[ncr.show].shortName:"The Afterimage / Pickup";
}
static void ncr_cancel(const char*why){
 ncr.phase=NCR_OFF;ncr.still=0;navGoalKind=0;if(why)tell(why);
}
static int ncr_walk(float x,float y){
 float dx=x-ncr.x,dy=y-ncr.y,len=sq(d2(dx,dy)),step=93.f*FOOT_PACE/60.f;
 if(len<=step){ncr.x=x;ncr.y=y;return 1;}
 ncr.a=look(dx,dy);ncr.x+=dx*step/len;ncr.y+=dy*step/len;return 0;
}
static int ncr_car_stopped(NavPoint q){
 return p.car>=0&&cars[p.car].owner&&cars[p.car].cond>0&&absf(cars[p.car].s)<8.f&&
  absf(p.x-q.x)<31.f&&absf(p.y-q.y)<13.f;
}
static void ncr_street_tick(void){
 if(!ncr.phase)return;
 if(p.dead){ncr_cancel(0);return;}
 if(ncr.phase==NCR_LEAVE){
  if(--ncr.walk<=0||ncr_walk(ncr.door.x,ncr.door.y))ncr.phase=NCR_OFF;
  return;
 }
 if(ncr.phase>=NCR_DRIVE&&(p.car!=ncr.ride||p.car<0||!cars[p.car].cond)){
  ncr_cancel("Show ride canceled: Jules won't continue without the same drivable car.");return;
 }
 if(ncr.phase<=NCR_BOARD){
  int stopped=ncr_car_stopped(ncr.pick)&&street_line_open(ncr.pick.x,ncr.pick.y-26,p.x,p.y,3);
  if(!stopped||(ncr.phase==NCR_BOARD&&p.car!=ncr.ride)){
   ncr.phase=NCR_WAIT;ncr.still=0;ncr_walk(ncr.pick.x,ncr.pick.y-26);return;
  }
  if(ncr.phase==NCR_WAIT){
   if(++ncr.still>=14){ncr.ride=p.car;ncr.phase=NCR_BOARD;}
   return;
  }
  if(ncr_walk(p.x,p.y)){
   ncr.phase=NCR_DRIVE;ncr.still=0;navGoalKind=0;tell("Jules: I'm in. Next stop is on the GPS.");
  }
  return;
 }
 NavPoint side={ncr.drop.x,max(ncr.drop.y-27,ncr.door.y)};
 if(!ncr_car_stopped(ncr.drop)||!street_line_open(p.x,p.y,side.x,side.y,3)){
  ncr.phase=NCR_DRIVE;ncr.still=0;return;
 }
 if(ncr.phase==NCR_DRIVE){
  if(++ncr.still>=14){ncr.phase=NCR_ALIGHT;ncr.x=p.x;ncr.y=p.y;ncr.side=side;}
  return;
 }
 if(ncr_walk(ncr.side.x,ncr.side.y))ncr_complete();
}
static int ncr_ped(float*x,float*y){
 if(ncr.phase!=NCR_WAIT&&ncr.phase!=NCR_BOARD&&ncr.phase!=NCR_ALIGHT&&ncr.phase!=NCR_LEAVE)return 0;
 *x=ncr.x;*y=ncr.y;return 1;
}
static void ncr_street_person(int x,int y){
 const NcRegular*r=ncr_cast+NCR_VOCALIST;
 int gait=ncr.phase==NCR_WAIT?0:(int)(sn(anim*12.f)*4);
 person(x,y,r->coat,gait,ncr.a,r->model,r->hue);
 if(ncr.phase!=NCR_LEAVE)diamond(x,y+5,7,0xf18acb);
}
static int ncr_near(void){
 if(ncRoom!=NC_FLOOR)return -1;
 float best=33*33.f;int idx=-1;
 for(int i=0;i<LEN(ncr_cast);i++){
  const NcRegular*r=ncr_cast+i;
  if(i==NCR_VOCALIST&&ncr_active())continue;
  float d=d2(ncX-r->x,ncY-r->y);
  if(d<best&&pulse_path(ncX,ncY,r->x,r->y,nc_solid)){best=d;idx=i;}
 }
 return idx;
}
static void ncr_actor(int i){
 if(i==NCR_VOCALIST&&ncr_active())return;
 const NcRegular*r=ncr_cast+i;int close=(ncPanel==NC_REGULAR&&ncr_contact==i)||ncr_near()==i;
 float a=close?look(ncX-r->x,ncY-r->y):1.57f+.32f*sn(anim+i);
 person(r->x,r->y,r->coat,0,a,r->model,r->hue);
 if(close)nc_label(r->x,r->y-43,r->name,i==NCR_PROMOTER?ncGold:ncIce);
 else disc(r->x,r->y-34,2,i==NCR_PROMOTER?ncGold:ncIce);
}
static const char*ncr_small_talk(void){
 static const char*const notes[2][4]={
  {"Jules: That second chorus in Velvet Relay? Try hearing it from the side of the room.",
   "Jules: The ferries sound like a second rhythm section when it rains.",
   "Jules: Pity the first noon crowd. They got the new set without a rehearsal.",
   "Jules: I keep a clean shirt in the Copper green room. The drummer stores whole bicycles."},
  {"Remi: Stand where the house lights land, not where the camera points.",
   "Remi: I'm working on a wall of old ticket stubs. Best colors are on the cheap seats.",
   "Remi: If the band looks sharp through a bad lens, the band can probably play.",
   "Remi: I shot the street outside a year before the new marquee went up."}
 };
 int i=cl(ncr_contact,0,1);return notes[i][(ncr_heard[i]-1u)%4u];
}
static void ncr_contact_open(int i){
 ncr_contact=i;nc_showpanel(NC_REGULAR);
 if(i!=NCR_PROMOTER){ncr_heard[i]++;return;}
 if(venue_ambiguous(nv.world[NCR_OWED_WORD])){
  nc_showpanel(NC_OLD_LEDGER);ncChoice=2;return; /* Do not guess or silently discard an old save's property. */
 }
 ncr_quote=ncr_offer();ncChoice=1;  /* Confirm explicitly instead of accepting by hammering E. */
}
static const char*ncr_option(void){
 if(ncr_owed())return "Collect your show ride fee";
 if(ncr_active())return "Cancel the current show ride";
 return "Accept the show ride";
}
static void ncr_contact_act(void){
 if(ncr_contact!=NCR_PROMOTER){ncPanel=NC_NONE;return;}
 if(ncr_owed()){if(ncr_pay())ncPanel=NC_NONE;return;}
 if(ncr_active()){ncr_cancel("Bryn: Booking called off. Come back if you change your mind.");ncPanel=NC_NONE;return;}
 ncr_begin();
}
static void ncr_contact_note(int x,int y){
 if(ncr_contact!=NCR_PROMOTER){nav_wrap(x,y,280,ncr_small_talk(),PAPER,7);return;}
 char desc[135];char*q;
 if(ncr_owed()){q=number(append(desc,"I still owe you $"),ncr_owed());append(q," clean for the last ride.");}
 else if(ncr_active())append(desc,"Your ride is already booked. Wait at the marked curb in an owned or licensed car.");
 else{int i=ncr_quote;q=append(desc,"Take Jules to ");q=append(q,ncr_shows[i].shortName);q=number(append(q," for soundcheck. I'll pay $"),ncr_shows[i].fee);append(q," clean.");}
 nav_wrap(x,y,280,desc,PAPER,6);
 if(!ncr_owed()&&!ncr_active())nav_wrap(x,y+155,280,"Stop at both marked curbs and wait for Jules to get in and out. Bring an owned or licensed car.",ncIce,5);
}

/* The raw $145 and $165 values are ALSO legitimate old shelf counts of
 * 17+1 and 37+1. Only the player can tell which they had. Enter defaults
 * to Back, and closing the panel makes no change to either representation. */
static void ncr_old_note(int x,int y){
 U old=nv.world[NCR_OWED_WORD];char line[240];char*q=append(line,"An older save used the same record for $");
 q=number(q,old);q=append(q," in ride fees and ");q=number(q,old&127u);
 append(q," Echo Bloom House C and 1 Select B on your shelf. Keep those lots, or replace them with the old fee. Back changes nothing.");
 nav_wrap(x,y,280,line,PAPER,12);
}
static void ncr_old_select(int row){
 U old=nv.world[NCR_OWED_WORD];if(!venue_ambiguous(old))return;
 if(!row){venue_split_write(venue_stock(),0);tell("Loft shelf confirmed. Your stored lots are unchanged.");}
 else{venue_split_write(0,old==145u?2:3);if(ncr_pay()){ncPanel=NC_NONE;return;}}
 ncPanel=NC_NONE;request_save();
}
