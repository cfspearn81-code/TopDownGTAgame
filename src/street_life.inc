/* Lane traffic and pedestrians are transient world state, not save data.
 * Included by game.c after move/open_space. Car 0 is the active owned ride.
 * The final slot holds the current job vehicle. */
#if __has_include("verdict_street.inc")
static int vd_cross_hold(const Car*,float,float);
static int vd_cross_player_stop(const Car*,float,float);
static int vd_cross_foot(float,float,float,float,float);
static void vd_cross_paint(void);
#else
#define vd_cross_hold(c,x,y) 0
#define vd_cross_player_stop(c,x,y) 0
#define vd_cross_foot(x,y,nx,ny,r) 0
#define vd_cross_paint() ((void)0)
#endif
#define STREET_HALF_L 15.5f
#define STREET_HALF_W 8.f
#define STREET_BOARD_SPEED 58.f
#define STREET_EXIT_SPEED 78.f
#define STREET_WALK 38.f
#define STREET_CIVILIANS 40
enum{DISPATCH_FIRST=44,DISPATCH_UNITS=8};

typedef struct {int l,t,r,b;} StreetLoop;
static const StreetLoop streetLoops[]={
 {1,1,4,3},{1,2,3,5},{3,1,6,3},{4,2,7,4},
 {2,4,4,6},{4,4,7,6},{1,5,3,7},{3,5,6,7},{5,5,7,7}
};
typedef struct {int live,loop,next;float pace;} StreetDriver;
enum{STREET_LEAVE,STREET_EN_ROUTE,STREET_ARRIVE};
typedef struct {float x,y,cx,cy;} StreetAddress;
typedef struct {int goal,visits,cross,leg;float tx,ty,mx,my,ax,ay,pause,crossX,crossY,blocked,fromX,fromY,stalled,crossWait;int detour,axis;} StreetWalk;
static StreetDriver streetDrivers[LEN(cars)];
static StreetWalk streetWalks[DISPATCH_FIRST+DISPATCH_UNITS];
static int streetJunctionOwner[64];

static int street_overlap(const Car*a,const Car*b){
 float x=b->x-a->x,y=b->y-a->y;
 if(d2(x,y)>=36.f*36.f)return 0;
 float ac=cs(a->a),as=sn(a->a),bc=cs(b->a),bs=sn(b->a);
 float parallel=absf(ac*bc+as*bs),perp=absf(ac*bs-as*bc);
 float l=STREET_HALF_L*(1+parallel)+STREET_HALF_W*perp;
 float w=STREET_HALF_W*(1+parallel)+STREET_HALF_L*perp;
 return absf(x*ac+y*as)<l&&absf(y*ac-x*as)<w&&absf(x*bc+y*bs)<l&&absf(y*bc-x*bs)<w;
}
/* Exact line against the same building footprints used by solid().
 * Subsampling can miss the tip of a wall, especially across diagonal corners. */
static int street_clip_axis(float pos,float delta,float low,float high,float*t0,float*t1){
 if(absf(delta)<.00001f)return pos>low&&pos<high;
 float a=(low-pos)/delta,b=(high-pos)/delta;
 if(a>b){float c=a;a=b;b=c;}
 if(a>*t0)*t0=a;if(b<*t1)*t1=b;
 return *t0<*t1;
}
static int street_line_open(float x,float y,float tx,float ty,float radius){
 if(x<35||y<35||tx<35||ty<35||x>ROOM-36||y>ROOM-36||tx>ROOM-36||ty>ROOM-36)return 0;
 float dx=tx-x,dy=ty-y;
 if(outer_is_new(x+radius,y+radius)||outer_is_new(tx+radius,ty+radius)){
  int steps=1+(int)((absf(dx)+absf(dy))/4.f);
  for(int i=0;i<=steps;i++)if(!outer_walkable(x+dx*i/steps,y+dy*i/steps,radius))return 0;
 }
 for(int yy=cl((int)((dy<0?ty:y)-radius)/256,0,7);yy<=cl((int)((dy>0?ty:y)+radius)/256,0,7);yy++)
  for(int xx=cl((int)((dx<0?tx:x)-radius)/256,0,7);xx<=cl((int)((dx>0?tx:x)+radius)/256,0,7);xx++){
   const BlockScenery*block=scenery+yy*8+xx;
   for(int i=0;i<block->count;i++){
    const StreetBounds*b=block->walls+i;float t0=0,t1=1;
    if(street_clip_axis(x,dx,b->x-radius,b->x+b->w+radius,&t0,&t1)&&
       street_clip_axis(y,dy,b->y-radius,b->y+b->h+radius,&t0,&t1))return 0;
   }
  }
 return 1;
}
/* The same rotated chassis that blocks movement also blocks reach and
 * impacts. Ignore only the car being boarded/exited or the target's own
 * car; a different parked car or wreck is still solid. */
static float street_car_fraction(float x,float y,float tx,float ty,float r,int ignore){
 float dx=tx-x,dy=ty-y,first=1.f;
 float xl=x<tx?x:tx,xh=x>tx?x:tx,yl=y<ty?y:ty,yh=y>ty?y:ty;
 for(int i=street_next_car(-1);i<LEN(cars);i=street_next_car(i))if(i!=ignore){
  const Car*c=cars+i;float pad=18.f+r*2;
  if(c->x<xl-pad||c->x>xh+pad||c->y<yl-pad||c->y>yh+pad)continue;
  float co=cs(c->a),si=sn(c->a),ox=x-c->x,oy=y-c->y,t0=0,t1=first;
  if(street_clip_axis(ox*co+oy*si,dx*co+dy*si,-STREET_HALF_L-r,STREET_HALF_L+r,&t0,&t1)&&
     street_clip_axis(oy*co-ox*si,dy*co-dx*si,-STREET_HALF_W-r,STREET_HALF_W+r,&t0,&t1))first=t0;
 }
 return first;
}
static int street_reach_open(float x,float y,float tx,float ty,float r,int ignore){
 return street_line_open(x,y,tx,ty,r)&&street_car_fraction(x,y,tx,ty,r,ignore)>=1.f;
}
/* A missed shot ends on the first solid pixel, never the far side of
 * cover. Only firing a shot pays for the handful of wall probes. */
static float street_tracer_fraction(float x,float y,float dx,float dy){
 float limit=street_car_fraction(x,y,x+dx,y+dy,1,-1);
 if(street_line_open(x,y,x+dx*limit,y+dy*limit,1))return limit;
 float lo=0,hi=limit;
 for(int i=0;i<9;i++){
  float mid=(lo+hi)*.5f;
  if(street_line_open(x,y,x+dx*mid,y+dy*mid,1))lo=mid;else hi=mid;
 }
 return lo;
}
static int street_on_camera(float x,float y){
 int left=camera_left(),top=camera_top();
 return x>=left-25&&x<=left+W+25&&y>=top-31&&y<=top+H+31;
}
/* Test the actual rotated body against building boxes. Sparse chassis probes
 * missed building corners, and an embedded old car could drive deeper in. */
static float street_wall_depth(const Car*c){
 float co=cs(c->a),si=sn(c->a),ac=absf(co),as=absf(si);
 float rx=STREET_HALF_L*ac+STREET_HALF_W*as,ry=STREET_HALF_L*as+STREET_HALF_W*ac;
 float l=c->x-rx,h=c->x+rx,t=c->y-ry,b=c->y+ry,depth=0;
 if(l<35)depth=35-l;if(t<35&&35-t>depth)depth=35-t;
 if(h>ROOM-36&&h-(ROOM-36)>depth)depth=h-(ROOM-36);
 if(b>ROOM-36&&b-(ROOM-36)>depth)depth=b-(ROOM-36);
 if(outer_is_new(h,b)){
  /* Historic inner-edge limits are now wide gates in the surveyed road.
     Test the rotated hull perimeter instead of an axis-aligned proxy. */
  for(int ui=-2;ui<=2;ui++)for(int vi=-2;vi<=2;vi++){
   if(ui&&ui!=-2&&ui!=2&&vi&&vi!=-2&&vi!=2)continue;
   float u=ui*STREET_HALF_L/2.f,v=vi*STREET_HALF_W/2.f;
   float px=c->x+u*co-v*si,py=c->y+u*si+v*co;
   if(!outer_onroad(px,py))depth=depth<1.f?1.f:depth;
  }
 }
 for(int yy=cl((int)t/256,0,7);yy<=cl((int)b/256,0,7);yy++)
  for(int xx=cl((int)l/256,0,7);xx<=cl((int)h/256,0,7);xx++){
   const BlockScenery*block=scenery+yy*8+xx;
   for(int i=0;i<block->count;i++){
    const StreetBounds*v=block->walls+i;float wx=v->w*.5f,wy=v->h*.5f;
    float dx=v->x+wx-c->x,dy=v->y+wy-c->y;
    float a=rx+wx-absf(dx),b=ry+wy-absf(dy);
    if(a<=0||b<=0)continue;
    float u=STREET_HALF_L+wx*ac+wy*as-absf(dx*co+dy*si);
    float w=STREET_HALF_W+wx*as+wy*ac-absf(dy*co-dx*si);
    if(u<=0||w<=0)continue;
    if(b<a)a=b;if(u<a)a=u;if(w<a)a=w;
    if(a>depth)depth=a;
   }
  }
 float rail=vd_chess_rail_depth(c);return rail>depth?rail:depth;
}
static int street_hits_wall(const Car*c){return street_wall_depth(c)>0.f;}
static int street_car_clear(int id,const Car*old,const Car*next){
 float penetration=street_wall_depth(next);
 if(penetration>0.f&&(!old||penetration+.001f>=street_wall_depth(old)))return 0;
 for(int j=street_next_car(-1);j<LEN(cars);j=street_next_car(j)){
  if(j==id)continue;
  const Car*b=cars+j;
  if(street_overlap(next,b)&&(!old||!street_overlap(old,b)||
      d2(next->x-b->x,next->y-b->y)<=d2(old->x-b->x,old->y-b->y)))return 0;
 }
 return 1;
}
static int street_move_car(int id,float dx,float dy){
 Car*c=cars+id,next=*c;next.x+=dx;next.y+=dy;
 if(!street_car_clear(id,c,&next))return 0;
 *c=next;return 1;
}
/* Fast cars can move more than a small obstacle's width per tick.
 * Check their travel in short pieces without changing the fixed clock. */
static void street_drive_car(int id,float dx,float dy){
 int steps=d2(dx,dy)>3.95f*3.95f?2:1;
 dx/=steps;dy/=steps;
 for(int i=0;i<steps;i++)if(!street_move_car(id,dx,dy)){
  /* At a shallow angle tire grip can carry the car along the wall. */
  int grazed=absf(dx)>2.5f*absf(dy)?street_move_car(id,dx,0):
             absf(dy)>2.5f*absf(dx)?street_move_car(id,0,dy):0;
  if(!grazed)break;
  cars[id].s*=.995f;
 }
}
static int street_turn_car(int id,float angle){
 Car*c=cars+id,next=*c;next.a=angle;
 float penetration=street_wall_depth(&next);
 if(penetration>0.f&&penetration+.001f>=street_wall_depth(c))return 0;
 for(int j=street_next_car(-1);j<LEN(cars);j=street_next_car(j))if(j!=id&&
      street_overlap(&next,cars+j)&&!street_overlap(c,cars+j))return 0;
 c->a=angle;return 1;
}
static float street_foot_car_depth(const Car*c,float x,float y,float feet){
 float dx=x-c->x,dy=y-c->y,r=STREET_HALF_L+feet+STREET_HALF_W+feet;
 if(absf(dx)>r||absf(dy)>r)return 0;
 float co=cs(c->a),si=sn(c->a);
 float along=STREET_HALF_L+feet-absf(dx*co+dy*si);
 float across=STREET_HALF_W+feet-absf(dy*co-dx*si);
 return along>0&&across>0?(along<across?along:across):0;
}
static int street_foot_car_clear(float x,float y,int ignore,float feet){
 for(int i=street_next_car(-1);i<LEN(cars);i=street_next_car(i))if(i!=ignore){
  if(street_foot_car_depth(cars+i,x,y,feet)>0)return 0;
 }
 return 1;
}
/* Knockback and rival movement share this physical test. An actor displaced
 * by a moving car can step back out but cannot move deeper into it. */
static int street_step_past_cars(float x,float y,float nx,float ny,float feet){
 if(vd_cross_foot(x,y,nx,ny,feet))return 0;
 for(int i=street_next_car(-1);i<LEN(cars);i=street_next_car(i)){
  const Car*c=cars+i;float after=street_foot_car_depth(c,nx,ny,feet);
  if(after<=0)continue;
  float before=street_foot_car_depth(c,x,y,feet);
  if(after>before+.001f||(absf(after-before)<=.001f&&d2(nx-c->x,ny-c->y)<=d2(x-c->x,y-c->y)))return 0;
 }
 return 1;
}
static void street_actor_move(Ped*q,float dx,float dy,float feet){
 if(!solid(q->x+dx,q->y,feet)&&street_step_past_cars(q->x,q->y,q->x+dx,q->y,feet))q->x+=dx;
 if(!solid(q->x,q->y+dy,feet)&&street_step_past_cars(q->x,q->y,q->x,q->y+dy,feet))q->y+=dy;
}
/* Foot movement collides with every body, including wrecks, but people
 * loaded beside a parked car or displaced by a rear impact can step out. */
static int street_foot_move_clear(float x,float y){
 if(solid(x,y,10))return 0;
 for(int i=STREET_CIVILIANS;i<LEN(peds);i++){
  const Ped *guard=peds+i;
  if(guard->role!=2||guard->hp<=0)continue;
  float after=d2(x-guard->x,y-guard->y);
  if(after<11*11&&after<d2(p.x-guard->x,p.y-guard->y))return 0;
 }
 return street_step_past_cars(p.x,p.y,x,y,7.f);
}
static void street_player_step(float dx,float dy){
 if(dx&&!night_phone_lane_stop(p.x,p.y,p.x+dx,p.y,8)&&street_foot_move_clear(p.x+dx,p.y))p.x+=dx;
 if(dy&&!night_phone_lane_stop(p.x,p.y,p.x,p.y+dy,8)&&street_foot_move_clear(p.x,p.y+dy))p.y+=dy;
}
static int street_foot_clear(float x,float y,int ignore){
 return open_space(x,y)&&street_foot_car_clear(x,y,ignore,7.f);
}
/* Transactional: find room before taking a payment or issuing a job.
 * The separate apply marks a previously moving slot as no longer ambient. */
static int street_find_car_space(int id,Car*car,float x,float y,int departure,int min_radius){
 static const unsigned char radius[]={23,27,32,37,45,58,74,94,116};
 for(int r=0;r<LEN(radius);r++)if(radius[r]>=min_radius)for(int i=0;i<16;i++){
  float heading=i*.3926991f,dx=cs(heading)*radius[r],dy=sn(heading)*radius[r];
  for(int j=0;j<2;j++){
   Car next=*car;next.x=x+dx;next.y=y+dy;
   next.a=wrap(car->a+j*1.570796f);next.s=0.f;
   if(!street_car_clear(id,0,&next))continue;
   if(departure){
    Car pullout=next;
    pullout.x+=cs(next.a)*departure;pullout.y+=sn(next.a)*departure;
    if(!street_car_clear(id,0,&pullout))continue;
   }
   *car=next;return 1;
  }
 }
 return 0;
}
static void street_stage_car(int id,Car car){
 cars[id]=car;streetDrivers[id].live=0;
}
static int street_near_car(void){
 if(p.car>=0)return -1;
 float best=38.f*38.f;
 int choice=-1;int boardable=0;
 for(int i=street_next_car(-1);i<LEN(cars);i=street_next_car(i)){
  const Car*c=cars+i;
  float dx=c->x-p.x,dy=c->y-p.y,d=d2(dx,dy);
  if(c->cond<=0||(i==0&&!sv.owned)||d>=38.f*38.f)continue;
  int slow=absf(c->s)<=STREET_BOARD_SPEED;
  if(slow<boardable||(slow==boardable&&d>=best))continue;
  if(!night_phone_lane_stop(p.x,p.y,c->x,c->y,2)&&street_reach_open(p.x,p.y,c->x,c->y,2,i)){best=d;choice=i;boardable=slow;}
 }
 return choice;
}
/* +1 entered, 0 unrelated/unavailable, -1 moving too fast to board. */
static int street_enter_car(int i){
 if(p.car>=0||!street_car_exists(i)||i!=street_near_car())return 0;
 Car*c=cars+i;
 if(absf(c->s)>STREET_BOARD_SPEED)return -1;
 streetDrivers[i].live=0;outer_take_wheel(i);
 p.car=i;p.x=c->x;p.y=c->y;p.a=c->a;
 return 1;
}
/* A stopped car's door may itself be clear while closing traffic would
 * reach the landing point before the player can step away. */
static int street_safe_landing(float x,float y){
  for(int j=street_next_car(-1);j<LEN(cars);j=street_next_car(j))if(j!=p.car&&absf(cars[j].s)>14.f){
   Car*d=cars+j;
   if(d2(d->x-x,d->y-y)>90.f*90.f)continue;
   float a=cs(d->a),b=sn(d->a),ahead=((x-d->x)*a+(y-d->y)*b)*(d->s>0?1:-1),side=absf((y-d->y)*a-(x-d->x)*b);
   if(ahead>=-8&&ahead<20+absf(d->s)*.60f&&side<14)return 0;
  }
 return 1;
}
static int street_exit_car(void){
 if(!street_car_exists(p.car))return 0;
 Car*c=cars+p.car;
 if(absf(c->s)>STREET_EXIT_SPEED)return 0;
 float co=cs(c->a),si=sn(c->a);
 /* Prefer a curbside door; check the other door, then behind the car.
  * Never silently eject into a wall, another vehicle or fast traffic. */
 static const float candidates[][2]={{0,21},{0,-21},{-25,0},{-19,19},{-19,-19},{25,0},{19,19},{19,-19}};
 for(int i=0;i<LEN(candidates);i++){
  float u=candidates[i][0],v=candidates[i][1],x=c->x+u*co-v*si,y=c->y+u*si+v*co;
  if(night_phone_lane_stop(c->x,c->y,x,y,7)||!street_foot_clear(x,y,p.car)||!street_reach_open(c->x,c->y,x,y,3,p.car))continue;
  if(street_safe_landing(x,y)){p.x=x;p.y=y;p.car=-1;c->s=0;return 1;}
 }
 /* Only on a wreck: a normal door can be boxed in on all four sides.
  * Look for the closest clear landing with walkable onward space. Pass
  * over the pileup only, never through walls/terrain or a closed gate.
  * Ordinary exits and garage handoffs still require an actual free door. */
 if(c->cond<=0)for(int r=26;r<=240;r+=6)for(int j=0;j<24;j++){
  float a=c->a+(j+6)*.26179939f,dx=cs(a),dy=sn(a),x=c->x+r*dx,y=c->y+r*dy;
  float tx=x+dx*10,ty=y+dy*10;
  if(!street_foot_clear(x,y,-1)||!street_foot_clear(tx,ty,-1)||
     night_phone_lane_stop(c->x,c->y,tx,ty,7)||!street_line_open(c->x,c->y,x,y,7)||
     !street_reach_open(x,y,tx,ty,7,-1)||!street_safe_landing(x,y))continue;
  p.x=x;p.y=y;p.car=-1;c->s=0;return 1;
 }
 return 0;
}
static void street_loop_point(int which,int n,float*x,float*y){
 const StreetLoop *l=streetLoops+which;
 float left=l->l*256+10.f,right=l->r*256-10.f,top=l->t*256+10.f,bottom=l->b*256-10.f;
 static const signed char o[][2]={{-17,0},{-5,5},{0,17},{0,-17},{-5,-5},{-17,0},
    {17,0},{5,-5},{0,-17},{0,17},{5,5},{17,0}};
 n%=12;
 *x=((n<6)?right:left)+o[n][0];*y=((n<3||n>=9)?top:bottom)+o[n][1];
}
static int street_is_walker(int i){
 return i<STREET_CIVILIANS?peds[i].role==0:i>=DISPATCH_FIRST;
}
static int street_yield(const Car*c,float fx,float fy){
 /* No ambient vehicle flattens a pedestrian. Brake early for anyone
  * ahead in this lane and wait for a crossing to clear. */
 float limit=22.f+c->s*.54f;
 for(int i=0;i<LEN(streetWalks);i++)if(peds[i].hp>0&&street_is_walker(i)&&streetWalks[i].cross){
  float dx=peds[i].x-c->x,dy=peds[i].y-c->y;
  float along=dx*fx+dy*fy,side=absf(dy*fx-dx*fy);
  if(along>=-13&&along<limit&&side<13.5f){
   /* If the chassis is already on the stripe and the pedestrian is
    * holding just outside it, roll clear instead of mutual yield. The
    * per-step body check below still never drives through the person. */
   float co=cs(c->a),si=sn(c->a),bodySide=absf(dy*co-dx*si);
   if(absf(dx*co+dy*si)<STREET_HALF_L&&bodySide>=STREET_HALF_W+3.f)continue;
   return 1;
  }
 }
 if(p.car<0&&!p.dead){
  float dx=p.x-c->x,dy=p.y-c->y;
  if(dx*fx+dy*fy>=-12&&dx*fx+dy*fy<limit&&absf(dy*fx-dx*fy)<14.f)return 1;
 }
 return 0;
}
static int street_ambient_step(int id,float dx,float dy){
 Car*c=cars+id,next=*c;next.x+=dx;next.y+=dy;
 float co=cs(next.a),si=sn(next.a);
 for(int j=0;j<=LEN(streetWalks);j++){
  float ux,uy;
  if(j==LEN(streetWalks)){if(p.car>=0||p.dead)continue;ux=p.x-next.x;uy=p.y-next.y;}
  else{if(peds[j].hp<=0||!street_is_walker(j))continue;ux=peds[j].x-next.x;uy=peds[j].y-next.y;}
  if(d2(ux,uy)<24.f*24.f&&absf(ux*co+uy*si)<STREET_HALF_L+3.f&&absf(uy*co-ux*si)<STREET_HALF_W+3.f){
   if(d2(ux,uy)<=d2(ux+dx,uy+dy))return 0;
  }
 }
 return street_move_car(id,dx,dy);
}
static int street_junction(float x,float y){
 int bx=(int)(x+128.f)/256,by=(int)(y+128.f)/256;
 return (bx>0&&bx<8&&by>0&&by<8&&absf(x-bx*256.f)<92.f&&absf(y-by*256.f)<92.f)?8*by+bx:-1;
}
static void street_tick_traffic(float dt){
 /* Two perpendicular drivers can reach a turn nose-to-nose and each
  * stop forever. Reserve enough space for stopping distance and the
  * full diagonally turning rear chassis, not just the crosswalk itself. */
 for(int j=0;j<LEN(streetJunctionOwner);j++){
  int id=streetJunctionOwner[j];
  if(id<0||id>=nc||!streetDrivers[id].live||p.car==id||cars[id].owner||cars[id].cond<=0||street_junction(cars[id].x,cars[id].y)!=j)
   streetJunctionOwner[j]=-1;
 }
 for(int i=0;i<nc;i++)if(streetDrivers[i].live&&p.car!=i&&!cars[i].owner&&cars[i].cond>0){
  int j=street_junction(cars[i].x,cars[i].y);
  if(j>=0&&streetJunctionOwner[j]<0)streetJunctionOwner[j]=i;
 }
 for(int i=0;i<nc;i++){
  StreetDriver*d=streetDrivers+i;
  if(!d->live||p.car==i||cars[i].cond<=0||cars[i].owner)continue;
  Car*c=cars+i;float tx,ty;
  street_loop_point(d->loop,d->next,&tx,&ty);
  float dx=tx-c->x,dy=ty-c->y,dist=sq(d2(dx,dy));
  if(dist<.02f){d->next=(d->next+1)%12;continue;}
  float fx=dx/dist,fy=dy/dist,desired=look(dx,dy),turn=wrap(desired-c->a);
  float cap=3.8f*dt;
  if(turn>cap)turn=cap;
  if(turn<-cap)turn=-cap;
  street_turn_car(i,wrap(c->a+turn));
  float goal=d->pace;
  if(d->next%3!=0||dist<38)goal=32.f;
  int j=street_junction(c->x,c->y);
  if((j>=0&&streetJunctionOwner[j]!=i)||street_yield(c,fx,fy)||vd_cross_hold(c,fx,fy))goal=0;
  float ahead=6+c->s*.26f;if(ahead>dist)ahead=dist;
  Car probe=*c;probe.x+=fx*ahead;probe.y+=fy*ahead;
  if(!street_car_clear(i,c,&probe))goal=0;
  float accel=(goal<c->s?135.f:92.f)*dt;
  c->s=c->s<goal?(c->s+accel<goal?c->s+accel:goal):(c->s-accel>goal?c->s-accel:goal);
  float amount=c->s*dt;if(amount>dist)amount=dist;
  if(amount<=0)continue;
  if(!street_ambient_step(i,fx*amount,fy*amount))c->s=0;
  else if(dist-amount<.02f)d->next=(d->next+1)%12;
 }
}

static float street_nearest_line(float at){return ((int)(at+128.f)/256)*256.f;}
static float street_curb(float at,int tie){
 float line=street_nearest_line(at);
 return line+((at>line||(at==line&&tie))?STREET_WALK:-STREET_WALK);
}
static float street_side_coordinate(float at,int tie){
 float line=street_nearest_line(at),d=at-line;if(absf(d)>=STREET_WALK)return at;
 return street_curb(at,tie);
}
static StreetAddress street_venue_address(int site,int tie){
 const Place*l=loc+site;
 float cx=street_curb(l->x,tie&1),cy=street_curb(l->y,tie&2);
 /* Zoned storefronts face south and open onto the next east/west
  * sidewalk. Use their own frontage, not the other side of the block. */
 float inset=l->x%256;
 if(inset>=STREET_WALK&&inset<=256.f-STREET_WALK){
  float south=((l->y>>8)+1)*256.f-STREET_WALK;
  if(street_line_open(cx,south,l->x,south,4)&&street_line_open(l->x,south,l->x,l->y,4))
   return (StreetAddress){l->x,south,cx,south};
 }
 StreetAddress a={street_side_coordinate(l->x,tie&1),cy,cx,cy};
 StreetAddress b={cx,street_side_coordinate(l->y,tie&2),cx,cy};
 /* Named storefronts needn't be at intersections. Approach the nearest
  * reachable point on their curb, horizontally or vertically, via a corner. */
 float da=d2(a.x-l->x,a.y-l->y),db=d2(b.x-l->x,b.y-l->y);
 int passA=street_line_open(a.cx,a.cy,a.x,a.y,4),passB=street_line_open(b.cx,b.cy,b.x,b.y,4);
 if(da<=db&&passA)return a;
 if(passB)return b;
 if(passA)return a;
 return (StreetAddress){cx,cy,cx,cy};
}
static void street_civ_goal(int i){
 StreetWalk*w=streetWalks+i;
 StreetAddress a=street_venue_address(w->goal,i+w->visits);
 w->ax=a.x;w->ay=a.y;w->tx=a.cx;w->ty=a.cy;
}
static int street_civ_spawn_room(int i,int slots,float x,float y){
 if(!street_foot_clear(x,y,-1))return 0;
 for(int j=0;j<slots;j++)if(i!=j&&peds[j].hp>0&&d2(peds[j].x-x,peds[j].y-y)<17.f*17.f)return 0;
 return 1;
}
static int street_civ_find_spawn(int i,int slots,int site,int offscreen,StreetAddress*out){
 StreetAddress a=street_venue_address(site,i);
 if((!offscreen||!street_on_camera(a.x,a.y))&&street_civ_spawn_room(i,slots,a.x,a.y)){*out=a;return 1;}
 /* Keep the named storefront catchments but place later walkers down
  * the same sidewalk instead of stacking on identical shop coordinates. */
 for(int n=1;n<=7;n++)for(int try=0;try<4;try++){
  int axis=(try/2+!!(i&2))%2,sign=(try+i)%2?1:-1;
  float x=a.x+(axis?0:17.f*n*sign),y=a.y+(axis?17.f*n*sign:0);
  float cross=axis?x:y;
  if(absf(cross-street_nearest_line(cross))<STREET_WALK-1.f)continue;
  /* Never spawn in the middle of a crosswalk or route through a facade. */
  float dist=axis?y:x;if(absf(dist-street_nearest_line(dist))<STREET_WALK-2.f)continue;
  if((offscreen&&street_on_camera(x,y))||!street_civ_spawn_room(i,slots,x,y))continue;
  float cx=axis?x:a.cx,cy=axis?a.cy:y;
  if(!street_line_open(x,y,cx,cy,4)||absf(x-cx)+absf(y-cy)>166.f)continue;
  *out=(StreetAddress){x,y,cx,cy};return 1;
 }
 return 0;
}
static void street_spawn_civilian_at(int i,int address,StreetAddress a){
 peds[i]=(Ped){a.x,a.y,0,0,2,0,0,0,0};
 StreetWalk*w=streetWalks+i;
 *w=(StreetWalk){.goal=(address+1+i%5)%LEN(loc),.leg=STREET_LEAVE,.mx=a.cx,.my=a.cy,.fromX=a.x,.fromY=a.y,
  .pause=(i%6)*.38f,.axis=(i/LEN(loc))&1};
 street_civ_goal(i);
}
static void street_spawn_civilian(int i){
 int address=i%LEN(loc);
 StreetAddress a;
 for(int k=0;k<LEN(loc);k++){
  int s=(address+k)%LEN(loc);
  if(street_civ_find_spawn(i,i,s,0,&a)){street_spawn_civilian_at(i,s,a);return;}
 }
 /* No camera-dependent drops: a bounded curb-grid scan finds a legal
  * alternative even if every authored venue was temporarily saturated. */
 for(int y=1;y<8;y++)for(int x=1;x<8;x++)for(int k=0;k<4;k++){
  float cx=x*256.f+(k&1?38:-38),cy=y*256.f+(k&2?38:-38);
  if(street_civ_spawn_room(i,i,cx,cy)){street_spawn_civilian_at(i,address,(StreetAddress){cx,cy,cx,cy});return;}
 }
}
static int street_respawn_civilian(int i){
 for(int n=0;n<LEN(loc);n++){
  int address=(i+n)%LEN(loc);StreetAddress a;
  if(street_civ_find_spawn(i,STREET_CIVILIANS,address,1,&a)){
   street_spawn_civilian_at(i,address,a);return 1;
  }
 }
 return 0;
}
static int street_civ_traffic(const Ped*q,float tx,float ty){
 /* A stopped car is a physical obstruction too. Don't step into one on
  * a marked crossing just because it has zero closing speed. */
 for(int n=1;n<=10;n++)
  if(!street_foot_car_clear(q->x+(tx-q->x)*n*.1f,q->y+(ty-q->y)*n*.1f,-1,3.f))return 1;
 for(int i=street_next_car(-1);i<LEN(cars);i=street_next_car(i)){
  const Car*c=cars+i;
  if((i==0&&!sv.owned)||c->cond<=0||absf(c->s)<8.f||d2(c->x-q->x,c->y-q->y)>116.f*116.f)continue;
  float co=cs(c->a)*(c->s>0?1:-1),si=sn(c->a)*(c->s>0?1:-1),spd=absf(c->s);
  int acrossX=absf(tx-q->x)>absf(ty-q->y);
  if(acrossX&&absf(si)>.55f){
   float t=(q->y-c->y)/si;
   float x=c->x+co*t;
   if(t>=-18&&t<20+spd*1.18f&&x>min((int)q->x,(int)tx)-13&&x<max((int)q->x,(int)tx)+13)return 1;
  }else if(!acrossX&&absf(co)>.55f){
   float t=(q->x-c->x)/co;
   float y=c->y+si*t;
   if(t>=-18&&t<20+spd*1.18f&&y>min((int)q->y,(int)ty)-13&&y<max((int)q->y,(int)ty)+13)return 1;
  }
 }
 return 0;
}
static float street_corner_step(float value,float target,int*cross){
 if(absf(value-target)<.4f)return target;
 float center=street_nearest_line(value),dir=target>value?1.f:-1.f;
 if(absf(value-(center-dir*STREET_WALK))<.5f){
  float out=center+dir*STREET_WALK;
  if((target-out)*dir>=0){*cross=1;return out;}
 }else{
  float out=center+dir*(256-STREET_WALK);
  if((target-out)*dir>=0)return out;
 }
 return target;
}
static int street_can_start_leg(Ped*q,StreetWalk*w){
 if(!w->cross)return 1;
 if(street_civ_traffic(q,w->mx,w->my)){
  w->mx=q->x;w->my=q->y;w->cross=0;w->pause=.18f;return 0;
 }
 w->crossX=q->x;w->crossY=q->y;return 1;
}
static void street_walk_step(Ped*q,StreetWalk*w,float dt,float speed){
 speed*=FOOT_PACE;
 float dx=w->mx-q->x,dy=w->my-q->y,n=sq(d2(dx,dy));
 if(n<.001f)return;
 q->a=look(dx,dy);float step=speed*dt;if(step>n)step=n;
 float x=q->x+dx*(step/n),y=q->y+dy*(step/n);
 /* A car can pivot across a retreating walker's safety margin.
  * Let the walker back out without admitting any deeper overlap. */
 if(!solid(x,y,4)&&!vd_cross_foot(q->x,q->y,x,y,3.f)&&(w->cross==2?street_step_past_cars(q->x,q->y,x,y,3.f):
                                  street_foot_car_clear(x,y,-1,3.f))){
  q->x=x;q->y=y;q->s=speed;w->blocked=0;
 }else if(w->cross==1&&(w->blocked+=dt)>.40f){
  /* A car can turn toward a stripe after a civilian has started crossing.
   * Don't leave both users yielding forever: return to the same curb, then
   * wait for a second opening. Never step off the zebra into traffic. */
  w->mx=w->crossX;w->my=w->crossY;w->cross=2;w->blocked=0;w->pause=.15f;
 }
}
static int street_civ_gap(const Ped*q,float x,float y){
 for(int j=0;j<LEN(peds);j++){
  const Ped*r=peds+j;if(r==q||r->hp<=0)continue;
  if(absf(r->x-x)>13.f||absf(r->y-y)>13.f||npc_replaces(N_ACT_STREET+j))continue;
  float after=d2(r->x-x,r->y-y),before=d2(r->x-q->x,r->y-q->y);
  if(after<12.f*12.f&&after<before-.0005f)return 0;
 }
 return 1;
}
static int street_civ_cede_zebra(Ped*q,StreetWalk*w,float dt){
 if(w->cross)return 0;
 for(int j=0;j<LEN(streetWalks);j++){
  const Ped*r=peds+j;const StreetWalk*z=streetWalks+j;
  if(r==q||r->hp<=0||!z->cross||!street_is_walker(j))continue;
  if(d2(z->mx-q->x,z->my-q->y)>12.4f*12.4f||d2(r->x-q->x,r->y-q->y)>31.f*31.f)continue;
  /* Give an existing crossing pedestrian space to exit. Normal traffic
   * queue ordering resumes at the exact curb point after they pass. */
  float dx=z->mx-z->crossX,dy=z->my-z->crossY;
  if(z->cross==2){dx=z->mx-r->x;dy=z->my-r->y;}
  float sideX=absf(dy)>absf(dx)?1.f:0,sideY=1.f-sideX,step=dt*27.f;
  for(int n=1;n>=-1;n-=2){
   float x=q->x+n*sideX*step,y=q->y+n*sideY*step;
   float curb=sideX?x:y;
   if(absf(curb-street_nearest_line(curb))<31.f||solid(x,y,4)||
     !street_foot_car_clear(x,y,-1,3.f)||!street_civ_gap(q,x,y)||
     d2(z->mx-x,z->my-y)<=d2(z->mx-q->x,z->my-q->y))continue;
   q->x=x;q->y=y;q->a=look(n*sideX,n*sideY);q->s=27.f;return 1;
  }
 }
 return 0;
}
static float street_civ_corner(float value,float target,int*cross){
 float center=street_nearest_line(value),dir=target>value?1.f:-1.f;
 float a=center-dir*STREET_WALK,b=center+dir*STREET_WALK;
 /* A sidestep may finish a few meters shy of the exact authored curb.
  * Walk there first rather than stepping a diagonal straight into traffic. */
 if((target-b)*dir>=0&&(a-value)*dir>.45f)return a;
 return street_corner_step(value,target,cross);
}
static int street_civ_cross_queue(const Ped*q,const StreetWalk*w){
 float dx=w->mx-q->x,dy=w->my-q->y;
 for(int j=0;j<LEN(streetWalks);j++){
  const Ped*r=peds+j;if(r==q||r->hp<=0||!streetWalks[j].cross||!street_is_walker(j))continue;
  float x=r->x-q->x,y=r->y-q->y;
  if(absf(dx)>absf(dy)){
   if(absf(y)<11.f&&x*dx>0&&absf(x)<absf(dx)+4.f)return 1;
  }else if(absf(x)<11.f&&y*dy>0&&absf(y)<absf(dy)+4.f)return 1;
 }
 return 0;
}
static int street_civ_yield(Ped*q,StreetWalk*w,float dt,float speed){
 float dx=w->mx-q->x,dy=w->my-q->y,n=sq(d2(dx,dy));if(n<.001f)return 0;
 float stride=speed*FOOT_PACE*dt;if(stride>n)stride=n;
 float u=dx/n,v=dy/n,nx=q->x+u*stride,ny=q->y+v*stride;
 if(street_civ_gap(q,nx,ny))return 0;
 if(!w->cross&&w->leg==STREET_EN_ROUTE){
  /* A crowded sidewalk can be wider on the storefront or on the curb
   * side. Try a polite step to the right, then the left, never the lane. */
  for(int side=1;side>=-1;side-=2){
   float x=q->x+(u*.12f-v*side)*stride,y=q->y+(v*.12f+u*side)*stride;
   float cross=absf(dx)>absf(dy)?y:x;
   if(absf(cross-street_nearest_line(cross))<31.f||solid(x,y,4)||!street_civ_gap(q,x,y)||
    !street_foot_car_clear(x,y,-1,3.f)||vd_cross_foot(q->x,q->y,x,y,3.f))continue;
   q->x=x;q->y=y;q->a=look(u,v);q->s=speed*FOOT_PACE*.82f;return 1;
  }
 }
 return 1; /* Let the lead walker occupy their space; don't overlap. */
}
static void street_civ_replan(int i){
 Ped*q=peds+i;StreetWalk*w=streetWalks+i;
 w->detour=1;w->mx=w->fromX;w->my=w->fromY;w->stalled=0;
 w->pause=.25f+(i%5)*.08f;
 /* If a blocker pinned the pedestrian at their previous marker, the
  * ordinary target selector can choose a new route immediately. */
 if(d2(q->x-w->mx,q->y-w->my)<.18f)w->pause=.47f;
}
static void street_tick_civilian(int i,float dt){
 Ped*q=peds+i;StreetWalk*w=streetWalks+i;q->s=0;
 if(street_civ_cede_zebra(q,w,dt))return;
 if(w->pause>0){w->pause-=dt;return;}
 if(d2(w->mx-q->x,w->my-q->y)<.18f){
  if(!street_civ_gap(q,w->mx,w->my)){
   if(!w->cross&&(w->stalled+=dt)>2.1f+(i%6)*.22f)street_civ_replan(i);
   return;
  }
  q->x=w->mx;q->y=w->my;w->cross=0;w->blocked=0;w->stalled=0;
  if(w->detour){
   w->detour=0;w->axis^=1;w->goal=(w->goal+3+i%4)%LEN(loc);w->visits++;
   w->leg=STREET_EN_ROUTE;street_civ_goal(i);
  }
  if(w->leg==STREET_LEAVE)w->leg=STREET_EN_ROUTE;
  if(w->leg==STREET_ARRIVE){
   w->mx=w->tx;w->my=w->ty;w->leg=STREET_LEAVE;
   w->goal=(w->goal+1+(i+w->visits++)%5)%LEN(loc);
   w->axis=(i+w->visits+w->goal)&1;
   w->pause=1.6f+(i%5)*.55f;
   street_civ_goal(i);return;
  }
  w->fromX=q->x;w->fromY=q->y;
  int dx=absf(q->x-w->tx)>.4f,dy=absf(q->y-w->ty)>.4f;
  if(!dx&&!dy){
   /* Exact-corner destinations are already reached; keep them in the
    * arrival phase so their next tick begins the same pause as mid-block. */
   w->leg=STREET_ARRIVE;w->mx=w->ax;w->my=w->ay;
  }else if(dx&&(!dy||!w->axis))w->mx=street_civ_corner(q->x,w->tx,&w->cross);
  else w->my=street_civ_corner(q->y,w->ty,&w->cross);
  if(w->cross){
   if(street_civ_cross_queue(q,w)||!street_can_start_leg(q,w)){
    w->mx=q->x;w->my=q->y;w->cross=0;w->pause=.22f+(i%5)*.05f;
    if((w->crossWait+=.24f)>3.8f+(i%6)*.22f){w->crossWait=0;street_civ_replan(i);}
    return;
   }
   w->crossWait=0;
  }else w->crossWait=0;
 }
 float speed=w->cross?62.f:27.f+(i%6)*2.f;
 if(p.heat>35.f&&d2(p.x-q->x,p.y-q->y)<110.f*110.f)speed+=17.f;
 if(!street_civ_yield(q,w,dt,speed))street_walk_step(q,w,dt,speed);
 else if(w->cross==1&&(w->blocked+=dt)>.62f){
  w->mx=w->crossX;w->my=w->crossY;w->cross=2;w->blocked=0;w->pause=.20f;
 }
 if(q->s>0)w->stalled=0;
 else if(!w->cross&&(w->stalled+=dt)>2.1f+(i%6)*.22f)street_civ_replan(i);
}
static void street_spawn(void){
 roadFX_reset();
 z(streetDrivers,sizeof streetDrivers);z(streetWalks,sizeof streetWalks);
 for(int j=0;j<LEN(streetJunctionOwner);j++)streetJunctionOwner[j]=-1;
 nc=3;
 cars[0]=(Car){284,528,0,0,0,1,100};
 cars[1]=(Car){485.25f,813,1.570796f,0,1,0,100};
 cars[2]=(Car){538,720,1.570796f,0,2,0,100};
 for(int k=0;k<120&&nc<28;k++){
  int j=1+rnd()%7,block=1+rnd()%6,side=rnd()%2?1:-1,t=rnd()%LEN(specs);
  float x=j*256+side*26.75f,y=block*256+61+rnd()%135,a=side*1.570796f;
  if(k%2){float q=x;x=y;y=q;a=side>0?0:3.141593f;}
  Car c={x,y,a,0,t,0,100};int clear=1;
  for(int i=0;i<nc;i++)if(street_overlap(cars+i,&c)){clear=0;break;}
  if(clear&&!street_hits_wall(&c))cars[nc++]=c;
 }
 for(int k=0;k<13;k++){
  int loop=k%LEN(streetLoops),n=(k*7)%4*3;
  float ax,ay,bx,by;
  street_loop_point(loop,(n+11)%12,&ax,&ay);street_loop_point(loop,n,&bx,&by);
  float f=.18f+(k%5)*.146f;
  Car c={ax+(bx-ax)*f,ay+(by-ay)*f,look(bx-ax,by-ay),31.f,k%LEN(specs),0,100};
  if(street_car_clear(nc,0,&c)){
   cars[nc]=c;streetDrivers[nc]=(StreetDriver){1,loop,n,45.f+(k%4)*4.f};nc++;
  }
 }
 /* The contract slot enters the city when a contact stages a vehicle. */
 cars[nc++]=(Car){.x=-100,.y=-100};
 for(int i=0;i<LEN(peds);i++){
  if(i<STREET_CIVILIANS)street_spawn_civilian(i);else peds[i]=(Ped){.role=1};
 }
}
