#define DC_TEST
#include "game.c"
#include <assert.h>
#include <string.h>
static void press(U k){tick(k);tick(0);}
static void walk(U k,int frames){for(int i=0;i<frames;i++)tick(k);tick(0);}
static void move_to(int x,int y){p.x=x;p.y=y;if(p.car>=0){cars[p.car].x=x;cars[p.car].y=y;}tick(0);}
static void frame(const char*name){paint();FILE*f=fopen(name,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);for(int j=0;j<W*H;j++){U v=pixels[j];fputc(v>>16,f);fputc(v>>8,f);fputc(v,f);}fclose(f);}
static void visit(int i){assert(scene==1&&p.car<0);move_to(loc[i].x,loc[i].y);press(USE);assert(scene==3&&interior_active_loc==i);}
static void counter(void){interior_x=interior_desk_x;interior_y=interior_desk_y;press(USE);}
static void leave(void){if(menu>=0)press(ESC);interior_x=W/2;interior_y=418;press(USE);assert(scene==1);}
static void dropoff(int i){visit(i);counter();assert(menu<0);leave();}
static int count_ink(U color,int xa,int ya,int xb,int yb){
 int n=0;for(int y=ya;y<yb;y++)for(int x=xa;x<xb;x++)n+=pixels[y*W+x]==color;
 return n;
}
static void visit_job_room(void){
 int target=job_room();assert(target>=0&&scene==1&&p.car<0);
 move_to(ordinaryDoorX[target],ordinaryDoorY[target]);press(USE);
 assert(scene==4&&walkin_active==target);
 const InteriorProp *f=wi_f;
 for(int y=f->y-26;y<=f->y+f->h+26;y+=4)for(int x=f->x-26;x<=f->x+f->w+26;x+=4){
  if(wi_solid(x,y))continue;
  wi_x=x;wi_y=y;
  if(wi_at_station()&&!wi_at_door())goto at_station;
 }
 assert(0);
at_station:
 press(USE);
 wi_x=W/2;wi_y=416;press(USE);assert(scene==1);
}
static void assert_officer_route(int i){
 Ped*q=peds+DISPATCH_FIRST+i;
 float x=absf(q->x-street_nearest_line(q->x)),y=absf(q->y-street_nearest_line(q->y));
 assert(!solid(q->x,q->y,4));
 assert(absf(x-STREET_WALK)<.01f||absf(y-STREET_WALK)<.01f);
 if(x<STREET_WALK-.01f||y<STREET_WALK-.01f)assert(streetWalks[DISPATCH_FIRST+i].cross);
}
static void sidewalk_patrol_test(void){
 init_city();p.heat=0;nc=0;
 int visited[DISPATCH_UNITS]={0};
 for(int frame=0;frame<1800;frame++){
  dispatch_tick(1.f/60);
  for(int i=0;i<DISPATCH_UNITS;i++){
   DispatchUnit*u=dispatchUnits+i;Ped*q=peds+DISPATCH_FIRST+i;
   assert_officer_route(i);
   if(d2(q->x-u->beat.x,q->y-u->beat.y)<1)visited[i]=1;
   if(u->station)for(int j=4;j<i;j++)assert(dispatch_distance(u->home,dispatchUnits[j].home)>=24);
  }
 }
 for(int i=0;i<DISPATCH_UNITS;i++)assert(visited[i]);
 /* A call from the middle of a road is answered from the pavement.
  * Then remove heat while units are away and follow them back home. */
 p.x=1024;p.y=1100;p.heat=180;p.hp=100;
 for(int frame=0;frame<1800;frame++){
  dispatch_tick(1.f/60);
  for(int i=0;i<DISPATCH_UNITS;i++)assert_officer_route(i);
 }
 assert(dispatch_visible()&&dispatch_engaged()==6);
 p.heat=0;
 for(int frame=0;frame<2400;frame++){
  dispatch_tick(1.f/60);
  for(int i=0;i<DISPATCH_UNITS;i++)assert_officer_route(i);
 }
 for(int i=0;i<DISPATCH_UNITS;i++)assert(dispatchUnits[i].state==D_POST);
 /* A moving player's car has priority before entry. A parked car on the
  * stripes also blocks entry. After it clears, ambient drivers yield. */
 Ped*q=peds+DISPATCH_FIRST;StreetWalk*w=streetWalks+DISPATCH_FIRST;
 q->x=474;q->y=550;*w=(StreetWalk){.mx=q->x,.my=q->y};
 DispatchPoint opposite={550,550};
 nc=2;p.car=1;cars[1]=(Car){502,485,1.570796f,90,0,0,100};
 for(int i=0;i<60;i++)dispatch_walk_to(0,opposite,1.f/60,37);
 assert(q->x==474&&!w->cross);
 cars[1].y=550;cars[1].s=0;
 for(int i=0;i<60;i++)dispatch_walk_to(0,opposite,1.f/60,37);
 assert(q->x==474&&!w->cross);
 cars[1].y=400;
 for(int i=0;i<35;i++)dispatch_walk_to(0,opposite,1.f/60,37);
 assert(w->cross&&q->x>480);
 Car approaching={q->x,520,1.570796f,65,0,0,100};
 assert(street_yield(&approaching,0,1));
 for(int i=0;i<120;i++){dispatch_walk_to(0,opposite,1.f/60,37);assert_officer_route(0);}
 assert(q->x==550&&q->y==550&&!w->cross);
 puts("Passed: sidewalk posts, neighborhood beats, pursuit, return, and crosswalk traffic.");
}
static void city_rhythm_test(void){
 init_city();scene=1;nc=0;
 int trees=0,shops=0,tram=-1;
 for(int tile=0;tile<64;tile++){
  BlockScenery*block=scenery+tile;
  for(int i=0;i<block->count;i++){
   StreetBounds b=block->walls[i];
   float x=b.x+b.w*.5f,y=b.y+b.h*.5f;
   assert(solid(b.x+1,b.y+1,0)&&solid(b.x+b.w-1,b.y+b.h-1,0));
   assert(!street_line_open(b.x-15,y,b.x+b.w+15,y,1));
   Car car={.x=x,.y=y,.a=.78f};assert(street_hits_wall(&car));
   if(i<block->treesFrom)continue;
   trees++;p.x=b.x-11;p.y=y;float before=p.x;
   for(int j=0;j<18;j++)street_player_step(2,0);
   assert(p.x>=before&&p.x<=b.x-10);
  }
  if(!lot[tile][0]){
   int x=(tile%8)*256,y=(tile/8)*256;
   assert(street_line_open(x+48,y+126,x+208,y+126,10));
   assert(street_line_open(x+126,y+48,x+126,y+208,10));
  }
 }
 assert(trees>0);
 for(int i=0;i<nb;i++){
  const Building*b=buildings+i;if(!b->w)continue;
  int roofY=max(b->y/256*256+42,b->y-b->ht);
  int east=min(b->x/256*256+213,b->x+b->w+b->ht/2-1);
  assert(solid(b->x+b->w/2,roofY+1,0));
  assert(solid(east,b->y+b->h/2,0));
  assert(street_hits_wall(&(Car){.x=east+2,.y=b->y+b->h/2,.a=.78f}));
 }
 for(int i=0;i<nb;i++)if(ordinaryDoorY[i]){
  shops++;p.x=ordinaryDoorX[i];p.y=ordinaryDoorY[i];assert(open_space(p.x,p.y));
  NavPoint door=nav_door(LEN(loc)+i);mapOn=1;
  navigation_pointer(NAV_MAP_X+door.x*NAV_MAP_SIZE/ROOM,NAV_MAP_Y+door.y*NAV_MAP_SIZE/ROOM,0);
  assert(navSelected==LEN(loc)+i);mapOn=0;
  NavAnchor connections[6];assert(nav_anchors(door,connections)>0);
  if(tram_hall(i))tram=i;
 }
 assert(shops==63&&tram>=0);
 walkin_open(tram);wi_x=wi_f[0].x+wi_f[0].w/2;wi_y=wi_f[0].y+wi_f[0].h+10;
 assert(wi_at_station());sv.cash=4;press(ENTER);assert(walkin_active==tram&&sv.cash==4);
 sv.cash=10;p.heat=50;press(ENTER);assert(walkin_active==tram&&sv.cash==10);p.heat=0;
 beginJob(JOB_COURIER);press(ENTER);assert(walkin_active==tram&&sv.cash==10);end_job();
 int stop=tram_destination(tram);press(ENTER);
 assert(walkin_active==stop&&sv.cash==5&&sv.x==ordinaryDoorX[stop]&&sv.y==ordinaryDoorY[stop]);
 press(USE);assert(scene==1&&p.x==ordinaryDoorX[stop]);
 sv.intro=4;sv.legal=1;beginJob(JOB_COURIER);int first=job_venue();assert(!job_has_clock());
 job_clock_tick(400);assert(job.kind==JOB_COURIER);
 for(int i=0;i<3;i++)assert(interior_job_drop(job_venue()));
 beginJob(JOB_COURIER);assert(job_venue()!=first);end_job();
 sv.owned=1;nc=1;p.car=0;p.x=900;p.y=512;cars[0]=(Car){900,512,0,125,0,1,100};
 press(USE);assert(p.car==0&&!stoppingToExit);
 cars[0].s=125;walk(USE,90);assert(p.car==-1&&absf(cars[0].s)<5);
 nc=2;cars[1]=(Car){p.x+24,p.y,0,0,1,0,100};p.heat=0;
 press(USE);assert(p.car<0&&!cars[1].reported&&p.heat==0);
 const Building*b=buildings+zone_idx[0];sv.x=b->x+b->w/2;sv.y=b->y-b->ht+2;
 spawnPlayer();assert(open_space(p.x,p.y)&&d2(p.x-sv.x,p.y-sv.y)<45*45);
 printf("Passed: %d tree trunks, roof edges, park paths, all storefront pins and approaches, trams, courier rotation, braking exits and old save positions.\n",trees);
}
int main(void){
 init_city();assert(nb<=LEN(buildings));assert(scene==0&&sv.cash==0&&sv.gear==-1&&sv.owned==0&&job.kind<0);
 frame("build/title.ppm");press(ENTER);assert(scene==5);press(ENTER);assert(scene==3&&interior_active_loc==12);walk(0,20);frame("build/apartment.ppm");
 walk(DOWN,47);press(USE);assert(scene==1&&sv.intro==1);
 paint();assert(navGoalKind==1&&navCount>=2);int distance=navDist;
 walk(LEFT,30);paint();assert(navDist<distance);
 move_to(loc[0].x,loc[0].y);assert(placeNear()==0);frame("build/free-roam.ppm");
 press(USE);assert(scene==3&&interior_active_loc==0&&menu<0);
 walk(UP,43);press(USE);assert(menu==0);frame("build/dispatch.ppm");press(ENTER);
 assert(job.kind==0&&menu<0&&sv.intro==2);leave();dropoff(2);assert(job.step==1);dropoff(3);assert(job.step==2);dropoff(0);
 assert(job.kind<0&&sv.legal==1&&sv.intro==3&&sv.cash>=115&&sv.dirty==0);
 visit(13);walk(0,20);frame("build/armory.ppm");counter();assert(menu==14);
 press(ENTER);assert(sv.gear==0&&sv.cash>=20);leave();
 visit(5);counter();assert(menu==5);press(ENTER);assert(!sv.b[1].own);leave();
 sv.cash=140;visit(3);counter();press(ENTER);assert(sv.owned&1);leave();
 move_to(cars[0].x,cars[0].y);press(ENTER);assert(p.car==0);
 Car saved=cars[1];cars[1]=(Car){cars[0].x+33,cars[0].y,0,0,1,0,100};
 assert(!street_move_car(0,5,0));cars[0].s=94;walk(UP,3);assert(cars[0].cond<100&&cars[0].cond>0);
 cars[1]=saved;cars[0].s=0;assert(street_exit_car());
 sv.cash+=40;visit(3);counter();doOpt(35);assert(cars[0].cond==100);leave();
 int moving=-1;for(int i=2;i<nc;i++)if(streetDrivers[i].live){moving=i;break;}assert(moving>0);
 move_to(cars[moving].x,cars[moving].y);cars[moving].s=35;press(ENTER);
 assert(p.car==moving&&!streetDrivers[moving].live&&p.heat>=38);cars[moving].s=0;assert(street_exit_car());
 /* Parking and re-entering an already reported car is the same theft.
  * It still cannot take taxi dispatch calls. */
 float theftHeat=p.heat;
 press(ENTER);assert(p.car==moving&&p.heat==theftHeat&&!cars[moving].owner);
 sv.taxiFares=1;press(TAXI_NEXT);assert(job.kind<0);sv.taxiFares=0;
 press(USE);assert(p.car<0);
 for(int i=0;i<LEN(loc);i++){
  const VenuePlan *v=zone_plan(i);int id=zone_idx[i];Building*b=buildings+id;
  assert(zone_is_building(id)&&zone_owner[v->gy*8+v->gx]==i&&solid(b->x+b->w/2,b->y+b->h/2,1)&&open_space(loc[i].x,loc[i].y));
  for(int j=i+1;j<LEN(loc);j++)assert(zone_idx[j]!=id);
  int road=(v->gy+1)*256;for(int y=loc[i].y;y<=road;y++)assert(open_space(loc[i].x,y));
 }
 for(int y=35;y<ROOM-35;y+=2)for(int x=35;x<ROOM-35;x+=2)
   if(x%256<=29||x%256>=227||y%256<=29||y%256>=227)assert(!roofMask[y*ROOM+x]);
 move_to(256,512);p.a=0;p.shot=0;p.heat=0;
 peds[0]=(Ped){275,512,0,0,2,0,0,0,0};press(PUNCH);assert(punchFlash>0&&peds[0].hp==1&&p.heat>=26&&p.trail<=0);
 dispatch_reset();p.heat=180;p.quiet=100;
 int seen=0;for(int i=0;i<130;i++){dispatch_tick(1.f/60);int n=dispatch_engaged();if(n)seen=1;assert(n<=6);if(i<40)assert(!n);}
 assert(seen&&dispatch_engaged()>=1);
 p.heat=0;for(int i=0;i<300;i++)dispatch_tick(1.f/60);assert(!dispatch_engaged());
 for(int i=0;i<200;i++)street_tick_civilian(0,1.f/60);
 for(int i=0;i<LEN(peds);i++)pedHit[i]=0;
 for(int j=0;j<nb;j++){Building*b=buildings+j;if(b->w>0&&b->h>0)assert(beam_block(b->x+2,b->y+3));}
 cx=0;cy=270;Car lamp={256,512,0,50,0,1,100};rect(0,0,W,H,0x19324c);projectLights(&lamp);
 U lit=pixels[(512-cy)*W+276];assert(lit>0x19324c);
 fastGfx=1;rect(0,0,W,H,0x19324c);projectLights(&lamp);assert(pixels[(512-cy)*W+276]==0x19324c);fastGfx=0;
 press(MAP);assert(mapOn);pointer_input(610,334,1);assert(navHasManual);frame("build/map.ppm");
 pointer_input(610,334,2);assert(!navHasManual);pointer_input(500,330,0);press(ENTER);assert(navHasManual);press(ENTER);assert(!navHasManual);
 press(ESC);assert(!mapOn);pointer_input(610,334,1);assert(!navHasManual);
 visit(4);walk(UP,90);assert(int_at_grow_gate());press(USE);assert(!interior_subroom);leave();
 sv.cash=2999;visit(4);counter();press(ENTER);assert(!sv.b[0].own);
 sv.cash=7500;press(ENTER);assert(sv.b[0].own&&sv.cash==4500);leave();
 visit(2);counter();press(ENTER);assert(sv.units>=3);leave();
 visit(4);counter();press(ENTER);assert(sv.b[0].timer==0);press(ESC);
 interior_x=GROW_GATE_X;interior_y=GROW_GATE_Y+12;press(USE);assert(lantern_terminal());
 counter();press(ENTER);assert(sv.b[0].timer>0&&menu==4);
 float growAtStart=growth_fraction(0);
 walk(0,119);assert(menu==4&&growth_fraction(0)>growAtStart&&absf(sv.b[0].timer-118.f)<.1f);
 int initialStock=sv.stocks[0];walk(0,7150);
 assert(menu==4&&sv.b[0].timer==0&&sv.stocks[0]==initialStock+prod[0].units&&sv.b[0].batches==1);
 walk(0,120);assert(sv.b[0].batches==1);
 press(ESC);interior_x=W/2;interior_y=418;press(USE);assert(!interior_subroom);leave();
 start_batch(&sv.b[0],4.f);
 press(ESC);assert(menu==PAUSE_MENU);
 float pausedBatch=sv.b[0].timer,pausedDay=sv.dayClock;U pausedTime=sv.playedTicks;
 walk(0,245);
 assert(sv.b[0].timer==pausedBatch&&sv.dayClock==pausedDay&&sv.playedTicks==pausedTime);
 press(ESC);press(HELP);assert(scene==2);
 walk(0,245);assert(sv.b[0].batches==1&&sv.b[0].timer>3.f);press(HELP);
 press(MAP);assert(mapOn);
 saveFlash=0;tick(SAVE);
 assert(mapOn&&saveFlash>0&&sv.hash==savehash());
 walk(0,245);assert(sv.b[0].timer==0&&sv.b[0].batches==2);press(ESC);
 visit(4);assert(!interior_subroom);walk(0,20);frame("build/fern-shop.ppm");
 walk(UP,90);assert(int_at_grow_gate());press(USE);assert(scene==3&&interior_subroom);
 start_batch(&sv.b[0],10.f);walk(0,25);frame("build/grow-young.ppm");walk(0,500);frame("build/grow-tall.ppm");
 assert(growth_fraction(0)>.80f);
 walk(DOWN,12);press(USE);assert(scene==3&&!interior_subroom);
 leave();
 visit(14);walk(0,20);frame("build/precinct.ppm");counter();assert(menu==15);press(ESC);
 interior_x=CHIEF_X;interior_y=CHIEF_Y;assert(!int_solid(interior_x,interior_y));press(USE);assert(notice[0]=='C');
 interior_x=CUSTODY_X;interior_y=CUSTODY_Y;assert(!int_solid(interior_x,interior_y));press(USE);assert(notice[0]=='C');leave();
 /* Swept gates only score during a circuit and from the registered car. */
 p.heat=0;sv.owned=1;sv.charges=1;sv.restitution=1;beginJob(JOB_SPRINT);
 assert(job.step==0&&!job_has_clock());
 move_to(job.xs[0],job.ys[0]);assert(job.step==0);
 for(int i=1;i<nc-1;i++){cars[i].x=1900;cars[i].y=1900;streetDrivers[i].live=0;}
 p.car=0;cars[0]=(Car){job.xs[0],job.ys[0],0,50,0,1,100};
 move_to(job.xs[0],job.ys[0]);assert(job.step==1&&job_has_clock()&&jobRide==0);
 cars[1]=(Car){0,0,0,50,0,2,100};p.car=1;
 move_to(job.xs[1],job.ys[1]);assert(job.step==1);
 cars[1].x=1900;cars[1].y=1900;cars[1].s=0;
 p.car=0;int paid=sv.cash;
 for(int gate=1;gate<job.total&&job.kind==JOB_SPRINT;gate++){
  cars[0].s=50;move_to(job.xs[gate],job.ys[gate]);
  assert(job.kind<0||job.step==gate+1);
 }
 assert(job.kind<0&&sv.cash==paid+205&&p.heat==0);
 assert(sv.charges&&sv.restitution==1);sv.charges=sv.restitution=0;
 p.car=-1;beginJob(1);assert(taxi.phase==TAXI_WAIT&&job.kind==1);
 p.car=0;cars[0]=(Car){job.xs[0],job.ys[0],0,100,0,1,100};p.x=cars[0].x;p.y=cars[0].y;
 taxi_tick(1.f/60);assert(taxi.phase==TAXI_WAIT&&job.step==0);
 cars[0].s=0;for(int i=0;i<75;i++)taxi_tick(1.f/60);
 assert(taxi.phase==TAXI_RIDING&&job.step==1);
 paid=sv.cash;p.x=job.xs[1];p.y=job.ys[1];cars[0].x=p.x;cars[0].y=p.y;cars[0].s=60;
 taxi_tick(1.f/60);assert(job.kind==1&&sv.cash==paid);
 cars[0].s=0;for(int i=0;i<75;i++)taxi_tick(1.f/60);
 assert(job.kind<0&&sv.cash>paid&&sv.taxiFares==1);
 int oldFrom=taxi.pickup,oldTo=taxi.dropoff,oldRider=taxi.rider;
 press(TAXI_NEXT);assert(job.kind==1&&taxi.pickup!=oldFrom&&taxi.pickup!=oldTo&&taxi.dropoff!=oldFrom&&taxi.dropoff!=oldTo&&taxi.rider!=oldRider);
 paint();assert(navGoal.x==job.xs[0]&&navGoal.y==job.ys[0]);int keep=taxi.pickup;
 press(TAXI_NEXT);assert(taxi.pickup==keep&&job.step==0);
 cars[0].s=0;move_to(job.xs[0],job.ys[0]);walk(0,75);
 assert(job.step==1&&taxi.phase==TAXI_RIDING);paint();assert(navGoal.x==job.xs[1]&&navGoal.y==job.ys[1]);
 int base=job.price,miles=taxi.meters;assert(base==60+miles/16);
 paid=sv.cash;move_to(job.xs[1],job.ys[1]);walk(0,75);assert(sv.taxiFares==2&&sv.cash-paid==base+taxi_tip_amount(base,0,0,0)&&job.kind<0);
 int shortTrip=taxi_road_distance(340,755,860,755),longTrip=taxi_road_distance(340,755,1830,1491);
 assert(longTrip>shortTrip&&(60+longTrip/16)>(60+shortTrip/16));
 assert(taxi_tip_amount(110,0,0,0)>taxi_tip_amount(110,1,16,1)&&taxi_tip_amount(110,5,100,6)==0);
 int seenLocations=0,seenRiders=0;
 for(int i=0;i<100;i++){
  oldFrom=taxi.pickup;oldTo=taxi.dropoff;oldRider=taxi.rider;assert(taxi_offer());
  assert(taxi.pickup!=oldFrom&&taxi.pickup!=oldTo&&taxi.dropoff!=oldFrom&&taxi.dropoff!=oldTo&&taxi.rider!=oldRider);
  for(int j=0;j<2;j++){int x,y,curb;assert(taxi_curb(j?taxi.dropoff:taxi.pickup,&x,&y,&curb));assert(x==job.xs[j]&&y==job.ys[j]&&curb==taxi.curb[j]);}
  seenLocations|=1<<taxi.pickup;seenRiders|=1<<taxi.rider;
 }
 assert(__builtin_popcount((U)seenLocations)>=10&&__builtin_popcount((U)seenRiders)==LEN(riders));job.kind=-1;
 p.car=-1;press(TAXI_NEXT);assert(job.kind<0);beginJob(1);p.car=0;cars[0].s=0;p.x=job.xs[0];p.y=job.ys[0];
 for(int i=0;i<75;i++)taxi_tick(1.f/60);
 assert(taxi.phase==TAXI_RIDING);paid=sv.cash;p.car=-1;taxi_tick(0);assert(job.kind<0&&sv.cash==paid);
 p.car=-1;
 assert(facilityPrice[0]==3000&&facilityPrice[1]==10000&&prod[0].time==120&&prod[2].time==120);
 assert(batch_units(0,2)*prod[0].price-batch_supply(0)*15==205);
 assert(batch_units(0,3)*prod[0].price-batch_supply(0)*15==359);
 assert(batch_units(2,3)*prod[2].price-batch_supply(1)*15>750);
 visit(0);interior_x=GROW_GATE_X;interior_y=GROW_GATE_Y+12;
 assert(int_at_staff_gate()&&!int_solid(interior_x,interior_y));press(USE);assert(!interior_subroom);
 counter();assert(menu==0);doOpt(LEGAL_OPEN);assert(menu==16);sv.cash=600;doOpt(LEGAL_BUY_FIRST);assert(sv.legit[0].owned&&sv.cash==240);press(ESC);
 interior_x=GROW_GATE_X;interior_y=GROW_GATE_Y+12;press(USE);assert(interior_subroom);
 walk(0,20);frame("build/rear-account.ppm");counter();assert(menu==19);
 press(ESC);interior_x=W/2;interior_y=418;press(USE);assert(scene==3&&!interior_subroom);leave();
 /* HUD: the health bar and the two balances must never share a row. */
 sv.cash=12345678;sv.dirty=23456789;sv.charges=1;sv.restitution=2;paint();
 int green=0,red=0,blue=0;
 for(int y=15;y<150;y++)for(int x=675;x<944;x++){
   U c=pixels[y*W+x];if(c==CASH_GREEN&&y<37)green++;if(c==CASH_RED&&y>=39&&y<64)red++;if(c==0x6ea7ee){assert(y>=70&&y<76);blue++;}
 }
 assert(green&&red&&blue);frame("build/economy-hud.ppm");
 gain(0,105,"Street sale.");assert(strstr(notice,"DIRTY MONEY +$105"));paint();
 assert(notice_color()==CASH_RED&&count_ink(CASH_RED,20,438,700,467)>30&&count_ink(CASH_GREEN,20,438,700,467)==0);
 p.car=0;paint();assert(count_ink(CASH_RED,650,40,944,63)>40);
 p.car=-1;visit(0);walk(0,20);gain(0,34,"Street sale.");paint();
 assert(count_ink(CASH_RED,650,30,944,55)>40&&count_ink(CASH_RED,20,483,730,512)>10);
 counter();gain(0,48,"Street sale.");paint();
 assert(count_ink(CASH_RED,650,30,944,55)>40);
 press(ESC);leave();
 int shells=0,portals=0;
 for(int i=0;i<nb;i++){
  Building *b=buildings+i;if(!b->w||!b->h)continue;shells++;
  if(zone_is_building(i))continue;
  assert(ordinaryDoorY[i]&&open_space(ordinaryDoorX[i],ordinaryDoorY[i]));
  move_to(ordinaryDoorX[i],ordinaryDoorY[i]);assert(ordinary_door_near()==i);
  press(USE);assert(scene==4&&walkin_active==i);paint();
  if(portals%10==0){char path[45];char*q=append(path,"build/walkin-");q=number(q,portals);append(q,".ppm");frame(path);}
  /* Return using the existing city-facing outside location, with no save change. */
  scene=1;portals++;
 }
 assert(shells==portals+LEN(loc)&&shells==78);
 for(int i=0;i<32;i++){
  float a=i*6.283186f/32;rect(0,0,W,H,DARK);person(480,260,0xc6d979,0,a,4,3);
  Pt h=rig(480,260,cs(a),sn(a),1,0,14);int whites=0;
  for(int y=216;y<=304;y++)for(int x=436;x<=524;x++)if(pixels[y*W+x]==0xffffff){
   assert(d2(x-h.x,y-h.y)>=36);whites++;
  }
  assert(whites>=28);
  if(i==24)frame("build/facing-north.ppm");
 }
 request_save();assert(savehash()==sv.hash&&saveFlash>0);

 /* Recovery requires a roadside restart and a stopped handoff in the same van. */
 init_city();scene=1;sv.intro=4;sv.charges=1;sv.restitution=2;
 visit(10);counter();press(ENTER);assert(job.kind==JOB_RECOVERY);leave();
 int van=jobRide;assert(van==nc-1&&cars[van].cond==0);
 paint();assert(navGoalKind==4&&navGoal.x==(int)cars[van].x);
 move_to(job.xs[0]-25,job.ys[0]);press(ENTER);assert(p.car<0&&job.step==0);
 press(USE);assert(job.step==1&&cars[van].cond==65);
 press(ENTER);assert(p.car==van);
 move_to(job.xs[1],job.ys[1]);assert(job.step==1&&sv.cash==0);
 cars[van].s=20;press(USE);assert(job.step==1&&p.car==van&&sv.cash==0);
 cars[van].s=0;
 press(USE);assert(job.kind<0&&p.car<0&&sv.cash==160&&!sv.charges&&cars[van].cond==0);

 /* Only keying the assigned Vectra can start the delivery leg. */
 init_city();scene=1;sv.intro=4;sv.owned=1;cars[0].cond=100;
 visit(1);counter();press(DOWN);press(ENTER);assert(job.kind==JOB_HOT);leave();
 int target=jobRide;
 paint();assert(navGoalKind==4&&navGoal.x==(int)cars[target].x);
 move_to(cars[1].x,cars[1].y+25);press(ENTER);
 assert(p.car==1&&job.step==0);press(USE);assert(p.car<0);
 move_to(cars[target].x-25,cars[target].y);press(ENTER);
 assert(job.step==1&&p.car==target&&job_vehicle_target()<0&&p.heat>=70);
 move_to(job.xs[1],job.ys[1]);press(USE);assert(job.step==1&&sv.dirty==0);
 p.heat=0;press(USE);assert(job.kind<0&&p.car<0&&sv.dirty==270);
 beginJob(JOB_HOT);target=jobRide;
 move_to(cars[target].x-25,cars[target].y);press(ENTER);assert(job.step==1);
 cars[target].cond=0;cars[target].s=30.f;tick(0);
 assert(job.kind<0&&p.car<0&&cars[target].s==0&&street_foot_clear(p.x,p.y,-1));
 p.car=0;cars[0].s=50.f;p.hit=0.f;p.hp=1;health(10);
 assert(p.dead&&cars[0].s==0);

 /* The vault and signal jobs use the interactive fixture inside the right room. */
 init_city();scene=1;sv.intro=4;sv.gear=1;
 visit(1);counter();press(ENTER);assert(job.kind==JOB_VAULT);leave();
 move_to(job.xs[0],job.ys[0]);assert(job.step==0&&p.heat==0);
 visit_job_room();
 assert(job.step==1&&p.heat>0&&sv.dirty==0);
 visit(1);counter();assert(job.step==1&&menu<0&&sv.dirty==0);leave();
 p.heat=0;visit(1);counter();assert(job.kind<0&&sv.dirty==260);leave();

 visit(1);counter();press(DOWN);press(DOWN);press(ENTER);
 assert(job.kind==JOB_SIGNAL&&jobBuildings[0]!=jobBuildings[1]);leave();
 visit_job_room();assert(job.kind==JOB_SIGNAL&&job.step==1);
 move_to(job.xs[1],job.ys[1]);assert(job.step==1&&sv.dirty==260);
 visit_job_room();assert(job.kind<0&&sv.dirty==560);

 /* The crew guards a physical cache. Use the armory weapon to get to it. */
 visit(1);counter();press(DOWN);press(DOWN);press(DOWN);press(ENTER);
 assert(job.kind==JOB_RIVALS);leave();
 move_to(job.xs[0],job.ys[0]);assert(rivalsDeployed&&job_rivals_active());
 press(USE);assert(job.step==0&&sv.dirty==560);
 for(int i=JOB_RIVAL_FIRST;i<JOB_RIVAL_FIRST+JOB_RIVAL_COUNT;i++){
  Ped *guard=peds+i;
  if(guard->hp<=0)continue;
  p.x=guard->x-35;p.y=guard->y;p.a=0;p.shot=0;
  press(FIRE);assert(guard->hp<=0);
 }
 move_to(job.xs[0],job.ys[0]);press(USE);assert(job.step==1&&sv.dirty==560);
 visit(7);counter();assert(job.kind<0&&sv.dirty==920&&sv.crime==3);leave();

 /* A safehouse requires time inside and becomes the recovery destination. */
 sv.cash=600;visit(9);counter();press(ENTER);assert(sv.home&&sv.cash==50);
 p.hp=40;heat(90);press(ENTER);assert(shelterT>0&&p.hp==40&&p.heat>=90);
 walk(0,610);assert(shelterT==0&&p.hp==100&&p.heat==0);leave();
 p.hp=1;p.hit=0;health(10);walk(0,50);press(ENTER);
 assert(!p.dead&&sv.cash==50&&d2(p.x-loc[9].x,p.y-loc[9].y)<9);
 /* A lethal wreck next to a venue must finish the downed transition
  * even when the player presses the interact key on that tick. */
 init_city();scene=1;sv.owned=1;p.car=0;p.hp=1;
 cars[0]=(Car){loc[0].x,loc[0].y,0,0,0,1,0};
 tick(USE);assert(p.dead&&scene==1&&menu<0);
 walk(0,50);press(ENTER);assert(!p.dead&&scene==1);

 /* An available parked car wins over a closer fast-moving vehicle. */
 p.x=p.y=1024;
 cars[1]=(Car){1024,1044,0,75,0,0,100};
 cars[2]=(Car){1054,1024,0,0,0,0,100};
 assert(street_near_car()==2);
 cars[2].cond=0;assert(street_near_car()==1);
 press(ESC);assert(menu==PAUSE_MENU);press(MAP);assert(!mapOn&&menu==PAUSE_MENU);press(ESC);
 /* Lead follows travel in both directions and the world edges stay bounded. */
 p.car=0;cars[0]=(Car){1024,1024,0,150,0,1,100};
 for(int i=0;i<120;i++)update_camera_lead();
 assert(camera_left()>1024-W/2+60&&camera_top()==1024-H/2);
 cars[0].s=-50;for(int i=0;i<120;i++)update_camera_lead();
 assert(camera_left()<1024-W/2);p.x=35;assert(camera_left()==0);
 spawnPlayer();assert(cameraLeadX==0&&cameraLeadY==0);
 city_rhythm_test();
 sidewalk_patrol_test();
 puts("Passed: vehicle re-entry, parked car selection, pause/map isolation, lethal wreck interaction and driving camera.");
 puts("Passed: stopped recovery handoff, licensed circuit, exact Vectra, two radio studios, carried lockbox, guarded consignment, restitution and safehouse recovery.");
 puts("Passed: apartment spawn with zero cash/fists; doorway/counter walk; three indoor deliveries; weapon purchase; property batch while menu and map stay open; solid car, crash and repair; moving car theft; punch; finite dispatch; warm lights and low FX; GPS/manual route, automatic foot and driving checkpoints, taxi boarding/drop-off/cancellation plus T and 100 varied offers, wallet semantics, 78 building paths, and save checksum.");
}
