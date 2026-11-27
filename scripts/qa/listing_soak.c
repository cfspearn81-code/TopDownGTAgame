/* Exercise all seventeen city listings via the same F6 and navigation inputs
 * as live play. This covers discoverability/ownership/escape; per-listing
 * outcome tests in test-city-listings.sh cover their actual authored endings. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
static void sk(U k){tick(k);tick(0);}
static void stopwork(void){
 if(vdPanel==2)sk(ESC);
 if(vdPanel)sk(ESC);
 if(vdActive>=0){sk(ACTIVITIES);assert(vdPanel==1);sk(DOWN);sk(ENTER);}
 if(job.kind>=0)end_job();
 assert(vdActive<0&&!vdPanel&&job.kind<0);
}
int main(int argc,char**argv){
 unsigned seed=argc>1?strtoul(argv[1],0,10):1,ran=0;
 init_city();starter_save();profiles_activate();scene=1;menu=-1;mapOn=0;sv.intro=4;sv.cash=1430;sv.dirty=12;sv.owned=1;sv.taxiFares=1;
 p.hp=100;p.dead=0;p.x=768;p.y=1038;
 for(int k=0;k<VA_COUNT;k++){
  const int target=(seed%VA_COUNT+7*k)%VA_COUNT;
  sk(ACTIVITIES);assert(vdPanel==1);while(vdSelected!=target)sk(DOWN);
  if((k+(seed>>6))%3==0){sk(ESC);sk(ACTIVITIES);assert(vdPanel==1&&vdSelected==target);}
  sk(ENTER);
  assert(vdActive==target||(target==VA_A3&&job.kind==JOB_TAXI));
  if((k+seed)%4==0&&vdPanel==2&&target!=VA_A3){if(seed%2)sk(RIGHT);sk(ENTER);}
  if(!(k%4))paint();
  stopwork();ran++;
  assert(sv.cash==1430&&sv.dirty==12);
 }
 printf("seed=%u checked=%d F6 listing launch/browse/cancel cases, including native-cab handoff; no unearned pay.\n",seed,ran);
}
