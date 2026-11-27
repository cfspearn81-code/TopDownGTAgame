/* All camera state is presentation-only. Fixed steps, never paint calls,
 * release a shortened boom. World-space controls stay north-up on foot.
 * At the helm p mirrors the real hull; it never indexes the road-car array. */
static float harbor_boat_speed(void);
enum {CH_FOOT=-2,CH_HELM=-3};
typedef struct {float x,y,a,dist,high,aim,anchor;int subject;float look;} ChMount;
static struct {int active,subject;float boom,yaw,lookX,lookY,x,y,rx,ry,ra,rd,rh;} chSpring;
/* This is the road camera's own suspension, not the car mesh suspension.
 * Road heading, signed speed and acceleration drive an actual boom/orbit;
 * no screen distortion, per-pixel resampling, RNG or motion shake. */
static struct {float prevSpeed,brake,lead,orbit;int cond;} chDrive;
static int ch_mode_active(void){return ch_pref()&&!fastGfx;}
static int ch_enabled(void){return ch_mode_active()&&!p.dead&&(scene==1||(scene==2&&helpBack==1));}
static void ch_reset_spring(void){
 chSpring.active=0;ch.ride=-1;chDrive.brake=chDrive.lead=chDrive.orbit=0;
}
static float ch_clampf(float x,float l,float h){return x<l?l:x>h?h:x;}
static float ch_damp(float have,float want,float gain,float cap){
 return have+ch_clampf((want-have)*gain,-cap,cap);
}
static void ch_motion(float speed,float err){
 if(!ch_mode_active()){ch.ride=-1;return;}
 const Car*c=cars+p.car;
 if(ch.ride!=p.car||ngCamRide!=p.car||ch.range<1.f){
  ch.range=79;ch.height=38;
  chDrive.prevSpeed=speed;chDrive.cond=c->cond;
  chDrive.brake=chDrive.lead=chDrive.orbit=0;
 }
 ch.ride=p.car;
 /* Below one world unit/second the car is optically parked. Suppress
    subpixel creep (e.g. after releasing the brake) rather than orbiting. */
 float moving=ch_clampf(speed-1.f,0,205.f);
 float pace=moving/205.f,corner=ch_clampf(absf(err),0,.78f)*ch_clampf(moving/7.f,0,1);
 const float direction=c->s<-.75f?-1.f:1.f;
 float decel=chDrive.prevSpeed-speed;
 /* A single small compression from actual deceleration or a bump.
    Release with no bounce; reversing at steady speed is not braking. */
 float brake=ch_clampf((decel-.52f)*.15f,0,1);
 if(c->cond+2<chDrive.cond&&brake<.64f)brake=.64f;
 chDrive.brake=ch_damp(chDrive.brake,brake,brake>chDrive.brake?.30f:.105f,.26f);
 chDrive.prevSpeed=speed;chDrive.cond=c->cond;
 float stretch=direction>0&&moving>0?ngCamBurst:0;
 float range=79.f+21.f*pace+18.f*stretch-10.f*corner-13.f*chDrive.brake;
 float high=38.f+8.f*pace+10.f*corner-4.75f*chDrive.brake;
 ch.range=ch_damp(ch.range,range,.097f,1.32f);
 ch.height=ch_damp(ch.height,high,.107f,.74f);
 /* 2D lead maps to a true look point ahead of (or behind) the chassis,
    not moving the car or the camera's collision pivot into a wall. */
 float forward=direction>0?22.f*pace+6.f*stretch-7.f*chDrive.brake:-9.f*ch_clampf(moving/60.f,0,1);
 chDrive.lead=ch_damp(chDrive.lead,ch_clampf(forward,-9.f,24.f),.12f,.93f);
 /* The 2D "roll" cue becomes a small real side orbit around the chassis
    during the apex. The horizon stays stable, including when switching. */
 float bank=ch_clampf(-err*direction*.27f*ch_clampf(moving/94.f,0,1),-.073f,.073f);
 chDrive.orbit=ch_damp(chDrive.orbit,bank,.15f,.009f);
}
static int ch_subject(void){return harbor_boat_active()?CH_HELM:street_car_exists(p.car)?p.car:CH_FOOT;}
static ChMount ch_mount(void){
 int id=ch_subject();
 if(id>=0){
  const Car*c=cars+id;
  int motion=ch.ride==id&&ngCamRide==id;
  return(ChMount){c->x,c->y,motion?wrap(ngCamAngle+chDrive.orbit):ngCamRide==id?ngCamAngle:c->a,
   ch.ride==id&&ch.range>1?ch.range:79.f,ch.ride==id&&ch.height>1?ch.height:38.f,4,.303f,id,motion?chDrive.lead:0.f};
 }
 if(id==CH_HELM){
  float fast=ch_clampf(absf(harbor_boat_speed())/158.f,0,1);
  return(ChMount){p.x,p.y,chSpring.active&&chSpring.subject==id?chSpring.yaw:p.a,
    86.f+fast*19.f,42.f+fast*4.f,4,.268f,id};
 }
 return(ChMount){p.x+chSpring.lookX,p.y+chSpring.lookY,-1.5707963268f,53.f,52.f,6,.158f,id};
}
/* Slabs are tested against the actual sightline in 3 dimensions. A flat
 * ocean/bridge clearance flag or an off-road lawn cannot retract a camera.
 * Exact wall contacts also avoid the old five-world-unit staircase. */
static void ch_trim(float*fit,ChMount m,float dx,float dy,float l,float t,float r,float b,float top){
 const float pad=3.f;
 l-=m.x+pad;r-=m.x-pad;t-=m.y+pad;b-=m.y-pad;
 float lo=0.f,hi=*fit;
 if(absf(dx)<.0001f){if(l>0||r<0)return;}
 else{float a=l/dx,e=r/dx;if(a>e){float k=a;a=e;e=k;}if(a>lo)lo=a;if(e<hi)hi=e;if(hi<lo)return;}
 if(absf(dy)<.0001f){if(t>0||b<0)return;}
 else{float a=t/dy,e=b/dy;if(a>e){float k=a;a=e;e=k;}if(a>lo)lo=a;if(e<hi)hi=e;if(hi<lo)return;}
 if(hi<0||lo> *fit||m.aim+(m.high-m.aim)*lo/m.dist>top+2.f)return;
 *fit=ch_clampf(lo-1.f,1.f,*fit);
}
static float ch_clearance(ChMount m,float co,float si){
 float fit=m.dist,dx=-co,dy=-si;
 float bx=m.x+dx*m.dist,by=m.y+dy*m.dist;
 float l=(bx<m.x?bx:m.x)-5,r=(bx>m.x?bx:m.x)+5,t=(by<m.y?by:m.y)-5,b=(by>m.y?by:m.y)+5;
 if(l<BASE_ROOM&&t<BASE_ROOM&&r>=0&&b>=0){
  for(int gy=cl((int)t/256,0,7);gy<=cl((int)b/256,0,7);gy++)for(int gx=cl((int)l/256,0,7);gx<=cl((int)r/256,0,7);gx++){
   int tile=gy*8+gx;
   for(int i=0;i<lot[tile][0];i++){
    int n=lot[tile][i+1];const Building*v=buildings+n;StreetBounds q=building_bounds(v);
    ch_trim(&fit,m,dx,dy,q.x,q.y,q.x+q.w,q.y+q.h,22+1.55f*v->ht+(n%11==6?19:0));
   }
   const BlockScenery*blk=scenery+tile;
   for(int j=blk->treesFrom;j<blk->count;j++){const StreetBounds*q=blk->walls+j;
    ch_trim(&fit,m,dx,dy,q->x-2,q->y-2,q->x+q->w+2,q->y+q->h+2,30);}
  }
 }
 if(t<3000&&b>2020&&l<1110)for(int i=0;i<woodPropN;i++){
  const WoodProp*w=woodProps+i;if(w->kind==WOOD_UNDERGROWTH||w->kind==WOOD_POST)continue;
  float rad=w->kind==WOOD_TREE?w->radius+2:w->radius;
  if(w->x+rad<l||w->y+rad<t||w->x-rad>r||w->y-rad>b)continue;
  ch_trim(&fit,m,dx,dy,w->x-rad,w->y-rad,w->x+rad,w->y+rad,w->height);
 }
 if(r>2420&&b>2010)for(int i=0;i<LEN(harborPieces);i++)if(harborPiecePlaced[i]){
  const HarborPiece*q=harborPieces+i;
  ch_trim(&fit,m,dx,dy,q->x,q->y,q->x+q->w,q->y+q->h,q->height);
 }
 return fit;
}
static void ch_fixed_step(void){
 if(!ch_enabled()){chSpring.active=0;return;}
 int id=ch_subject(),snap=!chSpring.active||id!=chSpring.subject||d2(p.x-chSpring.x,p.y-chSpring.y)>180*180;
 if(snap){chSpring.lookX=chSpring.lookY=0;chSpring.yaw=p.a;}
 else if(id==CH_HELM)chSpring.yaw=wrap(chSpring.yaw+wrap(p.a-chSpring.yaw)*.096f);
 if(id==CH_FOOT){
  /* Only point toward physically traversed ground, not attempted motion
   * against walls, held menus, or a travel/door teleport. */
  float dx=snap?0:ch_clampf((p.x-chSpring.x)*5.5f,-6.f,6.f);
  float dy=snap?0:ch_clampf((p.y-chSpring.y)*5.5f,-6.f,6.f);
  chSpring.lookX+=(dx-chSpring.lookX)*.12f;chSpring.lookY+=(dy-chSpring.lookY)*.12f;
  if(solid(p.x+chSpring.lookX,p.y+chSpring.lookY,1)){chSpring.lookX=chSpring.lookY=0;}
 }
 chSpring.x=p.x;chSpring.y=p.y;chSpring.subject=id;chSpring.active=1;
 ChMount m=ch_mount();float safe=ch_clearance(m,cs(m.a),sn(m.a));
 chSpring.rx=m.x;chSpring.ry=m.y;chSpring.ra=m.a;chSpring.rd=m.dist;chSpring.rh=m.high;
 if(snap||safe<=chSpring.boom)chSpring.boom=safe;
 else chSpring.boom+=ch_clampf((safe-chSpring.boom)*.10f,0.f,1.50f);
}
static void ch_frame_rig(void){
 ChMount m=ch_mount();ch.co=cs(m.a);ch.si=sn(m.a);
 /* Read-only fallback covers a teleport before the next game tick, field
  * guide redraws, and a frame that is painted more than once. */
 int sampled=chSpring.active&&chSpring.subject==m.subject&&m.x==chSpring.rx&&m.y==chSpring.ry&&m.a==chSpring.ra&&m.dist==chSpring.rd&&m.high==chSpring.rh;
 float fit=sampled?chSpring.boom:ch_clearance(m,ch.co,ch.si);
 if(!sampled&&chSpring.active&&chSpring.subject==m.subject&&d2(p.x-chSpring.x,p.y-chSpring.y)<16.f&&chSpring.boom<fit)fit=chSpring.boom;
 float high=m.high+ch_clampf(34.f-fit,0,34.f)*.93f;
 chFit=fit;
 /* When the boom is braced against a wall, keep the scene framed on
    the chassis. In open space this is the actual new forward sightline. */
 float optical=fit;
 if(m.subject>=0)optical+=m.look*ch_clampf((fit-24.f)/48.f,0,1);
 float target=m.anchor,ca=sq(1.f-target*target),hAim=high-m.aim,diag=sq(optical*optical+hAim*hAim);
 chSin=(hAim*ca-optical*target)/diag;chCos=(optical*ca+hAim*target)/diag;
 ch.x=m.x-fit*ch.co;ch.y=m.y-fit*ch.si;ch.h=high;
}
static int ch_world_input_allowed(void){
 if(p.dead||menu>=0||mapOn||wheelOpen||vdPanel||taxiRadio||atmOpen>=0||npcTalkOn)return 0;
 if(scene==1||scene==3||scene==4)return 1;
#if __has_include("night_club.inc")
 if(scene==NIGHT_CLUB_SCENE)return !ncPanel;
#endif
#if __has_include("night_venues.inc")
 if(scene==NIGHT_VENUES_SCENE)return nvs_room==NVR_GLASS&&nvs_mode==4;
#endif
 if(scene==NIGHT_TRAP_SCENE)return nt_pane<0;
 return 0;
}
