/* Switchboard Drop, the East Freight signal desk and its occasional roll-by.
 * Session-only: no persistent grants, player vehicle staging or rng draws. */
#define VD_HAS_B5 1
#define VD_HAS_B6A 1
#define VD_HAS_B6B 1
enum {SW_PICK_X=1482,SW_NEAR_X=1322,SW_FAR_X=1820,SW_Y=992,
      CR_X=1792,CR_Y=1024,CR_BOOTH_X=1748,CR_BOOTH_Y=1067};
static struct {int tape,clock,target;} swTape;
static int sw_at(int x,int y,int r){
 return scene==1&&p.car<0&&d2(p.x-x,p.y-y)<r*r&&street_line_open(p.x,p.y,x,y,2);
}
static void sw_label(int wx,int wy,const char*s,U col){
 if(ch_record_label(wx,wy,s,col))return;
 int x=sx(wx),y=sy(wy),w=textw(s,0)+12;
 if(x < -w||x > W+w||y < -30||y > H+30)return;
 box(x-w/2,y-22,w,20,0x111e27,col);center(x,y-24,s,col,0);
}
static void sw_cabinet(int wx,int wy,int held,int ready,const char*s){
 int x=sx(wx),y=sy(wy);U col=ready?TEAL:GOLD;
 box(x-9,y-8,18,16,0x263947,col);rect(x-6,y-5,12,4,0x131b24);
 for(int i=0;i<4;i++)rect(x-5+i*3,y+2,1,2,col);
 if(held)diamond(x,y+15,4,col);
 sw_label(wx,wy-6,s,col);
}
static void sw_update(void){
 if(!swTape.tape){vd_goal(SW_PICK_X,SW_Y,"East Freight / cassette cabinet");vd_status("At the cassette cabinet: E on foot to take the tape and start pursuit. $90 dirty; no weapon needed.");return;}
 int secs=(480-swTape.clock+59)/60;if(secs<0)secs=0;
 char out[180],*e=append(out,"WEST / RELAY: ");
 if(secs){e=number(append(e,"WAIT "),secs);e=append(e,"s");}else e=append(e,"READY");
 append(e,"   |   EAST / NIGHT DESK: READY. E on foot at either open slot. Keep moving or wait; same $90.");
 vd_status(out);vd_goal(swTape.target==1?SW_NEAR_X:SW_FAR_X,SW_Y,swTape.target==1?"West / relay slot":"East / night desk");
}
static int vd_b5(int ev,U keys,U tap){
 (void)keys;
 if(ev==VE_RESET||ev==VE_CANCEL){swTape.tape=swTape.clock=swTape.target=0;return 0;}
 if(ev==VE_START){swTape.tape=swTape.clock=0;swTape.target=2;sw_update();return 0;}
 if(ev==VE_WORLD&&scene==1){
  if(!swTape.tape)sw_cabinet(SW_PICK_X,SW_Y,0,1,"SWITCHBOARD / CASSETTE");
  else{sw_cabinet(SW_NEAR_X,SW_Y,1,swTape.clock>=480,swTape.clock>=480?"WEST / READY":"WEST / WAIT");
       sw_cabinet(SW_FAR_X,SW_Y,1,1,"EAST / READY");}
 }
 if(ev!=VE_TICK||scene!=1)return 0;
 if(!swTape.tape){
  if((tap&USE)&&sw_at(SW_PICK_X,SW_Y,33)){
   /* Selecting a route is the last chance to decline. No charge or alert
    * just for inspecting the cabinet; no underlying personal pin changes. */
   swTape.tape=1;
   vd_choose("Switchboard: take the cassette and trigger pursuit?",
             "WEST / Relay, 160 m. WAIT 8 seconds, then READY. $90 dirty.",
             "EAST / Night desk, 338 m. READY now, stays open. $90 dirty.");
   return 1;
  }
  return 0;
 }
 if(vdChoice){
  swTape.target=vdChoice;
  if(p.heat<40.f)heat(40.f-p.heat);
  tell("Cassette collected. Both contacts honor the tape, even with heat.");
 }
 if(swTape.clock<480)swTape.clock++;
 sw_update();
 if(!(tap&USE))return 0;
 int atWest=sw_at(SW_NEAR_X,SW_Y,35),atEast=sw_at(SW_FAR_X,SW_Y,35);
 if(!atWest&&!atEast)return 0;
 if(atWest&&swTape.clock<480){tell("WEST RELAY / WAIT for its lamp, or take the tape east. NIGHT DESK / READY.");return 1;}
 vd_end();swTape.tape=0;
 gain(0,90,atWest?"West relay took the cassette.":"Night desk took the cassette.");request_save();return 1;
}

/* Four cargo tractors on two short freight lanes. These are scene equipment,
 * not borrowable road cars. Queued heads physically move, and both moving
 * equipment and road traffic give way to people. No global traffic changes. */
typedef struct {int dir,live,moving;float x,y;} CrTug;
static struct {
 int mode,locked,served[2],worker,request,walks,ticks,entry;
 int cooldown;
 float workerY;
 CrTug tug[4];
} cr;
enum {CR_OFF,CR_KEEPER,CR_CONVOY};
static int cr_near(float x,float y,float r){return d2(p.x-x,p.y-y)<r*r;}
static int cr_area(void){return absf(p.x-CR_X)<168&&absf(p.y-CR_Y)<310;}
static int cr_effective(void){
 if(!cr.locked||scene!=1||menu>=0||mapOn||p.dead||vdPanel)return 0;
 if(cr.mode==CR_KEEPER)return vdActive==VA_B6A&&p.car<0&&cr_near(CR_BOOTH_X,CR_BOOTH_Y,86);
 return cr.mode==CR_CONVOY&&p.car>=0&&cr_area()&&job.kind<0&&(vdActive<0||vdActive==VA_B6B);
}
static int cr_clear_run(void){
 int previous=cr.mode;int cd=cr.cooldown;
 z(&cr,sizeof cr);cr.cooldown=cd;
 return previous;
}
static int cr_walkers_in_box(void){
 for(int i=0;i<LEN(streetWalks);i++)if(peds[i].hp>0&&streetWalks[i].cross&&
  absf(peds[i].x-CR_X)<53&&absf(peds[i].y-CR_Y)<48)return 1;
 return 0;
}
static int cr_native_car_at(float x,float y,int halfx,int halfy){
 for(int i=street_next_car(-1);i<LEN(cars);i=street_next_car(i))if(absf(cars[i].x-x)<halfx+19&&absf(cars[i].y-y)<halfy+19){
  /* Use the rotated native chassis rather than demanding a clear city
   * block. Parked curbside vehicles have room beside the narrow tugs. */
  const Car*c=cars+i;
  if(absf(c->x-x)<halfx+absf(cs(c->a))*STREET_HALF_L+absf(sn(c->a))*STREET_HALF_W&&
     absf(c->y-y)<halfy+absf(sn(c->a))*STREET_HALF_L+absf(cs(c->a))*STREET_HALF_W)return 1;
 }
 return 0;
}
static int cr_near_person(float x,float y,float w,float h){
 if(p.car<0&&absf(p.x-x)<w+5&&absf(p.y-y)<h+5)return 1;
 for(int i=0;i<LEN(peds);i++)if(peds[i].hp>0&&absf(peds[i].x-x)<w+4&&absf(peds[i].y-y)<h+4)return 1;
 return 0;
}
static float cr_depth(float x,float y,float ox,float oy,float w,float h,float r){
 float dx=w+r-absf(x-ox),dy=h+r-absf(y-oy);
 return dx>0&&dy>0?(dx<dy?dx:dy):0;
}
static int cr_deeper(float x,float y,float nx,float ny,float ox,float oy,float w,float h,float r){
 float old=cr_depth(x,y,ox,oy,w,h,r),now=cr_depth(nx,ny,ox,oy,w,h,r);
 return now>0&&(now>old+.001f||(absf(now-old)<=.001f&&d2(nx-ox,ny-oy)<d2(x-ox,y-oy)));
}
static int vd_cross_foot(float x,float y,float nx,float ny,float r){
 if(!cr.locked||!cr_effective()||absf(x-CR_X)>155||absf(y-CR_Y)>70)return 0;
 for(int i=0;i<4;i++){CrTug*t=cr.tug+i;if(t->live&&cr_deeper(x,y,nx,ny,t->x,t->y,9,5,r))return 1;}
 return 0;
}
static int cr_stop_vector(const Car*c,float dx,float dy,int soft){
 if(absf(c->x-CR_X)>166||absf(c->y-CR_Y)>103)return 0;
 float lead=soft?15.f+c->s*.27f:0.f;
 /* Only hold the approach: anyone already inside a gate can drive out.
  * Reverse, parallel travel and the adjacent bypass remain available. */
 if(absf(dy)>=absf(dx)&&absf(c->x-CR_X)<33&&
   ((dy>0&&c->y<=CR_Y-63&&c->y+dy+lead>=CR_Y-63)||
    (dy<0&&c->y>=CR_Y+63&&c->y+dy-lead<=CR_Y+63)))return 1;
 if(absf(dx)>absf(dy)&&absf(c->y-CR_Y)<32&&
   ((dx>0&&c->x<=CR_X-129&&c->x+dx+lead>=CR_X-129)||
    (dx<0&&c->x>=CR_X+129&&c->x+dx-lead<=CR_X+129)))return 1;
 /* The moving equipment and WALK signal are also physical even if an
  * unconventional approach bypasses a stop line. */
 float ax=absf(cs(c->a)),ay=absf(sn(c->a));
 float w=ax*STREET_HALF_L+ay*STREET_HALF_W,h=ay*STREET_HALF_L+ax*STREET_HALF_W;
 for(int i=0;i<4;i++){CrTug*t=cr.tug+i;if(t->live&&cr_deeper(c->x,c->y,c->x+dx,c->y+dy,t->x,t->y,9+w,5+h,0))return 1;}
 return cr.worker&&cr_deeper(c->x,c->y,c->x+dx,c->y+dy,CR_X+28,cr.workerY,4+w,4+h,0);
}
static int vd_cross_hold(const Car*c,float fx,float fy){
 return cr.locked&&cr_effective()&&cr_stop_vector(c,fx,fy,1);
}
static int vd_cross_player_stop(const Car*c,float dx,float dy){
 return cr.locked&&cr_effective()&&(dx!=0||dy!=0)&&cr_stop_vector(c,dx,dy,0);
}
static int cr_start(int mode){
 /* Do not materialize a cart on top of a passer-by. Wait for the native
  * street to clear its berth; E/Enter never forces a blocked launch. */
 CrTug next[4];
 for(int i=0;i<4;i++){
  int side=mode==CR_KEEPER?i/2:0,index=i%2,dir=side?-1:1;
  int live=mode==CR_KEEPER||i<2;
  float x=CR_X-dir*(61+index*30),y=CR_Y+dir*9;
  if(live&&(cr_native_car_at(x,y,10,5)||cr_near_person(x,y,10,5)))return 0;
  next[i]=(CrTug){dir,live,0,x,y};
 }
 cr.locked=1;cr.mode=mode;cr.ticks=cr.worker=cr.request=cr.walks=0;cr.workerY=CR_Y-41;
 cr.served[0]=cr.served[1]=0;cp(cr.tug,next,sizeof next);return 1;
}
static int cr_active_cart(void){for(int i=0;i<4;i++)if(cr.tug[i].live&&cr.tug[i].moving)return 1;return 0;}
static int cr_release(int side){
 if(cr.request||cr.worker||cr_walkers_in_box()||cr_active_cart())return 0;
 int first=side*2;
 for(int i=first;i<first+2;i++)if(cr.tug[i].live){cr.tug[i].moving=1;return 1;}
 return 0;
}
static void cr_move(void){
 cr.ticks++;
 if(cr.worker){
  if(npc_holds(N_ACT_CROSS)||npc_replaces(N_ACT_CROSS)||street_extra_down(N_ACT_CROSS))return;
  float dest=cr.worker==1?CR_Y+41:CR_Y-41;
  float sign=cr.worker==1?1.f:-1.f,ny=cr.workerY+sign*.72f;
  if(cr_native_car_at(CR_X+28,ny,4,4))return;
  cr.workerY=ny;
  if(sign*(dest-ny)<=0){cr.workerY=dest;cr.worker=0;cr.walks++;}
  return;
 }
 for(int i=0;i<4;i++){
  CrTug*t=cr.tug+i;if(!t->live||!t->moving)continue;
  float nx=t->x+t->dir*1.03f;
  if(cr_native_car_at(nx,t->y,10,5)||cr_near_person(nx,t->y,10,5))continue;
  t->x=nx;
  if(t->dir*(nx-CR_X)>112){
   t->live=t->moving=0;cr.served[i/2]++;
   if(cr.mode==CR_KEEPER&&(cr.served[0]+cr.served[1])%2==1)cr.request=1;
  }
 }
}
static void cr_ink_tug(const CrTug*t,int id){
 int x=sx(t->x),y=sy(t->y),dir=t->dir;U c=dir>0?0xf2ba57:0x78c9df;
 rect(x-10,y-6,20,3,0x121c25);rect(x-10,y+4,20,3,0x121c25);
 box(x-11,y-5,22,10,c,0x253541);rect(x+(dir>0?4:-9),y-3,5,6,0x28536b);
 for(int k=-4;k<=2;k+=3)rect(x+k,y-3,1,6,0x915f4b);
 rect(x+dir*10,y-3,2,2,PAPER);rect(x+dir*10,y+2,2,2,PAPER);
 box(x-7,y-18,14,12,0x20323a,c);
 /* White tally, legible at Low FX; no dependence on red/green. */
 for(int i=0;i<=id%2;i++)rect(x-2+i*4,y-15,2,6,PAPER);
}
static void cr_bar(int x,int y,int vertical){
 int len=48;
 if(vertical){rect(x-2,y-len/2,4,len,0x362e2c);for(int j=0;j<len;j+=8)rect(x-2,y-len/2+j,4,4,GOLD);}
 else{rect(x-len/2,y-2,len,4,0x362e2c);for(int j=0;j<len;j+=8)rect(x-len/2+j,y-2,4,4,GOLD);}
}
static void vd_cross_paint(void){
 int keeper=vdActive==VA_B6A,driver=vdActive==VA_B6B||cr.mode==CR_CONVOY;
 if(scene!=1||(!keeper&&!driver))return;
 int x=sx(CR_X),y=sy(CR_Y);
 if(x< -420||x>W+420||y< -380||y>H+380)return;
 if(keeper){
  int bx=sx(CR_BOOTH_X),by=sy(CR_BOOTH_Y);
  box(bx-12,by-7,24,14,0x233947,GOLD);rect(bx-8,by-4,4,4,TEAL);rect(bx+4,by-4,4,4,GOLD);
  sw_label(CR_BOOTH_X-40,CR_BOOTH_Y+34,cr.locked?"SIGNAL DESK / ON DUTY":"SIGNAL DESK / E",GOLD);
 }
 if(driver){
  sw_label(CR_X,CR_Y-236,"< BYPASS WEST 1 BLOCK",GOLD);
  sw_label(CR_X,CR_Y+236,"< BYPASS WEST 1 BLOCK",GOLD);
  sw_label(CR_X-256,CR_Y,"BYPASS / THROUGH TRAFFIC",TEAL);
 }
 sw_label(CR_X+18,CR_Y-68,cr.locked?"STOP / FREIGHT":"NORTH GATE / OPEN",cr.locked?GOLD:TEAL);
 sw_label(CR_X+18,CR_Y+115,cr.locked?"STOP / FREIGHT":"SOUTH GATE / OPEN",cr.locked?GOLD:TEAL);
 if(!cr.locked)return;
 for(int k=0;k<8;k++)rect(x+22,y-37+k*11,12,4,0xbdc2af);
 cr_bar(x,y-43,0);cr_bar(x,y+43,0);cr_bar(x-110,y,1);cr_bar(x+110,y,1);
 for(int i=0;i<4;i++)if(cr.tug[i].live)cr_ink_tug(cr.tug+i,i);
 if(cr.mode==CR_KEEPER){
  float wx=CR_X+28,wy=cr.workerY,wa=cr.worker==2?-1.57f:1.57f;
  int step=cr.worker?(cr.ticks/9)%4:0;
  npc_present_pose(N_ACT_CROSS,&wx,&wy,&wa,&step);
  if(!npc_replaces(N_ACT_CROSS)&&!street_extra_down(N_ACT_CROSS))person(sx(wx),sy(wy),0xc0dc8d,step,wa,0,2);
  if(cr.worker||cr.request)sw_label(CR_X+28,(int)cr.workerY-20,cr.worker?"WALK / HOLD":"WALK REQUEST",PAPER);
 }
}
static void cr_keeper_status(void){
 char buf[200],*e=number(append(buf,"WEST [E] "),2-cr.served[0]);
 e=number(append(e,"  /  EAST [Enter] "),2-cr.served[1]);
 if(cr.request)append(e,"  |  WALK REQUEST. Enter to stop both queues and give the pedestrian a turn.");
 else if(cr.worker||cr_walkers_in_box())append(e,"  |  WALK. Hold both queues until the stripe clears.");
 else if(cr_active_cart())append(e,"  |  Freight moving. Watch for the walk signal.");
 else append(e,"  |  CLEAR. Release one waiting tug. Step away to hand back the desk.");
 vd_status(buf);
}
static int vd_b6a(int ev,U keys,U tap){
 (void)keys;
 if(ev==VE_RESET||ev==VE_CANCEL){if(cr.mode==CR_KEEPER)cr_clear_run();return 0;}
 if(ev==VE_START){cr_clear_run();vd_goal(CR_BOOTH_X,CR_BOOTH_Y,"East Freight / signal desk");
  vd_status("Walk to the striped signal desk by Wrecker Yard. E to take duty. Two queues, one pedestrian crossing.");return 0;}
 if(ev!=VE_TICK)return 0;
 if(cr.mode==CR_KEEPER&&!cr_effective()){
  cr_clear_run();vd_end();tell("Desk handed back. Both approaches have normal right of way.");return 0;
 }
 if(scene!=1)return 0;
 if(!cr.locked){
  if((tap&USE)&&sw_at(CR_BOOTH_X,CR_BOOTH_Y,31)){
   if(!cr_start(CR_KEEPER)){tell("Freight berth occupied. Let the street clear, then press E.");return 1;}
   cr_keeper_status();tell("Two west tugs, two east tugs. E or Enter releases one. WALK stops both.");return 1;
  }return 0;
 }
 cr_move();
 if(cr.served[0]+cr.served[1]>=4){
  cr_clear_run();vd_end();gain(1,75,"All four tugs crossed. Desk back on automatic.");request_save();return 1;
 }
 cr_keeper_status();
 if(tap&(USE|ENTER)){
  if(cr.request){
   if(tap&ENTER){cr.request=0;cr.worker=cr.walks%2?2:1;tell("WALK granted. Both freight signals are locked at STOP.");}
   else tell("Pedestrian waiting. Press Enter to give WALK before the next tug.");
  }else if(!cr_release(!!(tap&ENTER)))tell(cr.worker||cr_walkers_in_box()?"WALK. Both signals stay at STOP.":cr_active_cart()?"Wait for the tug to clear.":"That queue is clear. Release the other one.");
  cr_keeper_status();return 1;
 }
 return 0;
}
/* Non-job ambient event: even when scheduled by the city's clock it never
 * calls vd_launch, changes the atlas route, pays out or occupies a job. */
static int cr_approaching(void){
 if(p.car<0||!cr_area()||absf(p.x-CR_X)>42||absf(p.y-CR_Y)<72)return 0;
 Car*c=cars+p.car;float along=sn(c->a);
 return absf(along)>.56f&&((p.y<CR_Y&&along>0)||(p.y>CR_Y&&along<0));
}
static void cr_convoy_end(int active,const char*notice){
 cr_clear_run();cr.cooldown=60*105;
 if(active)vd_end();
 if(notice)tell(notice);
}
static void cr_set_driver_goal(void){
 vd_goal(CR_X,CR_Y+cr.entry*225,cr.entry<0?"East Freight / north approach":"East Freight / south approach");
 vd_status("Approach the signal in any car. Stop for the freight tugs, or take the parallel road one block west.");
}
static int vd_b6b(int ev,U keys,U tap){
 (void)keys;(void)tap;
 if(ev==VE_RESET){cr_clear_run();cr.cooldown=0;return 0;}
 if(ev==VE_CANCEL){if(cr.mode==CR_CONVOY){cr_clear_run();cr.cooldown=60*105;}return 0;}
 if(ev==VE_START){
  cr_clear_run();cr.entry=p.y>CR_Y?1:-1;cr_set_driver_goal();return 0;
 }
 if(ev!=VE_AMBIENT)return 0;
 if(cr.cooldown>0)cr.cooldown--;
 int active=vdActive==VA_B6B;
 if(cr.mode==CR_KEEPER)return 0;
 if(!active&&(job.kind>=0||vdActive>=0||p.dead||p.heat>0||scene!=1||p.car<0)){
  if(cr.mode==CR_CONVOY)cr_convoy_end(0,0);return 0;
 }
 if(scene!=1||p.dead){if(cr.mode==CR_CONVOY)cr_convoy_end(active,0);return 0;}
 if(cr.mode!=CR_CONVOY){
  if(!active){
   /* Sparse, deterministic timetable, only after the introductory shift,
    * outside existing careers and pursuits, on this single approach. */
   if(cr.cooldown||sv.intro<4||sv.playedTicks<7200u||(sv.playedTicks/3600u)%4u!=2u)return 0;
  }
  if(!cr_approaching())return 0;
  if(!cr_start(CR_CONVOY))return 0;
  cr.entry=p.y<CR_Y?-1:1;
  if(active){vd_goal(CR_X,CR_Y-cr.entry*114,"East Freight / beyond the gates");
   vd_status("STOP at the bars. Two tugs are crossing. Wait for OPEN, or take the signed road one block west.");}
  tell("EAST FREIGHT / STOP. Two tugs crossing. Bypass: one block WEST.");
 }
 if(p.car<0||p.dead){cr_convoy_end(active,"Road signal back on automatic.");return 0;}
 if(cr.locked&&!cr_area()){
  /* Release the moment the driver leaves the controlled zone. Continue
   * observing their choice only; no city priority or wallet change. */
  cr.locked=0;
  for(int i=0;i<4;i++)cr.tug[i].live=0;
  if(active)vd_status("Freight control released. To take the bypass, cross on the next parallel road to the west.");
 }
 if(cr.locked){
  cr_move();
  if(!cr_active_cart()&&cr.ticks>=28)cr_release(0);
  if(cr.served[0]>=2){
   cr.locked=0;
   if(active)vd_status("OPEN. Continue through the raised signal and drive beyond the opposite stop line.");
   tell("FREIGHT CLEAR / OPEN. Cross when the street is clear.");
  }else if(cr.ticks>900){
   /* A blocked berth, e.g. a parked player car, must never create an
    * indefinite red signal. A dispatcher holds the rest at source. */
   cr.locked=0;for(int i=0;i<4;i++)cr.tug[i].live=0;
   if(active)vd_status("CARTS HELD AT DEPOT / OPEN. Cross when the street is clear.");
   tell("Control held the remaining freight. Vehicle signals OPEN.");
  }
 }
 if((p.y-CR_Y)*cr.entry < -55){
  if(absf(p.x-(CR_X-256))<71){cr_convoy_end(active,"West bypass: you crossed the freight line without waiting.");return 0;}
  if(!cr.locked&&absf(p.x-CR_X)<80){cr_convoy_end(active,"Across the freight line. The signal is back on automatic.");return 0;}
 }
 if(absf(p.y-CR_Y)>370||p.x<CR_X-365||p.x>CR_X+180){
  cr_convoy_end(active,"Freight zone left. Control released; normal traffic resumes.");
 }
 return 0;
}
