/* GLASS RELAY / APARTMENT FOUR. An optional sidestep from stock interiors.
 * No host dependencies; simulation and scenery share the native framebuffer. */
enum { NVS_STAGE=8,NVS_SHELF=9,NVS_FINISH=15,NVS_OWNED=16,
       NVS_MOUNT=17,NVS_SURF=18,NVS_ECHO=19,NVS_TOL=20 };
enum { NVR_GLASS=1,NVR_HOME,NVR_CART,NVR_LEASE };
#include "night_venue_kit.inc"
enum { NVH_GLASS_CROWN=72,NVH_PRICE=640,NVH_RENOVATE=180 };
enum { NVF_TEA,NVF_NIGHTCAP,NVF_GLASSLEAF,NVF_RESET, NVF_NIGHTCAP_PRICE=5 };
static const int nvs_aquaria[]={17,36,55,74},nvs_stops[]={42,59,61,78};
static const char *const nvs_exhibit[]={"01  Kelp Forest","02  Jelly Drift","03  Ray Lagoon","04  Shoal Walk"};
static const char *const nvs_places[]={"Locker","Display case","Bunk","Refreshments","Residence"};
static const char *const nvs_cart_tabs[]={"Finishes","Pulse frames","Attachments","Charge cells"};
static const char *const nvs_finish[]={"Factory pearl","Seafoam enamel","Copper circuit","Iris lacquer"};
static const U nvs_colors[]={0xc8d9d6,0x7de6bc,0xeab67a,0xbda8ec};
static const char *const nvs_motion[]={"WEST","EAST","NORTH"};
static const int nvs_targets[]={1,3,2},nvs_prices[]={0,24,36,48};
static int nvs_back,nvs_room,nvs_tab,nvs_sel,nvs_mode,nvs_step,nvs_dials[3],nvs_saved,nvs_folio;
static U nvs_frames;
static int nvs_recov_ticks,nvs_leaf_grade,nvs_position,nvs_bonus,nvs_rail_dir,nvs_rail_walk;
static char nvs_log[150];
static void nvs_msg(const char*s){append(nvs_log,s);}
static int nvs_stage(void){return cl((int)(nv.world[NVS_STAGE]&255u),0,5);}
static int nvs_finish_id(void){return cl((int)nv.world[NVS_FINISH],0,3);}
static int nvs_has(int n){return n==0 || (nv.world[NVS_OWNED]&(1<<n));}
static int night_venue_vendor_stop(void){return nvs_stops[cl((int)sv.dayClock/(DAY_SECONDS/4),0,3)];}
static int nvs_cart_here(void){return night_venue_vendor_stop();}
static int nvs_penthouse(void){return !!(nv.world[NVH_DEED]&1u);}
static int nvs_renovated(void){return !!(nv.world[NVH_DEED]&2u);}
/* The public Night Wire report is derived from the city timetable, even
 * if the player was inside when the Cart checked in. It isn't a sensor
 * for an unseen or moving street actor. */
static int night_venue_vendor_report(int *x,int *y,int *clock){
 int open=sv.dayClock<300.f||sv.dayClock>=1140.f;
 int minNight=(int)sv.dayClock+(sv.dayClock<300.f?300:-1140);
 int pick=open?cl(minNight/150,0,3):3;
 static const short stops[4][2]={{2172,1280},{2660,824},{2696,1480},{2240,2280}};
 static const short reports[4]={1140,1290,0,150};
 int here=nvs_stops[pick];
 *x=ROOM>2740?stops[pick][0]:ordinaryDoorX[here];
 *y=ROOM>2740?stops[pick][1]:ordinaryDoorY[here];
 *clock=reports[pick];
 return open;
}
static int night_venue_vendor_pos(int *x,int *y){
 if(sv.dayClock>=300.f&&sv.dayClock<1140.f)return 0;
 int clock;return night_venue_vendor_report(x,y,&clock);
}
static int night_venue_vendor_entry(U tap){
 int x,y;
 if(!(tap&USE)||scene!=1||p.car>=0||p.dead||menu>=0||mapOn||vdPanel||vdActive>=0||job.kind>=0||atmOpen>=0||
    !night_venue_vendor_pos(&x,&y)||d2(p.x-x,p.y-y)>42*42)return 0;
 nvs_back=1;nvs_room=NVR_CART;nvs_tab=1;nvs_sel=nvs_mode=nvs_step=nvs_saved=0;nvs_frames=0;
 nvs_msg("Night Cart is parked. See the fit-out board for frames, attachments and charge cells.");
 scene=NIGHT_VENUES_SCENE;walking=0;return 1;
}
static int nvs_sky(void){return nvs_back==4&&walkin_active==NVH_GLASS_CROWN;}
static void nvs_store(void){request_save();nvs_saved=saveFlash<0?-1:1;}
static int night_venues_open(int b){(void)b;return 0;} /* retain vanilla rooms and photo missions */
static int nvs_door(void){
 if(menu>=0||p.dead||mapOn||vdPanel||vdActive>=0||job.kind>=0)return 0;
 if(scene==3&&interior_active_loc>=0&&interior_active_loc<LEN(loc)&&!interior_subroom){
  int k=loc[interior_active_loc].kind;
  if(k==13||(k==9&&sv.home))return NVR_HOME;
 }
 if(scene==4&&walkin_active>=0){
  for(int j=0;j<4;j++)if(walkin_active==nvs_aquaria[j])return NVR_GLASS;
  if(walkin_active==nvs_cart_here())return NVR_CART;
  if(walkin_active==NVH_GLASS_CROWN)return nvs_penthouse()?NVR_HOME:NVR_LEASE;
 }
 return 0;
}
static int night_venues_entry(U tap){
 if(!(tap&ENTER))return 0;
 int kind=nvs_door();if(!kind)return 0;
 nvs_back=scene;nvs_room=kind;nvs_tab=nvs_sel=nvs_mode=nvs_step=nvs_saved=nvs_folio=0;nvs_frames=0;nvs_leaf_grade=0;nvs_recov_ticks=0;
 if(kind==NVR_GLASS)for(int j=0;j<4;j++)if(walkin_active==nvs_aquaria[j])nvs_tab=j;
 nvs_msg("");scene=10;walking=0;return 1;
}
static void night_venues_hint(void){
 int k=nvs_door();if(!k)return;
 const char *s=k==NVR_GLASS?"ENTER / GLASS RELAY":k==NVR_HOME?"ENTER / PRIVATE LOFT":k==NVR_LEASE?"ENTER / SKY LEASE DESK":"ENTER / SIGNAL CART";
 int w=textw(s,0)+22;box(W-w-17,447,w,27,0x10282e,0x467578);textat(W-w-6,450,s,TEAL,0);
}
static int nvs_use_key(U tap){return !!(tap&(ENTER|USE));}
static int nvs_go(int i,int step,int n){return (i+n+step)%n;}
static void nvs_progress(int stage,const char*s){
 nv.world[NVS_STAGE]=(nv.world[NVS_STAGE]&~255u)|(U)stage;nvs_mode=nvs_step=0;nvs_msg(s);nvs_store();
}
static int nvs_wrasse(int t){
 /* 3.5-second return sweep, central target with a generous exposure window. */
 int k=t%210;return 174+(k<=105?k:210-k)*595/105;
}
/* Completed story objectives stay completed even when the clean wallet is
 * full. Two spare one-bit flags hold the indivisible $75 and $30 receipts. */
#define DC_VENUE_AQUA_PAYOUT
enum { NVS_GRANT_HELD=1<<9,NVS_STIPEND_HELD=1<<10 };
static int nvs_aqua_pay(int value,int bit){
 if(sv.cash>WALLET_LIMIT-value){nv.world[NVS_STAGE]|=(U)bit;return 0;}
 credit_wallet(1,value);nv.world[NVS_STAGE]&=~(U)bit;return value;
}
static int nvs_aqua_claim(void){
 U hold=nv.world[NVS_STAGE]&(NVS_GRANT_HELD|NVS_STIPEND_HELD);if(!hold)return 0;
 int paid=0;if(hold&NVS_GRANT_HELD)paid+=nvs_aqua_pay(75,NVS_GRANT_HELD);
 if(hold&NVS_STIPEND_HELD)paid+=nvs_aqua_pay(30,NVS_STIPEND_HELD);
 if(paid){char line[140];char*q=number(append(line,"Inez paid $"),paid);
  append(q,nv.world[NVS_STAGE]&(NVS_GRANT_HELD|NVS_STIPEND_HELD)?" clean. Your other receipt is still here.":" clean. Your receipts are settled.");
  nvs_msg(line);nvs_store();}
 else nvs_msg("Deposit some clean cash at an ATM. Inez is holding your payment here at Shoal Walk.");
 return 1;
}
static void nvs_aqua_tick(U tap){
 int stage=nvs_stage();
 if(nvs_mode==1){ /* spot a species moving through the polarized gate */
  if(nvs_use_key(tap)){
   if(absf(nvs_wrasse((int)nvs_frames)-475)<=58)nvs_progress(2,"Tag logged. Inez needs the three lamps at Jelly Drift next.");
   else nvs_msg("Not yet. Follow the gold wrasse into the brass viewfinder.");
  }return;
 }
 if(nvs_mode==2){
  if(tap&LEFT)nvs_sel=nvs_go(nvs_sel,-1,3);if(tap&RIGHT)nvs_sel=nvs_go(nvs_sel,1,3);
  if(nvs_use_key(tap)){
   nvs_dials[nvs_sel]=(nvs_dials[nvs_sel]+1)%4;
   if(nvs_dials[0]==1&&nvs_dials[1]==3&&nvs_dials[2]==2)nvs_progress(3,"Lamp circuit restored. Ray Lagoon has a stranded kite ray.");
   else nvs_msg("One detent clockwise. Match all three numbered dial faces.");
  }return;
 }
 if(nvs_mode==3){
  if(tap&(LEFT|RIGHT|UP|DOWN)){
   U arrow=tap&(LEFT|RIGHT|UP|DOWN),target=nvs_step==0?LEFT:nvs_step==1?RIGHT:UP;
   if(arrow==target){if(++nvs_step==3)nvs_progress(4,"Ray released. The last thing left is raising the Shoal Walk shutter.");
    else nvs_msg("Gate responds. Guide the ray through the next marked channel.");
   }else {nvs_step=0;nvs_msg("The ray circled back. Follow the three route marks from the start.");}
  }return;
 }
 if(nvs_mode==4){
  if(nvs_use_key(tap)){
   int window=cl((nvs_position-8)/222,0,3);
   static const char *const text[]={"Inez: Your permit is on the board. Staff can safely hold the late tours now.",
    "Lightwell: Lantern fish scatter below the caustics as your shadow passes.",
    "Nursery glass: A speckled kitten ray swims beneath the school.",
    "East bench: The restored wing finally gives the night volunteers a place to sit."};
   nvs_msg(text[window]);nvs_bonus|=1<<window;
   if(nvs_bonus==15&&!(nv.world[NVS_STAGE]&256u)){
    nv.world[NVS_STAGE]|=256u;
    nvs_msg(nvs_aqua_pay(30,NVS_STIPEND_HELD)?"Your full tour is in the keeper's journal. $30 overnight steward stipend awarded.":
     "Tour recorded. Inez is holding your full $30 stipend. Make room in your wallet, then press E at Shoal Walk.");nvs_store();
   }
  }return;
 }
 if(stage>=5&&nvs_tab==3&&(tap&FIRE)){
  nvs_mode=4;nvs_position=126;nvs_bonus=nvs_rail_walk=0;nvs_rail_dir=1;nvs_msg("The east wing is open. Walk the rail and inspect all four stops.");return;
 }
 if(tap&(LEFT|RIGHT)){nvs_tab=nvs_go(nvs_tab,(tap&LEFT)?-1:1,4);nvs_msg("");}
 if(!nvs_use_key(tap))return;
 if(stage==0){nvs_progress(1,"Work order accepted. Start with the gold wrasse at Kelp Forest.");return;}
 if(stage==1&&nvs_tab==0){nvs_mode=1;nvs_frames=0;nvs_msg("The wrasse patrols the glass. Press E inside the center brackets.");return;}
 if(stage==2&&nvs_tab==1){nvs_mode=2;nvs_sel=0;z(nvs_dials,sizeof nvs_dials);nvs_msg("Three banks, three detents: match the numbers engraved above each lamp.");return;}
 if(stage==3&&nvs_tab==2){nvs_mode=3;nvs_step=0;nvs_msg("Open the channels in order: WEST, EAST, NORTH. Use the arrows.");return;}
 if(stage==4&&nvs_tab==3){
  nvs_progress(5,nvs_aqua_pay(75,NVS_GRANT_HELD)?"Shutter raised! Shoal Walk reopened. Conservation grant: $75":
   "Shoal Walk reopened. Your full $75 grant is on hold. Make room in your wallet, then press E here.");return;
 }
 if(stage==5&&nvs_tab==3&&nvs_aqua_claim())return;
 static const char*const lens[]={"Gold wrasse thread the fronds; silvery baitfish follow the warm current.",
 "Moon bells pulse from their rims. Under the lamps, their trailing threads shimmer.",
 "The little kite rays dip a wing when they turn above the lagoon floor.",
 "Lantern shoals split into two streams around the central light."};
 nvs_msg((nvs_tab==3&&stage<5)?"Closed wing. The brass shutter waits for staff clearance.":lens[nvs_tab]);
}
#if __has_include("night_phone.inc")
/* Three independent 7-bit shelf counts per design, distinct from the
 * commissioned stockroom. A transfer never converts or re-grades a print. */
#include "venue_shelf_ledger.inc"
static U nvs_ed_word(int id){return id>=6?venue_stock():nv.world[21+id/3];}
static int nvs_ed_count(int id){return cl((nvs_ed_word(id)>>((id%3)*7))&127,0,99);}
static int nvs_ed_review(int id){return id>=6&&venue_ambiguous(nv.world[NCR_OWED_WORD]);}
static void nvs_ed_xfer(int out){
 int id=nvs_sel,shelf=nvs_ed_count(id),bag=np_get(NP_BAG+id,99),next=out?shelf-1:shelf+1;
 if(nvs_ed_review(id)){nvs_msg("These three grades share an old ride receipt. No lots moved. M pins Bryn at The Afterimage to resolve it.");return;}
 if((out&&!shelf)||(!out&&!bag)){nvs_msg(out?"No prints of this edition on your home shelf.":"No matching print in your satchel.");return;}
 if((out&&bag>=99)||(!out&&shelf>=99)){nvs_msg("The destination for this edition is full.");return;}
 int shift=(id%3)*7;
 U word=(nvs_ed_word(id)&~(127u<<shift))|((U)next<<shift);
 if(id>=6)venue_split_write(word,venue_fee_token());else nv.world[21+id/3]=word;
 nv.phone[NP_BAG+id]=bag+(out?1:-1);
 nvs_msg(out?"One exact strain and grade returned to your satchel.":"One matching lot stashed with its grade intact.");nvs_store();
}
#endif
static void nvs_transfer(int out){
#if __has_include("night_phone.inc")
 if(nvs_folio){nvs_ed_xfer(out);return;}
#endif
 int type=nvs_sel/2,grade=nvs_sel&1,slot=NVS_SHELF+nvs_sel;
 int loose=sv.stocks[type],boxed=cl(nv.world[slot],0,999);
 if(out){
  if(!boxed){nvs_msg("That shelf is empty.");return;}
  if(loose>=SAVE_STOCK_LIMIT){nvs_msg("The warehouse is full.");return;}
  nv.world[slot]=boxed-1;sv.stocks[type]++;nvs_msg("Returned one lot to bulk warehouse stock. The shelf label stays here.");
 }else{
  if(!loose){nvs_msg("No loose stock for this lot. Complete a batch at an owned property first.");return;}
  if(boxed>=999){nvs_msg("That shelf is full.");return;}
  if(grade&&sv.cash<3){nvs_msg("Select storage needs $3 clean for a sealed presentation case.");return;}
  if(grade)sv.cash-=3;
  nv.world[slot]=boxed+1;sv.stocks[type]--;nvs_msg(grade?"Select lot sealed, labeled and stored.":"House lot labeled and stored.");
 }
 nvs_store();
}
static const int nvs_rest_hours[]={2,8,4};
static int nvs_drink(int i){
 if(i==NVF_RESET){
  nv.world[NVS_SURF]=nv.world[NVS_ECHO]=nv.world[NVF_EXTRA]=nv.world[NVS_TOL]=0;nvs_recov_ticks=0;
  nvs_msg("Quiet reset. The lights and sound settle; your regular movement returns.");nvs_store();return 1;
 }
#if __has_include("night_phone.inc")
 if(i==NVF_GLASSLEAF){
  int id=nvs_leaf_grade,n=np_get(NP_BAG+id,99);
  if(!n){nvs_msg("No matching Glasshouse lot in your satchel. T selects another strain and grade.");return 0;}
  nv.phone[NP_BAG+id]=n-1;
 }
#endif
 if(i==NVF_NIGHTCAP){
  if(sv.cash<NVF_NIGHTCAP_PRICE){nvs_msg("Night Bell is $5 clean at the refreshment counter.");return 0;}
  sv.cash-=NVF_NIGHTCAP_PRICE;
 }
 int field=i==NVF_TEA?NVS_SURF:i==NVF_NIGHTCAP?NVS_ECHO:NVF_EXTRA;
 int tol=cl(nv.world[NVS_TOL],0,3600);
 nv.world[field]=cl(2100-tol/3,900,2100);nv.world[NVS_TOL]=min(3600,tol+720);
 nvs_recov_ticks=90;
 if(!a1AudioPending)a1AudioPending=((nv.world[NVS_SURF]>0)+(nv.world[NVS_ECHO]>0)+(nv.world[NVF_EXTRA]>0)>1)?18:15+i;
 static const char *const feel[]={
 "Sunline fizz: bright gold light and lively footsteps.",
 "Night Bell cocktail: softer blue light, mellow station tones and heavier footsteps.",
 "Glassleaf roll: curling teal haze, a soft chime and steady foot acceleration."};
 nvs_msg(feel[i]);nvs_store();return 1;
}
static int night_venue_bar_drink(void){
 int bought=nvs_drink(NVF_NIGHTCAP);tell(nvs_log);return bought;
}
static void nvs_pin(int b){
 nav_slot();navManualLoc=LEN(loc)+b;navManual=nav_door(navManualLoc);navHasManual=1;
}
static void nvs_case_apply(void){
 if(sv.gear<0){nvs_msg("The display needs a pulse tool from Vanta Armory.");return;}
 if(!nvs_has(nvs_sel)){nvs_msg("Find the Signal Cart to collect this cosmetic finish.");return;}
 nv.world[NVS_FINISH]=nvs_sel;nv.world[NVS_MOUNT]=1;
 nvk_set(NVK_CASE_BIT,3,nvk_frame());nvk_set(NVK_CASE_LENS_BIT,3,nvk_glass());nvk_set(NVK_CASE_CELL_BIT,3,nvk_kind());
 nvs_msg("Display snapshot mounted with current attachments and finish.");nvs_store();
}
static void nvs_home_tick(U tap){
 if(tap&(LEFT|RIGHT)){nvs_tab=nvs_go(nvs_tab,(tap&LEFT)?-1:1,LEN(nvs_places));nvs_sel=0;nvs_msg("");}
#if __has_include("night_phone.inc")
 if(nvs_tab==0&&(tap&TAXI_NEXT)){nvs_folio^=1;nvs_sel=0;nvs_msg("");}
#endif
 int count=nvs_tab==0?(nvs_folio?9:6):nvs_tab==1?4:nvs_tab==2?3:nvs_tab==3?4:1;
 if(tap&UP)nvs_sel=nvs_go(nvs_sel,-1,count);
 if(tap&DOWN)nvs_sel=nvs_go(nvs_sel,1,count);
#if __has_include("night_phone.inc")
 if(nvs_tab==3&&nvs_sel==NVF_GLASSLEAF&&(tap&TAXI_NEXT)){nvs_leaf_grade=(nvs_leaf_grade+1)%9;nvs_msg("");return;}
#endif
 if((nvs_tab==1||nvs_tab==0||nvs_tab==4)&&(tap&MAP)){
  int b=nvs_tab==4?NVH_GLASS_CROWN:nvs_cart_here();
  if(nvs_tab==0){
#if __has_include("night_phone.inc") && __has_include("night_club.inc")
   if(nvs_folio&&nvs_ed_review(nvs_sel)){nvs_pin(night_club_listing());nvs_msg("The Afterimage pinned. Ask Bryn about the old shelf/ride entry.");return;}
#endif
   int best=0;float dist=1e9f;
   for(int i=0;i<4;i++){int k=nvs_aquaria[i];float d=d2(ordinaryDoorX[k]-p.x,ordinaryDoorY[k]-p.y);if(d<dist){dist=d;best=i;}}
   b=nvs_aquaria[best];
  }
  if(nvs_tab==1){
   int x,y;
   if(night_venue_vendor_pos(&x,&y)){nav_track_cart();nvs_msg("Tracking the Night Cart. The atlas updates at each new stop.");return;}
  }
  nvs_pin(b);
  if(nvs_tab==4){
   char where[135];char*q=append(where,walkin_name(b));append(q," pinned. Press Enter in its lobby for the Glass Crown lease desk.");nvs_msg(where);return;
  }
  nvs_msg(
   nvs_tab==1?"Cosmetic gallery pinned. The equipment van operates from 19:00 until 05:00.":"Nearest aquarium pinned. Enter its lobby, then press ENTER for the Glass Relay.");return;
 }
 if(nvs_tab==0){if(nvs_use_key(tap))nvs_transfer(0);else if(tap&FIRE)nvs_transfer(1);return;}
 if(nvs_tab==1&&(tap&FIRE)){nv.world[NVS_MOUNT]=0;nvs_msg("Display cradle cleared. Your equipped tool is unchanged.");nvs_store();return;}
 if(!nvs_use_key(tap))return;
 if(nvs_tab==1)nvs_case_apply();
 else if(nvs_tab==2){
  if(p.heat>.1f||job.kind>=0||vdActive>=0){nvs_msg("You cannot sleep during a job or pursuit.");return;}
  night_rest_clock(nvs_rest_hours[nvs_sel],nvs_sel==2);
  nv.world[NVS_SURF]=nv.world[NVS_ECHO]=nv.world[NVF_EXTRA]=nv.world[NVS_TOL]=0;nvs_recov_ticks=0;
  p.hp=min(100,p.hp+(nvs_sel==1?(nvs_sky()&&nvs_renovated()?48:35):12));
  nvs_msg(nvs_sel==2?"Four hours later. The front passed; the harbor is clear for a while.":
   nvs_sel==1?"Eight hours later. You wake rested to the sound of the harbor.":"Two hours later. Your watch chimes and the harbor lights change.");
  nvs_store();
 }else if(nvs_tab==3)nvs_drink(nvs_sel);
 else if(nvs_sky()&&!nvs_renovated()){
  if(sv.cash<NVH_RENOVATE){nvs_msg("Renovating the penthouse costs $180 clean cash.");return;}
  sv.cash-=NVH_RENOVATE;nv.world[NVH_DEED]|=2;nvs_msg("Renovation paid: living wall, harbor floor lights and a deluxe bed installed.");nvs_store();
 }else nvs_msg(nvs_penthouse()?"The deed is yours. Use the Sky Club's private lift to visit.":"M pins the Glass Crown leasing desk. A Breakwater deed qualifies you.");
}
static void nvs_lease_tick(U tap){
 if(!nvs_use_key(tap))return;
 if(!sv.home){nvs_msg("Earn your resident references by buying the Breakwater Safehouse first.");return;}
 if(case_blocks_price(NVH_PRICE)){nvs_msg("Purchase on hold. Settle your case at the Revenue Annex.");return;}
 if(sv.cash<NVH_PRICE){nvs_msg("The Glass Crown penthouse costs $640 clean cash.");return;}
 spend(NVH_PRICE);nv.world[NVH_DEED]|=1;nvs_room=NVR_HOME;nvs_tab=nvs_sel=0;
 nvs_msg("Penthouse purchased. Your private inventory and display are accessible here too.");nvs_store();
}
static void nvs_cart_tick(U tap){
 if(nvs_back==1&&(tap&(LEFT|RIGHT))){nvs_tab=nvs_go(nvs_tab,(tap&LEFT)?-1:1,4);nvs_sel=0;nvs_msg("");}
 int count=3;
 if(tap&DOWN)nvs_sel=nvs_go(nvs_sel,1,count);
 if(tap&UP)nvs_sel=nvs_go(nvs_sel,-1,count);
 int id=nvs_sel;
 if(tap&FIRE&&nvs_tab==1){nvk_cycle_frame(0);nvs_msg("Legacy pulse re-equipped. The original unlimited pulse is unchanged.");nvs_store();return;}
 if(tap&FIRE&&nvs_tab==3&&sv.gear>=0&&nvk_frame()){if(nvk_fill()){nvs_msg("Selected cells loaded from reserve.");nvs_store();}else nvs_msg("No more matching cells fit.");return;}
 if(!nvs_use_key(tap))return;
 if(nvs_tab==0){
  id++;
  if(nvs_has(id)){nvs_msg("This finish is already in your home catalogue.");return;}
  if(sv.cash<nvs_prices[id]){nvs_msg("Not enough clean cash for this finish.");return;}
  sv.cash-=nvs_prices[id];nv.world[NVS_OWNED]|=1<<id;nvs_msg("Finish collected. Mount it at your apartment display case.");
 }else{
  if(sv.gear<0){nvs_msg("A legacy pulse license from Vanta Armory unlocks the equipment catalogue.");return;}
  if(nvs_tab==1){
   id++;
   if(!nvk_owned(id)){
    if(case_blocks_price(nvk_frames[id].price)){nvs_msg("Purchase on hold. Settle your case at the Revenue Annex.");return;}
    if(sv.cash<nvk_frames[id].price){nvs_msg("Not enough clean cash for this frame.");return;}
    spend(nvk_frames[id].price);nv.world[NVK_BUYS]|=1u<<id;
    nvk_put(0,nvk_reserve(0)+min(6,max(0,99-nvk_reserve(0)-(nvk_kind()==0?nvk_load():0))));
   }
   if(nvk_frame()!=id)nvk_cycle_frame(id);
   nvk_fill();nvs_msg("Frame equipped. Load compatible cells with L on foot.");
  }else if(nvs_tab==2){
   if(id&&!nvk_owned(3+id)){
    if(sv.cash<nvk_lens_cost[id]){nvs_msg("Not enough clean cash for this attachment.");return;}
    sv.cash-=nvk_lens_cost[id];nv.world[NVK_BUYS]|=1u<<(3+id);
   }
   nvk_set_glass(id);nvs_msg("Attachment installed on your configurable pulse frames.");
  }else{
   if(!nvk_frame()){nvs_msg("Equip a charged frame first. The Legacy pulse uses unlimited default power.");return;}
   int room=99-nvk_reserve(id)-(nvk_kind()==id?nvk_load():0);
   if(room<6){nvs_msg("Not enough reserve space for another six-cell arcade pack.");return;}
   if(sv.cash<nvk_cell_cost[id]){nvs_msg("Not enough clean cash for a cell pack.");return;}
   sv.cash-=nvk_cell_cost[id];nvk_cycle_cell(id);nvk_put(id,nvk_reserve(id)+6);nvk_fill();
   nvs_msg("Pack purchased, color selected and cells loaded. Space tops up here; L tops up on foot.");
  }
 }
 nvs_store();
}
static void night_venues_tick(U keys,U tap){
 if(nvs_room==NVR_CART&&nvs_back==1){
  int x,y,open=night_venue_vendor_pos(&x,&y);
  if(!open||d2(p.x-x,p.y-y)>42*42){
   scene=1;nvs_room=0;tell(open?"Signal Cart moved to its next stop. Check Night Wire for the new pin.":"Signal Cart closed for the day. It returns at 19:00.");return;
  }
 }
 if(nvs_room==NVR_GLASS&&nvs_mode==4){
  int dx=!!(keys&RIGHT)-!!(keys&LEFT),next=cl(nvs_position+dx*3,72,918);
  nvs_rail_walk=next!=nvs_position;if(dx)nvs_rail_dir=dx;nvs_position=next;
 }
 nvs_frames++;if(nvs_frames>1200000U)nvs_frames=0;
 p.secs+=1.f/60.f;
 if(tap&ESC){
  if(nvs_room==NVR_GLASS&&nvs_mode){nvs_mode=nvs_step=0;nvs_msg("Back at the gallery rail.");return;}
  scene=nvs_back==1?1:nvs_back==3?3:4;nvs_room=0;return;
 }
 if(nvs_room==NVR_GLASS)nvs_aqua_tick(tap);
 else if(nvs_room==NVR_HOME)nvs_home_tick(tap);
 else if(nvs_room==NVR_LEASE)nvs_lease_tick(tap);
 else nvs_cart_tick(tap);
}
static void night_venues_world_tick(void){
 if(p.dead||scene==0||scene==2||scene==5||menu==PAUSE_MENU||vdPanel)return;
 for(int i=NVS_SURF;i<=NVS_TOL;i++)if(nv.world[i]>0)nv.world[i]--;
 if(nv.world[NVF_EXTRA]>0)nv.world[NVF_EXTRA]--;
 if(nvs_recov_ticks>0)nvs_recov_ticks--;
}
static float night_venues_walk_factor(void){
 /* Fixed ramp rather than random jitter; controls are never inverted or forced. */
 int a=cl(nv.world[NVS_SURF],0,420),b=cl(nv.world[NVS_ECHO],0,420);
 int c=cl(nv.world[NVF_EXTRA],0,420);
 return 1.f+a*.14f/420.f-b*.12f/420.f+c*.055f/420.f+(a&&b?.025f*min(a,b)/420.f:0.f);
}
static void night_venues_fx(void){
 if(scene!=1||mapOn||p.dead||menu>=0||vdPanel||atmOpen>=0)return;
 int a=cl(nv.world[NVS_SURF],0,2100),b=cl(nv.world[NVS_ECHO],0,2100),c=cl(nv.world[NVF_EXTRA],0,2100);
 int f=nvk_frame(),cell=nvk_kind();
 if(p.car<0&&sv.gear>=0&&f){
  char row[90];char*q=append(row,nvk_frames[f].name);q=append(q,"  /  ");q=number(q,nvk_load());q=append(q," + ");q=number(q,nvk_reserve(cell));append(q,"   L");
  int w=textw(row,0)+17;box(W-w-16,82,w,25,DARK,nvk_cell_ink[cell]);textat(W-w-8,83,row,PAPER,0);
 }
 if(!a&&!b&&!c)return;
 int j=(a>0)+(b>0)+(c>0);
 U color=j>1?0xbfadcf:a?0xecbd72:b?0xb9a4e4:0x83dec5;int life=max(a,max(b,c)),k=cl((life+59)/60,1,7);
 char msg[55];char*q=append(msg,j>1?"AFTERGLOW / DOUBLE EXPOSURE ":a?"AFTERGLOW / SUNLINE ":b?"AFTERGLOW / NIGHT BELL ":"AFTERGLOW / GLASSLEAF ");q=number(q,(life+59)/60);append(q,"s");
 int w=textw(msg,0)+16;box(W-w-15,119,w,24,DARK,mix(DARK,color,k));textat(W-w-7,120,msg,color,0);
 if(fastGfx)return;
 U dim=mix(DARK,color,k/2);
 for(int n=0;n<9;n++){int x=7+(n*107+(int)(anim*(a?8:3)))%946,y=144+(n*47)%284;
  if(b){line(x-3,y+1,x+3,y+1,dim);dot(x,y,dim);}
  if(a){dot(x,y-1,dim);line(x-1,y,x+1,y,dim);dot(x,y+1,dim);}
  if(c){line(x,y,x+3,y-3,dim);line(x+3,y-3,x-1,y-6,dim);}
 }
 for(int y=76;y<H-50;y+=2){
  int w0=cl((int)(8*sn(y*.023f+anim)+(b?12:7)),0,28);
  for(int r=0;r<w0;r++)if((r+y)%4==0){dot(r,y,mix(surface[y*rw+r],color,1));dot(W-1-r,y,mix(surface[y*rw+W-1-r],color,1));}
 }
}

/* Hand-built reliefs: parallax plants, multiple body plans, specular glass and
 * layered blinds. Deterministic decoration must not consume the city's RNG. */
static U nvs_hash(U x){x=(x^77u)*2654435761u;return x^(x>>15);}
static void nvs_oval(int x,int y,int rx,int ry,U col){
 if(rx<1||ry<1)return;
 for(int dy=-ry;dy<=ry;dy++){
  int dx=(int)(rx*sq(1.f-(dy*dy)/(float)(ry*ry)));
  rect(x-dx,y+dy,dx*2+1,1,col);
 }
}
static void nvs_grad(int x,int y,int w,int h,U a,U b){
 for(int v=0;v<h;v++){int a0=h-v,b0=v;
 U col=RGB((((a>>16)&255)*a0+((b>>16)&255)*b0)/h,(((a>>8)&255)*a0+((b>>8)&255)*b0)/h,((a&255)*a0+(b&255)*b0)/h);
 rect(x,y+v,w,1,col);}
}
static void nvs_fish(int x,int y,int size,int dir,U color,int phase){
 int r=size,dy=(int)(sn(phase*.084f)*r*.3f);
 tri(x-dir*(r-1),y,x-dir*(r+7),y-r/2+dy,x-dir*(r+7),y+r/2+dy,mix(color,0x2d5462,3));
 nvs_oval(x,y,r,r/2,color);tri(x-r/2,y-1,x-2,y-r/2-3,x+r/2,y-1,mix(color,PAPER,1));
 line(x-dir*(r/2),y+1,x+dir*(r/3),y+1,mix(color,0x294d62,2));
 dot(x+dir*(r-3),y-1,DARK);
 if(r>7){line(x+dir*2,y-r/3,x+dir*4,y-1,mix(color,0x23364c,3));
  for(int k=-r/2;k<r/3;k+=3)dot(x+k,y+r/4,mix(color,PAPER,2));}
}
static void nvs_jelly(int x,int y,int r,U tint,float t){
 for(int h=0;h<r/2+2;h++){
  int wid=(int)(sq(1.f-(1.f-h/(float)(r/2+2))*(1.f-h/(float)(r/2+2)))*r);
  rect(x-wid,y+h,wid*2+1,1,mix(tint,0x293454,cl(h*7/(r/2+3),1,6)));
 }
 for(int k=-r+2;k<=r-2;k+=3)dot(x+k,y+r/2+(k&1),mix(tint,0x494d71,2));
 line(x-r/3,y+2,x+1,y+1,mix(tint,PAPER,2));
 for(int k=-2;k<=2;k++){
  int u=x+k*r/3,v=y+r/2;for(int j=1;j<=6;j++){
   int xx=x+k*r/3+(int)(sn(t*1.6f+j*.75f+k)*3);
   line(u,v,xx,v+r/5+1,mix(tint,0x233659,j));u=xx;v+=r/5+1;
  }
 }
}
static void nvs_ray(int x,int y,int r,int dir,float t,U c){
 int wing=(int)(sn(t)*r*.25f);int a=x+dir*r,b=x-dir*r;
 tri(a,y,x-dir*r/3,y-r/2+wing,b,y,c);
 tri(x-dir*r/3,y-r/2+wing,x-dir*(r+3),y-r/2+wing-3,x-dir*r/2,y-2,mix(c,0x345965,2));
 tri(x-dir*r/3,y+r/2-wing,x-dir*(r+3),y+r/2-wing+3,x-dir*r/2,y+2,mix(c,0x345965,3));
 tri(a,y,b,y,x-dir*r/3,y+r/2-wing,mix(c,0x185259,3));
 line(b,y,b-dir*(r*2),y+(int)(sn(t*.8f)*5),mix(c,0x31575e,2));
 line(a-dir*5,y-2,x-dir*r/4,y-r/2+wing,mix(c,PAPER,2));
 for(int k=-1;k<=1;k++)dot(x-dir*2,y+k*3,0xb8c7aa);
}
static void nvs_plant(int x,int y,int len,float t,U c,int seed){
 int px=x,py=y;for(int i=1;i<9;i++){
  int xx=x+(int)(sn(t*.66f+i*.41f+seed)*i*1.0f),yy=y-len*i/8;
  line(px,py,xx,yy,c);
  int hand=i%2?-1:1;
  tri(xx,yy,xx+hand*(4+i/2),yy-6,px,py,c);
  px=xx;py=yy;
 }
}
/* Habitat silhouettes: each viewing window has its own physical landscape,
 * rendered behind the creatures rather than stamped onto the foreground. */
static void nvs_habitat(int tab,float t){
 const int bottom=329;
 U sea=tab==1?0x262d4b:tab==2?0x25515b:tab==3?0x234552:0x174855;
 /* Bands of light bend through moving surface waves; compact horizontal
  * raster spans keep this inexpensive on the unaccelerated Linux backend. */
 for(int n=0;n<5;n++){
  int head=105+n*177+(int)(sn(t*.26f+n)*5);
  for(int y=91;y<185;y+=3){
   int d=y-91,left=head-d/2,width=7+(d/6);
   for(int j=0;j<width;j+=3){
    int xx=left+j;if(xx<43||xx>917)continue;
    int k=cl(3-d/40-absf(j-width/2)/(float)(width/2+1),1,3);
    U under=surface[y*rw+xx],light=mix(under,tab==1?0x627197:0x71bcb5,k==3?2:1);
    rect(xx,y,min(3,918-xx),2,light);
   }
  }
 }
 for(int i=0;i<88;i++){
  int xx=43+i*10;int rid=13+9*sn(i*.12f+tab)+5*sn(i*.46f);
  rect(xx,bottom-rid,min(10,918-xx),rid,sea);
 }
 if(tab==0||tab==3){
  /* Broad ribbon kelp, in small rocky groups instead of a flat picket. */
  for(int j=0;j<13;j++){
   int root=j<7?55+22*j:725+(j-7)*32,h=47+(j*39)%118;
   int px=root,py=bottom-4;
   U leaf=tab==0?(j&1?0x397774:0x296465):(j&1?0x386473:0x2d566b);
   for(int k=1;k<=13;k++){
    int xx=root+(int)(sn(t*.51f+j*1.6f+k*.33f)*k*1.7f);
    int yy=bottom-4-h*k/13;
    line(px,py,xx,yy,leaf);line(px+1,py,xx+1,yy,leaf);
    if(k>1){int side=(k&1)?1:-1,wide=(14-k)*2+3;
     tri(px,py,xx+side*wide,yy-6,xx+side*4,yy+2,leaf);
     line(px,py,xx+side*(wide-2),yy-4,mix(leaf,0x6a9a82,2));}
    px=xx;py=yy;
   }
  }
 }else if(tab==1){
  /* A submerged intake arch and three physical lamps relight after service. */
  for(int i=0;i<3;i++){int xx=126+i*318;
   rect(xx,216,11,107,0x333c58);rect(xx+13,228,3,94,0x434c67);
   rect(xx-17,209,53,7,0x454962);tri(xx-17,209,xx+9,196,xx+34,209,0x4b5165);
   rect(xx-11,216,41,3,nvs_stage()>=3?0xcebbcb:0x646275);
  }
 }else{
  /* An old tram wheel and the sloped decking on the lagoon floor. */
  line(112,285,311,309,0x9b9b82);line(112,286,311,310,0x6f857b);
  tri(112,287,311,311,178,315,0x426566);
  for(int i=0;i<10;i++)line(124+17*i,287+i*2,120+17*i,304+i,0x64827b);
  disc(256,303,15,0x53747a);disc(256,303,11,0x2b535c);disc(256,303,3,0x7c8c83);
  for(int i=0;i<8;i++)line(256,303,256+(int)(cs(i*.7854f)*11),303+(int)(sn(i*.7854f)*11),0x617e7d);
  for(int j=0;j<19;j++){int xx=390+j*26,yy=311+(int)(6*sn(j*.7));line(xx,yy,xx+16,yy-2,0x567b7a);}
 }
}
static void nvs_tank(int tab,int stage){
 int active=tab!=3||stage>=5;
 float t=nvs_frames/60.f;static const U top[]={0x176e78,0x29365d,0x2d696e,0x476c80};
 static const U base[]={0x092d3b,0x191e39,0x183943,0x172d43};
 box(28,72,904,275,0x11252f,0x53676c);box(35,78,890,261,0x23363f,0x75928e);
 int x=42,y=84,w=876,h=248;nvs_grad(x,y,w,h,active?top[tab]:0x263c44,active?base[tab]:0x202935);
 int frame=(int)nvs_frames;
 if(active){
  for(int j=0;j<17;j++){int xx=x+17+j*51+(int)(sn(j+t*.45f)*8);
   U c=mix(top[tab],0xb0e8cf,1);
   line(xx,y+3,xx+11+(j%4)*3,y+4,c);line(xx+10,y+6,xx+21,y+6,mix(top[tab],0x7bada8,1));
   for(int k=0;k<4;k++)line(xx+17-k*2,y+11+k*8,xx+8-k*2,y+18+k*9,mix(top[tab],0x94adba,1));
  }
  for(int j=0;j<62;j++){U seed=nvs_hash(j+tab*103);int xx=x+4+(seed%866);
   int yy=y+14+(seed/937+frame/(3+j%5))%222;U c=mix(base[tab],0x85b8b5,1+j%2);
   dot(xx,yy,c);if(j%6==0)line(xx,yy-1,xx+1,yy-1,c);}
  for(int j=0;j<44;j++){
   int xx=x+2+j*20,dy=(int)(7*sn(j*.6f)+5*sn(j*1.5f));
   int ht=14+(int)(nvs_hash(j*79+tab)%29);U tone=mix(base[tab],tab==1?0x82698f:0x487570,2);
   tri(xx-7,y+h-8+dy,xx+6,y+h-ht,xx+21,y+h-8+dy,tone);
   if(tab==0||tab==3)nvs_plant(xx,y+h-10,25+(j*19)%70,t,tone,j);
  }
  nvs_habitat(tab,t);
  if(tab==1){
   for(int j=0;j<14;j++){U s=nvs_hash(j+102);int xx=84+s%780+(int)(14*sn(t*.32f+j));
    int yy=125+(s/251)%126+(int)(12*cs(t*.51f+j));
    nvs_jelly(xx,yy,8+(s%15)+(int)(2*sn(j+t*2)),stage<3?0x9f94ba:(j&1?0xf3b2ce:0xa7e2dd),t+j);}
  }else if(tab==2){
   for(int j=0;j<7;j++){int dir=j%2?1:-1,cycle=(frame/(2+j%2)+j*154)%716;
    int xx=124+(dir>0?cycle:716-cycle);
    nvs_ray(xx,150+(j*29)%113+(int)(9*sn(t*.7f+j)),15+(j*7)%19,dir,t*1.4f+j,mix(0x41676e,0xb3c5a5,2+j%5));}
  }else{
   for(int j=0;j<(tab==3?47:24);j++){
    int dir=(j%5<2)?-1:1;int pos=(frame*(1+j%3)/4+j*97)%785;
    int xx=83+(dir>0?pos:785-pos),yy=139+(j*71)%115+(int)(7*sn(t*1.7f+j*.37f));
    U tint=tab==3?(j%4==0?0xf0d595:0xa2d5dc):(j%6==0?0xffb966:j%3?0x7db5ad:0x94c6be);
    nvs_fish(xx,yy,4+(j*7)%8,dir,tint,frame+j*33);
   }
   if(tab==0){int k=frame%210;nvs_fish(nvs_wrasse(frame),214+(int)(4*sn(t)),16,k<105?1:-1,0xffc763,frame);}
  }
  for(int j=0;j<24;j++){
   int xx=x+8+j*37;U s=nvs_hash(j+881);int ht=8+s%15;
   tri(xx-10,y+h,xx+6,y+h-ht,xx+29,y+h,0x1c454e);
   if((tab==0||tab==3)&&j%2==0)nvs_plant(xx+8,y+h-2,24+j%3*16,t,0x477c71,j+19);
  }
  for(int j=0;j<3;j++){
   int xx=x+113+j*296;line(xx,y+9,xx-21,y+35,mix(top[tab],PAPER,2));
   line(xx+5,y+10,xx-6,y+25,mix(top[tab],PAPER,1));}
  if(nvs_mode==1){
   int lo=417,hi=533;U ink=GOLD;
   for(int i=0;i<2;i++){int xx=i?hi:lo,sg=i?-1:1;
    line(xx,188,xx+sg*13,188,ink);line(xx,188,xx,201,ink);line(xx,239,xx+sg*13,239,ink);line(xx,239,xx,226,ink);}
  }
 }else{
  for(int yy=y+7;yy<y+h;yy+=14){rect(x,yy,w,7,0x34484c);rect(x,yy+7,w,2,0x192d33);}
  box(330,157,300,104,0x15262c,0xa27f48);rect(339,166,7,86,0xb89158);
  textat(367,174,"EAST WING",GOLD,1);textat(369,214,stage==4?"CLEARED TO REOPEN":"RESTORATION IN PROGRESS",PAPER,0);
 }
 rect(x,y+h-1,w,2,0x7c978b);rect(x,y+h+2,w,3,0x172933);
 for(int i=0;i<9;i++)rect(42+i*108,338,6,3,0x7b8e87);
}
static void nvs_symbol(int x,int y,int size,int model,U col){
 /* A playful fictional pulse prop, with circular resonators and prism tail. */
 int s=size;U d=mix(col,0x233744,4);
 box(x-4*s,y-4*s,19*s,8*s,d,col);rect(x-8*s,y-2*s,5*s,4*s,col);
 tri(x+3*s,y+3*s,x+10*s,y+3*s,x+4*s,y+10*s,d);
 disc(x+5*s,y,5*s,0x30424c);disc(x+5*s,y,3*s,col);disc(x+5*s,y,s,0xecf3cc);
 for(int i=0;i<(model?4:2);i++)rect(x+(10+i*2)*s,y-2*s,s,4*s,mix(col,0xdaeccc,2));
}
static void nvs_pulse_picture(int x,int y,int size,int frame,int glass,int cell,U tint){
 if(!frame){nvs_symbol(x,y,size,0,tint);return;}
 const int z0=size,length=frame==1?17:frame==2?12:25;
 U light=nvk_cell_ink[cell],dark=mix(tint,0x1a2a3c,4);
 tri(x-length*z0,y,x+7*z0,y-5*z0,x+(length-2)*z0,y,mix(tint,PAPER,2));
 tri(x-length*z0,y,x+7*z0,y+5*z0,x+(length-2)*z0,y,dark);
 if(frame==3)for(int i=0;i<3;i++){rect(x+(i-1)*7*z0,y-7*z0,2*z0,14*z0,light);rect(x+(i-1)*7*z0,y-4*z0,6*z0,8*z0,dark);}
 if(frame==2){disc(x-7*z0,y,5*z0,dark);disc(x-7*z0,y,3*z0,light);}
 if(frame==1)for(int i=0;i<3;i++)disc(x+i*3*z0,y,2*z0,light);
 if(glass==1){disc(x+(length-2)*z0,y,4*z0,tint);disc(x+(length-2)*z0,y,2*z0,light);}
 if(glass==2)box(x-2*z0,y-8*z0,10*z0,5*z0,light,tint);
}
static void nvs_flat(int x,int y,int w,int h,U tint){box(x,y,w,h,tint,mix(tint,0x131923,4));rect(x+1,y+1,w-2,2,mix(tint,PAPER,2));}
static void nvs_loft(void){
 int night=sv.dayClock<360||sv.dayClock>1080;
 nvs_grad(30,76,900,264,nvs_sky()?0x38484f:night?0x27363e:0x42494a,nvs_sky()?0x534539:0x453b39);
 if(nvs_sky()){
  nvs_grad(40,88,880,183,night?0x2f4a61:0x688c9c,0xb88a78);
  for(int j=0;j<68;j++){U k=nvs_hash(j+68);int xx=40+j*13,ht=20+k%92;
   rect(xx,211-ht,11,ht,0x405b68);for(int q=0;q<ht/8;q++)if((j+k+q)%3==0)rect(xx+2,209-q*7,4,2,0xa4af98);}
  for(int j=0;j<8;j++)rect(40+j*125,88,5,181,0x56544c);
  rect(40,212,879,59,0x59544a);
 }
 for(int j=0;j<7;j++){int yy=86+j*24;rect(31,yy,898,1,0x4c4c47);
  for(int k=1;k<9;k++)rect(36+k*102+(j%2)*37,yy-10,1,9,0x444747);}
 /* Harbor window: mullions in front of a tidal inlet and layered skyline. */
 box(337,91,304,122,0x644f44,0xaa907a);nvs_grad(343,97,292,109,night?0x264355:0x6f9ca4,night?0x726776:0xd6c3a0);
 for(int i=0;i<22;i++){int xx=344+i*13,ht=9+nvs_hash(i*783)%39;rect(xx,170-ht,11,ht,night?0x384d59:0x607d81);
  for(int m=0;m<2;m++)for(int f=0;f<ht/9;f++)if((i+m+f)%3==0)rect(xx+2+4*m,166-8*f,2,2,0xcebd8e);}
 nvs_grad(343,170,292,36,night?0x456775:0x93afb0,night?0x304950:0x527e81);
 for(int j=0;j<31;j++){int xx=344+(j*81+(int)(nvs_frames/8))%274;rect(xx,173+j*11%30,8+j%3*3,1,night?0x73929b:0xa9c4ba);}
 for(int i=0;i<4;i++)rect(343+i*95,97,5,109,0x4c4643);rect(341,143,294,4,0x514b45);
 /* Cabinet, bed, fitted fountain, and the pulse cradle are distinct objects. */
 nvs_flat(64,138,116,163,0x6b6450);
 for(int i=0;i<3;i++){nvs_flat(70,148+i*45,104,40,0x576461);rect(112,158+i*45,28,4,0xa69e82);box(82,169+i*45,18,7,0xb3af8c,0x817968);}
 box(199,119,115,93,0x172f36,0xb69a70);box(206,126,101,79,0x244550,0x667976);
 if(nv.world[NVS_MOUNT]&&sv.gear>=0)nvs_pulse_picture(255,160,2,nvk_cfg(NVK_CASE_BIT,3),min(2,nvk_cfg(NVK_CASE_LENS_BIT,3)),min(2,nvk_cfg(NVK_CASE_CELL_BIT,3)),nvs_colors[nvs_finish_id()]);
 else{line(219,180,227,180,0x718582);line(260,180,268,180,0x718582);rect(233,190,41,3,0x586e6e);}
 nvs_flat(195,243,124,53,0x554c45);
 box(672,150,75,97,0x2f3442,0x867c78);rect(680,158,59,29,0x5f7579);for(int i=0;i<2;i++){
  rect(686+i*31,191,14,5,i?0xae9dbb:0xd2a96c);rect(692+i*31,197,5,16,0xa3a9a1);rect(681+i*31,225,23,12,0x66646c);}
 nvs_flat(660,244,100,55,0x675649);rect(671,251,77,4,0x8b7464);
 nvs_flat(365,242,268,48,0x8a7660);box(352,225,287,38,0x66736d,0x43524f);
 box(363,219,54,18,0xbfb399,0x958e7c);rect(427,223,203,31,0x8b6f70);
 for(int xx=435;xx<632;xx+=15)line(xx,224,xx+8,253,0xa38980);
 rect(367,279,10,23,0x463f3c);rect(617,279,10,23,0x463f3c);
 /* A green hanging plant and a shaded reading lamp soften hard edges. */
 rect(804,210,61,8,0x756957);nvs_flat(820,188,32,23,0x9c7962);
 for(int j=0;j<8;j++)nvs_plant(825+j*2,188,28+(j*7)%22,nvs_frames/300.f,0x6f8970,j+2);
 rect(318,197,3,77,0xb09f7e);tri(321,165,304,198,337,198,0xc4ae84);rect(307,198,28,4,0x776b5a);rect(309,274,21,4,0x877760);
 nvs_grad(31,301,898,33,0x514f46,0x3a3b38);for(int i=0;i<14;i++)line(49+i*65,304,31+i*68,334,0x666052);
 if(nvs_sky()&&nvs_renovated()){
  box(779,117,129,108,0x294653,0xbaab7b);
  nvs_grad(783,122,121,98,0x376b69,0x142d36);
  for(int j=0;j<8;j++)nvs_plant(789+j*15,219,30+j*7%50,nvs_frames/180.f,0x54977c,j);
  for(int j=0;j<18;j++)rect(41+j*49,320,13,2,0xb8a884);
  rect(389,225,215,25,0xb4ae95);for(int j=0;j<10;j++)line(393+j*20,227,401+j*20,248,0x7e9a97);
 }
 if(nvs_tab==3&&nvs_recov_ticks>0){
  int k=90-nvs_recov_ticks,up=nvs_recov_ticks>44;
  person(789,269,0x87aaa3,0,0.f,4,1);
  int hand=up?235+(k<30?8-k/4:0):241;
  if(nvs_sel==NVF_GLASSLEAF){
   line(785,hand,791,hand-4,TEAL);for(int j=0;j<3;j++){int x=795+(int)(sn(k*.25f+j)*6),y=hand-10-j*6-k%10;line(x,y,x+3,y-4,0x82b2a8);}
  }else{box(787,hand,12,9,nvs_sel==NVF_NIGHTCAP?0xb78cc5:0xe0bd79,0xecdbc2);line(789,hand+3,796,hand+3,0xd6c7ae);}
 }
 int focus[]={123,255,507,715,836};int xx=focus[nvs_tab];tri(xx-7,106,xx+7,106,xx,114,0xe7cb88);
 box(30,75,900,3,0x8a8576,0x8a8576);rect(30,334,900,3,0x8a8576);
}
static void nvs_tabs(const char*const*names,int n,int selected){
 int w=904/n;
 for(int i=0;i<n;i++){int x=30+i*w,active=i==selected;
  rect(x,350,w-5,33,active?0x254245:0x17282e);rect(x,380,w-5,3,active?0x90d9c1:0x34454b);
  center(x+(w-5)/2,355,names[i],active?PAPER:MUTED,0);}
}
static void nvs_head(const char *eyebrow,const char *title){
 rect(0,0,W,H,0x101e26);rect(0,0,9,H,0x92d8c2);
 textat(29,8,eyebrow,0x9ac7bc,0);textat(28,29,title,PAPER,1);
 char clockbuf[6];day_clock_label(clockbuf);textat(796,9,"PORT VANTA",MUTED,0);textat(803,34,clockbuf,GOLD,0);rect(30,66,900,1,0x466269);
}
static void nvs_tail(const char*label,const char*line1,const char*line2,const char*keys){
 textat(31,395,label,GOLD,0);
 nav_wrap(31,418,892,line1,PAPER,2);
 nav_wrap(31,465,894,nvs_log[0]?nvs_log:line2,0xaabebd,2);
 rect(30,510,900,1,0x49585c);textat(32,515,keys,TEAL,0);
 if(nvs_saved&&saveFlash<0)textat(813,516,"SAVE FAILED",PINK,0);
}
static const char*const nvs_orders[]={
 "Inez / Coast Keeper   |   Sign on to restore the four galleries.",
 "01 / Use Kelp Forest's viewfinder to tag the gold wrasse.",
 "02 / Bring the three lamps online at Jelly Drift.",
 "03 / Guide a stranded kite ray through Ray Lagoon's marked channels.",
 "04 / Raise the shutter at Shoal Walk and collect the grant.",
 "05 / All four exhibits are open. Staff credit: Port Vanta volunteer corps."};
static void nvs_aqua_paint(void){
 int stage=nvs_stage();
 nvs_head("PORT AUTHORITY   /   UNDERGLASS CONSERVATION","The Glass Relay");
 nvs_tank(nvs_tab,stage);nvs_tabs(nvs_exhibit,4,nvs_tab);
 if(nvs_mode==4){
  nvs_grad(42,284,876,62,0x455c5b,0x34464a);
  rect(42,283,876,3,0x9ca988);
  static const char*const stops[]={"Inez","Lightwell","Nursery","East bench"};
  for(int i=0;i<4;i++){int xx=119+i*222;rect(xx,285,2,32,0xa2b092);
   box(xx-48,291,101,23,0x18313a,nvs_bonus&(1<<i)?TEAL:GOLD);center(xx+2,291,stops[i],PAPER,0);}
  person(151,329,0xbcb98a,0,3.14f,MODEL_STAFF,1);
  person(nvs_position,335,0x51948b,nvs_rail_walk?(int)(nvs_frames%16)/4:0,nvs_rail_dir<0?3.14159f:0.f,MODEL_PLAYER,1);
  nvs_tail("WALK THE EAST WING","Four marked stops: E to meet Inez or inspect the viewing station by your feet.",
   "The reopened promenade is visible here and remains available after restarting.",
   "LEFT / RIGHT: walk the rail     E / Inspect stop     ESC / Back to gallery");
  return;
 }
 if(nvs_mode==2){
  for(int i=0;i<3;i++){int xx=265+i*160;
   box(xx,236,115,72,0x192629,nvs_sel==i?GOLD:0x749796);char row[30];fmt(row,"TARGET  ",nvs_targets[i]);center(xx+57,240,row,MUTED,0);
   number(row,nvs_dials[i]);center(xx+57,263,row,nvs_targets[i]==nvs_dials[i]?TEAL:PAPER,1);}
 }
 if(nvs_mode==3){
  for(int i=0;i<3;i++){int xx=249+i*157;box(xx,252,148,44,0x193038,i==nvs_step?GOLD:0x588480);
   center(xx+74,262,nvs_motion[i],i<nvs_step?TEAL:i==nvs_step?GOLD:MUTED,0);}
 }
 int waiting=stage==5&&nvs_tab==3&&(nv.world[NVS_STAGE]&(NVS_GRANT_HELD|NVS_STIPEND_HELD));
 const char *actions=waiting?"E: collect held payment":stage==0?"E: sign work order":stage==1&&nvs_tab==0?"E: start observation":stage==2&&nvs_tab==1?"E: service lamp bank":stage==3&&nvs_tab==2?"E: guide the kite ray":stage==4&&nvs_tab==3?"E: reopen east wing":"E: inspect exhibit";
 const char *commands=nvs_mode==1?"E / Capture through brass frame     ESC / Cancel":
 nvs_mode==2?"LEFT / RIGHT: select lamp     E / Turn dial     ESC / Cancel":
 nvs_mode==3?"ARROWS / Route the ray     ESC / Cancel":waiting?"LEFT / RIGHT: gallery     E / Collect     SPACE / Walk the open wing     ESC / Lobby":stage>=5&&nvs_tab==3?"LEFT / RIGHT: gallery     E / Inspect     SPACE / Walk the open wing     ESC / Lobby":"LEFT / RIGHT: change gallery     E / Interact     ESC / Return to lobby";
 nvs_tail(nvs_mode?"FIELD OPERATION":actions,stage==5&&(nv.world[NVS_STAGE]&(NVS_GRANT_HELD|NVS_STIPEND_HELD))?"Inez has a payment on hold. Make wallet space and press E at Shoal Walk.":nvs_orders[stage],
 "The four aquariums share a live glass relay. Switch galleries to operate their viewing stations.",commands);
}
static const char*const nvs_pantry[]={"Sunline / bright fizz","Night Bell / cocktail   $5","Glassleaf / selected roll","Quiet reset"};
static const char*const nvs_sleep[]={"Take a short nap / 2h","Sleep a full shift / 8h","Wait out weather / 4h"};
static void nvs_home_paint(void){
 char title[130]={0},summary[158]={0};const char *help="";const char *note="";
 nvs_head("RESIDENT SERVICES   /   PRIVATE QUARTERS",nvs_sky()?"Glass Crown Penthouse":interior_active_loc==9?"The Breakwater Loft":"Apartment Four");
 nvs_loft();nvs_tabs(nvs_places,LEN(nvs_places),nvs_tab);
 if(nvs_tab==0){
#if __has_include("night_phone.inc")
  if(nvs_folio){
   np_print(title,nvs_sel);char*q=append(summary,"Satchel: ");q=number(q,np_get(NP_BAG+nvs_sel,99));if(nvs_ed_review(nvs_sel))append(q,"  /  Shelf on hold: ask Bryn at The Afterimage to resolve your old ride receipt. M pins the club.");
   else{q=append(q,"  /  Private shelf: ");q=number(q,nvs_ed_count(nvs_sel));append(q,"  /  Keeps this exact strain and grade.");}
  }else
#endif
  {
   int i=nvs_sel/2,g=nvs_sel%2;char*q=append(title,prod[i].name);append(q,g?"   /   SELECT CASE":"   /   HOUSE SHELF");
   q=append(summary,"Bulk warehouse: ");q=number(q,sv.stocks[i]);q=append(q,"    /    On shelf: ");q=number(q,nv.world[NVS_SHELF+nvs_sel]);append(q,g?"   /   $3 clean to case each lot.":"    /    Free to store.");
  }
#if __has_include("night_phone.inc")
  help=nvs_folio&&nvs_ed_review(nvs_sel)?"M / Pin Bryn     T / Stock or lots     UP/DOWN / Bin     LEFT/RIGHT / Room     ESC / Back":"T Stock/Lots   UP/DOWN Bin   E Store   SPACE Take   M Pin aquarium   LEFT/RIGHT   ESC";
#else
  help="UP/DOWN Shelf   E Store   SPACE Take   M Pin aquarium   LEFT/RIGHT Room   ESC Back";
#endif
  note=nvs_folio?"Nine private bins: three fictional strains, each graded House, Select and Top Shelf.":"Three legacy stock varieties, each with a House and Select shelf.";
 }else if(nvs_tab==1){
  char*q=append(title,nvs_finish[nvs_sel]);append(q,nvs_has(nvs_sel)?"  /  IN CATALOGUE":"  /  NOT COLLECTED");
  nvk_format(summary,nvk_frame(),nvk_glass(),nvk_kind(),nvk_load());
  help="UP/DOWN Finish   E Mount   SPACE Clear   M Pin cart   LEFT/RIGHT Room   ESC Back";
  int x,y;
  note=night_venue_vendor_pos(&x,&y)?"Night Cart is open. M tracks its moving route. E mounts the equipped look.":"Night Cart opens 19:00-05:00. M pins the cosmetic gallery when the van is closed.";
 }else if(nvs_tab==2){
  append(title,nvs_sleep[nvs_sel]);append(summary,"Rest changes city time and restores health. Timed jobs and pursuits block sleep.");
  help="UP / DOWN: duration     E / Sleep     LEFT / RIGHT: room     ESC / Back";
  note="The city clock, plants and closing time advance. Franchise wages and Revenue deadlines count active play.";
 }else if(nvs_tab==3){
  append(title,nvs_pantry[nvs_sel]);char*q=summary;
  static const char*const effect[]={"Gold light, bright tones and a brisk stride.","Cool blue edges, lower tones and weightier footsteps.","Curling teal haze, a glass chime and a lighter stride.","Clear all afterglow, restore regular movement and reset familiarity."};
  q=append(q,effect[nvs_sel]);
#if __has_include("night_phone.inc")
  if(nvs_sel==NVF_GLASSLEAF){q=append(q,"  Satchel: ");char species[70];np_print(species,nvs_leaf_grade);q=append(q,species);q=append(q," x");number(q,np_get(NP_BAG+nvs_leaf_grade,99));}
#endif
  help=nvs_sel==NVF_GLASSLEAF?"T Strain/grade   UP/DOWN Item   E Use   LEFT/RIGHT Room   ESC Back":nvs_sel==NVF_NIGHTCAP?"UP/DOWN Item   E Buy & drink   LEFT/RIGHT Room   ESC Back":"UP / DOWN: item     E / Use     LEFT / RIGHT: room     ESC / Back";
  note=nv.world[NVS_TOL]>1440?"High familiarity: effects expire earlier. Quiet reset or use the bunk to settle down.":nv.world[NVS_TOL]>60?"Effects can overlap. Familiarity shortens the afterglow.":"Fresh palate. Effects are imaginary and move with your game character.";
 }else{
  append(title,nvs_sky()?(nvs_renovated()?"RENOVATED / GLASS CROWN":"RENOVATE PENTHOUSE   $180"):nvs_penthouse()?"YOUR NEXT KEY / GLASS CROWN":sv.home?"PENTHOUSE AVAILABLE / $640":"STARTER RESIDENCE / APARTMENT FOUR");
  append(summary,nvs_sky()?"The penthouse offers the same private collection and lodging. Renovate for a harbor wall and deluxe bed.":"M pins the Glass Crown tower. Buy the Breakwater Safehouse first to qualify for the penthouse.");
  help="M / Pin Glass Crown     E / Inspect or renovate     LEFT / RIGHT: room     ESC / Back";
  note=nvs_sky()&&nvs_renovated()?"The deluxe bed improves recovery after a full sleep.":"Legacy safehouse counter and leases remain in the original rooms.";
 }
 nvs_tail(title,summary,note,help);
}
static void nvs_lease_paint(void){
 nvs_head("RESIDENT SERVICES   /   SKY CLUB ANNEX","Glass Crown Lease Desk");
 int old=nvs_tab;nvs_tab=4;nvs_loft();nvs_tab=old;
 nvs_tail("PENTHOUSE / $640 CLEAN",sv.home?"Breakwater deed confirmed. Buy the top-floor penthouse to open its private lift.":"Breakwater deed required. Lease it from Reed Sato first.",
  "Private storage, equipment display, refreshments and weather waits are included.",
  "E / Buy penthouse     ESC / Return to Sky Club");
}
static void nvs_cart_paint(void){
 nvs_head("PORT VANTA   /   ITINERANT DESIGN MARKET",nvs_back==1?"Signal Night Cart":"Signal Gallery");
 nvs_grad(32,77,896,248,0x443b49,0x18212b);
 /* A painted night van sits under an awning: wheels, lamps, folding hatch. */
 box(140,137,566,120,0x596565,0xc8b69b);
 tri(706,146,772,214,706,214,0x707c7b);box(701,204,79,53,0x687471,0x8d9c8d);
 tri(714,166,748,202,714,202,0x2b4249);rect(152,145,537,13,0xe8bb83);
 for(int i=0;i<20;i++){rect(158+i*26,143,13,21,0xb36e63);}
 box(159,168,516,65,0x26343e,0xcaa975);for(int j=0;j<3;j++){
  if(nvs_tab==0)nvs_symbol(220+j*172,193,2,1,nvs_colors[j+1]);
  else nvs_pulse_picture(221+j*173,198,2,nvs_tab==1?j+1:max(1,nvk_frame()),nvs_tab==2?j:nvk_glass(),nvs_tab==3?j:nvk_kind(),nvs_colors[j+1]);
 }
 for(int j=0;j<2;j++){int xx=j?707:213;disc(xx,260,26,0x18212b);disc(xx,260,14,0x85928e);disc(xx,260,6,0x36484c);}
 rect(750,225,21,8,0xffcf81);rect(164,235,512,4,0xe2bd92);
 nvs_tabs(nvs_cart_tabs,nvs_back==1?4:1,nvs_tab);
 static const char*const finish_desc[]={"Coat the home display model; this choice is cosmetic.","Coat the home display model; this choice is cosmetic.","Coat the home display model; this choice is cosmetic."};
 int id=nvs_sel;
 const char*const *labels=nvs_tab==0?nvs_finish+1:nvs_tab==2?nvk_lens:nvk_cell;
 const char*desc=nvs_tab==0?finish_desc[id]:nvs_tab==1?nvk_frames[id+1].desc:nvs_tab==2?nvk_lens_desc[id]:nvk_cell_desc[id];
 int price=nvs_tab==0?nvs_prices[id+1]:nvs_tab==1?nvk_frames[id+1].price:nvs_tab==2?nvk_lens_cost[id]:nvk_cell_cost[id];
 int owned=nvs_tab==0?nvs_has(id+1):nvs_tab==1?nvk_owned(id+1):nvs_tab==2?(!id||nvk_owned(id+3)):0;
 for(int j=0;j<3;j++){
  int xx=46+j*293;box(xx,306,282,35,0x25303b,nvs_sel==j?GOLD:0x58666c);
  center(xx+140,311,nvs_tab==1?nvk_frames[j+1].name:labels[j],nvs_colors[j+1],0);
 }
 char title[100],s[150];char*q=append(title,nvs_tab==1?nvk_frames[id+1].name:labels[id]);q=append(q," / ");if(owned)append(q,"OWNED");else fmt(q,"$",price);
 q=append(s,desc);q=append(q,"   Wallet $");number(q,sv.cash);
 char configured[120];
 nvk_format(configured,nvk_frame(),nvk_glass(),nvk_kind(),nvk_load());
 nvs_tail(title,s,nvs_back==1?configured:"Night-only arcade fit-out opens 19:00-05:00 at a rotating street stop.",
 nvs_back!=1?"UP/DOWN Finish    E / Buy    ESC / Back":nvs_tab==1?"LEFT/RIGHT Tab   UP/DOWN Pick   E Equip   SPACE Legacy pulse   ESC Back":nvs_tab==3?"LEFT/RIGHT Tab   UP/DOWN Pick   E Buy 6 cells   SPACE Top up   ESC Back":"LEFT/RIGHT Tab   UP/DOWN Pick   E Buy/install   ESC Back");
}
static void night_venues_paint(void){
 if(nvs_room==NVR_GLASS)nvs_aqua_paint();
 else if(nvs_room==NVR_HOME)nvs_home_paint();else if(nvs_room==NVR_LEASE)nvs_lease_paint();else nvs_cart_paint();
}
