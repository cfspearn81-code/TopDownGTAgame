#include "game.h"
#include "font.inc"
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
#define ROOM 2048
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
static int ordinaryDoorX[LEN(buildings)],ordinaryDoorY[LEN(buildings)];
static void setup_ordinary_doors(void);
static int zone_is_building(int i);
static const char *walkin_name(int i);
static void walkin_open(int building);
static int contract_room(int kind,int x,int y,int exclude);
static void walkin_tick(U keys,U tap);
static void walkin_paint(void);
static void bake_scenery(void);
static void zone_venues(void);
static void navigation_input(U tap);
static void navigation_pointer(int sx,int sy,U clicks);
static void open_interior(int loc_index);
static void interior_tick(U keys,U tap);
static int lantern_terminal(void);
static void interior_paint(void);
typedef struct{int x,y;const char *label,*npc,*note;U tint;int kind;} Place;
static const Place originalLoc[]={
{256,512,"MERCURY DISPATCH","Mara Finch","Legal work: delivery routes and taxi fares.",TEAL,0},
{512,768,"VAULT 39","Ivo Slate","Optional contracts from the city's underworld.",PINK,1},
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
static const char* district(int x,int y){if(y<545)return "GLASS MILE";if(y>1160)return x<900?"LANTERN WARD":"BREAKWATER";return x<850?"TIDE MARKET":x<1400?"NIGHTGLASS":"EAST FREIGHT";}
typedef struct{const char*name;int price;float vmax,acc,turn;U c;} CarSpec;
static const CarSpec specs[]={
{"MOTE",125,151,104,2.8f,0x59bbad},{"VECTRA",360,193,145,2.3f,0x94b2f5},{"PALLAS",620,167,110,1.8f,0xeab052},{"NIGHTJAR",970,237,165,2.7f,0xdf648f},{"COURIER 4",480,174,112,2.95f,0xbbd377}
};
typedef struct{float x,y,a,s;int type,owner,cond,reported;}Car;
typedef struct{float x,y,a,s;int hp,role,next;float life,cool;}Ped;
static Car cars[42];static int nc;
static Ped peds[52];
typedef struct{float x,y,vx,vy,t;U c;} Particle;
static Particle fx[100];static int fn;
typedef struct{int own,crew,tier,product,durationMs,batches;float timer;} Business;
typedef struct{int owned;float elapsed,cooldown;int pending;} LegalBusiness;
#include "character.inc"
typedef struct{U magic,rev;int cash,hp,armor,gear,units,owned,selection,legal,crime,sales,arrests,vehicleCond,home;float x,y;Business b[2];int stocks[3],dirty,pending,audit,charges,restitution,legacyRivalCooldown,protect;float clearT,auditT;U intro;int carCond[5];U taxiFares;LegalBusiness legit[3];float dayClock;U playedTicks;Look look;unsigned char legacyClock,reserved;char name[20];U hash;} Save;
#define LEGACY6_SIZE (__builtin_offsetof(Save,playedTicks)+4)
static Save sv;
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
typedef struct{int kind,step,total,price;float time;int xs[6],ys[6];}Job;
static Job job={.kind=-1};
static int jobRide=-1;
enum{TAXI_WAIT,TAXI_BOARDING,TAXI_RIDING,TAXI_EXITING,TAXI_WALKING};
typedef struct{float x,y,a,still,linger;int phase,ride,pickup,dropoff,rider,curb[2],meters,crashes,damage,rough;float roughHold,roughCooldown;} Taxi;
static Taxi taxi;
static int lastTaxiPickup=-1,lastTaxiDrop=-1,lastTaxiRider=-1;
static U taxiSeed=0xc3b11d19u;
static U taxi_random(void){taxiSeed^=taxiSeed<<13;taxiSeed^=taxiSeed>>17;taxiSeed^=taxiSeed<<5;return taxiSeed;}
void set_session_seed(U seed){taxiSeed=(seed^0xc3b11d19u)|1u;}
static const struct {const char*name;U outfit;int face;} riders[]={
 {"MIA",0xd5a9eb,2},{"ROWAN",0x91e6ce,1},{"JULES",0xff9e92,4},{"ARI",0x9bbdff,0},
 {"SASHA",0xfcd37e,3},{"DEV",0xbcafd9,2},{"IMANI",0xef90c8,1},{"NOAH",0xaad482,4},
 {"ELI",0xe8b58b,0},{"REN",0x8ad7ee,3},{"SAM",0xd5c89f,2},{"LEO",0xe4a082,4}
};
typedef struct{float x,y,a,heat,quiet,hit,shot,secs,noticeT,trail;int car,hp,dead,seen;} Player;
static Player p;
enum{PAUSE_MENU=12};
static int scene=0,menu=-1,sel=0,mapOn=0,wason=0,cx,cy,deadAnim,saveFlash,fastGfx,sideSlot;static float crashFlash;
static float anim,footcycle,punchFlash,pedHit[LEN(peds)],shelterT;static int walking;
static float cameraLeadX,cameraLeadY;
static void update_camera_lead(void){
 float vx=0,vy=0;
 if(p.car>=0){
  const Car *car=&cars[p.car];
  vx=cs(car->a)*car->s*.48f;
  vy=sn(car->a)*car->s*.34f;
 }
 cameraLeadX+=(vx-cameraLeadX)*.055f;
 cameraLeadY+=(vy-cameraLeadY)*.055f;
}
static int camera_left(void){return cl((int)(p.x+cameraLeadX)-W/2,0,ROOM-W);}
static int camera_top(void){return cl((int)(p.y+cameraLeadY)-H/2,0,ROOM-H);}
static U prev;
static int by=-1,near=-1,lastCar=-1,helpBack=1;
#define CASH_GREEN 0x85F36Bu
#define CASH_RED   0xFF535Cu
static char notice[100];
static U noticeInk;
static U notice_color(void){return noticeInk?noticeInk:PAPER;}
static void tell(const char*s){append(notice,s);p.noticeT=4.4f;noticeInk=0;}
static void wallet_note(const char*s,int a,int clean){
 char*q=append(notice,s);q=append(q,clean?"  CLEAN +$":"  DIRTY MONEY +$");number(q,a);
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
static int spend(int x){if(sv.cash<x){tell("Not enough credits.");return 0;}sv.cash-=x;return 1;}
static void ground(int x,int y,int w,int h,U c){for(int j=max(0,y);j<min(ROOM,y+h);j++)for(int i=max(0,x);i<min(ROOM,x+w);i++)city[j*ROOM+i]=c;}
static int solid(float x,float y,float r){if(x<35||x>ROOM-36||y<35||y>ROOM-36)return 1;int* a=lot[((int)y>>8)*8+((int)x>>8)];for(int i=0;i<a[0];i++){Building *b=buildings+a[i+1];if(x+r>b->x&&x-r<b->x+b->w&&y+r>b->y&&y-r<b->y+b->h)return 1;}return 0;}
static int open_space(float x,float y){return !solid(x,y,10);}
static void move(float *x,float*y,float dx,float dy,float r){if(!solid(*x+dx,*y,r))*x+=dx;if(!solid(*x,*y+dy,r))*y+=dy;}
static void particle(float x,float y,int n,U c){while(n--){Particle*q=fx+(fn++%LEN(fx));q->x=x;q->y=y;q->vx=(int)(rnd()%151)-75;q->vy=(int)(rnd()%151)-75;q->t=.2f+(rnd()%36)*.01f;q->c=c;}}
static void setupCity(void){
 rng=0x873f24c7u;nb=0;z(lot,sizeof lot);cp(loc,originalLoc,sizeof loc);for(int y=0;y<ROOM;y++)for(int x=0;x<ROOM;x++){int xx=x%256,yy=y%256,n=(int)(rnd()%12);U c;if(xx<29||xx>=227||yy<29||yy>=227){c=RGB(24+n,34+n,43+n);if((xx==0&&yy>40&&yy<219&&yy%35<17)||(yy==0&&xx>40&&xx<219&&xx%35<17))c=0x90845a;}else if(xx<42||xx>214||yy<42||yy>214)c=RGB(48+n,55+n,60+n);else c=RGB(32+n,44+n,47+n);if(x>1390&&y>1110)c=mix(c,0x264044,2);else if(y<550)c=mix(c,0x4a4258,2);else if(x<850)c=mix(c,0x54433e,2);if(x<25||y<25||x>ROOM-26||y>ROOM-26)c=RGB(11+n/2,31+n,40+n);city[y*ROOM+x]=c;}
 for(int iy=0;iy<8;iy++)for(int ix=0;ix<8;ix++){
  int tx=ix*256,ty=iy*256,kind=(int)(rnd()%8);
  if(kind==0||kind==2){ground(tx+47,ty+47,161,161,0x273b35);for(int j=0;j<22;j++){int px=tx+52+rnd()%149,py=ty+52+rnd()%149;ground(px,py,5,5,(j%2)?0x3e5848:0x344d3c);}ground(tx+48,ty+123,162,7,0x566260);ground(tx+123,ty+48,7,160,0x566260);continue;}
  int n=1+(kind%3!=0);for(int k=0;k<n;k++){lot[iy*8+ix][k+1]=nb;lot[iy*8+ix][0]=n;Building*b=buildings+nb++;b->x=tx+53+k*88+rnd()%9;b->y=ty+70+rnd()%29;b->w=min(n==1?130:68,tx+200-b->x);b->h=min(65+(int)(rnd()%43),ty+210-b->y);b->ht=9+rnd()%22;static const U roofs[]={0x485b66,0x52606d,0x584867,0x596157,0x6a555c,0x4c6263};b->roof=roofs[rnd()%LEN(roofs)];b->wall=mix(b->roof,DARK,4);ground(b->x-4,b->y-4,b->w+17,b->h+16,0x1a262d);}
 }
 for(int i=1;i<8;i++)for(int j=1;j<8;j++){int x=i*256,y=j*256;for(int k=-21;k<22;k+=6){ground(x+k,y-38,3,8,0x8a9988);ground(x+k,y+31,3,8,0x8a9988);ground(x-38,y+k,8,3,0x8a9988);ground(x+31,y+k,8,3,0x8a9988);}ground(x-29,y-29,5,5,0x669686);ground(x+25,y+25,5,5,0x669686);}
 zone_venues();setup_ordinary_doors();
}
#include "street_life.inc"
static void dispatch_reset(void);
static void spawnCars(void){street_spawn();dispatch_reset();}
static U hashbytes(unsigned n){const unsigned char *a=(const unsigned char*)&sv;U h=2166136261u;for(unsigned i=0;i<n;i++)h=(h^a[i])*16777619u;return h;}
static U savehash(void){return hashbytes(sizeof(Save)-4);}
static int valid_save_core(void){
 if(sv.hp<0||sv.hp>100||sv.armor<0||sv.armor>90||sv.units<0||sv.units>1000000000||sv.legal<0||sv.legal>1000000000||sv.crime<0||sv.crime>1000000000||sv.sales<0||sv.sales>1000000000||sv.home<0||sv.home>2000000001)return 0;
 if(!(sv.auditT>=-1.f&&sv.auditT<=60.f)||!(sv.clearT>=-1.f&&sv.clearT<=1000.f))return 0;
 if(sv.pending<0||sv.pending>1000000||sv.restitution<0||sv.restitution>3)return 0;
 for(int i=0;i<3;i++)if(sv.stocks[i]<0||sv.stocks[i]>500000000)return 0;
 if(!(sv.x>=35&&sv.y>=35&&sv.x<2013&&sv.y<2013)||sv.selection<0||sv.selection>=LEN(specs)||sv.gear< -1||sv.gear>1||sv.owned<0||sv.owned>=1<<LEN(specs)||sv.cash<0||sv.dirty<0)return 0;
 for(int i=0;i<2;i++){Business*b=sv.b+i;if(b->product<0||b->product>=LEN(prod)||b->tier<0||b->tier>3||b->crew<0||b->crew>2||b->own<0||b->own>1||(b->own&&b->tier<1)||!(b->timer>=-1.f&&b->timer<=7200.f)||b->batches<0||b->batches>1000000000)return 0;}
 return 1;
}
static int valid_ledger_save(void){
 if(!(sv.dayClock>=0.f&&sv.dayClock<480.f))return 0;
 for(int i=0;i<LEN(sv.legit);i++){
   const LegalBusiness*b=sv.legit+i;
   if(b->owned<0||b->owned>1||b->pending<0||b->pending>480||!(b->elapsed>=0.f&&b->elapsed<121.f)||!(b->cooldown>=0.f&&b->cooldown<=960.f))return 0;
 }
 return 1;
}
static void legacy_profile(void){
 sv.playedTicks=0;sv.look=defaultLook;sv.legacyClock=1;sv.reserved=0;z(sv.name,sizeof sv.name);
}
static void seal_loaded(void){sv.rev=7;sv.hash=savehash();wason=1;}
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
 legacy_profile();seal_loaded();
}
static void migrate_old_save(void){
 for(int i=0;i<LEN(specs);i++)sv.carCond[i]=100;
 sv.carCond[sv.selection]=cl(sv.vehicleCond,0,100);sv.taxiFares=0;migrate_economy();
}
#ifndef DC_TEST
extern int disk_read(void*,unsigned);extern int disk_write(const void*,unsigned);
#ifdef _WIN_NATIVE
extern void disk_slot(int);
extern int disk_present(void);
extern int disk_delete(void);
extern int disk_create(const void*,unsigned);
#endif
#endif
static void starter_save(void){
 z(&sv,sizeof sv);sv.magic=0x44433131;sv.rev=7;sv.hp=100;sv.gear=-1;sv.look=defaultLook;
 sv.x=loc[12].x;sv.y=loc[12].y;sv.vehicleCond=100;for(int i=0;i<LEN(specs);i++)sv.carCond[i]=100;sv.b[1].product=2;
}
static int name_ok(const char*n){for(int i=0;i<20;i++){if(!n[i])return 1;if(n[i]<32||n[i]>126)return 0;}return 0;}
void load_save(void){
 wason=0;z(&sv,sizeof sv);
#ifndef DC_TEST
 if(disk_read(&sv,sizeof sv)&&sv.magic==0x44433131&&sv.rev==7&&sv.hash==savehash()&&valid_save_core()&&valid_ledger_save()&&sv.intro<=4&&look_valid(&sv.look)&&sv.legacyClock<=1&&!sv.reserved&&name_ok(sv.name)){wason=1;return;}
 if(disk_read(&sv,LEGACY6_SIZE)&&sv.magic==0x44433131&&sv.rev==6&&sv.playedTicks==hashbytes(LEGACY6_SIZE-4)&&valid_save_core()&&valid_ledger_save()&&sv.intro<=4){legacy_profile();seal_loaded();return;}
 if(disk_read(&sv,LEGACY6_SIZE-52)&&sv.magic==0x44433131&&sv.rev==5&&(U)sv.legit[0].owned==hashbytes(LEGACY6_SIZE-56)&&valid_save_core()&&sv.intro<=4){migrate_economy();return;}
 if(disk_read(&sv,LEGACY6_SIZE-56)&&sv.magic==0x44433131&&sv.rev==4&&sv.taxiFares==hashbytes(LEGACY6_SIZE-60)&&valid_save_core()&&sv.intro<=4){sv.taxiFares=0;migrate_economy();return;}
 if(disk_read(&sv,LEGACY6_SIZE-76)&&sv.magic==0x44433131&&sv.rev==3&&(U)sv.carCond[0]==hashbytes(LEGACY6_SIZE-80)&&valid_save_core()&&sv.intro<=4){migrate_old_save();return;}
 if(disk_read(&sv,LEGACY6_SIZE-80)&&sv.magic==0x44433131&&sv.rev==2&&sv.intro==hashbytes(LEGACY6_SIZE-84)&&valid_save_core()){sv.intro=4;migrate_old_save();return;}
#endif
 starter_save();
}
void request_save(void){if(scene==0||scene==5||p.dead)return;sv.x=p.x;sv.y=p.y;sv.hp=p.hp;sv.hash=savehash();
#ifndef DC_TEST
 saveFlash=disk_write(&sv,sizeof sv)?200:-200;
#else
 saveFlash=200;
#endif
}
static void profiles_init(void);
static void profiles_open(int create);
static void profiles_tick(U tap);
static void profiles_pointer(int sx,int sy,U clicks);
static void profiles_paint(void);
static void spawnPlayer(void){
 cameraLeadX=cameraLeadY=0;
 p=(Player){0};p.x=sv.x;p.y=sv.y;
 if(!open_space(p.x,p.y)){
   int xx=cl(((int)p.x+128)/256,1,7)*256,yy=cl(((int)p.y+128)/256,1,7)*256;
   p.x=xx;p.y=yy;
 }
 p.hp=cl(sv.hp,15,100);p.car=-1;cars[0].type=sv.selection;cars[0].cond=sv.owned?cl(sv.carCond[sv.selection],0,100):0;
}
void init_city(void){setupCity();bake_scenery();spawnCars();load_save();spawnPlayer();scene=0;menu=-1;mapOn=0;shelterT=0;job.kind=jobRide=-1;lastCar=near=by=-1;lastTaxiPickup=lastTaxiDrop=lastTaxiRider=-1;profiles_init();}
static void heat(float n){p.heat+=n;if(p.heat>300)p.heat=300;p.quiet=8.0f;}
static void health(int n){
 if(p.hit>0)return;
 if(sv.armor){int a=min(sv.armor,n);sv.armor-=a;n-=a;}
 p.hp=max(0,p.hp-n);p.hit=.32f;
 if(!p.hp){
  if(p.car>=0)cars[p.car].s=0;
  p.dead=1;menu=-1;deadAnim=0;particle(p.x,p.y,26,PINK);
 }
}
typedef struct {const char *n,*note;int action;U ink;} Option;
static Option opt[14];static int nopt;
static char strings[14][100];
static int canDirtyPurchase(int cost){
 if(sv.charges&&cost>=240){tell("Case hold: major purchases frozen. Visit Revenue.");return 0;}
 if(sv.cash>=cost){sv.cash-=cost;return 1;}
 if((long long)sv.cash+sv.dirty<cost){tell("Not enough credits.");return 0;}
 if(cost>=220||sv.dirty>=400){if(!sv.audit){sv.audit=1;sv.auditT=8;tell("AUDIT FLAG. Revenue officers en route.");request_save();}else tell("Purchase blocked by Revenue.");return 0;}
 sv.dirty-=cost-sv.cash;sv.cash=0;return 1;
}
static int credit_wallet(int legal,int amount){
 int *which=legal?&sv.cash:&sv.dirty;
 int accepted=min(max(0,amount),2147483647-*which);
 *which+=accepted;return accepted;
}
static void gain(int legal,int amount,const char *label){wallet_note(label,credit_wallet(legal,amount),legal);}
static void active(int m){menu=m;nopt=0;sel=0;}
static void add_colored(const char*n,const char*note,int action,U ink){if(nopt>=LEN(opt))return;opt[nopt++]=(Option){n,note,action,ink};}
static void add(const char*n,const char*note,int action){add_colored(n,note,action,0);}
static void priced(const char*n,int price,const char*note,int action){char*d=append(strings[nopt],n);fmt(d,"  $",price);add(strings[nopt],note,action);}
#include "legal_business.inc"
static int carNear(void){return street_near_car();}
static int placeNear(void){float best=45*45;int chosen=-1;for(int i=0;i<LEN(loc);i++){float d=d2(loc[i].x-p.x,loc[i].y-p.y);if(d<best){chosen=i;best=d;}}return chosen;}
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
static int ownedJob(void){if(job.kind>=0){tell("Finish or abandon your current job first.");return 1;}return 0;}
static int taxi_curb(int where,int *x,int *y,int *curb){
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
static int taxi_occupied(void){return job.kind==1&&(taxi.phase==TAXI_RIDING||taxi.phase==TAXI_EXITING)&&p.car==taxi.ride;}
static void taxi_quality(float dt,float before,float angleBefore,int impact){
 if(!taxi_occupied())return;
 if(impact>0){taxi.crashes++;taxi.damage+=impact;taxi.roughHold=0;taxi.roughCooldown=.7f;return;}
 Car*c=cars+p.car;
 taxi.roughCooldown-=dt;
 if(d2(p.x-job.xs[1],p.y-job.ys[1])<115.f*115.f){taxi.roughHold=0;return;}
 int sudden=before>98.f&&before-absf(c->s)>8.5f;
 int corner=before>110.f&&absf(wrap(c->a-angleBefore))>.039f;
 if(corner)taxi.roughHold+=dt;else taxi.roughHold=0;
 if((sudden||taxi.roughHold>.13f)&&taxi.roughCooldown<=0){taxi.rough++;taxi.roughCooldown=1.f;taxi.roughHold=0;}
}
static int taxi_offer(void){
 struct{int px,py,from,dx,dy,to,fromCurb,toCurb;} best={0};
 int chosen=0;
 for(int i=0;i<LEN(loc);i++){
  if(i==lastTaxiPickup||i==lastTaxiDrop)continue;
  int x,y,curb;if(!taxi_curb(i,&x,&y,&curb))continue;
  if(d2(p.x-x,p.y-y)<150*150)continue;
  for(int j=0;j<LEN(loc);j++){
   if(j==i||j==lastTaxiDrop||j==lastTaxiPickup)continue;
   int qx,qy,other;if(!taxi_curb(j,&qx,&qy,&other))continue;
   float dx=(float)(qx-x),dy=(float)(qy-y);
   if(d2(dx,dy)<350*350)continue;
   if(taxi_random()%(++chosen)==0){
    best.px=x;best.py=y;best.from=i;best.dx=qx;best.dy=qy;best.to=j;best.fromCurb=curb;best.toCurb=other;
   }
  }
 }
 if(!chosen)return 0;
 int rider=(int)(taxi_random()%(LEN(riders)-1));if(lastTaxiRider>=0){if(rider>=lastTaxiRider)rider++;}
 else rider=(int)(taxi_random()%LEN(riders));
 lastTaxiPickup=best.from;lastTaxiDrop=best.to;lastTaxiRider=rider;
 jobRide=-1;
 int distance=taxi_road_distance(best.px,best.py,best.dx,best.dy);
 job=(Job){1,0,2,60+distance/16,145.f+distance*.021f,{best.px,best.dx},{best.py,best.dy}};
 taxi=(Taxi){.x=best.px,.y=best.fromCurb,.a=1.570796f,.phase=TAXI_WAIT,.ride=-1,.pickup=best.from,.dropoff=best.to,.rider=rider,.curb={best.fromCurb,best.toCurb},.meters=distance};
 return 1;
}
static void next_fare(void){
 if(job.kind>=0){tell("Finish the current job first.");return;}
 if(!sv.taxiFares){tell("Complete a Taxi Fare from Mercury Dispatch first.");return;}
 if(p.car<0||!cars[p.car].owner){tell("Use an owned or licensed car for taxi calls.");return;}
 if(!taxi_offer()){tell("No nearby fare available yet.");return;}
 char msg[100];char*q=append(msg,"New fare: pick up ");q=append(q,riders[taxi.rider].name);q=append(q," near ");q=append(q,loc[taxi.pickup].label);append(q,".");
 tell(msg);
}
#include "jobs.inc"
static int taxi_walk_to(float x,float y,float dt){
 float dx=x-taxi.x,dy=y-taxi.y,len=sq(d2(dx,dy)),v=93*dt;
 if(len<=v){taxi.x=x;taxi.y=y;return 1;}
 taxi.a=look(dx,dy);taxi.x+=dx*v/len;taxi.y+=dy*v/len;return 0;
}
static const char* taxi_objective(void){
 if(taxi.phase==TAXI_WAIT)return "Pick up the passenger. Pull over and stop.";
 if(taxi.phase==TAXI_BOARDING)return "Passenger boarding. Hold still.";
 if(taxi.phase==TAXI_RIDING)return "Passenger on board. Follow GPS to the drop-off.";
 return "Passenger getting out. Hold still.";
}
static void taxi_tick(float dt){
 if(taxi.phase==TAXI_WALKING&&taxi.linger>0){
   taxi.linger-=dt;if(taxi_walk_to(loc[taxi.dropoff].x,loc[taxi.dropoff].y,dt))taxi.linger=0;
 }
 if(job.kind!=1||p.dead)return;
 if(taxi.phase>=TAXI_RIDING&&p.car!=taxi.ride){job.kind=-1;taxi.linger=0;tell("Fare cancelled: you left the taxi.");return;}
 int nearStop=d2(p.x-job.xs[job.step],p.y-job.ys[job.step])<37*37&&absf(p.y-(job.ys[job.step]+13))<=22;
 int stopped=p.car>=0&&cars[p.car].owner&&absf(cars[p.car].s)<5;
 switch(taxi.phase){
 case TAXI_WAIT:
   taxi_walk_to(job.xs[0],taxi.curb[0],dt);
   if(nearStop&&stopped){if((taxi.still+=dt)>.27f){taxi.phase=TAXI_BOARDING;taxi.ride=p.car;tell("Passenger boarding.");}}
   else taxi.still=0;
   break;
 case TAXI_BOARDING:
   if(!nearStop||!stopped||p.car!=taxi.ride){taxi.phase=TAXI_WAIT;taxi.still=0;break;}
   if(taxi_walk_to(p.x,p.y,dt)){taxi.phase=TAXI_RIDING;taxi.still=0;advanceJob();char msg[90];char*q=append(msg,"Passenger on board. Next stop: ");append(q,loc[taxi.dropoff].label);tell(msg);}
   break;
 case TAXI_RIDING:
   if(nearStop&&stopped){if((taxi.still+=dt)>.27f){taxi.phase=TAXI_EXITING;taxi.x=p.x;taxi.y=p.y;tell("Passenger getting out.");}}
   else taxi.still=0;
   break;
 case TAXI_EXITING:
   if(!nearStop||!stopped){taxi.phase=TAXI_RIDING;taxi.still=0;break;}
   if(taxi_walk_to(job.xs[1],taxi.curb[1],dt)){taxi.phase=TAXI_WALKING;taxi.linger=2.5f;
    int base=job.price,tip=taxi_tip_amount(base,taxi.crashes,taxi.damage,taxi.rough);
    advanceJob();char msg[96];char*q=append(msg,"Fare $");q=number(q,base);q=append(q," + tip $");q=number(q,tip);append(q,". New call available: T.");tell(msg);noticeInk=CASH_GREEN;}
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
static int warehouse_has_room(int site){
 const Business *b=sv.b+site;
 return b->crew==2||warehouse_units(site)+batch_units(b->product,b->tier)<=60;
}
static void production_tick(float dt){
 int changed=legal_tick(dt);
 for(int i=0;i<2;i++){
  Business*b=sv.b+i;if(!b->own)continue;
  if(b->timer>0){
   b->timer-=dt;
   if(b->timer<=.0002f){
    b->timer=0;b->durationMs=0;
    int units=batch_units(b->product,b->tier);
    if(b->crew==2){
     gain(0,units*prod[b->product].price-5,"Crew moved a batch ($5 fee deducted).");sv.sales+=units;
    }else{
     sv.stocks[b->product]+=units;tell(i?"Cinder batch ready. Stock is in storage.":"Grow batch ready. Stock is in storage.");
    }
    b->batches++;if(b->batches%4==0&&sv.cash>=18){sv.cash-=18;tell("Property upkeep: $18 clean.");noticeInk=CASH_GREEN;}
    changed=1;
   }
  }
  if(b->crew&&b->timer<=0&&sv.units>=batch_supply(i)&&warehouse_has_room(i)){
   sv.units-=batch_supply(i);start_batch(b,prod[b->product].time);changed=1;
  }
 }
 if(sv.pending&&sv.clearT>0&&!sv.audit&&!sv.charges){sv.clearT-=dt;if(sv.clearT<=.0002f){int amount=sv.pending;sv.pending=0;sv.clearT=0;cashNote("Exchange settled.",credit_wallet(1,amount));changed=1;}}
 if(changed)request_save();
}
static void businessTick(float dt){
 if(sv.audit&&(sv.auditT-=dt)<=0){
  sv.dirty=0;sv.pending=0;sv.clearT=0;legal_seize();
  sv.audit=0;sv.charges=1;sv.restitution=0;
  tell("Revenue froze the accounts and seized recorded dirty money.");noticeInk=CASH_RED;
  request_save();
 }
}
static void sell_stock(int product){
 int units=min(20,sv.stocks[product]);
 if(!units){tell("No stock to sell.");return;}
 sv.stocks[product]-=units;sv.sales+=units;
 const int site=prod[product].site;
 const char *receipt="Kiosk 09 paid for the stock.";
 // Large public sales expose the source warehouse to rival crews.
 if(units>=15&&sv.b[site].own&&!warehouse_secured(site)&&warehouse_units(site)>=3){
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
 case 0:add("COURIER ROUTE","Deliver two parcels and return the signed manifest to Mara.",10);add("TAXI FARE","Pick up a rider and drive to the drop-off.",11);add("BUSINESS OFFICE","Buy a legal courier franchise and check its account.",LEGAL_OPEN);break;
 case 1:
 add("VAULT RUN","Take the sealed lockbox, lose the police and bring it back to Ivo.",14);
 add("HOT PROPERTY","Steal the marked Vectra and hand it off to Vera after losing the police.",15);
 add("SIGNAL RAID","Move the relay cartridge from its studio to a backup console.",16);
 add("RIVAL COLLECTION","Recover the guarded consignment and deliver it to Vera.",17);break;
 case 2:priced("SUPPLY CRATE",45,"Three universal game-only batch credits.",20);priced("MED PATCH",25,"Treat minor injuries. Restores up to 50 health.",22);add("BUSINESS OFFICE","Buy or manage this market outlet.",LEGAL_OPEN);break;
 case 14:if(sv.gear<0)priced("BASIC PULSE PISTOL",95,"A ranged weapon. Space uses your equipped tool.",25);
 priced("ARMOR VEST",55,"Adds 45 points of protective armor.",21);
 if(sv.gear==0)priced("PULSE AMP",155,"Upgrade your pulse pistol's range and damage.",23);break;
 case 3:for(d=0;d<LEN(specs);d++){if(sv.owned&(1<<d))add(specs[d].name,"Owned: deploy from your garage.",30+d);else priced(specs[d].name,specs[d].price,"Buy outright and deploy immediately.",30+d);}{
  int ride=service_ride();
  if(ride<0)add("NO VEHICLE IN SERVICE RANGE","Park a car at Nightglass or own a garage ride.",0);
  else if(cars[ride].cond==100)add("VEHICLE ALREADY SERVICED","The eligible car is at full condition.",0);
  else{
   char *label=append(strings[nopt],"SERVICE ");label=append(label,specs[cars[ride].type].name);append(label,"  $32");
   add(strings[nopt],ride?"Repair the vehicle parked at Nightglass.":"Repair your selected garage vehicle.",35);
  }
 }break;
 case 4:case 5:{int i=m-4;Business*b=sv.b+i;
 if(!b->own){priced("BUY PROPERTY",facilityPrice[i],i?"Unlocks the synthetic product line.":"Unlocks two fictional grow products.",40+i);break;}
 if(b->timer<=0){
  if(i||lantern_terminal())add("START BATCH",i?"Uses two supplies from Ferry Market. 120 seconds.":"Uses one supply from Ferry Market. 120 seconds.",44+i);
  else add("GROW ROOM TERMINAL","Start batches at the terminal behind the fern shop.",0);
 }
 else{fmt(strings[nopt],"BATCH RUNNING / SECONDS: ",(int)(b->timer+.9999f));add(strings[nopt],"Keeps growing here and while you do other jobs.",0);}
 if(!i&&lantern_terminal())add(b->product==0?"SWITCH TO VIOLET HAZE":"SWITCH TO NIGHTLEAF","Two different yield and value profiles.",46);
 if(b->tier<3)priced("UPGRADE PROPERTY",tier_price(i,b->tier),"More product each batch, up to tier 3.",48+i);
 if(b->crew<2)priced(b->crew?"HIRE DISTRIBUTOR":"HIRE OPERATOR",b->crew?210:130,b->crew?"Automatically sells batches for a $5 flat fee.":"Queues batches automatically when supplies are available.",50+i);
 if(!warehouse_secured(i))priced("SECURE THIS WAREHOUSE",125,"Protects this facility from rival theft after large Kiosk sales.",52);
 fmt(strings[nopt],"SUPPLIES IN STORAGE  ",sv.units);add(strings[nopt],"Buy more supply crates from June at Ferry Market.",0);
 break;}
 case 6:add("SANCTIONED SPRINT","Cross the line to start a timed loop in the same licensed car.",13);break;
 case 7:
 for(d=0;d<3;d++){
  char*q=append(strings[nopt],"SELL ");q=append(q,prod[d].name);q=append(q," / STOCK ");number(q,sv.stocks[d]);
  add(strings[nopt],"Selling 15 or more units risks losing 3 from unsecured stock.",60+d);
 }
 if(sv.pending){fmt(strings[nopt],"CLEARING LEDGER / SEC: ",(int)sv.clearT+1);add(strings[nopt],"Payment settles while you roam.",0);}
 else add("EXCHANGE AT OWNED BUSINESSES","Use the Business Office at an owned legal franchise.",0);
 break;
 case 8:priced("FULL TREATMENT",42,"Return to full vitality.",24);priced("ARMOR VEST",55,"Adds 45 points of protective armor.",21);break;
 case 9:
 if(!sv.home)priced("BUY SAFEHOUSE",550,"Own a place to rest, hide from police and recover after being downed.",70);
 else add("REST AT SAFEHOUSE","Stay inside for 10 seconds to heal and clear pursuit.",72);
 break;
 case 10:add("LICENSED RECOVERY","Restart Dax's stalled van, then drive it to his service bay.",12);add("BUSINESS OFFICE","Buy or manage the recovery franchise.",LEGAL_OPEN);break;
 case 11:
 if(sv.charges){add("PAY CASE PENALTY  $180","Clean credits only. Clears purchase restrictions immediately.",80);fmt(strings[nopt],"COMMUNITY SERVICE  /  ",sv.restitution);append(strings[nopt]+sl(strings[nopt])," OF 3");add(strings[nopt],"Complete three courier or recovery shifts to clear the case.",0);}
 else add(sv.audit?"AUDIT TEAM EN ROUTE":"NO OPEN CASE","High-value purchases with dirty money may trigger a case.",0);
 break;
 case 15:add("DISPATCH DESK","The station and neighborhood posts share one finite police roster.",0);break;
 case 13:add("GO INSIDE","Step into your apartment.",94);break;
 case PAUSE_MENU:
 add("RESUME","Return to the game.",90);add("SAVE MANUALLY","Writes your career to your local save file.",91);
 if(job.kind>=0)add("ABANDON CURRENT CONTRACT","No payout. Free roam continues.",92);
 add("SAVE MENU","Switch careers or create a new character.",95);
 add("SHOW CONTROLS","Movement, action and menu keys.",93);break;
 }
 if(menu!=PAUSE_MENU&&legal_menu_index(menu)<0&&legal_rear_index(menu)<0)add("BACK","Close this menu.",90);
 sel=cl(sel,0,max(0,nopt-1));
}
static void doOpt(int a){
 if(legal_opt(a,&menu))return;
 if(a>=10&&a<=17){beginJob(a-10);return;}
 if(a>=30&&a<=34){
  if(job.kind==JOB_SPRINT&&job.step>0&&jobRide==0){
   tell("Finish or abandon the sprint before changing the registered car.");return;
  }
  int i=a-30,newCar=!(sv.owned&(1<<i));
  int cond=newCar?100:cl(sv.carCond[i],0,100);
  Car next={p.x,p.y,0,0,i,1,cond};
  if(!street_find_car_space(0,&next,p.x,p.y,0)){
   tell("No clear curb space. Try the garage again.");return;
  }
  if(newCar&&!canDirtyPurchase(specs[i].price))return;
  street_stage_car(0,next);
  sv.owned|=1<<i;sv.selection=i;sv.vehicleCond=sv.carCond[i]=cond;lastCar=0;p.car=-1;
  tell(cond?"Ride ready by the curb.":"Wreck at the curb; use Service at the desk.");menu=-1;request_save();return;
 }
 switch(a){
 case 20:if(canDirtyPurchase(45)){sv.units+=3;tell("Three supply credits added.");}break;
 case 21:if(sv.armor==90)tell("Armor is already full.");else if(canDirtyPurchase(55)){sv.armor=min(90,sv.armor+45);tell("Armor equipped.");}break;
 case 22:case 24:if(p.hp==100)tell("Health is already full.");else if(canDirtyPurchase(a==22?25:42)){p.hp=min(100,p.hp+(a==22?50:100));tell("Vitality restored.");}break;
 case 25:if(sv.gear<0&&canDirtyPurchase(95)){sv.gear=0;if(sv.intro==3)sv.intro=4;tell("Pulse tool equipped. Space to fire.");}break;
 case 23:if(sv.gear!=0)tell("An unmodified pulse pistol is required.");else if(canDirtyPurchase(155)){sv.gear=1;tell("Pulse amp installed.");}break;
 case 35:{
  int ride=service_ride();
  if(ride<0)tell("Bring a vehicle to Nightglass first.");
  else if(cars[ride].cond==100)tell("The vehicle is already at full condition.");
  else if(canDirtyPurchase(32)){
   cars[ride].cond=100;
   if(!ride)sv.vehicleCond=sv.carCond[sv.selection]=100;
   tell("Vehicle serviced to 100 condition.");
  }
  break;
 }
 case 40:case 41:if(!sv.b[a-40].own&&canDirtyPurchase(facilityPrice[a-40])){sv.b[a-40]=(Business){1,0,1,a==40?0:2,0,0,0};tell("Property is yours.");}break;
 case 44:case 45:{Business*b=sv.b+a-44;if(!b->own||b->timer>0||(a==44&&!lantern_terminal()))break;if(!warehouse_has_room(a-44)){tell("Warehouse full. Sell stock at Kiosk 09 first.");break;}int need=batch_supply(a-44);if(sv.units<need){tell("Not enough supply credits. Buy some first.");break;}sv.units-=need;start_batch(b,prod[b->product].time);tell(a==44?"Batch started. Check the plants in the back room.":"Cinder batch started.");break;}
 case 46:if(!lantern_terminal())return;else if(sv.b[0].timer>0)tell("Wait for the batch to finish first.");else sv.b[0].product^=1;break;
 case 48:case 49:{Business*b=sv.b+a-48;if(b->own&&b->tier<3){
    int site=a-48;
    if(b->timer>0&&b->crew!=2&&warehouse_units(site)+batch_units(b->product,b->tier+1)>60)
      tell("Clear storage at Kiosk 09 before expanding this batch.");
    else if(canDirtyPurchase(tier_price(site,b->tier))){b->tier++;tell("Production tier upgraded.");}
   }break;}
 case 50:case 51:{Business*b=sv.b+a-50;if(b->own&&b->crew<2&&canDirtyPurchase(b->crew?210:130)){b->crew++;tell(b->crew==1?"Operator hired. Batches start when supplies and storage are available.":"Distributor hired. Future batches ship directly to Kiosk 09.");}break;}
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
 case 80:if(spend(180)){sv.charges=0;sv.restitution=0;tell("Fine paid. Case cleared.");}break;
 case 90:menu=-1;return;
 case 91:request_save();menu=-1;return;
 case 92:end_job();tell("Contract abandoned.");menu=-1;return;
 case 93:helpBack=scene;scene=2;return;
 case 94:open_interior(12);return;
 case 95:profiles_open(0);return;
 default:return;
 }
 request_save();
}
static void punch(void){
 if(p.shot>0)return;
 p.shot=.34f;punchFlash=.21f;float co=cs(p.a),si=sn(p.a);
 int hit=-1;float best=27*27;
 for(int i=0;i<LEN(peds);i++){Ped*q=peds+i;float dx=q->x-p.x,dy=q->y-p.y,d=d2(dx,dy);
  if(q->hp<=0||dx*co+dy*si<3||absf(dy*co-dx*si)>13||d>=best)continue;
  if(!street_line_open(p.x,p.y,q->x,q->y,1))continue;
  hit=i;best=d;
 }
 if(hit<0)return;Ped*q=peds+hit;q->hp--;pedHit[hit]=.22f;move(&q->x,&q->y,co*10,si*10,8);heat(26);
 particle(q->x,q->y,8,0xffc18a);
 if(!q->hp){q->life=12;heat(30);tell("Knockdown. Patrols alerted.");}
}
static void shoot(void){
 if(p.shot>0)return;
 if(sv.gear<0){punch();return;}
 p.shot=sv.gear>0?.21f:.33f;p.trail=.11f;heat(24);particle(p.x+cs(p.a)*14,p.y+sn(p.a)*14,5,GOLD);
 int range=sv.gear>0?165:112,hit=-1;float best=range;
 float co=cs(p.a),si=sn(p.a);
 for(int i=0;i<LEN(peds);i++){
   Ped*q=peds+i;if(q->hp<=0)continue;
   float dx=q->x-p.x,dy=q->y-p.y,d=dx*co+dy*si,side=absf(dx*si-dy*co);
   if(d>0&&d<best&&side<13+d*.12f&&street_line_open(p.x,p.y,q->x,q->y,1)){hit=i;best=d;}
 }
 if(hit>=0){Ped*q=peds+hit;q->hp-=sv.gear>0?2:1;particle(q->x,q->y,10,q->role?TEAL:PINK);if(q->hp<=0){q->life=12;heat(q->role?28:41);}}
}
#include "dispatch.inc"
static void npcTick(float dt){
 for(int i=0;i<JOB_RIVAL_FIRST+JOB_RIVAL_COUNT;i++){
  Ped*q=peds+i;q->cool-=dt;
  if(q->hp<=0){q->life-=dt;if(q->life<0&&i<STREET_CIVILIANS)street_respawn_civilian(i);continue;}
  if(!q->role){street_tick_civilian(i,dt);continue;}
  if(q->role!=2)continue;
  float dx=p.x-q->x,dy=p.y-q->y,dist=d2(dx,dy);float kk=sq(dist);
  q->s=0;
  if(kk>29){
   q->a=look(dx,dy);float v=70*dt/kk;
   float ox=q->x,oy=q->y;move(&q->x,&q->y,dx*v,dy*v,9);
   if(d2(ox-q->x,oy-q->y)>.01f)q->s=70;
  }
  if(dist<72*72&&dist>4&&q->cool<=0&&street_line_open(q->x,q->y,p.x,p.y,2)){q->cool=1.05f;if(p.quiet<3)p.quiet=3;health(8);particle(p.x,p.y,5,PINK);}
 }
 dispatch_tick(dt);
}
static void enterExit(void){
 if(p.car>=0){if(!street_exit_car())tell("Slow down or pull over to exit safely.");return;}
 int i=carNear();
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
void tick(U keys){
 const float dt=1.f/60; U tap=keys&~prev;prev=keys;
 // Keep fractional frame precision during long sessions.
 anim+=dt;if(anim>=3600.f)anim-=3600.f;
 if(scene==0||scene==5){profiles_tick(tap);return;}
 if(!p.dead&&(tap&(SLOT|NEW_SLOT))){profiles_open(!!(tap&NEW_SLOT));return;}
 if(saveFlash>0)saveFlash--;if(saveFlash<0)saveFlash++;
 if(!p.dead&&scene!=2&&menu!=PAUSE_MENU){
   if(sv.playedTicks!=~0u)sv.playedTicks++;
   businessTick(dt);
   production_tick(dt);
 }
 if(tap&SAVE)request_save();
 if(scene==3&&menu!=PAUSE_MENU&&shelterT>0&&(shelterT-=dt)<=0)finish_shelter_rest();
 if(scene==3){interior_tick(keys,tap);return;}
 if(scene==4){walkin_tick(keys,tap);return;}
 if(scene==2){if(tap&(ESC|ENTER|USE|HELP)){scene=helpBack;}return;}
 if(p.dead){
  deadAnim++;
  if(deadAnim>45&&(tap&(ENTER|USE))){
   int destination=sv.home?9:8;
   if(p.heat>0&&dispatch_engaged())sv.arrests++;
   if(!sv.home)sv.cash=max(0,sv.cash-35);
   sv.x=loc[destination].x;sv.y=loc[destination].y;sv.hp=100;
   end_job();spawnPlayer();
   tell(sv.home?"Recovered at your Breakwater safehouse.":"Recovered at Tideway Clinic. Fee: up to $35.");
   request_save();
  }
  return;
 }
 if(tap&HELP){helpBack=1;scene=2;return;}
 if((tap&MAP)&&menu<0){mapOn=!mapOn;if(mapOn)navigation_input(tap);return;}
 if(mapOn){if(tap&ESC)mapOn=0;else navigation_input(tap);return;}
 if(tap&GFX){fastGfx=!fastGfx;tell(fastGfx?"Low FX on: rain and dynamic glows off.":"Full FX on.");}
 if((tap&TAXI_NEXT)&&menu<0)next_fare();
 if(tap&ESC){if(menu>=0){menu=-1;return;}active(PAUSE_MENU);makeMenu();return;}
 p.secs+=dt;p.noticeT-=dt;
 if(menu>=0){makeMenu();if(tap&UP)sel=(sel+nopt-1)%nopt;if(tap&DOWN)sel=(sel+1)%nopt;if(tap&(USE|ENTER))doOpt(opt[sel].action);return;}
 p.hit-=dt;p.shot-=dt;p.trail-=dt;punchFlash-=dt;crashFlash-=dt;for(int i=0;i<LEN(pedHit);i++)pedHit[i]-=dt;
 job_clock_tick(dt);
 int x=!!(keys&RIGHT)-!!(keys&LEFT),y=!!(keys&DOWN)-!!(keys&UP);
 walking=(x||y)&&p.car<0;if(walking)footcycle=wrap(footcycle+(keys&BRAKE?.31f:.22f));
 float checkpointX=p.x,checkpointY=p.y;
 street_tick_traffic(dt);
 if(p.car>=0){Car*c=cars+p.car;const CarSpec*s=specs+c->type;
  float speedBefore=absf(c->s),angleBefore=c->a;int taxiImpact=0;
  if(y<0)c->s+=s->acc*dt;if(y>0)c->s-=s->acc*1.25f*dt;
  if(!y)c->s*=.977f;if(keys&BRAKE)c->s*=.90f;
  float limit=s->vmax*(.50f+.005f*c->cond);if(c->s>limit)c->s=limit;if(c->s<-limit*.38f)c->s=-limit*.38f;
  float f=absf(c->s);if(f>7&&x)street_turn_car(p.car,wrap(c->a+x*s->turn*(keys&BRAKE?1.3f:1)*dt*(c->s>0?1:-1)));
  float xx=c->x,yy=c->y;
  street_move_car(p.car,cs(c->a)*c->s*dt,sn(c->a)*c->s*dt);
  if(d2(c->x-xx,c->y-yy)<f*f*dt*dt*.32f&&f>16){
   int damage=cl((int)(f*f/930),1,37);taxiImpact=damage;c->cond=max(0,c->cond-damage);c->s*=-.27f;
   if(damage>=3){particle(xx,yy,14,GOLD);if(crashFlash<=0){char msg[45];fmt(msg,"CRASH  /  CONDITION ",c->cond);tell(msg);}crashFlash=.45f;}
   if(p.car==0&&c->owner==1)sv.vehicleCond=sv.carCond[c->type]=c->cond;
  }
  if(f>52){
    float co=cs(c->a),si=sn(c->a);
    for(int i=0;i<LEN(peds);i++)if(peds[i].hp>0){
      float u=peds[i].x-c->x,v=peds[i].y-c->y;
      if(absf(u*co+v*si)<15&&absf(v*co-u*si)<6.5f){
        peds[i].hp=0;peds[i].life=12;c->s*=.55f;particle(c->x,c->y,11,PINK);
        if(p.heat<110)heat(45);else p.quiet=8;
        tell("Pedestrian hit. Traffic patrol alerted.");
        break;
      }
    }
  }
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
 }else{float v=(keys&BRAKE?183:123)*dt*(x&&y?.707107f:1);if(x||y)p.a=look(x,y);street_player_step(x*v,y*v);if(keys&PUNCH)punch();else if(keys&FIRE)shoot();}
 check_arrival(checkpointX,checkpointY);
 job_rival_encounter();
 taxi_tick(dt);
 by=placeNear();near=carNear();
 if((tap&USE)&&!job_use_street()){
  if(p.car>=0)enterExit();
  else {
   int shell=ordinary_door_near();
   if(choose_place(by,shell))open_interior(by);
   else if(shell>=0)walkin_open(shell);
   else enterExit();
  }
 }
 if(scene!=1)return;
 if((tap&ENTER)&&!(tap&USE)&&p.car<0&&near>=0)enterExit();
 if(job.kind==1&&taxi.phase>=TAXI_RIDING&&p.car!=taxi.ride)taxi_tick(0);
 p.quiet-=dt;if(p.quiet<=0&&p.heat>0){float rate=p.car>=0?26:19;p.heat-=dt*rate;if(p.heat<0)p.heat=0;}
 update_camera_lead();
 npcTick(dt);
 for(int i=0;i<LEN(fx);i++)if(fx[i].t>0){fx[i].t-=dt;fx[i].x+=fx[i].vx*dt;fx[i].y+=fx[i].vy*dt;}
}
int get_scene(void){return scene;}
void clear_input(void){prev=0;}
void pointer_input(int sx,int sy,U clicks){if(scene==0||scene==5){profiles_pointer(sx,sy,clicks);return;}if(scene==1&&mapOn&&!p.dead)navigation_pointer(sx,sy,clicks);}

static int sx(float x){return(int)x-cx;}static int sy(float y){return(int)y-cy;}
static int beam_block(int x,int y){return (U)x>=ROOM||(U)y>=ROOM||roofMask[y*ROOM+x];}
static void projectLights(Car*c){
 if(fastGfx||c->cond<=0)return;
 int x=sx(c->x),y=sy(c->y);if(x<-95||y<-95||x>W+95||y>H+95)return;
 float co=cs(c->a),si=sn(c->a);int range=61+(c->cond*19)/100;
 int rays[17];for(int ray=0;ray<17;ray++){float side=(ray-8)*.047f;int d;
  for(d=12;d<range;d+=3){
   if(beam_block((int)(c->x+co*d-si*side*d),(int)(c->y+si*d+co*side*d)))break;
  }rays[ray]=d;
 }
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
static void carbody(Car*c,int x,int y){
 float ca=cs(c->a),sa=sn(c->a);int ax[4],ay[4];
 static const int vx[]={-16,15,15,-16},vy[]={-8,-8,8,8};
 U color=c->cond>0?specs[c->type].c:0x3b4447;

 for(int i=0;i<4;i++){ax[i]=x+(int)(vx[i]*ca-vy[i]*sa);ay[i]=y+(int)(vx[i]*sa+vy[i]*ca);}
 tri(ax[0]+3,ay[0]+5,ax[1]+3,ay[1]+5,ax[2]+3,ay[2]+5,0x10191f);tri(ax[0]+3,ay[0]+5,ax[2]+3,ay[2]+5,ax[3]+3,ay[3]+5,0x10191f);
 for(int i=0;i<4;i++)disc(x+(int)(vx[i]*.66f*ca-vy[i]*sa),y+(int)(vx[i]*.66f*sa+vy[i]*ca),3,0x0d141c);
 tri(ax[0],ay[0],ax[1],ay[1],ax[2],ay[2],color);tri(ax[0],ay[0],ax[2],ay[2],ax[3],ay[3],color);
 for(int i=0;i<4;i++)line(ax[i],ay[i],ax[(i+1)%4],ay[(i+1)%4],mix(color,DARK,i==0?2:5));
 int roofFront=c->type==3?1:5,roofBack=c->type==2?-11:-8;
 for(int u=roofBack;u<=roofFront;u++)for(int v=-5;v<=5;v++){
   U cc=u==roofFront||u==roofBack?0xb3c9cf:u==roofFront-1||u==roofBack+1?0x283b50:v==-5||v==5?0x203038:mix(color,0x22283e,3);
   if(c->type==4&&v==0)cc=0x80a36c;
   dot(x+(int)(u*ca-v*sa),y+(int)(u*sa+v*ca),cc);
 }
 for(int i=0;i<2;i++){int k=i?5:-5;int x1=x+(int)(13*ca+k*sa),y1=y+(int)(13*sa-k*ca),x2=x-(int)(14*ca+k*sa),y2=y-(int)(14*sa-k*ca);
  disc(x1,y1,2,0xffe0a3);dot(x1,y1,0xffebbf);disc(x2,y2,1,0xff567f);}
 if(c->cond>0&&c->cond<45){disc(x-2,y-3,3,0x303640);if(c->cond<23){disc(x-3-(int)(anim*8)%7,y-8,3,0x4a535b);}}
 if(c->owner==1&&p.car<0){line(x-4,y-18,x,y-14,LIME);line(x,y-14,x+4,y-18,LIME);}
}
typedef struct{int x,y;}Pt;
static int previewRig;
static Pt rig(int x,int y,float co,float si,int u,int v,int z){
 float depth=u*si+v*co;
 if(previewRig){depth*=.28f;z=(5*z+1)/3;}
 return (Pt){x+(int)(u*co-v*si),y+(int)depth-z};
}
static void rod(Pt a,Pt b,int r,U c){for(int off=-r;off<=r;off++){line(a.x+off,a.y,b.x+off,b.y,c);if(off)line(a.x,a.y+off,b.x,b.y+off,c);}}
static void quad(Pt a,Pt b,Pt c,Pt d,U col){tri(a.x,a.y,b.x,b.y,c.x,c.y,col);tri(a.x,a.y,c.x,c.y,d.x,d.y,col);}
static void figure(int x,int y,U jacket,int step,float a,int style,int hue,const Look*look){
 float co=cs(a),si=sn(a);int hero=style==4,top=0,legs=0,wide=0,face=0,hs=-1;U ink=0x111a27;
 static const U skins[]={0xdeb291,0xa16352,0x805248,0xe3c3a7,0xbd8571};
 static const U hairs[]={0x29323c,0x201720,0x45403f,0x66433f,0x34263b};
 U skin=skins[hue%5],hair=hairs[hue%5],pants=0x42546b,shoe=0x111b26;
 if(look){
  const unsigned char*k=look->part;wide=(int)k[BUILD]-1;skin=charSkin[k[SKIN]];hs=k[HAIR];hair=charHair[k[HAIR_TONE]];
  face=k[DETAIL];top=k[TOP];legs=k[LEGS];pants=charLeg[k[LEG_TONE]];shoe=charShoe[k[SHOES]];
  if(!hero||jacket!=PINK)jacket=charTop[k[TOP_TONE]];
 }
 U seam=mix(jacket,ink,4),rim=mix(jacket,PAPER,3);
 if(previewRig)rect(x-6,y+2,13,2,0x151d27);else disc(x+3,y+4,8,0x151d27);
 for(int side=-1;side<=1;side+=2){
  Pt hip=rig(x,y,co,si,-2,side*3,5),boot=rig(x,y,co,si,-2+step*side,side*4,0);
  Pt knee={(hip.x+boot.x)/2,(hip.y+boot.y)/2};
  rod(hip,boot,legs==1||legs==4?2:1,legs==2?skin:pants);
  if(legs==2)rod(hip,knee,1,pants);
  if(legs==1||legs==4)dot(knee.x,knee.y,mix(pants,PAPER,3));
  if(legs==3)dot(boot.x,boot.y-2,mix(pants,ink,4));
  disc(boot.x,boot.y,2,shoe);dot(boot.x,boot.y-1,look?mix(shoe,PAPER,4):0x829aab);
 }
 int w=7+wide+(top==2||top==3);
 Pt lt=rig(x,y,co,si,1,-w,10),rt=rig(x,y,co,si,1,w,10),rb=rig(x,y,co,si,-4,4+wide,5),lb=rig(x,y,co,si,-4,-4-wide,5);
 quad(lt,rt,rb,lb,ink);
 quad(rig(x,y,co,si,1,2-w,10),rig(x,y,co,si,1,w-2,10),rig(x,y,co,si,-4,3+wide,6),rig(x,y,co,si,-4,-3-wide,6),jacket);
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
 Pt head=rig(x,y,co,si,1,0,14);int hx=head.x,hy=head.y;
 if(previewRig)rod((Pt){hx,hy+3},rig(x,y,co,si,1,0,10),1,mix(skin,ink,2));
 if(top==3)disc(hx,hy+3,5,seam);
 if(hs==8){disc(hx-(int)(co*4),hy-(int)(si*4)+2,2,hair);}
 if(hs==9){rect(hx-4,hy-3,9,8,hair);rect(hx-3,hy+4,7,2,mix(hair,ink,2));}
 disc(hx,hy,4,ink);disc(hx,hy,3,skin);
 if(!look){
  U cap=(style==1||style==3)?seam:hair;rect(hx-2,hy-3,5,2,cap);dot(hx-2,hy-1,cap);
  if(style==1||style==3)line(hx-3,hy-2,hx+3,hy-2,rim);
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
   dot(hx-1,hy,ink);dot(hx+1,hy,ink);
   if(face==1||face==2){line(hx-3,hy-1,hx+3,hy-1,ink);dot(hx-2,hy,face==1?TEAL:ink);dot(hx+2,hy,face==1?TEAL:ink);}
   if(face==3||face==4){line(hx-2,hy+1,hx+2,hy+1,mix(skin,hair,face==3?4:7));if(face==4)rect(hx-1,hy+2,3,2,hair);}
   if(face==5){rect(hx-2,hy+1,5,2,0x8aa3ab);dot(hx,hy+2,0x526878);}
  }
  if(face==6){dot(hx-3,hy+1,0xffdc8e);dot(hx+3,hy+1,0xffdc8e);}
 }
 if(hero){
    /* The highest hairstyle is 22 px above the foot anchor. Move the arrow
       along the facing vector as that height projects into its path. */
    const int start=max(17,(int)(13.f-22.f*si));
    Pt tip=rig(x,y,co,si,start+11,0,0),el=rig(x,y,co,si,start+3,-5,0),er=rig(x,y,co,si,start+3,5,0);
    Pt sl=rig(x,y,co,si,start,-2,0),sr=rig(x,y,co,si,start,2,0);
    Pt il=rig(x,y,co,si,start+4,-2,0),ir=rig(x,y,co,si,start+4,2,0);
    tri(tip.x,tip.y+1,el.x,el.y+1,er.x,er.y+1,0x132532);
    quad(sl,il,ir,sr,0xffffff);tri(tip.x,tip.y,el.x,el.y,er.x,er.y,0xffffff);
 }
}
static void person(int x,int y,U jacket,int step,float a,int style,int hue){figure(x,y,jacket,step,a,style,hue,style==4?&sv.look:0);}
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
 if(id<=2&&w>65){static const char *brand[]={"MOTEL","NITE","OFFICES","VANTA","HOTEL","GARAGE","RAIL","POST"};
   if(w>77)center(x+w/2,y-r+h-29,brand[(x/256+3*(y/256)+id)%LEN(brand)],tint,0);
   else{for(int j=0;j<3;j++)rect(x+10+j*15,y-r+h-25,8,3,mix(tint,roof,4));}}
 line(x,y-r,x+w-1,y-r,mix(top,tint,3));
 line(x,y-r,x,f,0x839ba0);
}
static void light(int x,int y,int radius,U color,int weight){
 int rr=radius*radius;
 for(int j=max(0,y-radius);j<min(ROOM,y+radius);j++)for(int i=max(0,x-radius);i<min(ROOM,x+radius);i++){int d=(i-x)*(i-x)+(j-y)*(j-y);int a=(rr-d)*weight/rr;if(a>0){U*q=city+j*ROOM+i;*q=mix(*q,color,min(7,a));}}
}
static void sceneryTree(int x,int y,int id){
 const U greens[]={0x356c58,0x4c845e,0x4f786e,0x578c6d};
 U a=greens[id%4];
 disc(x+5,y+5,12,0x172823);rect(x-2,y-1,4,10,0x715b4a);
 disc(x,y-8,11,0x253e3b);disc(x-3,y-11,8,a);disc(x+3,y-11,6,mix(a,0x91bb78,2));
}
#include "district_art.inc"
static void setup_ordinary_doors(void){
 z(ordinaryDoorX,sizeof ordinaryDoorX);z(ordinaryDoorY,sizeof ordinaryDoorY);
 for(int i=0;i<nb;i++){
  const Building*b=buildings+i;
  if(b->w<=0||b->h<=0||zone_is_building(i))continue;
  int x=b->x+b->w/2+b->ht/4,y=b->y+b->h+15;
  ordinaryDoorX[i]=x;ordinaryDoorY[i]=y;
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
   if(zone_is_lot(gy*8+gx)){}
   else if(lot[gy*8+gx][0]==0){for(int j=0;j<9;j++)sceneryTree(x+56+(j%3)*58+(id*j)%11,y+65+(j/3)*56,id+j);}
   else{sceneryTree(x+46,y+182,id);sceneryTree(x+201,y+191,id+2);rect(x+61,y+173,16,8,0x445162);rect(x+61,y+173,16,2,0x708596);}
   if(gx&&gy){int xx=x+31,yy=y-34;U c=y<600?0x845987:x>1300?0x388b8c:0x8d7352;light(xx,yy,46,c,4);
    rect(xx-1,yy-1,3,13,0x293842);rect(xx-4,yy-4,8,4,mix(c,PAPER,5));
    if((gx+gy)%2==0){int tx=x-34,ty=y+38;light(tx,ty,27,0x549e92,3);rect(tx,ty,2,10,0x4b6364);rect(tx-2,ty-4,6,5,TEAL);}
   }
 }
 z(roofMask,sizeof roofMask);
 for(int i=0;i<nb;i++){Building*b=buildings+i;if(b->w<=0||b->h<=0)continue;
  int tx=(b->x/256)*256,ty=(b->y/256)*256;
  clipLeft=tx+42;clipTop=ty+42;clipRight=tx+214;clipBottom=ty+214;
  if(!zone_is_building(i))building(b);
  for(int y=max(clipTop,b->y-b->ht-1);y<min(clipBottom,b->y+b->h+1);y++)
   for(int x=max(clipLeft,b->x);x<min(clipRight,b->x+b->w+b->ht/2+1);x++)roofMask[y*ROOM+x]=1;
 }
 clipLeft=clipTop=0;clipRight=clipBottom=ROOM;
 for(int i=0;i<LEN(loc);i++)zone_art(i);
 for(int i=0;i<nb;i++)paint_ordinary_door(i);
 surface=pixels;rw=W;rh=H;clipLeft=clipTop=0;clipRight=W;clipBottom=H;
}
static void diamond(int x,int y,int r,U c){line(x-r,y,x,y-r,c);line(x,y-r,x+r,y,c);line(x+r,y,x,y+r,c);line(x,y+r,x-r,y,c);}
static void mark(int x,int y,int r,U c){
 for(int i=0;i<3;i++)diamond(x,y,r+i,mix(DARK,c,7-i));
 disc(x,y,3,c);
}
#include "navigation.inc"
static void hud(void){
 char v[130],*a;
 const char *objective=0;
 if(job.kind>=0){
  a=append(v,jobNames[job.kind]);a=append(a,"  ");a=number(a,job.step+1);
  a=append(a,"/");a=number(a,job.total);
  if(job_has_clock())fmt(a,"  SEC ",(int)job.time);
  objective=job.kind==JOB_TAXI?(job_vehicle_target()>=0?"Enter the marked taxi or use your own car.":taxi_objective()):job_objective();
 }
 else if(sv.intro==1||sv.intro==2){append(v,"FIRST SHIFT");objective="Enter Mercury Dispatch. Talk to Mara.";}
 else if(sv.intro==3){append(v,"FIRST PAYCHECK");objective="M: pick Vanta Armory, Motors, or more work.";}
 if(!objective&&sv.taxiFares&&p.car>=0&&cars[p.car].owner){append(v,"NEW CALL AVAILABLE");objective="T: accept the next taxi fare.";}
 if(objective){
  int width=max(500,max(textw(objective,0),textw(v,0))+26);
  box(16,16,width,53,DARK,0x415766);rect(16,16,3,53,LIME);
  textat(28,18,v,LIME,0);textat(28,40,objective,PAPER,0);
 }
 box(650,16,294,79,DARK,0x415766);
 draw_wallet(650,16,294);
 rect(845,70,82,6,0x34434b);rect(845,70,p.hp*82/100,6,0x6ea7ee);
 if(p.car>=0){fmt(v,"CAR ",cars[p.car].cond);textat(670,64,v,cars[p.car].cond<40?GOLD:MUTED,0);}else textat(670,64,sv.gear<0?"FISTS":"PULSE",MUTED,0);
 int h=(p.heat>15)+(p.heat>80)+(p.heat>150);
 if(h>0){textat(761,64,"HEAT",PINK,0);for(int i=0;i<h;i++)diamond(813+i*9,76,3,PINK);}
 else if(p.heat>0)textat(761,64,"CLEARING",PINK,0);
 if(sv.armor){rect(845,80,82,3,0x34434b);rect(845,80,sv.armor*82/90,3,TEAL);}
 if(sv.audit||sv.charges){
  box(675,101,269,48,0x221923,0x9a486d);
  textat(688,103,sv.audit?"REVENUE EN ROUTE":"CASE HOLD",CASH_RED,0);
  if(sv.audit)fmt(v,"ARRIVAL  ",(int)sv.auditT+1);else{a=append(v,"RESTITUTION  ");a=number(a,sv.restitution);append(a," / 3");}
  textat(688,124,v,PAPER,0);
 }
 const char*use=job_street_prompt();
 if(!use&&p.car>=0)use=absf(cars[p.car].s)>STREET_EXIT_SPEED?"SHIFT  SLOW TO EXIT":"E  EXIT CAR";
 else if(!use){
  int shell=ordinary_door_near();
  if(choose_place(by,shell)){a=append(v,"E  ENTER ");append(a,loc[by].label);use=v;}
  else if(shell>=0){a=append(v,"E  ENTER ");append(a,walkin_name(shell));use=v;}
  else if(near>=0){
   if(absf(cars[near].s)>STREET_BOARD_SPEED)use="WAIT FOR CAR TO SLOW";
   else use=cars[near].owner||cars[near].reported?"GET IN  [ENTER]":"STEAL VEHICLE  [ENTER]";
  }
 }
 if(use){int w=textw(use,0)+26;box((W-w)/2,487,w,30,DARK,0x566a75);center(W/2,491,use,PAPER,0);}
 if(p.noticeT>0){int w=min(735,25+textw(notice,0));box(16,438,w,28,DARK,0x455766);textat(28,440,notice,notice_color(),0);}
 if(saveFlash)textat(18,508,saveFlash>0?"SAVED":"SAVE ERROR",saveFlash>0?TEAL:PINK,0);
}
static void modal(int x,int y,int w,int h,U c){for(int i=0;i<W*H;i++)pixels[i]=((pixels[i]>>1)&0x007f7f7f)+0x050a0d;rect(x+8,y+8,w,h,0x080f14);box(x,y,w,h,0x101e25,0x526c73);rect(x,y,w,4,c);}
#include "interiors.inc"
#include "streetside_interiors.inc"
#include "profiles.inc"
void paint(void){
 if(scene==0||scene==5){profiles_paint();return;}
 if(scene==3){interior_paint();return;}
 if(scene==4){walkin_paint();return;}
 if(scene==1&&mapOn){minimap();return;}
 cx=camera_left();cy=camera_top();
 typedef U V4 __attribute__((vector_size(16),aligned(1),may_alias));
 for(int y=0;y<H;y++){const V4 *q=(const V4*)(city+(cy+y)*ROOM+cx);V4 *r=(V4*)(pixels+y*W);for(int x=0;x<W/4;x++)r[x]=q[x];}
 if(!fastGfx)for(int i=0;i<100;i++){int x=(i*197+((int)(anim*28))%W)%W,y=(i*233+((int)(anim*174))%H)%H;
   U c=mix(pixels[y*W+x],0x6e8d9d,2);line(x,y,x-2,y+5,c);}
 for(int i=0;i<LEN(loc);i++){const Place*l=loc+i;int x=sx(l->x),y=sy(l->y);if(x<-110||y<-25||x>W+110||y>H+25)continue;
  if(i==by||(i==0&&sv.intro>0&&sv.intro<3)){
    int w=textw(l->label,0)+16;box(x-w/2,y-40,w,23,DARK,mix(l->tint,DARK,3));center(x,y-40,l->label,l->tint,0);
  }
 }
 if((sv.intro==1||sv.intro==2)&&job.kind<0){int tx=sx(loc[0].x),ty=sy(loc[0].y);mark(cl(tx,27,W-27),cl(ty,150,H-70),16+(int)(sn(anim*4)*3),LIME);}
 for(int i=0;i<nc;i++)if(i==p.car||absf(cars[i].s)>9)projectLights(cars+i);
 for(int i=0;i<nc;i++){Car*c=cars+i;if(c->cond<=0&&i==0&&!sv.owned)continue;int x=sx(c->x),y=sy(c->y);if(x>-25&&x<W+25&&y>-25&&y<H+25)carbody(c,x,y);}
 for(int i=0;i<LEN(peds);i++){Ped*q=peds+i;int x=sx(q->x),y=sy(q->y);if(x<-22||x>W+22||y<-29||y>H+22)continue;if(q->hp<=0){if(q->life>0){line(x-6,y-3,x+6,y+3,0x815763);disc(x-7,y-4,3,0x85757b);}continue;}
 U c=pedHit[i]>0?0xffdfbc:q->role==1?(dispatch_posted(i)?0x668df0:(((int)(anim*9)&1)?0x668df0:PINK)):q->role==2?PINK:(i%2)?0x92b7b4:0xca8c71;person(x,y,c,q->s<=2?0:(int)(sn(anim*(q->role?16:11)+i)*4),q->a,q->role,i);
 }
 if((job.kind==1&&taxi.phase!=TAXI_RIDING)||(taxi.phase==TAXI_WALKING&&taxi.linger>0)){
  int px=sx(taxi.x),py=sy(taxi.y);if(px>-26&&py>-33&&px<W+26&&py<H+26){
   int stride=taxi.phase==TAXI_WAIT?0:(int)(sn(anim*15)*5);
   person(px,py,riders[taxi.rider].outfit,stride,taxi.a,0,riders[taxi.rider].face);
   if(job.kind==1){center(px,py-36,riders[taxi.rider].name,0xe2b3ff,0);diamond(px,py+5,8,0xb988f2);}
  }
 }
 int x=sx(p.x),y=sy(p.y);
 if(p.car<0&&!p.dead)person(x,y,p.hit>0&&(int)(anim*20)&1?PINK:0xc6d979,walking?(int)(sn(footcycle)*5):0,p.a,4,3);
 if(p.trail>0){int xx=x,yy=y;for(int j=15;j<(sv.gear>0?166:113);j+=5){if(solid(p.x+cs(p.a)*j,p.y+sn(p.a)*j,1))break;xx=x+(int)(cs(p.a)*j);yy=y+(int)(sn(p.a)*j);}line(x,y,xx,yy,GOLD);}
 for(int i=0;i<LEN(fx);i++)if(fx[i].t>0)disc(sx(fx[i].x),sy(fx[i].y),2,fx[i].c);
 if(job.kind==JOB_RIVALS&&job.step==0&&rivalsDeployed){
  int x=sx(job.xs[0]),y=sy(job.ys[0]);
  box(x-10,y-7,20,14,0x594537,0x9e8163);
  rect(x-9,y-1,18,2,0x272b30);rect(x-2,y-4,4,8,0xd0a07a);
 }
 if(job.kind>=0){
  int ride=job_vehicle_target();
  NavPoint target=nav_job_goal();
  int x=sx(target.x),y=sy(target.y);
  U color=ride>=0?GOLD:job.kind<4?TEAL:PINK;
  if(x<30||x>W-30||y<82||y>H-60){
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
 if(scene==1&&!mapOn)hud();if(scene==1)minimap();
 if(scene==2){modal(175,100,610,340,TEAL);textat(205,122,"FIELD GUIDE",TEAL,1);wrapped(205,180,"WASD or arrows: move / drive. Shift: sprint / handbrake. E: interact / exit. Enter: get in a nearby car. X: punch. Space: fists or pistol. T: another taxi fare. M: map and GPS. F2: low FX. F3: saves. F4: new career. F5: save. Esc: pause / back.",PAPER,61);
 textat(205,280,"Teal markers = legal work. Pink = street contracts.",LIME,0);textat(205,307,"Grow and synth properties are optional income paths.",LIME,0);textat(205,334,"Civil cases: pay Revenue or work 3 courier/recovery shifts.",GOLD,0);textat(205,388,"ENTER / ESC  RETURN",TEAL,0);return;}
 if(p.dead){modal(240,149,480,212,PINK);textat(268,171,"DOWN IN VANTA",PINK,1);textat(269,231,sv.home?"Recover at your Breakwater safehouse.":"Tideway Clinic charges up to $35 for recovery.",PAPER,0);textat(269,284,"ENTER  /  RECOVER",LIME,0);return;}
 if(menu>=0){paint_active_menu();draw_wallet(650,16,294);}
}
