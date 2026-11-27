#define DC_TEST
#include "game.c"
#include <assert.h>
#include <string.h>

static void cart_probes(void){
 static const int times[]={1140,1290,0,150,300,1080,1140};
 static const NavPoint stops[]={{2172,1280},{2660,824},{2696,1480},{2240,2280},{2240,2280},{2240,2280},{2172,1280}};
 static const int last[]={1140,1290,0,150,150,150,1140};
 p.x=1024;p.y=1024;sv.intro=4;scene=1;nav_slot();
 sv.dayClock=1140;np_app=NP_WIRE;np_row=7;np_apply();
 assert(nav_cart_tracking()&&nav_span()==ROOM);
 for(int i=0;i<LEN(times);i++){
  sv.dayClock=(float)times[i];nav_update();
  assert(nav_cart_tracking()&&nav_same(navManual,stops[i])&&navCartClock==last[i]);
  int open=i<4||i==6;assert(navCartOpen==open);
  if(open)assert(navGoalKind==3&&nav_same(navGoal,stops[i])&&navCount>=2);
  else assert(!navGoalKind&&!navCount);
  NPLine line;np_line(&line,7);
  assert(strstr(line.suffix,open?"LIVE":"WAITING")&&strstr(line.detail,open?"Latest report":"Last report 02:30"));
 }
 p.x=navManual.x;p.y=navManual.y;nav_update();assert(nav_cart_tracking());
 /* Daytime can show the last report without steering over a job/personal pin. */
 sv.dayClock=500;sv.intro=1;nav_update();assert(navGoalKind==1&&nav_cart_tracking()&&!navCartOpen);
 sv.intro=4;np_apply();assert(!navHasManual);
 np_apply();assert(nav_cart_tracking());navigation_pointer(0,0,2);assert(!navHasManual);
 np_apply();assert(nav_cart_tracking());navSelected=1;nav_set();assert(navHasManual&&!nav_cart_tracking()&&navManualLoc==1);
 np_apply();assert(nav_cart_tracking());++sideSlot;nav_update();assert(!navHasManual);
 puts("Night Cart: all check-ins and 05:00 closing, dormant route fallback and last time, arrival, replace/clear/toggle and career isolation.");
}

static void patrol_probes(void){
 dispatch_reset();
 for(int i=0;i<DISPATCH_UNITS;i++)peds[DISPATCH_FIRST+i].hp=0;
 Ped*q=peds+DISPATCH_FIRST;
 q->hp=20;q->cool=999.f;q->x=1024;q->y=1024;dispatchUnits[0].state=D_RESPONSE;
 streetWalks[DISPATCH_FIRST]=(StreetWalk){.mx=q->x,.my=q->y};
 p.x=1600;p.y=1792;p.heat=120.f;
 dispatch_tick(1.f/60);assert(!dispatchPublicValid);
 p.x=1072;p.y=1024;
 assert(street_line_open(q->x,q->y,p.x,p.y,2));
 dispatch_tick(1.f/60);assert(dispatchSaw&&dispatchPublicValid);
 DispatchPoint observed=dispatchPublicLast;
 int x,y;assert(!dispatch_public_area(&x,&y));
 /* A hidden report still updates AI pursuit; it cannot leak into the map. */
 p.x=1408;p.y=1792;p.heat=140.f;
 for(int i=0;i<101;i++)dispatch_tick(1.f/60.f);
 assert(!dispatchSaw&&dispatchPublicLast.x==observed.x&&dispatchPublicLast.y==observed.y);
 assert(dispatch_public_area(&x,&y)==6&&x==(int)observed.x/96*96+48&&y==(int)observed.y/96*96+48);
 assert(x!=(int)p.x&&y!=(int)p.y&&x!=(int)observed.x);
 rect(0,0,W,H,0);const NavView v={256,50,448,448,0,0,ROOM};
 nav_search_ring(&v);
 U color=mix(0x101522,0x91b9ff,6);int paint=0;
 int cx=nav_x(&v,x),cy=nav_y(&v,y);
 for(int py=cy-19;py<=cy+19;py++)for(int px=cx-19;px<=cx+19;px++)paint+=pixels[py*W+px]==color;
 assert(paint>=40&&pixels[cy*W+cx]==0);
 for(int i=0;i<200;i++)dispatch_public_atlas_tick(1.f/60.f);
 assert(dispatch_public_area(&x,&y)==4);
 for(int i=0;i<210;i++)dispatch_tick(1.f/60.f);
 assert(dispatch_public_area(&x,&y)==2);
 for(int i=0;i<100;i++)dispatch_tick(1.f/60.f);
 assert(!dispatch_public_area(&x,&y));
 /* Blocked line within sight range is not a visual contact. */
 dispatch_reset();
 for(int i=1;i<DISPATCH_UNITS;i++)peds[DISPATCH_FIRST+i].hp=0;
 int blocked=0;
 for(int i=0;i<nb&&!blocked;i++){
  const StreetBounds b=building_bounds(buildings+i);if(b.w<12||b.w>148)continue;
  float bx=b.x-11.f,by=b.y+b.h*.5f,tx=b.x+b.w+11.f;
  if(open_space(bx,by)&&open_space(tx,by)&&!street_line_open(bx,by,tx,by,2)){
   blocked=1;q->x=bx;q->y=by;q->hp=20;q->cool=999.f;
   streetWalks[DISPATCH_FIRST]=(StreetWalk){.mx=q->x,.my=q->y};dispatchUnits[0].state=D_RESPONSE;
   p.x=tx;p.y=by;p.heat=120.f;
  }
 }
 assert(blocked);
 dispatch_tick(1.f/60.f);assert(!dispatchSaw&&!dispatchPublicValid);
 puts("Dispatch: no incident-call/hidden-player leak; genuine line of sight, wall rejection, broad atlas ring and 3-stage fade.");
}

static void district_probes(void){
 p.dead=0;p.heat=0;p.x=750;p.y=900;scene=1;menu=atmOpen=-1;mapOn=vdPanel=0;
 district_notice_reset();
 for(int i=0;i<240;i++){p.x=848+(i&3);district_notice_tick(1.f/60.f);}
 assert(!districtTTL&&!strcmp(districtStable,"TIDE MARKET"));
 p.x=920;
 for(int i=0;i<53;i++)district_notice_tick(1.f/60.f);
 assert(districtTTL>2.f&&!strcmp(districtStable,"NIGHTGLASS"));
 rect(0,0,W,H,0);
 district_notice_paint();int ink=0;
 for(int y=16;y<40;y++)for(int x=300;x<451;x++)ink+=pixels[y*W+x]!=0;
 assert(ink>100&&!pixels[18*W+467]);
 p.x=1480;
 for(int i=0;i<130;i++)district_notice_tick(1.f/60.f);
 assert(!strcmp(districtStable,"NIGHTGLASS")); /* no budget yet for second title */
 for(int i=0;i<232;i++)district_notice_tick(1.f/60.f);
 assert(districtTTL>2.f&&!strcmp(districtStable,"EAST FREIGHT"));
 rect(0,0,W,H,0);mapOn=1;district_notice_paint();assert(!pixels[16*W+450]);mapOn=0;
 for(int i=0;i<145;i++)district_notice_tick(1.f/60.f);
 assert(districtTTL<=0.f);
 p.x=2464;p.y=1024;district_notice_reset();
 assert(!strcmp(districtStable,"FREIGHT PARKWAY")&&!districtTTL);
 puts("District: boundary jitter suppressed, one title after stable arrival, repeat budget, expiry and overlay restraint.");
}
int main(void){
 init_city();p.dead=0;p.car=-1;p.hp=100;menu=atmOpen=-1;
 cart_probes();patrol_probes();district_probes();
}
