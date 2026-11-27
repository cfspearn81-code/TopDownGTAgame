/* Signal / Port Vanta handset. Persistent fields are uint32 schema values, never pointers.
   Legacy Prism Folio slots now represent fictional cannabis lots; every unit retains
   its strain and quality through the grow cabinet, processing, shelf, satchel and delivery. */
enum { NP_BIND, NP_STATION, NP_POST, NP_CURSOR, NP_CARD, NP_JOB, NP_DONE, NP_SAMPLE,
 NP_NEWTYPE,NP_NEWGRADE,NP_MODULES,NP_BATCH,NP_LEFT,NP_SHELF=16,NP_BAG=25,
 NP_HEAT=34,NP_DECAY=40,NP_HOLD,NP_REP,NP_POPSTAMP,NP_RIVAL,NP_LASTSALE,NP_STREAK,NP_PROFILE,NP_BATCH_MODE,NP_PATROL,NP_SIM,NP_SIM_COOL,NP_ALIAS,NP_HEAT_EAST };
enum { NP_INBOX,NP_TRACE,NP_WIRE,NP_LOFT,NP_STOCK,NP_RADIO,NP_PREF,NP_NAPP,NP_RECEIPTS=NP_NAPP };
/* These are supplied by the release's venue, club, and garage modules. */
static int night_garage_meet_site(int*,int*),night_venue_vendor_stop(void),night_venue_vendor_pos(int*,int*),night_club_signature_demand(int);

static int np_return=1,np_app,np_row,np_confirm,np_link_pending;
static char np_note[148];
static int np_tracking=-1,np_tracking_slot=-1;
static const char *const np_design[]={"Halo Moss","Prism Fern","Echo Bloom"};
static const char *const np_grade[]={"House C","Select B","Top Shelf A"};
static const char *const np_ed[]={"House / C","Select / B","Top Shelf / A"};
typedef struct{const char *name,*line;int id,qty,venue,pay;} NPBrief;
/* Entry zero is an always-available legal courier route with a prepaid parcel. */
static const NPBrief np_brief[]={
 {"Mara / Dispatch","A field proof for tonight's ferry zine. This legal parcel is prepaid.",0,1,0,105},
 {"Lux / Apartment Four","Two lots of the House grade. Halo Moss only, please.",0,2,12,154},
 {"Orin / Ferry stand","Select Halo Moss only. One lot for the collection.",1,1,2,198},
 {"Ada / Kiosk Nine","Two matching House Prism Fern lots, please.",3,2,7,185},
 {"Tavi / Dispatch corner","Two Select Prism Fern lots, please. Not House grade.",4,2,0,295},
 {"Eli / Breakwater","One Top Shelf Echo Bloom. I won't trade for another grade.",8,1,9,425},
 {"Jun / Tideway","I'm after one Top Shelf Halo Moss lot.",2,1,8,305},
 {"Sol / Ferry Market","Two House Echo Bloom. Same strain, same grade.",6,2,2,245},
 {"Rin / Glass Mile","One Top Shelf Prism Fern for the collection.",5,1,6,365},
 {"Nell / South shelves","Two Select Echo Bloom lots to complete an order.",7,2,9,345}
};
static const char *const np_apps[]={"Inbox","Quest log","Night Wire","Glasshouse","Stockroom","Radio","Settings"};
static const char *const np_sub[]={"Customer requests","Routes & milestones","Posts from the city","Lantern grow house","Shelves & satchel","Original city stations","Keys & device"};
static const U np_color[]={0x7ce6cc,0xffcf82,0xff99ae,0xaccf86,0xafb1fc,0x83c7fa,0xb0bac6};
static int np_get(int k,int cap){U a=nv.phone[k];return a>(U)cap?cap:(int)a;}
static int np_area(int x,int y){
#ifdef BASE_ROOM
 if(x>=BASE_ROOM-36||y>=BASE_ROOM-36)return y<820?6:y<1130?7:y<1570?8:x>=2180?9:10;
#endif
 if(y<545)return 0;if(y>1160)return x<900?1:2;return x<850?3:x<1400?4:5;
}
static const char*const np_areas[]={"Glass Mile","Lantern Ward","Breakwater","Tide Market","Nightglass","East Freight",
 "Glass Bluff","Freight Parkway","Belt Workshops","Harbor Pier","Salt Reach"};
static int np_heat_index(int area){return area<6?NP_HEAT+area:NP_HEAT_EAST+area-6;}
static int night_phone_heat_at(int x,int y){return np_get(np_heat_index(np_area(x,y)),100);}
static void night_phone_heat_add(int x,int y,int delta){int k=np_heat_index(np_area(x,y));nv.phone[k]=cl(np_get(k,100)+cl(delta,-100,100),0,100);}
#define NIGHT_PHONE_HAS_REST 1
static void night_phone_rest(int hours){
 /* Only the saved arcade district meter, not the active patrol state. */
 night_phone_heat_add((int)p.x,(int)p.y,-3*cl(hours,0,8));
}
static int night_phone_signature(int design,int consume){
 if(design<0||design>=LEN(np_design))return 0;
 int k=NP_BAG+design*3+2,n=np_get(k,99);
 if(!consume)return n;
 if(!n)return 0;
 nv.phone[k]=(U)(n-1);return 1;
}
static int night_phone_premium(int consume){int count=0;for(int i=0;i<3;i++){int n=night_phone_signature(i,consume);if(consume&&n)return 1;count+=n;}return count;}
static int night_phone_radio(void){int v=np_get(NP_STATION,4);if(v<4)return v;static const int a[]={1,2,3,2,1,3,1,3,3,2,2};return a[np_area((int)p.x,(int)p.y)];}
static int night_phone_toggle(U tap){return !!(tap&(np_get(NP_BIND,1)?PHONE_ALT:PHONE));}
static void np_msg(const char*s){int i=0;while(i<((int)sizeof np_note)-1&&s[i]){np_note[i]=s[i];i++;}np_note[i]=0;}
static int np_job(void){U v=nv.phone[NP_JOB];return v>0&&v<=(U)LEN(np_brief)?(int)v-1:-1;}
static int np_house_near(void){return p.car<0&&((scene==9?np_return:scene)==3?interior_active_loc==4:
  (scene==1||(scene==9&&np_return==1))&&d2(p.x-loc[4].x,p.y-loc[4].y)<76.f*76.f);}
static int np_unlocked(int id){int t=id/3,g=id%3,m=np_get(NP_MODULES,7);return (t==0||(m&(1<<(t-1))))&&(g==0||(g==1?(m&3)!=0:(m&4)!=0));}
static int np_capacity(void){int m=np_get(NP_MODULES,7);return 2+!!(m&1)+!!(m&2)+!!(m&4);}
static int np_load(int id){return 1+id/3+id%3;}
static int np_active_batch(void){U v=nv.phone[NP_BATCH];return v>0&&v<=9?(int)v-1:-1;}
static int np_active_left(void){return np_get(NP_LEFT,4500);}
static int np_sel(void){return 3*np_get(NP_NEWTYPE,2)+np_get(NP_NEWGRADE,2);}
/* Cabinet profiles are abstract game modifiers, not production settings. */
static const char *const np_profile[]={"Balanced","Low draw","Showpiece"};
static int np_profile_mode(void){return np_get(NP_PROFILE,2);}
static int np_profile_power(int id,int mode){return max(1,np_load(id)+(mode==1?-1:mode==2?1:0));}
static int np_batch_yield(int mode){return mode==1?2:mode==2?4:3;}
static int np_batch_seconds(int id,int mode){return 18+id%3*8+id/3*5+(mode==1?10:mode==2?-4:0);}
static int np_cost(int id,int mode){return 14+id/3*8+id%3*7+(mode==1?-3:mode==2?10:0);}
static int np_job_dest(void){int j=np_job();if(j<0)return -1;if(j==0&&!np_get(NP_SAMPLE,1))return 2;return np_brief[j].venue;}
static int night_phone_route(int*x,int*y){int v=np_job_dest();if(v<0)return 0;*x=loc[v].x;*y=loc[v].y;return 1;}
static void np_pin(int id,int x,int y){np_tracking=-1;nav_slot();navHasManual=1;navManualLoc=id;navManual=(NavPoint){x,y};navGoalKind=0;}
static void np_venue(int v){np_pin(v,loc[v].x,loc[v].y);}
static void np_clear_ours(int v){if(np_tracking_slot==sideSlot&&np_tracking==v&&navHasManual&&navManualLoc==v&&v>=0){np_tracking=-1;navHasManual=0;navManualLoc=-1;navGoalKind=0;}}
static void np_track(void){int v=np_job_dest();if(v>=0){np_venue(v);np_tracking=v;np_tracking_slot=sideSlot;np_msg("Address pinned. Open the atlas with M on the street.");}else np_msg("No active request. Choose a customer in Inbox.");}
static void np_print(char*d,int id){char*q=append(d,np_design[cl(id,0,8)/3]);q=append(q," / ");append(q,np_grade[cl(id,0,8)%3]);}
static int np_offer(int r){if(!r)return 0;int start=(int)(nv.phone[NP_DONE]%9u);return 1+(start+r-1)%9;}
static int night_phone_take_link(void){int p=np_link_pending;np_link_pending=0;return p;}
static void night_phone_link_result(int ok){
 if(scene==NIGHT_PHONE_SCENE&&np_app==NP_RADIO&&np_row==5)
  np_msg(ok?"Handed off to your browser. Music plays there, not in Dead Channel.":"Couldn't launch the browser. Open https://music.youtube.com/ yourself.");
}
static int np_hour(void){return cl((int)sv.dayClock/60,0,23);}
static int np_live(void){return np_hour()>=20||np_hour()<5;}
/* Scope note: all locations, customers and stock are fictional game objects. */
static int np_fair_selector(void){int h=np_hour();return h>=8&&h<18?1:h>=22||h<2?2:0;}
static int np_fair_location(int which){return which==2?7:2;}
static const char *np_fair_name(int which){return which==2?"Kiosk Lantern Exchange":"Ferry Night Garden";}
static U np_stamp(void){return 1u+sv.playedTicks/(DAY_SECONDS*60u);}
static int daily_rewards_tick(int closed,float dt){
 const U day=DAY_SECONDS*60u;
 int bucket=!closed&&absf(dt-1.f/60.f)<.000001f&&sv.playedTicks&&!(sv.playedTicks%day);
 if(!closed&&!bucket)return 0;
 U *marks[]={&nv.phone[NP_POPSTAMP],&nv.garage[36]},stamp=np_stamp();int changed=0;
 for(int i=0;i<2;i++){
  if(closed&&*marks[i]){*marks[i]=0;changed=1;}
  else if(bucket&&*marks[i]==stamp-1){*marks[i]=stamp;changed=1;}
 }
 return changed;
}
static int np_new_order(void){return 1+(int)(nv.phone[NP_DONE]%9u);}
enum {NP_PARKWAY_X=2464,NP_PARKWAY_Y=1024,NP_PARKWAY_MIN=968,NP_PARKWAY_MAX=1080};
static int np_closed(void){int h=np_hour();return (h>=6&&h<8)||(h>=16&&h<18);}
static int night_phone_lane_stop(float x,float y,float nx,float ny,int radius){
 if(!np_closed())return 0;
 /* Do not prevent movement away from the boundary if a new closure
  * starts while already between the barricade posts. */
 if(y+radius<=NP_PARKWAY_MIN&&ny+radius<=NP_PARKWAY_MIN)return 0;
 if(y-radius>=NP_PARKWAY_MAX&&ny-radius>=NP_PARKWAY_MAX)return 0;
 if(x<NP_PARKWAY_X&&nx>x&&nx+radius>=NP_PARKWAY_X)return 1;
 if(x>NP_PARKWAY_X&&nx<x&&nx-radius<=NP_PARKWAY_X)return 1;
 return 0;
}
static void night_phone_contact_paint(void){
 int j=np_job(),at=np_job_dest();if(j<1||at<0||np_get(NP_HOLD,600)>0)return;
 const Place *spot=&loc[at];
 if(d2(p.x-spot->x,p.y-spot->y)>210.f*210.f)return;
 static const int dx[]={22,-22,16,-22,0},dy[]={8,9,22,20,20};
 for(int i=0;i<LEN(dx);i++){
  int x=spot->x+dx[i],y=spot->y+dy[i];
  if(solid(x,y,7))continue;
  int xx=sx(x),yy=sy(y);
  if(xx>18&&xx<W-18&&yy>12&&yy<H-13)disc(xx,yy-29,2,0xff9adb);
  break;
 }
}
static void night_phone_road_paint(void){
 if(!np_closed())return;
 int x=NP_PARKWAY_X-cx,y=NP_PARKWAY_Y-cy;
 if(x<-24||x>W+24||y<-85||y>H+85)return;
 for(int r=-50;r<=50;r+=20){
  box(x-5,y+r-5,10,10,0x4a3331,0xd7aa71);
  if(((int)(anim*2)+r/20)%2==0)disc(x,y+r-3,2,GOLD);
  else disc(x,y+r-3,2,0x6f5543);
 }
 for(int r=-54;r<=54;r+=8)rect(x-2,y+r,4,6,0xe4aa60);
}
static int np_cash(int amt){if(!canDirtyPurchase(amt)){np_msg(notice);return 0;}return 1;}
static void np_commit(void){int cross=revenueCrossed;finish_purchase();if(cross)np_msg(notice);request_save();}
static void np_accept(int j){
 if(np_job()>=0){np_msg("Finish or cancel the tracked request before calling back.");return;}
 nv.phone[NP_JOB]=j+1;nv.phone[NP_SAMPLE]=0;nv.phone[NP_HOLD]=0;nv.phone[NP_RIVAL]=0;nv.phone[NP_PATROL]=0;
 np_app=NP_TRACE;np_row=0;
 np_msg(j?"Request accepted. Match the exact strain and grade, then meet on foot.":"Prepaid proof reserved. Walk up to Ferry Market and press E.");request_save();
}
static void np_sow(void){
 int id=np_sel(),mode=np_profile_mode(),power=np_profile_power(id,mode),produce=np_batch_yield(mode);if(!sv.b[0].own){np_msg("Lease Lantern Grow Loft first.");return;}
 if(np_active_batch()>=0){np_msg("One lot at a time. Process the current lot first.");return;}
 if(!np_unlocked(id)){np_msg("Locked strain or grade. Check the upgrade menu.");return;}
 if(power>np_capacity()){np_msg("That lot exceeds the cabinet's power budget.");return;}
 if(np_get(NP_SHELF+id,99)>99-produce){np_msg("This shelf is full. Move lots to your satchel first.");return;}
 if(!np_cash(np_cost(id,mode)))return;
 nv.phone[NP_BATCH]=id+1;nv.phone[NP_LEFT]=np_batch_seconds(id,mode)*60;nv.phone[NP_BATCH_MODE]=(U)mode;
 if(power>=3)night_phone_heat_add(loc[4].x,loc[4].y,power+(mode==2?4:0));
 np_msg("Cabinet running. It advances while you play, not while reading Signal.");np_commit();
}
static void np_bind(void){int id=np_active_batch();if(id<0){np_msg("Nothing to process. Queue a lot in Glasshouse.");return;}
 if(np_active_left()){np_msg("Not ready. Return after the cabinet countdown.");return;}
 if(!np_house_near()){np_venue(4);np_msg("Process the ready lot at Lantern Grow Loft on foot. Atlas pinned.");return;}
 int produce=np_batch_yield(np_get(NP_BATCH_MODE,2));
 if(np_get(NP_SHELF+id,99)>99-produce){np_msg("Free enough shelf space before processing your lot.");return;}
 nv.phone[NP_SHELF+id]=np_get(NP_SHELF+id,99)+produce;nv.phone[NP_BATCH]=0;nv.phone[NP_LEFT]=0;nv.phone[NP_BATCH_MODE]=0;
 char words[92];number(append(words,"Processed and shelved "),produce);append(words+sl(words)," matching units. Collect them in Stockroom.");np_msg(words);request_save();
}
static void np_xfer(int id){int n=np_get(NP_SHELF+id,99),b=np_get(NP_BAG+id,99);
 if(!n){np_msg("This shelf is empty. Process a ready Glasshouse lot.");return;}
 if(!np_house_near()){np_venue(4);np_msg("Visit Lantern Grow Loft on foot to collect stock. Atlas pinned.");return;}
 int amt=min(3,min(n,99-b));if(!amt){np_msg("That satchel slot is full.");return;}nv.phone[NP_SHELF+id]=n-amt;nv.phone[NP_BAG+id]=b+amt;
 np_msg("Lots moved to satchel with strain and grade intact.");request_save();
}
static int np_modcost(int i){return 95+65*i;}
static void np_module(int which){int m=np_get(NP_MODULES,7);if(!sv.b[0].own){np_msg("Lease Lantern Grow Loft first.");return;}
 if(m&(1<<which)){np_msg("Already fitted. This module adds one power slot.");return;}
 if(!np_cash(np_modcost(which)))return;nv.phone[NP_MODULES]=m|(1<<which);np_msg("Module fitted. Strain unlocks and power limits updated.");np_commit();
}
static void night_phone_world_tick(float dt){
 if(dt<=0||p.dead)return;
 int k=cl((int)(dt*60.f+.5f),1,60);
 int left=np_active_left();if(np_active_batch()>=0&&left){nv.phone[NP_LEFT]=max(0,left-k);if(!nv.phone[NP_LEFT]){tell("Signal / Lantern lot ready. Process it at the loft.");request_save();}}
 int cooldown=np_get(NP_SIM_COOL,3600);if(cooldown)nv.phone[NP_SIM_COOL]=max(0,cooldown-k);
 int h=np_get(NP_HOLD,600);if(h){nv.phone[NP_HOLD]=max(0,h-k);if(!nv.phone[NP_HOLD])tell("Night Wire / Your contact is back. Resume the handoff.");}
 int hasheat=0;for(int i=0;i<LEN(np_areas);i++)hasheat|=np_get(np_heat_index(i),100);if(!hasheat){nv.phone[NP_DECAY]=0;return;}
 int t=np_get(NP_DECAY,3600)+k;if(t>=1800){for(int i=0;i<LEN(np_areas);i++){int hi=np_heat_index(i),q=np_get(hi,100);nv.phone[hi]=max(0,q-2);}t=0;}nv.phone[NP_DECAY]=t;
}
static int night_phone_action(U tap){
 if(!(tap&USE)||scene!=1||menu>=0||mapOn||p.dead||p.car>=0)return 0;
 int fair=np_fair_selector(),venue=np_fair_location(fair);
 if(fair&&np_get(NP_CARD,2)==fair&&d2(p.x-loc[venue].x,p.y-loc[venue].y)<43.f*43.f){
  U stamp=np_stamp();int slot=fair==2?NP_BAG+3:NP_BAG;
  /* Only an actual gift consumes E. A full satchel must not lock both the
   * shop doorway and an unrelated handoff at this same street address. */
  if(nv.phone[NP_POPSTAMP]!=stamp&&np_get(slot,99)<99){
   nv.phone[NP_CARD]=0;nv.phone[NP_POPSTAMP]=stamp;nv.phone[slot]=np_get(slot,99)+1;
   tell(fair==2?"Lantern Exchange / Free House-grade Prism Fern lot collected.":"Ferry Night Garden / Free House-grade Halo Moss lot collected.");request_save();return 1;
  }
 }
 if(np_job()<0)return 0;
 int j=np_job(),v=np_job_dest();if(d2(p.x-loc[v].x,p.y-loc[v].y)>43.f*43.f)return 0;
 const NPBrief*b=&np_brief[j];
 if(!j&&!np_get(NP_SAMPLE,1)){nv.phone[NP_SAMPLE]=1;if(np_tracking_slot==sideSlot&&np_tracking==v&&navHasManual&&navManualLoc==v){np_venue(np_job_dest());np_tracking=np_job_dest();}tell("Mara's sealed proof collected. Bring it to Mercury Dispatch. E on foot.");request_save();return 1;}
 if(j&&np_get(NP_BAG+b->id,99)<b->qty){tell("Signal / Not enough stock of that strain and grade. Visit Glasshouse and Stockroom.");return 0;}
 if(np_get(NP_HOLD,600)){tell("Contact delayed. Check Quest log for the remaining time.");return 1;}
 if(j&&street_extra_down(N_ACT_OUT_SIGNAL+j-1)){tell("Contact recovering. The order is still active.");return 1;}
 /* A tense neighborhood can spawn the game's existing patrol response
     and a separate story interruption. Both are single-trigger per request. */
 int attention=night_phone_heat_at(loc[v].x,loc[v].y);
 if(j&&attention>=85&&b->id%3==2&&!np_get(NP_PATROL,1)){
  nv.phone[NP_PATROL]=1;nv.phone[NP_HOLD]=360;heat(24);
  tell("Night Wire / Patrol is checking the corner. Handoff delayed for six seconds.");request_save();return 1;
 }
 if(j&&attention>=65&&!np_get(NP_RIVAL,1)){
  nv.phone[NP_RIVAL]=1;nv.phone[NP_HOLD]=600;tell("Rival crew interrupted the meetup. The contact returns in 10 seconds.");request_save();return 1;}
 if(j)nv.phone[NP_BAG+b->id]=np_get(NP_BAG+b->id,99)-b->qty;
 int area=np_area(loc[v].x,loc[v].y),streak=nv.phone[NP_LASTSALE]==(U)(area+1)?min(5,np_get(NP_STREAK,5)+1):1;
 nv.phone[NP_LASTSALE]=area+1;nv.phone[NP_STREAK]=streak;
 if(j){
  int x=loc[v].x,y=loc[v].y,delta=5+b->id%3*5+streak*2;
  if(!(b->id%3))delta=night_gear_witness_delta(x,y,delta);
  night_phone_heat_add(x,y,delta);
 }
 nv.phone[NP_JOB]=nv.phone[NP_SAMPLE]=nv.phone[NP_HOLD]=nv.phone[NP_RIVAL]=nv.phone[NP_PATROL]=0;nv.phone[NP_DONE]=min(1000000,np_get(NP_DONE,1000000)+1);
 nv.phone[NP_REP]=min(999,np_get(NP_REP,999)+1+b->id%3);np_clear_ours(v);
 gain(!j,b->pay,j?"Night Wire / Matching lot delivered.":"Signal / Proof delivered.");request_save();return 1;
}
static void np_sim_buy(void){
 int n=np_get(NP_SIM,3);if(n>=3){np_msg("Three fictional SIM tokens already held.");return;}
 if(!np_cash(85))return;nv.phone[NP_SIM]=(U)(n+1);
 np_msg("Fictional SIM token acquired. Activate a token below.");np_commit();
}
static void np_sim_activate(void){
 int n=np_get(NP_SIM,3);if(!n){np_msg("No fictional SIM token held. Purchase through this game menu.");return;}
 if(np_get(NP_SIM_COOL,3600)){np_msg("The fictional relay won't accept another token during its arcade cooldown.");return;}
 if(!night_phone_heat_at((int)p.x,(int)p.y)){np_msg("Local attention is already clear. No token spent.");return;}
 nv.phone[NP_SIM]=(U)(n-1);nv.phone[NP_SIM_COOL]=3600;nv.phone[NP_ALIAS]=(U)((np_get(NP_ALIAS,3)+1)&3);
 night_phone_heat_add((int)p.x,(int)p.y,-15);
 np_msg("Signal alias refreshed. Local game attention -15. Active patrols are unaffected.");request_save();
}
static const char *const np_stations[]={"Radio off","Glasswave","Lantern FM","Freightline","District dial"};
static const char *const np_style[]={"Mute the handset and car station.","Glass Mile / analog neon arpeggios.","Lantern Ward / soft midnight lounge.","East Freight / clipped bass and percussion.","Retune as you cross a district border."};
static const char *const np_modules[]={"Lighting upgrade","Hydroponics upgrade","Ventilation upgrade"};
static const char *const np_moddesc[]={"Unlocks Prism Fern and Select grade. Adds one power slot.","Unlocks Echo Bloom and Select grade. Adds one power slot.","Unlocks Top Shelf for all unlocked strains. Adds one slot."};
typedef struct{char label[100],suffix[95],detail[155];} NPLine;
enum{NP_WIRE_WOODLINE=13,NP_WIRE_HARBOR=14,NP_WIRE_ROWS=18};
static int np_rows(void);
static void np_label(NPLine*r,const char*name,const char*small,const char*detail){append(r->label,name);append(r->suffix,small);append(r->detail,detail);}
#include "signal_receipts.inc"
/* Receipts remains on its own M shortcut and does not change the seven-app ring. */
static int np_rows(void){
 static const int counts[]={4,4,NP_WIRE_ROWS,10,9,6,5};
 return np_app==NP_RECEIPTS?RRC_ROWS:counts[cl(np_app,0,NP_NAPP-1)];
}
static const char *const np_meet_name[]={"Switchyard Social","Bluff Lights","Wharf After Dark"};
static const char *const np_booth[]={"Velvet","Mirage","Aurora"};
static int np_card_booth(void){return (np_hour()/4)%LEN(np_booth);}
static const char*np_wthr(void){int w=night_world_weather();return w==NIGHT_FOG?"FOG":w==NIGHT_RAIN?"RAIN":"CLEAR";}
static void np_line(NPLine *l,int r){
 z(l,sizeof *l);if(np_app==NP_RECEIPTS){rrc_line(l,r);return;}int j=np_job(),batch=np_active_batch(),edition=np_sel();char*q;
 switch(np_app){
 case NP_INBOX:{
  int id=np_offer(r);const NPBrief *b=np_brief+id;append(l->label,b->name);fmt(l->suffix,"$",b->pay);append(l->suffix+sl(l->suffix),j==id?"  ACTIVE":!id?"  CLEAN":"  SCRIP");
  q=append(l->detail,b->line);q=append(q,"  ");if(!id)append(q,"Pick up at Ferry Market.");
  else{q=number(q,b->qty);q=append(q," x ");np_print(q,b->id);}
 }break;
 case NP_TRACE:
  if(r==0){if(j<0)np_label(l,"No request tracked","IDLE","Use Inbox to accept a request. Mara always has a prepaid proof, no property necessary.");
   else{np_label(l,np_brief[j].name,"PIN ROUTE","");q=append(l->detail,!j&&!np_get(NP_SAMPLE,1)?"Pick up the proof at ":"Meet the contact at ");q=append(q,loc[np_job_dest()].label);q=append(q,". Press E beside the address on foot.");
    if(np_get(NP_HOLD,600)){q=append(q," Delay: ");q=number(q,(np_get(NP_HOLD,600)+59)/60);append(q,"s.");}}}
  else if(r==1){if(j<0)np_label(l,"Package manifest","EMPTY","Each request specifies a strain, grade and quantity.");
   else if(!j)np_label(l,"Sealed field proof",np_get(NP_SAMPLE,1)?"COLLECTED":"TO COLLECT","The Ferry Market holds this prepaid proof for you. It is separate from your stock.");
   else{np_print(l->label,np_brief[j].id);q=number(l->suffix,np_get(NP_BAG+np_brief[j].id,99));q=append(q," / ");q=number(q,np_brief[j].qty);append(q," HELD");append(l->detail,"Queue the exact strain and grade in Glasshouse, process it at Lantern, then collect in Stockroom.");}}
  else if(r==2){np_label(l,"Career log","","Completed requests and contact reputation across the entire save slot.");q=number(l->suffix,np_get(NP_DONE,1000000));q=append(q," DONE / ");q=number(q,np_get(NP_REP,999));append(q," REP");}
  else np_label(l,np_confirm?"Confirm cancel":"Cancel request",j<0?"N/A":"OPTION","Canceling releases the customer. Your regular satchel stock stays yours.");
  break;
 case NP_WIRE:{
  int x,y,meet=night_garage_meet_site(&x,&y),fair=np_fair_selector();
  if(r==0)np_label(l,"@silkcurtain / Live set",np_live()?"ON STAGE":"20:00","The Afterimage headlines nightly 20:00 to 04:59. Resident performers play matinees. Pin the door.");
  if(r==1){np_label(l,"@shiftgrid / Meet bulletin",meet<0?"19:00":"MEET OPEN","");
   if(meet>=0&&meet<LEN(np_meet_name)){q=append(l->detail,np_meet_name[meet]);append(q," is live. Bring your build to inspect cars or enter a judged showcase.");}
   else append(l->detail,"Nightly meet opens at 19:00. Select to pin tonight's stop in advance.");}
  if(r==2)np_label(l,"@motordesk / Race invite","RACE DESK","Take an owned ride to Motor Works for a time trial, or join tonight's meet for an invitation to the street race circuit.");
  if(r==3){np_label(l,"@ferrygarden / Pop-up",fair?"LIVE":"SCHEDULE","");if(fair){
     q=append(l->detail,np_fair_name(fair));append(q,np_get(fair==2?NP_BAG+3:NP_BAG,99)==99?
      ": matching satchel slot full. Free one space to collect; the shop door still works.":
      ": RSVP here, then E on foot outside the venue. One free House-grade lot per elapsed day.");}
   else append(l->detail,"Ferry Night Garden: 08:00-17:59. Kiosk Lantern Exchange: 22:00-01:59. Check back for RSVP.");}
  if(r==4)np_label(l,"@curbwire / Street report",np_wthr(),night_world_weather()==NIGHT_RAIN?"Rain bulletin: watch for wet pavement. Pin Breakwater.":night_world_weather()==NIGHT_FOG?"Fog bulletin: reduced visibility by the port. Pin Breakwater.":"No storm bulletin. Clear sightlines reported on the waterfront. Pin Breakwater.");
  if(r==5){np_label(l,"@backpage / District pulse","","Repeated sales, sightings and high-value grades raise local attention. At 65 a rival may delay a handoff; at 85 patrols notice Top Shelf.");q=number(l->suffix,night_phone_heat_at((int)p.x,(int)p.y));append(q," / 100");}
  if(r==6){const NPBrief *offer=np_brief+np_new_order();np_label(l,"@requestline / Customer DM","OPEN MESSAGE","");
   q=append(l->detail,offer->name);q=append(q,": ");q=number(q,offer->qty);q=append(q," x ");q=append(q,np_design[offer->id/3]);q=append(q," / ");q=append(q,np_ed[offer->id%3]);q=append(q,". Pays $");number(q,offer->pay);}
  if(r==7){
   int vx,vy,vtime,on=night_venue_vendor_report(&vx,&vy,&vtime);char stamp[6];city_time_label(vtime,stamp);
   int tracked=navSlot==sideSlot&&nav_cart_tracking();
   np_label(l,"@cartwatch / Night Cart",on?tracked?"LIVE TRACKING":"VENDOR ONLINE":tracked?"WAITING":"CLOSED","");
   q=append(l->detail,on?"Latest report ":"Last report ");q=append(q,stamp);q=append(q," / ");
   q=append(q,district(vx,vy));
   if(!on)q=append(q,". Reopens 19:00");
   append(q,tracked?". Enter stops tracking.":". Enter follows each stop on the atlas.");
  }
  if(r==8)np_label(l,"@roadworks / Freight Parkway",np_closed()?"CLOSED":"OPEN",np_closed()?"Signed parkway closure ahead. Local streets remain open. Pin the approach to see the barriers.":"Parkway open now. Temporary roadworks are posted 06:00-07:59 and 16:00-17:59 daily.");
  if(r==9){int booth=np_card_booth(),need=night_club_signature_demand(booth);np_label(l,"@silkcurtain / VIP order","TOP SHELF","");
   if(need>=0&&need<LEN(np_design)){q=append(l->detail,np_booth[booth]);q=append(q," booth requests ");q=append(q,np_design[need]);append(q," / Top Shelf A. Pin The Afterimage management.");}
   else append(l->detail,"Stage booths accept exact Top Shelf strains after you purchase The Afterimage. Pin the club.");}
  if(r==10)np_label(l,"@porchlight / Take a break","VOLUNTARY","Apartment Four has a Bunk and fountain reset for the game's temporary effects and familiarity meter. Pin your front door.");
  if(r==11)np_label(l,"@groundwire / Garden note","ARCADE","Cabinet power is a game stat. Low Draw takes longer and uses less power; Showpiece needs spare slots and produces more attention.");
  if(r>=NP_WIRE_HARBOR&&r<NP_WIRE_ROWS)harbor_jobs_card(r-NP_WIRE_HARBOR,l->label,l->suffix,l->detail);
  if(r==NP_WIRE_WOODLINE){np_label(l,"@woodline / Ranger bulletin","","Walk south from Seawall Road to the Ranger for trail surveys, specimen quests and chests. Press Enter to pin the desk.");append(number(l->suffix,woodline_post_status())," / 3 FILED");}
  if(r==12)np_label(l,"@belllane / Trap House",night_trap_owned()?"OWNED":"$480 CLEAN","Bell Lane Annex, southwest of Apartment Four. Separate property with a graded stockroom, construction and repair dispatch. Select to pin.");
 }break;
 case NP_LOFT:
  if(r==0){np_label(l,"Lantern Grow Loft",sv.b[0].own?"OWNED":"$3000","Lease the existing Lantern Grow Loft. Owners of the original property already have the grow cabinet.");}
  else if(r==1){np_label(l,"Strain",np_design[edition/3],"Cycle strains. Lighting unlocks Prism Fern. Hydroponics unlocks Echo Bloom.");}
  else if(r==2){np_label(l,"Quality grade",np_ed[edition%3],"Select a grade: C House, B Select, A Top Shelf. Higher grades use more arcade power.");}
  else if(r==3){np_label(l,"Queue cabinet","","Start a fictional cannabis lot. Cabinet time only passes outside Signal.");int mode=np_profile_mode();q=number(l->suffix,np_profile_power(edition,mode));q=append(q,"/");q=number(q,np_capacity());q=append(q," POWER  $");number(q,np_cost(edition,mode));}
  else if(r==4){np_label(l,"Process and shelve",batch<0?"IDLE":!np_active_left()?"READY":"","");if(batch>=0){np_print(l->detail,batch);q=append(l->detail+sl(l->detail),". Process the completed lot at the loft on foot.");if(np_active_left()){q=number(l->suffix,(np_active_left()+59)/60);append(q,"s LEFT");}}
      else append(l->detail,"No active lot. Queue one above.");}
  else if(r<=7){np_label(l,np_modules[r-5],"",np_moddesc[r-5]);if(np_get(NP_MODULES,7)&(1<<(r-5)))append(l->suffix,"FITTED");else fmt(l->suffix,"$",np_modcost(r-5));}
  else if(r==8)np_label(l,"Pin Lantern Grow Loft","ATLAS","Package and collect on foot at Lantern Ward, either inside the shop or outside its door.");
  else np_label(l,"Arcade power profile",np_profile[np_profile_mode()],"Enter to cycle Balanced, Low Draw and Showpiece. Changes apply to the next lot, never to an active lot.");
  break;
 case NP_STOCK:
  np_print(l->label,r);q=number(l->suffix,np_get(NP_SHELF+r,99));q=append(q," SHELF  ");q=number(q,np_get(NP_BAG+r,99));append(q," BAG");
  append(l->detail,r%3==2?"Collect up to three Top Shelf units at Lantern. Afterimage booths request exact strains.":"Collect up to three matching units at Lantern. Customers accept only the requested strain and grade.");break;
 case NP_RADIO:if(r<5){np_label(l,np_stations[r],np_get(NP_STATION,4)==r?"SELECTED":"TUNE",np_style[r]);}
  else np_label(l,"YouTube Music",np_confirm?"CONFIRM OPEN":"OPEN BROWSER","Official https://music.youtube.com/. Selecting twice hands off to your default browser. No in-game streaming or login.");
  break;
 case NP_PREF:
  if(!r)np_label(l,"Phone key",np_get(NP_BIND,1)?"F7":"P","Select to switch between P and F7. The preferred binding is saved for this career.");
  else if(r==1)np_label(l,"Device / Signal 09","OFFLINE","Local save-slot data only. No online accounts. External music always opens separately.");
  else if(r==2){np_label(l,"Buy burner SIM token","$85 GAME CASH","This is an offline fictional inventory item, not a real SIM card or service.");}
  else if(r==3){np_label(l,"Activate burner SIM","","Spends one fictional token, rotates the Signal alias and subtracts 15 district attention. Does not clear an active patrol.");
   q=number(l->suffix,np_get(NP_SIM,3));q=append(q," HELD");if(np_get(NP_SIM_COOL,3600)){q=append(q," / ");q=number(q,(np_get(NP_SIM_COOL,3600)+59)/60);append(q,"s");}}
  else np_label(l,"Shade Supply","PIN SHOP","Pin the signed west porch of Saltmarsh Cinema in Glass Mile. Fictional gear and wearable cosmetics.");

 }
}
static void np_apply(void){int r=np_row,j=np_job();
 if(np_app==NP_RECEIPTS){rrc_apply();return;}
 if(np_app==NP_INBOX){np_accept(np_offer(r));return;}
 if(np_app==NP_TRACE){if(r==0)np_track();else if(r==3&&j>=0){if(!np_confirm){np_confirm=1;np_msg("Press Enter again to cancel. Any direction backs out.");return;}
    np_clear_ours(np_job_dest());nv.phone[NP_JOB]=nv.phone[NP_SAMPLE]=nv.phone[NP_HOLD]=nv.phone[NP_RIVAL]=nv.phone[NP_PATROL]=0;np_confirm=0;np_msg("Request canceled. Your satchel was not touched.");request_save();}
   else if(r==1&&j>0){np_app=NP_STOCK;np_row=np_brief[j].id;np_msg("Showing the exact strain and grade on your manifest.");}return;}
 if(np_app==NP_WIRE){
  if(r==0||r==9){int b=night_club_listing();if(b<0||b>=nb||!ordinaryDoorY[b]){np_msg("No current club door to pin.");return;}
   np_pin(LEN(loc)+b,ordinaryDoorX[b],ordinaryDoorY[b]);}
  else if(r==1){int x=1280,y=768;night_garage_meet_site(&x,&y);np_pin(-1,x,y);}
  else if(r==2){np_venue(3);}
  else if(r==3){int fair=np_fair_selector();if(!fair){np_msg("No open pop-up. Check the schedule on this post.");return;}
   np_venue(np_fair_location(fair));if(nv.phone[NP_POPSTAMP]==np_stamp()){np_msg("Daily market gift already collected. Venue pinned.");return;}
   nv.phone[NP_CARD]=(U)fair;request_save();np_msg("RSVP saved. Press E beside this venue on foot while the pop-up is open.");return;}
  else if(r==4)np_venue(9);
  else if(r==5){np_msg(np_areas[np_area((int)p.x,(int)p.y)]);return;}
  else if(r==6){np_app=NP_INBOX;np_row=1;np_msg("Customer DM opened. Confirm in Inbox to accept the exact strain and grade.");return;}
  else if(r==7){
   np_tracking=-1;nav_slot();
   if(nav_cart_tracking()){nav_clear();np_msg("Night Cart tracking stopped.");}
   else{nav_track_cart();np_msg(navCartOpen?
    "Tracking the Night Cart. M opens the regional atlas after returning to the street.":
    "Tracking the Night Cart. Atlas shows its last stop; the route resumes at 19:00.");}
   return;
  }
  else if(r==8)np_pin(-1,NP_PARKWAY_X-66,NP_PARKWAY_Y);
  else if(r==10)np_venue(12);
  else if(r==11){np_app=NP_LOFT;np_row=9;np_msg("Glasshouse profile opened.");return;}
  else if(r>=NP_WIRE_HARBOR&&r<NP_WIRE_ROWS){harbor_jobs_phone(r-NP_WIRE_HARBOR);return;}
  else if(r==NP_WIRE_WOODLINE){woodline_pin_ranger();np_msg("Ranger desk pinned. Follow the trail south of Seawall Road on foot.");return;}
  else if(r==12){np_pin(NAV_TRAP,NT_DOOR_X,NT_DOOR_Y);np_msg("Trap House pinned. Enter at the marked Bell Lane side door on foot.");return;}
  np_msg("Night Wire location pinned on the atlas.");return;
 }
 if(np_app==NP_LOFT){
  if(r==0){if(sv.b[0].own){np_venue(4);np_msg("Already yours. Lantern Grow Loft pinned.");return;}
   if(!np_confirm){np_confirm=1;np_msg("Lease Lantern for $3000? Enter again confirms. Any arrow cancels.");return;}
   np_confirm=0;if(!np_cash(facilityPrice[0]))return;sv.b[0]=(Business){1,0,1,0,0,0,0};np_msg("Lantern is yours. Stockroom and the legacy grow room are unlocked.");np_commit();}
  else if(r==1||r==2){int k=r==1?NP_NEWTYPE:NP_NEWGRADE;nv.phone[k]=(np_get(k,2)+1)%3;np_msg(np_unlocked(np_sel())?"Strain and grade selected.":"Selection locked. Check the upgrade menu.");request_save();}
  else if(r==3)np_sow();else if(r==4)np_bind();else if(r<=7)np_module(r-5);else if(r==8){np_venue(4);np_msg("Lantern Grow Loft pinned.");}
  else{nv.phone[NP_PROFILE]=(U)((np_profile_mode()+1)%3);np_msg("Arcade power profile set for your next batch.");request_save();}return;
 }
 if(np_app==NP_STOCK){np_xfer(r);return;}
 if(np_app==NP_RADIO){if(r<5){nv.phone[NP_STATION]=r;np_msg(r?"Station selected.":"Radio switched off.");request_save();}else if(!np_confirm){np_confirm=1;np_msg("Open official YouTube Music in your default browser? Press Enter again. No in-game streaming.");}
  else{np_confirm=0;np_link_pending=1;np_msg("Requesting your default browser.");}return;}
 if(np_app==NP_PREF){if(!r){nv.phone[NP_BIND]=!np_get(NP_BIND,1);np_msg(nv.phone[NP_BIND]?"F7 now opens and closes Signal.":"P now opens and closes Signal.");request_save();}
  else if(r==2)np_sim_buy();else if(r==3)np_sim_activate();
  else if(r==4){night_gear_pin();np_msg("Shade Supply pinned. Press Enter at the signed west porch.");}}

}
static void night_phone_open(void){
 if(scene==NIGHT_PHONE_SCENE)return;np_link_pending=0;np_return=scene;scene=NIGHT_PHONE_SCENE;np_app=np_job()>=0?NP_TRACE:NP_INBOX;np_row=np_confirm=0;np_note[0]=0;
}
static void night_phone_tick(U keys,U tap){(void)keys;
 if(tap&ESC||night_phone_toggle(tap)){scene=np_return;np_confirm=np_link_pending=0;return;}
 if(tap&MAP){if(np_app==NP_RECEIPTS)rrc_back();else rrc_open();return;}
 if(np_app==NP_RECEIPTS&&(tap&(LEFT|RIGHT))){rrc_back();return;}
 if(tap&(LEFT|RIGHT)){np_app=(np_app+NP_NAPP+((tap&RIGHT)?1:-1))%NP_NAPP;np_row=np_confirm=0;np_note[0]=0;return;}
 int n=np_rows();if(tap&(UP|DOWN)){np_row=(np_row+n+((tap&DOWN)?1:-1))%n;np_confirm=0;np_note[0]=0;return;}
 if(tap&(USE|ENTER))np_apply();
}
/* The landscape handset remains fully software-rendered and legible at 960x540. */
static void np_round(int x,int y,int w,int h,int radius,U c){
 rect(x+radius,y,w-radius*2,h,c);rect(x,y+radius,w,h-radius*2,c);
 disc(x+radius,y+radius,radius,c);disc(x+w-1-radius,y+radius,radius,c);
 disc(x+radius,y+h-1-radius,radius,c);disc(x+w-1-radius,y+h-1-radius,radius,c);
}
static void np_words(int x,int y,int limit,int maxlines,const char*s,U col){
 char out[160];int len=0,words=0;
 while(*s&&maxlines>0){while(*s==' ')s++;if(!*s)break;
  int n=0;while(s[n]&&s[n]!=' ')n++;
  if(len+n+2>=((int)sizeof out))break;int start=len;if(words)out[len++]=' ';
  for(int i=0;i<n;i++)out[len++]=s[i];out[len]=0;
  if(textw(out,0)>limit&&words){out[start]=0;textat(x,y,out,col,0);len=words=0;y+=19;maxlines--;continue;}
  words++;s+=n;
 }
 if(words&&maxlines)textat(x,y,out,col,0);
}
static void np_glyph(int i,int x,int y,U c){
 switch(i){
 case 0:box(x-7,y-5,15,11,0x152932,c);line(x-7,y-5,x,y+1,c);line(x,y+1,x+6,y-5,c);break;
 case 1:line(x-4,y-6,x-4,y+6,c);line(x-4,y-6,x+5,y-6,c);line(x-4,y+2,x+5,y-6,c);disc(x-4,y+6,2,c);break;
 case 2:for(int j=0;j<3;j++){line(x-6,y-5+j*5,x+5-j*2,y-5+j*5,c);dot(x-9,y-5+j*5,c);}break;
 case 3:diamond(x,y-2,6,c);line(x,y-6,x,y+6,c);line(x-4,y+6,x+4,y+6,c);break;
 case 4:for(int j=0;j<3;j++){line(x-7,y-3+j*3,x,y+j*3,c);line(x,y+j*3,x+7,y-3+j*3,c);}break;
 case 5:for(int j=0;j<5;j++){int v=2+(j*5%7);rect(x-7+j*3,y-v,2,2*v+1,c);}break;
 default:line(x-7,y-4,x+7,y-4,c);line(x-7,y+4,x+7,y+4,c);disc(x-2,y-4,2,c);disc(x+4,y+4,2,c);break;
 }
}
static int np_first(void){return cl(np_row-2,0,max(0,np_rows()-5));}
static void night_phone_pointer(int sx,int sy,U clicks){
 if(scene!=NIGHT_PHONE_SCENE||!(clicks&1))return;
 if(sx>=77&&sx<289&&sy>=144&&sy<144+7*38){np_app=cl((sy-144)/38,0,6);np_row=np_confirm=0;np_note[0]=0;}
 else if(sx>=RR_SIGNAL_X&&sx<RR_SIGNAL_X+RR_SIGNAL_W&&sy>=RR_SIGNAL_Y&&sy<RR_SIGNAL_Y+RR_SIGNAL_H)rrc_open();
 else if(sx>=326&&sx<855&&sy>=170&&sy<330){int r=np_first()+(sy-170)/32;if(r>=np_rows())return;if(r==np_row)np_apply();else{np_row=r;np_confirm=0;np_note[0]=0;}}
 else if(sx>=727&&sy>=445&&sx<=874&&sy<=475){scene=np_return;np_confirm=np_link_pending=0;}
}
static void night_phone_paint(void){
 /* Wallpaper: a quiet night skyline beyond the glass, with a fine cartographic grid. */
 for(int y=0;y<H;y+=6)rect(0,y,W,6,mix(0x14212e,0x090c16,y*8/H));
 for(int i=0;i<31;i++){int x=(i*113+17)%W,y=(i*71+11)%H;disc(x,y,i%3?1:2,0x40505a);}
 for(int i=0;i<46;i++){int x=i*24,h=16+(i*41%90);rect(x,H-h,19,h,0x0a101c);rect(x+3,H-h+5,2,4,0x39404e);}
 np_round(49,51,873,444,24,0x090b12);np_round(43,44,874,444,24,0x697a84);
 np_round(45,46,870,440,22,0x293642);np_round(50,51,860,430,19,0x070c14);np_round(57,58,846,416,15,0x10202a);
 rect(61,83,838,1,0x303f49);textat(80,60,"PORT VANTA   /   MUNICIPAL BAND",0x8da9ae,0);
 char clock[6];day_clock_label(clock);textat(776,60,clock,0xc3dbd8,0);for(int i=0;i<4;i++)rect(837+i*5,75-i*3,3,3+i*3,0x86d9be);box(865,68,16,8,0x10202a,0x8eb9b6);rect(867,70,10,4,0x8eb9b6);rect(881,70,2,4,0x8eb9b6);
 rect(70,90,228,346,0x132530);rect(298,90,1,347,0x30454b);
 textat(81,91,"Signal",0xe3f6ed,1);static const char *const np_alias_label[]={"// 09","// WAVE","// MOSS","// ECHO"};textat(206,107,np_alias_label[np_get(NP_ALIAS,3)],0x71b6ac,0);
 rect(81,134,196,1,0x344951);
 for(int i=0;i<NP_NAPP;i++){
  int y=144+i*38,on=i==np_app;U c=np_color[i];if(on){np_round(76,y,211,33,5,0x243e45);rect(76,y+4,3,25,c);}
  np_glyph(i,97,y+15,on?c:0x698c96);textat(118,y+6,np_apps[i],on?PAPER:0x9fb6bd,0);
  if(on)textat(265,y+6,">",c,0);
 }
 int receipts=np_app==NP_RECEIPTS,rx=RR_SIGNAL_X,ry=RR_SIGNAL_Y;
 U a=receipts?rrc_ink:np_color[np_app];
 np_round(rx,ry,RR_SIGNAL_W,RR_SIGNAL_H,5,receipts?0x3b3434:0x1a3037);
 if(receipts)rect(rx,ry+3,3,24,rrc_ink);
 box(rx+15,ry+6,12,17,0x1c2b31,rrc_ink);
 for(int j=0;j<3;j++)line(rx+18,ry+10+j*4,rx+24,ry+10+j*4,rrc_ink);
 textat(rx+42,ry+4,"Revenue receipts",receipts?PAPER:0xc8a99a,0);
 textat(rx+181,ry+4,"M",rrc_ink,0);
 textat(326,93,receipts?"Revenue receipts":np_apps[np_app],PAPER,1);
 textat(328,140,receipts?"Municipal account / latest wallet changes":np_sub[np_app],a,0);
 char pg[48];number(append(number(pg,np_row+1)," / "),np_rows());textat(862-textw(pg,0),141,pg,0x91a6ac,0);
 rect(325,162,538,1,0x395058);int first=np_first();for(int j=0;j<5&&first+j<np_rows();j++){
  NPLine l;np_line(&l,first+j);int y=170+j*32,hi=first+j==np_row;np_round(325,y,538,29,3,hi?0x274047:j%2?0x162932:0x192d36);
  if(hi)rect(325,y+3,3,23,a);textat(338,y+4,l.label,hi?PAPER:0xc7d6d8,0);textat(850-textw(l.suffix,0),y+4,l.suffix,hi?a:0x90a9b1,0);
 }
 if(np_rows()>5){rect(868,170,2,157,0x344951);int hh=157*5/np_rows();int yy=(157-hh)*first/(np_rows()-5);rect(868,170+yy,2,hh,a);}
 NPLine l;np_line(&l,np_row);rect(326,338,536,1,0x3b4a51);textat(327,342,np_note[0]?"SIGNAL RESPONSE":"FIELD NOTE",a,0);np_words(327,363,527,3,np_note[0]?np_note:l.detail,np_note[0]?0xffdfae:0xc4d5d8);
 rect(73,442,810,1,0x31444c);textat(81,450,"< >  APPS      UP / DOWN  SELECT      ENTER  ACTION",0x9cb2bb,0);
 char exitText[32]="";append(exitText,np_get(NP_BIND,1)?"F7":"P");append(exitText+sl(exitText)," / ESC   CLOSE");textat(865-textw(exitText,0),450,exitText,a,0);
}
