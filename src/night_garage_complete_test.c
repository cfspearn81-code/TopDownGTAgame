/* Additive garage/schedule regression. Run in the final expanded source. */
#define DC_TEST
#include "game.c"
#include <assert.h>
static void setup(void){
 init_city();starter_save();scene=1;mapOn=0;menu=-1;end_job();vd_reset();ng_reset();
 sv.intro=4;sv.cash=2400;sv.owned=1;sv.selection=0;sv.carCond[0]=100;
 nv.garage[0]=1;p.dead=0;p.car=-1;p.hp=100;clear_input();
}
static void meet_schedule(void){
 setup();int x,y;
 sv.dayClock=1250;ng_schedule_tick();assert(night_garage_meet_site(&x,&y)==0&&x==1280&&y==768);
 sv.dayClock=35;ng_schedule_tick();assert(night_garage_meet_site(&x,&y)==0);
 sv.dayClock=600;ng_schedule_tick();assert(night_garage_meet_site(&x,&y)==-1);
 sv.dayClock=1250;ng_schedule_tick();
 if(ng_courses()==5){
  assert(night_garage_meet_site(&x,&y)==1&&x==2690&&y==384&&!solid(x,y,15));
  sv.dayClock=630;ng_schedule_tick();sv.dayClock=1250;ng_schedule_tick();
  assert(night_garage_meet_site(&x,&y)==2&&x==1792&&y==2304&&!solid(x,y,15));
  for(int r=3;r<5;r++){
   ng.event=NG_RACE;ng.bookedCourse=r;
   for(int i=0;i<5;i++)assert(!solid(ngRun[r].gate[i].x,ngRun[r].gate[i].y,14));
   for(int i=0;i<110;i++){Car g;ng.frames=(ngRun[r].seconds*60*i)/110;assert(ng_ghost_car(&g)&&!solid(g.x,g.y,5));}
  }
 }else assert(night_garage_meet_site(&x,&y)==0);
}
static void rear_tags_and_editions(void){
 setup();ng_enter_panel(NG_BUILD,1);
 ng.idx=ng.part=6;ng.model=0;ng.val=3;int before=sv.cash;
 assert(ng_buy_part()&&ng_installed(0).tag==3&&sv.cash==before-35);
 int r=2,t=ngRun[r].type;sv.owned|=(1<<t)|(1<<1);sv.carCond[t]=100;
 nv.garage[1+t*6]=1;ng.bookedCourse=r;ng.frames=60;ng.paid=2;ng.spare=1;sv.owned&=~2;
 ng_finish_run();assert(ng_archive(r)&&ng_installed(t).paint==1&&sv.owned&2);
 ng_enter_panel(NG_COLLECTION,1);ng.chosen=r;ng.idx=1;ng_ui_accept();
 assert(ng.tab==NG_CONFIRM&&ng.idx==0);ng_ui_accept();assert(ng_installed(t).paint==1);
 ng.idx=1;ng_ui_accept();assert(ng.tab==NG_CONFIRM);ng.idx=1;ng_ui_accept();
 assert(ng_installed(t).paint==7&&ng_installed(t).kit==2&&ng_installed(0).tag==3);
}
static void sort_stock(void){
 setup();ng_enter_panel(NG_MAIN,1);assert(ng_begin_recovery());int id=ng.target;p.car=id;
 p.x=cars[id].x;p.y=cars[id].y;night_garage_street(0);assert(ng.event==NG_DROP);
 p.x=cars[id].x=NG_DOCK_X;p.y=cars[id].y=NG_DOCK_Y;cars[id].s=0;night_garage_street(ENTER);
 assert(scene==NIGHT_GARAGE_SCENE&&ng.tab==NG_RETURN);ng_ui_accept();assert(ng.tab==NG_SALVAGE);
 ng.idx=(ng.targetToken+1)%3;ng_ui_accept();assert(!ng.tokens&&cars[id].x>0);
 for(int i=0;i<3;i++){ng.idx=ng.targetToken;ng_ui_accept();}
 assert(scene==1&&ng.event==NG_IDLE&&ng_read(38,20)==3&&cars[id].x<0&&p.car<0);
}
static void drive_expanded(int route){
 if(ng_courses()!=5)return;
 setup();sv.dayClock=650;sv.owned=1u<<3;sv.selection=3;sv.carCond[3]=100;
 nc=1;z(streetDrivers,sizeof streetDrivers);
 /* Controlled course/geometry test: no ambient traffic in either fleet.
    The separate Outer Loop traffic regression tests physical interactions. */
 for(int i=0;i<OUT_TRAFFIC_COUNT;i++)outDriver[i]=(OutDriver){.driven=1,.outOfSight=-10000.f};
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
 static const NavPoint b[]={{2690,384},{2598,380},{2530,437},{2460,481},{2388,471},{2332,421},{2263,382},{2197,411},{2144,482},{2048,512},{1792,512},{1792,1024}};
 static const NavPoint h[]={{1792,2304},{1792,1792},{1024,1792},{1024,2144},{1136,2265},{1360,2321},{1520,2368},{1592,2464}};
 const NavPoint*q=route==3?b:h;int count=route==3?LEN(b):LEN(h);
 cars[0]=(Car){q[0].x,q[0].y,look(q[1].x-q[0].x,q[1].y-q[0].y),0,3,1,100};
 p.x=cars[0].x;p.y=cars[0].y;p.car=0;ng_enter_panel(NG_RUNS,1);ng.route=route;ng.meet=0;assert(ng_begin_race());
 for(int i=0;i<181;i++)tick(0);assert(ng.event==NG_RACE);
 int j=1,frames=0;
 for(;ng.event==NG_RACE&&j<count&&frames<3000;frames++){
  float dx=q[j].x-p.x,dy=q[j].y-p.y;
  if(d2(dx,dy)<(j==count-1?12*12:18*18)&&j<count-1){j++;dx=q[j].x-p.x;dy=q[j].y-p.y;}
  float err=wrap(look(dx,dy)-cars[0].a),dist=sq(d2(dx,dy));
  U keys=UP;float speed=absf(cars[0].s);
  if(absf(err)>.095f)keys|=err>0?RIGHT:LEFT;
  if(absf(err)>.5f||(dist<65&&j<count-1)||speed>124)keys|=BRAKE;
  tick(keys);
 }
 assert(ng.event==NG_IDLE&&ng_read(ng_best(route),36000)>0&&cars[0].cond>=98);
 printf("%s physical-input win in %.1fs, condition %d.\n",ngRun[route].name,ng_read(ng_best(route),36000)/60.0,cars[0].cond);
}
int main(void){meet_schedule();rear_tags_and_editions();sort_stock();drive_expanded(3);drive_expanded(4);puts("Garage stock-sort, style tag, rival archive, and correctly placed rotating paddocks passed.");return 0;}
