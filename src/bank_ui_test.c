/* Play the ATM/Revenue interaction through game keys and test its native raster. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <string.h>
static void press(U k){tick(k);tick(0);}
static void frame(const char*name){
 paint();FILE*f=fopen(name,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int j=0;j<W*H;j++){U v=pixels[j];fputc(v>>16,f);fputc(v>>8,f);fputc(v,f);}fclose(f);
}
static void approach(int i){
 const BankAtm*a=bankAtms+i;
 p.x=a->x;p.y=a->y+42;
 assert(!solid(p.x,p.y,10)&&atm_near()<0);
 for(int j=0;j<16;j++)tick(UP);
 tick(0);
 assert(atm_near()==i&&p.y>a->y+1);
 assert(placeNear()<0&&ordinary_door_near()<0);
}
static void discover(void){
 scene=1;sv.intro=4;p.x=660;p.y=768;p.car=-1;
 press(MAP);assert(mapOn);
 for(int i=0;i<LEN(bankAtms);i++){
  NavPoint door=nav_door(NAV_ATM_BASE+i);
  int x=NAV_MAP_X+door.x*NAV_MAP_SIZE/BASE_ROOM,y=NAV_MAP_Y+door.y*NAV_MAP_SIZE/BASE_ROOM;
  pointer_input(x,y,1);
  assert(navSelected==NAV_ATM_BASE+i&&navManualLoc==navSelected);
  paint();assert(navHasManual&&navGoalKind==3&&navCount>=2);
  assert(nav_same(navGoal,door));
  if(!i)frame("build/atm-atlas.ppm");
 }
 press(FIRE);assert(navSelected==NAV_TRAP); /* Bell Lane follows the last ATM. */
 press(FIRE);assert(navSelected==0);
 for(int i=1;i<=LEN(loc);i++)press(FIRE);
 assert(navSelected==NAV_ATM_BASE);
 press(LEFT);assert(navSelected>=LEN(loc)&&navSelected<LEN(loc)+nb);
 press(ESC);assert(!mapOn);
}
static void terminals(void){
 nc=0;scene=1;menu=-1;sv.intro=4;sv.cash=175;sv.dirty=671;
 for(int i=0;i<LEN(bankAtms);i++){
  approach(i);assert(solid(bankAtms[i].x,bankAtms[i].y-6,6));
  if(!i)frame("build/atm-kiosk.ppm");
  press(USE);assert(atmOpen==i&&atmMode==0&&atm_amount()==100);
  if(i){press(ESC);continue;}
  assert(sv.cash==175&&bank_balance()==0);
  float x=p.x,y=p.y;
  press(LEFT);assert(atm_amount()==50&&p.x==x&&p.y==y);
  press(RIGHT);press(RIGHT);assert(atm_amount()==175&&atmStep==LEN(atmAmounts));
  press(ENTER);assert(sv.cash==0&&bank_balance()==175&&sv.dirty==671&&atmLastMove==175);
  press(ENTER);assert(atmRejected&&sv.cash==0&&bank_balance()==175);
  press(DOWN);press(LEFT);assert(atmMode&&atm_amount()==100);
  tick(ENTER);for(int j=0;j<32;j++)tick(ENTER);tick(0);
  assert(sv.cash==100&&bank_balance()==75&&sv.dirty==671&&atmLastMove==100);
  frame("build/atm-window.ppm");
  press(ESC);assert(atmOpen<0&&!mapOn&&menu<0);
 }
}
static void truthful_case_alert(void){
 int opened=revenueCrossed;RevenueReceipt prevReceipt=revenueReceipt;
 Save previous=sv;NightSave prevNight=nv;
 sv.charges=0;sv.cash=0;sv.dirty=45;sv.dirtySpent=500;
 assert(canDirtyPurchase(45));finish_purchase();RevenueReceipt r=revenue_seizure_notice();
 assert(r.kind==REVENUE_THRESHOLD&&!r.dirty&&!r.pending&&!strcmp(bank_alert_title(r),"REVENUE NOTICE"));
 assert(strstr(notice,"No funds seized."));
 revenue_close_case();sv.dirty=49;sv.dirtySpent=500;sv.pending=39;
 sv.legit[0]=(LegalBusiness){.owned=1,.pending=75};
 assert(canDirtyPurchase(45));finish_purchase();r=revenue_seizure_notice();
 assert(r.dirty==4&&r.pending==114&&!strcmp(bank_alert_title(r),"ASSETS SEIZED"));
 assert(strstr(notice,"Funds seized."));
 assert(textw("DIRTY -$2147483647  /  PENDING -$1001440",0)<482);
 sv=previous;nv=prevNight;revenueCrossed=opened;revenueReceipt=prevReceipt;
}
static void seizure(void){
 sv.cash=0;sv.dirty=1000;sv.dirtySpent=REVENUE_DIRTY_LIMIT-45;scene=1;
 open_interior(2);interior_x=interior_desk_x;interior_y=interior_desk_y;
 press(USE);assert(menu==2&&opt[sel].action==20);
 press(ENTER);assert(!assetAlert&&!sv.charges&&sv.dirty==955);
 press(ENTER);assert(assetAlert==SEIZURE_TICKS&&sv.charges&&assetSeizure.kind==REVENUE_THRESHOLD&&assetSeizure.clean==0&&assetSeizure.dirty==910);
 assert(!sv.dirty&&bank_balance()==75);
 frame("build/atm-seizure.ppm");
 int shook=0;
 for(int i=0;i<SEIZURE_SHAKE;i++){
  fastGfx=0;int a=bank_warning_offset();if(a)shook=1;assert(abs(a)<=4);
  fastGfx=1;assert(!bank_warning_offset());tick(0);
 }
 fastGfx=0;assert(shook&&!bank_warning_offset()&&assetAlert>0);
 press(ESC);assert(menu<0&&assetAlert>0);
 float x=interior_x;tick(LEFT);tick(0);assert(x>interior_x); /* Popup never consumes controls. */
 for(int i=0;i<SEIZURE_TICKS;i++)tick(0);
 assert(!assetAlert);
 scene=1;approach(0);press(USE);press(DOWN);press(ENTER);
 assert(atmOpen==0&&sv.cash==75&&bank_balance()==0&&sv.charges);
 press(ESC);assert(bank_deposit(25)==25&&sv.cash==50&&sv.bank==25);sv.dirty=15;sv.sweepTicks=1;
 tick(0);assert(assetAlert==SEIZURE_TICKS&&assetSeizure.kind==REVENUE_SWEEP&&assetSeizure.clean==50&&assetSeizure.dirty==15&&sv.bank==25);
 assert(textw("CLEAN -$2147483647  /  DIRTY -$2147483647",0)<482);
 frame("build/atm-recurring-seizure.ppm");
 profiles_init();assert(atmOpen<0&&!assetAlert);
}
int main(void){
 init_city();discover();terminals();truthful_case_alert();seizure();
 puts("Passed: all three walk-up ATMs and atlas routes, exact keyboard transfers, real seizure popup and timeout, low FX, and continued clean ATM access.");
}
