/* Arcade skiffs use the same 60 Hz controls as cars, but remain separate
 * from the numbered road-vehicle, pedestrian and save pools. Navigation
 * and dock geometry belong to the harbor's shared terrain mask. */
#define DC_HARBOR_BOATS 1
enum{HB_DOCKS=3,HB_AMBIENT=5,HB_WAKE=104,HB_CONDITION=100};
enum{HB_SKIFF,HB_CABIN,HB_CARGO,HB_PATROL};
typedef struct {float x,y,a,s,vx,vy;int kind;} HarborHull;
typedef struct {
 HarborHull hull;
 float goalX[4],goalY[4],blocked;
 int route,live,leg;
} HarborTraffic;
typedef struct {float x,y,a,time,span;int owner;} HarborWake;
static HarborTraffic hbTraffic[HB_AMBIENT];
static HarborWake hbWakes[HB_WAKE];
static struct {
 HarborHull hull;
 float x0,y0,safeX,safeY,contactSpeed,cool,wakeTime;
 int active,dock,condition,contact;
} hb;
static float hbClock,hbNpcWake;
static int hbReady,hbWakeSlot;
static int harbor_boat_active(void){return hb.active;}
static float harbor_boat_speed(void){return hb.active?hb.hull.s:0.f;}
static int harbor_boat_save_point(float*x,float*y){
 if(!hb.active)return 0;*x=hb.safeX;*y=hb.safeY;return 1;
}
static int harbor_boat_hit(float x,float y,float radius,float minimumSpeed){
 if(scene!=1||mapOn||menu>=0||p.dead||!hb.active||!hb.contact||radius<=0||hb.contactSpeed<minimumSpeed)return 0;
 float dx=hb.hull.x-hb.x0,dy=hb.hull.y-hb.y0,n=d2(dx,dy);
 if(n<.027f)return 0;
 float u=((x-hb.x0)*dx+(y-hb.y0)*dy)/n;
 if(u<0)u=0;if(u>1)u=1;
 return d2(hb.x0+dx*u-x,hb.y0+dy*u-y)<=radius*radius;
}
static void harbor_boats_reset(void){
 z(&hb,sizeof hb);hb.dock=-1;hb.condition=HB_CONDITION;
 z(hbTraffic,sizeof hbTraffic);z(hbWakes,sizeof hbWakes);
 hbReady=hbWakeSlot=0;hbClock=hbNpcWake=0;
}
static int harbor_boat_nav_clear(float x,float y){return harbor_nav_water(x,y,16);}
static int harbor_boat_clear(float x,float y,float a){
 float u=cs(a)*11.f,v=sn(a)*11.f;
 return harbor_nav_water(x,y,7)&&harbor_nav_water(x+u,y+v,5)&&harbor_nav_water(x-u,y-v,5);
}
static int hb_dock(int id,int*lx,int*ly,int*wx,int*wy){
 return id>=0&&id<HB_DOCKS&&harbor_dock_point(id*2,lx,ly)&&harbor_dock_point(id*2+1,wx,wy);
}
static int hb_land_clear(float x,float y){
 return street_foot_clear(x,y,-1);
}
static int hb_landing(int dock,float*outX,float*outY){
 int lx,ly,wx,wy;
 if(!hb_dock(dock,&lx,&ly,&wx,&wy))return 0;
 /* Return onto this dock's walkable apron, never the water or wall.
  * The first candidate is the authored standing anchor. */
 static const signed char nearby[][2]={{0,0},{-12,0},{12,0},{0,-12},{-12,-12},{12,-12},{0,-21},{-20,0},{20,0},{0,10}};
 for(int i=0;i<LEN(nearby);i++){
  float x=lx+nearby[i][0],y=ly+nearby[i][1];
  if(hb_land_clear(x,y)&&street_line_open(x,y,lx,ly,3)){
   *outX=x;*outY=y;return 1;
  }
 }
 return 0;
}
static int hb_near_dock(float x,float y,int onWater){
 int selected=-1;float best=onWater?32.f*32.f:29.f*29.f;
 for(int i=0;i<HB_DOCKS;i++){
  int lx,ly,wx,wy;if(!hb_dock(i,&lx,&ly,&wx,&wy))continue;
  float dd=onWater?d2(x-wx,y-wy):d2(x-lx,y-ly);
  if(dd>=best)continue;
  if(!onWater&&!street_line_open(x,y,lx,ly,4))continue;
  selected=i;best=dd;
 }
 return selected;
}
static const char* hb_dock_name(int i){
 static const char*const names[]={"PUBLIC LANDING","CARGO QUAY","TIDE STATION"};
 return names[cl(i,0,HB_DOCKS-1)];
}
static void hb_tow(void){
 float x=hb.safeX,y=hb.safeY;
 if(!hb_landing(hb.dock,&x,&y)&&!hb_land_clear(x,y))
  for(int i=0;i<HB_DOCKS;i++)if(hb_landing(i,&x,&y))break;
 if(!hb_land_clear(x,y)){
  /* Even if every harbor apron were blocked, the tow can return us
   * to an ordinary-city safe footing rather than leave the driver wet. */
  float ox=loc[9].x,oy=loc[9].y;
  for(int r=6;r<180&&!hb_land_clear(x,y);r+=6)for(int i=0;i<16;i++){
   float tx=ox+r*cs(i*.3926991f),ty=oy+r*sn(i*.3926991f);
   if(hb_land_clear(tx,ty)){x=tx;y=ty;break;}
  }
 }
 hb.active=hb.contact=0;hb.hull.s=hb.hull.vx=hb.hull.vy=0;
 p.x=x;p.y=y;p.a=-1.570796f;walking=0;
 tell("Harbor tow / back on the quay. A fresh launch is ready.");
 request_save();
}
static int hb_board(int dock){
 int lx,ly,wx,wy;
 if(!hb_dock(dock,&lx,&ly,&wx,&wy)||!harbor_boat_clear(wx,wy,1.5707963f)){
  tell("This berth is closed. Try another marked landing.");return 0;
 }
 hb.hull=(HarborHull){.x=wx,.y=wy,.a=1.5707963f,.kind=HB_SKIFF};
 hb.safeX=p.x;hb.safeY=p.y;hb.active=1;hb.dock=dock;
 hb.condition=HB_CONDITION;hb.cool=hb.wakeTime=0;hb.contact=0;
 p.x=wx;p.y=wy;p.a=hb.hull.a;walking=0;near=by=-1;stoppingToExit=0;
 pulse_guard_reset();
 tell("Harbor launch / steer with arrows or WASD. Shift slows. E at a marked berth.");
 return 1;
}
static int hb_land(int dock){
 float x,y;
 if(!hb_landing(dock,&x,&y)){tell("Quay blocked. Try a different landing.");return 0;}
 hb.active=hb.contact=0;hb.dock=dock;hb.hull.s=hb.hull.vx=hb.hull.vy=0;
 p.x=x;p.y=y;p.a=-1.5707963f;walking=0;
 tell("Moored safely. A harbor launch remains available here.");
 request_save();return 1;
}
static void hb_wake(const HarborHull*h,int owner,float strength){
 if(fastGfx||strength<10.f)return;
 float co=cs(h->a),si=sn(h->a),sign=h->s>=0?1.f:-1.f;
 HarborWake*w=hbWakes+(hbWakeSlot++%HB_WAKE);
 *w=(HarborWake){h->x-co*sign*11,h->y-si*sign*11,h->a+(sign<0?3.1415926f:0.f),1.f,6.f+strength*.084f,owner};
 if(hbWakeSlot>1000000)hbWakeSlot=0;
}
static int hb_lane_open(float x,float y,float tx,float ty){
 float dx=tx-x,dy=ty-y;int n=1+(int)(sq(d2(dx,dy))/7.f);
 for(int i=0;i<=n;i++)if(!harbor_nav_water(x+dx*i/n,y+dy*i/n,16))return 0;
 return 1;
}
static int hb_route(HarborTraffic*b,int id){
 static const short centers[][2]={{2900,2410},{2750,2520},{2850,2690},{2565,2840},{2950,2810},{2690,2920},{2980,2580}};
 for(int k=0;k<LEN(centers);k++)for(int w=0;w<3;w++){
  int i=(id+k)%LEN(centers),a=centers[i][0],c=centers[i][1],rx=75-14*w,ry=65-13*w;
  float x[4]={a-rx,a+rx,a+rx,a-rx},y[4]={c-ry,c-ry,c+ry,c+ry};
  int ok=1;
  for(int j=0;j<4&&ok;j++)if(!hb_lane_open(x[j],y[j],x[(j+1)%4],y[(j+1)%4]))ok=0;
  if(!ok)continue;
  for(int j=0;j<id&&ok;j++)if(hbTraffic[j].route==i+1)ok=0;
  if(!ok&&k<LEN(centers)-1)continue;
  for(int j=0;j<4;j++){b->goalX[j]=x[j];b->goalY[j]=y[j];}
  int st=(id*3)%4,n=(st+1)%4;float spread=.22f+.13f*(id%4);
  b->hull=(HarborHull){x[st]+(x[n]-x[st])*spread,y[st]+(y[n]-y[st])*spread,
    look(x[n]-x[st],y[n]-y[st]),0,0,0,id==HB_AMBIENT-1?HB_PATROL:id%3};
  b->route=i+1;b->leg=n;b->live=id!=HB_AMBIENT-1;return 1;
 }
 return 0;
}
static int hb_close(float x,float y){
 return p.x>2150.f&&p.y>1810.f&&d2(p.x-x,p.y-y)<860.f*860.f;
}
static void harbor_boats_world(float dt){
 for(int i=0;i<HB_WAKE;i++)if(hbWakes[i].time>0.f)hbWakes[i].time-=dt;
 if(scene!=1||p.x<2070.f||p.y<1840.f)return;
 if(!hbReady){
  hbReady=1;
  for(int i=0;i<HB_AMBIENT;i++)hb_route(hbTraffic+i,i);
 }
 hbClock+=dt;if(hbClock>3600.f)hbClock-=3600.f;
 hbNpcWake-=dt;int wake=hbNpcWake<=0;if(wake)hbNpcWake=.12f;
 for(int i=0;i<HB_AMBIENT;i++){
  HarborTraffic*b=hbTraffic+i;HarborHull*h=&b->hull;
  if(!b->route)continue;
  int patrol=h->kind==HB_PATROL;
  int watch=hb.active&&p.heat>42.f;
  if(patrol&&!b->live){
   if((watch||(int)hbClock%51>=28)&&d2(h->x-p.x,h->y-p.y)>96.f*96.f)b->live=1;
   else continue;
  }
  if(patrol&&b->live&&!watch&&(int)hbClock%51<23&&d2(h->x-p.x,h->y-p.y)>720.f*720.f){b->live=0;continue;}
  float tx=b->goalX[b->leg],ty=b->goalY[b->leg];
  if(d2(h->x-tx,h->y-ty)<21.f*21.f){
   b->leg=(b->leg+1)%4;tx=b->goalX[b->leg];ty=b->goalY[b->leg];
  }
  /* Simple arcade patrol response; the same water and collision gates
   * apply as for civilian traffic. No map spawning in front of the player. */
  if(patrol&&watch&&d2(h->x-p.x,h->y-p.y)>42.f*42.f&&d2(h->x-p.x,h->y-p.y)<218.f*218.f&&hb_lane_open(h->x,h->y,p.x,p.y)){
   tx=p.x;ty=p.y;
  }
  float d=wrap(look(tx-h->x,ty-h->y)-h->a),step=2.65f*dt;
  if(d>step)d=step;if(d<-step)d=-step;
  float angle=wrap(h->a+d);
  if(harbor_boat_clear(h->x,h->y,angle))h->a=angle;
  const float pace=patrol?(watch?76.f:51.f):(31.f+i*7.f);
  float speed=h->s+(pace-h->s)*dt*3.0f;
  float x=h->x+cs(h->a)*speed*dt,y=h->y+sn(h->a)*speed*dt;
  int block=!harbor_boat_clear(x,y,h->a);
  if(hb.active&&d2(x-hb.hull.x,y-hb.hull.y)<24.f*24.f&&d2(x-hb.hull.x,y-hb.hull.y)<d2(h->x-hb.hull.x,h->y-hb.hull.y))block=1;
  for(int j=0;j<HB_AMBIENT&&!block;j++)if(j!=i&&hbTraffic[j].live){
   const HarborHull*q=&hbTraffic[j].hull;
   if(d2(x-q->x,y-q->y)<22.f*22.f&&d2(x-q->x,y-q->y)<d2(h->x-q->x,h->y-q->y))block=1;
  }
  if(block){h->s*=.88f;b->blocked+=dt;}
  else{h->x=x;h->y=y;h->s=speed;b->blocked=0;}
  if(b->blocked>2.f){b->leg=(b->leg+1)%4;b->blocked=0;}
  if(wake&&hb_close(h->x,h->y))hb_wake(h,i+1,absf(h->s));
 }
}
static int hb_block_hull(HarborHull*h,float x,float y){
 if(!harbor_boat_clear(x,y,h->a))return 1;
 for(int i=0;i<HB_AMBIENT;i++)if(hbTraffic[i].live){
  const HarborHull*q=&hbTraffic[i].hull;
  if(d2(x-q->x,y-q->y)<23.f*23.f&&d2(x-q->x,y-q->y)<d2(h->x-q->x,h->y-q->y))return 1;
 }
 return 0;
}
static int harbor_boats_tick(U keys,U tap,float dt){
 hb.contact=0;
 if(scene!=1||p.dead||menu>=0||mapOn)return 0;
 if(!hb.active){
  if(!(tap&USE)||p.car>=0||p.x<2150||p.y<1960)return 0;
  int dock=hb_near_dock(p.x,p.y,0);if(dock<0)return 0;
  hb_board(dock);return 1;
 }
 HarborHull*h=&hb.hull;int dock=hb_near_dock(h->x,h->y,1);
 if(tap&USE){
  if(dock<0)tell("Moor at a marked quay to leave the boat.");
  else if(d2(h->vx,h->vy)>19.f*19.f||absf(h->s)>20.f)tell("Slow the launch with Shift, then press E to moor.");
  else if(hb_land(dock))return 1;
 }
 hb.x0=h->x;hb.y0=h->y;
 int throttle=!!(keys&UP)-!!(keys&DOWN),steer=!!(keys&RIGHT)-!!(keys&LEFT);
 if(throttle){
  float accel=throttle>0?111.f:72.f;
  if(throttle*h->s<0)accel*=1.42f;
  h->s+=dt*throttle*accel;
 }else h->s*=1.f-dt*1.60f;
 if(keys&BRAKE)h->s*=1.f-dt*7.6f;
 if(h->s<-55.f)h->s=-55.f;if(h->s>158.f)h->s=158.f;
 if(absf(h->s)<.16f)h->s=0;
 float pace=absf(h->s)/104.f;if(pace>1)pace=1;
 float angle=wrap(h->a+steer*(h->s<-7?-1.f:1.f)*(1.15f+pace*1.37f)*dt);
 if(harbor_boat_clear(h->x,h->y,angle))h->a=angle;
 float damp=dt*(keys&BRAKE?9.4f:3.4f);
 h->vx+=(cs(h->a)*h->s-h->vx)*damp;
 h->vy+=(sn(h->a)*h->s-h->vy)*damp;
 float x=h->x+h->vx*dt,y=h->y+h->vy*dt,speed=sq(d2(h->vx,h->vy));
 hb.cool-=dt;
 if(hb_block_hull(h,x,y)){
  if(!hb_block_hull(h,x,h->y)){h->x=x;h->vy*=-.20f;}
  else if(!hb_block_hull(h,h->x,y)){h->y=y;h->vx*=-.20f;}
  else{h->vx*=-.20f;h->vy*=-.20f;}
  h->s*=.42f;
  if(speed>54.f&&hb.cool<=0.f){
   hb.condition=max(0,hb.condition-cl((int)((speed-38.f)*.17f),3,24));hb.cool=.48f;
   if(hb.condition<=0){hb_tow();return 1;}
   char out[68];fmt(out,"Hull contact / integrity ",hb.condition);tell(out);
  }
 }else{h->x=x;h->y=y;}
 hb.contactSpeed=speed;hb.contact=1;
 hb.wakeTime-=dt;if(hb.wakeTime<=0){hb.wakeTime=.075f;hb_wake(h,0,speed);}
 p.x=h->x;p.y=h->y;p.a=h->a;walking=0;near=by=-1;
 return 1;
}

/* Helm rendering is isolated from hb_board, movement, hull and traffic.
 * Neither camera, animated foam, cosmetics nor the instrument panel can
 * advance an actor, mutate hull condition, or consume the game RNG. */
typedef struct{int x,y;} HbDrawPoint;
typedef struct{int x,y;float c,s;} HbDrawRig;
static HbDrawPoint hb_art(const HbDrawRig*r,float x,float y){
 return (HbDrawPoint){r->x+(int)(x*r->c-y*r->s),r->y+(int)(x*r->s+y*r->c)};
}
static void hb_quad(HbDrawPoint a,HbDrawPoint b,HbDrawPoint c,HbDrawPoint d,U ink){
 tri(a.x,a.y,b.x,b.y,c.x,c.y,ink);tri(a.x,a.y,c.x,c.y,d.x,d.y,ink);
}
static void hb_edge(const HbDrawRig*r,float a,float b,float c,float d,U ink){
 HbDrawPoint p=hb_art(r,a,b),q=hb_art(r,c,d);line(p.x,p.y,q.x,q.y,ink);
}
static void hb_deck(const HbDrawRig*r,float aft,float fore,float half,U ink){
 hb_quad(hb_art(r,aft,-half),hb_art(r,fore,-half),hb_art(r,fore,half),hb_art(r,aft,half),ink);
}
static void hb_deck_rect(const HbDrawRig*r,float a,float f,float port,float starboard,U ink){
 hb_quad(hb_art(r,a,port),hb_art(r,f,port),hb_art(r,f,starboard),hb_art(r,a,starboard),ink);
}
static void hb_art_dot(const HbDrawRig*r,float u,float v,int radius,U color){
 HbDrawPoint p=hb_art(r,u,v);
 if(radius)disc(p.x,p.y,radius,color);else dot(p.x,p.y,color);
}
static void hb_hull_paint(const HarborHull*h){
 int x=sx(h->x),y=sy(h->y);if(x<-32||y<-32||x>W+32||y>H+32)return;
 HbDrawRig r={x,y,cs(h->a),sn(h->a)};
 static const U paint[]={0x6abdad,0xd89451,0xb86c5d,0x86add9};
 static const U stripe[]={0xdcf6df,0xffe0ad,0xefd4a9,0xdfedf0};
 int skin=cl(h->kind,0,3),dark=night_world_is_night();
 U c=paint[skin],rail=stripe[skin],side=mix(c,0x192f45,4);
 float length=skin==HB_CARGO?18.f:skin==HB_PATROL?16.4f:15.7f,beam=skin==HB_CARGO?8.1f:7.f;
 float step[]={-length,-length+4,length-8.f,length-3.f,length};
 float widths[]={beam*.84f,beam,beam*.93f,beam*.47f,.10f};
 for(int i=0;i<4;i++){
  HbDrawPoint a=hb_art(&r,step[i],-widths[i]),b=hb_art(&r,step[i+1],-widths[i+1]);
  HbDrawPoint c0=hb_art(&r,step[i+1],widths[i+1]),d=hb_art(&r,step[i],widths[i]);
  hb_quad((HbDrawPoint){a.x+1,a.y+2},(HbDrawPoint){b.x+1,b.y+2},
   (HbDrawPoint){c0.x+1,c0.y+2},(HbDrawPoint){d.x+1,d.y+2},0x123342);
  hb_quad(a,b,c0,d,i>1?mix(side,c,4):side);
  hb_edge(&r,step[i],-widths[i],step[i+1],-widths[i+1],rail);
  hb_edge(&r,step[i],widths[i],step[i+1],widths[i+1],c);
 }
 hb_edge(&r,step[0],-widths[0],step[0],widths[0],mix(c,rail,2));
 for(int j=-1;j<=1;j+=2){
  hb_edge(&r,-length+3,j*(beam-1.6f),length-10,j*(beam-1.6f),0x253943);
  if(!fastGfx)for(int i=-8;i<7;i+=5)hb_art_dot(&r,i,j*(beam-.8f),0,rail);
  hb_art_dot(&r,length-4,j*1.65f,0,0xbecbc5);
 }
 if(skin==HB_SKIFF){
  hb_deck(&r,-length-1.45f,-length+1.9f,1.85f,0x263746); /* outboard */
  hb_edge(&r,-length-.6f,-1.5f,-length+.5f,1.5f,0x718f90);
  hb_deck(&r,-10.2f,-5.8f,4.5f,0x445963);
  hb_edge(&r,-9.24f,-4.1f,-9.24f,4.1f,0xc4a688); /* seat rails */
  hb_deck(&r,-3.2f,3.5f,4.4f,0x526d76);
  hb_edge(&r,2.55f,-4.8f,2.55f,4.8f,dark?0x94b3b7:0xafdddd);
  hb_edge(&r,3.3f,-4.f,3.3f,4.f,0x597785); /* windscreen */
  hb_art_dot(&r,-4.1f,0,1,h==&hb.hull?charTop[sv.look.part[TOP_TONE]]:0xcca36f);
#if __has_include("wardrobe_catalog.inc")
  if(h==&hb.hull&&ward_hat(ward_live()))hb_art_dot(&r,-3.23f,0,0,ward_hat_color(ward_live()));
#endif
  hb_edge(&r,6,-1.9f,10.6f,-.8f,0xb3a38e);
 }else if(skin==HB_CARGO){
  static const U cargo[]={0xa5805d,0xb18b65,0x8b9b8f};
  for(int i=0;i<3;i++){
   float a=-14.f+5.2f*i;
   hb_deck(&r,a,a+4.15f,4.7f,cargo[i]);
   hb_edge(&r,a+.5f,-3.6f,a+3.6f,-3.6f,mix(cargo[i],PAPER,2));
   hb_edge(&r,a+2.1f,-4.35f,a+2.1f,4.35f,0x584a37);
  }
  hb_deck(&r,2.8f,8.f,4.75f,0x5b7f85);
  hb_edge(&r,7.4f,-4.4f,7.4f,4.4f,dark?0xe9c993:0xa9d6cc);
 }else{
  hb_deck(&r,-8.75f,6.f,4.8f,mix(rail,0x526f79,3));
  hb_deck(&r,-6.9f,4.8f,3.55f,skin==HB_PATROL?0xa1b7c5:0xdac9a9);
  for(int j=-1;j<=1;j+=2){
   hb_edge(&r,-6.3f,4.3f*j,-2.25f,4.3f*j,dark?0xcaaf83:0x759ba5);
   hb_edge(&r,-.3f,4.3f*j,3.95f,4.3f*j,dark?0xd8bf91:0x82a2ae);
  }
  hb_edge(&r,4.6f,-3.75f,4.6f,3.75f,0x587a87);
  if(skin==HB_PATROL){
   int phase=(int)(anim*7.f)&1;
   hb_art_dot(&r,-3.2f,-2.9f,1,phase?0x5886b8:0x64c1ff);
   hb_art_dot(&r,-3.2f,2.9f,1,phase?0xc4ecf4:0x5d8ba9);
  }else hb_art_dot(&r,-3,-1,1,0xa17c58);
 }
 if(dark){
  hb_art_dot(&r,length-5.f,-2.9f,1,0xf58e87);
  hb_art_dot(&r,length-5.f,2.9f,1,0x9cf4b0);
  hb_art_dot(&r,-length+3.f,0,0,0xf5e9c7);
 }
}
static void hb_foam(float x,float y,int ink){
 int px=sx(x),py=sy(y);if((U)px>=W||(U)py>=H||!harbor_nav_water(x,y,0))return;
 U*dst=pixels+py*W+px;
 *dst=mix(*dst,nwStorm>.35f?0xa0b4bc:0xbdede4,ink);
}
static void hb_wakes_paint(void){
 if(fastGfx)return;
 for(int i=0;i<HB_WAKE;i++){
  const HarborWake*w=hbWakes+i;if(w->time<=0.f)continue;
  int x=sx(w->x),y=sy(w->y);if(x<-39||y<-39||x>W+39||y>H+39)continue;
  float elapsed=1.f-w->time,k=w->span+elapsed*14.f,co=cs(w->a),si=sn(w->a);
  int strength=cl((int)(w->time*(w->owner?4.4f:6.3f)),1,6);
  for(int side=-1;side<=1;side+=2)for(int sample=0;sample<11;sample++){
   float t=sample*.1f,jitter=sn(i*1.7f+side*1.5f+sample*1.02f)*elapsed;
   float sideways=side*k*(t*.88f+t*t*.12f)+jitter,f=elapsed*6.0f+t*8.3f;
   hb_foam(w->x-co*f-si*sideways,w->y-si*f+co*sideways,strength);
   if((sample+i+side+16)%3==0){
    f+=1.6f;sideways+=side*(1.2f+jitter*.65f);
    hb_foam(w->x-co*f-si*sideways,w->y-si*f+co*sideways,max(1,strength-2));
   }
  }
  /* Central stern wash holds for the fresh, near-field part of the wake. */
  if(w->time>.32f)for(int j=0;j<5;j++){
   float f=elapsed*5.f+j*.95f,v=sn(i+j*1.7f)*(.5f+elapsed*1.4f);
   hb_foam(w->x-co*f-si*v,w->y-si*f+co*v,max(1,strength-j/2));
  }
 }
}
static void harbor_boats_paint(void){
 if(!hb.active&&(p.x<2020.f||p.y<1800.f))return;
 hb_wakes_paint();
 for(int i=0;i<HB_AMBIENT;i++)if(hbTraffic[i].live)hb_hull_paint(&hbTraffic[i].hull);
 if(hb.active)hb_hull_paint(&hb.hull);
 for(int dock=0;dock<HB_DOCKS;dock++){
  int x,y,wx,wy;
  if(hb.active&&dock==hb.dock)continue;
  if(!hb_dock(dock,&x,&y,&wx,&wy)||sx(wx)<-32||sx(wx)>W+32||sy(wy)<-32||sy(wy)>H+32)continue;
  if(hb.active&&d2(wx-hb.hull.x,wy-hb.hull.y)<32.f*32.f)continue;
  if(harbor_boat_clear(wx,wy,1.5707963f)){
   HarborHull launch={.x=wx,.y=wy,.a=1.5707963f,.kind=HB_SKIFF};
   hb_hull_paint(&launch);
  }
 }
}
static int hb_heading(float a){
 /* Angle zero points map east, and screen-down points south. */
 int degrees=90+(int)(a*57.2957795f+(a>=0?.5f:-.5f));
 return (degrees%360+360)%360;
}
static void hb_compass(int x,int y,float angle){
 int h=hb_heading(angle);
 U ink=0xdbebda;disc(x,y,12,0x1a303d);disc(x,y,9,0x182630);
 for(int j=0;j<4;j++){
  int dx=j==1?9:j==3?-9:0,dy=j==0?-9:j==2?9:0;
  dot(x+dx,y+dy,j?0x789496:0xffdc9c);
 }
 float co=cs(angle),si=sn(angle);
 int dx=(int)(co*7.5f),dy=(int)(si*7.5f);
 tri(x+dx,y+dy,x-(int)(co*3.5f+si*3.f),y-(int)(si*3.5f-co*3.f),
     x-(int)(co*3.5f-si*3.f),y-(int)(si*3.5f+co*3.f),ink);
 char label[5]={(char)('0'+h/100),(char)('0'+h/10%10),(char)('0'+h%10),0};
 textat(x+17,y-10,label,ink,0);
}
static void harbor_boats_hint(void){
 if(scene!=1||menu>=0||mapOn||p.dead)return;
 if(hb.active){
  int dock=hb_near_dock(hb.hull.x,hb.hull.y,1);
  int recent=dock>=0&&absf(hb.hull.s)<=20.f&&d2(hb.hull.vx,hb.hull.vy)<19.f*19.f;
  int edge=hb.hull.x<100.f||hb.hull.y<100.f||hb.hull.x>ROOM-100.f||hb.hull.y>ROOM-100.f;
  const char*t=recent?"E  MOOR AT QUAY":dock>=0?"SHIFT  SLOW TO MOOR":
   edge?"CHART EDGE   /   TURN INLAND":"ARROWS / WASD  HELM    SHIFT  BRAKE";
  int x=265,y=475;box(x,y,431,50,0x111e27,0x4f7e81);
  rect(x+1,y+1,429,2,recent?TEAL:0x5b8f8f);
  hb_compass(x+19,y+14,hb.hull.a);
  line(x+74,y+6,x+74,y+24,0x38515e);
  textat(x+82,y+4,t,recent?TEAL:edge&&dock<0?GOLD:PAPER,0);
  textat(x+10,y+28,"HULL",0xabb9b7,0);
  rect(x+56,y+36,68,5,0x283b42);rect(x+56,y+36,hb.condition*68/100,5,hb.condition>32?TEAL:PINK);
  for(int i=1;i<4;i++)dot(x+56+i*17,y+36,0x111e27);
  char info[100];char*q=number(append(info,"KNOTS "),min(99,(int)(absf(hb.hull.s)*.27f)));
  q=append(q,"  /  ");append(q,dock>=0?hb_dock_name(dock):hb.hull.s<-.16f?"ASTERN / OPEN WATER":"HARBOR CHANNEL");
  textat(x+136,y+28,info,0xb7cbc5,0);
 }else if(p.car<0&&p.x>2150.f&&p.y>1960.f){
  int dock=hb_near_dock(p.x,p.y,0);if(dock<0)return;
  char label[90];append(append(label,"E  BOARD LAUNCH  /  "),hb_dock_name(dock));
  int w=textw(label,0)+22;box(W/2-w/2,487,w,29,0x14252b,0x62a6a0);center(W/2,491,label,TEAL,0);
 }
}
