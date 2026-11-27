/* Render-only pose adapters. Live peds, held/rejoining positions, taxi riders,
   and the exterior lane snapshots keep their original simulation owners. */
typedef struct {float x,y,a;int step,role,hue;U coat;} StreetPerson;
static int street_night_rider_pose(StreetPerson*,int*);
static int street_ped_pose(int i,StreetPerson*out){
 const Ped*q=peds+i;int id=N_ACT_STREET+i;
 if(q->hp<=0||npc_replaces(id))return 0;
 *out=(StreetPerson){q->x,q->y,q->a,q->s<=2?0:(int)(sn(anim*FOOT_PACE*(q->role?16.f:11.f)+i)*4),q->role,i,
   pedHit[i]>0?0xffdfbc:q->role==MODEL_OFFICER?(dispatch_posted(i)?0x668df0:(((int)(anim*9)&1)?0x668df0:PINK)):
      q->role==MODEL_RIVAL?PINK:(i%2)?0x92b7b4:0xca8c71};
 if(npc_present_pose(id,&out->x,&out->y,&out->a,&out->step)&&q->role==MODEL_OFFICER)out->coat=0x668df0;
 return 1;
}
static int street_rider_pose(StreetPerson*out){
 if(!((job.kind==JOB_TAXI&&taxi.phase!=TAXI_RIDING)||(taxi.phase==TAXI_WALKING&&taxi.linger>0))||
    (U)taxi.rider>=(U)LEN(riders)||npc_replaces(N_ACT_RIDER+taxi.rider))return 0;
 *out=(StreetPerson){taxi.x,taxi.y,taxi.a,taxi.phase==TAXI_WAIT?0:(int)(sn(anim*(15.f*FOOT_PACE))*5),
                    0,riders[taxi.rider].face,riders[taxi.rider].outfit};
 npc_present_pose(N_ACT_RIDER+taxi.rider,&out->x,&out->y,&out->a,&out->step);
 return 1;
}
static int street_outer_pose(int i,StreetPerson*out){
 *out=(StreetPerson){.hue=i+13};
 return outer_walk_pose(i,&out->x,&out->y,&out->a,&out->step,&out->coat);
}
enum {SP_SIDE=34,SP_X=17,SP_Y=23,SP_OUT_MAX=24};
static U streetPersonSprite[SP_SIDE*SP_SIDE];
typedef struct {int left,right,top,bottom;const Building*b;} StreetPersonClip;
/* Do not erase neighboring bodies by restamping an unlit rectangular
   building region. Clip only pixels hidden behind a real nearer facade.
   Include adjacent tiles on both sides to handle block seams. */
static int street_person_clips(int x,int y,StreetPersonClip*out){
 int n=0;
 for(int by=cl((y-SP_Y)/256,0,7);by<=cl((y+SP_SIDE-SP_Y)/256,0,7);by++)
 for(int bx=cl((x-SP_X)/256,0,7);bx<=cl((x+SP_SIDE-SP_X)/256,0,7);bx++){
  const int*r=lot[by*8+bx];
  for(int j=1;j<=r[0];j++){
   const Building*b=buildings+r[j];StreetBounds f=building_bounds(b);
   if(y>=b->y+b->h||x+SP_SIDE-SP_X<f.x||x-SP_X>=f.x+f.w||
      y+SP_SIDE-SP_Y<f.y||y-SP_Y>=f.y+f.h)continue;
   if(n<8)out[n++]=(StreetPersonClip){f.x,f.x+f.w,f.y,f.y+f.h,b};
  }
 }
 return n;
}
static int street_person_visible(int x,int y,const StreetPersonClip*cuts,int n){
 for(int i=0;i<n;i++){
  const StreetPersonClip*f=cuts+i;const Building*b=f->b;
  if(x>=f->left&&x<f->right&&y>=f->top&&y<f->bottom&&
     x<b->x+b->w+cl((y-(b->y-b->ht))/2,0,b->ht/2))return 0;
 }
 return 1;
}
static void street_person_shadow(int x,int y){
 /* Dress the ground at the existing boot anchor, never with a flat human
    stamp. Feathered 14x6 contact oval reads on wet venues and pale paving. */
 for(int v=-2;v<=3;v++)for(int u=-7;u<=7;u++){
  int xx=x+u,yy=y+v+2,dx=xx-cx,dy=yy-cy;
  if((U)dx>=W||(U)dy>=H||(U)xx>=ROOM||(U)yy>=ROOM||roofMask[yy*ROOM+xx])continue;
  int r=u*u+4*v*v;if(r>54)continue;
  U*dst=pixels+dy*W+dx;int amount=3-(r>24)-(r>41);
  *dst=mix(*dst,0x101824,amount);
 }
}
static void street_person_draw(const StreetPerson*a){
 const int x=(int)a->x,y=(int)a->y,dx=x-cx-SP_X,dy=y-cy-SP_Y;
 if(dx>=W||dy>=H||dx+SP_SIDE<=0||dy+SP_SIDE<=0)return;
 StreetPersonClip cuts[8];int n=street_person_clips(x,y,cuts);
 /* Render the existing articulated sprite exactly at its natural world
    scale. This temporary layer is only for depth and local light. */
 U*old=surface;int ow=rw,oh=rh,l=clipLeft,r=clipRight,t=clipTop,b=clipBottom,skip=streetPersonNoShadow;
 for(int i=0;i<LEN(streetPersonSprite);i++)streetPersonSprite[i]=0xff000000u;
 surface=streetPersonSprite;rw=rh=clipRight=clipBottom=SP_SIDE;clipLeft=clipTop=0;
 streetPersonNoShadow=1;
 person(SP_X,SP_Y,a->coat,a->step,a->a,a->role,a->hue);
 streetPersonNoShadow=skip;surface=old;rw=ow;rh=oh;clipLeft=l;clipRight=r;clipTop=t;clipBottom=b;
 int tint=cl((int)(nw_dark()*53.f),0,58);
 /* Lens potential at the feet is shared across the whole upright body;
    no artificial "match the closest asphalt pixel" flattening. */
 U lamp=headlamps_ground(0x131b27,x,y);
 int glow=cl((int)(lamp&255)-0x27,0,88);
 for(int v=max(0,-dy);v<min(SP_SIDE,H-dy);v++)for(int u=max(0,-dx);u<min(SP_SIDE,W-dx);u++){
  U ink=streetPersonSprite[v*SP_SIDE+u];if(ink>>24)continue;
  if(!street_person_visible(x+u-SP_X,y+v-SP_Y,cuts,n))continue;
  if(tint)ink=nw_blend(ink,0x252d47,tint);
  if(glow)ink=nw_blend(ink,lamp,glow);
  pixels[(dy+v)*W+dx+u]=ink;
 }
}
/* Existing defeated state stays low and non-graphic, but a distinct coat,
   hands and two boots identify an actual resting pose, not a map stripe. */
static void street_person_downed(int x,int y,int i){
 const Ped*q=peds+i;float co=cs(q->a),si=sn(q->a);
 U coat=q->role==MODEL_OFFICER?0x607aa2:q->role==MODEL_RIVAL?0xa3718b:i&1?0x6f9a96:0xa47564;
 U trim=mix(coat,0x263047,3),skin=0xbe9b88;
 Pt head=rig(x,y,co,si,-6,1,2),left=rig(x,y,co,si,-2,-4,1),right=rig(x,y,co,si,0,3,1);
 Pt hips=rig(x,y,co,si,3,0,1),shoulder=rig(x,y,co,si,-3,0,1);
 rod(hips,rig(x,y,co,si,9,3,0),1,0x435469);
 rod(hips,rig(x,y,co,si,7,-3,0),1,0x394a62);
 disc(rig(x,y,co,si,9,3,0).x,rig(x,y,co,si,9,3,0).y,1,0x1a2437);
 rod(shoulder,hips,2,trim);rod(shoulder,hips,1,coat);
 rod(shoulder,left,1,trim);rod(shoulder,right,1,coat);dot(left.x,left.y,skin);dot(right.x,right.y,skin);
 disc(head.x,head.y,2,skin);dot(head.x-1,head.y-1,0x343044);
}
