/* Real fixed-step inputs. Stage only a legal career and its starting doorstep.
 * Define FILM_REPLAY switch by passing a path prefix to emit native tapes,
 * career fixture and milestone frame numbers. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
static U tape[40000];static int nt;static char prefix[400];static int noisy,onlyFirst;
static void k(U key){assert(nt<LEN(tape));tape[nt++]=key;tick(key);}
static void hold(U key,int frames){for(int i=0;i<frames;i++)k(key);}
static void key(U key){k(0);k(key);k(0);}
static void trace(const char*s){fprintf(stderr,"%s frame %d @ %.1f,%.1f a %.2f v %.1f car %d listing %d [%d,%d] job %d/%d: %s\n",s,nt,p.x,p.y,p.a,p.car<0?-1:cars[p.car].s,p.car,vdActive,dvPhase,dvClient,job.kind,job.step,notice);}
static void module(int id){key(ACTIVITIES);assert(vdPanel==1);while(vdSelected!=id)key(DOWN);key(ENTER);assert(vdActive==id);}
static void shot(const char*label){
 if(!prefix[0])return;char f[500];snprintf(f,sizeof f,"%s.frames",prefix);FILE*out=fopen(f,"a");assert(out);fprintf(out,"%d %s\n",nt,label);fclose(out);
}
static void init(int x,int y){
 init_city();sv.x=x;sv.y=y;sv.intro=4;sv.owned=1;sv.selection=0;sv.carCond[0]=100;sv.legal=2;sv.cash=318;sv.hash=savehash();
 if(prefix[0]){char f[500];snprintf(f,sizeof f,"%s.dat",prefix);FILE*out=fopen(f,"wb");assert(out);fwrite(&sv,sizeof sv,1,out);fclose(out);
 snprintf(f,sizeof f,"%s.frames",prefix);fclose(fopen(f,"w"));}
 wason=1;profiles_init();nt=0;key(ENTER);assert(scene==1&&p.car<0);key(GFX);
}
static void done(void){
 key(SAVE);if(!prefix[0])return;
 char f[500];snprintf(f,sizeof f,"%s.plan",prefix);FILE*out=fopen(f,"w");assert(out);
 fprintf(out,"%d %x\n",nt,sv.hash);for(int t=0;t<nt;){int s=t+1;while(s<nt&&tape[t]==tape[s])s++;fprintf(out,"%d %u\n",s-t,tape[t]);t=s;}fclose(out);
}
static void walkXY(float x,float y){
 assert(p.car<0);
 for(int i=0;i<400;i++){
  float dx=x-p.x,dy=y-p.y;int kx=absf(dx)>=1.6f?(dx>0?RIGHT:LEFT):0,ky=absf(dy)>=1.6f?(dy>0?DOWN:UP):0;
  if(!kx&&!ky){k(0);return;}k(kx|ky|BRAKE);
 }
 trace("walk stalled");assert(0);
}
static void board(int i){
 assert(p.car<0);int n=0;
 while(street_near_car()!=i&&n++<300){
  float dx=cars[i].x-p.x,dy=cars[i].y-p.y;
  k((absf(dx)>1.8f?(dx>0?RIGHT:LEFT):0)|(absf(dy)>1.8f?(dy>0?DOWN:UP):0)|BRAKE);
 }
 key(USE);if(p.car!=i)trace("board failed");assert(p.car==i);
}
static void moveX(int target){
 assert(p.car>=0);for(int i=0;i<700;i++){
  Car*c=cars+p.car;float delta=target-c->x,dir=cs(c->a)>0?1:-1,s=c->s*dir;
  if(absf(delta)<5&&absf(s)<4){k(0);return;}
  if(absf(delta)<absf(s)*.135f+3&&s*delta>0)k(BRAKE);
  else if(vdActive==VA_B2A&&absf(s)>44&&s*delta>0)k(0);
  else if(delta*dir>0)k(UP);else k(DOWN);
 }
 trace("straight move stalled");assert(0);
}
static void drive(float x,float y,int halt){
 for(int i=0;i<650;i++){
  if(p.car<0||p.dead){trace("lost car");fprintf(stderr,"target %.0f %.0f heat %.2f hp %d\n",x,y,p.heat,p.hp);}assert(p.car>=0&&!p.dead);Car*c=cars+p.car;float dx=x-c->x,dy=y-c->y,dist=sq(d2(dx,dy));
  if(dist<(halt?10.f:10.f)&&(!halt||absf(c->s)<4.5f)){k(0);return;}
  float angle=wrap(look(dx,dy)-c->a),speed=absf(c->s);U keys=0;
  if(angle>.017f)keys|=RIGHT;else if(angle<-.017f)keys|=LEFT;
  if(halt&&dist<speed*.16f+5)keys=BRAKE;
  else if(absf(angle)>.165f)keys|=BRAKE|UP;
  else if(dist>50||speed<40)keys|=UP;
  else keys|=BRAKE;
  if(speed>=(vdActive==VA_B2A?44.f:76.f)&&!(keys&BRAKE))keys&=~(U)UP;
  k(keys);
  if(noisy&&i%160==159){trace("steer");for(int n=0;n<nc;n++)if(n!=p.car&&d2(cars[n].x-p.x,cars[n].y-p.y)<65*65)fprintf(stderr,"  object %d %.1f %.1f a %.2f s %.1f\n",n,cars[n].x,cars[n].y,cars[n].a,cars[n].s);}
 }trace("drive stalled");fprintf(stderr,"target %.1f %.1f\n",x,y);assert(0);
}
static void film(void){
 init(VD_FILM_X,VD_FILM_Y+23);Car own=cars[0];int n=nc;
 module(VA_A2);key(USE);assert(p.car==dvBase&&p.car>=0&&nc==n+1);
 hold(UP,33);hold(UP|RIGHT,32);hold(BRAKE,45);trace("rolling line");
 assert(dvShot==2&&dvPhase==2);shot("wide");paint();
 key(ENTER);assert(dvPhase==1&&p.car==dvBase);hold(UP,33);hold(UP|RIGHT|BRAKE,26);hold(UP,24);hold(BRAKE,45);
 trace("inside line");assert(dvShot==1&&dvPhase==2);shot("inside");paint();key(USE);
 assert(vdActive<0&&p.car<0&&nc==n&&!memcmp(&own,cars,sizeof own)&&sv.cash==318);
 done();puts("Film: Low FX, rolling and handbrake sequences, two distinct automatic title cards, instant retake / wrap, owned ride unchanged.");
}
static void valet(int farFirst){
 init(VD_VALET_X,VD_VALET_Y+22);Car own=cars[0];int n=nc;module(VA_B2A);key(USE);
 assert(dvMine(p.car));moveX(dvValetX[farFirst]);key(USE);assert(dvClient==1&&dvParked==farFirst&&p.car<0);
 /* Clear front curb, walk back to desk, take the next keys. For the first
  * bay's parked car to stay solid, pass on the south sidewalk/curb. */
 walkXY(VD_VALET_X,VD_VALET_Y+22);key(USE);assert(dvMine(p.car)&&cars[p.car].type==2);
 if(!farFirst){
  /* The first client is in the nearer bay. Swing around it instead of
   * ever driving through the signed-in car. */
  drive(1078,750,1);drive(dvValetX[1]-25,755,1);drive(dvValetX[1],dvValetY[1],1);
 }else moveX(dvValetX[0]);
 key(USE);trace("valet complete");assert(nc==n&&p.car<0&&vdActive<0&&sv.cash==358&&!memcmp(&own,cars,sizeof own));
 hold(0,15);assert(sv.cash==358);shot("valet-paid");done();puts("Valet: chose a bay, returned first keys on foot, received second car, used other bay, one payout.");
}
static void puzzle(void){
 init(VD_APRON_X,VD_APRON_Y+17);Car own=cars[0];int n=nc;module(VA_B2B);key(USE);assert(dvPhase&&p.car<0&&nc==n+2);
 Car outside=dvCar(176,1243,0,0,1);assert(vd_chess_rail_depth(&outside)>0);outside.reported=0;assert(vd_chess_rail_depth(&outside)==0);
 board(dvBase);moveX(79);key(ENTER);assert(cars[dvBase].x==157&&cars[dvBase+1].x==80&&p.car<0);
 board(dvBase);moveX(70);key(USE);assert(p.car<0);
 /* Move the compact into the west pocket; bring the van through the open
  * throat from the south apron and straighten up before entering the rails. */
 board(dvBase+1);drive(114,1300,1);drive(108,1261,1);drive(138,1260,1);drive(205,1260,1);key(USE);trace("van deep");assert(dvChessPark(1)&&p.car<0);
 walkXY(111,1284);board(dvBase);moveX(157);key(USE);k(0);trace("puzzle complete");
 assert(nc==n&&p.car<0&&vdActive<0&&sv.cash==318&&!memcmp(&own,cars,sizeof own));shot("puzzle-done");done();
 puts("Curb Chess: collision rails scoped to puzzle vehicles, blocked mouth, move compact, park van through real entry, compact back, reset and exit.");
}
static void raceInput(int pause){
 init(1109,230);module(VA_B9);trace("near clock");key(USE);trace("started?");assert(job.kind==JOB_SPRINT);
 int i=jobRide;trace("loaner");board(i);trace("boarded");
 drive(1148,258,1);assert(job.step==1);
 if(pause){int old=dvRaw.n;float t=job.time;U city=sv.playedTicks;key(MAP);old=dvRaw.n;t=job.time;hold(0,73);assert(mapOn&&sv.playedTicks>city&&dvRaw.n==old&&job.time==t);key(MAP);}
 drive(1277,269,1);assert(job.step==2);drive(1264,510,1);assert(job.step==3);
 drive(1030,513,1);assert(job.step==4);{int lane=1038;drive(lane,473,1);drive(lane,300,1);drive(1027,270,1);}assert(job.step==5);
 for(int j=0;j<400&&job.kind==JOB_SPRINT;j++){Car*c=cars+p.car;float a=wrap(look(1110-c->x,265-c->y)-c->a);k(UP|(a>.1?RIGHT:a<-.1?LEFT:0)|(absf(a)>.2f?BRAKE:0));}
 trace("finish");assert(vdPanel==2&&dvNewLap);shot("keep");key(ENTER);assert(dvRival.n&&vdActive<0&&sv.cash==523);
 if(onlyFirst)return; int kept=dvRival.n;
 hold(BRAKE,50);key(USE);assert(p.car<0);trace("parked for rematch");
 assert(dvAt(loc[6].x,loc[6].y,44));module(VA_B9);key(USE);assert(job.kind==JOB_SPRINT);board(jobRide);
 drive(1148,258,1);assert(job.step==1);assert(dvVisible&&dvRacingGhost()==&dvRival);paint();shot("rival");
 key(ENTER);assert(!dvVisible);key(ENTER);assert(dvVisible);
 drive(1277,269,1);assert(job.step==2);shot("split");
 if(!getenv("SKIP_SECOND")){drive(1264,502,1);assert(job.step==3);int south=495;drive(1236,south,1);drive(1030,south,1);assert(job.step==4);drive(1038,473,1);drive(1038,300,1);drive(1027,270,1);
 for(int j=0;j<400&&job.kind==JOB_SPRINT;j++){Car*c=cars+p.car;float a=wrap(look(1110-c->x,265-c->y)-c->a);k(UP|(a>.1?RIGHT:a<-.1?LEFT:0)|(absf(a)>.2f?BRAKE:0));}
 assert(vdPanel==2&&dvNewLap);key(DOWN);key(ENTER);assert(!dvNewLap&&dvRival.n==kept&&sv.cash==728);
 }else{key(ACTIVITIES);key(DOWN);key(ENTER);assert(dvRival.n==kept&&vdActive<0&&job.kind<0);}
 done();puts("Shadow: all stock gates, keep $205 lap, rematch with on/off wireframe rival, atlas freezes race samples, first split.");
}

static void parity(void){
 onlyFirst=1;raceInput(1);static U plan[4000];int end=nt;
 assert(end<LEN(plan));cp(plan,tape,end*sizeof(U));
 static DvFilm ref;dvCopyFilm(&ref,&dvRaw);Car original=cars[p.car];int brake=0;
 for(int i=0;i<ref.n;i++)brake+=ref.at[i].brake;assert(brake>8);
 for(int show=0;show<2;show++){
  init(1109,230);int start=nt;int injected=0;dvCopyFilm(&dvRival,&ref);
  for(int i=start;i<end;i++){
   if(vdActive==VA_B9)dvVisible=show;
   if(!injected&&job.kind==JOB_SPRINT&&job.step==2&&!mapOn&&plan[i]==UP){
    int n=dvRaw.n;float t=job.time;U city=sv.playedTicks;
    k(MAP);hold(0,139);k(MAP);assert(!mapOn&&n==dvRaw.n&&t==job.time&&sv.playedTicks>city);injected=1;
   }
   if(show&&job.kind==JOB_SPRINT&&dvRecording&&i%47==0)paint();k(plan[i]);
  }
  assert(injected&&ref.n==dvRaw.n&&memcmp(ref.splits,dvRaw.splits,sizeof ref.splits)==0);
  assert(memcmp(ref.at,dvRaw.at,sizeof(DvPose)*ref.n)==0&&!memcmp(&original,cars+p.car,sizeof original));
  assert(!dvNewLap&&vdActive<0&&sv.cash==523);
 }
 printf("PASS: %d identical poses / six splits, %d braking samples; ghost hidden vs drawn, inserted atlas dwell, identical final car and payout.\n",ref.n,brake);
}

int main(int argc,char**argv){assert(argc>=2);if(argc>2)snprintf(prefix,sizeof prefix,"%s",argv[2]);set_session_seed(17);
 noisy=!!getenv("VERBOSE");if(!strcmp(argv[1],"film"))film();else if(!strcmp(argv[1],"valet"))valet(1);else if(!strcmp(argv[1],"valet-near"))valet(0);
 else if(!strcmp(argv[1],"puzzle"))puzzle();else if(!strcmp(argv[1],"race"))raceInput(1);else if(!strcmp(argv[1],"parity"))parity();else assert(0);
}
