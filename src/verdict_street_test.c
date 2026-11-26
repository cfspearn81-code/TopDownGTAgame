/* Compile against the integrated native core. World state is staged at the
 * start of each case; actions, movement, traffic, dispatch and payouts tick. */
#define DC_TEST
#include "game.c"
#include <stdio.h>
#include <assert.h>
#include <string.h>
static void b6_press(U k){tick(k);tick(0);}
static void b6_run(U k,int n){while(n--)tick(k);}
static void b6_at(int x,int y){p.x=x;p.y=y;tick(0);}
static void b6_start(void){
 init_city();scene=1;menu=-1;mapOn=0;p.dead=0;sv.intro=4;p.car=-1;sv.cash=0;sv.dirty=0;sv.charges=0;prev=0;end_job();
}
static void b6_select(int id){b6_press(ACTIVITIES);assert(vdPanel==1);for(int i=0;i<id;i++)b6_press(DOWN);b6_press(ENTER);assert(vdActive==id);}
static void b6_cancel(void){b6_press(ACTIVITIES);b6_press(DOWN);b6_press(ENTER);assert(vdActive==-1);}
static void b6_drop(int route){
 b6_start();b6_at(SW_PICK_X,SW_Y);b6_select(VA_B5);b6_press(USE);assert(vdPanel==2&&p.heat==0);
 if(route==2)b6_press(DOWN);
 b6_press(ENTER);assert(swTape.target==route&&p.heat>0&&strstr(vdLine,"WAIT")&&strstr(vdLine,"READY"));
 if(route==1){b6_run(LEFT|BRAKE,66);assert(sw_at(SW_NEAR_X,SW_Y,32));b6_press(USE);assert(vdActive==VA_B5&&sv.dirty==0);
  b6_run(0,413);assert(swTape.clock>=480);
 }else{b6_run(RIGHT|BRAKE,139);assert(sw_at(SW_FAR_X,SW_Y,33));assert(p.heat>0&&dispatch_engaged()>0);}
 b6_press(USE);assert(vdActive<0&&sv.dirty==90&&sv.cash==0&&p.hp>0);b6_press(USE);assert(sv.dirty==90);
 printf("switchboard route %d: $%d dirty, heat %.1f, no combat; legal shifts unchanged\n",route,sv.dirty,p.heat);
}
static int b6_gate(void){
 for(int i=0;i<600;i++){
  if(!cr_active_cart()&&!cr.worker&&!cr_walkers_in_box())return i;
  tick(0);
 }
 fprintf(stderr,"blocked gate: served %d/%d, worker %d, ticks %d, xy %.0f,%.0f\n",cr.served[0],cr.served[1],cr.worker,cr.ticks,p.x,p.y);assert(0);return 0;
}
static void b6_keeper(void){
 b6_start();b6_at(CR_BOOTH_X,CR_BOOTH_Y);b6_select(VA_B6A);b6_press(USE);assert(cr.locked);
 b6_press(USE);assert(cr_active_cart());
 int i=0;while(!cr.request&&i++<480)tick(0);assert(cr.request);
 b6_press(USE);assert(cr.request&&!cr_active_cart());b6_press(ENTER);assert(cr.worker&&!cr.request);
 b6_press(ENTER);assert(!cr_active_cart());printf("pedestrian WALK kept both tugs stopped\n");
 b6_gate();b6_press(ENTER);b6_gate();b6_press(ENTER);b6_gate();assert(cr.request);b6_press(ENTER);b6_gate();b6_press(USE);
 for(i=0;i<400&&vdActive>=0;i++)tick(0);
 assert(vdActive<0&&!cr.locked&&sv.cash==75);
 printf("Keeper released two west and two east tugs; $75 clean exactly once\n");
 b6_start();b6_at(CR_BOOTH_X,CR_BOOTH_Y);b6_select(VA_B6A);b6_press(USE);b6_run(DOWN|BRAKE,42);
 assert(vdActive<0&&!cr.locked&&sv.cash==0);
 /* Physical gates don't engage elsewhere and return to stock immediately. */
 b6_start();b6_at(CR_BOOTH_X,CR_BOOTH_Y);b6_select(VA_B6A);b6_press(USE);assert(cr.locked);
 Car in={CR_X,CR_Y-72,1.57f,45},other={CR_X-256,CR_Y-72,1.57f,45};
 assert(vd_cross_hold(&in,0,1)&&!vd_cross_hold(&other,0,1));
 b6_cancel();assert(!vd_cross_hold(&in,0,1)&&!cr.locked);
 printf("Walking out and cancellation reset local control; adjacent junction unchanged\n");
}
static void b6_driver_init(int startY,int active){
 b6_start();sv.owned=1;cars[0]=(Car){CR_X,(float)startY,1.570796f,0,0,1,100};
 p.car=0;p.x=cars[0].x;p.y=cars[0].y;p.a=cars[0].a;sv.playedTicks=active?600:7300;
 if(active)b6_select(VA_B6B);
 tick(0);assert(cr.mode==CR_CONVOY&&cr.locked);
}
static void b6_wait(int active){
 b6_driver_init(CR_Y-143,active);int sawRed=0,roadReleased=0,sawBoth=0;
 for(int i=0;i<1800;i++){
  tick(UP);
  if(cr.served[0]==2)sawBoth=1;
  if(cr.locked){assert(p.y<CR_Y-62.9f);if(cars[0].s==0&&p.y>CR_Y-67)sawRed=1;}
  else roadReleased=1;
  if(cr.mode==CR_OFF)break;
 }
 if(cr.mode)fprintf(stderr,"convoy truth: mode %d lock %d served %d y%.1f s%.1f tick%d\n",cr.mode,cr.locked,cr.served[0],p.y,cars[0].s,cr.ticks);
 assert(sawRed&&sawBoth&&roadReleased&&cr.mode==CR_OFF&&vdActive==-1&&p.y>CR_Y+55&&cars[0].cond==100&&sv.cash==0);
 printf("%s driver: full-throttle approach auto-stopped safely, two tugs cleared, gate reopened\n",active?"Listed":"Ordinary");
}
static void b6_detour(void){
 b6_driver_init(CR_Y-274,1);
 /* Directed steering commands only; stock car controls turn left from
  * south to west, then right from west to south at the bypass road. */
 int ticks=0;while(cars[0].a<3.07f&&ticks++<300)tick(UP|RIGHT|BRAKE);
 printf("turn1: %.0f %.0f heading %.2f\n",p.x,p.y,p.a);
 ticks=0;while(p.x>CR_X-245&&ticks++<270)tick(UP);
 printf("approach: %.0f %.0f heading %.2f\n",p.x,p.y,p.a);
 while(cars[0].a>1.60f&&ticks++<460)tick(UP|LEFT|BRAKE);
 int n=0;while(cr.mode==CR_CONVOY&&n++<420)tick(UP);
 printf("detour: %.0f %.0f heading %.2f notice %s\n",p.x,p.y,p.a,notice);
 assert(vdActive<0&&cr.mode==CR_OFF&&strstr(notice,"West bypass")&&sv.cash==0);
}
static void b6_pause_and_reload(void){
 b6_start();b6_at(CR_BOOTH_X,CR_BOOTH_Y);b6_select(VA_B6A);b6_press(USE);b6_press(USE);
 Car in={CR_X,CR_Y-72,1.57f,45};int t=cr.ticks;float x=cr.tug[0].x;
 b6_press(ESC);assert(menu==PAUSE_MENU&&!vd_cross_hold(&in,0,1));b6_run(0,90);assert(cr.ticks==t&&cr.tug[0].x==x);
 b6_press(ESC);assert(menu<0&&cr.locked);
 b6_press(SAVE);assert(sv.rev==10&&sv.cash==0);
 init_city();assert(!cr.locked&&cr.mode==CR_OFF&&vdActive<0&&!vd_cross_hold(&in,0,1));
 puts("Pause freezes freight; reset clears an in-progress shift without changing Rev10.");
}
int main(void){
 b6_drop(2);b6_drop(1);b6_keeper();b6_wait(1);b6_wait(0);b6_detour();b6_pause_and_reload();puts("Street listings tests passed.");return 0;
}
