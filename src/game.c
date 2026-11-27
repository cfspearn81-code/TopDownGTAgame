#include "game.h"
#include "font.inc"
#include "verdict_api.inc"
#include "npc_api.inc"
#ifdef DC_TEST
#include <stdio.h>
#include <stdlib.h>
#endif
#define RGB(r,g,b) (((r)<<16)|((g)<<8)|(b))
#define DARK 0x0a151c
#define PAPER 0xf7f1db
#define MUTED 0xa5bdc3
#define TEAL 0x7bead8
#define LIME 0xdaff7a
#define PINK 0xff7b9c
#define GOLD 0xffcf77
#define BASE_ROOM 2048
#define ROOM 3072
#define NSTATE 18
#define LEN(a) ((int)(sizeof(a)/sizeof(*(a))))
U pixels[W*H];
static U *surface=pixels; static int rw=W,rh=H;
static int clipLeft,clipTop,clipRight=W,clipBottom=H;
static unsigned char roofMask[ROOM*ROOM];
static U city[ROOM*ROOM],rng=0x873f24c7u;
static U rnd(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static int min(int a,int b){return a<b?a:b;} static int max(int a,int b){return a>b?a:b;} static int cl(int a,int l,int h){return min(h,max(l,a));}
static float absf(float f){return f<0?-f:f;} static float d2(float a,float b){return a*a+b*b;}
static float sq(float x){if(x<=0)return 0;float r;__asm__("sqrtss %1,%0":"=x"(r):"x"(x));return r;}
static float wrap(float a){
 const float turn=6.283186f, halfTurn=turn*.5f;
 if(a>=turn||a<=-turn)a-=(int)(a/turn)*turn;
 if(a>halfTurn)a-=turn;
 if(a<-halfTurn)a+=turn;
 return a;
}
static float sn(float a){a=wrap(a);if(a>1.570796f)a=3.141593f-a;if(a<-1.570796f)a=-3.141593f-a;float z=a*a;return a*(1-z*(0.16666667f-z*(0.008333333f-z*0.00019841f)));}
static float cs(float a){return sn(a+1.570796f);}
static float look(float x,float y){if(absf(x)<.001f)return y>0?1.570796f:-1.570796f;float t=y/x,a;if(absf(t)<=1)a=t*(.785398f+.273f*(1-absf(t)));else{float u=1/t;a=(t>0?1.570796f:-1.570796f)-u*(.785398f+.273f*(1-absf(u)));}return x<0?wrap(a+3.141593f):a;}
static int sl(const char*s){int i=0;while(s[i])i++;return i;}
static void cp(void*d,const void*s,unsigned n){volatile unsigned char *a=d;const unsigned char*b=s;while(n--)*a++=*b++;}
static void z(void*d,unsigned n){volatile unsigned char*a=d;while(n--)*a++=0;}
static char *append(char*d,const char*s){while(*s)*d++=*s++;*d=0;return d;}
static char *number(char*d,int n){if(n<0){*d++='-';n=-n;}char a[12];int i=0;do{a[i++]=(char)('0'+n%10);n/=10;}while(n);while(i)*d++=a[--i];*d=0;return d;}
static void rect(int x,int y,int w,int h,U c){int x1=min(min(rw,clipRight),x+w),y1=min(min(rh,clipBottom),y+h);for(int j=max(clipTop,y);j<y1;j++){U*p=surface+j*rw;for(int i=max(clipLeft,x);i<x1;i++)p[i]=c;}}
static void dot(int x,int y,U c){if(x>=clipLeft&&y>=clipTop&&x<clipRight&&y<clipBottom&&(U)x<(U)rw&&(U)y<(U)rh)surface[y*rw+x]=c;}
static U mix(U a,U b,int k){int m=8-k;return RGB((((a>>16)&255)*m+((b>>16)&255)*k)>>3,(((a>>8)&255)*m+((b>>8)&255)*k)>>3,((a&255)*m+(b&255)*k)>>3);}
static void box(int x,int y,int w,int h,U bg,U border){rect(x,y,w,h,border);rect(x+1,y+1,w-2,h-2,bg);}
static void disc(int x,int y,int rad,U c){for(int j=-rad;j<=rad;j++)for(int i=-rad;i<=rad;i++)if(i*i+j*j<=rad*rad)dot(x+i,y+j,c);}
static void line(int x,int y,int xx,int yy,U c){int dx=max(xx-x,x-xx),dy=-max(yy-y,y-yy),sx=x<xx?1:-1,sy=y<yy?1:-1,err=dx+dy;for(;;){dot(x,y,c);if(x==xx&&y==yy)break;int e=2*err;if(e>=dy){err+=dy;x+=sx;}if(e<=dx){err+=dx;y+=sy;}}}
static void tri(int ax,int ay,int bx,int by,int cx,int cy,U c){int area=(bx-ax)*(cy-ay)-(by-ay)*(cx-ax);if(!area)return;int m=area>0?1:-1;for(int y=max(0,min(ay,min(by,cy)));y<=min(rh-1,max(ay,max(by,cy)));y++)for(int x=max(0,min(ax,min(bx,cx)));x<=min(rw-1,max(ax,max(bx,cx)));x++){int q=(bx-ax)*(y-ay)-(by-ay)*(x-ax),r=(cx-bx)*(y-by)-(cy-by)*(x-bx),s=(ax-cx)*(y-cy)-(ay-cy)*(x-cx);if(q*m>=0&&r*m>=0&&s*m>=0)dot(x,y,c);}}
static int textw(const char*s,int big){int n=0;for(;*s;s++){int k=(unsigned char)*s-32;if(k>=0&&k<95)n+=big?fw1[k]:fw0[k];}return n;}
static void textat(int x,int y,const char*s,U c,int big){int w=big?35:10,h=big?40:20;const unsigned char*f=big?font1:font0;const unsigned char*fw=big?fw1:fw0;for(;*s;s++){int k=(unsigned char)*s-32;if(k<0||k>=95)continue;if(x+w>=0&&x<rw)for(int j=0;j<h;j++)for(int i=0;i<w;i++){int a=f[(k*h+j)*w+i];if(a&&x+i>=clipLeft&&y+j>=clipTop&&x+i<clipRight&&y+j<clipBottom&&(U)(x+i)<(U)rw&&(U)(y+j)<(U)rh){U*p=surface+(y+j)*rw+x+i;*p=a==7?c:mix(*p,c,a);}}x+=fw[k];}}
static void center(int x,int y,const char*s,U c,int big){textat(x-textw(s,big)/2,y,s,c,big);}
static void wrapped(int x,int y,const char*s,U c,int limit){char a[94];while(*s){int n=0,cut=0;while(s[n]&&n<limit){if(s[n]==' ')cut=n;n++;}if(s[n]&&cut)n=cut;for(int j=0;j<n;j++)a[j]=s[j];a[n]=0;textat(x,y,a,c,0);s+=n;while(*s==' ')s++;y+=21;}}

typedef struct{int x,y,w,h,ht;U roof,wall;}Building;
static Building buildings[140];static int nb,lot[64][3];
typedef struct{int x,y,w,h;} StreetBounds;
/* Each block holds its building silhouettes followed by its tree trunks. */
typedef struct{StreetBounds walls[10];int count,treesFrom;} BlockScenery;
static BlockScenery scenery[64];
static void setup_scenery(void);
static void atm_place(void);
static void bank_ui_reset(void);
static int ordinaryDoorX[LEN(buildings)],ordinaryDoorY[LEN(buildings)];
enum { NIGHT_CLUB_LEGACY=66 };
static int ncFrontId=-1;
static int night_club_listing(void){return ncFrontId>=0?ncFrontId:NIGHT_CLUB_LEGACY;}
static void night_club_front_lot(void);
static void setup_ordinary_doors(void);
static int zone_is_building(int i);
static const char *walkin_name(int i);
static int tram_hall(int i),tram_destination(int i),a1_can_ride(void);
static const char *tram_block(void);
static const char *venue_address(int b);
static const char *hud_line(const char *s,int limit,char out[130]);
static void walkin_open(int building);
static int contract_room(int kind,int x,int y,int exclude);
static void walkin_tick(U keys,U tap);
static void walkin_paint(void);
static void bake_scenery(void);
static void zone_venues(void);
static void navigation_input(U tap);
static void navigation_follow_job(void);
static void navigation_pointer(int sx,int sy,U clicks);
static void open_interior(int loc_index);
static void interior_tick(U keys,U tap);
static int lantern_terminal(void);
static void interior_paint(void);
typedef struct{int x,y;const char *label,*npc,*note;U tint;int kind;} Place;
static const Place originalLoc[]={
{256,512,"MERCURY DISPATCH","Mara Finch","Legal work: delivery routes and taxi fares.",TEAL,0},
{512,768,"VAULT 39","Ivo Slate","Nightclub and bar. Private meetings are in the back.",PINK,1},
{768,512,"FERRY MARKET","June Vale","Supplies, provisions and recovery items.",GOLD,2},
{1024,768,"NIGHTGLASS MOTORS","Lex Rook","Buy, repair and switch between owned rides.",TEAL,3},
{512,1280,"LANTERN GROW LOFT","Nell Ash","The starter grow property. Turn credits into game-only batches.",LIME,4},
{1280,1280,"CINDER WORKS","Dr. Kite","A larger lab for abstract synthetic production.",LIME,5},
{1024,256,"GLASS MILE CIRCUIT","Paz Ibarra","Sanctioned sprint, no heat and real prize money.",TEAL,6},
{1536,768,"KIOSK 09","Vera Voss","Sell stock here. Credit exchange is at owned storefronts.",PINK,7},
{256,1536,"TIDEWAY CLINIC","Sol Mercer","Recover health and pick up armor.",GOLD,8},
{1536,1536,"BREAKWATER SAFEHOUSE","Reed Sato","Rest here to recover health and lose police pursuit.",TEAL,9},
{1792,1024,"WRECKER YARD","Dax Morro","Licensed recovery jobs and salvage runs.",TEAL,10},
{768,256,"REVENUE ANNEX","Officer Holt","Pay a civil penalty or take civilian restitution shifts.",GOLD,11},
{256,666,"YOUR APARTMENT","Apartment 4","A cramped walk-up off Tide Market.",TEAL,13},
{768,768,"VANTA ARMORY","Ada Flint","Purchase a pistol, armor and upgrades at the counter.",GOLD,14},
{1280,512,"VANTA POLICE HQ","Desk Officer","Headquarters and patrol dispatch.",0x8aabff,15}
};
static Place loc[LEN(originalLoc)];
static const char* district(int x,int y){if(x>=BASE_ROOM-36||y>=BASE_ROOM-36){return y<820?"GLASS BLUFF":y<1130?"FREIGHT PARKWAY":y<1570?"BELT WORKSHOPS":x>=2180?"HARBOR PIER":"SALT REACH";}if(y<545)return "GLASS MILE";if(y>1160)return x<900?"LANTERN WARD":"BREAKWATER";return x<850?"TIDE MARKET":x<1400?"NIGHTGLASS":"EAST FREIGHT";}
static const char*venue_address(int b){return b==ncFrontId?"BASE VILLAGE":district(ordinaryDoorX[b],ordinaryDoorY[b]);}
/* Forward caps use each model's former 10/100-condition speed at full
 * health. Keep throttle response and low-speed control unchanged. */
/* Human locomotion follows one pace, across named/ordinary interiors,
 * outdoor actors, and taxi boarding. Their other clocks stay real-time. */
#define FOOT_PACE .8f
#define FOOT_WALK_SPEED (123.f*FOOT_PACE)
#define FOOT_SPRINT_SPEED (183.f*FOOT_PACE)
#define NAMED_WALK_STEP (2.42f*FOOT_PACE)
#define NAMED_RUN_STEP (3.55f*FOOT_PACE)
#define WALKIN_WALK_STEP (2.5f*FOOT_PACE)
#define WALKIN_RUN_STEP (3.6f*FOOT_PACE)
typedef struct{const char*name;int price;float vmax,reverse,acc,turn;U c;} CarSpec;
static const CarSpec specs[]={
 {"MOTE",125,276.f*.55f,61,242,2.8f,0x59bbad},
 {"VECTRA",360,328.f*.55f,75,275,2.3f,0x94b2f5},
 {"PALLAS",620,300.f*.55f,66,239,1.95f,0xeab052},
 {"NIGHTJAR",970,373.f*.55f,90,315,2.7f,0xdf648f},
 {"COURIER 4",480,286.f*.55f,69,230,2.55f,0xbbd377},
};
typedef struct{float x,y,a,s;int type,owner,cond,reported,wear;}Car;
static void outer_take_wheel(int);
static void outer_life_reset(void),outer_life_tick(float),outer_life_paint(void);
typedef struct{float x,y,a,s;int hp,role,next;float life,cool;}Ped;
/* Two temporary customer slots can follow the unchanged stock actor count.
 * Ordinary jobs still get their stock final slot when no listing is active. */
enum{STREET_CITY_CAR_LIMIT=44,OUT_TRAFFIC_COUNT=7};
static Car cars[STREET_CITY_CAR_LIMIT+OUT_TRAFFIC_COUNT];static int nc;
#if __has_include("verdict_driving.inc")
static float vd_chess_rail_depth(const Car*);
static void vd_shadow_sample(U keys),vd_shadow_gate(int step);
#else
#define vd_chess_rail_depth(c) 0.f
#define vd_shadow_sample(keys) ((void)0)
#define vd_shadow_gate(step) ((void)0)
#endif
static Ped peds[52];
typedef struct{float x,y,vx,vy,t;U c;} Particle;
static Particle fx[100];static int fn;
typedef struct{int own,crew,tier,product,durationMs,batches;float timer;} Business;
typedef struct{int owned;float elapsed,cooldown;int pending;} LegalBusiness;
#include "character.inc"
typedef struct{U magic,rev;int cash,hp,armor,gear,units,owned,selection,legal,crime,sales,arrests,vehicleCond,home;float x,y;Business b[2];int stocks[3],dirty,pending,audit,charges,restitution,legacyRivalCooldown,protect;float clearT,auditT;U intro;int carCond[5];U taxiFares;LegalBusiness legit[3];float dayClock;U playedTicks;Look look;unsigned char legacyClock,medPatches;char name[20];int bank,dirtySpent,sweepTicks;U hash;} Save;
enum{SAVE_REV=10,SAVE_REV_PATCHES=11,MAX_MED_PATCHES=99,DAY_SECONDS=24*60,LEGACY_DAY_SECONDS=8*60,
 SAVE_COUNTER_LIMIT=1000000000,REVENUE_DIRTY_LIMIT=500,REVENUE_SWEEP_TICKS=5*60*60,WALLET_LIMIT=2147483647, SAVE_STOCK_LIMIT=500000000};
#define LEGACY8_SIZE (__builtin_offsetof(Save,sweepTicks)+4)
#define LEGACY7_SIZE (__builtin_offsetof(Save,bank)+4)
_Static_assert(sizeof(Save)==304&&__builtin_offsetof(Save,medPatches)==267&&__builtin_offsetof(Save,bank)==288&&__builtin_offsetof(Save,dirtySpent)==292&&__builtin_offsetof(Save,sweepTicks)==296&&__builtin_offsetof(Save,hash)==300,"Unexpected on-disk save layout");
#define LEGACY6_SIZE (__builtin_offsetof(Save,playedTicks)+4)
static Save sv;
/* Native city/contract indices and the outer district keep separate ranges
 * while sharing the exact same player-facing chassis and physics. */
static int street_car_exists(int i){
 return (i>=0&&i<nc&&(i||sv.owned))||
        (i>=STREET_CITY_CAR_LIMIT&&i<LEN(cars)&&cars[i].x>=35);
}
static int street_next_car(int i){
 if(++i<nc){if(i||sv.owned)return i;if(++i<nc)return i;}
 if(i<STREET_CITY_CAR_LIMIT)i=STREET_CITY_CAR_LIMIT;
 while(i<LEN(cars)&&!street_car_exists(i))i++;
 return i;
}

typedef struct { U club[32],garage[64],phone[64],world[24]; } NightSave;
typedef struct { Save base; NightSave night; U hash; } Save11;
enum { NIGHT_SAVE_REV=11, NIGHT_CLUB_SCENE=7, NIGHT_GARAGE_SCENE=8, NIGHT_PHONE_SCENE=9,
       NIGHT_VENUES_SCENE=10,NIGHT_TRAP_SCENE=11 };
enum {NT_DOOR_X=174,NT_DOOR_Y=944};
enum { NIGHT_CLEAR, NIGHT_FOG, NIGHT_RAIN };
_Static_assert(sizeof(NightSave)==736 && sizeof(Save11)==1044 && __builtin_offsetof(Save11,night.world[6])==968,"Unexpected Night District save layout");
static NightSave nv;
#include "vehicle_condition.inc"
static U night_hash(const void *bytes,unsigned n){
 const unsigned char *p=bytes;U h=2166136261u;
 while(n--)h=(h^*p++)*16777619u;
 return h;
}
static int night_club_open(int),night_garage_open(int),night_venues_open(int);
static int ng_power(const Car*),ng_grip(const Car*),ng_route_goal(int*,int*);
static void ch_motion(float,float),ch_project(int*,int*),ch_badge(void),ng_schedule_tick(void);
static int ch_draw(void),ch_pref(void),chProjected;
static void night_club_tick(U,U),night_garage_tick(U,U),night_phone_tick(U,U),night_venues_tick(U,U);
static void night_club_paint(void),night_garage_paint(void),night_phone_paint(void),night_venues_paint(void);
static void night_phone_open(void),night_phone_world_tick(float);
static int night_phone_toggle(U),night_phone_action(U);
static void night_world_reset(void),night_world_tick(float),night_world_bake(void),night_world_street_fx(void),night_world_hud(void);
static float night_world_brake(void),night_world_grip(void),night_world_foot(void);
static int night_world_is_night(void),night_world_weather(void);
static int night_club_track(void),night_phone_radio(void),night_garage_audio(void);
static void night_club_business_tick(void),ng_reset(void),night_garage_street(U),ng_world(void),ng_gate_overlay(void);
static int ncr_active(void),ncr_target(int*,int*),ncr_ped(float*,float*);
static void ncr_reset(void),ncr_street_tick(void),ncr_street_person(int,int);
static const char*ncr_stage_text(void),*ncr_stop_name(void);
static void night_club_pointer(int,int,U),night_phone_pointer(int,int,U);
static int night_phone_premium(int),night_phone_route(int*,int*),night_phone_heat_at(int,int);
static void night_phone_heat_add(int,int,int);
static int np_closed(void),night_phone_lane_stop(float,float,float,float,int),night_phone_take_link(void),night_phone_signature(int,int);
static void night_phone_link_result(int),night_phone_road_paint(void),night_phone_contact_paint(void),night_phone_rest(int);
static int night_club_queue(void),night_club_signature_demand(int),night_club_listing(void);
static int night_trap_owned(void),night_trap_entry(U);
static void night_trap_tick(U,U),night_trap_paint(void),night_trap_world_tick(float),night_trap_street_paint(void),night_trap_hint(void);
static int night_venue_apply_pulse(int*,int*,float*,U*),night_venue_vendor_stop(void);
static void nvk_stats(int*,int*,float*,U*);
static void night_venue_reload(U);
static int night_venue_vendor_pos(int*,int*),night_venue_vendor_report(int*,int*,int*),night_venue_vendor_entry(U);
static int night_garage_meet_site(int*,int*);
static int night_venues_entry(U),night_club_entry(U);
static void night_venues_hint(void),night_venues_world_tick(void),night_venues_fx(void);
static float night_venues_walk_factor(void);
static void night_rest_clock(unsigned,int);
enum{DAY_BADGE_X=16,DAY_BADGE_W=104,DAY_BADGE_H=24,DAY_BADGE_LABEL_X=131};
/* One real minute is one city hour and one real second is a city minute. */
static void city_time_label(int second,char label[6]){
 int hour=second/60,minute=second%60;
 label[0]='0'+hour/10;label[1]='0'+hour%10;label[2]=':';
 label[3]='0'+minute/10;label[4]='0'+minute%10;label[5]=0;
}
static void day_clock_label(char label[6]){city_time_label((int)sv.dayClock,label);}
static void draw_day_clock(int y){
 char label[6];day_clock_label(label);
 box(DAY_BADGE_X,y,DAY_BADGE_W,DAY_BADGE_H,DARK,0x415766);
 int x=DAY_BADGE_X+13,v=y+12;disc(x,v,7,TEAL);disc(x,v,5,DARK);
 line(x,v,x,v-3,PAPER);line(x,v,x+3,v,PAPER);
 center(DAY_BADGE_X+61,y+1,label,PAPER,0);
}
enum{REVENUE_NO_RECEIPT,REVENUE_THRESHOLD,REVENUE_SWEEP};
typedef struct{int kind,clean,dirty,pending;} RevenueReceipt;
static int revenueCrossed;
static RevenueReceipt revenueReceipt;
#include "revenue_receipts.inc"
static RevenueReceipt revenue_seizure_notice(void){
 RevenueReceipt receipt=revenueReceipt;revenueReceipt=(RevenueReceipt){0};return receipt;
}
static void revenue_open_case(void){
 if(sv.charges)return;
 sv.charges=1;sv.restitution=0;sv.sweepTicks=REVENUE_SWEEP_TICKS;
}
static void revenue_close_case(void){
 sv.charges=sv.restitution=sv.sweepTicks=0;
}
typedef struct {const char*name;int cost,time,units,price,site;} Product;
static const Product prod[]={{"NIGHTLEAF",15,120,5,22,0},{"VIOLET HAZE",15,120,5,23,0},{"GLINT",30,120,6,46,1}};
static const int facilityPrice[]={3000,10000};
static int batch_units(int product,int tier){
 static const int outcomes[3][3]={{5,10,17},{5,10,16},{6,11,18}};
 return outcomes[cl(product,0,2)][cl(tier-1,0,2)];
}
static int batch_supply(int property){return property?2:1;}
static int tier_price(int property,int tier){return property?(tier==1?1650:3000):(tier==1?650:1150);}
enum{JOB_COURIER,JOB_TAXI,JOB_RECOVERY,JOB_SPRINT,JOB_VAULT,JOB_HOT,JOB_SIGNAL,JOB_RIVALS};
static const char* jobNames[]={"COURIER SHIFT","TAXI FARE","HARBOR RECOVERY","GLASS MILE SPRINT","VAULT RUN","HOT PROPERTY","SIGNAL RAID","RIVAL COLLECTION"};
typedef struct{int kind,step,total,price;float time;int xs[6],ys[6],unpaid;}Job;
static Job job={.kind=-1};
static int jobRide=-1;
enum{TAXI_WAIT,TAXI_BOARDING,TAXI_RIDING,TAXI_EXITING,TAXI_WALKING};
typedef struct{float x,y,a,still,linger;int phase,ride,pickup,dropoff,rider,curb[2],meters,crashes,damage,rough;float roughHold,roughCooldown,contactSeconds,contactX,contactY;int copper,pedHits;} Taxi;
static Taxi taxi;
enum{CAB_NONE,CAB_OFFER,CAB_DIRECT,CAB_VIA,CAB_SEEN,CAB_RANK_ACTION=151};
static int cabRequested;
static int taxi_authored_offer(int which);
static int taxi_via_copper(void),taxi_copper_x(void),taxi_copper_y(void),taxi_detour_bonus(void);
static const char *taxi_route_name(void);
static void cab_choose_route(U tap),cab_passing_copper(void),cab_marquee_paint(void),cab_rank_counter(void);
static int lastTaxiPickup=-1,lastTaxiDrop=-1,lastTaxiRider=-1;
static U taxiSeed=0xc3b11d19u;
static U taxi_random(U *seed){U n=*seed;n^=n<<13;n^=n>>17;n^=n<<5;return *seed=n;}
static void taxi_reset_radio(void);
static void taxi_start_receipt(void);
static void taxi_draw_radio(void);
void set_session_seed(U seed){taxiSeed=(seed^0xc3b11d19u)|1u;npcRandom=seed^0xa1387f29u;}
static const struct {const char*name;U outfit;int face;} riders[]={
 {"MIA",0xd5a9eb,2},{"ROWAN",0x91e6ce,1},{"JULES",0xff9e92,4},{"ARI",0x9bbdff,0},
 {"SASHA",0xfcd37e,3},{"DEV",0xbcafd9,2},{"IMANI",0xef90c8,1},{"NOAH",0xaad482,4},
 {"ELI",0xe8b58b,0},{"REN",0x8ad7ee,3},{"SAM",0xd5c89f,2},{"LEO",0xe4a082,4}
};
typedef struct{float x,y,a,heat,quiet,hit,shot,secs,noticeT,trail;int car,hp,dead,seen;} Player;
static Player p;
static float trailX,trailY,trailEndX,trailEndY;
static U trailInk=GOLD;
enum{PAUSE_MENU=12};
static int scene=0,menu=-1,sel=0,mapOn=0,wason=0,cx,cy,deadAnim,saveFlash,fastGfx,sideSlot;static float crashFlash;
static float anim,footcycle,punchFlash,pedHit[LEN(peds)],shelterT;static int walking;
static float cameraLeadX,cameraLeadY;
static float ngCamAngle,ngCamRoll,ngCamZoom,ngCamSpeed,ngCamBurst;
static int ngCamRide=-1;
static void ng_cam_reset(void){ngCamAngle=ngCamRoll=ngCamZoom=ngCamSpeed=ngCamBurst=0;ngCamRide=-1;cameraLeadX=cameraLeadY=0;}
static void update_camera_lead(void){
 float vx=0,vy=0,zoom=0,roll=0;
 if(street_car_exists(p.car)){
  const Car*c=cars+p.car;float speed=absf(c->s);
  if(ngCamRide!=p.car){ngCamAngle=c->a;ngCamSpeed=speed;ngCamBurst=0;}
  /* A short angular spring shows the side/rear through the apex, then
   * homes to north-up; the atlas, controls and physics never rotate. */
  float err=wrap(c->a-ngCamAngle);
  ngCamAngle=wrap(ngCamAngle+err*.10f);
  float bend=absf(err);if(bend>1)bend=1;
  float lead=c->s*(1.f-.40f*bend);
  vx=cs(ngCamAngle)*lead*.48f;vy=sn(ngCamAngle)*lead*.34f;
  if(vx>108)vx=108;if(vx<-108)vx=-108;if(vy>76)vy=76;if(vy<-76)vy=-76;
  if(c->s>=0&&speed>15&&speed>ngCamSpeed+.60f)ngCamBurst+=(1-ngCamBurst)*.06f;else ngCamBurst*=.958f;
  ngCamSpeed=speed;ch_motion(speed,err);
  float f=speed/205.f;if(f>1)f=1;
  if(!fastGfx){
   roll=err*(c->s>=0?1.f:-1.f)*.08f*f;
   if(roll>.037f)roll=.037f;if(roll<-.037f)roll=-.037f;
   zoom=.145f-.067f*f-.042f*ngCamBurst+.018f*bend;
  }
 }
 ngCamRide=p.car;ngCamRoll+=(roll-ngCamRoll)*.14f;ngCamZoom+=(zoom-ngCamZoom)*.07f;
 cameraLeadX+=(vx-cameraLeadX)*.055f;cameraLeadY+=(vy-cameraLeadY)*.055f;
}
static float ng_cam_scale(void){
 float zoom=1+ngCamZoom,minz=1.f+absf(ngCamRoll)*1.85f;
 return zoom>minz?zoom:minz;
}
static void ng_cam_point(int*x,int*y){
 if(chProjected){ch_project(x,y);return;}
 if((ngCamZoom<.001f&&absf(ngCamRoll)<.001f)||fastGfx)return;
 float u=*x-W/2,v=*y-H/2,co=cs(ngCamRoll),si=sn(ngCamRoll),zoom=ng_cam_scale();
 *x=W/2+(int)((u*co+v*si)*zoom);*y=H/2+(int)((v*co-u*si)*zoom);
}
static void ng_cam_compose(void){
 if((ngCamZoom<.001f&&absf(ngCamRoll)<.001f)||fastGfx)return;
 static U out[W*H];
 float zoom=ng_cam_scale();
 int a=(int)(65536.f*cs(ngCamRoll)/zoom),b=(int)(65536.f*sn(ngCamRoll)/zoom);
 for(int y=0;y<H;y++){
  int u=(W/2<<16)-(W/2)*a-(y-H/2)*b,v=(H/2<<16)-(W/2)*b+(y-H/2)*a;
  for(int x=0;x<W;x++,u+=a,v+=b)out[y*W+x]=pixels[cl(v>>16,0,H-1)*W+cl(u>>16,0,W-1)];
 }
 for(int i=0;i<W*H;i++)pixels[i]=out[i];
}
static int camera_left(void){return cl((int)(p.x+cameraLeadX)-W/2,0,ROOM-W);}
static int camera_top(void){return cl((int)(p.y+cameraLeadY)-H/2,0,ROOM-H);}
static U prev,wheelSuppressed;
static int wheelOpen,wheelPreviewFists,wheelFists,wheelX=-1,wheelY=-1,wheelFeedback;
static int wheel_tick(U keys,U tap);
static int pistol_equipped(void){return sv.gear>=0&&!wheelFists;}
static int by=-1,near=-1,lastCar=-1,helpBack=1;
#define CASH_GREEN 0x85F36Bu
#define CASH_RED   0xFF535Cu
static char notice[192];
static U noticeInk;
static int storefrontCaption;
static U notice_color(void){return noticeInk?noticeInk:PAPER;}
static char*notice_copy(const char*s,int reserve){int n=0;while(s[n]&&n<(int)sizeof(notice)-reserve-1){notice[n]=s[n];n++;}notice[n]=0;return notice+n;}
static void tell(const char*s){storefrontCaption=0;notice_copy(s,0);p.noticeT=4.4f;noticeInk=0;}
static void wallet_note(const char*s,int a,int clean){
 storefrontCaption=0;char*q=notice_copy(s,32);q=append(q,clean?"  CLEAN +$":"  DIRTY MONEY +$");number(q,a);
 p.noticeT=4.4f;noticeInk=clean?CASH_GREEN:CASH_RED;
}
static void cashNote(const char*s,int a){wallet_note(s,a,1);}
static void fmt(char*d,const char*s,int a){number(append(d,s),a);}
static void draw_wallet(int x,int y,int w){
 char v[24];box(x,y,w,49,0x0d1720,0x415766);
 for(int i=0;i<2;i++){
  int ry=y+3+i*22;U color=i?CASH_RED:CASH_GREEN;
  rect(x+3,ry+3,4,14,color);fmt(v,"$",i?sv.dirty:sv.cash);
  const char *label=i?"DIRTY MONEY":"CLEAN CASH";
  int amountX=x+w-11-textw(v,0);
  if(x+13+textw(label,0)+10>amountX)label=i?"DIRTY":"CLEAN";
  textat(x+13,ry,label,color,0);
  textat(amountX,ry,v,color,0);
 }
}
static int spend(int x){if(sv.cash<x){tell("Not enough credits.");return 0;}sv.cash-=x;rr_checkout(x,x,0);return 1;}
static void ground(int x,int y,int w,int h,U c){for(int j=max(0,y);j<min(ROOM,y+h);j++)for(int i=max(0,x);i<min(ROOM,x+w);i++)city[j*ROOM+i]=c;}
static StreetBounds building_bounds(const Building*b){
 int blockX=b->x/256*256,blockY=b->y/256*256;
 int x=b->x,y=max(blockY+42,b->y-b->ht);
 return (StreetBounds){x,y,min(blockX+214,x+b->w+b->ht/2+1)-x,b->y+b->h+1-y};
}
#include "outer_city.inc"
static int solid(float x,float y,float r){
 if(x<35||x>ROOM-36||y<35||y>ROOM-36)return 1;
 if(outer_is_new(x,y))return !outer_area(x,y,r);
 const BlockScenery*block=&scenery[((int)y/256)*8+(int)x/256];
 for(int i=0;i<block->count;i++){
  const StreetBounds*b=block->walls+i;
  if(x+r>b->x&&x-r<b->x+b->w&&y+r>b->y&&y-r<b->y+b->h)return 1;
 }
 return 0;
}
static int open_space(float x,float y){return !solid(x,y,10);}
static void particle(float x,float y,int n,U c){while(n--){Particle*q=fx+(fn++%LEN(fx));q->x=x;q->y=y;q->vx=(int)(rnd()%151)-75;q->vy=(int)(rnd()%151)-75;q->t=.2f+(rnd()%36)*.01f;q->c=c;}}
static void setupCity(void){
 rng=0x873f24c7u;nb=0;z(lot,sizeof lot);cp(loc,originalLoc,sizeof loc);for(int y=0;y<BASE_ROOM;y++)for(int x=0;x<BASE_ROOM;x++){int xx=x%256,yy=y%256,n=(int)(rnd()%12);U c;if(xx<29||xx>=227||yy<29||yy>=227){c=RGB(24+n,34+n,43+n);if((xx==0&&yy>40&&yy<219&&yy%35<17)||(yy==0&&xx>40&&xx<219&&xx%35<17))c=0x90845a;}else if(xx<42||xx>214||yy<42||yy>214)c=RGB(48+n,55+n,60+n);else c=RGB(32+n,44+n,47+n);if(x>1390&&y>1110)c=mix(c,0x264044,2);else if(y<550)c=mix(c,0x4a4258,2);else if(x<850)c=mix(c,0x54433e,2);if(x<25||y<25||x>BASE_ROOM-26||y>BASE_ROOM-26)c=RGB(11+n/2,31+n,40+n);city[y*ROOM+x]=c;}
 for(int iy=0;iy<8;iy++)for(int ix=0;ix<8;ix++){
  int tx=ix*256,ty=iy*256,kind=(int)(rnd()%8);
  if(kind==0||kind==2){ground(tx+47,ty+47,161,161,0x273b35);for(int j=0;j<22;j++){int px=tx+52+rnd()%149,py=ty+52+rnd()%149;ground(px,py,5,5,(j%2)?0x3e5848:0x344d3c);}ground(tx+48,ty+123,162,7,0x566260);ground(tx+123,ty+48,7,160,0x566260);continue;}
  int n=1+(kind%3!=0);for(int k=0;k<n;k++){lot[iy*8+ix][k+1]=nb;lot[iy*8+ix][0]=n;Building*b=buildings+nb++;b->x=tx+53+k*88+rnd()%9;b->y=ty+70+rnd()%29;b->w=min(n==1?130:68,tx+200-b->x);b->h=min(65+(int)(rnd()%43),ty+210-b->y);b->ht=9+rnd()%22;static const U roofs[]={0x485b66,0x52606d,0x584867,0x596157,0x6a555c,0x4c6263};b->roof=roofs[rnd()%LEN(roofs)];b->wall=mix(b->roof,DARK,4);ground(b->x-4,b->y-4,b->w+17,b->h+16,0x1a262d);}
 }
 for(int i=1;i<8;i++)for(int j=1;j<8;j++){int x=i*256,y=j*256;for(int k=-21;k<22;k+=6){ground(x+k,y-38,3,8,0x8a9988);ground(x+k,y+31,3,8,0x8a9988);ground(x-38,y+k,8,3,0x8a9988);ground(x+31,y+k,8,3,0x8a9988);}ground(x-29,y-29,5,5,0x669686);ground(x+25,y+25,5,5,0x669686);}
 zone_venues();night_club_front_lot();setup_ordinary_doors();setup_scenery();atm_place();
}
#include "road_fx.inc"
#include "street_life.inc"
#include "pulse_pose.inc"
static void dispatch_reset(void);
static void spawnCars(void){street_spawn();dispatch_reset();}
static U hashbytes(unsigned n){const unsigned char *a=(const unsigned char*)&sv;U h=2166136261u;for(unsigned i=0;i<n;i++)h=(h^a[i])*16777619u;return h;}
static U savehash(void){return hashbytes(sizeof(Save)-4);}
static int valid_save_core(void){
 if(sv.hp<0||sv.hp>100||sv.armor<0||sv.armor>90||sv.units<0||sv.units>1000000000||sv.legal<0||sv.legal>1000000000||sv.crime<0||sv.crime>1000000000||sv.sales<0||sv.sales>1000000000||sv.home<0||sv.home>2000000001)return 0;
 if(!(sv.auditT>=-1.f&&sv.auditT<=60.f)||!(sv.clearT>=-1.f&&sv.clearT<=1000.f))return 0;
 if(sv.pending<0||sv.pending>1000000||sv.restitution<0||sv.restitution>3)return 0;
 for(int i=0;i<3;i++)if(sv.stocks[i]<0||sv.stocks[i]>SAVE_STOCK_LIMIT)return 0;
 if(!(sv.x>=35&&sv.y>=35&&sv.x<2013&&sv.y<2013)||sv.selection<0||sv.selection>=LEN(specs)||sv.gear< -1||sv.gear>1||sv.owned<0||sv.owned>=1<<LEN(specs)||sv.cash<0||sv.dirty<0)return 0;
 for(int i=0;i<2;i++){Business*b=sv.b+i;if(b->product<0||b->product>=LEN(prod)||b->tier<0||b->tier>3||b->crew<0||b->crew>2||b->own<0||b->own>1||(b->own&&b->tier<1)||!(b->timer>=-1.f&&b->timer<=7200.f)||b->batches<0||b->batches>1000000000)return 0;}
 return 1;
}
static int valid_ledger_save(int day_seconds){
 if(!(sv.dayClock>=0.f&&sv.dayClock<(float)day_seconds))return 0;
 for(int i=0;i<LEN(sv.legit);i++){
   const LegalBusiness*b=sv.legit+i;
   if(b->owned<0||b->owned>1||b->pending<0||b->pending>480||!(b->elapsed>=0.f&&b->elapsed<121.f)||!(b->cooldown>=0.f&&b->cooldown<=2.f*day_seconds))return 0;
 }
 return 1;
}
static void legacy_profile(void){
 sv.playedTicks=0;sv.look=defaultLook;sv.legacyClock=1;sv.medPatches=0;z(sv.name,sizeof sv.name);
}
/* Preserve the current time of day and which closing each outstanding
 * deposit was promised. Rev9 and older put those closings 8 minutes apart. */
static void migrate_day_clock(void){
 const float until_old_close=LEGACY_DAY_SECONDS-sv.dayClock;
 for(int i=0;i<LEN(sv.legit);i++){
  LegalBusiness*b=sv.legit+i;
  if(b->owned&&b->pending>0&&b->cooldown>0.f){
   int closings=b->cooldown>until_old_close+LEGACY_DAY_SECONDS*.5f?2:1;
   b->cooldown=closings*DAY_SECONDS-sv.dayClock;
  }
 }
}
static int business_crew_limit(int site){return site==0?1:2;}
/* Empty stock keeps the original Rev10 encoding and can still be opened by
 * older builds. The same-size Rev11 format makes that formerly reserved
 * byte meaningful only while it carries 1..99 patches. */
static void stamp_save(void){sv.rev=sv.medPatches?SAVE_REV_PATCHES:SAVE_REV;sv.hash=savehash();}
static void accept_loaded_save(void){
 /* The old Lantern distributor includes its operator. Current and old
    careers keep that hire, along with the original batch and balances. */
 sv.b[0].crew=min(sv.b[0].crew,business_crew_limit(0));
 stamp_save();wason=1;
}
static void seal_loaded(int bank_already_present){
 /* A legacy pending flag used the retired threshold. Only confirmed cases
  * survive. Older cases get a fresh five minutes without confiscation. */
 sv.audit=0;sv.auditT=0.f;
 if(!bank_already_present)sv.bank=sv.dirtySpent=0;
 sv.sweepTicks=sv.charges?REVENUE_SWEEP_TICKS:0;
 migrate_day_clock();
 accept_loaded_save();
}
static void migrate_economy(void){
 static const float oldTimes[]={24.f,34.f,40.f};
 z(sv.legit,sizeof(sv.legit));sv.dayClock=0;
 for(int i=0;i<2;i++){
   Business*b=sv.b+i;
   if(b->timer>0){float initial=b->durationMs>0?b->durationMs*.001f:oldTimes[b->product];
     float ratio=b->timer/initial;if(ratio>1)ratio=1;
     b->timer=prod[b->product].time*ratio;b->durationMs=prod[b->product].time*1000;
   }else b->durationMs=0;
 }
 legacy_profile();seal_loaded(0);
}
static void migrate_old_save(void){
 for(int i=0;i<LEN(specs);i++)sv.carCond[i]=100;
 sv.carCond[sv.selection]=cl(sv.vehicleCond,0,100);sv.taxiFares=0;migrate_economy();
}
#ifndef DC_TEST
extern int disk_read(void*,unsigned);extern int disk_write(const void*,unsigned);
#if defined(_WIN_NATIVE) || defined(DC_LINUX_NATIVE)
extern void disk_slot(int);
extern int disk_present(void);
extern int disk_delete(void);
extern int disk_create(const void*,unsigned);
#endif
#ifdef DC_LINUX_NATIVE
extern int linux_disk_backup_upgrade(int);
#endif
#endif
static void starter_save(void){
 z(&nv,sizeof nv);z(&sv,sizeof sv);sv.magic=0x44433131;sv.rev=SAVE_REV;sv.hp=100;sv.gear=-1;sv.look=defaultLook;
 sv.x=loc[12].x;sv.y=loc[12].y;sv.vehicleCond=100;for(int i=0;i<LEN(specs);i++)sv.carCond[i]=100;sv.b[1].product=2;
}
static int name_ok(const char*n){for(int i=0;i<20;i++){if(!n[i])return 1;if(n[i]<32||n[i]>126)return 0;}return 0;}
static int valid_profile_save(int extended){
 /* Upstream uses the 304-byte Rev11 only when patches are stocked.
    The 1044-byte Base Village extension also has Rev11; it accepts empty
    and stocked patch counts without changing either disk layout. */
 int patch_ok=extended?sv.medPatches<=MAX_MED_PATCHES:sv.rev==SAVE_REV_PATCHES?sv.medPatches>0&&sv.medPatches<=MAX_MED_PATCHES:!sv.medPatches;
 return sv.intro<=4&&look_valid(&sv.look)&&sv.legacyClock<=1&&patch_ok&&name_ok(sv.name);
}
static int valid_banking_save(void){
 return sv.bank>=0&&sv.dirtySpent>=0&&!sv.audit&&sv.auditT==0.f;
}
static int valid_sweep_save(void){
 return sv.charges?sv.sweepTicks>0&&sv.sweepTicks<=REVENUE_SWEEP_TICKS:sv.sweepTicks==0;
}
enum{NW_SAVE_X=6,NW_SAVE_Y=7,NW_SAVE_SCALE=64,HYBRID_SAVE_MARK=63};
static void save_outer_position(void){
 nv.world[NW_SAVE_X]=nv.world[NW_SAVE_Y]=0;
 if(outer_is_new(p.x,p.y)&&outer_area(p.x,p.y,3)){
  nv.world[NW_SAVE_X]=(U)(p.x*NW_SAVE_SCALE+.5f);
  nv.world[NW_SAVE_Y]=(U)(p.y*NW_SAVE_SCALE+.5f);
 }
}
static void restore_outer_position(void){
 nv.world[NW_SAVE_X]&=0x7fffffffu;
 if(!nv.world[NW_SAVE_X]&&!nv.world[NW_SAVE_Y])return;
 float x=(float)nv.world[NW_SAVE_X]/NW_SAVE_SCALE,y=(float)nv.world[NW_SAVE_Y]/NW_SAVE_SCALE;
 if(outer_is_new(x,y)&&outer_area(x,y,3)){sv.x=x;sv.y=y;}
 else nv.world[NW_SAVE_X]=nv.world[NW_SAVE_Y]=0;
}
/* Old Afterhours builds can still read the compatible 304-byte prefix
 * with a safe original-city location; only the new build sees outer terrain. */
static void outer_safe_anchor(Save*s){
 static const int gates[][2]={{1792,512},{1792,1024},{1792,1792},{1024,1792}};
 float best=1.e20f;int pick=0;
 for(int i=0;i<LEN(gates);i++){
  float d=d2(p.x-gates[i][0],p.y-gates[i][1]);
  if(d<best){best=d;pick=i;}
 }
 s->x=(float)gates[pick][0];s->y=(float)gates[pick][1];
}
void load_save(void){
 wason=0;z(&sv,sizeof sv);z(&nv,sizeof nv);
#ifndef DC_TEST
 Save11 withNight;
 if(disk_read(&withNight,sizeof withNight)&&withNight.base.rev==NIGHT_SAVE_REV&&
    withNight.base.magic==0x44433131&&withNight.base.hash==night_hash(&withNight.base,sizeof(Save)-4)&&withNight.hash==night_hash(&withNight,sizeof withNight-4)){
  cp(&sv,&withNight.base,sizeof sv);
  if(valid_save_core()&&valid_profile_save(1)&&valid_banking_save()&&valid_sweep_save()&&valid_ledger_save(DAY_SECONDS)){
   cp(&nv,&withNight.night,sizeof nv);restore_outer_position();accept_loaded_save();return;
  }
  z(&sv,sizeof sv);
 }
 if(disk_read(&sv,sizeof sv)&&sv.magic==0x44433131&&sv.hash==savehash()&&valid_save_core()&&valid_profile_save(0)&&valid_banking_save()&&valid_sweep_save()){
  if((sv.rev==SAVE_REV||sv.rev==SAVE_REV_PATCHES)&&valid_ledger_save(DAY_SECONDS)){accept_loaded_save();return;}
  if(sv.rev==9&&valid_ledger_save(LEGACY_DAY_SECONDS)){
   migrate_day_clock();accept_loaded_save();return;
  }
 }
 if(disk_read(&sv,LEGACY8_SIZE)&&sv.magic==0x44433131&&sv.rev==8&&(U)sv.sweepTicks==hashbytes(LEGACY8_SIZE-4)&&valid_save_core()&&valid_ledger_save(LEGACY_DAY_SECONDS)&&valid_profile_save(0)&&valid_banking_save()){seal_loaded(1);return;}
 if(disk_read(&sv,LEGACY7_SIZE)&&sv.magic==0x44433131&&sv.rev==7&&(U)sv.bank==hashbytes(LEGACY7_SIZE-4)&&valid_save_core()&&valid_ledger_save(LEGACY_DAY_SECONDS)&&valid_profile_save(0)){seal_loaded(0);return;}
 if(disk_read(&sv,LEGACY6_SIZE)&&sv.magic==0x44433131&&sv.rev==6&&sv.playedTicks==hashbytes(LEGACY6_SIZE-4)&&valid_save_core()&&valid_ledger_save(LEGACY_DAY_SECONDS)&&sv.intro<=4){legacy_profile();seal_loaded(0);return;}
 if(disk_read(&sv,LEGACY6_SIZE-52)&&sv.magic==0x44433131&&sv.rev==5&&(U)sv.legit[0].owned==hashbytes(LEGACY6_SIZE-56)&&valid_save_core()&&sv.intro<=4){migrate_economy();return;}
 if(disk_read(&sv,LEGACY6_SIZE-56)&&sv.magic==0x44433131&&sv.rev==4&&sv.taxiFares==hashbytes(LEGACY6_SIZE-60)&&valid_save_core()&&sv.intro<=4){sv.taxiFares=0;migrate_economy();return;}
 if(disk_read(&sv,LEGACY6_SIZE-76)&&sv.magic==0x44433131&&sv.rev==3&&(U)sv.carCond[0]==hashbytes(LEGACY6_SIZE-80)&&valid_save_core()&&sv.intro<=4){migrate_old_save();return;}
 if(disk_read(&sv,LEGACY6_SIZE-80)&&sv.magic==0x44433131&&sv.rev==2&&sv.intro==hashbytes(LEGACY6_SIZE-84)&&valid_save_core()){sv.intro=4;migrate_old_save();return;}
#endif
 starter_save();
}
void request_save(void){if(scene==0||scene==5||p.dead)return;sv.x=p.x;sv.y=p.y;sv.hp=p.hp;stamp_save();save_outer_position();
#ifndef DC_TEST
 int changed=0;const unsigned char *p=(const unsigned char*)&nv;
 for(unsigned i=0;i<sizeof nv;i++)if(p[i]){changed=1;break;}
 if(changed){
  if(sv.medPatches)nv.garage[HYBRID_SAVE_MARK]|=0x80000000u;
  Save11 blob;cp(&blob.base,&sv,sizeof sv);cp(&blob.night,&nv,sizeof nv);
  blob.base.rev=NIGHT_SAVE_REV;if(outer_is_new(blob.base.x,blob.base.y))outer_safe_anchor(&blob.base);
  blob.night.world[NW_SAVE_X]|=0x80000000u;
  blob.base.hash=night_hash(&blob.base,sizeof(Save)-4);
  blob.hash=night_hash(&blob,sizeof blob-4);
#ifdef DC_LINUX_NATIVE
  if(!linux_disk_backup_upgrade(sv.medPatches)){saveFlash=-200;return;}
#endif
  saveFlash=disk_write(&blob,sizeof blob)?200:-200;
 }else saveFlash=disk_write(&sv,sizeof sv)?200:-200;
#else
 saveFlash=200;
#endif
}
static void profiles_init(void);
static void profiles_open(int create);
static void profiles_tick(U tap);
static void profiles_pointer(int sx,int sy,U clicks);
static void profiles_paint(void);
static int stoppingToExit;
static void district_notice_reset(void),district_notice_tick(float);
static void spawnPlayer(void){
 npc_reset();pulse_guard_reset();ncr_reset();
 wheelFists=wheelOpen=wheelFeedback=0;wheelSuppressed=0;
 stoppingToExit=0;
 ng_cam_reset();
 p=(Player){0};p.x=sv.x;p.y=sv.y;
 if(!street_foot_clear(p.x,p.y,-1)){
   /* A saved driver resumes on foot; clear parked vehicles as well as
    * newly solid scenery before restoring the player.
    * Find a nearby clear footing before falling back to the block's curb. */
   int found=0;
   for(int radius=4;radius<=240&&!found;radius+=4)for(int i=0;i<16;i++){
    float a=i*.3926991f,x=sv.x+cs(a)*radius,y=sv.y+sn(a)*radius;
    if(street_foot_clear(x,y,-1)){p.x=x;p.y=y;found=1;break;}
   }
   if(!found){p.x=street_curb(sv.x,1);p.y=street_curb(sv.y,1);}

 }
 /* A living saved career keeps its exact health. Preserve the historical
  * recovery fallback for a zero-HP imported career. */
 p.hp=sv.hp?cl(sv.hp,1,100):15;p.car=-1;cars[0].type=sv.selection;vehicle_set_health(cars,sv.owned?vehicle_saved_hp(sv.selection):0);
 if(sv.owned&&outer_is_new(p.x,p.y)){
  Car nearRide=cars[0];int rx=(int)p.x,ry=(int)p.y;
  int lane=outer_nearest(p.x,p.y,&rx,&ry,80);
  if(lane>=0){const OuterRoad*r=outerRoads+lane;
   nearRide.a=look(outerNodes[r->b].x-outerNodes[r->a].x,outerNodes[r->b].y-outerNodes[r->a].y);}
  if(street_find_car_space(0,&nearRide,p.x,p.y,0,33))street_stage_car(0,nearRide);
 }
 district_notice_reset();
}
void init_city(void){taxi_reset_radio();vd_reset();night_world_reset();outer_life_reset();ng_reset();revenueCrossed=0;revenueReceipt=(RevenueReceipt){0};setupCity();bake_scenery();spawnCars();load_save();spawnPlayer();scene=0;menu=-1;mapOn=0;shelterT=0;job.kind=jobRide=-1;lastCar=near=by=-1;lastTaxiPickup=lastTaxiDrop=lastTaxiRider=-1;profiles_init();}
static void heat(float n){p.heat+=n;if(p.heat>300)p.heat=300;p.quiet=8.0f;}
static void health(int n){
 if(p.hit>0)return;
 if(sv.armor){int a=min(sv.armor,n);sv.armor-=a;n-=a;}
 p.hp=max(0,p.hp-n);p.hit=.32f;
 if(!p.hp){
  if(p.car>=0)cars[p.car].s=0;
  vd_cancel();p.dead=1;menu=-1;deadAnim=0;particle(p.x,p.y,26,PINK);
 }
}
typedef struct {const char *n,*note;int action;U ink;} Option;
static Option opt[14];static int nopt;
static char strings[14][100];
/* Only an approved checkout changes this meter, and only its actual dirty
 * allocation counts. Other wallet activity and ATM transfers never use it. */
static int case_blocks_price(int cost){return sv.charges&&cost>=REV_MAJOR_PRICE;}
static int canDirtyPurchase(int cost){
 if(cost<0)return 0;
 if(case_blocks_price(cost)){tell("Case hold: major purchases frozen. Visit Revenue.");return 0;}
 int clean=min(sv.cash,cost),dirty=cost-clean;
 if(dirty>sv.dirty){tell("Not enough credits.");return 0;}
 sv.cash-=clean;sv.dirty-=dirty;rr_checkout(cost,clean,dirty);
 int previous=sv.dirtySpent;
 sv.dirtySpent+=min(dirty,WALLET_LIMIT-sv.dirtySpent);
 if(previous<=REVENUE_DIRTY_LIMIT&&sv.dirtySpent>REVENUE_DIRTY_LIMIT)revenueCrossed=1;
 return 1;
}
static int bank_balance(void){return sv.bank;}
static int bank_transfer(int*from,int*to,int amount){
 if(scene==0||scene==5||p.dead||amount<=0||amount>*from||amount>WALLET_LIMIT-*to)return 0;
 *from-=amount;*to+=amount;
 request_save();
 if(saveFlash<0){
  *from+=amount;*to-=amount;sv.hash=savehash();
  return 0;
 }
 return amount;
}
static int bank_deposit(int amount){return bank_transfer(&sv.cash,&sv.bank,amount);}
static int bank_withdraw(int amount){return bank_transfer(&sv.bank,&sv.cash,amount);}
static int credit_wallet(int legal,int amount){
 int *which=legal?&sv.cash:&sv.dirty;
 int accepted=min(max(0,amount),WALLET_LIMIT-*which);
 *which+=accepted;return accepted;
}
static void gain(int legal,int amount,const char *label){wallet_note(label,credit_wallet(legal,amount),legal);}
static void active(int m){menu=m;nopt=0;sel=0;}
static void add_colored(const char*n,const char*note,int action,U ink){if(nopt>=LEN(opt))return;opt[nopt++]=(Option){n,note,action,ink};}
static void add(const char*n,const char*note,int action){add_colored(n,note,action,0);}
static void priced(const char*n,int price,const char*note,int action){char*d=append(strings[nopt],n);fmt(d,"  $",price);add(strings[nopt],note,action);}
#include "legal_business.inc"
static int carNear(void){return street_near_car();}
static int placeNear(void){float best=45*45;int chosen=-1;for(int i=0;i<LEN(loc);i++){float d=d2(loc[i].x-p.x,loc[i].y-p.y);if(d<best&&street_line_open(p.x,p.y,loc[i].x,loc[i].y,3)){chosen=i;best=d;}}return chosen;}
static int ordinary_door_near(void){
 if(p.car>=0)return -1;
 float closest=27.f*27.f;int selected=-1;
 for(int i=0;i<nb;i++){
  if(!ordinaryDoorY[i])continue;
  float dx=ordinaryDoorX[i]-p.x,dy=ordinaryDoorY[i]-p.y,dist=d2(dx,dy);
  if(dist>=closest)continue;
  int blocked=0;for(int step=1;step<4;step++)if(solid(p.x+dx*step*.25f,p.y+dy*step*.25f,3)){blocked=1;break;}
  if(!blocked){selected=i;closest=dist;}
 }
 return selected;
}
static int choose_place(int venue,int shell){
 if(venue<0)return 0;
 if(shell<0)return 1;
 return d2(loc[venue].x-p.x,loc[venue].y-p.y)<=d2(ordinaryDoorX[shell]-p.x,ordinaryDoorY[shell]-p.y);
}
static int ownedJob(void){if(ncr_active()){tell("Finish or cancel the Afterimage show ride first.");return 1;}if(vd_locked()){tell("Close the city listing (F6) before taking another job.");return 1;}if(job.kind>=0){tell("Finish or abandon your current job first.");return 1;}return 0;}
/* Expansion fares use signed, driveable shoulders at named landmarks, not
 * arbitrary coordinates in the hills or water. First-time downtown shifts
 * and the original seeded inner-city tariff stay on the legacy grid. */
static int taxi_outer_index(int where){
 int k=where-LEN(loc);return (unsigned)k<LEN(outerStops)?k:-1;
}
static const char *taxi_stop_label(int where){
 int k=taxi_outer_index(where);return k>=0?outerStops[k].label:loc[where].label;
}
static int taxi_dispatch_distance(int x0,int y0,int x1,int y1);
static int taxi_curb(int where,int *x,int *y,int *curb){
 int k=taxi_outer_index(where);
 if(k>=0){
  OuterPoint n=outerNodes[outerStops[k].node];
  *x=n.x-8;*y=n.y-(k==2?8:13);*curb=n.y-(k==2?21:26);
  return !solid(*x,*y,12)&&!solid(*x,*curb,6)&&street_line_open(*x,*y,*x,*curb,4);
 }
 if((unsigned)where>=LEN(loc))return 0;
 *x=loc[where].x;int road=(loc[where].y/256+1)*256;*y=road-13;*curb=road-35;
 if(*x<45||*x>ROOM-45||*y<45||*y>ROOM-45)return 0;
 if(solid(*x,*y,12)||solid(*x,*curb,8))return 0;
 for(int j=*curb;j<=*y;j+=4)if(solid(*x,j,5))return 0;
 return 1;
}
static int taxi_abs(int x){return x<0?-x:x;}
static int taxi_road_distance(int x0,int y0,int x1,int y1){
 if(y0==y1)return taxi_abs(x1-x0);
 int best=ROOM*3;
 for(int road=256;road<=1792;road+=256)best=min(best,taxi_abs(x0-road)+taxi_abs(x1-road));
 return best+taxi_abs(y1-y0);
}
static int taxi_tip_amount(int base,int hits,int severity,int rough){
 int percent=cl(26-9*hits-severity/3-4*rough,0,26);
 return (base*percent+50)/100;
}
/* A passenger who witnesses a pedestrian hit still pays the agreed base,
 * but does not leave a safe-driving tip. No penalty before pickup. */
static int taxi_current_tip(void){
 return taxi.pedHits?0:taxi_tip_amount(job.price,taxi.crashes,taxi.damage,taxi.rough);
}
static int taxi_occupied(void){return job.kind==1&&(taxi.phase==TAXI_RIDING||taxi.phase==TAXI_EXITING)&&p.car>=0&&p.car==taxi.ride;}
static void taxi_quality(float dt,float before,float angleBefore,int impact){
 if(!taxi_occupied())return;
 /* A rebound can hit the same bumper several times in one scrape. The
  * passenger counts one incident, but every point of damage still counts. */
 taxi.contactSeconds-=dt;
 if(impact>0){
  if(taxi.contactSeconds<=0||d2(p.x-taxi.contactX,p.y-taxi.contactY)>24.f*24.f){
   taxi.crashes++;
  }
  taxi.damage+=impact;
  taxi.contactSeconds=.35f;taxi.contactX=p.x;taxi.contactY=p.y;
  taxi.roughHold=0;taxi.roughCooldown=.7f;return;
 }
 Car*c=cars+p.car;
 taxi.roughCooldown-=dt;
 if(d2(p.x-job.xs[1],p.y-job.ys[1])<115.f*115.f){taxi.roughHold=0;return;}
 int sudden=before>98.f&&before-absf(c->s)>8.5f;
 int corner=before>110.f&&absf(wrap(c->a-angleBefore))>.039f;
 if(corner)taxi.roughHold+=dt;else taxi.roughHold=0;
 if((sudden||taxi.roughHold>.13f)&&taxi.roughCooldown<=0){taxi.rough++;taxi.roughCooldown=1.f;taxi.roughHold=0;}
}
/* Draft the same call as T without mutating the active job or dispatch
 * history. A quoted call stores the complete offer, not another random roll. */
typedef struct {Job job;Taxi taxi;U after;} TaxiOffer;
static int taxi_draft(TaxiOffer *offer){
 U draftSeed=taxiSeed;
 struct{int px,py,from,dx,dy,to,fromCurb,toCurb;} best={0};
 int chosen=0,outer=outer_is_new(p.x,p.y);U toss=draftSeed;
 /* Keep a first downtown Dispatch shift in the original grid. Once
  * licensed, roughly one in four downtown radio calls has an outbound
  * destination; pickups stay in the dispatcher's current region. */
 int outbound=!outer&&sv.taxiFares&&(taxi_random(&toss)>>30)==0;
 int pickupEnd=LEN(loc)+(outer?LEN(outerStops):0);
 int dropStart=outbound?LEN(loc):0,dropEnd=LEN(loc)+((outer||outbound)?LEN(outerStops):0);
 for(int i=outer?LEN(loc):0;i<pickupEnd;i++){
  if(i==lastTaxiPickup||i==lastTaxiDrop)continue;
  int x,y,curb;if(!taxi_curb(i,&x,&y,&curb))continue;
  if(d2(p.x-x,p.y-y)<150*150)continue;
  for(int j=dropStart;j<dropEnd;j++){
   if(j==i||j==lastTaxiDrop||j==lastTaxiPickup)continue;
   int qx,qy,other;if(!taxi_curb(j,&qx,&qy,&other))continue;
   float dx=(float)(qx-x),dy=(float)(qy-y);
   if(d2(dx,dy)<350*350)continue;
   if(taxi_random(&draftSeed)%(++chosen)==0){
    best.px=x;best.py=y;best.from=i;best.dx=qx;best.dy=qy;best.to=j;best.fromCurb=curb;best.toCurb=other;
   }
  }
 }
 if(!chosen)return 0;
 int rider=(int)(taxi_random(&draftSeed)%(LEN(riders)-1));if(lastTaxiRider>=0){if(rider>=lastTaxiRider)rider++;}
 else rider=(int)(taxi_random(&draftSeed)%LEN(riders));
 int distance=(outer||outbound)?taxi_dispatch_distance(best.px,best.py,best.dx,best.dy):taxi_road_distance(best.px,best.py,best.dx,best.dy);
 if(distance<=0)return 0;
 offer->job=(Job){JOB_TAXI,0,2,60+distance/16,145.f+distance*.021f,{best.px,best.dx},{best.py,best.dy}};
 offer->taxi=(Taxi){.x=best.px,.y=best.fromCurb,.a=1.570796f,.phase=TAXI_WAIT,.ride=-1,.pickup=best.from,.dropoff=best.to,.rider=rider,.curb={best.fromCurb,best.toCurb},.meters=distance};
 offer->after=draftSeed;
 return 1;
}
static void taxi_commit_call(const TaxiOffer *o){
 taxi_start_receipt();
 taxiSeed=o->after;job=o->job;taxi=o->taxi;jobRide=-1;
 lastTaxiPickup=taxi.pickup;lastTaxiDrop=taxi.dropoff;lastTaxiRider=taxi.rider;
}
static int taxi_offer(void){
 if(cabRequested){
  if(!taxi_authored_offer(cabRequested-1))return 0;
  taxi_start_receipt();return 1;
 }
 TaxiOffer o;if(!taxi_draft(&o))return 0;
 taxi_commit_call(&o);return 1;
}
static int taxi_call_allowed(void){
 if(ncr_active()){tell("Finish the active promoter ride first.");return 0;}
 if(vd_locked()){tell("Close the city listing (F6) before taking a fare.");return 0;}
 if(job.kind>=0){tell("Finish the current job first.");return 0;}
 if(!sv.taxiFares){tell("Complete a Taxi Fare from Mercury Dispatch first.");return 0;}
 if(p.car<0||!cars[p.car].owner){tell("Use an owned or licensed car for taxi calls.");return 0;}
 return 1;
}
static void taxi_dispatch_accepted(void){
 jobRide=p.car;
 navigation_follow_job();
 char msg[100];char*q=append(msg,"New fare: pick up ");q=append(q,riders[taxi.rider].name);q=append(q," near ");q=append(q,taxi_stop_label(taxi.pickup));append(q,".");
 tell(msg);
}
static void next_fare(void){
 if(!taxi_call_allowed())return;
 if(!taxi_offer()){tell("No nearby fare available yet.");return;}
 taxi_dispatch_accepted();
}
static void taxi_record_ride(int base,int tip,int bonus,int credit,int bumps);
#include "jobs.inc"
static int taxi_walk_to(float x,float y,float dt){
 float dx=x-taxi.x,dy=y-taxi.y,len=sq(d2(dx,dy)),v=93.f*FOOT_PACE*dt;
 if(len<=v){taxi.x=x;taxi.y=y;return 1;}
 taxi.a=look(dx,dy);taxi.x+=dx*v/len;taxi.y+=dy*v/len;return 0;
}
static const char* taxi_objective(void){
 static char goal[112];
 if(taxi.phase==TAXI_WAIT){
  char *q=append(goal,"Pick up ");q=append(q,riders[taxi.rider].name);q=append(q," at ");
  q=append(q,taxi_route_name());append(q,". Stop at the curb.");return goal;
 }
 if(taxi.phase==TAXI_BOARDING)return "Passenger boarding. Hold still.";
 if(taxi.phase==TAXI_RIDING){
  if(taxi.copper==CAB_OFFER)return "To Vanta Police HQ. T: see Copper for +$20. Enter: go straight.";
  if(taxi.copper==CAB_VIA)return "Drive past Copper Playhouse for +$20, then HQ. T: skip Copper.";
  if(taxi.copper==CAB_SEEN)return "Mina's name was in lights. Finish at Vanta Police HQ for +$20.";
  char *q=append(goal,"Drop off ");q=append(q,riders[taxi.rider].name);q=append(q," at ");
  q=append(q,taxi_route_name());append(q,".");return goal;
 }
 return "Passenger getting out. Hold still.";
}
static void taxi_tick(float dt){
 if(npc_replaces(N_ACT_RIDER+taxi.rider)||npc_holds(N_ACT_RIDER+taxi.rider))return;
 if(taxi.phase==TAXI_WALKING&&taxi.linger>0){
   taxi.linger-=dt;
   int k=taxi_outer_index(taxi.dropoff);
   int tx=k>=0?outerNodes[outerStops[k].node].x-2:loc[taxi.dropoff].x;
   int ty=k>=0?taxi.curb[1]:loc[taxi.dropoff].y;
   if(taxi_walk_to(tx,ty,dt))taxi.linger=0;
 }
 if(job.kind!=1||p.dead)return;
 if(taxi.phase>=TAXI_RIDING&&p.car!=taxi.ride){end_job();taxi.linger=0;tell("Fare cancelled: you left the taxi.");return;}
 /* The painted pickup includes the curb where the rider is standing. */
 int nearStop=d2(p.x-job.xs[job.step],p.y-job.ys[job.step])<37*37&&
              p.y>=taxi.curb[job.step]-1&&p.y<=job.ys[job.step]+35;
 int stopped=p.car>=0&&cars[p.car].owner&&absf(cars[p.car].s)<5;
 switch(taxi.phase){
 case TAXI_WAIT:
   taxi_walk_to(job.xs[0],taxi.curb[0],dt);
   if(nearStop&&stopped){if((taxi.still+=dt)>.27f){taxi.phase=TAXI_BOARDING;taxi.ride=p.car;tell("Passenger boarding.");}}
   else taxi.still=0;
   break;
 case TAXI_BOARDING:
   if(!nearStop||!stopped||p.car!=taxi.ride){taxi.phase=TAXI_WAIT;taxi.still=0;break;}
   if(taxi_walk_to(p.x,p.y,dt)){taxi.phase=TAXI_RIDING;taxi.still=0;advanceJob();char msg[90];char*q=append(msg,"Passenger on board. Next stop: ");append(q,taxi_stop_label(taxi.dropoff));
    tell(taxi.copper==CAB_OFFER?"Ren: My sister Mina's on the Copper Playhouse marquee. Pass it for $20 extra?":msg);}
   break;
 case TAXI_RIDING:
   cab_passing_copper();
   if(nearStop&&stopped){if((taxi.still+=dt)>.27f){taxi.phase=TAXI_EXITING;taxi.x=p.x;taxi.y=p.y;tell("Passenger getting out.");}}
   else taxi.still=0;
   break;
 case TAXI_EXITING:
   if(!nearStop||!stopped){taxi.phase=TAXI_RIDING;taxi.still=0;break;}
   if(taxi_walk_to(job.xs[1],taxi.curb[1],dt)){taxi.phase=TAXI_WALKING;taxi.linger=2.5f/FOOT_PACE;
    int base=job.price,tip=taxi_current_tip(),before=sv.cash;
    advanceJob();char msg[96];char*q=append(msg,"Fare $");q=number(q,base);q=append(q," + tip $");q=number(q,tip);
    int bonus=taxi_detour_bonus();
    if(bonus){q=append(q," + Copper $");q=number(q,bonus);}
    if(sv.cash-before<base+tip+bonus){
     q=number(append(q,". $"),sv.cash-before);append(q," credited (wallet limit). T: next call.");
    }else append(q,". New call available: T.");
    tell(msg);noticeInk=CASH_GREEN;}
   break;
 default:break;
 }
}
static void start_batch(Business*b,float seconds){
 b->timer=seconds;b->durationMs=(int)(seconds*1000.f+.5f);
}
static float growth_fraction(int i){
 Business*b=sv.b+i;
 if(!b->own||b->timer<=0)return 0;
 float full=b->durationMs>0?b->durationMs*.001f:prod[b->product].time;
 float progress=full>0?1.f-b->timer/full:0;
 return progress<0?0:progress>1?1:progress;
}
/* The operations clock is independent of the current menu and interface.
 * Run exactly once per game tick and save both a harvest and any next batch
 * as one state. Do not use this clock to advance pursuits or inspections. */
static int warehouse_units(int site){return site?sv.stocks[2]:sv.stocks[0]+sv.stocks[1];}
static int warehouse_secured(int site){return sv.protect&(1|(2<<site));}
enum { WAREHOUSE_CAPACITY=60 };
static int warehouse_direct_ship(int site){return site==1&&sv.b[1].crew==2;}
static int warehouse_can_fit(int site,int units){
 return warehouse_direct_ship(site)||warehouse_units(site)<=WAREHOUSE_CAPACITY-units;
}
static int warehouse_has_room(int site){
 const Business*b=sv.b+site;
 return warehouse_can_fit(site,batch_units(b->product,b->tier));
}
/* Rev2-era records can contain an unscheduled pending amount with a zero
 * timer. Leave them byte-for-byte alone; mark only a newly matured but
 * uncollectable exchange using this already-valid negative float. */
#define CLEARING_WAIT_FOR_WALLET (-.75f)
static int cinder_awaits_wallet(void){
 const Business*b=sv.b+1;
 return warehouse_direct_ship(1)&&b->timer>0.f&&b->timer<=.0002f&&
        WALLET_LIMIT-sv.dirty<batch_units(b->product,b->tier)*prod[b->product].price-5;
}
static void production_tick_mode(float dt,int active_work){
 int changed=active_work?legal_tick(dt):legal_tick_mode(dt,0);
 for(int i=0;i<2;i++){
  Business*b=sv.b+i;if(!b->own)continue;
  if(b->timer>0){
   float left=b->timer-dt;
   if(left<=.0002f){
    int units=batch_units(b->product,b->tier);
    /* A running pre-update Lantern batch can finish over the nominal
       capacity exactly once. Do not create an un-loadable save even if
       an imported inventory is already at the validator's ceiling. */
    int direct=warehouse_direct_ship(i),fee=units*prod[b->product].price-5;
    if((!direct&&sv.stocks[b->product]>SAVE_STOCK_LIMIT-units)||
       (direct&&WALLET_LIMIT-sv.dirty<fee)){
     /* Keep the finished batch, its contents and supplied inputs intact.
      * Save the waiting state only once, not on every full-wallet tick. */
     if(b->timer>.0002f){b->timer=.0001f;changed=1;
      tell(direct?"Cinder shipment ready. Make room in your dirty wallet.":"Harvest ready. Make room in storage at Kiosk 09.");
     }
     continue;
    }
    b->timer=0;b->durationMs=0;
    if(warehouse_direct_ship(i)){
     gain(0,fee,"Crew moved a batch ($5 fee deducted).");sv.sales=min(SAVE_COUNTER_LIMIT,sv.sales+units);
    }else{
     sv.stocks[b->product]+=units;
     tell(i?"Cinder batch ready. Stock is in storage.":
          warehouse_units(0)>WAREHOUSE_CAPACITY?"Grow batch stored. Sell excess stock at Kiosk 09.":"Grow batch ready. Stock is in storage.");
    }
    /* The total is also a saved four-batch upkeep/animation counter.
     * At its representable ceiling preserve the modulo-four phase. */
    b->batches=b->batches>=SAVE_COUNTER_LIMIT?SAVE_COUNTER_LIMIT-3:b->batches+1;
    if(b->batches%4==0&&sv.cash>=18){sv.cash-=18;tell("Property upkeep: $18 clean.");noticeInk=CASH_GREEN;}
    changed=1;
   }else b->timer=left;
  }
  if(b->crew&&b->timer<=0&&sv.units>=batch_supply(i)&&warehouse_has_room(i)){
   sv.units-=batch_supply(i);start_batch(b,prod[b->product].time);changed=1;
  }
 }
 if(sv.pending&&!sv.charges&&(sv.clearT>0.f||sv.clearT==CLEARING_WAIT_FOR_WALLET)){
  int waiting=sv.clearT>0.f;
  if(waiting)sv.clearT-=dt;
  if(sv.clearT<=.0002f){
   int accepted=credit_wallet(1,sv.pending);sv.pending-=accepted;
   sv.clearT=sv.pending?CLEARING_WAIT_FOR_WALLET:0.f;
   if(accepted)cashNote(sv.pending?"Exchange partly settled; remainder held.":"Exchange settled.",accepted);
   else if(waiting)tell("Exchange ready. Make room in your clean wallet.");
   changed|=waiting||accepted>0;
  }
 }
 if(changed)request_save();
}
static void production_tick(float dt){production_tick_mode(dt,1);}
/* Only the opening dirty-spend rule cancels unsettled dirty proceeds.
 * Closing the purchase before seizing keeps both results in one save. */
static void finish_purchase(void){
 if(!revenueCrossed)return;
 revenueCrossed=0;
 int pending=sv.pending;for(int i=0;i<LEN(sv.legit);i++)pending+=sv.legit[i].pending;
 revenueReceipt=(RevenueReceipt){REVENUE_THRESHOLD,0,sv.dirty,pending};
 rr_record(REVENUE_THRESHOLD,0,sv.dirty,pending);
 sv.dirty=0;sv.pending=0;sv.clearT=0;legal_seize();
 revenue_open_case();
 if(job_is_service(job.kind)){
  job.unpaid=1;tell("Revenue case open. This shift now counts as unpaid community service.");
 }else tell(revenueReceipt.dirty||pending?"Funds seized. Resolve your case at the Revenue Annex.":"No funds seized. Resolve your case at the Revenue Annex.");
 noticeInk=CASH_RED;
}
/* Called once per active 60 Hz frame. Cash earned just before this
 * deadline is still on hand. A quiet zero-balance sweep restarts the
 * deadline without an asset-seized alert, and saves the new deadline. */
static void revenue_tick(void){
 if(!sv.charges)return;
 if(--sv.sweepTicks>0)return;
 sv.sweepTicks=REVENUE_SWEEP_TICKS;
 if(sv.cash||sv.dirty){
  revenueReceipt=(RevenueReceipt){REVENUE_SWEEP,sv.cash,sv.dirty};
  rr_record(REVENUE_SWEEP,sv.cash,sv.dirty,0);
  sv.cash=sv.dirty=0;
  tell("Revenue seized cash on hand. Bank funds are safe.");
  noticeInk=CASH_RED;
 }
 request_save();
}
static char*revenue_time(char*label){
 int seconds=(sv.sweepTicks+59)/60;
 label=number(label,seconds/60);*label++=':';if(seconds%60<10)*label++='0';
 return number(label,seconds%60);
}
/* A checkout has one shared quote for the kiosk card and actual handoff.
 * No inventory transfers (or theft triggered) for credits that cannot fit. */
static int kiosk_offer(int item){
 return min(min(20,sv.stocks[item]),(WALLET_LIMIT-sv.dirty)/prod[item].price);
}
static int kiosk_rivals(int item,int units){
 int site=prod[item].site;
 return units>=15&&sv.b[site].own&&!warehouse_secured(site)&&warehouse_units(site)-units>=3;
}
static const char*kiosk_quote(int item){
 static char rows[3][100];int units=kiosk_offer(item),price=prod[item].price;
 if(!sv.stocks[item])return "No stock ready for this product.";
 if(!units)return "No sale: not enough space in your dirty wallet. You keep the stock.";
 char*q=number(rows[item],units);q=number(append(q,units==1?" unit at $":" units at $"),price);
 q=number(append(q," = $"),units*price);q=append(q," dirty.");
 if(kiosk_rivals(item,units))append(q," Rivals will take 3 more from this warehouse.");
 else if(units<min(20,sv.stocks[item]))append(q," Other stock stays in storage.");
 return rows[item];
}
static void sell_stock(int product){
 int quoted=min(20,sv.stocks[product]),units=kiosk_offer(product);
 if(!quoted){tell("No stock to sell.");return;}
 if(!units){tell("Dirty wallet full for this sale. No stock handed over.");return;}
 const int site=prod[product].site,rivals=kiosk_rivals(product,units);
 sv.stocks[product]-=units;sv.sales=min(SAVE_COUNTER_LIMIT,sv.sales+units);
 const char *receipt=units==quoted?"Kiosk 09 paid for the stock.":"Partial sale. Remaining stock stays in storage.";
 // Large public sales expose the source warehouse to rival crews.
 if(rivals){
  const int first=site?2:0,last=site?2:1;
  int seized=3;
  for(int i=first;i<=last;i++){int take=min(seized,sv.stocks[i]);sv.stocks[i]-=take;seized-=take;}
  receipt=site?"Sale paid. Rivals took 3 unsecured Cinder units.":"Sale paid. Rivals took 3 unsecured Lantern units.";
 }
 gain(0,units*prod[product].price,receipt);
}
static int service_ride(void){
 if(lastCar>0&&cars[lastCar].owner!=2&&lastCar!=jobRide&&
    d2(cars[lastCar].x-loc[3].x,cars[lastCar].y-loc[3].y)<105*105)return lastCar;
 return sv.owned?0:-1;
}
static void finish_shelter_rest(void){
 shelterT=0;
 p.hp=100;p.heat=p.quiet=0;
 tell("Rested at Breakwater. Health restored and pursuit cleared.");
 request_save();
}
static void makeMenu(void){
 if(menu<0)return;nopt=0;int m=menu,d;
 switch(m){
 case 16:case 17:case 18:legal_make_menu(legal_menu_index(m));break;
 case 19:case 20:case 21:legal_counter_menu(legal_rear_index(m));break;
 case 0:
 if(job_is_service(JOB_COURIER))add("COURIER ROUTE / UNPAID SERVICE","Deliver two parcels and return to Mara. Counts toward restitution and pays $0.",10);
 else add("COURIER ROUTE","Deliver two parcels and return the signed manifest to Mara.",10);
 add("TAXI FARE","Quick dispatch: pick up a rider and drive to the drop-off.",11);
 add("BUSINESS OFFICE","Buy a legal courier franchise and check its account.",LEGAL_OPEN);
 add("CAB RANK / COMPARE TWO CALLS","Preview pickup, total distance and base fare. No call required.",CAB_RANK_ACTION);break;
 case 1:
 add("VAULT RUN","Take the sealed lockbox, lose the police and bring it back to Ivo.",14);
 add("HOT PROPERTY","Steal the marked Vectra and hand it off to Vera after losing the police.",15);
 add("SIGNAL RAID","Move the relay cartridge from its studio to a backup console.",16);
 add("RIVAL COLLECTION","Recover the guarded consignment and deliver it to Vera.",17);
 add("TALK TO IVO","Ask him about the club after hours.",97);break;
 case 2:priced("SUPPLY CRATE",45,"Three universal game-only batch credits.",20);priced("MED PATCH",25,p.hp<100?"Use a patch now to restore up to 50 HP.":"At full HP, stock a patch. Hold Tab, press V when hurt.",22);add("BUSINESS OFFICE","Buy or manage this market outlet.",LEGAL_OPEN);break;
 case 14:if(sv.gear<0)priced("BASIC PULSE PISTOL",95,"A ranged weapon. Space uses your equipped tool.",25);
 priced("ARMOR VEST",55,"Adds 45 points of protective armor.",21);
 if(sv.gear==0)priced("PULSE AMP",155,"Upgrade your pulse pistol's range and damage.",23);break;
 case 3:for(d=0;d<LEN(specs);d++){if(sv.owned&(1<<d))add(specs[d].name,"Owned: deploy from your garage.",30+d);else priced(specs[d].name,specs[d].price,"Buy outright and deploy immediately.",30+d);}{
  int ride=service_ride();
  if(ride<0)add("NO VEHICLE IN SERVICE RANGE","Park a car at Nightglass or own a garage ride.",0);
  else if(vehicle_hp(cars+ride)==VEHICLE_CONDITION_MAX)add("VEHICLE ALREADY SERVICED","The eligible car is at full condition.",0);
  else{
   char *label=append(strings[nopt],"SERVICE ");label=append(label,specs[cars[ride].type].name);append(label,"  $32");
   add(strings[nopt],ride?"Repair the vehicle parked at Nightglass.":"Repair your selected garage vehicle.",35);
  }
 }add("AFTERHOURS / MOTOR WORKS","Browse the custom garage, inspect builds or join a night run.",184);break;
 case 4:case 5:{int i=m-4;Business*b=sv.b+i;
 if(!b->own){priced("BUY PROPERTY",facilityPrice[i],i?"Unlocks the synthetic product line.":"Unlocks two fictional grow products.",40+i);break;}
 if(b->timer<=0){
  if(i||lantern_terminal())add("START BATCH",i?"Uses two supplies from Ferry Market. 120 seconds.":"Uses one supply from Ferry Market. 120 seconds.",44+i);
  else add("GROW ROOM TERMINAL","Start batches at the terminal behind the fern shop.",0);
 }
 else if(i&&cinder_awaits_wallet())add("SHIPMENT READY / WALLET FULL","The crew holds this batch until its entire payout fits in dirty cash.",0);
 else{fmt(strings[nopt],"BATCH RUNNING / SECONDS: ",(int)(b->timer+.9999f));add(strings[nopt],"Keeps growing here and while you do other jobs.",0);}
 if(!i&&lantern_terminal())add(b->product==0?"SWITCH TO VIOLET HAZE":"SWITCH TO NIGHTLEAF","Two different yield and value profiles.",46);
 if(b->tier<3)priced("UPGRADE PROPERTY",tier_price(i,b->tier),"More product each batch, up to tier 3.",48+i);
 if(b->crew<business_crew_limit(i))priced(b->crew?"HIRE DISTRIBUTOR":"HIRE OPERATOR",b->crew?210:130,b->crew?"Automatically sells batches for a $5 flat fee.":"Queues batches automatically when supplies are available.",50+i);
 if(!warehouse_secured(i))priced("SECURE THIS WAREHOUSE",125,"Protects this facility from rival theft after large Kiosk sales.",52);
 fmt(strings[nopt],"SUPPLIES IN STORAGE  ",sv.units);add(strings[nopt],"Buy more supply crates from June at Ferry Market.",0);
 break;}
 case 6:add("SANCTIONED SPRINT","Cross the line to start a timed loop in the same licensed car.",13);break;
 case 7:
 for(d=0;d<3;d++){
  char*q=append(strings[nopt],"SELL ");q=append(q,prod[d].name);q=append(q," / STOCK ");number(q,sv.stocks[d]);
  add(strings[nopt],kiosk_quote(d),60+d);
 }
 if(sv.pending){if(sv.clearT>0.f){fmt(strings[nopt],"CLEARING LEDGER / SEC: ",legal_seconds(sv.clearT));add(strings[nopt],"Payment settles while you roam.",0);}
  else if(sv.clearT==CLEARING_WAIT_FOR_WALLET){fmt(strings[nopt],"READY / CLEAN CREDITS HELD: $",sv.pending);add(strings[nopt],"Make room in your clean wallet to receive the rest.",0);}
  else add("ARCHIVED LEDGER","This old career has an unscheduled ledger entry.",0);}
 else add("EXCHANGE AT OWNED BUSINESSES","Use the Business Office at an owned legal franchise.",0);
 break;
 case 8:priced("FULL TREATMENT",42,"Return to full vitality.",24);priced("ARMOR VEST",55,"Adds 45 points of protective armor.",21);break;
 case 9:
 if(!sv.home)priced("BUY SAFEHOUSE",550,"Own a place to rest, hide from police and recover after being downed.",70);
 else add("REST AT SAFEHOUSE","Stay inside for 10 seconds to heal and clear pursuit.",72);
 break;
 case 10:
 if(job_is_service(JOB_RECOVERY))add("LICENSED RECOVERY / UNPAID SERVICE","Restart and return Dax's van. Counts toward restitution and pays $0.",12);
 else add("LICENSED RECOVERY","Restart Dax's stalled van, then drive it to his service bay.",12);
 add("BUSINESS OFFICE","Buy or manage the recovery franchise.",LEGAL_OPEN);break;
 case 11:
 if(sv.charges){
  fmt(strings[nopt],"PAY CASE PENALTY  $",REV_CASE_FINE);
  add(strings[nopt],"Clean credits only. Clears purchase restrictions immediately.",80);
  char*q=number(append(strings[nopt],"COMMUNITY SERVICE  /  "),sv.restitution);
  number(append(q," OF "),RESTITUTION_SHIFTS);
  add(strings[nopt],"Finish three unpaid courier or recovery shifts to clear the case. All pay $0.",0);
  revenue_time(append(strings[nopt],"NEXT CASH SWEEP  "));
  add(strings[nopt],"Every 5 active minutes, Revenue takes cash on hand. Clean funds in the bank stay safe.",0);
 }
 else add("NO OPEN CASE","Spend over $500 in dirty money in one game day and Revenue will seize the remainder.",0);
 {char*q=number(append(strings[nopt],"DIRTY SPENDING TODAY  $"),sv.dirtySpent);
  number(append(q," / $"),REVENUE_DIRTY_LIMIT);
  add(strings[nopt],"Only the dirty portion of purchases counts. Resets at midnight in the 24-minute game day.",0);}
 break;
 case 15:add("DISPATCH DESK","The station and neighborhood posts share one finite police roster.",0);break;
 case 13:add("GO INSIDE","Step into your apartment.",94);break;
 case PAUSE_MENU:
 add("RESUME","Return to the game.",90);add("SAVE MANUALLY","Writes your career to your local save file.",91);
 if(job.kind>=0)add("ABANDON CURRENT CONTRACT","No payout. Free roam continues.",92);
 add("SAVE MENU","Switch careers or create a new character.",95);
 add("SHOW CONTROLS","Movement, action and menu keys.",93);
 add("CITY LISTINGS  /  F6","Free-roam activities and neighborhood stories.",96);break;
 }
 if(npc_front_option())add("PERSONAL INTERACTION","Say hello, stay quiet, or draw your owned pistol.",N_ACT_OPEN);
 if(menu!=PAUSE_MENU&&legal_menu_index(menu)<0&&legal_rear_index(menu)<0)add("BACK","Close this menu.",90);
 sel=cl(sel,0,max(0,nopt-1));
}
static void doOpt(int a){
 if(a==N_ACT_OPEN){if(npc_open_context())menu=-1;return;}
 if(a==184){night_garage_open(-1);return;}
 if(a==CAB_RANK_ACTION){cab_rank_counter();return;}
 if(legal_opt(a,&menu))return;
 if(a>=10&&a<=17){beginJob(a-10);return;}
 if(a>=30&&a<=34){
  if(job.kind==JOB_SPRINT&&job.step>0&&jobRide==0){
   tell("Finish or abandon the sprint before changing the registered car.");return;
  }
  int i=a-30,newCar=!(sv.owned&(1<<i));
  Car next={.x=p.x,.y=p.y,.type=i,.owner=1};
  vehicle_set_health(&next,newCar?VEHICLE_CONDITION_MAX:vehicle_saved_hp(i));
  if(!street_find_car_space(0,&next,p.x,p.y,0,94)&&
     !street_find_car_space(0,&next,p.x,p.y,0,0)){
   tell("No clear curb space. Try the garage again.");return;
  }
  if(newCar&&!canDirtyPurchase(specs[i].price))return;
  if(newCar&&sv.intro==3)sv.intro=4;
  street_stage_car(0,next);
  sv.owned|=1<<i;sv.selection=i;sv.vehicleCond=next.cond;vehicle_store_health(i,&next);lastCar=0;p.car=-1;
  tell(next.cond?"Ride ready by the curb.":"Wreck at the curb; use Service at the desk.");menu=-1;finish_purchase();request_save();return;
 }
 switch(a){
 case 20:if(sv.units>SAVE_COUNTER_LIMIT-3){tell("Supply storage full. Nothing charged.");return;}if(canDirtyPurchase(45)){sv.units+=3;tell("Three supply credits added.");}break;
 case 21:if(sv.armor==90)tell("Armor is already full.");else if(canDirtyPurchase(55)){sv.armor=min(90,sv.armor+45);tell("Armor equipped.");}break;
 case 22:
  if(p.hp==100&&sv.medPatches>=MAX_MED_PATCHES){tell("Patch stock is full. Nothing charged.");return;}
  if(canDirtyPurchase(25)){
   if(p.hp<100){p.hp=min(100,p.hp+50);tell("Vitality restored.");}
   else{char m[68];fmt(m,"Med patch stocked. Hold Tab to see your supply: ",++sv.medPatches);tell(m);}
  }
  break;
 case 24:if(p.hp==100)tell("Health is already full.");else if(canDirtyPurchase(42)){p.hp=100;tell("Vitality restored.");}break;
 case 25:if(sv.gear<0&&canDirtyPurchase(95)){sv.gear=0;wheelFists=0;if(sv.intro==3)sv.intro=4;tell("Pulse pistol equipped. Space fires. Hold Tab to switch.");}break;
 case 23:if(sv.gear!=0)tell("An unmodified pulse pistol is required.");else if(canDirtyPurchase(155)){sv.gear=1;tell("Pulse amp installed.");}break;
 case 35:{
  int ride=service_ride();
  if(ride<0)tell("Bring a vehicle to Nightglass first.");
  else if(vehicle_hp(cars+ride)==VEHICLE_CONDITION_MAX)tell("The vehicle is already at full condition.");
  else if(canDirtyPurchase(32)){
   vehicle_set_health(cars+ride,VEHICLE_CONDITION_MAX);
   if(!ride){sv.vehicleCond=100;vehicle_store_health(sv.selection,cars);}
   tell("Vehicle serviced to 100 condition.");
  }
  break;
 }
 case 40:case 41:if(!sv.b[a-40].own&&canDirtyPurchase(facilityPrice[a-40])){sv.b[a-40]=(Business){1,0,1,a==40?0:2,0,0,0};tell("Property is yours.");}break;
 case 44:case 45:{Business*b=sv.b+a-44;if(!b->own||b->timer>0||(a==44&&!lantern_terminal()))break;if(!warehouse_has_room(a-44)){tell("Warehouse full. Sell stock at Kiosk 09 first.");break;}int need=batch_supply(a-44);if(sv.units<need){tell("Not enough supply credits. Buy some first.");break;}sv.units-=need;start_batch(b,prod[b->product].time);tell(a==44?"Batch started. Check the plants in the back room.":"Cinder batch started.");break;}
 case 46:if(!lantern_terminal())return;else if(sv.b[0].timer>0)tell("Wait for the batch to finish first.");else sv.b[0].product^=1;break;
 case 48:case 49:{Business*b=sv.b+a-48;if(b->own&&b->tier<3){
    int site=a-48;
    if(b->timer>0&&!warehouse_can_fit(site,batch_units(b->product,b->tier+1)))
      tell("Clear storage at Kiosk 09 before expanding this batch.");
    else if(canDirtyPurchase(tier_price(site,b->tier))){b->tier++;tell("Production tier upgraded.");}
   }break;}
 case 50:case 51:{
  int site=a-50;Business*b=sv.b+site;
  if(!b->own||b->crew>=business_crew_limit(site))return;
  if(canDirtyPurchase(b->crew?210:130)){
   b->crew++;tell(b->crew==1?"Operator hired. Batches start when supplies and storage are available.":"Distributor hired. Future Cinder batches ship directly to Kiosk 09.");
  }break;
 }
 case 52:{
  int site=menu-4;
  if(site>=0&&site<2&&sv.b[site].own&&!warehouse_secured(site)&&canDirtyPurchase(125)){
   sv.protect|=2<<site;tell("Lockboxes installed. This warehouse is secured.");
  }
  break;
 }
 case 60:case 61:case 62:sell_stock(a-60);break;
 case 70:if(!sv.home&&canDirtyPurchase(550)){sv.home=1;tell("Breakwater is yours. Rest here or recover here when downed.");}break;
 case 72:
  if(sv.home){shelterT=10.f;menu=-1;tell("Resting. Stay inside to finish.");}
  return;
 case 80:if(spend(REV_CASE_FINE)){revenue_close_case();tell(job.unpaid&&job.kind>=0?"Fine paid. Your active service shift still pays $0; new shifts pay again.":"Fine paid. Case cleared.");}break;
 case 90:menu=-1;return;
 case 91:request_save();menu=-1;return;
 case 92:end_job();tell("Contract abandoned.");menu=-1;return;
 case 93:helpBack=scene;scene=2;return;
 case 94:open_interior(12);return;
 case 95:profiles_open(0);return;
 case 97:if(menu==1)tell("Ivo: Mika keeps the club. Behind this door, people ask me for favors.");return;
 case 96:menu=-1;vdPanel=1;vdCursor=0;return;
 default:return;
 }
 finish_purchase();request_save();
}
static void punch(void){
 if(p.shot>0)return;
 p.shot=.34f;punchFlash=.21f;float co=cs(p.a),si=sn(p.a);
 int hit=-1;float best=27*27;
 for(int i=0;i<LEN(peds);i++){Ped*q=peds+i;float dx=q->x-p.x,dy=q->y-p.y,d=d2(dx,dy);
  if(q->hp<=0||npc_replaces(N_ACT_STREET+i)||dx*co+dy*si<3||absf(dy*co-dx*si)>13||d>=best)continue;
  if(!street_reach_open(p.x,p.y,q->x,q->y,1,-1))continue;
  hit=i;best=d;
 }
 OuterHit crowd=outer_pick(p.x,p.y,co,si,1,best,3,p.x,p.y);
 if(crowd.id>=0){int down=outer_injure(crowd.id,1,crowd.x,crowd.y);
  particle(crowd.x,crowd.y,8,0xffc18a);heat(26);
  if(down){heat(30);tell("Knockdown. Patrols alerted.");}return;}
 if(hit<0)return;Ped*q=peds+hit;q->hp--;pedHit[hit]=.22f;street_actor_move(q,co*10,si*10,8);heat(26);
 particle(q->x,q->y,8,0xffc18a);
 if(!q->hp){q->life=12;heat(30);tell("Knockdown. Patrols alerted.");}
}
static void shoot(void){
 if(p.shot>0)return;
 if(!pistol_equipped()){punch();return;}
 PulsePose g=pulse_pose(2.5f);
 /* Do not consume charge or fire while the pose is held against cover. */
 if(g.hand)return;
 int range=sv.gear>0?165:112,damage=sv.gear>0?2:1,hit=-1;
 float cooldown=sv.gear>0?.21f:.33f;U beam=GOLD;
 if(!night_venue_apply_pulse(&range,&damage,&cooldown,&beam))return;
 trailInk=beam;p.shot=cooldown;p.trail=.11f;heat(24);
 firedPulse=g;firedX=p.x;firedY=p.y;firedA=p.a;
 trailX=g.mx;trailY=g.my;particle(trailX,trailY,5,beam);
 float best=range,co=g.co,si=g.si;
 /* Both the original aiming cone and muzzle-level cover checks apply
    to all fitted pulse frames. */
 for(int i=0;i<LEN(peds);i++){
   Ped*q=peds+i;if(q->hp<=0||npc_replaces(N_ACT_STREET+i))continue;
   float dx=q->x-p.x,dy=q->y-p.y,d=dx*co+dy*si,side=absf(dx*si-dy*co);
   if(d>0&&d>=g.extent-.01f&&d<best&&side<13+d*.12f&&
      street_reach_open(p.x,p.y,q->x,q->y,1,-1)&&street_reach_open(g.mx,g.my,q->x,q->y,1,-1)){hit=i;best=d;}
 }
 OuterHit crowd=outer_pick(p.x,p.y,co,si,0,best,g.extent-.01f,g.mx,g.my);
 if(crowd.id>=0){
  trailEndX=crowd.x;trailEndY=crowd.y;particle(crowd.x,crowd.y,10,PINK);
  if(outer_injure(crowd.id,damage,crowd.x,crowd.y))heat(41);
  return;
 }
 if(hit>=0){trailEndX=peds[hit].x;trailEndY=peds[hit].y;}
 else{
  float end=street_tracer_fraction(p.x,p.y,co*range,si*range)*range;
  float left=end-g.extent;if(left<0)left=0;
  float t=street_tracer_fraction(g.mx,g.my,co*left,si*left);
  trailEndX=g.mx+co*left*t;trailEndY=g.my+si*left*t;
 }
 if(hit>=0){Ped*q=peds+hit;q->hp-=damage;particle(q->x,q->y,10,q->role?TEAL:PINK);if(q->hp<=0){q->life=12;heat(q->role?28:41);}}
}
#include "dispatch.inc"
static void npcTick(float dt){
 for(int i=0;i<JOB_RIVAL_FIRST+JOB_RIVAL_COUNT;i++){
  Ped*q=peds+i;if(npc_replaces(N_ACT_STREET+i)||npc_holds(N_ACT_STREET+i))continue;q->cool-=dt;
  if(q->hp<=0){q->life-=dt;if(q->life<0&&i<STREET_CIVILIANS)street_respawn_civilian(i);continue;}
  if(!q->role){street_tick_civilian(i,dt);continue;}
  if(q->role!=2)continue;
  float dx=p.x-q->x,dy=p.y-q->y,dist=d2(dx,dy);float kk=sq(dist);
  q->s=0;
  if(kk>29){
   q->a=look(dx,dy);const float speed=70.f*FOOT_PACE;float v=speed*dt/kk;
   float ox=q->x,oy=q->y;street_actor_move(q,dx*v,dy*v,7);
   if(d2(ox-q->x,oy-q->y)>.01f)q->s=speed;
  }
  if(dist<72*72&&dist>4&&q->cool<=0&&street_reach_open(q->x,q->y,p.x,p.y,2,p.car)){q->cool=1.05f;if(p.quiet<3)p.quiet=3;health(8);particle(p.x,p.y,5,PINK);}
 }
 dispatch_tick(dt);
}
static void vehicle_use(int ride){
 if(p.car>=0){
  if(absf(cars[p.car].s)>STREET_EXIT_SPEED){stoppingToExit=1;tell("Hold E to brake and step out.");}
  else if(!street_exit_car())tell("The door is blocked. Pull into a clear space.");
  return;
 }
 int i=ride;
 if(i<0)return;
 int result=street_enter_car(i);
 if(result<0){tell("Wait for the car to slow down.");return;}
 if(!result)return;
 lastCar=i;
 if(job.kind==JOB_HOT&&job.step==0&&i==jobRide){
  cars[i].reported=1;advanceJob();heat(70);tell("Target acquired. Shake the police before the handoff.");
 }else if(!cars[i].owner&&!cars[i].reported){
  cars[i].reported=1;heat(38);tell("Vehicle reported stolen.");
 }

}
#include "bank_ui.inc"
#include "taxi_radio.inc"
enum{STREET_USE_NONE,STREET_USE_VENUE,STREET_USE_SHOP,STREET_USE_ATM,STREET_USE_CAR};
typedef struct{int kind,id;} StreetUse;
/* Keep the HUD and E key in agreement when street interactions overlap. */
static StreetUse street_use_target(int venue,int ride){
 int shell=ordinary_door_near();
 if(choose_place(venue,shell))return (StreetUse){STREET_USE_VENUE,venue};
 if(shell>=0)return (StreetUse){STREET_USE_SHOP,shell};
 int atm=atm_near();
 if(atm>=0)return (StreetUse){STREET_USE_ATM,atm};
 if(ride>=0)return (StreetUse){STREET_USE_CAR,ride};
 return (StreetUse){STREET_USE_NONE,-1};
}
void tick(U keys){
 const float dt=1.f/60; wheelSuppressed&=keys;keys&=~wheelSuppressed;
 U tap=keys&~prev;prev=keys;
 keys=npc_filter_keys(keys,&tap);
 if(taxiRadio&&taxi_radio_input(tap)){stoppingToExit=0;return;}
 if(wheel_tick(keys,tap)){stoppingToExit=0;return;}
 if(!(keys&USE)||scene!=1||menu>=0||mapOn||p.dead||p.car<0)stoppingToExit=0;
 // Keep fractional frame precision during long sessions.
 anim+=dt;if(anim>=3600.f)anim-=3600.f;
 if(scene==0||scene==5){profiles_tick(tap);return;}
 if(scene==NIGHT_PHONE_SCENE){if(tap&SAVE)request_save();night_phone_tick(keys,tap);return;}
 if(vd_input(tap))return;
 if(taxi_radio_input(tap))return;
 if(!p.dead&&menu<0&&!mapOn&&(scene==1||scene==3||scene==4||scene==NIGHT_CLUB_SCENE||scene==NIGHT_GARAGE_SCENE||scene==NIGHT_VENUES_SCENE||scene==NIGHT_TRAP_SCENE)
    &&night_phone_toggle(tap)){night_phone_open();return;}
 if(!p.dead&&(tap&(SLOT|NEW_SLOT))){profiles_open(!!(tap&NEW_SLOT));return;}
 if(saveFlash>0)saveFlash--;if(saveFlash<0)saveFlash++;
 if(!p.dead&&scene!=2&&menu!=PAUSE_MENU){
   if(sv.playedTicks!=~0u)sv.playedTicks++;
   production_tick(dt);
   revenue_tick();
   night_world_tick(dt);night_phone_world_tick(dt);night_trap_world_tick(dt);night_club_business_tick();night_venues_world_tick();ng_schedule_tick();outer_life_tick(dt);
 }
 if(tap&SAVE)request_save();
 if(scene==3&&menu!=PAUSE_MENU&&shelterT>0&&(shelterT-=dt)<=0)finish_shelter_rest();
 bank_warning_tick();
 if(scene==NIGHT_CLUB_SCENE){night_club_tick(keys,tap);return;}
 if(scene==NIGHT_GARAGE_SCENE){night_garage_tick(keys,tap);return;}
 if(scene==NIGHT_VENUES_SCENE){night_venues_tick(keys,tap);return;}
 if(scene==NIGHT_TRAP_SCENE){night_trap_tick(keys,tap);return;}
 if(night_trap_entry(tap))return;
 if(night_club_entry(tap))return;
 if((scene==3||scene==4)&&night_venues_entry(tap))return;
 if(scene==3){if(menu<0)night_venue_reload(tap);interior_tick(keys,tap);return;}
 if(scene==4){walkin_tick(keys,tap);return;}
 if(scene==2){if(tap&(ESC|ENTER|USE|HELP)){scene=helpBack;}return;}
 if(p.dead){
  deadAnim++;
  if(deadAnim>45&&(tap&(ENTER|USE))){
   int destination=sv.home?9:8;
   if(p.heat>0&&dispatch_engaged()&&sv.arrests<WALLET_LIMIT)sv.arrests++;
   if(!sv.home)sv.cash=max(0,sv.cash-35);
   sv.x=loc[destination].x;sv.y=loc[destination].y;sv.hp=100;
   end_job();spawnPlayer();
   tell(sv.home?"Recovered at your Breakwater safehouse.":"Recovered at Tideway Clinic. Fee: up to $35.");
   request_save();
  }
  return;
 }
 if(bank_ui_tick(tap))return;
 if(tap&HELP){helpBack=1;scene=2;return;}
 if((tap&MAP)&&menu<0){mapOn=!mapOn;if(mapOn)navigation_input(tap);return;}
 if(mapOn){if(tap&ESC)mapOn=0;else{dispatch_public_atlas_tick(dt);navigation_input(tap);}return;}
 if((tap&CAMERA)&&p.car>=0&&menu<0){
  nv.garage[60]=!ch_pref();
  tell(ch_pref()?(fastGfx?"Chase view selected. Turn off F2 Low FX to use it.":"Chase view / C for overhead."):"Overhead view / C for chase.");
  request_save();
 }
 if(tap&GFX){fastGfx=!fastGfx;tell(fastGfx?"Low FX on: rain and dynamic glows off.":"Full FX on.");}
 if(menu<0){
  if(taxi_occupied()&&taxi.phase==TAXI_RIDING&&(taxi.copper==CAB_OFFER||taxi.copper==CAB_VIA))
   cab_choose_route(tap);
  else if(tap&TAXI_NEXT)next_fare();
 }
 if(tap&ESC){if(menu>=0){menu=-1;return;}active(PAUSE_MENU);makeMenu();return;}
 p.secs+=dt;p.noticeT-=dt;
 if(menu>=0){makeMenu();if(tap&UP)sel=(sel+nopt-1)%nopt;if(tap&DOWN)sel=(sel+1)%nopt;if(tap&(USE|ENTER))doOpt(opt[sel].action);return;}
 p.hit-=dt;p.shot-=dt;p.trail-=dt;punchFlash-=dt;crashFlash-=dt;for(int i=0;i<LEN(pedHit);i++)pedHit[i]-=dt;
 job_clock_tick(dt);
 int x=!!(keys&RIGHT)-!!(keys&LEFT),y=!!(keys&DOWN)-!!(keys&UP);
 walking=(x||y)&&p.car<0;if(walking)footcycle=wrap(footcycle+FOOT_PACE*(keys&BRAKE?.31f:.22f));
 float checkpointX=p.x,checkpointY=p.y;
 roadFX_tick();
 street_tick_traffic(dt);
 if(p.car>=0){pulse_guard_reset();Car*c=cars+p.car;const CarSpec*s=specs+c->type;
  float speedBefore=absf(c->s),angleBefore=c->a;int taxiImpact=0;
  if(stoppingToExit){y=0;keys|=BRAKE;}
  /* Pressing the opposite pedal brakes first. The same input can then
   * creep in the other direction instead of launching backwards. */
  int power=ng_power(c),gripTier=ng_grip(c);
  if(y<0)c->s+=s->acc*(1+.12f*power)*dt*(c->s<-1.f?1.45f:1.f);
  if(y>0)c->s-=s->acc*dt*(c->s>1.f?2.4f*night_world_brake():.53f);
  if(!y)c->s*=.977f;if(keys&BRAKE)c->s*=1.f-.10f*night_world_brake();
  float healthFactor=.50f+.005f*c->cond;
  float limit=s->vmax*healthFactor*(1+.065f*power),reverse=s->reverse*healthFactor;
  if(c->s>limit)c->s=limit;if(c->s<-reverse)c->s=-reverse;
  if(vd_cross_player_stop(c,cs(c->a)*c->s*dt,sn(c->a)*c->s*dt))c->s=0;
  float f=absf(c->s);
  if(f>7&&x){
   /* On full throttle, lengthen the arc instead of rotating in place
    * at highway speed. Shift brings back quick, tight steering. */
   float grip=f>155.f?155.f/f:1.f;
   if(grip<.65f)grip=.65f;
   street_turn_car(p.car,wrap(c->a+x*s->turn*(1+.07f*gripTier)*grip*night_world_grip()*(keys&BRAKE?1.3f:1)*dt*(c->s>0?1:-1)));
  }
  float xx=c->x,yy=c->y;
  if(night_phone_lane_stop(xx,yy,xx+cs(c->a)*c->s*dt,yy+sn(c->a)*c->s*dt,17)){
   c->s=f=0;
   if(p.noticeT<=0)tell("Freight Parkway closed. Use the Belt Workshops detour.");
  }
  street_drive_car(p.car,cs(c->a)*c->s*dt,sn(c->a)*c->s*dt);
  if(d2(c->x-xx,c->y-yy)<f*f*dt*dt*.32f&&f>16){
   int damage=cl((int)(f*f/1450),1,37);taxiImpact=damage;vehicle_impact(c,damage);c->s*=-.27f;
   if(damage>=3){particle(xx,yy,14,GOLD);if(crashFlash<=0){char msg[45];vehicle_condition_text(append(msg,"CRASH  /  CONDITION "),c);tell(msg);}crashFlash=.45f;}
   if(p.car==0&&c->owner==1){sv.vehicleCond=c->cond;vehicle_store_health(c->type,c);}
  }
  if(f>52){
    float co=cs(c->a),si=sn(c->a);
    int struck=0;
    for(int i=0;i<LEN(peds);i++)if(peds[i].hp>0&&!npc_replaces(N_ACT_STREET+i)){
      float u=peds[i].x-c->x,v=peds[i].y-c->y;
      if(absf(u*co+v*si)<15&&absf(v*co-u*si)<6.5f){
        peds[i].hp=0;peds[i].life=12;struck=1;
        break;
      }
    }
    if(struck||outer_car_hit(c->x,c->y,co,si)){
      c->s*=.55f;particle(c->x,c->y,11,PINK);
      if(p.heat<110)heat(45);else p.quiet=8;
      if(taxi_occupied()){taxi.pedHits++;tell("Pedestrian hit. Passenger withdrew the tip. Traffic patrol alerted.");}
      else tell("Pedestrian hit. Traffic patrol alerted.");
    }
  }
  roadFX_player_car(c,xx,yy,angleBefore,speedBefore,y,keys);
  p.x=c->x;p.y=c->y;p.a=c->a;
  taxi_quality(dt,speedBefore,angleBefore,taxiImpact);
  if(c->cond<=0){
   c->s=0;
   if(!street_exit_car())p.car=-1;
   health(27);
   if(p.dead)return;
   if((job.kind==JOB_HOT||job.kind==JOB_RECOVERY||job.kind==JOB_SPRINT)&&job.step>0&&c-cars==jobRide){
    end_job();tell("Job failed: the assigned vehicle was wrecked.");
   }else tell("Wrecked. Find another ride or visit Nightglass Motors.");
  }
 }else{
  night_venue_reload(tap);
  float v=(keys&BRAKE?FOOT_SPRINT_SPEED:FOOT_WALK_SPEED)*dt*night_world_foot()*night_venues_walk_factor()*(x&&y?.707107f:1);
  if(x||y)p.a=look(x,y);
  street_player_step(x*v,y*v);npc_stance(p.x,p.y,&p.a);pulse_guard_tick(1,p.x,p.y,p.a,0);
  if(!npc_attack(keys,p.x,p.y,p.a)){if(keys&PUNCH)punch();else if(keys&FIRE)shoot();}
 }
 if(stoppingToExit&&p.car>=0&&absf(cars[p.car].s)<5){
  stoppingToExit=0;
  if(!street_exit_car())tell("The door is blocked. Pull into a clear space.");
 }
 vd_shadow_sample(keys);
 check_arrival(checkpointX,checkpointY);
 job_rival_encounter();
 taxi_tick(dt);ncr_street_tick();
 by=placeNear();near=carNear();
 if((tap&USE)&&(keys&BRAKE)&&p.car<0&&npc_open_near())tap&=~USE;
 if(vd_frame(keys,tap))tap&=~(USE|ENTER);
 night_garage_street(tap);if(scene!=1)return;
 if((tap&USE)&&!job_use_street()&&!night_phone_action(tap)&&!night_venue_vendor_entry(tap)){
  if(p.car>=0)vehicle_use(-1);
  else {
   StreetUse target=street_use_target(by,near);
   switch(target.kind){
   case STREET_USE_VENUE:open_interior(target.id);break;
   case STREET_USE_SHOP:walkin_open(target.id);break;
   case STREET_USE_ATM:atm_open(target.id);break;
   case STREET_USE_CAR:vehicle_use(target.id);break;
   default:npc_open_near();break;
   }
  }
 }
 npc_encounter_tick(dt);
 if(scene!=1)return;
 if(job.kind==1&&taxi.phase>=TAXI_RIDING&&p.car!=taxi.ride)taxi_tick(0);
 p.quiet-=dt;if(p.quiet<=0&&p.heat>0){float rate=p.car>=0?26:19;p.heat-=dt*rate;if(p.heat<0)p.heat=0;}
 update_camera_lead();
 district_notice_tick(dt);
 npcTick(dt);
 for(int i=0;i<LEN(fx);i++)if(fx[i].t>0){fx[i].t-=dt;fx[i].x+=fx[i].vx*dt;fx[i].y+=fx[i].vy*dt;}
}
int get_scene(void){return scene;}
void clear_input(void){prev=0;stoppingToExit=0;wheelOpen=wheelFeedback=0;wheelSuppressed=0;}
void pointer_input(int sx,int sy,U clicks){
 /* Host letterboxing/leave messages have no logical click target. */
 if((U)sx>=W||(U)sy>=H){sx=sy=-1;clicks=0;}
 wheelX=sx;wheelY=sy;
 if(wheelOpen)return;
 if(scene==0||scene==5){profiles_pointer(sx,sy,clicks);return;}
 if(scene==NIGHT_CLUB_SCENE){night_club_pointer(sx,sy,clicks);return;}
 if(scene==NIGHT_PHONE_SCENE){night_phone_pointer(sx,sy,clicks);return;}
 if(scene==1&&mapOn&&!p.dead)navigation_pointer(sx,sy,clicks);
}

static int sx(float x){return(int)x-cx;}static int sy(float y){return(int)y-cy;}
static int beam_block(int x,int y){return (U)x>=ROOM||(U)y>=ROOM||roofMask[y*ROOM+x]||(outer_is_new(x,y)&&!outer_onroad(x,y));}
/* Both camera modes consume the same wall-clipped lamp footprint. */
static void road_beam_rays(const Car*c,float co,float si,int range,int rays[17]){
 for(int ray=0;ray<17;ray++){float side=(ray-8)*.047f;int d;
  for(d=12;d<range;d+=3){
   if(beam_block((int)(c->x+co*d-si*side*d),(int)(c->y+si*d+co*side*d)))break;
  }rays[ray]=d;
 }
}
static void projectLights(Car*c){
 if(fastGfx||c->cond<=0)return;
 int x=sx(c->x),y=sy(c->y);if(x<-95||y<-95||x>W+95||y>H+95)return;
 float co=cs(c->a),si=sn(c->a);int range=61+(c->cond*19)/100;
 int rays[17];road_beam_rays(c,co,si,range,rays);
 int x1=x+(int)(co*range),y1=y+(int)(si*range),edge=range*3/8+3;
 for(int py=max(0,min(y,y1)-edge);py<=min(H-1,max(y,y1)+edge);py++)for(int px=max(0,min(x,x1)-edge);px<=min(W-1,max(x,x1)+edge);px++){
  float dx=px-x,dy=py-y,forward=dx*co+dy*si;if(forward<12||forward>=range)continue;
  float side=(dy*co-dx*si)/forward;
  float width=absf(side);if(width>.37f)continue;
  int ray=cl((int)(side/.047f+8.5f),0,16);
  if(forward>rays[ray]||beam_block(px+cx,py+cy))continue;
  int intensity=(int)((range-forward)*9/range*(1-width/.39f));if(intensity<1)continue;
  U b=pixels[py*W+px];int r=min(255,((b>>16)&255)+intensity*15),g=min(255,((b>>8)&255)+intensity*11),bl=min(255,(b&255)+intensity*5);
  pixels[py*W+px]=RGB(r,g,bl);
 }
}
#include "model_vehicle.inc"
#include "night_vehicles.inc"
static void carbody(Car*c,int x,int y){
 int type=cl(c->type,0,4),owned=c==cars&&c->owner==1&&(sv.owned&(1<<type));
 if(!owned){model_car_stock(c,x,y);return;}
 NgMods m=ng_installed(type);
 ng_model(x,y,c->a,1.f,0,type,specs[type].c,m,c->cond,road_lamps(c),owned&&m.sub&&absf(c->s)<2&&p.car<0);
 int hp=vehicle_hp(c);if(hp>0&&hp<135){disc(x-2,y-3,3,0x303640);if(hp<69)disc(x-3-(int)(anim*8)%7,y-8,3,0x4a535b);}
 if(c->owner==1&&p.car<0){line(x-4,y-18,x,y-14,LIME);line(x,y-14,x+4,y-18,LIME);}
}
typedef struct{int x,y;}Pt;
static int previewRig;
/* Keep the gameplay feet on the same world point. Scale the articulated
 * positions, not the completed bitmap: 1 px details retain sharp ink.
 * The full-size portrait and world-facing arrow use the unchanged rig. */
static int person_offset(int v){return (6*v+(v<0?-3:3))/7;}
static Pt rig_unscaled(int x,int y,float co,float si,int u,int v,int z){
 float depth=u*si+v*co;
 if(previewRig){depth*=.28f;z=(5*z+1)/3;}
 return (Pt){x+(int)(u*co-v*si),y+(int)depth-z};
}
static Pt rig(int x,int y,float co,float si,int u,int v,int z){
 Pt at=rig_unscaled(x,y,co,si,u,v,z);
 if(!previewRig){at.x=x+person_offset(at.x-x);at.y=y+person_offset(at.y-y);}
 return at;
}
static void rod(Pt a,Pt b,int r,U c){for(int off=-r;off<=r;off++){line(a.x+off,a.y,b.x+off,b.y,c);if(off)line(a.x,a.y+off,b.x,b.y+off,c);}}
static void quad(Pt a,Pt b,Pt c,Pt d,U col){tri(a.x,a.y,b.x,b.y,c.x,c.y,col);tri(a.x,a.y,c.x,c.y,d.x,d.y,col);}
typedef struct {Pt muzzle,slide,grip,support;float co,si;int hand;} PulseSprite;
static float modelExtraPunch;
#include "model_person.inc"
#include "pulse_sprite.inc"
static void figure_held(int x,int y,U jacket,int step,float a,int style,int hue,const Look*look,const PulseSprite*gun){
 float co=cs(a),si=sn(a);int hero=style==MODEL_PLAYER,top=0,legs=0,wide=0,face=0,hs=-1;U ink=0x111a27;
 static const U skins[]={0xdeb291,0xa16352,0x805248,0xe3c3a7,0xbd8571};
 static const U hairs[]={0x29323c,0x201720,0x45403f,0x66433f,0x34263b};
 U skin=skins[hue%5],hair=hairs[hue%5],pants=0x42546b,shoe=0x111b26;
 if(look){
  const unsigned char*k=look->part;wide=(int)k[BUILD]-1;skin=charSkin[k[SKIN]];hs=k[HAIR];hair=charHair[k[HAIR_TONE]];
  face=k[DETAIL];top=k[TOP];legs=k[LEGS];pants=charLeg[k[LEG_TONE]];shoe=charShoe[k[SHOES]];
  if(!hero||jacket!=PINK)jacket=charTop[k[TOP_TONE]];
 }
 if(!look&&!previewRig){
  if(style==MODEL_OFFICER||style==MODEL_DISPATCH)pants=0x394a63;
  else if(style==MODEL_RIVAL)pants=0x463946;
  else if(style==MODEL_STAFF)pants=0x3f5260;
  else if(style==MODEL_WORKER)pants=0x5b5147;
  else{
   static const U fabric[]={0x43556b,0x615568,0x455a5a,0x565261,0x4d4f65};
   pants=fabric[hue%LEN(fabric)];
  }
 }
 U seam=mix(jacket,ink,4),rim=mix(jacket,PAPER,3);
 if(previewRig)rect(x-6,y+2,13,2,0x151d27);
 else for(int dy=-3;dy<=3;dy++){
  int half=person_offset(7-(dy<0?-dy:dy)),row=y+person_offset(3+dy);
  line(x+2-half,row,x+2+half,row,0x19232b);
 }
 int w=7+wide+(top==2||top==3);
 if(!previewRig)model_live_body(x,y,co,si,step,w,wide,top,legs,style,look!=0,hero,jacket,skin,pants,shoe,ink,gun);
 else {
 for(int side=-1;side<=1;side+=2){
  Pt hip=rig(x,y,co,si,-2,side*3,5),boot=rig(x,y,co,si,-2+step*side,side*4,0);
  Pt knee={(hip.x+boot.x)/2,(hip.y+boot.y)/2};
  rod(hip,boot,legs==1||legs==4?2:1,legs==2?skin:pants);
  if(legs==2)rod(hip,knee,1,pants);
  if(legs==1||legs==4)dot(knee.x,knee.y,mix(pants,PAPER,3));
  if(legs==3)dot(boot.x,boot.y-2,mix(pants,ink,4));
  disc(boot.x,boot.y,2,shoe);dot(boot.x,boot.y-1,look?mix(shoe,PAPER,4):0x829aab);
 }
 const int chest=1;
 Pt lt=rig(x,y,co,si,chest,-w,10),rt=rig(x,y,co,si,chest,w,10),rb=rig(x,y,co,si,-4,4+wide,5),lb=rig(x,y,co,si,-4,-4-wide,5);
 quad(lt,rt,rb,lb,ink);
 quad(rig(x,y,co,si,chest,2-w,10),rig(x,y,co,si,chest,w-2,10),rig(x,y,co,si,-4,3+wide,6),rig(x,y,co,si,-4,-3-wide,6),jacket);
 line(lt.x,lt.y,rt.x,rt.y,rim);line(lb.x,lb.y,rb.x,rb.y,seam);
 if(top==2||top==4||top==5)line((lt.x+rt.x)/2,(lt.y+rt.y)/2,(lb.x+rb.x)/2,(lb.y+rb.y)/2,top==4?rim:seam);
 for(int side=-1;side<=1;side+=2){
  int reach=hero&&side==1&&punchFlash>0?9+(int)(punchFlash*28):0;
  Pt shoulder=rig(x,y,co,si,1,side*(w-1),10),hand=rig(x,y,co,si,reach-step*side,side*(reach?3:w),4);
  Pt sleeve={(shoulder.x*2+hand.x)/3,(shoulder.y*2+hand.y)/3};
  rod(shoulder,hand,1,look&&(top==0||top==1||top==5)?skin:seam);
  if(look&&(top==0||top==5))rod(shoulder,sleeve,1,top==5?ink:seam);
  if(top==4)line(shoulder.x,shoulder.y,hand.x,hand.y,rim);
  if(top==5)dot(shoulder.x-side,shoulder.y+2,0xffdd8d);
  disc(hand.x,hand.y,1,skin);
 }
 }
 /* Facing away, the shoulder grip can tuck behind the silhouette
    but must never overwrite hair and face. The muzzle geometry stays
    off to the side so its cap and flash remain readable. */
 int gunBehind=gun&&!gun->hand&&si<-.38f;
 if(gunBehind)pulse_pistol_paint(gun);
 Pt head=rig(x,y,co,si,1,0,14);int hx=head.x,hy=head.y;
 rod((Pt){hx,hy+3},rig(x,y,co,si,1,0,10),1,mix(skin,ink,2));
 if(top==3||(!previewRig&&style==MODEL_RIVAL))disc(hx,hy+3,5,seam);
 if(hs==8){disc(hx-(int)(co*4),hy-(int)(si*4)+2,2,hair);}
 if(hs==9){rect(hx-4,hy-3,9,8,hair);rect(hx-3,hy+4,7,2,mix(hair,ink,2));}
 disc(hx,hy,4,ink);disc(hx,hy,3,skin);
 if(!previewRig)model_head_lights(hx,hy,co,si,ink,skin,hair,style,hue,face);
 if(!look){
  int capStyle=style==MODEL_OFFICER||style==MODEL_DISPATCH||style==MODEL_WORKER;
  U cap=capStyle?seam:hair;rect(hx-2,hy-3,5,2,cap);dot(hx-2,hy-1,cap);
  if(capStyle){
   line(hx-3,hy-2,hx+3,hy-2,rim);
   if(!previewRig&&si>-.25f)dot(hx+(co<-.3f?-1:co>.3f?1:0),hy-2,style==MODEL_WORKER?0xe3bd73:0xbecfdb);
  }else if(!previewRig&&hue%3==1){
   dot(hx-2,hy-3,mix(hair,PAPER,2));dot(hx+2,hy-2,hair);
  }else if(!previewRig&&hue%3==2){dot(hx+2,hy-3,mix(hair,PAPER,2));dot(hx+2,hy-1,hair);}
 }else{
  int to=co<-.15f?-1:1;
  switch(hs){
  case 0: dot(hx,hy-3,mix(skin,hair,1));break;
  case 1:rect(hx-2,hy-3,5,1,hair);dot(hx-3,hy-2,hair);break;
  case 2:rect(hx-2,hy-4,5,2,hair);line(hx-3*to,hy-2,hx-2*to,hy,hair);dot(hx+to,hy-4,mix(hair,PAPER,2));break;
  case 3:rect(hx-1,hy-5,4,3,hair);dot(hx+3,hy-3,hair);line(hx-2,hy-2,hx-2,hy,mix(hair,skin,4));break;
  case 4:for(int j=-3;j<=3;j+=2)disc(hx+j,hy-3-(j&2?0:1),1,hair);break;
  case 5:disc(hx,hy-4,4,hair);dot(hx-2,hy-6,mix(hair,PAPER,2));dot(hx+2,hy-7,mix(hair,PAPER,2));break;
  case 6:rect(hx-1,hy-7,3,5,hair);dot(hx,hy-8,mix(hair,PAPER,3));break;
  case 7:rect(hx-3,hy-4,7,2,hair);line(hx-4,hy-2,hx-4,hy+2,hair);line(hx+4,hy-2,hx+4,hy+2,hair);break;
  case 8:rect(hx-2,hy-4,5,3,hair);dot(hx-2,hy+2,mix(hair,PAPER,4));break;
  case 9:rect(hx-3,hy-4,7,2,hair);line(hx-4,hy-2,hx-4,hy+4,hair);line(hx+4,hy-2,hx+4,hy+4,hair);break;
  }
  if(si>-.58f){
   if(previewRig||absf(co)<.65f){dot(hx-1,hy,ink);dot(hx+1,hy,ink);}
   else dot(hx+(co>0?2:-2),hy,ink);
   if(face==1||face==2){line(hx-3,hy-1,hx+3,hy-1,ink);dot(hx-2,hy,face==1?TEAL:ink);dot(hx+2,hy,face==1?TEAL:ink);}
   if(face==3||face==4){line(hx-2,hy+1,hx+2,hy+1,mix(skin,hair,face==3?4:7));if(face==4)rect(hx-1,hy+2,3,2,hair);}
   if(face==5){rect(hx-2,hy+1,5,2,0x8aa3ab);dot(hx,hy+2,0x526878);}
  }
  if(face==6){dot(hx-3,hy+1,0xffdc8e);dot(hx+3,hy+1,0xffdc8e);}
 }
 if(gun&&!gunBehind)pulse_pistol_paint(gun);
 if(hero&&!gun){
    /* Keep the direction arrow at the existing world size and offset.
       The highest hairstyle still needs clearance when facing north. */
    const int start=max(17,(int)(13.f-22.f*si));
    Pt tip=rig_unscaled(x,y,co,si,start+11,0,0),el=rig_unscaled(x,y,co,si,start+3,-5,0),er=rig_unscaled(x,y,co,si,start+3,5,0);
    Pt sl=rig_unscaled(x,y,co,si,start,-2,0),sr=rig_unscaled(x,y,co,si,start,2,0);
    Pt il=rig_unscaled(x,y,co,si,start+4,-2,0),ir=rig_unscaled(x,y,co,si,start+4,2,0);
    tri(tip.x,tip.y+1,el.x,el.y+1,er.x,er.y+1,0x132532);
    quad(sl,il,ir,sr,0xffffff);tri(tip.x,tip.y,el.x,el.y,er.x,er.y,0xffffff);
 }
}
static void figure(int x,int y,U jacket,int step,float a,int style,int hue,const Look*look){
 figure_held(x,y,jacket,step,a,style,hue,look,0);
}
static void person(int x,int y,U jacket,int step,float a,int style,int hue){figure(x,y,jacket,step,a,style,hue,style==MODEL_PLAYER?&sv.look:0);}
static PulsePose npc_room_pulse(float x,float y,float a,int step,PulseRoomSolid room);
static void player_in_room(float x,float y,int dx,int dy,U jacket,int step,float a,int hue,int (*solid_room)(float,float)){
 if(pistol_equipped()&&p.car<0&&!p.dead){
  PulseSprite gun=pulse_project(npc_room_pulse(x,y,a,step,solid_room),dx,dy);
  figure_held((int)x-dx,(int)y-dy,jacket,step,a,MODEL_PLAYER,hue,&sv.look,&gun);
 }else person((int)x-dx,(int)y-dy,jacket,step,a,MODEL_PLAYER,hue);
}
static const char* storefront_sign(int i);
static void building(Building*b){
 int x=b->x,y=b->y,w=b->w,h=b->h,r=b->ht,f=y+h-r;int id=(x*13+y*29)%7;
 U tint=y<550?0xd087f4:x<870?0xf4a77d:y>1150?0x79f1de:0x8db0ff;
 U roof=mix(b->roof,0x1f283d,3),front=mix(b->wall,tint,2),side=mix(front,0x0c1b25,4);
 rect(x+10,y+12,w+17,h+5,mix(0x19212c,DARK,3));rect(x+18,y+17,w+14,h-1,0x19282e);
 tri(x+w,y-r,x+w+r/2,y-3,x+w+r/2,y+h,side);
 tri(x+w,y-r,x+w+r/2,y+h,x+w,f,side);
 tri(x,f,x+r/2,y+h,x+w+r/2,y+h,front);
 tri(x,f,x+w+r/2,y+h,x+w,f,front);
 rect(x,y-r,w,h,roof);
 U skirt=mix(roof,tint,2),top=mix(roof,0x98aeb5,2);
 box(x+2,y-r+2,w-4,h-4,roof,skirt);
 box(x+6,y-r+6,w-12,h-12,mix(roof,0x0d1821,1),top);
 if(id==3||id==5){for(int i=11;i<w-13;i+=12){int yy=y-r+13+((i/12)&1)*4;box(x+i,yy,9,h-26,0x223b51,0x668da2);line(x+i+4,yy,x+i+4,yy+h-27,0x547789);}}
 else{
  box(x+w/2-20,y-r+12,31,22,0x314b59,0x748799);
  for(int k=0;k<3;k++)rect(x+w/2-17,y-r+16+k*5,24,2,0x5c7789);
  box(x+12,y-r+h-26,19,12,0x283d4a,0x627587);
  rect(x+w-24,y-r+h-28,14,15,0x283a46);
  if(w>100){disc(x+w-26,y-r+31,13,mix(roof,tint,3));disc(x+w-26,y-r+31,10,0x354d57);line(x+w-32,y-r+31,x+w-20,y-r+31,top);}
 }
 for(int j=8;j<w-7;j+=10){U c=((x+j+y)%7<2)?tint:mix(0x233842,tint,3);rect(x+j,f+3,6,2,c);if(r>15)rect(x+j+4,f+9,5,2,mix(c,front,2));}
 if(id<=2&&w>65){const char*sign=storefront_sign((int)(b-buildings));
   if(textw(sign,0)<w-10)center(x+w/2,y-r+h-29,sign,tint,0);
   else{for(int j=0;j<3;j++)rect(x+10+j*15,y-r+h-25,8,3,mix(tint,roof,4));}}
 line(x,y-r,x+w-1,y-r,mix(top,tint,3));
 line(x,y-r,x,f,0x839ba0);
}
static void light(int x,int y,int radius,U color,int weight){
 int rr=radius*radius;
 for(int j=max(0,y-radius);j<min(ROOM,y+radius);j++)for(int i=max(0,x-radius);i<min(ROOM,x+radius);i++){int d=(i-x)*(i-x)+(j-y)*(j-y);int a=(rr-d)*weight/rr;if(a>0){U*q=city+j*ROOM+i;*q=mix(*q,color,min(7,a));}}
}
#include "street_lamps.inc"
static void tree_crown(int x,int y,int id){
 const U greens[]={0x356c58,0x4c845e,0x4f786e,0x578c6d};
 U a=greens[id%4];
 rect(x-2,y-1,4,10,0x715b4a);
 disc(x,y-8,11,0x253e3b);disc(x-3,y-11,8,a);disc(x+3,y-11,6,mix(a,0x91bb78,2));
}
#include "district_art.inc"
#include "club_front.inc"
static void setup_ordinary_doors(void){
 z(ordinaryDoorX,sizeof ordinaryDoorX);z(ordinaryDoorY,sizeof ordinaryDoorY);
 for(int i=0;i<nb;i++){
  const Building*b=buildings+i;
  if(b->w<=0||b->h<=0||zone_is_building(i))continue;
  int x=i==ncFrontId?b->x+121:b->x+b->w/2+b->ht/4,y=b->y+b->h+15;
  ordinaryDoorX[i]=x;ordinaryDoorY[i]=y;
 }
}
static void setup_scenery(void){
 z(scenery,sizeof scenery);
 for(int tile=0;tile<64;tile++){
  BlockScenery*block=scenery+tile;
  const int*row=lot[tile];
  for(int n=1;n<=row[0];n++)block->walls[block->count++]=building_bounds(buildings+row[n]);
  block->treesFrom=block->count;
  if(zone_is_lot(tile))continue;
  int ox=(tile%8)*256,oy=(tile/8)*256;
  /* Plant on grass in each quadrant; both paved park paths stay clear. */
  static const int grove[][2]={{69,82},{100,103},{166,77},{191,106},{79,162},{100,193},{157,174},{191,192}};
  static const int yard[][2]={{58,192},{196,193}};
  const int (*places)[2]=row[0]?yard:grove;
  int count=row[0]?LEN(yard):LEN(grove);
  for(int i=0;i<count;i++){
   int x=ox+places[i][0],y=oy+places[i][1],clear=1;
   for(int n=1;n<=row[0];n++){
    int id=row[n];StreetBounds b=building_bounds(buildings+id);
    if(x+13>b.x&&x-13<b.x+b.w&&y+6>b.y&&y-28<b.y+b.h)clear=0;
    if(absf(x-ordinaryDoorX[id])<24&&absf(y-ordinaryDoorY[id])<38)clear=0;
   }
   if(clear)block->walls[block->count++]=(StreetBounds){x-3,y-3,6,6};
  }
 }
}
static void paint_ordinary_door(int i){
 if(!ordinaryDoorY[i])return;
 const Building*b=buildings+i;
 int x=ordinaryDoorX[i],end=b->y+b->h;
 U glow=mix(b->roof,0xffca83,5);
 box(x-7,end-15,15,16,0x1b2d39,0x899a9f);
 rect(x-4,end-13,9,7,0x527384);rect(x+3,end-6,2,3,0xffca83);
 rect(x-9,end+1,19,3,glow);
 rect(x-4,end+5,9,10,0x556168);rect(x-4,end+11,9,2,0x7b8281);
}
static void bake_scenery(void){
 surface=city;rw=rh=ROOM;clipLeft=clipTop=0;clipRight=clipBottom=ROOM;
 for(int gy=0;gy<8;gy++)for(int gx=0;gx<8;gx++){
   int x=gx*256,y=gy*256,id=(gx*79+gy*23)%11;
   for(int k=46;k<224;k+=54){int xx=x+k,yy=y+34;line(xx,yy,xx+22,yy,0x4d5b67);rect(xx,yy,1,14,0x53606a);}
   BlockScenery*block=scenery+gy*8+gx;
   for(int i=block->treesFrom;i<block->count;i++){
    StreetBounds trunk=block->walls[i];
    disc(trunk.x+8,trunk.y+1,12,0x172823);
    tree_crown(trunk.x+3,trunk.y-4,id+i);
   }
   StreetLamp fixture;
   for(int k=0;k<2;k++)if(street_lamp_legacy(gx,gy,k,&fixture))street_lamp_bake(&fixture);
 }
 z(roofMask,sizeof roofMask);
 for(int tile=0;tile<64;tile++){
  const BlockScenery*block=scenery+tile;
  for(int i=0;i<block->count;i++){
   StreetBounds b=block->walls[i];
   for(int y=b.y;y<b.y+b.h;y++)for(int x=b.x;x<b.x+b.w;x++)roofMask[y*ROOM+x]=1;
  }
 }
 for(int i=0;i<nb;i++){Building*b=buildings+i;if(b->w<=0||b->h<=0)continue;
  int tx=(b->x/256)*256,ty=(b->y/256)*256;
  clipLeft=tx+42;clipTop=ty+42;clipRight=tx+214;clipBottom=ty+214;
  if(!zone_is_building(i))building(b);
 }
 clipLeft=clipTop=0;clipRight=clipBottom=ROOM;
 for(int i=0;i<LEN(loc);i++)zone_art(i);
 for(int i=0;i<nb;i++)paint_ordinary_door(i);
 paint_afterimage_front();
 night_world_bake();
 outer_bake();
 for(int i=0;i<LEN(outerNodes);i++){
  StreetLamp fixture;if(street_lamp_outer(i,&fixture))street_lamp_bake(&fixture);
 }
 surface=pixels;rw=W;rh=H;clipLeft=clipTop=0;clipRight=W;clipBottom=H;
}
static void diamond(int x,int y,int r,U c){line(x-r,y,x,y-r,c);line(x,y-r,x+r,y,c);line(x+r,y,x,y+r,c);line(x,y+r,x-r,y,c);}
static void mark(int x,int y,int r,U c){
 for(int i=0;i<3;i++)diamond(x,y,r+i,mix(DARK,c,7-i));
 disc(x,y,3,c);
}
#include "navigation.inc"
#include "regional_dispatch.inc"
#include "district_notice.inc"
/* Wrap onto complete words at the actual raster width. A toast can carry
 * an indoor conversation back onto the street, where the radar owns the
 * right-hand edge. Never stretch the box and then draw its text outside it. */
static const char *hud_line(const char *s,int limit,char out[130]){
 while(*s==' ')s++;
 int n=0,w=0,cut=-1;
 while(s[n]&&n<129){
  int k=(unsigned char)s[n]-32,next=k>=0&&k<95?fw0[k]:0;
  if(w+next>limit&&n)break;
  if(s[n]==' ')cut=n;
  w+=next;n++;
 }
 if(s[n]&&s[n]!=' '&&cut>0)n=cut;
 for(int i=0;i<n;i++)out[i]=s[i];out[n]=0;
 s+=n;while(*s==' ')s++;
 return s;
}
static void street_toast(void){
 const int maxWidth=735,pad=12;
 char row[130];const char*s=notice;int rows=0,width=0;
 while(*s){s=hud_line(s,maxWidth-2*pad,row);width=max(width,textw(row,0));rows++;}
 if(!rows)return;
 int h=28+(rows-1)*21,y=466-h;
 box(16,y,width+2*pad,h,DARK,0x455766);
 s=notice;while(*s){s=hud_line(s,maxWidth-2*pad,row);textat(16+pad,y+2,row,notice_color(),0);y+=21;}
}
static void objective_card(const char*title,const char*objective){
 const int pad=12,limit=594;
 char row[130];const char *lines[2]={title,objective};int rows=0,width=0;
 for(int field=0;field<2;field++){
  const char*s=lines[field];while(*s){s=hud_line(s,limit,row);width=max(width,textw(row,0));rows++;}
 }
 int h=11+21*rows,top=16+DAY_BADGE_H+5;
 box(16,top,width+2*pad,h,DARK,0x415766);rect(16,top,3,h,LIME);
 int y=top+2;
 for(int field=0;field<2;field++){
  const char*s=lines[field];while(*s){
   s=hud_line(s,limit,row);textat(16+pad,y,row,field?PAPER:LIME,0);y+=21;
  }
  y++;
 }
}
static void hud(void){
 draw_day_clock(16);
 char v[130],*a;
 const char *objective=0;
 if(job.kind>=0){
  a=append(v,jobNames[job.kind]);a=append(a,"  ");a=number(a,job.step+1);
  a=append(a,"/");a=number(a,job.total);
  if(job_is_service(job.kind))a=append(a,"  /  COMMUNITY SERVICE $0");
  else if(job.unpaid)a=append(a,"  /  UNPAID SHIFT $0");
  if(job.kind==JOB_TAXI&&taxi_occupied()){
   a=append(a,"  FARE $");a=number(a,job.price);a=append(a,"  TIP $");
   a=number(a,taxi_current_tip());
   if(taxi.copper==CAB_SEEN)a=append(a,"  COPPER +$20");
  }
  if(job_has_clock())fmt(a,"  SEC ",(int)job.time);
  objective=job.kind==JOB_TAXI?(job_vehicle_target()>=0?"Enter the marked taxi or use your own car.":taxi_objective()):job_objective();
 }
 else if(vdActive>=0){append(v,vdNames[vdActive]);objective=vdLine;}
 else if(ncr_active()){append(v,"AFTERIMAGE / SHOW RIDE");objective=ncr_stage_text();}
 else if(sv.intro==1||sv.intro==2){append(v,"FIRST SHIFT");objective="Enter Mercury Dispatch. Talk to Mara.";}
 else if(sv.intro==3){append(v,"FIRST PAYCHECK");objective=sv.owned?"M: choose another job or a city destination.":"Your first car is $125 at Nightglass Motors.";}
 if(!objective&&sv.taxiFares&&p.car>=0&&cars[p.car].owner){append(v,"NEW CALL AVAILABLE");objective="T: quick fare. Park for Q: quote / R: receipt.";}
 if(objective)objective_card(v,objective);
 box(650,16,294,79,DARK,0x415766);
 draw_wallet(650,16,294);
 rect(845,70,82,6,0x34434b);rect(845,70,p.hp*82/100,6,0x6ea7ee);
 if(p.car>=0){vehicle_condition_text(append(v,"CAR "),cars+p.car);textat(670,64,v,vehicle_hp(cars+p.car)<120?GOLD:MUTED,0);}else textat(670,64,pistol_equipped()?"PULSE":"FISTS",MUTED,0);
 int h=(p.heat>15)+(p.heat>80)+(p.heat>150);
 if(h>0){textat(761,64,"HEAT",PINK,0);for(int i=0;i<h;i++)diamond(813+i*9,76,3,PINK);}
 else if(p.heat>0)textat(761,64,"CLEARING",PINK,0);
 if(sv.armor){rect(845,80,82,3,0x34434b);rect(845,80,sv.armor*82/90,3,TEAL);}
 if(sv.charges){
  box(675,101,269,48,0x221923,0x9a486d);
  textat(688,103,"CASE HOLD",CASH_RED,0);
  revenue_time(append(v,"SWEEP "));
  textat(932-textw(v,0),103,v,GOLD,0);
  a=number(append(v,"RESTITUTION  "),sv.restitution);number(append(a," / "),RESTITUTION_SHIFTS);
  textat(688,124,v,PAPER,0);
 }
 if(atmOpen>=0)return; /* A terminal has its own action and receipt rows. */
 const char*use=job_street_prompt();
 if(!use&&p.car>=0)use=stoppingToExit?"BRAKING  /  HOLD E":absf(cars[p.car].s)>STREET_EXIT_SPEED?"HOLD E  BRAKE AND EXIT":"E  EXIT CAR";
 else if(!use){
  StreetUse target=street_use_target(by,near);
  switch(target.kind){
  case STREET_USE_VENUE:a=append(v,"E  ENTER ");append(a,loc[target.id].label);use=v;break;
  case STREET_USE_SHOP:a=append(v,"E  ENTER ");a=append(a,walkin_name(target.id));a=append(a,"   /   ");append(a,venue_address(target.id));use=v;break;
  case STREET_USE_ATM:use="E  USE ATM";break;
  case STREET_USE_NONE:use=npc_prompt();break;
  case STREET_USE_CAR:{
   const Car*c=cars+target.id;
   use=absf(c->s)>STREET_BOARD_SPEED?"WAIT FOR CAR TO SLOW":
          c->owner||c->reported?"E  GET IN":"E  STEAL VEHICLE";
   break;
  }
  }
 }
 if(use){int w=textw(use,0)+26;box((W-w)/2,487,w,30,DARK,0x566a75);center(W/2,491,use,PAPER,0);}
 if(p.noticeT>0)street_toast();
 if(saveFlash)textat(18,508,saveFlash>0?"SAVED":"SAVE ERROR",saveFlash>0?TEAL:PINK,0);
}
static void modal(int x,int y,int w,int h,U c){for(int i=0;i<W*H;i++)pixels[i]=((pixels[i]>>1)&0x007f7f7f)+0x050a0d;rect(x+8,y+8,w,h,0x080f14);box(x,y,w,h,0x101e25,0x526c73);rect(x,y,w,4,c);}
typedef struct{const char *key,*action;} GuideBinding;
static const GuideBinding guideStreet[]={
 {"WASD/ARROWS","Move or steer"},{"SHIFT","Sprint / handbrake"},
 {"E","Use / talk / board"},{"ENTER","Confirm / ride tram"},
 {"HOLD E","Brake to exit at speed"},{"X / SPACE","Punch / attack"},
 {"T / Q / R","Taxi: accept / quote / receipt"},{"TAB / V","Wheel / med patch"},
 {"SHIFT + E","Talk at shared prompt"},{"C","Switch driving view"}
},guideCareer[]={
 {"M","Open / close atlas"},{"F8","City / region in atlas"},
 {"LMB / RMB","Set / clear GPS pin"},{"P / F7","Signal (assigned key)"},
 {"L","Reload fitted tool"},
 {"F1","Field guide"},{"F2","Toggle low FX"},
 {"F3 / F4","Saves / new career"},{"F5","Save current career"},
 {"ESC / F6","Pause / city listings"}
};
static int guide_step(int count){return count>9?20:count>8?23:26;}
static void guide_column(int x,const char*title,const GuideBinding *rows,int count,int keyw){
 textat(x,153,title,TEAL,0);rect(x,177,367,1,0x39545e);
 int step=guide_step(count);
 for(int i=0;i<count;i++){
  int y=183+i*step;
  if(!(i&1))rect(x,y,367,step-3,0x162931);
  textat(x+7,y+1,rows[i].key,GOLD,0);textat(x+keyw,y+1,rows[i].action,PAPER,0);
 }
}
static void field_guide(void){
 modal(64,37,832,466,TEAL);
 textat(92,58,"PORT VANTA  /  PLAYER MANUAL",MUTED,0);
 textat(92,83,"FIELD GUIDE",PAPER,1);
 guide_column(92,"STREET CONTROLS",guideStreet,LEN(guideStreet),141);
 guide_column(500,"MAP, SIGNAL & CAREER",guideCareer,LEN(guideCareer),113);
 rect(92,395,775,1,0x39545e);
 textat(92,401,"TEAL",TEAL,0);textat(151,401,"Legal work",PAPER,0);
 textat(314,401,"PINK",PINK,0);textat(373,401,"Street contracts",PAPER,0);
 textat(92,429,"Cases: pay Revenue or finish three courier or recovery shifts.",MUTED,0);
 textat(92,463,"ENTER / ESC   RETURN",TEAL,0);
}
#include "interiors.inc"
#include "streetside_interiors.inc"
#include "profiles.inc"
typedef struct{int x,y,kind,id;} StreetSprite;
enum{SPRITE_CAR,SPRITE_PED,SPRITE_RIDER,SPRITE_HERO,SPRITE_TREE,SPRITE_ATM,SPRITE_NIGHT_RIDER};
static void add_sprite(StreetSprite*list,int*count,int x,int y,int kind,int id){
 if(x<cx-44||x>cx+W+44||y<cy-44||y>cy+H+44)return;
 int i=(*count)++;
 while(i>0&&list[i-1].y>y){list[i]=list[i-1];i--;}
 list[i]=(StreetSprite){x,y,kind,id};
}
/* Actors behind a facade can extend beyond their foot position. Restore
 * only the visible face where that sprite overlaps the baked building. */
static void cover_behind_buildings(int x,int y){
 if((U)x>=BASE_ROOM||(U)y>=BASE_ROOM)return;
 int tile=(y/256)*8+x/256;
 if((U)tile>=64)return;
 const int*row=lot[tile];
 for(int i=1;i<=row[0];i++){
  const Building*b=buildings+row[i];StreetBounds shell=building_bounds(b);
  if(y>=b->y+b->h||y+18<shell.y||x+34<shell.x||x-34>=shell.x+shell.w)continue;
  int left=max(cx,max(x-34,shell.x)),right=min(cx+W,min(x+35,shell.x+shell.w));
  int top=max(cy,max(y-49,shell.y)),end=min(cy+H,min(y+19,shell.y+shell.h));
  for(int yy=top;yy<end;yy++){
   /* The east wall slopes away from the top-right corner. */
   int edge=b->x+b->w+cl((yy-(b->y-b->ht))/2,0,b->ht/2);
   for(int xx=left;xx<min(right,edge);xx++)pixels[(yy-cy)*W+xx-cx]=city[yy*ROOM+xx];
  }
 }
}
static void night_city_actors(void){
 int door=night_club_listing();
 int count=night_club_queue(),wx=ordinaryDoorX[door],wy=ordinaryDoorY[door];
 if(count&&wx){
  int x=sx(wx),y=sy(wy);
  if((unsigned)x<W&&(unsigned)y<H){
   for(int i=0;i<count;i++){
    int px=wx-21-(i%4)*16,py=wy+16+(i/4)*13;
    if(!solid(px,py,5))person(sx(px),sy(py),i%3?0x887392:0x76a7ac,
                               (int)(sn(anim*(3+i*.4f))*2),-1.5708f,0,i);
   }
   if(count>=4){rect(x-73,y+34,50,2,0xc39870);disc(x-73,y+34,2,GOLD);disc(x-23,y+34,2,GOLD);}
  }
 }
 if(night_venue_vendor_pos(&wx,&wy)){
  int x=sx(wx)-21,y=sy(wy)+21;
  if(x>-30&&y>-30&&x<W+30&&y<H+30){
   ng_model(x,y,0,1.65f,0,4,0x884a5c,(NgMods){0,0,0,0,0,0},100,0,0);
   rect(x-21,y+15,44,2,0xdc8cb2);
   diamond(sx(wx),sy(wy),9,0xf6aada);
  }
 }
}
static void draw_street_actors(void){
 vd_cross_paint();
 /* A viewport including sprite margins covers at most 6 x 4 city blocks. */
 static StreetSprite list[LEN(cars)+LEN(peds)+3+6*4*8+LEN(bankAtms)];int count=0;
 for(int i=street_next_car(-1);i<LEN(cars);i=street_next_car(i))add_sprite(list,&count,cars[i].x,cars[i].y,SPRITE_CAR,i);
 for(int i=0;i<LEN(peds);i++)if(peds[i].hp>0||peds[i].life>0)
  add_sprite(list,&count,peds[i].x,peds[i].y,SPRITE_PED,i);
 if((job.kind==JOB_TAXI&&taxi.phase!=TAXI_RIDING)||(taxi.phase==TAXI_WALKING&&taxi.linger>0))
  add_sprite(list,&count,taxi.x,taxi.y,SPRITE_RIDER,0);
 float nx,ny;if(ncr_ped(&nx,&ny))add_sprite(list,&count,nx,ny,SPRITE_NIGHT_RIDER,0);
 if(p.car<0&&!p.dead)add_sprite(list,&count,p.x,p.y,SPRITE_HERO,0);
 for(int gy=cl((cy-44)/256,0,7);gy<=cl((cy+H+44)/256,0,7);gy++)
  for(int gx=cl((cx-44)/256,0,7);gx<=cl((cx+W+44)/256,0,7);gx++){
   const BlockScenery*block=scenery+gy*8+gx;
   for(int i=block->treesFrom;i<block->count;i++){
    StreetBounds t=block->walls[i];add_sprite(list,&count,t.x+3,t.y+3,SPRITE_TREE,(gx*79+gy*23)%11+i);
   }
  }
 for(int i=0;i<LEN(bankAtms);i++)add_sprite(list,&count,bankAtms[i].x,bankAtms[i].y,SPRITE_ATM,i);
 for(int j=0;j<count;j++){
  StreetSprite v=list[j];int x=v.x-cx,y=v.y-cy;
  switch(v.kind){
  case SPRITE_TREE:tree_crown(x,y-7,v.id);break;
  case SPRITE_ATM:atm_kiosk(x,y);break;
  case SPRITE_CAR:carbody(cars+v.id,x,y);break;
  case SPRITE_HERO:{
   U jacket=p.hit>0&&((int)(anim*20)&1)?PINK:0xc6d979;
   if(pistol_equipped()){
    PulseSprite gun=pulse_project(pulse_visible_pose(),cx,cy);
    figure_held(x,y,jacket,player_gait(),p.a,MODEL_PLAYER,3,&sv.look,&gun);
   }else person(x,y,jacket,player_gait(),p.a,MODEL_PLAYER,3);
   break;}
  case SPRITE_RIDER:{
   if(npc_replaces(N_ACT_RIDER+taxi.rider))break;
   float px=taxi.x,py=taxi.y,a=taxi.a;
   int step=taxi.phase==TAXI_WAIT?0:(int)(sn(anim*(15.f*FOOT_PACE))*5);
   npc_present_pose(N_ACT_RIDER+taxi.rider,&px,&py,&a,&step);
   person((int)px-cx,(int)py-cy,riders[taxi.rider].outfit,step,a,0,riders[taxi.rider].face);
   if(job.kind==JOB_TAXI){center(x,y-36,riders[taxi.rider].name,0xe2b3ff,0);diamond(x,y+5,8,0xb988f2);}break;}
  case SPRITE_NIGHT_RIDER:ncr_street_person(x,y);break;
  case SPRITE_PED:{
   int i=v.id;Ped*q=peds+i;if(npc_replaces(N_ACT_STREET+i))break;
   if(q->hp<=0){line(x-6,y-3,x+6,y+3,0x815763);disc(x-7,y-4,3,0x85757b);break;}
   U c=pedHit[i]>0?0xffdfbc:q->role==1?(dispatch_posted(i)?0x668df0:(((int)(anim*9)&1)?0x668df0:PINK)):q->role==2?PINK:(i%2)?0x92b7b4:0xca8c71;
   float px=q->x,py=q->y,a=q->a;
   int step=q->s<=2?0:(int)(sn(anim*FOOT_PACE*(q->role?16.f:11.f)+i)*4);
   if(npc_present_pose(N_ACT_STREET+i,&px,&py,&a,&step)&&q->role==1)c=0x668df0;
   person((int)px-cx,(int)py-cy,c,step,a,q->role,i);break;}
  }
  if(v.kind!=SPRITE_TREE&&v.kind!=SPRITE_ATM)cover_behind_buildings(v.x,v.y);
 }
}
/* The field guide is an overlay. Pass its previous view to the renderer
 * without changing the authoritative scene or resuming simulation. */
static void paint_world(int view){
 if(view==3){interior_paint();if(scene!=2)night_venues_hint();return;}
 if(view==4){
  walkin_paint();
  if(scene!=2){
   night_venues_hint();
   if(walkin_active==NIGHT_CLUB_LEGACY&&menu<0){box(695,443,249,28,0x261829,0xad6283);center(818,448,"ENTER / THE AFTERIMAGE",0xf7b5d1,0);}
  }
  return;
 }
 if(view==NIGHT_CLUB_SCENE){night_club_paint();return;}
 if(view==NIGHT_GARAGE_SCENE){night_garage_paint();return;}
 if(view==NIGHT_VENUES_SCENE){night_venues_paint();return;}
 if(view==NIGHT_TRAP_SCENE){night_trap_paint();return;}
 if(mapOn){minimap();return;}
 cx=camera_left();cy=camera_top();
 int introVenue=job.kind<0&&vdActive<0&&!ncr_active()?nav_intro_venue():-1;
 if(ch_draw())goto street_objectives;
 typedef U V4 __attribute__((vector_size(16),aligned(1),may_alias));
 for(int y=0;y<H;y++){const V4 *q=(const V4*)(city+(cy+y)*ROOM+cx);V4 *r=(V4*)(pixels+y*W);for(int x=0;x<W/4;x++)r[x]=q[x];}
 roadFX_paint(cx,cy);
 night_world_street_fx();
 night_phone_road_paint();night_trap_street_paint();
 cab_marquee_paint();
 for(int i=street_next_car(-1);i<LEN(cars);i=street_next_car(i))if(i==p.car||absf(cars[i].s)>9)projectLights(cars+i);
 draw_street_actors();outer_life_paint();night_city_actors();
 night_phone_contact_paint();npc_body_paint(cx,cy);vd_world();
 atm_kiosk_hint();
 int x=sx(p.x),y=sy(p.y);
 for(int i=0;i<LEN(fx);i++)if(fx[i].t>0)disc(sx(fx[i].x),sy(fx[i].y),2,fx[i].c);
 if(p.trail>0){
  int mx=sx(trailX),my=sy(trailY),ex=sx(trailEndX),ey=sy(trailEndY);
  line(mx,my,ex,ey,trailInk);
  if(p.trail>.064f){
   float m=sq(d2((float)(ex-mx),(float)(ey-my)));
   /* The flash brightens the muzzle, not the far side of a contact. */
   dot(mx,my,0xfff2bd);
   if(m>3.f)line(mx,my,mx+(int)((ex-mx)*3.f/m),my+(int)((ey-my)*3.f/m),0xfff2bd);
  }
 }
 if(job.kind==JOB_RIVALS&&job.step==0&&rivalsDeployed){
  int x=sx(job.xs[0]),y=sy(job.ys[0]);
  box(x-10,y-7,20,14,0x594537,0x9e8163);
  rect(x-9,y-1,18,2,0x272b30);rect(x-2,y-4,4,8,0xd0a07a);
 }
 ng_world();
 if(scene==1&&menu<0&&atmOpen<0)night_venues_fx();
 ng_cam_compose();
 street_objectives:;
 for(int i=0;i<LEN(loc);i++){const Place*l=loc+i;int x=sx(l->x),y=sy(l->y);ng_cam_point(&x,&y);if(x<-110||y<-25||x>W+110||y>H+25)continue;
  if(i==by||i==introVenue){
    int w=textw(l->label,0)+16;box(x-w/2,y-40,w,23,DARK,mix(l->tint,DARK,3));center(x,y-40,l->label,l->tint,0);
  }
 }
 /* Use map-sized type over the physical marquee, so it is legible even from
    the player's first position outside Apartment 4. */
 if(ncFrontId>=0&&p.car<0&&p.x<550&&absf(p.y-ordinaryDoorY[ncFrontId])<215){
  int x=sx(ordinaryDoorX[ncFrontId]),y=sy(ordinaryDoorY[ncFrontId])+74;
  const char*label="STRIP CLUB / THE AFTERIMAGE";int w=textw(label,0)+18;
  if(x>=w/2+4&&x<=W-w/2-4&&y>=169&&y<=H-90){
   box(x-w/2,y-25,w,24,0x271b30,0xd68eb2);center(x,y-24,label,0xffc5e0,0);
   tri(x-5,y-25,x+5,y-25,x,y-30,0xd68eb2);
  }
 }
 if(introVenue>=0){int tx=sx(loc[introVenue].x),ty=sy(loc[introVenue].y);ng_cam_point(&tx,&ty);if(tx>=750&&ty>=333)tx=730;mark(cl(tx,27,W-27),cl(ty,150,H-70),16+(int)(sn(anim*4)*3),LIME);}
 if(vdActive>=0&&vdGoalX>=0){
  int gx=sx(vdGoalX),gy=sy(vdGoalY);ng_cam_point(&gx,&gy);
  if(gx>=25&&gx<W-25&&gy>125&&gy<H-65)diamond(gx,gy,12,TEAL);
  else{int ax=cl(gx,29,W-30),ay=cl(gy,138,H-80);if(ax>730&&ay>365)ax=730;mark(ax,ay,10,TEAL);}
 }
 if(job.kind>=0){
  int ride=job_vehicle_target();
  NavPoint target=nav_job_goal();
  int x=sx(target.x),y=sy(target.y);ng_cam_point(&x,&y);
  U color=ride>=0?GOLD:job.kind<4?TEAL:PINK;
  if(job.kind==JOB_SPRINT&&ride<0&&x>=30&&x<W-30&&y>=82&&y<H-60){
   int leg=job.step?job.step-1:0,next=job.step?job.step:1;
   int horizontal=absf(job.xs[next]-job.xs[leg])>absf(job.ys[next]-job.ys[leg]);
   for(int stripe=-22;stripe<22;stripe+=5){
    U ink=((stripe+22)/5)%2?DARK:PAPER;
    rect(horizontal?x-2:x+stripe,horizontal?y+stripe:y-2,horizontal?4:5,horizontal?5:4,ink);
   }
   center(x,y-42,job.step==0?"START":job.step==job.total-1?"FINISH":"GATE",GOLD,0);
  }else if(x<30||x>W-30||y<82||y>H-60){
   int ax=cl(x,30,W-35),ay=cl(y,105,H-74);
   // Keep an off-screen marker away from the wallet and minimap.
   if(ax>=630&&ay<165)ay=165;
   if(ax>=750&&ay>=375)ax=750;
   mark(ax,ay,12,color);
  }else if(ride>=0){
   diamond(x,y-18,8,color);
  }else if(job.step==1&&(job.kind==JOB_RECOVERY||job.kind==JOB_HOT)){
   line(x-23,y-11,x+23,y-11,color);line(x-23,y+11,x+23,y+11,color);
   diamond(x,y-20,6,color);
  }else if(job_venue()>=0||job_room()>=0)diamond(x,y,9,color);
  else mark(x,y,14+(int)(sn(anim*4)*4),color);
 }
 int phoneX,phoneY;
 if(ncr_target(&phoneX,&phoneY)){
  int px=sx(phoneX),py=sy(phoneY);ng_cam_point(&px,&py);
  if(px>=34&&px<=W-34&&py>=97&&py<=H-70&&!(px>=750&&py>=333)){
   box(px-21,py-9,43,18,0x29202d,0xf18acb);diamond(px,py,8,0xf18acb);
  }else {int ax=cl(px,31,W-32),ay=cl(py,148,H-72);if(ax>=750&&ay>=333)ax=730;mark(ax,ay,8,0xf18acb);}
 }else if(job.kind<0&&vdActive<0&&night_phone_route(&phoneX,&phoneY)){
  int px=sx(phoneX),py=sy(phoneY);ng_cam_point(&px,&py);
  if(px>=34&&px<=W-34&&py>=97&&py<=H-70&&!(px>=750&&py>=333))diamond(px,py,10,0xff9adb);
  else {int ax=cl(px,31,W-32),ay=cl(py,148,H-72);if(ax>=750&&ay>=333)ax=730;mark(ax,ay,8,0xff9adb);}
 }
 hud();night_world_hud();minimap();ng_gate_overlay();ch_badge();night_trap_hint();district_notice_paint();
 if(scene==1&&menu<0&&p.car<0&&!p.dead&&job.kind<0&&vdActive<0&&!vdPanel&&atmOpen<0){
  int x,y;if(night_venue_vendor_pos(&x,&y)&&d2(p.x-x,p.y-y)<42*42){
   box(317,486,326,29,DARK,0xf6aada);center(W/2,489,"E   NIGHT SIGNAL VAN",0xf6aada,0);
  }
 }
 if(scene==2)return; /* No second modal underneath the guide. */
 if(p.dead){modal(240,149,480,212,PINK);textat(268,171,"DOWN IN VANTA",PINK,1);textat(269,231,sv.home?"Recover at your Breakwater safehouse.":"Tideway Clinic charges up to $35 for recovery.",PAPER,0);textat(269,284,"ENTER  /  RECOVER",LIME,0);return;}
 if(menu>=0){paint_active_menu();draw_wallet(650,16,294);}
 if(atmOpen>=0)atm_draw();
}
#include "weapon_wheel.inc"
void paint(void){
 if(scene==0||scene==5){profiles_paint();return;}
 if(scene==NIGHT_PHONE_SCENE){night_phone_paint();return;}
 if(scene==2){paint_world(helpBack);field_guide();return;}
 paint_world(scene);vd_draw();
 bank_warning_draw();
 npc_panel_paint();
 taxi_draw_radio();
 if(wheelOpen)wheel_paint();
}

#include "verdict_activities.inc"
#include "npc_encounter.inc"
#include "night_world.inc"
#if __has_include("night_club.inc")
#include "night_club.inc"
#else
static int night_club_open(int b){(void)b;return 0;}
static int ncr_active(void){return 0;}
static int ncr_target(int*x,int*y){(void)x;(void)y;return 0;}
static int ncr_ped(float*x,float*y){(void)x;(void)y;return 0;}
static void ncr_reset(void){}
static void ncr_street_tick(void){}
static void ncr_street_person(int x,int y){(void)x;(void)y;}
static const char*ncr_stage_text(void){return "";}
static const char*ncr_stop_name(void){return "";}
static void night_club_tick(U k,U t){(void)k;(void)t;}
static void night_club_paint(void){}
static int night_club_track(void){return 0;}
static int night_club_queue(void){return 0;}
static int night_club_signature_demand(int n){(void)n;return -1;}
static void night_club_business_tick(void){}
static void night_club_pointer(int x,int y,U c){(void)x;(void)y;(void)c;}
#endif
#if __has_include("night_garage.inc")
#include "night_garage.inc"
#else
static int night_garage_open(int b){(void)b;return 0;}
static void night_garage_tick(U k,U t){(void)k;(void)t;}
static void night_garage_paint(void){}
static int night_garage_audio(void){return 0;}
static void ng_reset(void){}
static void night_garage_street(U tap){(void)tap;}
static void ng_world(void){}
static void ng_gate_overlay(void){}
static int night_garage_meet_site(int*x,int*y){(void)x;(void)y;return 0;}
static void ng_schedule_tick(void){}
static NgMods ng_installed(int type){(void)type;return (NgMods){0};}
static int ng_power(const Car*c){(void)c;return 0;}
static int ng_grip(const Car*c){(void)c;return 0;}
static int ng_route_goal(int*x,int*y){(void)x;(void)y;return 0;}
#endif
#if __has_include("night_phone.inc")
#include "night_phone.inc"
#else
static int daily_rewards_tick(int closed,float dt){(void)closed;(void)dt;return 0;}
static void night_phone_open(void){}
static void night_phone_tick(U k,U t){(void)k;(void)t;}
static void night_phone_paint(void){}
static void night_phone_pointer(int x,int y,U c){(void)x;(void)y;(void)c;}
static void night_phone_world_tick(float dt){(void)dt;}
static int night_phone_toggle(U tap){(void)tap;return 0;}
static int night_phone_action(U tap){(void)tap;return 0;}
static int night_phone_radio(void){return 0;}
static int night_phone_premium(int consume){(void)consume;return 0;}
static int night_phone_signature(int design,int consume){(void)design;(void)consume;return 0;}
static int night_phone_take_link(void){return 0;}
static void night_phone_link_result(int ok){(void)ok;}
static void night_phone_rest(int hours){(void)hours;}
static void night_phone_road_paint(void){}
static void night_phone_contact_paint(void){}
static int night_phone_route(int*x,int*y){(void)x;(void)y;return 0;}
static int night_phone_heat_at(int x,int y){(void)x;(void)y;return 0;}
static void night_phone_heat_add(int x,int y,int amount){(void)x;(void)y;(void)amount;}
static int np_closed(void){return 0;}
static int night_phone_lane_stop(float x,float y,float a,float b,int radius){(void)x;(void)y;(void)a;(void)b;(void)radius;return 0;}
#endif
#include "night_trap.inc"
#if __has_include("night_venues.inc")
#include "night_venues.inc"
#else
static int night_venues_open(int b){(void)b;return 0;}
static void night_venues_tick(U k,U t){(void)k;(void)t;}
static void night_venues_paint(void){}
static int night_venues_entry(U tap){(void)tap;return 0;}
static void night_venues_hint(void){}
static void night_venues_world_tick(void){}
static void night_venues_fx(void){}
static int night_venue_vendor_stop(void){return -1;}
static int night_venue_apply_pulse(int*a,int*b,float*c,U*d){(void)a;(void)b;(void)c;(void)d;return 1;}
static void nvk_stats(int*a,int*b,float*c,U*d){(void)a;(void)b;(void)c;(void)d;}
static void night_venue_reload(U tap){(void)tap;}
static int night_venue_vendor_pos(int*x,int*y){(void)x;(void)y;return 0;}
static int night_venue_vendor_report(int*x,int*y,int*t){*x=*y=*t=0;return 0;}
static int night_venue_vendor_entry(U tap){(void)tap;return 0;}
static float night_venues_walk_factor(void){return 1.f;}
#endif
static int night_club_entry(U tap){return scene==4&&walkin_active==NIGHT_CLUB_LEGACY&&menu<0&&(tap&ENTER)&&night_club_open(walkin_active);}
/* The phone module never opens websites itself. The host consumes one
 * explicitly confirmed intent, with only the known official site allowed. */
int take_phone_link(void){return night_phone_take_link();}
void phone_link_result(int ok){night_phone_link_result(ok);}
int night_music_track(void){
 if(scene==NIGHT_CLUB_SCENE){int s=cl(night_club_track(),0,3);return s?2+s:0;}
 int station=cl(night_phone_radio(),0,3);
 if(!station||scene==0||scene==5||p.dead)return 0;
 if(p.car>=0)return 5+station+(night_garage_audio()>0?3:0);
 return night_garage_audio()==2?11+station:0;
}
int exit_save_ok(void){
 int original=scene;
 if(original==0||original==5){
  if(!profileReturn||saveFlash>=0)return 1;
  scene=profileReturn;
 }
 if(p.dead){scene=original;return 1;}
 request_save();
 scene=original;
 return saveFlash>=0;
}



#include "outer_life.inc"
#include "chase_view.inc"
/* Optional platform timing signpost. The regular paint path never calls
 * this, and the host draws it only while the player has opted in. */
void frame_probe_banner(const char*label,U ink){
 int w=textw(label,0)+16,x=(W-w)/2;
 U*old=surface;int aw=rw,ah=rh,l=clipLeft,t=clipTop,r=clipRight,b=clipBottom;
 surface=pixels;rw=W;rh=H;clipLeft=clipTop=0;clipRight=W;clipBottom=H;
 box(x,0,w,22,0x101e25,0x39545e);textat(x+8,1,label,ink,0);
 surface=old;rw=aw;rh=ah;clipLeft=l;clipTop=t;clipRight=r;clipBottom=b;
}
