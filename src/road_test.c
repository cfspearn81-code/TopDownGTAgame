/* Fixed street arrangements with keyboard actions through the gameplay tick.
 * The unguided city soak and recorded native career run complement these. */
#define DC_TEST
#include "game.c"
#include <assert.h>
static void reset_block(void){
 scene=1;menu=-1;mapOn=0;clear_input();bank_ui_reset();end_job();spawnCars();
 starter_save();sv.intro=4;sv.gear=0;
 p=(Player){.x=1024,.y=1024,.car=-1,.hp=100};p.a=1.570796f;
 nc=3;z(cars,sizeof cars);z(streetDrivers,sizeof streetDrivers);
 for(int i=0;i<LEN(cars);i++)cars[i].x=cars[i].y=-100;
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
}
static void play(U keys,int frames){while(frames--)tick(keys);}
static void road_cover(void){
 reset_block();cars[1]=(Car){1024,1049,.30f,0,1,0,100};
 int target=JOB_RIVAL_FIRST;
 peds[target]=(Ped){1024,1071,0,0,3,2,0,0,0};
 tick(FIRE);assert(peds[target].hp==3&&p.hp==100);
 assert(trailY==firedPulse.my&&trailY>1024&&trailEndY>1036&&trailEndY<1046);
 assert(trailX==firedPulse.mx&&trailEndY>=trailY);
 /* The visible trace stays where the shot happened even if we turn. */
 float tx=trailEndX,ty=trailEndY;tick(RIGHT);assert(tx==trailEndX&&ty==trailEndY);
 play(0,80);assert(p.hp==100&&street_foot_car_depth(cars+1,peds[target].x,peds[target].y,7)<.001f);
 /* Moving the occluder off-axis makes the same attack valid. */
 cars[1].x+=96;p.a=1.570796f;tick(FIRE);assert(peds[target].hp==2&&p.hp<100);
 reset_block();cars[1]=(Car){1024,1051,.1f,0,4,0,0};p.heat=60;p.quiet=20;
 peds[DISPATCH_FIRST]=(Ped){1024,1077,0,0,3,1};dispatchUnits[0].state=D_RESPONSE;
 tick(0);assert(p.hp==100&&dispatch_visible());
 cars[1].x+=100;tick(0);assert(p.hp<100);
 reset_block();p.y=1003;
 cars[1]=(Car){1024,1021,0,0,2,0,0};cars[2]=(Car){1024,1040,0,0,1,0,100};
 tick(USE);assert(p.car<0); /* No reaching straight through a wreck. */
 play(RIGHT,18);play(DOWN,21);tick(USE);assert(p.car==2);
 tick(0);tick(USE);assert(p.car<0&&street_foot_clear(p.x,p.y,-1));
 reset_block();sv.owned=1;sv.x=sv.y=1024;cars[0]=(Car){1024,1024,0,0,0,1,100};
 spawnPlayer();assert(p.car<0&&street_foot_clear(p.x,p.y,-1));
 assert(d2(p.x-1024,p.y-1024)<=32*32);
}
static int has_ink(U color){
 for(int y=0;y<H;y++)for(int x=0;x<W;x++)if(pixels[y*W+x]==color)return 1;
 return 0;
}
static void lamp_frame(void){rect(0,0,W,H,0x123456);carbody(cars+1,160,100);}
static void brake_and_lights(void){
 reset_block();p.y=1002;cars[1]=(Car){1024,1024,0,0,0,0,100};
 tick(USE);assert(p.car==1);play(UP,78);assert(cars[1].s>117);lamp_frame();
 assert(!has_ink(0xff5148)&&!has_ink(0xe2faff));
 tick(UP|RIGHT|BRAKE);assert(roadCount>0);lamp_frame();assert(has_ink(0xff5148));
 RoadMark first=roadMarks[0];int x=first.ax[0],y=first.ay[0];
 rect(0,0,W,H,0x34485c);roadFX_paint(x-100,y-100);assert(pixels[100*W+100]!=0x34485c);
 play(BRAKE,45);play(DOWN,45);assert(cars[1].s< -15);lamp_frame();assert(has_ink(0xe2faff)&&!has_ink(0xff5148));
 cars[1].cond=0;lamp_frame();assert(!has_ink(0xffebbf)&&!has_ink(0xff5148)&&!has_ink(0xe2faff));
 for(int i=0;i<=ROAD_MARK_LIFE;i++)roadFX_tick();assert(!roadCount);
 /* Thousands of marks across clock wrap can never grow buffers or
  * lengthen the normal and low-FX drawing paths without bound. */
 roadNow=0xfffffff0u;U seed=rng,hash=savehash();Car test={.x=300,.y=300,.s=90,.cond=100};
 for(int i=0;i<10000;i++){
  roadFX_tick();roadFX_player_car(&test,298,300,.02f,95,0,BRAKE);
  assert(roadCount<=ROAD_MARK_COUNT);
 }
 assert(roadCount==ROAD_MARK_COUNT);roadFX_paint(285,282);
 fastGfx=1;rect(0,0,W,H,0xa1a2a3);roadFX_paint(285,282);assert(!has_ink(0x0b131b));fastGfx=0;
 assert(seed==rng&&hash==savehash());
 for(int i=0;i<ROAD_MARK_LIFE;i++)roadFX_tick();assert(!roadCount);
}
static void use_key_priority(void){
 reset_block();p.y=1002;cars[1]=(Car){1024,1024,0,0,1,1,100};
 tick(ENTER);assert(p.car<0&&near==1);
 tick(USE);assert(p.car==1&&p.heat==0);
 play(USE,90);assert(p.car==1&&!stoppingToExit);
 tick(ENTER);assert(p.car==1);
 tick(USE);assert(p.car<0);

 reset_block();p.y=1002;cars[1]=(Car){1024,1024,0,59.f,1,0,100};
 tick(USE);assert(p.car<0&&!cars[1].reported);
 tick(0);cars[1].s=0;tick(USE);
 assert(p.car==1&&cars[1].reported&&p.heat>=38.f);

 reset_block();p.x=loc[0].x;p.y=loc[0].y;
 cars[1]=(Car){p.x+23,p.y,0,0,1,1,100};
 tick(0);assert(near==1&&street_use_target(by,near).kind==STREET_USE_VENUE);
 tick(USE);assert(p.car<0&&scene==3&&interior_active_loc==0);

 reset_block();
 int b=0;while(b<nb&&!ordinaryDoorY[b])b++;assert(b<nb);
 p.x=ordinaryDoorX[b];p.y=ordinaryDoorY[b];
 cars[1]=(Car){p.x+23,p.y+3,0,0,1,1,100};
 tick(0);assert(near==1&&street_use_target(by,near).kind==STREET_USE_SHOP);
 tick(USE);assert(p.car<0&&scene==4&&walkin_active==b);

 reset_block();p.x=bankAtms[0].x;p.y=bankAtms[0].y+ATM_FRONT;
 cars[1]=(Car){p.x+23,p.y,0,0,1,1,100};
 tick(0);assert(near==1&&street_use_target(by,near).kind==STREET_USE_ATM);
 tick(USE);assert(p.car<0&&scene==1&&atmOpen==0);
 tick(ESC);assert(atmOpen<0);

 reset_block();p.x=1792;p.y=1270;
 beginJob(JOB_RECOVERY);assert(job.kind==JOB_RECOVERY&&jobRide==2);
 cars[1]=(Car){p.x-22,p.y,0,0,1,1,100};
 tick(0);assert(near==1&&job_street_prompt());
 tick(USE);assert(p.car<0&&job.step==1&&cars[jobRide].cond==65);

 puts("E boards, steals and exits; a held E does not bounce out. Enter ignores cars; jobs, doors and ATMs win overlaps.");
}
/* Vehicle staging must not make the user fight E's door priority. */
static void direct_staging(void){
 for(int action=0;action<3;action++){
  reset_block();
  int venue=action==0?0:action==1?6:3;
  p.x=loc[venue].x;p.y=loc[venue].y;
  int ride;
  if(action<2){
   beginJob(action==0?JOB_TAXI:JOB_SPRINT);ride=jobRide;assert(ride==nc-1);
  }else{
   sv.cash=200;active(3);doOpt(30);ride=0;assert(sv.owned&1);
  }
  assert(d2(cars[ride].x-p.x,cars[ride].y-p.y)>91.f*91.f);
  for(int step=0;step<125&&p.car<0;step++){
   StreetUse target=street_use_target(placeNear(),carNear());
   if(target.kind==STREET_USE_CAR&&target.id==ride){tick(USE);break;}
   float x=cars[ride].x-p.x,y=cars[ride].y-p.y;
   U keys=(absf(x)>1&&absf(x)>absf(y)*.36f)?(x>0?RIGHT:LEFT):0;
   if(absf(y)>1&&absf(y)>absf(x)*.36f)keys|=y>0?DOWN:UP;
   tick(keys);
  }
  assert(p.car==ride);
 }
 puts("Direct E boarding for staged taxi, sprint and garage vehicles.");
}
static void start_drive(int model,int condition){
 reset_block();p.x=512;p.y=1002;
 cars[1]=(Car){512,1024,0,0,model,2,condition};
 tick(USE);assert(p.car==1);
}
static void measure_foot_pace(void){
 for(int run=0;run<2;run++){
  U keys=run?BRAKE:0;
  reset_block();p.x=512;
  play(RIGHT|keys,60);assert(absf(p.x-512-(run?146.4f:98.4f))<.1f);

  reset_block();p.x=loc[12].x;p.y=loc[12].y;
  tick(USE);assert(scene==3&&loc[interior_active_loc].kind==13);
  float start=interior_y;play(UP|keys,10);
  assert(absf(start-interior_y-(run?28.4f:19.36f))<.01f);

  reset_block();int facade=0;while(facade<nb&&!ordinaryDoorY[facade])facade++;
  assert(facade<nb);p.x=ordinaryDoorX[facade];p.y=ordinaryDoorY[facade];
  tick(USE);assert(scene==4&&walkin_active==facade);
  start=wi_y;play(UP|keys,10);
  assert(absf(start-wi_y-(run?28.8f:20.f))<.01f);
 }

 reset_block();peds[0]=(Ped){.x=512,.y=585,.hp=1,.role=0};
 streetWalks[0]=(StreetWalk){.mx=512,.my=675};
 peds[DISPATCH_FIRST]=(Ped){.x=550,.y=550,.hp=1,.role=1};
 streetWalks[DISPATCH_FIRST]=(StreetWalk){.mx=550,.my=640};
 int guard=JOB_RIVAL_FIRST;
 peds[guard]=(Ped){.x=924,.y=1024,.hp=1,.role=2};
 play(0,60);
 assert(absf(peds[0].y-585-21.6f)<.1f);
 assert(absf(peds[DISPATCH_FIRST].y-550-29.6f)<.1f);
 assert(absf(peds[guard].x-924-56.f)<.1f);

 /* A turning car pinned an already retreating walker inside its safety
  * margin in the slower 36,000-tick unattended-street replay. */
 reset_block();cars[1]=(Car){275.6f,525.1f,-.39f,0.f,0,0,100};
 peds[0]=(Ped){.x=294,.y=528.51f,.hp=1,.role=0};
 streetWalks[0]=(StreetWalk){.mx=294,.my=550,.cross=2,.crossX=294,.crossY=550};
 assert(!street_foot_car_clear(peds[0].x,peds[0].y,-1,3.f));
 play(0,8);
 assert(peds[0].y>533.f&&street_foot_car_clear(peds[0].x,peds[0].y,-1,3.f));

 reset_block();beginJob(JOB_TAXI);
 assert(job.kind==JOB_TAXI&&jobRide>=0);
 int id=jobRide;cars[id].x=job.xs[0];cars[id].y=job.ys[0];
 p.x=cars[id].x;p.y=cars[id].y;p.car=id;
 for(int n=0;n<60&&taxi.phase==TAXI_WAIT;n++)tick(0);
 assert(taxi.phase==TAXI_BOARDING);
 float y=taxi.y;tick(0);assert(absf(taxi.y-y-1.24f)<.01f);
 play(0,45);assert(taxi.phase==TAXI_RIDING&&job.step==1&&job.time>0);
 puts("Exact 20% slower street/interior and human NPC movement, with keyboard doors and timed taxi boarding.");
}
static void speed_and_stops(void){
 /* Record the previous build's caps independent of the production tuning,
  * so the exact requested 10%-condition reference cannot silently drift. */
 static const float oldCaps[]={276.f,328.f,300.f,373.f,286.f};
 assert(LEN(oldCaps)==LEN(specs));
 float foot[2];
 for(int sprint=0;sprint<2;sprint++){
  reset_block();p.x=512;
  play(RIGHT|(sprint?BRAKE:0),180);foot[sprint]=p.x-512;
  assert(absf(foot[sprint]-3.f*(sprint?FOOT_SPRINT_SPEED:FOOT_WALK_SPEED))<1.f);
 }
 for(int model=0;model<LEN(specs);model++){
  start_drive(model,100);
  const float expected=oldCaps[model]*.55f;
  play(UP,180);float xs=p.x,fast=cars[1].s;
  assert(xs-512>foot[0]&&fast==expected);
  assert(absf(cameraLeadX)<=108.f&&cars[1].cond==100);
  play(USE,55);
  assert(p.car<0&&cars[1].x>xs&&cars[1].x-xs<fast*.27f&&cars[1].cond==100);
  printf("%s: %.0f units in 3 s from rest (walk %.0f, sprint %.0f); tops out at %.2f.\n",
         specs[model].name,xs-512,foot[0],foot[1],fast);

  start_drive(model,100);play(UP,110);fast=cars[1].s;
  play(DOWN,15);assert(cars[1].s>0&&cars[1].s<fast*.60f&&road_lamps(cars+1)==1);
  play(DOWN,86);assert(absf(cars[1].s+specs[model].reverse)<.1f);
  assert(road_lamps(cars+1)==2&&cars[1].cond==100);

  /* Wear still cuts the new cap proportionally; it did not just get
   * replaced by a flat cap applicable even to nearly wrecked cars. */
  start_drive(model,10);play(UP,120);
  assert(cars[1].s==expected*.55f&&cars[1].cond==10);
 }
 start_drive(3,100);cars[2]=(Car){1390,1024,1.56f,0,1,0,100};
 for(int f=0;f<360&&cars[1].cond==100;f++){tick(UP);assert(!street_overlap(cars+1,cars+2));}
 const float top=oldCaps[3]*.55f;
 assert(cars[1].cond==100-cl((int)(top*top/1450),1,37)&&cars[1].s<0&&cars[1].x<1390);
}
static void circuit_steer_to(float x,float y,int stop){
 int car=p.car,condition=cars[car].cond;
 for(int i=0;i<720;i++){
  Car*c=cars+p.car;
  float dx=x-c->x,dy=y-c->y,dist=sq(d2(dx,dy));
  if(dist<(stop?17.f:13.f)&&(!stop||absf(c->s)<3.5f))return;
  float angle=wrap(look(dx,dy)-c->a),speed=absf(c->s);U keys=0;
  if(angle>.018f)keys|=RIGHT;else if(angle<-.018f)keys|=LEFT;
  if(stop&&dist<speed*.16f+12)keys=BRAKE;
  else if(absf(angle)>.19f)keys|=BRAKE|UP;
  else if(dist>70||speed<47)keys|=UP;
  else keys|=BRAKE;
  tick(keys);
  assert(p.car==car&&cars[car].cond==condition);
 }
 fprintf(stderr,"Drive did not reach %.1f %.1f; ended %.1f %.1f speed %.1f\n",x,y,p.x,p.y,cars[car].s);
 assert(0);
}
static void drive_the_circuit(void){
 reset_block();p.x=loc[6].x-55;p.y=235;
 cars[1]=(Car){p.x,256,0,0,3,2,100};
 tick(USE);assert(p.car==1);beginJob(JOB_SPRINT);assert(job.kind==JOB_SPRINT);
 int goal=job.total,money=sv.cash;
 for(int step=0;step<goal;step++){
   int x=job.xs[step],y=job.ys[step];
   circuit_steer_to(x,y,step>0&&step<goal-1);
   if(step+1==goal)assert(job.kind<0);
   else assert(job.kind==JOB_SPRINT&&job.step==step+1);
 }
 assert(sv.legal==1&&sv.cash>money&&sprintBest>0&&sprintBest<350);
 puts("Full six-gate Glass Mile sprint completed with accelerator, brake, and steering inputs.");
}
static void drive_the_recovery(void){
 reset_block();p.x=1792;p.y=1270;
 beginJob(JOB_RECOVERY);assert(job.kind==JOB_RECOVERY&&jobRide>=0);
 int id=jobRide,money=sv.cash;
 tick(USE);assert(job.step==1&&cars[id].cond==65);
 tick(0);tick(USE);assert(p.car==id);
 circuit_steer_to(1792,1120,0);circuit_steer_to(1792,1023,1);
 circuit_steer_to(job.xs[1],job.ys[1],1);
 tick(USE);assert(job.kind<0&&p.car<0&&sv.legal==1&&sv.cash>money);
 puts("Stalled van restarted, driven to Dax's marked bay, stopped and handed over.");
}
int main(void){
 init_city();road_cover();brake_and_lights();use_key_priority();direct_staging();measure_foot_pace();speed_and_stops();drive_the_circuit();drive_the_recovery();
 puts("Road QA: obstruction-aware boarding and firefights, solid pursuers, fixed impact tracers, safe reload footing, input-driven braking/reverse lamps and 10,000 bounded tire marks passed.");
}
