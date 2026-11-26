/* Shared world-space geometry for the held pistol and arcade shot.
 * A compact side carry is a separate pose, never a collapsed muzzle ray. */
typedef struct {float mx,my,co,si,extent;int hand;} PulsePose;
static PulsePose firedPulse;
static float firedX,firedY,firedA;
typedef int (*PulseRoomSolid)(float,float);
enum {PULSE_CLEAR_TICKS=5};
static int pulseGuardView,pulseGuardTicks;
static void pulse_guard_reset(void){pulseGuardView=pulseGuardTicks=0;}
static int player_gait(void){return walking?(int)(sn(footcycle)*5):0;}
static float pulse_min(float x,float hi){return x<hi?x:hi;}
static float pulse_shoulder(float co,float si){
 /* Shift pure north off the face. Diagonals already clear the head
    by their forward projection, so they keep their natural alignment. */
 if(si>=-.7f)return 0;
 float weight=1.f-1.42f*absf(co);
 if(weight<=0)return 0;
 return (co<-.01f?-7.1f:7.1f)*weight;
}
static PulsePose pulse_aim(float x,float y,float a,int stride,float recoil){
 float co=cs(a),si=sn(a);
 /* Elbows stay near the ribcage. Facing down, lift the hands a little
    so the pose doesn't hang below the body as it projects forward. */
 float advance=16.f-.29f*absf((float)stride)-recoil;
 float lift=8.f+.38f*stride+(si>0?2.25f*si:0.f),side=pulse_shoulder(co,si);
 float dx=co*advance-si*side,dy=si*advance-lift+co*side;
 return (PulsePose){x+dx,y+dy,co,si,dx*co+dy*si,0};
}
static int pulse_path(float x,float y,float tx,float ty,PulseRoomSolid room){
 if(!room)return street_reach_open(x,y,tx,ty,1,-1);
 float dx=tx-x,dy=ty-y;
 int steps=1+(int)(absf(dx)+absf(dy));
 for(int i=1;i<=steps;i++)if(room(x+dx*i/steps,y+dy*i/steps))return 0;
 return 1;
}
static int pulse_cover(float x,float y,PulsePose aim,float pad,PulseRoomSolid room){
 float forward=aim.extent<13.f?13.f:aim.extent;
 /* Test both the direction the player is facing and the visible weapon
    span. The short south-facing projection still lowers at a wall. */
 return !pulse_path(x,y,x+aim.co*(forward+pad),y+aim.si*(forward+pad),room)||
        !pulse_path(x,y,aim.mx+aim.co*pad,aim.my+aim.si*pad,room);
}
static void pulse_guard_tick(int view,float x,float y,float a,PulseRoomSolid room){
 if(!pistol_equipped()||p.car>=0||p.dead){pulse_guard_reset();return;}
 if(pulseGuardView!=view){pulseGuardView=view;pulseGuardTicks=0;}
 /* A fixed probe doesn't change with each stride or recoil. Use a small
    clearance margin and a brief hold so the sprite doesn't flicker at edges. */
 PulsePose aim=pulse_aim(x,y,a,0,0);
 if(pulse_cover(x,y,aim,pulseGuardTicks?2.75f:1.f,room)||
    (walking&&pulse_cover(x,y,pulse_aim(x,y,a,player_gait(),0),.6f,room)))pulseGuardTicks=PULSE_CLEAR_TICKS;
 else if(pulseGuardTicks>0)pulseGuardTicks--;
}
static PulsePose pulse_carry(float x,float y,PulsePose aim,int stride,int view,PulseRoomSolid room){
 int side=aim.co>.24f?-1:aim.co<-.24f?1:aim.si<0?1:-1;
 float sway=.23f*stride;
 /* Point the barrel down along an outside hip, choosing the open side.
    If both sides are tight use the higher compact carry at the shoulder. */
 float rise=pulseGuardView==view&&pulseGuardTicks>0&&pulseGuardTicks<PULSE_CLEAR_TICKS
           ?.5f*(PULSE_CLEAR_TICKS-pulseGuardTicks):0;
 float mx=x+side*9.5f,my=y+4.0f+sway-rise;
 if(!pulse_path(x,y,mx+side*1.5f,my+1.f,room)){
  if(pulse_path(x,y,x-side*11.f,my+1.f,room)){side=-side;mx=x+side*9.5f;}
  else{
   my=y-3.f+sway-rise;
   if(!pulse_path(x,y,x+side*10.f,my+1.f,room)&&pulse_path(x,y,x-side*10.f,my+1.f,room))side=-side;
   mx=x+side*8.5f;
  }
 }
 int hand=aim.si>.54f?-side:aim.si<-.54f?side:aim.co>=0?1:-1;
 return (PulsePose){mx,my,side*.53f,.848f,0,hand};
}
static PulsePose pulse_with_cover(float x,float y,float a,int stride,float recoil,int view,PulseRoomSolid room){
 PulsePose aim=pulse_aim(x,y,a,stride,recoil);
 if((pulseGuardView==view&&pulseGuardTicks)||
   pulse_cover(x,y,pulse_aim(x,y,a,0,0),1.f,room)||pulse_cover(x,y,aim,.6f,room))
  return pulse_carry(x,y,aim,stride,view,room);
 return aim;
}
static PulsePose pulse_pose(float recoil){
 PulsePose aim=pulse_with_cover(p.x,p.y,p.a,player_gait(),recoil,1,0);
 if(aim.hand)return aim;
 float co=aim.co,si=aim.si,dx=aim.mx-p.x,dy=aim.my-p.y,forward=aim.extent,scale=1;
 /* Point-blank actors aren't walls: keep the original close-hit cone.
    Tighten the extension only enough not to draw or shoot past one. */
 for(int i=0;i<LEN(peds);i++)if(peds[i].hp>0&&!npc_replaces(N_ACT_STREET+i)){
  float x=peds[i].x-p.x,y=peds[i].y-p.y;
  if(d2(x,y)>36.f*36.f)continue;
  float d=x*co+y*si;
  if(d>0&&d<forward&&absf(y*co-x*si)<13.f+d*.12f&&
     street_reach_open(p.x,p.y,peds[i].x,peds[i].y,1,-1))
   scale=pulse_min(scale,(d>2.f?d-2.f:d*.35f)/forward);
 }
 float targetX,targetY;
 if(npc_fighter_position(&targetX,&targetY)){
  float x=targetX-p.x,y=targetY-p.y,d=x*co+y*si;
  if(d>0&&d<forward&&absf(y*co-x*si)<13.f+d*.12f&&pulse_path(p.x,p.y,targetX,targetY,0))
   scale=pulse_min(scale,(d>2.f?d-2.f:d*.35f)/forward);
 }
 if(scale==1)return aim;
 /* Keep a north-facing grip beside the head during a short shot too.
    Restoring the lateral component doesn't add any forward reach. Check
    the new direct muzzle path (a corner can differ from the full pose). */
 float side=(1.f-scale)*pulse_shoulder(co,si);
 PulsePose tucked={p.x+dx*scale-si*side,p.y+dy*scale+co*side,co,si,forward*scale,0};
 if(side&&!pulse_path(p.x,p.y,tucked.mx,tucked.my,0))
  return pulse_carry(p.x,p.y,aim,player_gait(),1,0);
 return tucked;
}
static PulsePose pulse_visible_pose(void){
 /* Same-tick movement of other actors cannot detach the first flash
    from its recorded firing pose. In later frames honor current cover. */
 if(p.trail>.102f&&p.x==firedX&&p.y==firedY&&p.a==firedA)return firedPulse;
 return pulse_pose(p.trail>.085f?2.5f:p.trail>.054f?1.f:0.f);
}
static PulsePose pulse_room_pose(float x,float y,float a,int stride,PulseRoomSolid room){
 return pulse_with_cover(x,y,a,stride,0,scene,room);
}
