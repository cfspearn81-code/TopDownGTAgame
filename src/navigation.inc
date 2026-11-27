/* Port Vanta cartography and navigation. Included after the game drawing helpers. */
#define NAV_INK 0x101522u
#define NAV_SEA 0x12283bu
#define NAV_GRID 0x667483u
#define NAV_CURB 0x465264u
#define NAV_RIM 0x47566du
#define NAV_VIOLET 0xc68affu
#define NAV_DARK 0x251331u
#define NAV_WALK 0xe4c8f8u
enum{NAV_MAP_X=256,NAV_MAP_Y=50,NAV_MAP_SIZE=448,NAV_ATM_BASE=LEN(loc)+LEN(buildings)};
typedef struct{int x,y;} NavPoint;
typedef struct{int x,y,w,h,wx,wy,span;} NavView;
typedef struct{NavPoint a,via;int axis,walk,bent;} NavAnchor;
static NavPoint navCursor,navManual,navGoal,navOrigin,navCurb,navNodes[80];
static int navHasManual,navGoalKind,navCount,navDist,navSelected=-1,navReady,navManualLoc=-1,navVenue=-1,navLeg,navSlot=-1,navHover,navKeyboard,navPointerX=-999,navPointerY=-999;
static float navNext;
static int outerAtlas,navLastClosure=-1;
/* The subscription has its own negative identity, distinct from a dropped
 * map pin (-1); suggestions and missions that set a pin still replace it. */
enum{NAV_LIVE_CART=-2};
static int navCartOpen,navCartClock;
static int nav_cart_tracking(void){return navHasManual&&navManualLoc==NAV_LIVE_CART;}
static int nav_span(void){return outerAtlas||outer_is_new(p.x,p.y)?ROOM:BASE_ROOM;}
static int nav_abs(int x){return x<0?-x:x;}
static int nav_dist(NavPoint a,NavPoint b){return nav_abs(a.x-b.x)+nav_abs(a.y-b.y);}
/* The original square taxi tariff and grid-anchor cost use Manhattan
 * units. Distances displayed and weighted along surveyed diagonal roads
 * use actual polyline length without changing those legacy rewards. */
static int nav_length(NavPoint a,NavPoint b){
 int dx=nav_abs(a.x-b.x),dy=nav_abs(a.y-b.y);
 return !dx||!dy?dx+dy:(int)(sq(d2(dx,dy))+.5f);
}
static int nav_same(NavPoint a,NavPoint b){return a.x==b.x&&a.y==b.y;}
static int nav_road(int a){return cl((a+128)/256,1,7)*256;}
static NavPoint nav_player(void){return(NavPoint){(int)p.x,(int)p.y};}
static int nav_traversable(NavPoint a,NavPoint b,int radius){
 int d=nav_dist(a,b),steps=max(1,(d+7)/8);
 for(int i=1;i<=steps;i++)if(solid(a.x+(b.x-a.x)*i/(float)steps,a.y+(b.y-a.y)*i/(float)steps,(float)radius))return 0;
 return 1;
}
static int nav_anchors(NavPoint point,NavAnchor out[6]){
 int n=0,rx=nav_road(point.x),ry=nav_road(point.y);
 for(int axis=0;axis<2;axis++)for(int step=-1;step<=1;step++){
  int t=(axis?ry:rx)+step*256;if(t<256||t>1792)continue;
  NavPoint a=axis?(NavPoint){point.x,t}:(NavPoint){t,point.y};
  int d=nav_dist(point,a);if(d>270||!nav_traversable(point,a,10))continue;
  out[n++]=(NavAnchor){a,point,axis,d,0};
 }
 if(n)return n;
 // A building can hide all straight approaches from a courtyard or the
 // seawall. Pick a short, collision-tested dogleg before joining the grid.
 for(int axis=0;axis<2;axis++)for(int step=-1;step<=1;step++){
  int t=(axis?ry:rx)+step*256;if(t<256||t>1792)continue;
  int best=1<<20;NavAnchor found;
  for(int side=-1;side<=1;side+=2)for(int d=8;d<=208&&d<best;d+=8){
   NavPoint via=axis?(NavPoint){point.x+side*d,point.y}:(NavPoint){point.x,point.y+side*d};
   if(!nav_traversable(point,via,10))break;
   NavPoint road=axis?(NavPoint){via.x,t}:(NavPoint){t,via.y};
   int length=d+nav_dist(via,road);
   if(length<best&&nav_traversable(via,road,10)){best=length;found=(NavAnchor){road,via,axis,length,1};break;}
  }
  if(best<(1<<20))out[n++]=found;
 }
 return n;
}
static int nav_append(NavPoint route[8],int n,NavPoint q){if(!n||!nav_same(route[n-1],q))route[n++]=q;return n;}
static void nav_solve_base(NavPoint from,NavPoint to){
 NavAnchor a[6],b[6];int na=nav_anchors(from,a),nb=nav_anchors(to,b),best=32000;
 navCount=0;navDist=0;
 // A clear approach along the same bit of street needs no artificial detour.
 if(((from.x==to.x&&nav_abs(from.x-nav_road(from.x))<=24)||(from.y==to.y&&nav_abs(from.y-nav_road(from.y))<=24)||nav_dist(from,to)<60)&&nav_traversable(from,to,10)){
  navCount=2;navNodes[0]=from;navNodes[1]=to;navDist=nav_dist(from,to);best=navDist;
 }
 for(int i=0;i<na;i++)for(int j=0;j<nb;j++){
  NavPoint q[8],aa=a[i].a,bb=b[j].a;int n=0;
  n=nav_append(q,n,from);if(a[i].bent)n=nav_append(q,n,a[i].via);n=nav_append(q,n,aa);
  if(a[i].axis!=b[j].axis)n=nav_append(q,n,a[i].axis?(NavPoint){bb.x,aa.y}:(NavPoint){aa.x,bb.y});
  else if(a[i].axis&&aa.y!=bb.y){int cross=nav_road((aa.x+bb.x)/2);n=nav_append(q,n,(NavPoint){cross,aa.y});n=nav_append(q,n,(NavPoint){cross,bb.y});}
  else if(!a[i].axis&&aa.x!=bb.x){int cross=nav_road((aa.y+bb.y)/2);n=nav_append(q,n,(NavPoint){aa.x,cross});n=nav_append(q,n,(NavPoint){bb.x,cross});}
  n=nav_append(q,n,bb);if(b[j].bent)n=nav_append(q,n,b[j].via);n=nav_append(q,n,to);
  int d=0;for(int k=1;k<n;k++)d+=nav_dist(q[k-1],q[k]);
  // Favor center lines and fewer turns; do not route down a long sidewalk shortcut.
  int cost=d+(a[i].walk+b[j].walk)/2+n;
  if(cost<best){best=cost;navDist=d;navCount=n;for(int k=0;k<n;k++)navNodes[k]=q[k];}
 }
}
#include "outer_navigation.inc"
/* Fares on the Outer Loop use the same surveyed roads and closures as GPS.
 * Asking for a price must not erase the player's personal pin/route. */
static int taxi_dispatch_distance(int x0,int y0,int x1,int y1){
 NavPoint saved[LEN(navNodes)];cp(saved,navNodes,sizeof saved);
 int n=navCount,d=navDist,force=nav_force_road;nav_force_road=1;
 nav_solve((NavPoint){x0,y0},(NavPoint){x1,y1});nav_force_road=force;
 int route=navCount?navDist:0;cp(navNodes,saved,sizeof saved);navCount=n;navDist=d;return route;
}
/* Nav state is session-only. A selected stop from one career must not
   leak into another career when the player changes slots. */
static void nav_slot(void){
 if(navSlot==sideSlot)return;
 navSlot=sideSlot;outerAtlas=0;navReady=navHasManual=navGoalKind=navCount=navLeg=0;
 navSelected=navManualLoc=navVenue=-1;navHover=navKeyboard=navCartOpen=0;
}
/* Named counters use the address-book index. Storefront pins carry the
 * physical building index after that range. */
static void nav_track_cart(void){
 nav_slot();
 navHasManual=1;navManualLoc=NAV_LIVE_CART;navGoalKind=0;outerAtlas=1;
 navCartOpen=night_venue_vendor_report(&navManual.x,&navManual.y,&navCartClock);
}
static int nav_atm(int id){return id>=NAV_ATM_BASE&&id<NAV_ATM_BASE+LEN(bankAtms)?id-NAV_ATM_BASE:-1;}
#define NAV_OWN_CAR (NAV_ATM_BASE+LEN(bankAtms))
#define NAV_OUT_BASE (NAV_OWN_CAR+1)
#define NAV_TRAP (NAV_OUT_BASE+LEN(outerStops))
#define NAV_WOOD_BASE (NAV_TRAP+1)
#define NAV_GEAR (NAV_WOOD_BASE+WOOD_N_SITES+1)
static int nav_wood_stop(int id){return id>=NAV_WOOD_BASE&&id<NAV_WOOD_BASE+WOOD_N_SITES?id-NAV_WOOD_BASE:-1;}
static int nav_out_stop(int id){return id>=NAV_OUT_BASE&&id<NAV_OUT_BASE+LEN(outerStops)?id-NAV_OUT_BASE:-1;}
static NavPoint nav_door(int i){
 if(i==NAV_TRAP)return (NavPoint){NT_DOOR_X,NT_DOOR_Y};
 if(i==NAV_GEAR)return (NavPoint){OC_GEAR_X,OC_GEAR_Y};
 int wood=nav_wood_stop(i);if(wood>=0)return (NavPoint){woodline_sites[wood].x,woodline_sites[wood].y};
 int outer=nav_out_stop(i);if(outer>=0)return nav_outer_point(outerStops[outer].node);
 if(i==NAV_OWN_CAR)return(NavPoint){(int)cars[0].x,(int)cars[0].y};
 int k=nav_atm(i);if(k>=0)return (NavPoint){bankAtms[k].x,bankAtms[k].y+ATM_FRONT};
 if(i<LEN(loc))return (NavPoint){loc[i].x,loc[i].y};
 int shop=i-LEN(loc);return (NavPoint){ordinaryDoorX[shop],ordinaryDoorY[shop]};
}
static const char*nav_name(int i){if(i==NAV_GEAR)return "SHADE SUPPLY / GLASS MILE";int wood=nav_wood_stop(i);if(wood>=0)return woodline_names[wood];if(i==NAV_TRAP)return "TRAP HOUSE / BELL LANE";int outer=nav_out_stop(i);if(outer>=0)return outerStops[outer].label;if(i==NAV_OWN_CAR)return "YOUR REGISTERED CAR";int k=nav_atm(i);return k>=0?bankAtms[k].label:i<LEN(loc)?loc[i].label:walkin_name(i-LEN(loc));}
static NavPoint nav_job_goal(void){
 int ride=job_vehicle_target();
 if(ride>=0)return (NavPoint){(int)cars[ride].x,(int)cars[ride].y};
 if(taxi_via_copper())return (NavPoint){taxi_copper_x(),taxi_copper_y()};
 return (NavPoint){job.xs[job.step],job.ys[job.step]};
}
/* The opening route follows the career across save/reload and is superseded
 * by a manual pin or an accepted contract. The first paycheck leads to the
 * shop named in the objective instead of leaving the player without a route. */
static int nav_intro_venue(void){
 if(sv.intro==1||sv.intro==2)return 0;
 return sv.intro==3&&!sv.owned?3:-1;
}
static int nav_objective(NavPoint *to,int *venue){
 *venue=-1;
 if(navHasManual&&(!nav_cart_tracking()||navCartOpen)){*venue=nav_cart_tracking()?-1:navManualLoc;*to=*venue>=0?nav_door(*venue):navManual;return 3;}
 if(job.kind>=0&&job.step>=0&&job.step<job.total){
  *venue=job_venue();
  *to=nav_job_goal();
  return job_vehicle_target()>=0?4:2;
 }
 if(vdActive>=0){if(vdGoalX<0)return 0;*to=(NavPoint){vdGoalX,vdGoalY};return 5;}
 if(ng_route_goal(&to->x,&to->y))return 7;
 if(ncr_target(&to->x,&to->y))return 8;
 if(harbor_jobs_route(&to->x,&to->y))return 9;
 if(night_phone_route(&to->x,&to->y))return 6;
 int intro=nav_intro_venue();
 if(intro>=0){*venue=intro;*to=nav_door(intro);return 1;}
 return 0;
}
static int nav_park(NavPoint door,NavPoint *curb){
 if(outer_is_new(door.x,door.y)&&!outer_area(door.x,door.y,11)){
  int x,y,lane=outer_nearest(door.x,door.y,&x,&y,1300);
  float score=lane>=0?d2(door.x-x,door.y-y):1e20f;
  if(door.x<1090){
   for(int i=0;i<2;i++){
    OuterPoint at=woodline_path[i?WOOD_NORTH:WOOD_WEST];
    float d=d2(door.x-at.x,door.y-at.y);
    if(d<score){score=d;lane=0;x=at.x;y=at.y;}
   }
  }
  if(lane>=0){*curb=(NavPoint){x,y};return 1;}
 }
 int south=((door.y>>8)+1)*256;
 // The authored facilities face south. Park inside the lane nearest their
 // sidewalk, clear of the zebra crossings and with a tested walk to the door.
 if(south<=1792&&south-door.y>=28&&south-door.y<=140&&nav_abs(door.x-nav_road(door.x))>=56){
  NavPoint stop={door.x,south-22},road={door.x,south};
  if(nav_traversable(door,stop,10)&&nav_traversable(road,stop,10)){*curb=stop;return 1;}
 }
 return 0;
}
static int nav_sea_segment(NavPoint a,NavPoint b){
 int steps=max(1,(nav_dist(a,b)+7)/8);
 for(int i=0;i<=steps;i++)if(!harbor_boat_nav_clear(a.x+(b.x-a.x)*i/(float)steps,a.y+(b.y-a.y)*i/(float)steps))return 0;
 return 1;
}
/* A boat needs room to turn at a chart point, but a circle appreciably
 * overestimates the width needed on a straight stretch. Check the exact
 * immutable directional hull at close intervals on the new east channel. */
static int nav_sea_hull_segment(NavPoint a,NavPoint b){
 if(nav_same(a,b))return harbor_boat_nav_clear(a.x,a.y);
 float dx=b.x-a.x,dy=b.y-a.y,angle=look(dx,dy);
 int steps=(nav_dist(a,b)+3)/4;
 for(int i=0;i<=steps;i++)
  if(!harbor_boat_clear(a.x+dx*i/steps,a.y+dy*i/steps,angle))return 0;
 return 1;
}
static void nav_sea_east_channel(NavPoint a,NavPoint b){
 int reversed=a.y<b.y;
 NavPoint lower=reversed?b:a,upper=reversed?a:b;
 if(upper.y>=2010||upper.x<ROOM-145||!harbor_boat_nav_clear(lower.x,lower.y)||
    !harbor_boat_nav_clear(upper.x,upper.y))return;
 static const short elbowY[]={2210,2244,2250,2268,2316,2370,2440,2538,2636,2738};
 int best=1<<20;
 for(int x=ROOM-56;x<=ROOM-52;x++){
  NavPoint end={x,upper.y};
  if(!harbor_boat_nav_clear(end.x,end.y)||!nav_sea_hull_segment(end,upper))continue;
  for(int j=0;j<LEN(elbowY);j++){
   NavPoint entry={x,elbowY[j]},turn={lower.x,entry.y};
   if(!harbor_boat_nav_clear(entry.x,entry.y))continue;
   int length=nav_length(lower,entry)+nav_length(entry,end)+nav_length(end,upper);
   if(length>=best)continue;
   int detour=!nav_sea_hull_segment(lower,entry);
   if(detour){
    length=nav_length(lower,turn)+nav_length(turn,entry)+nav_length(entry,end)+nav_length(end,upper);
    if(length>=best||!harbor_boat_nav_clear(turn.x,turn.y)||
       !nav_sea_hull_segment(lower,turn)||!nav_sea_hull_segment(turn,entry))continue;
   }
   if(!nav_sea_hull_segment(entry,end))continue;
   NavPoint path[]={lower,detour?turn:lower,entry,end,upper};
   navCount=0;
   for(int k=0;k<LEN(path);k++){
    NavPoint q=path[reversed?LEN(path)-1-k:k];
    if(!navCount||!nav_same(navNodes[navCount-1],q))navNodes[navCount++]=q;
   }
   best=navDist=length;
  }
 }
}
static void nav_solve_sea(NavPoint a,NavPoint b){
 navCount=0;navDist=0;
 if(nav_sea_segment(a,b)){navNodes[0]=a;navNodes[1]=b;navCount=2;navDist=nav_length(a,b);return;}
 /* Try two chart doglegs below the docks; never promise a route
    across a quay, breakwater or a shoal. */
 static const int lanes[]={2268,2316,2370,2440,2538,2636,2738,2828};
 int best=1<<20;
 for(int i=0;i<LEN(lanes);i++){
  NavPoint c={a.x,lanes[i]},d={b.x,lanes[i]};
  int n=nav_dist(a,c)+nav_dist(c,d)+nav_dist(d,b);
  if(n<best&&nav_sea_segment(a,c)&&nav_sea_segment(c,d)&&nav_sea_segment(d,b)){
   best=n;navNodes[0]=a;navNodes[1]=c;navNodes[2]=d;navNodes[3]=b;navCount=4;navDist=n;
  }
 }
 if(!navCount)nav_sea_east_channel(a,b);
}
static void nav_update(void){
 NavPoint here=nav_player(),goal={0,0},curb;
 nav_slot();
 if(nav_cart_tracking())navCartOpen=night_venue_vendor_report(&navManual.x,&navManual.y,&navCartClock);
 if(navHasManual&&navManualLoc==NAV_OWN_CAR)navManual=nav_door(NAV_OWN_CAR);
 if(navHasManual&&!nav_cart_tracking()&&(navManualLoc<0||p.car<0)&&d2(p.x-navManual.x,p.y-navManual.y)<=27*27){
  navHasManual=0;if(scene==1&&!p.dead&&!mapOn)tell("GPS waypoint reached.");
 }
 int venue,kind=nav_objective(&goal,&venue);
 if(!kind){navGoalKind=navCount=navDist=navLeg=0;navVenue=-1;return;}
 int driving=p.car>=0&&(venue>=0||(kind==2&&job_room()>=0)||(outer_is_new(goal.x,goal.y)&&!outer_area(goal.x,goal.y,11)));
 int closed=np_closed();
 int mode=harbor_boat_active()?2:driving?1:0;
 if(closed!=navLastClosure||kind!=navGoalKind||venue!=navVenue||!nav_same(navGoal,goal)||navLeg!=mode||anim<navNext-.25f||
    nav_dist(here,navOrigin)>88||(anim>=navNext&&nav_dist(here,navOrigin)>=12)){
  navLastClosure=closed;navGoalKind=kind;navVenue=venue;navGoal=goal;navOrigin=here;navNext=anim+.18f;
  navLeg=mode;navCurb=goal;
  if(driving&&nav_park(goal,&curb))navCurb=curb;
  if(mode==2)nav_solve_sea(here,navCurb);
  else nav_solve(here,navCurb);
 }
}
static NavPoint nav_pin(void){
 if(navSelected>=0)return nav_door(navSelected);
 int x=nav_road(navCursor.x),y=nav_road(navCursor.y);
 if(outer_is_new(navCursor.x,navCursor.y)){
  if((harbor_boat_active()&&harbor_boat_nav_clear(navCursor.x,navCursor.y))||outer_walkable(navCursor.x,navCursor.y,10))return navCursor;
  if(outer_nearest(navCursor.x,navCursor.y,&x,&y,25)>=0)return(NavPoint){x,y};
 }
 return nav_abs(x-navCursor.x)<=nav_abs(y-navCursor.y)?(NavPoint){x,navCursor.y}:(NavPoint){navCursor.x,y};
}
static void navigation_follow_job(void){navHasManual=0;navManualLoc=-1;navGoalKind=0;}
static void nav_clear(void){navHasManual=0;navManualLoc=-1;tell("GPS waypoint cleared.");}
static void nav_set(void){
 NavPoint to=nav_pin();
 if((navSelected<0||p.car<0)&&d2(p.x-to.x,p.y-to.y)<=27*27){navHasManual=0;navManualLoc=-1;tell("You're already at that waypoint.");return;}
 navHasManual=1;navManual=to;navManualLoc=navSelected;tell("GPS waypoint set.");
}
static void navigation_input(U tap){
 nav_slot();
 if(tap&ATLAS_WORLD){outerAtlas=!outerAtlas;navHover=navKeyboard=0;navSelected=-1;return;}
 if(!navReady||(tap&MAP)){
  navReady=1;navHover=navKeyboard=0;navSelected=-1;navPointerX=navPointerY=-999;
 }
 if(tap&TAXI_NEXT){
  int next=navSelected==LEN(loc)+31?LEN(loc)+69:navSelected==LEN(loc)+69&&sv.owned?NAV_OWN_CAR:LEN(loc)+31;
  navSelected=next;navKeyboard=navHover=1;navCursor=nav_door(next);
 }
 // Space browses named venues and ATMs; left/right still browse only doors.
 if(tap&FIRE){
  int k=nav_atm(navSelected);
  int n=k>=0?LEN(loc)+k:(navSelected<LEN(loc)?navSelected:-1);
  int outer=nav_out_stop(navSelected),wood=nav_wood_stop(navSelected),base=LEN(loc)+LEN(bankAtms);
  int outdoor=nav_span()==ROOM?LEN(outerStops):0;
  int woodbase=base+outdoor,count=woodbase+(outdoor?WOOD_N_SITES:0);
  if(outer>=0)n=base+outer;
  if(wood>=0)n=woodbase+wood;
  if(navSelected==NAV_TRAP)n=count;
  n=(n+1)%(count+1);
  navKeyboard=navHover=1;
  navSelected=n<LEN(loc)?n:n<base?NAV_ATM_BASE+n-LEN(loc):n==count?NAV_TRAP:n<woodbase?NAV_OUT_BASE+n-base:NAV_WOOD_BASE+n-woodbase;
  navCursor=nav_door(navSelected);
 }
 if(tap&(LEFT|RIGHT)){
  int dir=tap&RIGHT?1:-1;
  int b=navSelected>=LEN(loc)&&navSelected<LEN(loc)+nb?navSelected-LEN(loc):dir>0?nb-1:0;
  do{b=(b+nb+dir)%nb;}while(!ordinaryDoorY[b]);
  navKeyboard=navHover=1;navSelected=LEN(loc)+b;navCursor=nav_door(navSelected);
 }
 if((tap&ENTER)&&navHover){if(navHasManual&&nav_same(navManual,nav_pin()))nav_clear();else nav_set();}
}
static void navigation_pointer(int sx,int sy,U clicks){
 nav_slot();
 if(clicks&2){if(navHasManual)nav_clear();return;}
 if(!clicks&&sx==navPointerX&&sy==navPointerY)return;
 navPointerX=sx;navPointerY=sy;navKeyboard=0;navHover=0;navSelected=-1;
 if(sx<NAV_MAP_X||sy<NAV_MAP_Y||sx>=NAV_MAP_X+NAV_MAP_SIZE||sy>=NAV_MAP_Y+NAV_MAP_SIZE)return;
 int span=nav_span();
 navCursor=(NavPoint){(sx-NAV_MAP_X)*span/NAV_MAP_SIZE,(sy-NAV_MAP_Y)*span/NAV_MAP_SIZE};
 if(navCursor.x<39||navCursor.y<39||navCursor.x>span-40||navCursor.y>span-40)return;
 navHover=1;
 int closest=11*11;
 for(int i=0;i<LEN(loc);i++){
  int dx=sx-NAV_MAP_X-loc[i].x*NAV_MAP_SIZE/span,dy=sy-NAV_MAP_Y-loc[i].y*NAV_MAP_SIZE/span,d=dx*dx+dy*dy;
  if(d<closest){navSelected=i;closest=d;}
 }
 for(int i=0;i<nb;i++)if(ordinaryDoorY[i]){
  NavPoint door=nav_door(LEN(loc)+i);
  int dx=sx-NAV_MAP_X-door.x*NAV_MAP_SIZE/span,dy=sy-NAV_MAP_Y-door.y*NAV_MAP_SIZE/span;
  int d=dx*dx+dy*dy;
  if(d<closest&&d<=7*7){navSelected=LEN(loc)+i;closest=d;}
 }
 for(int i=0;i<LEN(bankAtms);i++){
  NavPoint a=nav_door(NAV_ATM_BASE+i);
  int dx=sx-NAV_MAP_X-a.x*NAV_MAP_SIZE/span,dy=sy-NAV_MAP_Y-a.y*NAV_MAP_SIZE/span;
  int d=dx*dx+dy*dy;if(d<closest&&d<=9*9){navSelected=NAV_ATM_BASE+i;closest=d;}
 }
 if(sv.owned&&p.car!=0){
  NavPoint a=nav_door(NAV_OWN_CAR);int dx=sx-NAV_MAP_X-a.x*NAV_MAP_SIZE/span,dy=sy-NAV_MAP_Y-a.y*NAV_MAP_SIZE/span,d=dx*dx+dy*dy;
  if(d<=8*8&&d<=closest)navSelected=NAV_OWN_CAR;
 }
 if(span==ROOM)for(int i=0;i<LEN(outerStops);i++){
  NavPoint n=nav_outer_point(outerStops[i].node);
  int dx=sx-NAV_MAP_X-n.x*NAV_MAP_SIZE/span,dy=sy-NAV_MAP_Y-n.y*NAV_MAP_SIZE/span,d=dx*dx+dy*dy;
  if(d<closest&&d<=9*9){closest=d;navSelected=NAV_OUT_BASE+i;}
 }
 if(span==ROOM)for(int i=0;i<WOOD_N_SITES;i++){
  const OuterPoint q=woodline_sites[i];
  int dx=sx-NAV_MAP_X-q.x*NAV_MAP_SIZE/span,dy=sy-NAV_MAP_Y-q.y*NAV_MAP_SIZE/span,d=dx*dx+dy*dy;
  if(d<closest&&d<=8*8){closest=d;navSelected=NAV_WOOD_BASE+i;}
 }
 {int dx=sx-NAV_MAP_X-NT_DOOR_X*NAV_MAP_SIZE/span,dy=sy-NAV_MAP_Y-NT_DOOR_Y*NAV_MAP_SIZE/span;
  int d=dx*dx+dy*dy;if(d<=6*6&&d<closest){closest=d;navSelected=NAV_TRAP;}}
 {int dx=sx-NAV_MAP_X-OC_GEAR_X*NAV_MAP_SIZE/span,dy=sy-NAV_MAP_Y-OC_GEAR_Y*NAV_MAP_SIZE/span;
  int d=dx*dx+dy*dy;if(d<=6*6&&d<closest){closest=d;navSelected=NAV_GEAR;}}
 if(outer_is_new(navCursor.x,navCursor.y)&&navSelected<0){
  int x,y;
  if(!(harbor_boat_active()&&harbor_boat_nav_clear(navCursor.x,navCursor.y))&&
     !outer_walkable(navCursor.x,navCursor.y,10)&&outer_nearest(navCursor.x,navCursor.y,&x,&y,25)<0){navHover=0;return;}
 }
 if(clicks&1)nav_set();
}
static int nav_x(const NavView*v,int x){return v->x+(x-v->wx)*v->w/v->span;}
static int nav_y(const NavView*v,int y){return v->y+(y-v->wy)*v->w/v->span;}
static int nav_inside(const NavView*v,int x,int y,int pad){return x>=v->x+pad&&x<v->x+v->w-pad&&y>=v->y+pad&&y<v->y+v->h-pad;}
static void nav_rect(const NavView*v,int x,int y,int w,int h,U c){
 int a=max(v->x,x),b=max(v->y,y),u=min(v->x+v->w,x+w),d=min(v->y+v->h,y+h);
 if(a<u&&b<d)rect(a,b,u-a,d-b,c);
}
static void nav_land(const NavView*v,int x,int y,int w,int h,U c){
 int a=nav_x(v,x),b=nav_y(v,y);nav_rect(v,a,b,nav_x(v,x+w)-a,nav_y(v,y+h)-b,c);
}
static U nav_district(int x,int y){
 if(y<545)return 0x363340;
 if(y>1160)return x<900?0x263c36:0x31404a;
 return x<850?0x413736:x<1400?0x343a48:0x363b3e;
}
static void nav_seg(const NavView*,int,int,int,int,U);
static void nav_terrain(const NavView*v,int large){
 rect(v->x,v->y,v->w,v->h,NAV_SEA);
 for(int j=0;j<v->h;j+=large?11:9){int a=v->y+j;nav_rect(v,v->x+(j%3)*5,a,v->w,1,0x182d40);}
 nav_land(v,32,32,1984,1984,0x2e555d);
 nav_land(v,39,39,1970,1970,NAV_INK);
 for(int iy=0;iy<8;iy++)for(int ix=0;ix<8;ix++){
  int tx=ix*256,ty=iy*256,x=max(39,tx+27),y=max(39,ty+27),xe=min(2009,tx+230),ye=min(2009,ty+230);
  nav_land(v,x,y,xe-x,ye-y,nav_district(tx+128,ty+128));
  if(!lot[iy*8+ix][0]){
   nav_land(v,tx+48,ty+48,160,160,0x263e3d);
   nav_land(v,tx+48,ty+125,160,6,0x446359);
   nav_land(v,tx+125,ty+48,6,160,0x446359);
  }
 }
 for(int i=0;i<nb;i++){const Building*b=buildings+i;if(b->w<=0||b->h<=0)continue;
  nav_land(v,b->x-4,b->y-3,b->w+11,b->h+9,0x212a34);
  StreetBounds shell=building_bounds(b);
  nav_land(v,shell.x,shell.y,shell.w,shell.h,0x536172);
  if(large)nav_land(v,shell.x+4,shell.y+4,b->w-8,b->h/3,0x647384);
 }
 for(int i=1;i<=7;i++){
  const int a=i*256;
  nav_land(v,a-27,39,54,1970,NAV_CURB);nav_land(v,39,a-27,1970,54,NAV_CURB);
 }
 for(int i=1;i<=7;i++){
  const int a=i*256;
  nav_land(v,a-16,39,32,1970,NAV_GRID);nav_land(v,39,a-16,1970,32,NAV_GRID);
  if(large)for(int z=102;z<1990;z+=33){
   if(z%256>38&&z%256<213){nav_land(v,a-2,z,4,12,0x97a3aa);nav_land(v,z,a-2,12,4,0x97a3aa);}
  }
 }
 if(v->span==ROOM||v->wx+v->span>BASE_ROOM-36||v->wy+v->span>BASE_ROOM-36){
  /* The physical coastline, forest and tarn share the collision palette.
   * Row runs keep the map crisp without repainting millions of pixels. */
  int sample=large?2:3;
  for(int sy0=0;sy0<v->h;sy0+=sample){
   int wy=v->wy+(sy0+sample/2)*v->span/v->w;
   int start=0;U last=0;
   for(int sx0=0;sx0<=v->w;sx0+=sample){
    int wx=v->wx+(sx0+sample/2)*v->span/v->w;
    U ink=sx0>=v->w||!outer_is_new(wx,wy)?0:woodline_map_ink(outer_tile(wx,wy));
    if(ink==NAV_SEA)ink=0;
    if(ink!=last){if(last)nav_rect(v,v->x+start,v->y+sy0,sx0-start,sample,last);start=sx0;last=ink;}
   }
   if(last)nav_rect(v,v->x+start,v->y+sy0,v->w-start,sample,last);
  }
  for(int i=0;i<LEN(woodline_edges);i++){
   OuterPoint a=woodline_path[woodline_edges[i][0]],b=woodline_path[woodline_edges[i][1]];
   nav_seg(v,nav_x(v,a.x),nav_y(v,a.y),nav_x(v,b.x),nav_y(v,b.y),0xa09168);
  }
  for(int i=0;i<LEN(outerRoads);i++){
   const OuterRoad*r=outerRoads+i;NavPoint a=nav_outer_point(r->a),b=nav_outer_point(r->b);
   U c=r->kind==OUT_HWY?0x9a866c:r->kind==OUT_DOCK?0x628c93:r->kind==OUT_HILL?0x84758a:0x7b8190;
   int x=nav_x(v,a.x),y=nav_y(v,a.y),ux=nav_x(v,b.x),uy=nav_y(v,b.y);
   int width=large?(r->half>=50?3:2):1;
   for(int o=-width;o<=width;o++)nav_seg(v,x+o,y,ux+o,uy,c);
   for(int o=-width;o<=width;o++)nav_seg(v,x,y+o,ux,uy+o,c);
  }
  for(int i=0;i<LEN(outerStops);i++){
   NavPoint n=nav_outer_point(outerStops[i].node);int x=nav_x(v,n.x),y=nav_y(v,n.y);
   if(x>=v->x+7&&x<=v->x+v->w-7&&y>=v->y+7&&y<=v->y+v->h-7){diamond(x,y,large?6:3,outerStops[i].tint);dot(x,y,PAPER);}
  }
  for(int i=0;i<WOOD_N_SITES;i++){
   int x=nav_x(v,woodline_sites[i].x),y=nav_y(v,woodline_sites[i].y);
   if(nav_inside(v,x,y,7)){diamond(x,y,large?4:2,0xb4d096);dot(x,y,0xf2e0a8);}
  }
  if(large&&v->span==ROOM){
   int x=nav_x(v,402),y=nav_y(v,2838);
   center(x,y,"GREYREED WOOD",0xc8d9a9,0);
   x=nav_x(v,2770);y=nav_y(v,2425);center(x,y,"LANTERN BAY",0x8dbfbf,0);
   x=nav_x(v,2587);y=nav_y(v,52);center(x,y,"NORTHPOINT",0xd4bc94,0);
   for(int i=0;i<HARBOR_N_DOCK_POINTS;i+=2){
    int px=nav_x(v,harborDockPoints[i].x),py=nav_y(v,harborDockPoints[i].y);
    if(nav_inside(v,px,py,3)){nav_rect(v,px-2,py-1,5,3,0xa9d5c6);dot(px,py+2,0xa9d5c6);}
   }
  }
 }
}
static int nav_clip(const NavView*v,int x,int y){
 return (x<v->x?1:x>=v->x+v->w?2:0)|(y<v->y?4:y>=v->y+v->h?8:0);
}
static void nav_seg(const NavView*v,int x,int y,int u,int w,U c){
 while(1){
  int a=nav_clip(v,x,y),b=nav_clip(v,u,w);
  if(!(a|b)){line(x,y,u,w,c);return;}if(a&b)return;
  if(!a){int t=x;x=u;u=t;t=y;y=w;w=t;a=b;}
  int xx,yy;
  if(a&12){yy=a&4?v->y:v->y+v->h-1;xx=x+(u-x)*(yy-y)/(w-y);}
  else{xx=a&1?v->x:v->x+v->w-1;yy=y+(w-y)*(xx-x)/(u-x);}
  x=xx;y=yy;
 }
}
static void nav_stroke(const NavView*v,NavPoint p0,NavPoint p1,int w,U c){
 int a=nav_x(v,p0.x),b=nav_y(v,p0.y),u=nav_x(v,p1.x),h=nav_y(v,p1.y);
 for(int i=-w;i<=w;i++)for(int j=-w;j<=w;j++)if(i*i+j*j<=w*w+1)nav_seg(v,a+i,b+j,u+i,h+j,c);
}
static int nav_progress(NavPoint here,NavPoint *join){
 int first=1;*join=navNodes[1];
 // The cached path is updated at most a few times a second. Trim its first
 // consumed section with a fast segment projection every draw.
 int closest=1<<30;
 for(int i=1;i<min(navCount,4);i++){
  NavPoint a=navNodes[i-1],b=navNodes[i];int dx=b.x-a.x,dy=b.y-a.y,dd=dx*dx+dy*dy;
  int t=dd?cl((int)((((long long)(here.x-a.x)*dx+(long long)(here.y-a.y)*dy)*1024)/dd),0,1024):0;
  NavPoint q={a.x+(dx*t)/1024,a.y+(dy*t)/1024};
  int x=here.x-q.x,y=here.y-q.y,dist=x*x+y*y;
  if(dist<closest&&(dist<12*12||nav_traversable(here,q,7))){closest=dist;*join=q;first=i;}
 }
 if(closest==(1<<30)){*join=navNodes[0];return 0;}
 return first;
}
/* Read the solved route in travel order. The upcoming bend is based on
 * the road, not the car's current heading (which can drift in a slide).
 * Ignore the tiny lane-to-centre jogs and the final walk from the curb. */
typedef struct{NavPoint join;int first,total,turn,until;} NavReadout;
static NavReadout nav_readout(NavPoint here){
 NavReadout r={0};
 if(navCount<2)return r;
 r.first=nav_progress(here,&r.join);
 r.total=nav_length(here,r.join);
 for(int i=r.first;i<navCount;i++){
  NavPoint a=i==r.first?r.join:navNodes[i-1],b=navNodes[i];
  r.total+=nav_length(a,b);
  if(!r.turn&&i>0&&i+1<navCount&&r.total>=8){
   NavPoint start=navNodes[i-1],end=navNodes[i+1];
   int ax=b.x-start.x,ay=b.y-start.y,bx=end.x-b.x,by=end.y-b.y;
   if(nav_length(start,b)>=28&&nav_length(b,end)>=28){
    int cross=ax*by-ay*bx,dot=ax*bx+ay*by;
    /* Indicate real turns on inclined roads, not gentle lane curves. */
    if(cross&&(dot<=0||nav_abs(cross)>dot*.70f)){r.turn=cross>0?1:-1;r.until=r.total;}
   }
  }
 }
 return r;
}
static void nav_bend_badge(const NavView*v,const NavReadout*r){
 if(!r->turn)return;
 char s[24];append(number(s,r->until)," m");
 int w=textw(s,0)+27,x=v->x+v->w-w-3,y=v->y+4,mid=x+12;
 box(x,y,w,21,NAV_INK,NAV_RIM);
 /* Screen y increases toward the south: positive cross means right. */
 rect(mid-1,y+7,3,10,NAV_VIOLET);
 rect(r->turn>0?mid:mid-5,y+6,6,3,NAV_VIOLET);
 int tip=mid+r->turn*8,root=mid+r->turn*3;
 tri(tip,y+7,root,y+2,root,y+12,NAV_VIOLET);
 textat(x+23,y,s,PAPER,0);
}
static void nav_route(const NavView*v,int large,const NavReadout*r){
 if(navLeg==1&&!nav_same(navCurb,navGoal)){
  int x=nav_x(v,navCurb.x),y=nav_y(v,navCurb.y),ex=nav_x(v,navGoal.x),ey=nav_y(v,navGoal.y);
  int steps=max(2,(nav_abs(ex-x)+nav_abs(ey-y))/(large?5:4));
  for(int i=1;i<steps;i++){
   int px=x+(ex-x)*i/steps,py=y+(ey-y)*i/steps;
   if(nav_inside(v,px,py,2))disc(px,py,large?2:1,NAV_WALK);
  }
 }
 if(navCount<2)return;
 NavPoint here=nav_player(),join=r->join;int first=r->first;
 for(int pass=0;pass<2;pass++){
  int w=pass?(large?2:1):(large?4:3);U c=pass?NAV_VIOLET:NAV_DARK;
  if(first){nav_stroke(v,here,join,w,c);nav_stroke(v,join,navNodes[first],w,c);}
  for(int i=first+1;i<navCount;i++)nav_stroke(v,navNodes[i-1],navNodes[i],w,c);
 }
}
static void nav_icon(int x,int y,int i,int large){
 const U c=loc[i].tint;
 if(!large){disc(x,y,3,NAV_INK);disc(x,y,2,c);return;}
 disc(x,y,9,NAV_INK);disc(x,y,7,c);disc(x,y,5,NAV_INK);
 switch(loc[i].kind){
  case 0:case 10:rect(x-3,y-3,7,5,c);dot(x-2,y+2,c);dot(x+2,y+2,c);break;
  case 1:case 7:diamond(x,y,4,c);dot(x,y,c);break;
  case 2:case 11:rect(x-3,y-3,7,7,c);rect(x-2,y-1,5,1,NAV_INK);rect(x-2,y+1,5,1,NAV_INK);break;
  case 3:rect(x-4,y-1,9,4,c);rect(x-2,y-3,5,3,c);break;
  case 4:case 5:line(x-3,y+3,x+3,y-3,c);tri(x-3,y-2,x+4,y-4,x+1,y+3,c);break;
  case 6:line(x-3,y+4,x-3,y-4,c);rect(x-2,y-4,6,4,c);dot(x-1,y-3,NAV_INK);dot(x+1,y-2,NAV_INK);break;
  case 8:rect(x-1,y-4,3,9,c);rect(x-4,y-1,9,3,c);break;
  case 14:
   disc(x,y,4,c);disc(x,y,2,NAV_INK);line(x-4,y,x+4,y,c);line(x,y-4,x,y+4,c);dot(x,y,NAV_INK);break;
  case 15:
   rect(x-3,y-4,7,5,c);tri(x-3,y,x+3,y,x,y+4,c);line(x-2,y-2,x+2,y-2,NAV_INK);line(x,y-3,x,y+1,NAV_INK);break;
  default:tri(x-4,y,x,y-4,x+4,y,c);rect(x-3,y,7,4,c);dot(x,y+2,NAV_INK);break;
 }
}
static void nav_heading(int x,int y,int large){
 int n=large?10:8,w=large?6:4;
 float co=cs(p.a),si=sn(p.a);
 int ax=x+(int)(co*n),ay=y+(int)(si*n),bx=x-(int)(co*w+si*w),by=y-(int)(si*w-co*w),dx=x-(int)(co*w-si*w),dy=y-(int)(si*w+co*w);
 disc(x,y,large?10:8,0x202836);
 tri(ax,ay,bx,by,dx,dy,PAPER);
}
static void nav_label(const NavView*v,int x,int y,const char *s){
 int px=nav_x(v,x),py=nav_y(v,y),w=textw(s,0);
 if(px-w/2>v->x&&px+w/2<v->x+v->w)center(px,py,s,0xadb5bb,0);
}
static void nav_atm_icon(int x,int y,int large){
 if(!large){box(x-3,y-3,7,7,NAV_INK,ATM_MINT);dot(x,y,ATM_MINT);return;}
 box(x-7,y-7,15,15,NAV_INK,ATM_MINT);
 center(x,y-10,"$",ATM_MINT,0);
}
static void nav_club_icon(int x,int y,int large){
 U glow=0xff9ed0;int r=large?8:4;
 disc(x,y,r+1,0x251d30);diamond(x,y,r,glow);
 line(x-2,y,x+2,y,glow);line(x,y-2,x,y+2,glow);dot(x,y,PAPER);
}
/* Dashed and deliberately broad; no live unit coordinates are disclosed.
 * Only on the open atlas, as the minimap remains for driving and jobs. */
static void nav_search_ring(const NavView*v){
 int x,y,ink=dispatch_public_area(&x,&y);
 if(!ink)return;
 int px=nav_x(v,x),py=nav_y(v,y),r=max(6,112*v->w/v->span);
 U c=mix(NAV_INK,0x91b9ff,ink);
 for(int j=0;j<12;j++){
  float a=j*.523599f,b=a+.33f;
  nav_seg(v,px+(int)(cs(a)*r),py+(int)(sn(a)*r),px+(int)(cs(b)*r),py+(int)(sn(b)*r),c);
 }
}
static void nav_overlay(const NavView*v,int large,const NavReadout*r){
 nav_terrain(v,large);
 if(large){
  nav_label(v,460,102,"GLASS MILE");nav_label(v,380,950,"TIDE MARKET");nav_label(v,1140,944,"NIGHTGLASS");
  nav_label(v,1680,638,"EAST FREIGHT");nav_label(v,565,1710,"LANTERN WARD");nav_label(v,1436,1748,"BREAKWATER");
 }
 if(large)for(int i=0;i<nb;i++)if(ordinaryDoorY[i]){
  int x=nav_x(v,ordinaryDoorX[i]),y=nav_y(v,ordinaryDoorY[i]);
  if(tram_hall(i)){box(x-5,y-5,11,11,NAV_INK,GOLD);line(x-3,y-2,x+3,y-2,GOLD);line(x,y-2,x,y+3,GOLD);}
  else rect(x-1,y-1,3,3,0xb3b9ad);
 }
 if(large)nav_search_ring(v);
 nav_route(v,large,r);
 for(int i=0;i<LEN(loc);i++){int x=nav_x(v,loc[i].x),y=nav_y(v,loc[i].y);if(nav_inside(v,x,y,large?10:4))nav_icon(x,y,i,large);}
 for(int i=0;i<LEN(bankAtms);i++){
  NavPoint a=nav_door(NAV_ATM_BASE+i);int x=nav_x(v,a.x),y=nav_y(v,a.y);
  if(nav_inside(v,x,y,large?8:4))nav_atm_icon(x,y,large);
 }
 if(ncFrontId>=0){
  int x=nav_x(v,ordinaryDoorX[ncFrontId]),y=nav_y(v,ordinaryDoorY[ncFrontId]);
  if(nav_inside(v,x,y,large?10:5)){
   nav_club_icon(x,y,large);
   if(large&&x+105<v->x+v->w&&y+42<v->y+v->h)
    textat(x+11,y+21,"STRIP CLUB",0xffadc9,0);
  }
 }
 if(sv.owned&&p.car!=0){
  int x=nav_x(v,(int)cars[0].x),y=nav_y(v,(int)cars[0].y);
  if(nav_inside(v,x,y,large?9:5)){box(x-6,y-3,13,7,0x16232c,0xa3f78f);rect(x-3,y-6,7,3,0xa3f78f);dot(x-3,y+4,PAPER);dot(x+3,y+4,PAPER);}
 }
 if(large){
  int x0=nav_x(v,60),x1=nav_x(v,1988),y=nav_y(v,1999);
  if(nav_inside(v,x0,y,0)&&nav_inside(v,x1,y,0)){
   line(x0,y,x1,y,0x427c82);
   textat(x0+8,y-15,"SEAWALL ROAD",0x9dc6c6,0);
  }
 }
 {int x=nav_x(v,NT_DOOR_X),y=nav_y(v,NT_DOOR_Y);if(nav_inside(v,x,y,large?8:5)){
   U c=night_trap_owned()?0x8ae2bd:0xffbf7d;
   box(x-3,y-2,7,6,NAV_INK,c);line(x-4,y-2,x,y-6,c);line(x,y-6,x+4,y-2,c);dot(x,y+2,c);
 }}
 if(large){
  int x=nav_x(v,OC_GEAR_X),y=nav_y(v,OC_GEAR_Y);
  if(nav_inside(v,x,y,7)){diamond(x,y,6,0xa4e1c2);box(x-2,y-2,5,5,0x19353e,0xa4e1c2);}
 }
 NavPoint objective={0,0};
 int realGoal=0;U color=LIME;
 if(job.kind>=0&&job.step>=0&&job.step<job.total){
  realGoal=1;objective=nav_job_goal();color=job_vehicle_target()>=0?GOLD:job.kind<4?TEAL:PINK;
 }else if(vdActive>=0){realGoal=vdGoalX>=0;objective=(NavPoint){vdGoalX,vdGoalY};color=TEAL;
 }else if(ng_route_goal(&objective.x,&objective.y)){realGoal=1;color=GOLD;
 }else if(ncr_target(&objective.x,&objective.y)){realGoal=1;color=0xf18acb;
 }else if(harbor_jobs_route(&objective.x,&objective.y)){realGoal=1;color=0x83dff2;
 }else if(night_phone_route(&objective.x,&objective.y)){realGoal=1;color=0xff9adb;
 }else{int intro=nav_intro_venue();if(intro>=0){realGoal=1;objective=nav_door(intro);}}
 if(realGoal){int x=nav_x(v,objective.x),y=nav_y(v,objective.y);
  if(nav_inside(v,x,y,large?12:8)){diamond(x,y,large?11:7,color);dot(x,y,color);}
 }
 if(navHasManual&&(!nav_cart_tracking()||navCartOpen||large)){
  int x=nav_x(v,navManual.x),y=nav_y(v,navManual.y);
  if(nav_inside(v,x,y,large?10:7)){
   if(nav_cart_tracking()&&!navCartOpen)diamond(x,y,8,GOLD); /* last scheduled check-in */
   else{U c=nav_cart_tracking()?0xff9ac6:NAV_VIOLET;diamond(x,y,large?10:6,c);disc(x,y,large?3:2,c);}
  }
 }
 if(navLeg==1&&!nav_same(navCurb,navGoal)){int x=nav_x(v,navCurb.x),y=nav_y(v,navCurb.y);
  if(nav_inside(v,x,y,large?8:5)){disc(x,y,large?7:4,NAV_DARK);diamond(x,y,large?6:3,NAV_WALK);if(large)dot(x,y,NAV_WALK);}
 }
 const int xp=nav_x(v,(int)p.x),yp=nav_y(v,(int)p.y);
 nav_heading(xp,yp,large);
 if(large&&navHover){NavPoint q=nav_pin();int x=nav_x(v,q.x),y=nav_y(v,q.y);
  diamond(x,y,14,PAPER);line(x-17,y,x-11,y,PAPER);line(x+11,y,x+17,y,PAPER);line(x,y-17,x,y-11,PAPER);line(x,y+11,x,y+17,PAPER);
 }
}
static const char*nav_venue_note(int i,char *status){
 int k=loc[i].kind,li=legal_index_for_kind(k);
 if(li>=0&&sv.legit[li].owned){
  LegalBusiness*b=sv.legit+li;
  if(sv.audit||sv.charges)return "Revenue hold. Visit the annex to check your case.";
  if(b->pending){
   char*q=number(append(status,"Rear desk: $"),b->pending);
   if(b->cooldown>0)legal_duration(append(q," in "),b->cooldown);
   else append(q," ready to collect.");
  }else legal_duration(append(status,"Next franchise income in "),LEGAL_INTERVAL-b->elapsed);
  return status;
 }
 if((k==4||k==5)&&sv.b[k-4].own){
  Business*b=sv.b+k-4;
  char*q=number(append(status,"Stock: "),warehouse_units(k-4));
  if(b->timer>0)legal_duration(append(q,". Batch ready in "),b->timer);
  else if(!warehouse_has_room(k-4))append(q,". Warehouse full.");
  else append(q,sv.units<batch_supply(k-4)?". Buy supplies at Ferry Market.":". Terminal ready.");
  return status;
 }
 return loc[i].note;
}
static const char*nav_job_name(void){
 if(job.kind==JOB_TAXI){
#if __has_include("verdict_taxi.inc")
  return taxi_route_name();
#else
  return taxi_stop_label(taxi.phase==TAXI_WAIT||taxi.phase==TAXI_BOARDING?taxi.pickup:taxi.dropoff);
#endif
 }
 int v=job_venue(),b=job_room();
 if(v>=0)return loc[v].label;if(b>=0)return walkin_name(b);
 if(job.kind==JOB_RECOVERY)return job.step?"Wrecker Yard bay":"Stalled work van";
 if(job.kind==JOB_HOT)return job.step?"Kiosk 09 handoff":"Marked Vectra";
 if(job.kind==JOB_RIVALS)return "East Freight cache";
 if(job.kind==JOB_SPRINT)return job.step?"Glass Mile race gate":"Glass Mile start";
 return "Job checkpoint";
}
static void nav_wrap(int x,int y,int width,const char*s,U c,int maxrows){
 char line[130];for(int row=0;*s&&row<maxrows;row++){s=hud_line(s,width,line);textat(x,y+row*20,line,c,0);}
}
/* Keep footer text on one line. The compass and map key have separate
 * space from this label and from the upcoming-turn badge on the right. */
static const char *nav_compact_footer(char *s,NavPoint here,int dist,int atCurb,int width){
 if(!navGoalKind)return district(here.x,here.y);
 const char*source=navGoalKind==3?(nav_cart_tracking()?"CART":"PIN"):navGoalKind==4?"RIDE":
  navGoalKind==1?"CAREER":navGoalKind==5?"LISTING":navGoalKind==6?"DELIVERY":
  navGoalKind==7?"AFTERHOURS":navGoalKind==8?"SHOW RIDE":navGoalKind==9?"HARBOR":"JOB";
 char info[24];if(atCurb)append(info,"/ WALK");else if(navCount>=2)append(number(info,dist)," m");
 else append(info,nav_dist(here,navGoal)<30?"HERE":navLeg==2?"NO LINE":"NO ROAD");
 if(textw(source,0)+textw(" ",0)+textw(info,0)>width){
  if(navGoalKind==7)source="NIGHT";else if(navGoalKind==8)source="SHOW";
  else if(navGoalKind==6)source="DROP";else if(navGoalKind==5)source="GIG";else if(navGoalKind==9)source="SEA";
 }
 append(append(append(s,source)," "),info);return s;
}
static void minimap(void){
 if(scene!=2)nav_update();
 char s[72];NavPoint here=nav_player();NavReadout r=nav_readout(here);
 int d=r.total,atCurb=navLeg==1&&!nav_same(navCurb,navGoal)&&nav_dist(here,navCurb)<18;
 if(!mapOn){
  const int w=170,h=120,x=W-w-24,y=H-h-40,span=742;
  if(p.car>=0){
   const Car *ride=&cars[p.car];const CarSpec *spec=&specs[ride->type];
   box(x-4,y-39,w+8,33,NAV_INK,NAV_RIM);
   textat(x+4,y-37,spec->name,PAPER,0);
   const char *gear=ride->s< -2?"REV":ride->s>2?"DRIVE":"STOP";
   textat(x+w-textw(gear,0)-4,y-37,gear,ride->s< -2?GOLD:MUTED,0);
   rect(x+4,y-14,w-8,4,NAV_CURB);
   rect(x+4,y-14,cl((int)(absf(ride->s)*(w-8)/spec->vmax),0,w-8),4,TEAL);
  }
  box(x-4,y-4,w+8,h+30,NAV_INK,NAV_RIM);
  const NavView v={x,y,w,h,(int)p.x-span/2,(int)p.y-span*h/(2*w),span};
  nav_overlay(&v,0,&r);if(p.car>=0&&!atCurb)nav_bend_badge(&v,&r);box(x+5,y+4,22,21,NAV_INK,NAV_RIM);center(x+16,y+4,"N",PAPER,0);
  box(x+30,y+4,36,21,NAV_INK,NAV_RIM);textat(x+33,y+4,"[M]",PAPER,0);
  const char*footer=nav_compact_footer(s,here,d,atCurb,w-8);
  textat(x+4,y+h+1,footer,navGoalKind?NAV_VIOLET:MUTED,0);return;
 }
 rect(0,0,W,H,0x0d1420);box(10,10,W-20,H-20,0x111b29,NAV_RIM);
 const NavView v={NAV_MAP_X,NAV_MAP_Y,NAV_MAP_SIZE,NAV_MAP_SIZE,0,0,nav_span()};
 box(v.x-6,v.y-6,v.w+12,v.h+12,NAV_INK,NAV_RIM);nav_overlay(&v,1,&r);
 draw_day_clock(16);
 textat(29,42,"PORT",PAPER,1);textat(29,72,"VANTA",PAPER,1);textat(31,105,v.span==ROOM?"OUTER LOOP ATLAS":"STREET ATLAS  /  09",NAV_VIOLET,0);
 textat(31,129,"WAYPOINT",PAPER,0);rect(30,154,194,1,NAV_RIM);
 textat(31,158,"CLICK / ENTER   SET PIN",MUTED,0);textat(31,180,"RIGHT CLICK   CLEAR",MUTED,0);
 textat(31,202,"< / >   SHOP DOORS",MUTED,0);textat(31,224,"SPACE   VENUES / ATMS",MUTED,0);
 textat(31,246,"T   TRAM STOPS / CAR",GOLD,0);textat(31,268,"M / ESC   CLOSE",MUTED,0);
 textat(31,301,"MAP KEY",PAPER,0);rect(30,325,194,1,NAV_RIM);
 line(37,341,51,341,NAV_VIOLET);textat(61,331,"SELECTED ROUTE",NAV_VIOLET,0);
 diamond(44,362,6,TEAL);textat(61,352,"JOB / LISTING",TEAL,0);
 box(38,380,12,7,DARK,0xa3f78f);rect(41,377,6,3,0xa3f78f);textat(61,374,"OWNED CAR",0xa3f78f,0);
 diamond(44,406,6,GOLD);textat(61,396,"JOB VEHICLE",GOLD,0);
 box(39,423,11,11,NAV_INK,GOLD);center(44,418,"T",GOLD,0);textat(61,418,"TRAM HALL",GOLD,0);
 nav_atm_icon(44,451,1);textat(61,440,"BANK ATM",ATM_MINT,0);
 int lx,ly;
 if(dispatch_public_area(&lx,&ly)){diamond(44,471,5,0x91b9ff);textat(61,462,"LAST SEEN AREA",0x91b9ff,0);}
 textat(31,493,"F5   SAVE CAREER",MUTED,0);
 const int rx=719,rw0=220;
 textat(rx,53,navKeyboard?"SELECTED STOP":"MAP POINTER",NAV_VIOLET,0);rect(rx,79,rw0,1,NAV_RIM);
 if(navSelected>=0){
  int wood=nav_wood_stop(navSelected);
  char status[128];int atm=nav_atm(navSelected),shop=navSelected-LEN(loc);
  NavPoint a=nav_door(navSelected);nav_wrap(rx,83,rw0,nav_name(navSelected),PAPER,2);
  textat(rx,123,shop==ncFrontId&&ncFrontId>=0?"BASE VILLAGE":district(a.x,a.y),GOLD,0);
  if(shop>=0&&shop<nb&&tram_hall(shop)){
   int to=tram_destination(shop);char*q=append(status,"To ");append(q,walkin_name(to));nav_wrap(rx,145,rw0,status,PAPER,2);
   nav_wrap(rx,186,rw0,to==69?"Lantern Ward / West seawall":"East Freight / Wrecker Yard",MUTED,2);
   nav_wrap(rx,227,rw0,"Fare: $5 clean cash on hand",GOLD,1);
   nav_wrap(rx,249,rw0,tram_block()?tram_block():"Open: board at timetable.",tram_block()?PINK:TEAL,2);
  }else{
   int outer=nav_out_stop(navSelected);
   const char*note=navSelected==NAV_GEAR?"Gear merchant at the labeled west porch. Enter on foot. The original Saltmarsh Cinema uses E.":
    wood>=0?woodline_details[wood]:navSelected==NAV_TRAP?(night_trap_owned()?"Owned. Manage construction, graded storage and neighborhood repair bookings.":"For sale: $480 clean. Press Enter at the signed Bell Lane side door."):
     outer>=0?outerStops[outer].detail:navSelected==NAV_OWN_CAR?"Current registered vehicle. Enter pins its live location.":
     atm>=0?"Clean cash: deposit or withdraw at this kiosk.":navSelected<LEN(loc)?nav_venue_note(navSelected,status):
     shop==ncFrontId?"Tide Flats: straight west of Apartment 4. Direct E entry; no prerequisites.":"Storefront entrance. Enter pins this door.";
   nav_wrap(rx,149,rw0,note,MUTED,5);
  }
 }else{
  textat(rx,89,navHover?"STREET WAYPOINT":"CITY ATLAS",PAPER,0);
  nav_wrap(rx,119,rw0,navHover?"Click to pin this place. Walk from the road into open terrain.":"Point at a door. Press T to inspect both tram stops and your registered car.",MUTED,6);
 }
 textat(rx,297,"PURPLE LINE",NAV_VIOLET,0);rect(rx,321,rw0,1,NAV_RIM);
 const char *owner=navGoalKind==3?(nav_cart_tracking()?"Live Night Cart":"Personal pin"):navGoalKind==4?"Job vehicle":navGoalKind==2?"Active job":navGoalKind==5?"City listing":navGoalKind==6?"Phone delivery":navGoalKind==7?"Afterhours event":navGoalKind==8?"Afterimage show ride":navGoalKind==9?"Harbor objective":navGoalKind==1?"Career objective":"None";
 char guide[160];char*q=append(guide,owner);
 if(navGoalKind){q=append(q," / ");append(q,atCurb?"park and walk":navLeg==2?(navCount>=2?"water chart":"no clear water line"):navCount>=2?"road guidance":"no road route");}
 nav_wrap(rx,325,rw0,guide,PAPER,2);
 if(navGoalKind){
  if(navCount>=2){q=number(guide,d);append(q,navLeg==1&&!nav_same(navCurb,navGoal)?" m to curb":" m remaining");}
  else append(guide,nav_dist(here,navGoal)<30?"At destination":"Follow the marker");
  textat(rx,368,guide,NAV_VIOLET,0);
 }
 if(navGoalKind==8)nav_wrap(rx,394,rw0,ncr_stop_name(),0xf18acb,3);
 else if(nav_cart_tracking()){
  char when[6];city_time_label(navCartClock,when);
  q=append(guide,navCartOpen?"Checked in ":"Closed / last ");
  q=append(q,when);q=append(q," / ");append(q,district(navManual.x,navManual.y));
  nav_wrap(rx,394,rw0,guide,navCartOpen?0xff9ac6:GOLD,3);
 }else if(job.kind>=0||vdActive>=0||navVenue>=0){
  q=append(guide,job.kind>=0?"Job stop: ":vdActive>=0?"Listing: ":"Destination: ");
  append(q,job.kind>=0?nav_job_name():vdActive>=0?(vdGoalName[0]?vdGoalName:vdNames[vdActive]):nav_name(navVenue));
  nav_wrap(rx,394,rw0,guide,TEAL,3);
 }
 if(sv.charges){revenue_time(append(s,"REVENUE SWEEP "));textat(rx,465,s,PINK,0);}
 else if(saveFlash)textat(rx,465,saveFlash>0?"SAVED":"SAVE ERROR",saveFlash>0?TEAL:PINK,0);
 textat(rx,487,"F8 / CITY + OUTER LOOP",TEAL,0);
 textat(267,505,"CITY / REVENUE LIVE     |     JOB CLOCK PAUSED",GOLD,0);
}
#undef NAV_INK
#undef NAV_SEA
#undef NAV_GRID
#undef NAV_CURB
#undef NAV_RIM
#undef NAV_VIOLET
#undef NAV_DARK
#undef NAV_WALK
