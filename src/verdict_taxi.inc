/* Cab Rank and One More Stop: authored offers are transient. Once accepted,
 * both use the normal taxi vehicle, rider, fare clock and completion path. */
#define VD_HAS_A3 1
#define VD_HAS_B1 1

enum { CAB_COPPER_BUILDING=28 };
typedef struct {int from,to,rider;} CabCall;
static const CabCall cabCalls[]={{12,14,9},{2,13,4}};
static const char *const callPickup[]={"APARTMENT 4","FERRY MARKET"};
static char cabTicket[2][156];

static int taxi_copper_x(void){return ordinaryDoorX[CAB_COPPER_BUILDING];}
static int taxi_copper_y(void){return ((ordinaryDoorY[CAB_COPPER_BUILDING]>>8)+1)*256-13;}
static int taxi_via_copper(void){
 return job.kind==JOB_TAXI&&taxi.phase==TAXI_RIDING&&taxi.copper==CAB_VIA;
}
static int taxi_detour_bonus(void){return taxi.copper==CAB_SEEN?20:0;}
static const char *taxi_route_name(void){
 if(taxi_via_copper())return "COPPER PLAYHOUSE";
 if(job.step==0&&taxi.copper!=CAB_NONE)return callPickup[0];
 return loc[job.step==0?taxi.pickup:taxi.dropoff].label;
}
static int taxi_authored_offer(int which){
 if((unsigned)which>=LEN(cabCalls))return 0;
 CabCall c=cabCalls[which];
 int x,y,curb,xx,yy,curb2;
 if(!taxi_curb(c.from,&x,&y,&curb)||!taxi_curb(c.to,&xx,&yy,&curb2))return 0;
 int length=taxi_road_distance(x,y,xx,yy);
 job=(Job){JOB_TAXI,0,2,60+length/16,145.f+length*.021f,{x,xx},{y,yy}};
 taxi=(Taxi){.x=x,.y=curb,.a=1.570796f,.phase=TAXI_WAIT,.ride=-1,
   .pickup=c.from,.dropoff=c.to,.rider=c.rider,.curb={curb,curb2},.meters=length,
   .copper=which==0?CAB_OFFER:CAB_NONE};
 jobRide=-1;lastTaxiPickup=c.from;lastTaxiDrop=c.to;lastTaxiRider=c.rider;
 return 1;
}
/* Quote from the present position using the same street solver as the atlas.
 * The solve only writes these three scratch fields. Restore all three so
 * reading or declining a ticket cannot disturb the player's map or pin. */
static int cab_quote_meters(NavPoint origin,NavPoint goal){
 NavPoint saved[LEN(navNodes)];cp(saved,navNodes,sizeof saved);
 int oldCount=navCount,oldDist=navDist;
 nav_solve(origin,goal);
 int meters=navCount?navDist:taxi_road_distance(origin.x,origin.y,goal.x,goal.y);
 cp(navNodes,saved,sizeof saved);navCount=oldCount;navDist=oldDist;
 return meters;
}
static int cab_tickets(void){
 for(int i=0;i<LEN(cabCalls);i++){
  CabCall c=cabCalls[i];int x,y,xx,yy,curb;
  if(!taxi_curb(c.from,&x,&y,&curb)||!taxi_curb(c.to,&xx,&yy,&curb))return 0;
  NavPoint a={x,y},b={xx,yy};
  int empty=cab_quote_meters(nav_player(),a),occupied=cab_quote_meters(a,b);
  char *q=append(cabTicket[i],riders[c.rider].name);q=append(q,": ");
  q=append(q,callPickup[i]);q=append(q," to ");q=append(q,loc[c.to].label);
  q=append(q,". Pickup ");q=number(q,empty);q=append(q," m / trip total ");
  q=number(q,empty+occupied);q=append(q," m / base $");q=number(q,60+taxi_road_distance(x,y,xx,yy)/16);
  append(q,i?". Safe-driving tip extra.":". Optional Copper +$20; tip extra.");
 }
 return 1;
}
static void cab_start(int which){
 /* End the activity shell. From here the ordinary taxi owns deadlines, GPS,
  * cancel-on-exit, abandonment and the SINGLE payout/save transaction. */
 vd_end();
 if(job.kind>=0){tell("Finish or abandon the current job first.");return;}
 cabRequested=which+1;beginJob(JOB_TAXI);cabRequested=0;
 if(job.kind!=JOB_TAXI)return;
 tell(which?"Sasha is waiting at Ferry Market. Take a licensed car.":"Pick up Ren outside Apartment 4. Destination: Vanta Police HQ.");
}
static int vd_a3(int ev,U keys,U tap){
 (void)keys;(void)tap;if(ev==VE_START)cab_start(0);
 return 0;
}
static int vd_b1(int ev,U keys,U tap){
 (void)keys;(void)tap;
 if(ev==VE_START){
  if(!cab_tickets()){vd_end();tell("Cab Rank is off air. No curb available.");return 0;}
  vd_choose("CAB RANK / Compare road estimates. Trip total includes the pickup.",cabTicket[0],cabTicket[1]);
 }else if(ev==VE_TICK&&vdChoice){
  int which=vdChoice-1;if((unsigned)which<LEN(cabCalls))cab_start(which);
  return 1;
 }
 return 0;
}
static void cab_rank_counter(void){
 if(ownedJob())return;
 menu=-1;vd_launch(VA_B1);
}
/* Road trigger deliberately accepts either traffic lane. The sidewalk or an
 * empty taxi does not earn anything. No money is credited until drop-off. */
static void cab_passing_copper(void){
 if(!taxi_via_copper()||!taxi_occupied()||absf(cars[taxi.ride].s)<3.f)return;
 if(absf(p.x-taxi_copper_x())<=37.f&&absf(p.y-(taxi_copper_y()+13))<=30.f){
  taxi.copper=CAB_SEEN;tell("Ren: There! MINA VALE. Mom's going to cry. That's $20 extra at HQ.");
 }
}
static void cab_choose_route(U tap){
 if(!taxi_occupied()||taxi.phase!=TAXI_RIDING)return;
 if(taxi.copper==CAB_OFFER&&(tap&(ENTER|TAXI_NEXT))){
  taxi.copper=(tap&ENTER)?CAB_DIRECT:CAB_VIA;
  tell(taxi.copper==CAB_VIA?"Ren: Take the road facing Copper Playhouse. I'll add $20 at HQ.":"Ren: Straight to Vanta Police HQ, then. Thanks for the lift.");
 }else if(taxi.copper==CAB_VIA&&(tap&TAXI_NEXT)){
  taxi.copper=CAB_DIRECT;tell("Ren: No problem. Straight to HQ for the original fare.");
 }
}
/* A fixed, two-line bulb marquee above the actual theater entrance. Lettering
 * and bulbs remain readable in Low FX; the drive-by uses the facing road. */
static void cab_marquee_paint(void){
 int x=taxi_copper_x()-cx,y=ordinaryDoorY[CAB_COPPER_BUILDING]-cy-33;
 if(x<-42||x>W+42||y< -26||y>H+5)return;
 box(x-38,y+3,79,25,0x342436,0x291c28);
 box(x-40,y,79,25,0x672f36,0xe29a59);
 box(x-35,y+4,69,17,0x312737,0xb37554);
 zone_lettering(x,y+6,"COPPER PLAYHOUSE",0xf5db9e,1);
 zone_lettering(x,y+14,"MINA VALE",0xfd9ca6,1);
 for(int dx=-36;dx<=36;dx+=6){
  U lamp=fastGfx||((dx+36)/6+(int)(anim*3))%3?0xffdda2:0xb57551;
  rect(x+dx,y+1,2,2,lamp);rect(x+dx,y+22,2,2,lamp);
 }
}
