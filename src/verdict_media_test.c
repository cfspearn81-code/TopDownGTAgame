/* Test-only entrance and obstruction fixtures. F6/SDL and PE media-tour
 * recordings separately cover the walks, drives, operating both plates and
 * paying both camera calls. Build this source against the integrated src/. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <string.h>
static void mpress(U k){tick(k);tick(0);}
static void mstage(int id){walkin_open(id);wi_x=wi_f[0].x+wi_f[0].w/2;wi_y=wi_f[0].y+wi_f[0].h+12;assert(vm_station(id)&&!wi_solid(wi_x,wi_y));}
static void mchoice(int b){assert(vdPanel==2);if(b)mpress(DOWN);mpress(ENTER);}
static void mtitle(int b){mstage(b);mpress(ENTER);assert(vdPanel==2);}
int main(void){
 init_city();sv.intro=0;profiles_activate();mstage(VM_PRESS);
 mpress(ENTER);assert(vdActive<0&&!vdPanel);
 sv.intro=3;sv.legal=1;sv.cash=20;sv.dirty=27;sv.taxiFares=0;
 mpress(USE);assert(wi_sw[VM_PRESS]==1&&vdActive<0);
 mtitle(VM_PRESS);mchoice(1);assert(vm.printPhase==2);int x=vm.plateTick;
 mpress(HELP);for(int i=0;i<90;i++)tick(0);assert(vm.plateTick==x);mpress(HELP);
 mpress(ENTER);assert(vm.printPhase==3);mpress(ENTER);assert(vm.impression&&vm.quality==0&&sv.cash==65&&sv.legal==2&&vdActive<0);
 for(int i=0;i<6;i++)tick(0);assert(sv.cash==65);
 mtitle(VM_RECORDS);mchoice(1);assert(vm.booking==1&&vm.blockBill==1);vd_cancel();
 assert(vm.blockBill==1&&sv.cash==65); /* Announcement already ran; no fee without visit. */
 mtitle(VM_STUDIO);mchoice(0);int v=vm.van;assert(v==nc-1&&cars[v].type==4&&cars[v].owner==2);
 Car keep=cars[0];vd_cancel();assert(vm.van<0&&cars[v].x<0&&!memcmp(&keep,cars,sizeof keep));
 mtitle(VM_STUDIO);mchoice(1);v=vm.van;cars[v].cond=0;tick(0);assert(vm.newsPhase==0&&vm.van<0&&vdActive==VA_B7&&sv.cash==65);
 mpress(ENTER);mchoice(1);v=vm.van;assert(v>0&&cars[v].cond==100);
 scene=1;p.car=v;p.x=cars[v].x;p.y=cars[v].y;
 vd_cancel();assert(p.car==v&&cars[v].owner==2&&vm.returnVan==v);
 mpress(USE);tick(0);assert(p.car<0&&vm.returnVan<0&&cars[v].x<0&&!memcmp(&keep,cars,sizeof keep));
 sv.legal=8;vm.prevLegal=8;vm.prevKind=-1;job.kind=JOB_RECOVERY;vd_b8(VE_AMBIENT,0,0);job.kind=-1;vd_b8(VE_AMBIENT,0,0);assert(vm.historyN==0);
 job.kind=JOB_RECOVERY;vd_b8(VE_AMBIENT,0,0);job.kind=-1;sv.legal++;vd_b8(VE_AMBIENT,0,0);assert(vm.historyN&&vm.history[0].art==VM_HOOK);
 vd_reset();assert(!vm.impression&&!vm.historyN&&vm.blockBill<0&&vm.newsPage<0&&!vm.printPhase&&sv.cash==65&&sv.dirty==27);
 assert(sizeof sv==304);puts("Media: physical fixture entry, decline, dual plates, guide freeze, safe studio van cleanup / wreck / restage, history and career reset pass.");
 return 0;
}
