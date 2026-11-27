/* Curb/quote edge matrix. The companion SDL+PE tape supplies a whole drive
 * with real player steering; this script only stages starting conditions. */
#define _POSIX_C_SOURCE 200809L
#define DC_LINUX_NATIVE
#include "game.c"
#include "linux_save.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static void hit(U key){tick(0);tick(key);tick(0);}
static void park(NavPoint q,int owner,float speed){
 Car*c=cars+0;c->owner=owner;c->cond=100;c->x=q.x;c->y=q.y;c->a=0;c->s=speed;
 p.x=c->x;p.y=c->y;p.a=c->a;p.car=0;p.dead=0;p.heat=0;
}
static void promoter(void){
 int b=night_club_listing();assert(night_club_open(b));p.car=-1;p.dead=0;
 ncX=276;ncY=371;assert(ncr_near()==NCR_PROMOTER);hit(USE);
 assert(ncPanel==NC_REGULAR&&ncChoice==1&&ncr_contact==NCR_PROMOTER);
}
static void accept(int i){
 promoter();assert(ncr_quote==i);
 if(i==0)sv.dayClock=500;  /* Changing the hour never changes an open quote. */
 hit(DOWN);hit(ENTER);assert(ncr.phase==NCR_WAIT&&ncr.show==i&&!ncr_owed());
 ncX=480;ncY=423;hit(USE);assert(scene==1);
}
static void frames(int count){while(count--)tick(0);}
int main(void){
 assert(getenv("XDG_DATA_HOME")&&linux_disk_init()&&linux_disk_lock());init_city();profiles_activate();
 sv.intro=4;sv.owned=1;p.hp=100;
 for(int i=0;i<LEN(ncr_shows);i++)for(int j=0;j<12;j++){
  ncr_reset();sv.dayClock=i*480+20;sv.cash=500;
  accept(i);
  /* Stamp no quest save, prevent ordinary jobs from overlapping, and respect previous pins. */
  assert(ownedJob());NavPoint goal;int venue;
  navHasManual=1;navManualLoc=-1;navManual=(NavPoint){640,768};assert(nav_objective(&goal,&venue)==3);
  navHasManual=0;assert(nav_objective(&goal,&venue)==8&&nav_same(goal,ncr.pick));
  park(ncr.pick,0,0);frames(65);assert(ncr.phase==NCR_WAIT);
  park(ncr.drop,1,0);frames(65);assert(ncr.phase==NCR_WAIT&&sv.cash==500);
  park(ncr.pick,1,0);frames(65);assert(ncr.phase==NCR_DRIVE);
  if(j==0){p.car=-1;frames(2);assert(!ncr_active()&&sv.cash==500&&!ncr_owed());continue;}
  assert(nav_objective(&goal,&venue)==8&&nav_same(goal,ncr.drop));
  if(j==1)sv.cash=WALLET_LIMIT;
  park(ncr.drop,1,0);frames(90);assert(!ncr_active());
  if(j==1){
   assert(sv.cash==WALLET_LIMIT&&ncr_owed()==ncr_shows[i].fee&&saveFlash>0);
   load_save();assert(wason&&ncr_owed()==ncr_shows[i].fee);
   sv.cash=500;promoter();hit(DOWN);hit(ENTER);
   assert(ncPanel==NC_NONE&&!ncr_owed());
   scene=1;
  }
  frames(260);assert(ncr.phase==NCR_OFF&&sv.cash==500+ncr_shows[i].fee&&!ncr_owed()&&sv.taxiFares==0);
 }
 sv.dayClock=25;accept(0);
 promoter();hit(DOWN);hit(ENTER);assert(ncr.phase==NCR_OFF&&ncPanel==NC_NONE);
 nv.world[NCR_OWED_WORD]=0xffffffffu;assert(!ncr_owed());
 puts("PASS: 36 curb fixtures, fixed quotes at rollover, personal pin precedence, unlicensed refusal, wrong-curb refusal, 3 exits, 3 persisted wallet deferrals, 33 exact one-time payouts, and booking cancellation.");
 linux_disk_shutdown();return 0;
}
