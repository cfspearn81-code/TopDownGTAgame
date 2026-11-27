/* Complete ordinary game actions from test-only curb fixtures, through the
   same WASD, E and ENTER paths as a real career; no disk save is opened. */
#define main original_interior_routes
#include "interior_routes_test.c"
#undef main
#include <assert.h>

static void enter_venue(int index) {
 assert(scene==1&&p.car<0);
 room_name=loc[index].label;
 g_kind=loc[index].kind;
 p.x=loc[index].x;
 p.y=loc[index].y;
 tick(0);
 press_key(USE);
 assert(scene==3&&interior_active_loc==index&&ch_mode_active());
}
static void walk_to_counter(void) {
 go(DESK_G);
 paint();
 assert(ch_mode_active());
 press_key(USE);
}
static void leave_venue(void) {
 if(menu>=0)press_key(ESC);
 go(EXIT_G);
 press_key(USE);
 assert(scene==1&&ch_mode_active());
}
int main(void) {
 init_city();
 scene=1;
 menu=-1;
 sv.intro=4;
 p.car=-1;
 sv.cash=500;
 fastGfx=0;
 end_job();
 assert(!ch_mode_active());
 press_key(CAMERA);
 assert(ch_mode_active());

 enter_venue(2);
 int cash=sv.cash,credits=sv.units;
 walk_to_counter();
 assert(menu==2);
 press_key(ENTER);
 assert(sv.cash==cash-45&&sv.units==credits+3);
 leave_venue();

 enter_venue(0);
 cash=sv.cash;
 int shifts=sv.legal;
 walk_to_counter();
 assert(menu==0);
 press_key(ENTER);
 assert(job.kind==JOB_COURIER&&job.step==0);
 leave_venue();
 for(int j=0;j<=2;j++) {
  int dst=job_venue();
  assert(dst>=0&&dst<LEN(loc));
  enter_venue(dst);
  walk_to_counter();
  assert(menu<0);
  if(j<2)assert(job.kind==JOB_COURIER&&job.step==j+1&&sv.cash==cash);
  leave_venue();
 }
 assert(job.kind<0&&sv.legal==shifts+1&&sv.cash>cash);
 printf("3D Ferry Market purchase and paid Dispatch courier with %d indoor key ticks; counter and both deliveries used E, menus used ENTER.\n",steps_taken);
 return 0;
}
