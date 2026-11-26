/* A2 / B2a / B2b / B9: disposable set cars + session-only Glass Mile film.
 * Real vehicle handling, pedestrian/vehicle clearance and street interactions
 * are shared with regular play. No character save or ambient slot is borrowed. */
#define VD_HAS_A2 1
#define VD_HAS_B2A 1
#define VD_HAS_B2B 1
#define VD_HAS_B9 1
enum {VD_FILM_X=86,VD_FILM_Y=204, VD_VALET_X=1046,VD_VALET_Y=730,
      VD_APRON_X=84,VD_APRON_Y=1304, VD_RAIL_MARK=2209, VD_LOAN_MARK=2210};
static int dvBase=-1,dvN,dvPhase,dvShot,dvPass,dvClient,dvBay[2],dvParked=-1;
static int dvValetX[2],dvValetY[2];
static int dvMine(int i){
 return dvBase>=0&&i>=dvBase&&i<dvBase+dvN&&i<nc&&cars[i].owner==2&&
   (cars[i].reported==VD_RAIL_MARK||cars[i].reported==VD_LOAN_MARK);
}
static Car dvCar(float x,float y,float a,int type,int rail){
 return (Car){.x=x,.y=y,.a=a,.s=0,.type=type,.owner=2,.cond=100,.reported=rail?VD_RAIL_MARK:VD_LOAN_MARK};
}
static int dvAt(int x,int y,int r){return scene==1&&d2(p.x-x,p.y-y)<r*r&&street_line_open(p.x,p.y,x,y,1);}
static void dvDisembark(void){
 if(!dvMine(p.car))return;
 int i=p.car;cars[i].s=0;
 if(street_exit_car())return;
 /* If the set is struck in an alley, remove its cars first; the chassis
  * center itself is generally a legal foot point. Otherwise search nearby. */
 float x=cars[i].x,y=cars[i].y;p.car=-1;
 for(int r=0;r<260;r+=5)for(int k=0;k<16;k++){
  float a=k*.3926991f,tx=x+r*cs(a),ty=y+r*sn(a);
  if(street_foot_clear(tx,ty,i)){p.x=tx;p.y=ty;return;}
 }
 p.x=x;p.y=y;
}
static void dvStrike(void){
 if(dvBase>=0&&nc==dvBase+dvN){
  dvDisembark();
  if(dvMine(lastCar))lastCar=-1;
  for(int i=dvBase;i<nc;i++){streetDrivers[i]=(StreetDriver){0};cars[i]=(Car){.x=-100,.y=-100};}
  nc=dvBase;
 }
 dvBase=-1;dvN=dvPhase=dvPass=dvShot=dvClient=0;dvParked=-1;
}
static int dvOpenSlots(int n){
 if(dvBase>=0)return dvN==n;
 if(nc<1||nc+n>LEN(cars))return 0;
 dvBase=nc;dvN=n;for(int i=0;i<n;i++)street_stage_car(nc++,(Car){.x=-100,.y=-100});
 return 1;
}
static int dvClearSet(const Car*c){
 if(street_hits_wall(c))return 0;
 for(int i=0;i<nc;i++)if((i||sv.owned)&&!dvMine(i)&&street_overlap(c,cars+i))return 0;
 for(int i=0;i<LEN(peds);i++)if(peds[i].hp>0&&street_foot_car_depth(c,peds[i].x,peds[i].y,3)>0)return 0;
 return 1;
}
static void dvSeat(int id){
 Car*c=cars+id;p.car=lastCar=id;p.x=c->x;p.y=c->y;p.a=c->a;
 stoppingToExit=0;cameraLeadX=cameraLeadY=0;
}
static void dvFinish(void){dvStrike();vd_end();}
static int dvCanRestart(void){
 if(p.car>=0&&!dvMine(p.car)){tell("Park your own ride and step out before resetting the set.");return 0;}
 return 1;
}
static void dvBoard(int id){if(street_near_car()==id)vehicle_use(id);}
static void dvBadge(float x,float y,const char*t,U c){
 int xx=sx(x),yy=sy(y),w=textw(t,0)+10;if(xx<-w||xx>W+w||yy<-21||yy>=H)return;
 box(xx-w/2,yy,w,20,0x13222b,c);center(xx,yy-1,t,c,0);
}
static void dvBayMark(int x,int y,int w,int h,U c){
 int xx=sx(x),yy=sy(y);
 for(int i=-1;i<=1;i+=2)for(int j=-1;j<=1;j+=2){
  int a=xx+i*w,b=yy+j*h;line(a,b,a-i*9,b,c);line(a,b,a,b-j*7,c);
 }
}
static void dvArrow(int x,int y,float a,U col){
 float co=cs(a),si=sn(a);int xx=sx(x),yy=sy(y),ex=xx+(int)(19*co),ey=yy+(int)(19*si);
 for(int k=-1;k<=1;k++){int u=(int)(k*si),v=(int)(k*co);line(xx+u,yy+v,ex+u,ey+v,col);}
 tri(ex,ey,ex-(int)(9*co-6*si),ey-(int)(9*si+6*co),ex-(int)(9*co+6*si),ey-(int)(9*si-6*co),col);
}
/* A2: enter from the painted start and catch either geographically distinct
 * finish. No time grade, submission key or handbrake-flag shortcut. */
static int dvTake(void){
 Car c=dvCar(VD_FILM_X,VD_FILM_Y,0,0,0);
 if(!dvCanRestart())return 0;
 if(!dvClearSet(&c)||!dvOpenSlots(1)){tell("The chalk start is occupied. Clear the mark and try again.");return 0;}
 street_stage_car(dvBase,c);dvSeat(dvBase);dvPass=dvShot=0;dvPhase=1;
 vd_goal(VD_FILM_X+75,VD_FILM_Y+69,"Film set / camera marks");
 vd_status("Drive east past ACTION. Turn right. Stop on I (inside) or II (wide), nose south toward camera. Enter: retake.");
 tell("Cameras rolling. Inside and wide are equal takes.");return 1;
}
static void dvFilmPaint(void){
 int x=VD_FILM_X,y=VD_FILM_Y;
 if(!dvPhase){dvBadge(x,y+10,"FILM CALL  /  E",GOLD);return;}
 dvBadge(x+23,y-39,"ACTION",GOLD);dvArrow(x+19,y,0,GOLD);
 for(int i=0;i<2;i++){
  int xx=x+(i?91:58);U c=i?0xf2c77a:0x84dcd7;
  dvBayMark(xx,y+69,15,16,c);
  if(i){line(sx(xx-3),sy(y+65),sx(xx-3),sy(y+74),c);line(sx(xx+3),sy(y+65),sx(xx+3),sy(y+74),c);}
  else line(sx(xx),sy(y+65),sx(xx),sy(y+74),c);
 }
 int xx=sx(x+74),yy=sy(y+111);
 box(xx-9,yy-6,18,11,0x243842,PAPER);rect(xx+9,yy-4,5,7,GOLD);
 line(xx,yy+5,xx-6,yy+14,PAPER);line(xx,yy+5,xx+6,yy+14,PAPER);
 dvBadge(x+74,y+129,"CAM 01  /  FACE SOUTH",GOLD);
}
static int vd_a2(int ev,U keys,U tap){
 (void)keys;
 if(ev==VE_CANCEL||ev==VE_RESET){dvStrike();return 0;}
 if(ev==VE_START){
  dvPhase=0;vd_goal(VD_FILM_X,VD_FILM_Y,"West Quay / film call");
  vd_status("West Quay film set. Come on foot to the chalk start; E takes the free Mote. Two marks, no timer.");return 0;
 }
 if(ev==VE_WORLD){dvFilmPaint();return 0;}
 if(ev==VE_DRAW&&scene==1&&dvPhase==2){
  box(294,183,372,108,0x12242c,GOLD);rect(305,194,350,2,0xb18756);
  center(480,202,"QUAYSIDE PICTURES   /   TAKE PRINTED",GOLD,0);
  center(480,228,dvShot==1?"Inside Job":"Wide Release",PAPER,1);return 0;
 }
 if(ev!=VE_TICK||scene!=1)return 0;
 if(!dvPhase){
  if((tap&USE)&&p.car<0&&dvAt(VD_FILM_X,VD_FILM_Y,41)){dvTake();return 1;}return 0;
 }
 if((tap&ENTER)&&d2(p.x-VD_FILM_X,p.y-VD_FILM_Y)<340*340){dvTake();return 1;}
 if(dvPhase==2){
  if(tap&USE){dvFinish();tell("That's a wrap. West Quay is open again.");return 1;}return 0;
 }
 if(p.car!=dvBase){vd_status("Return to the film Mote. Enter near the set resets the take, including a wrecked loaner.");return 0;}
 Car*c=cars+dvBase;
 if(c->x>VD_FILM_X+15&&c->x<VD_FILM_X+80&&absf(c->y-VD_FILM_Y)<24&&absf(wrap(c->a))<.93f)dvPass=1;
 int idx=absf(c->x-(VD_FILM_X+58))<=15?1:absf(c->x-(VD_FILM_X+91))<=15?2:0;
 int on=idx&&absf(c->y-(VD_FILM_Y+69))<=17,face=absf(wrap(c->a-1.570796f))<=.66f;
 if(dvPass&&on&&face&&absf(c->s)<=6){
  dvShot=idx;dvPhase=2;vd_status("Print complete. Enter: instant retake. E: wrap and return the loaner.");
  tell(idx==1?"PRINT I / Inside Job. The near mark held the frame.":"PRINT II / Wide Release. The long sweep filled the frame.");return 0;
 }
 vd_status(!dvPass?"Roll east through ACTION, then turn right toward the two marks. Shift tightens the turn. Enter: retake.":
   on&&!face?"Position: ON MARK. Facing: turn the nose SOUTH toward CAM 01. Enter: retake.":
   on?"Position: ON MARK. Facing: GOOD. Ease off or hold Shift to stop.":
   "Position: find I (inside) or II (wide). Face SOUTH, stop. Shift: brake. Enter: retake.");
 return 0;
}
/* B2a: two independent customers remain present until both keys are returned.
 * Each bay is a real clean curb beside Nightglass, never a player car. */
static int dvBayFor(int id,int n){
 if(!dvMine(id))return -1;
 Car*c=cars+id;
 if(absf(c->s)>7)return -1;
 for(int i=0;i<n;i++)if(absf(c->x-dvValetX[i])<=17&&absf(c->y-dvValetY[i])<=13)return i;
 return -1;
}
static int dvValetKeys(void){
 int x=VD_VALET_X,y=VD_VALET_Y;
 Car c=dvCar(x,y,0,dvClient?2:1,0);
 if(!dvClearSet(&c)||(dvClient&&street_overlap(&c,cars+dvBase))){tell("Customer pull-in is occupied. Clear the key stand and retry.");return 0;}
 if(!dvClient){
  int n=0;static const int xs[]={1100,1194,1152,1240};
  for(int k=0;k<LEN(xs)&&n<2;k++){
   Car b=dvCar(xs[k],y,0,1,0);
   if(!dvClearSet(&b)||(n&&absf(xs[k]-dvValetX[0])<46))continue;
   dvValetX[n]=xs[k];dvValetY[n++]=y;
  }
  if(n<2||!dvOpenSlots(2)){tell("Too few empty curb bays. Wait for the curb to clear.");return 0;}
 }
 int i=dvBase+dvClient;street_stage_car(i,c);dvSeat(i);dvPhase=1;
 vd_goal(dvValetX[dvClient&&dvParked==0?1:0],y,"Nightglass / valet bays");
 vd_status(dvClient?"Client 2 / Pallas. Park in the OTHER painted bay. Stop, then E to step out.":
       "Client 1 / Vectra. Choose either painted bay A or B. Stop, then E to step out.");
 return 1;
}
static void dvValetPaint(void){
 dvBadge(VD_VALET_X,VD_VALET_Y-28,"VALET KEYS  /  E",GOLD);
 if(dvBase<0)return;
 for(int i=0;i<2;i++){
  U col=i==dvParked?GOLD:TEAL;
  dvBayMark(dvValetX[i],dvValetY[i],23,14,col);
  dvBadge(dvValetX[i],dvValetY[i]-42,i?"B":"A",col);
 }
}
static int vd_b2a(int ev,U keys,U tap){
 (void)keys;
 if(ev==VE_START){dvPhase=dvClient=0;dvParked=-1;vd_goal(VD_VALET_X,VD_VALET_Y,"Nightglass / valet keys");
  vd_status("Nightglass Motors: two customers, $40 clean for the shift. Walk to the key stand and press E.");return 0;}
 if(ev==VE_CANCEL||ev==VE_RESET){dvStrike();return 0;}
 if(ev==VE_WORLD){dvValetPaint();return 0;}
 if(ev!=VE_TICK||scene!=1)return 0;
 if(!dvPhase){
  if((tap&USE)&&p.car<0&&dvAt(VD_VALET_X,VD_VALET_Y,34)){dvValetKeys();return 1;}
  if(dvClient&&(tap&USE)&&p.car<0&&street_near_car()==dvBase){tell("First keys signed in. Second keys are at the stand.");return 1;}
  return 0;
 }
 if(cars[dvBase+dvClient].cond<=0){dvFinish();tell("Valet shift closed: client car wrecked. Reopen the listing to retry.");return 0;}
 /* A returned set of keys remains with the attendant, release these keys
  * instead of letting a later E move the first customer's parked car. */
 if(dvClient&&p.car<0&&(tap&USE)&&street_near_car()==dvBase){tell("First customer signed in. Take the second car.");return 1;}
 if(p.car<0&&(tap&USE)&&street_near_car()==dvBase+dvClient){dvBoard(dvBase+dvClient);return 1;}
 if(p.car!=dvBase+dvClient||!(tap&USE))return 0;
 int b=dvBayFor(p.car,2);
 if(b<0||b==dvParked)return 0; /* normal E still allows safe ordinary exits */
 if(!street_exit_car()){tell("No safe door here. Reposition in the painted bay and try E again.");return 1;}
 dvBay[dvClient]=b;
 if(!dvClient){
  dvParked=b;dvClient=1;dvPhase=0;
  vd_goal(VD_VALET_X,VD_VALET_Y,"Nightglass / second keys");
  vd_status("First customer parked. Walk to the key stand; E takes the second customer's keys.");
  tell("First set returned. Next customer: amber Pallas.");return 1;
 }
 dvFinish();gain(1,40,"Two customers parked.");request_save();return 1;
}
/* B2b: three temporary loading rails form a genuine single-file blind berth.
 * Their collision is local to the two puzzle cars. Normal traffic, walkers,
 * another vehicle, city routing and dispatch keep the exact stock geometry. */
static const StreetBounds dvRails[]={{141,1240,92,4},{141,1276,92,4},{229,1240,4,40}};
static float vd_chess_rail_depth(const Car*c){
 if(c->owner!=2||c->reported!=VD_RAIL_MARK||vdActive!=VA_B2B)return 0.f;
 float best=0,co=cs(c->a),si=sn(c->a),ac=absf(co),as=absf(si);
 for(int i=0;i<LEN(dvRails);i++){
  const StreetBounds*b=dvRails+i;float w=b->w*.5f,h=b->h*.5f,dx=b->x+w-c->x,dy=b->y+h-c->y;
  float a=w+STREET_HALF_L*ac+STREET_HALF_W*as-absf(dx),b0=h+STREET_HALF_L*as+STREET_HALF_W*ac-absf(dy);
  float u=STREET_HALF_L+w*ac+h*as-absf(dx*co+dy*si),v=STREET_HALF_W+w*as+h*ac-absf(dy*co-dx*si);
  if(a>0&&b0>0&&u>0&&v>0){if(b0<a)a=b0;if(u<a)a=u;if(v<a)a=v;if(a>best)best=a;}
 }
 return best;
}
static int dvChessPark(int idx){
 if(!dvMine(dvBase+idx))return 0;
 Car*c=cars+dvBase+idx;
 return c->cond>0&&absf(c->s)<=7&&absf(c->x-(idx?205:157))<=10&&absf(c->y-1260)<=9&&absf(cs(c->a))>.84f;
}
static int dvChessSet(void){
 Car a=dvCar(157,1260,0,0,1),b=dvCar(80,1302,0,4,1);
 if(!dvCanRestart())return 0;
 if(!dvClearSet(&a)||!dvClearSet(&b)||!dvOpenSlots(2)){tell("Apron occupied. Clear the loaner spaces, then reset.");return 0;}
 dvDisembark();street_stage_car(dvBase,a);street_stage_car(dvBase+1,b);
 /* Give a reset a stable legal foot point, clear of the newly staged cars. */
 int found=0;for(int r=0;r<=96&&!found;r+=4)for(int k=0;k<12&&!found;k++){
  float x=103+r*cs(k*.523599f),y=1283+r*sn(k*.523599f);
  if(street_foot_clear(x,y,-1)){p.x=x;p.y=y;found=1;}
 }
 dvPhase=1;vd_goal(179,1260,"Quay apron / tandem berth");
 tell("One lane, no side entrance. The van needs the deep berth. Enter resets the apron.");return 1;
}
static void dvChessPaint(void){
 if(!dvPhase){dvBadge(VD_APRON_X,VD_APRON_Y-15,"APRON KEYS  /  E",GOLD);return;}
 for(int i=0;i<LEN(dvRails);i++){
  StreetBounds r=dvRails[i];int x=sx(r.x),y=sy(r.y);
  rect(x,y,r.w,r.h,0x737674);
  for(int j=0;j<(r.w>r.h?r.w:r.h)-3;j+=10)
   rect(x+(r.w>r.h?j:0),y+(r.h>r.w?j:0),r.w>r.h?4:r.w,r.h>r.w?4:r.h,GOLD);
 }
 dvBayMark(157,1260,17,10,TEAL);dvBayMark(205,1260,17,10,GOLD);
 dvBadge(157,1214,"MOTE",TEAL);dvBadge(208,1230,"VAN",GOLD);
 dvBadge(90,1329,"SWING SPACE",PAPER);
}
static int vd_b2b(int ev,U keys,U tap){
 (void)keys;
 if(ev==VE_START){dvPhase=0;vd_goal(VD_APRON_X,VD_APRON_Y,"West Quay / pallet apron");
  vd_status("West Quay pallet apron. Come on foot; E opens the tandem parking puzzle. No timer.");return 0;}
 if(ev==VE_CANCEL||ev==VE_RESET){dvStrike();return 0;}
 if(ev==VE_WORLD){dvChessPaint();return 0;}
 if(ev!=VE_TICK||scene!=1)return 0;
 if(!dvPhase){
  if((tap&USE)&&p.car<0&&dvAt(VD_APRON_X,VD_APRON_Y,40)){dvChessSet();return 1;}return 0;
 }
 if((tap&ENTER)&&d2(p.x-160,p.y-1260)<360*360){dvChessSet();return 1;}
 if(dvChessPark(0)&&dvChessPark(1)&&p.car<0){
  dvFinish();tell("Curb Chess / CHECKMATE. The van is deep and the Mote guards the mouth.");return 1;
 }
 vd_status(dvChessPark(1)?"Van: DEEP. Put Mote on the mouth mark and step out. E: switch cars. Enter: reset.":
    "Back the Mote out of the mouth. Park van in DEEP berth, then return Mote to mouth. E: switch cars. Enter: reset.");
 if((tap&USE)&&p.car<0&&dvMine(street_near_car())){dvBoard(street_near_car());return 1;}
 return 0;
}
/* B9: poses are stored at exactly one sample per live race-clock tick.
 * Nobody inserts ghosts into cars[] or streetDrivers[]. Visibility doesn't
 * enter simulation or consume RNG. 95s limit bounds memory to ~140 KB.
 * A fresh career never inherits another character's best or footage. */
enum {DV_SHADOW_MAX=95*60+4};
typedef struct {short x,y,a;unsigned char brake,pad;} DvPose;
typedef struct {int n,splits[6];DvPose at[DV_SHADOW_MAX];} DvFilm;
static DvFilm dvRaw,dvLast,dvRival;
static int dvRecording,dvNewLap,dvShadowPhase,dvVisible,dvCurrentSplit;
static const DvFilm *dvRacingGhost(void){return dvRival.n?&dvRival:dvLast.n?&dvLast:0;}
static void dvCopyFilm(DvFilm*to,const DvFilm*from){
 to->n=from->n;cp(to->splits,from->splits,sizeof to->splits);cp(to->at,from->at,(unsigned)from->n*sizeof(DvPose));
}
static DvPose dvPose(U keys){
 const Car*c=cars+p.car;int braking=!!(keys&BRAKE)||((keys&DOWN)&&c->s>2)||((keys&UP)&&c->s<-2);
 return (DvPose){(short)(p.x*8+.5f),(short)(p.y*8+.5f),(short)(wrap(p.a)*4000),braking,0};
}
static void vd_shadow_sample(U keys){
 if(job.kind!=JOB_SPRINT||job.step==0){dvRecording=0;return;}
 if(!dvRecording||dvRaw.n>=DV_SHADOW_MAX)return;
 /* On-foot time counts against the sprint too. The shadow rests at the car
  * rather than depicting a floating chassis chasing its driver. */
 DvPose pose=(p.car==jobRide&&p.car>=0)?dvPose(keys):dvRaw.at[dvRaw.n-1];dvRaw.at[dvRaw.n++]=pose;
}
static void vd_shadow_gate(int step){
 if(step==0){dvRaw.n=1;z(dvRaw.splits,sizeof dvRaw.splits);dvRaw.at[0]=dvPose(prev);dvRecording=1;dvCurrentSplit=0;return;}
 if(!dvRecording||step<0||step>=LEN(dvRaw.splits))return;
 dvCurrentSplit=step;dvRaw.splits[step]=dvRaw.n-1;
 if(step==job.total-1){
  dvRecording=0;
  if(vdActive==VA_B9)dvNewLap=1;
  else dvCopyFilm(&dvLast,&dvRaw); /* last ordinary run, not an override of an explicitly kept rival */
 }
}
/* True ground paint points away from the start toward gate 1, irrespective
 * of the driver's approach. This also applies to the unmodified stock sprint. */
static void dvStartArrow(void){
 if(scene!=1||mapOn||job.kind!=JOB_SPRINT||job.step>1)return;
 int x=job.xs[0],y=job.ys[0],xx=sx(x),yy=sy(y);
 if(xx<30||xx>W-62||yy<100||yy>H-72)return;
 float a=look(job.xs[1]-x,job.ys[1]-y);dvArrow(x+8,y+9,a,GOLD);dvArrow(x+30,y+9,a,GOLD);
}
static void dvShadowPaint(void){
 const DvFilm*g=dvRacingGhost();
 if(!dvVisible||!g||!dvRecording||job.kind!=JOB_SPRINT||job.step==0||dvRaw.n<1)return;
 int frame=dvRaw.n-1;if(frame>=g->n)return;
 DvPose pose=g->at[frame];float x=pose.x/8.f,y=pose.y/8.f,a=pose.a/4000.f;int xx=sx(x),yy=sy(y);
 if(xx<-40||xx>W+40||yy<-40||yy>H+40)return;
 float co=cs(a),si=sn(a);int px[4],py[4];
 for(int i=0;i<4;i++){
  float u=i<2?16:-16,v=i==0||i==3?-8:8;
  px[i]=xx+(int)(co*u-si*v);py[i]=yy+(int)(si*u+co*v);
 }
 U ink=pose.brake?GOLD:0x93cfea;
 for(int i=0;i<4;i++)line(px[i],py[i],px[(i+1)%4],py[(i+1)%4],ink);
 line(px[3],py[3],xx,yy,ink);line(px[2],py[2],xx,yy,ink);
 for(int j=max(1,frame-48);j<=frame;j+=4)if(g->at[j].brake){
  int ax=sx(g->at[j].x/8.f),ay=sy(g->at[j].y/8.f);rect(ax-1,ay-1,3,3,GOLD);
 }
 dvBadge(x,y-38,pose.brake?"RIVAL  /  BRAKE":"RIVAL",ink);
}
static void dvShadowStatus(void){
 const DvFilm*g=dvRacingGhost();
 if(!g){vd_status(job.step?"First recording. Follow the circuit; the race clock pauses on the atlas.":"First recording. Cross START eastbound in the marked licensed car.");return;}
 if(!job.step){vd_status("Rival loaded. Cross START heading east. Enter: toggle shadow. Race clock pauses on atlas.");return;}
 char line[175],*q=append(line,dvVisible?"Shadow ON. ":"Shadow OFF. ");
 if(dvCurrentSplit&&dvCurrentSplit<LEN(g->splits)){
  int d=dvRaw.splits[dvCurrentSplit]-g->splits[dvCurrentSplit];
  q=number(append(q,"Gate "),dvCurrentSplit);q=append(q,d==0?" level. ":d<0?" ahead ":" behind ");
  if(d){q=sprint_time(q,((d<0?-d:d)*10+30)/60);q=append(q,". ");}
 }
 append(q,"Enter: toggle rival. Gold trail = braking. Atlas pauses race.");vd_status(line);
}
static int vd_b9(int ev,U keys,U tap){
 (void)keys;
 if(ev==VE_START){
  dvShadowPhase=dvNewLap=0;dvVisible=1;dvCurrentSplit=0;
  vd_goal(loc[6].x,loc[6].y,"Glass Mile / timekeeper");
  vd_status(dvRacingGhost()?"Glass Mile: E at the outdoor timekeeper to race your recorded rival. A free licensed car is available.":
    "Glass Mile: E at the outdoor timekeeper to record your first lap. Free loaner; a finished lap can become your rival.");
  return 0;
 }
 if(ev==VE_RESET){dvLast.n=dvRaw.n=dvRival.n=dvRecording=dvNewLap=dvShadowPhase=0;return 0;}
 if(ev==VE_CANCEL){if(dvShadowPhase==1&&job.kind==JOB_SPRINT)end_job();dvRecording=dvNewLap=dvShadowPhase=0;return 0;}
 if(ev==VE_DECOR){dvStartArrow();return 0;}
 if(ev==VE_DRAW&&scene==1&&dvShadowPhase==1&&job.kind==JOB_SPRINT){
  /* Stock job occupies the upper objective card. Keep the rival cue and
   * live split visible as a separate two-line slate below its instructions. */
  int y=152;char row[130];const char*next=hud_line(vdLine,551,row);
  box(17,y,577,*next?48:28,DARK,0x6f96a3);textat(29,y+2,row,0xaddef0,0);
  if(*next){hud_line(next,551,row);textat(29,y+23,row,PAPER,0);}
  return 0;
 }
 if(ev==VE_WORLD){if(!dvShadowPhase)dvBadge(loc[6].x,loc[6].y+9,"TIMEKEEPER  /  E",GOLD);else dvShadowPaint();return 0;}
 if(ev!=VE_TICK||scene!=1)return 0;
 if(!dvShadowPhase){
  if(!(tap&USE)||!dvAt(loc[6].x,loc[6].y,44)||(p.car>=0&&absf(cars[p.car].s)>7))return 0;
  int was=navHasManual,manualLoc=navManualLoc;
  vdActive=-1;beginJob(JOB_SPRINT);vdActive=VA_B9;navHasManual=was;navManualLoc=manualLoc;
  if(job.kind!=JOB_SPRINT)return 1;
  if(p.car<0&&jobRide>0&&cars[jobRide].owner==2){
   /* Only move the Vectra this very call just issued, not the personal
    * or previously licensed ride. Prefer a clear eastbound grid spot. */
   int staged=0;static const int row[]={0,11,-14,22,-22};
   for(int dx=62;dx<=122&&!staged;dx+=20)for(int j=0;j<LEN(row)&&!staged;j++){
    Car c=cars[jobRide];c.x=job.xs[0]-dx;c.y=job.ys[0]+row[j];c.a=0;c.s=0;
    if(!street_car_clear(jobRide,0,&c))continue;
    int clear=1;for(int n=0;n<LEN(peds);n++)if(peds[n].hp>0&&street_foot_car_depth(&c,peds[n].x,peds[n].y,3)>0)clear=0;
    if(clear){street_stage_car(jobRide,c);staged=1;}
   }
  }
  dvShadowPhase=1;vd_goal(-1,-1,0);dvShadowStatus();return 1;
 }
 if(dvShadowPhase==2){
  if(vdChoice){int keep=vdChoice==1;
   if(keep)dvCopyFilm(&dvRival,&dvRaw);
   dvNewLap=dvShadowPhase=0;vd_end();
   tell(keep?"Glass Mile archive: this lap is your session rival.":"Lap discarded. Your previous rival is unchanged.");
  }return 0;
 }
 if(dvNewLap){
  dvShadowPhase=2;vd_status("Glass Mile complete. Keep or discard this recording.");
  char title[100];char*q=sprint_time(append(title,"Glass Mile finish / "),((dvRaw.n-1)*10+30)/60);append(q,". Keep this lap as your rival?");
  vd_choose(title,"Keep this lap / replace the session rival","Discard this lap / leave the previous rival unchanged");return 1;
 }
 if(job.kind!=JOB_SPRINT){dvShadowPhase=dvRecording=0;vd_end();return 0;}
 if(tap&ENTER){dvVisible=!dvVisible;dvShadowStatus();return 1;}
 dvShadowStatus();return 0;
}
