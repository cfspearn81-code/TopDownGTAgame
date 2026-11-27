/* A small, local bartender/queue simulation. It neither carries street
 * actors across the doorway nor advances RNG, jobs, wallets or saves. */
enum {
 V39_BAR_X=551,V39_BAR_TOP=219,V39_BAR_W=125,V39_BAR_H=37,
 V39_PLAYER_X=614,V39_PLAYER_Y=281,V39_STAFF_X=706,V39_STAFF_Y=165,
 V39_CYCLE=224,V39_POUR=72,V39_HANDOFF=112,V39_SIP=156
};
static struct {int tick,serves,patron,requested,playerTick,playerServed;} v39;
static const int v39Px[]={572,657},v39Py[]={281,281},v39WaitX[]={549,674},v39WaitY[]={301,298};
static int v39_staff_near(void){
 return !interior_subroom&&d2(interior_x-V39_STAFF_X,interior_y-V39_STAFF_Y)<31*31;
}
static int v39_staff_visible(void){
 return !interior_subroom&&interior_x>=650&&interior_y<235;
}
static void v39_reset(void){z(&v39,sizeof v39);}
static void v39_expire_caption(const char*speaker){
 const char*s=notice;while(*speaker&&*speaker==*s){speaker++;s++;}
 if(!*speaker)p.noticeT=0;
}
static void v39_order(void){
 if(v39.playerServed){v39.playerServed=0;tell(npc_replaces(N_ACT_MIKA)?"You collect the house soda Mika already left on the counter.":"Mika: Enjoy. If you're meeting Ivo, use the staff door past the bar.");return;}
 if(npc_replaces(N_ACT_MIKA)){tell("Mika is away from the counter. Service will resume when she returns.");return;}
 if(v39.requested||v39.playerTick){tell("Mika: I have your order. I'll bring it to the bar.");return;}
 v39.requested=1;
 tell("Mika: One house soda, coming up. Stay by the counter.");
}
static void v39_bar_tick(void){
 if(npc_replaces(N_ACT_MIKA))return; /* No service from a bartender absent in an encounter. */
 if(v39.playerTick){
  if(++v39.playerTick==81){
   v39.playerTick=0;v39.playerServed=1;
   if(int_at_desk())tell("Mika: House soda is ready. Press E to take it.");
   else tell("Mika left your soda at the bar.");
  }
  return;
 }
 if(++v39.tick==V39_HANDOFF)v39.serves=(v39.serves+1)&255;
 if(v39.tick==V39_CYCLE){
  v39.tick=0;v39.patron^=1;
  if(v39.requested){v39.playerTick=1;v39.requested=0;}
 }
}
static void v39_club_plan(U a){
 int_counter(INT_DINER,V39_BAR_X,V39_BAR_TOP,V39_BAR_W,V39_BAR_H,a);
 int_prop(INT_RACK,537,148,119,27,0x947986);
 int_prop(INT_SOFA,198,318,115,28,0x7e5877);
 int_prop(INT_SOFA,551,330,94,27,0x765079);
 int_prop(INT_REEL,192,232,49,45,0xcc8dad);
 /* The bar-side return wall makes the staff door a recess, leaving a
  * clear route around the counter. Collision follows the actual props. */
 int_prop(INT_WALL,671,146,10,56,0x626076);
}
static void v39_rear_plan(U a){
 int_counter(INT_DINER,410,223,145,38,a);
 int_prop(INT_SOFA,224,154,117,37,0x7e5877);
 int_prop(INT_LOCKER,532,149,108,29,0x715466);
 int_prop(INT_RACK,213,231,90,32,0x785c76);
 int_prop(INT_CRATES,641,263,75,43,0xa18b6c);
 int_prop(INT_SOFA,586,329,143,26,0x765079);
}
static int v39_motion(int t,int a,int b){return cl((t-a)*100/max(1,b-a),0,100);}
static void v39_patron_at(int i,int*t,int*x,int*y){
 int p=i==v39.patron&&!v39.playerTick?v39.tick:0;
 int shift=p<V39_POUR?v39_motion(p,0,59):100-v39_motion(p,V39_SIP+12,V39_CYCLE-2);
 *t=p;*x=(v39WaitX[i]*(100-shift)+v39Px[i]*shift)/100;
 *y=(v39WaitY[i]*(100-shift)+v39Py[i]*shift)/100;
}
static void v39_cup(int x,int y,U ink){
 box(x-3,y-5,7,8,0x373e48,PAPER);rect(x-2,y-3,5,4,ink);dot(x-1,y-4,PAPER);
}
static int v39_person(int id,int dx,int dy,float x,float y,U coat,int step,float a,int style,int hue){
 if(npc_replaces(id))return 1;
 int held=npc_present_pose(id,&x,&y,&a,&step);
 person((int)x-dx,(int)y-dy,coat,step,a,style,hue);
 return held!=0;
}
static void v39_actor_paint(int dx,int dy){
 int i=v39.patron,t=v39.tick,m=v39.playerTick;
 int phase=m?v39_motion(m,1,25):t<V39_SIP?v39_motion(t,0,58):100-v39_motion(t,V39_SIP,V39_CYCLE);
 int target=m?V39_PLAYER_X:v39Px[i];
 const int floorX[]={383,450,405},floorY[]={263,287,324};
 const U floorCoat[]={0xa987ba,0xc7a578,0x729db7};
 for(int j=0;j<LEN(floorX);j++){
  float a=anim*(1.6f+.35f*j)+j*2.2f;
  v39_person(N_ACT_DANCE+j,dx,dy,floorX[j]+(int)(sn(a)*7),floorY[j]+(int)(cs(a)*4),
         floorCoat[j],(int)(sn(a*3.1f)*2),a,MODEL_CIVILIAN,2+j);
 }
 v39_person(N_ACT_DJ,dx,dy,301,150,0xc6a8d7,(int)(sn(anim*4.f)*2),1.570796f,MODEL_STAFF,3);
 int bx=(V39_PLAYER_X*(100-phase)+target*phase)/100,by=V39_BAR_TOP-12;
 U tone=i?0xe0ab87:0xaee6cf;
 if(m)tone=0xf2ca94;
 int mikaHeld=v39_person(N_ACT_MIKA,dx,dy,bx,by,0x8bbeb4,phase>0&&phase<100?(int)(anim*12)%3:0,
  phase>0&&phase<100&&m==0?(target>V39_PLAYER_X?0.f:3.14159f):1.570796f,MODEL_STAFF,4);
 for(int j=0;j<2;j++){
  int pt,px,py;v39_patron_at(j,&pt,&px,&py);
  float facing=pt>=V39_SIP+12?1.570796f:-1.570796f;
  int held=v39_person(N_ACT_PATRON+j,dx,dy,px,py,j?0xabb7e6:0xca937f,
   pt>0&&(pt<59||pt>V39_SIP+12)?(int)(anim*13)%3:0,facing,MODEL_CIVILIAN,j+1);
  if(!held&&pt>=V39_HANDOFF&&pt<V39_SIP+25){
   int q=v39_motion(pt,V39_HANDOFF,V39_HANDOFF+17);
   v39_cup(v39Px[j]+7*q/100-dx,V39_BAR_TOP+23+(py-17-(V39_BAR_TOP+23))*q/100-dy,tone);
  }
 }
 if(!mikaHeld&&((!m&&t>=V39_POUR&&t<V39_HANDOFF)||(m>=27&&m<61))){
  v39_cup(target-dx,V39_BAR_TOP+12-dy,tone);
  if((m+t)%12<7){line(target-dx,V39_BAR_TOP+1-dy,target-dx,V39_BAR_TOP+7-dy,tone);dot(target+2-dx,V39_BAR_TOP+5-dy,PAPER);}
 }
 if(m>=61||v39.playerServed){
  int q=m?v39_motion(m,61,79):100;
  v39_cup(V39_PLAYER_X-dx,V39_BAR_TOP+15+q/6-dy,0xf2ca94);
 }
 if(v39_staff_visible()){
  const char*s=v39_staff_near()?"E / EMPLOYEES ONLY":"EMPLOYEES ONLY";
  int w=textw(s,0)+14;box(V39_STAFF_X-dx-w/2,121-dy,w,24,DARK,0x81745e);
  center(V39_STAFF_X-dx,122-dy,s,PAPER,0);
 }
}
