/* Cross-system playable regression. The starting inventory, position at each
 * curb, and wallet limits are fixtures; interactions, boarding, alighting,
 * payments, shelf transfers and double claims are real game ticks & disk IO.
 * cc -std=c11 -O0 -Isrc scripts/qa/venue_boundary_test.c src/linux_save.c -o build/venue_boundary_test
 * XDG_DATA_HOME="$(mktemp -d)" build/venue_boundary_test 17
 */
#define main ordinary_gig_matrix
#include "night_club_gig_test.c"
#undef main
#include <string.h>
static U rng_seed;
static unsigned vrand(unsigned cap){rng_seed=rng_seed*1664525u+1013904223u;return (rng_seed>>7)%cap;}
static int observe;
static void in_loft(int id){
 p.car=-1;open_interior(12);hit(ENTER);assert(nvs_room==NVR_HOME);hit(TAXI_NEXT);assert(nvs_folio);
 for(int j=0;j<id;j++)hit(DOWN);
 assert(nvs_sel==id);
}
static void check_stock(int a,int b,int c){assert(nvs_ed_count(6)==a&&nvs_ed_count(7)==b&&nvs_ed_count(8)==c);}
static void finish_gig(int show,int wallet){
 ncr_reset();sv.dayClock=show*480+20;sv.cash=wallet;
 accept(show);park(ncr.pick,1,0);frames(65);assert(ncr.phase==NCR_DRIVE);
 park(ncr.drop,1,0);frames(90);assert(!ncr_active());
}
static void ledgers(void){
 int show=vrand(3),fee=ncr_shows[show].fee;int a=2+vrand(80),b=2+vrand(80),c=2+vrand(80);
 U shelf=(U)a|((U)b<<7)|((U)c<<14);
 nv.world[23]=shelf;sv.intro=4;sv.owned=1;p.hp=100;
 finish_gig(show,500);
 if(observe){printf("Old immediate $%d payout clobbered shelves [%d,%d,%d] to [%d,%d,%d]\n",fee,a,b,c,nvs_ed_count(6),nvs_ed_count(7),nvs_ed_count(8));return;}
 assert(sv.cash==500+fee&&!ncr_owed());check_stock(a,b,c);load_save();assert(wason);check_stock(a,b,c);
 finish_gig(show,WALLET_LIMIT-vrand(10));assert(ncr_owed()==fee);check_stock(a,b,c);
 load_save();assert(wason&&ncr_owed()==fee);check_stock(a,b,c);
 /* Take one actual matching edition out while Bryn holds the fee. */
 int id=6+vrand(3);in_loft(id);int bag=np_get(NP_BAG+id,99);
 hit(FIRE);assert(np_get(NP_BAG+id,99)==bag+1);a-=id==6;b-=id==7;c-=id==8;
 check_stock(a,b,c);load_save();assert(wason&&ncr_owed()==fee);check_stock(a,b,c);
 hit(ESC);sv.cash=400;promoter();hit(DOWN);hit(ENTER);assert(sv.cash==400+fee&&!ncr_owed());
 check_stock(a,b,c);load_save();assert(wason&&sv.cash==400+fee&&!ncr_owed());
 assert(!ncr_pay());check_stock(a,b,c);
#ifdef DC_VENUE_SHELF_LEDGER
 /* Two old exact values are valid quantities as well as possible held fees.
  * No auto-redemption. Back is inert. Verify both explicit recovery choices. */
 for(int value=145;value<=165;value+=20){
  for(int redeem=0;redeem<2;redeem++){
   nv.world[23]=(U)value;sv.cash=500;scene=1;int club=night_club_listing();
   /* Trying to use a shelf before seeing Bryn must NOT pick a side for you. */
   int grade=6+vrand(3);nv.phone[NP_BAG+grade]=2;in_loft(grade);hit(USE);hit(FIRE);hit(MAP);
   assert(nv.phone[NP_BAG+grade]==2&&nv.world[23]==(U)value&&navManualLoc==LEN(loc)+club);
   hit(SAVE);load_save();assert(wason&&nv.world[23]==(U)value);
   hit(ESC);scene=1;night_club_open(club);ncX=276;ncY=371;
   hit(USE);assert(ncPanel==NC_OLD_LEDGER&&ncChoice==2&&!ncr_owed());hit(ENTER);
   assert(ncPanel==NC_NONE&&nv.world[23]==(U)value&&sv.cash==500);
   hit(USE);hit(redeem?UP:DOWN);hit(ENTER);assert(ncPanel==NC_NONE);load_save();
   assert(sv.cash==500+(redeem?value:0)&&!ncr_owed()&&!venue_ambiguous(nv.world[23]));
   if(redeem)check_stock(0,0,0);else check_stock(value&127,1,0);
  }
  /* New shelf deposits landing on the same bit pattern are not old fees. */
  nv.world[23]=(U)(value-1);nv.phone[NP_BAG+6]=1;in_loft(6);hit(USE);
  assert(nv.phone[NP_BAG+6]==0&&!venue_ambiguous(nv.world[23])&&!ncr_owed());
  check_stock(value&127,1,0);hit(ESC);load_save();assert(!venue_ambiguous(nv.world[23]));
 }
 nv.world[23]=115;sv.cash=WALLET_LIMIT;check_stock(0,0,0);assert(ncr_owed()==115);
 nv.phone[NP_BAG+8]=1;in_loft(8);hit(USE);check_stock(0,0,1);assert(ncr_owed()==115);
 hit(ESC);sv.cash=330;assert(ncr_pay());load_save();assert(sv.cash==445&&!ncr_owed());check_stock(0,0,1);
#endif
}
static void gallery(void){
 nv.world[NVS_STAGE]=4;sv.cash=WALLET_LIMIT-1;sv.intro=4;
 walkin_open(nvs_aquaria[3]);hit(ENTER);assert(nvs_room==NVR_GLASS&&nvs_tab==3);hit(USE);assert(nvs_stage()==5);
 int postGrant=sv.cash;hit(FIRE);assert(nvs_mode==4);hit(USE);
 for(int q=1;q<4;q++){for(int i=0;i<74;i++)tick(RIGHT);tick(0);hit(USE);}
 assert(nvs_bonus==15);hit(ESC);
 if(observe){printf("Old aquarium $75 grant credited only $%d with $1 space; $30 stipend marked done at wallet ceiling; balance $%d.\n",postGrant-(WALLET_LIMIT-1),sv.cash);return;}
#ifdef DC_VENUE_AQUA_PAYOUT
 assert((nv.world[NVS_STAGE]&(NVS_GRANT_HELD|NVS_STIPEND_HELD))==(NVS_GRANT_HELD|NVS_STIPEND_HELD));
 assert(postGrant==WALLET_LIMIT-1&&sv.cash==postGrant);
 load_save();assert(wason&&nvs_stage()==5);
 sv.cash=WALLET_LIMIT-40;hit(USE);assert(sv.cash==WALLET_LIMIT-10&&!(nv.world[NVS_STAGE]&NVS_STIPEND_HELD)&&(nv.world[NVS_STAGE]&NVS_GRANT_HELD));
 sv.cash=200;hit(USE);assert(sv.cash==275&&!(nv.world[NVS_STAGE]&(NVS_GRANT_HELD|NVS_STIPEND_HELD)));
 hit(USE);assert(sv.cash==275);load_save();assert(wason&&sv.cash==275);
 hit(FIRE);hit(USE);for(int q=1;q<4;q++){for(int i=0;i<74;i++)tick(RIGHT);tick(0);hit(USE);}assert(sv.cash==275);hit(ESC);hit(ESC);
#endif
}
int main(int argc,char**argv){
 observe=argc>1&&!strcmp(argv[1],"--observe-baseline");
 rng_seed=argc>1?(U)strtoul(argv[1],0,0):21u;
 assert(getenv("XDG_DATA_HOME")&&linux_disk_init()&&linux_disk_lock());init_city();profiles_activate();
 ledgers();gallery();linux_disk_shutdown();puts(observe?"Baseline observations complete.":"PASS randomized ride/shelves + restarts, legacy ledger classification, conservation payouts at wallet cap.");return 0;
}
