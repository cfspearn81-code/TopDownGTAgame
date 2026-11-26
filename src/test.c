#define DC_TEST
#include "game.c"
#include <assert.h>
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
 sv.cash=2999;visit(4);counter();press(ENTER);assert(!sv.b[0].own);sv.cash=7500;press(ENTER);assert(sv.b[0].own&&sv.cash==4500);leave();visit(2);counter();press(ENTER);assert(sv.units>=3);leave();visit(4);counter();press(ENTER);assert(sv.b[0].timer>0&&menu==4);float growAtStart=growth_fraction(0);
 walk(0,119);assert(menu==4&&growth_fraction(0)>growAtStart&&absf(sv.b[0].timer-118.f)<.1f);
 int initialStock=sv.stocks[0];walk(0,7150);
 assert(menu==4&&sv.b[0].timer==0&&sv.stocks[0]==initialStock+prod[0].units&&sv.b[0].batches==1);
 walk(0,120);assert(sv.b[0].batches==1);leave();
 start_batch(&sv.b[0],4.f);press(MAP);assert(mapOn);walk(0,245);assert(sv.b[0].timer==0&&sv.b[0].batches==2);press(ESC);
 visit(4);assert(!interior_subroom);walk(0,20);frame("build/fern-shop.ppm");
 walk(UP,90);assert(int_at_grow_gate());press(USE);assert(scene==3&&interior_subroom);
 start_batch(&sv.b[0],10.f);walk(0,25);frame("build/grow-young.ppm");walk(0,500);frame("build/grow-tall.ppm");
 assert(growth_fraction(0)>.80f);
 walk(DOWN,12);press(USE);assert(scene==3&&!interior_subroom);
 leave();
 visit(14);walk(0,20);frame("build/precinct.ppm");counter();assert(menu==15);press(ESC);
 interior_x=CHIEF_X;interior_y=CHIEF_Y;assert(!int_solid(interior_x,interior_y));press(USE);assert(notice[0]=='C');
 interior_x=CUSTODY_X;interior_y=CUSTODY_Y;assert(!int_solid(interior_x,interior_y));press(USE);assert(notice[0]=='C');leave();
 /* No USE input at any outdoor point. */
 p.heat=0;job=(Job){4,0,2,220,100,{512,768},{512,512}};
 move_to(470,512);walk(RIGHT,6);assert(job.step==1);paint();assert(navGoal.x==768&&navGoal.y==512);
 walk(0,20);assert(job.step==1);
 int paid=sv.dirty;move_to(727,512);walk(RIGHT,6);assert(job.kind<0&&sv.dirty>paid);
 for(int i=0;i<nc;i++){cars[i].x=1900;cars[i].y=1900;streetDrivers[i].live=0;}
 job=(Job){2,0,1,97,100,{1024},{512}};
 move_to(1024,512);assert(job.step==0);
 p.car=0;cars[0]=(Car){987,512,0,145,0,1,100};p.x=987;p.y=512;paid=sv.cash;
 walk(UP,3);assert(job.kind<0&&sv.cash>paid);
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
 Job sample={6,0,1,10,50,{1024},{512}};job=sample;p.x=1064;p.y=512;
 check_arrival(985,512);assert(job.kind<0);
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
 gain(0,105,"Street sale.");paint();
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
 puts("Passed: apartment spawn with zero cash/fists; doorway/counter walk; three indoor deliveries; weapon purchase; property batch while menu and map stay open; solid car, crash and repair; moving car theft; punch; finite dispatch; warm lights and low FX; GPS/manual route, automatic foot and driving checkpoints, taxi boarding/drop-off/cancellation plus T and 100 varied offers, wallet semantics, 78 building paths, and save checksum.");
}
