/* Walkable interiors use the street's wardrobe and articulated meshes.
 * Four subpixel samples are confined to an actor's screen rectangle, never
 * an enlarged sprite or supersampled full frame. The existing room camera
 * and reciprocal depth keep fixtures, raised stages and characters aligned. */
enum {IC_W=256,IC_H=288};
typedef struct {float x,y,z;U c;} IcVertex;
typedef struct {int z;U c;} IcSample;
static IcSample icBuf[IC_W*IC_H*4];
static struct {int l,t,w,h,which,samples;float lightX,lightY,floor;U lamp;} ic;
static int ic_camera(float x,float y,float z,IcVertex*v){
 float b=ir.y-y,h=ir.h-z,depth=b*IR_C+h*IR_S;
 if(depth<8)return 0;
 float f=IR_F/depth;
 *v=(IcVertex){W*.5f+(x-ir.x)*f,ir.mid+(h*IR_C-b*IR_S)*f,IR_RES/depth,0};return 1;
}
static float ic_edge(IcVertex a,IcVertex b,float x,float y){return (b.x-a.x)*(y-a.y)-(b.y-a.y)*(x-a.x);}
static void ic_tri(IcVertex a,IcVertex b,IcVertex c){
 float area=ic_edge(a,b,c.x,c.y);if(absf(area)<.025f)return;
 if(area<0){IcVertex k=b;b=c;c=k;area=-area;}
 float minX=a.x,maxX=a.x,minY=a.y,maxY=a.y;
 #define IC_BOUNDS(q) do{if(q.x<minX)minX=q.x;if(q.x>maxX)maxX=q.x;if(q.y<minY)minY=q.y;if(q.y>maxY)maxY=q.y;}while(0)
 IC_BOUNDS(b);IC_BOUNDS(c);
 #undef IC_BOUNDS
 int l=max(ic.l,(int)minX),rr=min(ic.l+ic.w-1,(int)maxX),top=max(ic.t,(int)minY),bt=min(ic.t+ic.h-1,(int)maxY);
 if(l>rr||top>bt)return;
 float inv=1.f/area;
 float dx0=b.y-c.y,dx1=c.y-a.y,dx2=a.y-b.y,dy0=c.x-b.x,dy1=a.x-c.x,dy2=b.x-a.x;
 float bias=ic.samples==1?.5f:.25f;
 float xx=l+bias,yy=top+bias,p0=ic_edge(b,c,xx,yy),p1=ic_edge(c,a,xx,yy),p2=ic_edge(a,b,xx,yy);
 int ar=a.c>>16,ag=(a.c>>8)&255,ab=a.c&255,br=b.c>>16,bg=(b.c>>8)&255,bb=b.c&255,cr=c.c>>16,cg=(c.c>>8)&255,cb=c.c&255;
 /* Reciprocal depth, RGB planes and sample offsets are affine after
  * projection. Compute gradients once, not three weighted sums per pixel. */
 float dR=(ar*dx0+br*dx1+cr*dx2)*inv,eR=(ar*dy0+br*dy1+cr*dy2)*inv,fR=(ar*p0+br*p1+cr*p2)*inv;
 float dG=(ag*dx0+bg*dx1+cg*dx2)*inv,eG=(ag*dy0+bg*dy1+cg*dy2)*inv,fG=(ag*p0+bg*p1+cg*p2)*inv;
 float dB=(ab*dx0+bb*dx1+cb*dx2)*inv,eB=(ab*dy0+bb*dy1+cb*dy2)*inv,fB=(ab*p0+bb*p1+cb*p2)*inv;
 float dZ=(a.z*dx0+b.z*dx1+c.z*dx2)*inv,eZ=(a.z*dy0+b.z*dy1+c.z*dy2)*inv,fZ=(a.z*p0+b.z*p1+c.z*p2)*inv;
 float ss0[4],ss1[4],ss2[4],ssZ[4],ssR[4],ssG[4],ssB[4];
 for(int i=0;i<ic.samples;i++){
  int sample=ic.samples==2?i*3:i;float sx=(sample&1)*.5f,sy=(sample>>1)*.5f;
  ss0[i]=dx0*sx+dy0*sy;ss1[i]=dx1*sx+dy1*sy;ss2[i]=dx2*sx+dy2*sy;
  ssZ[i]=dZ*sx+eZ*sy;ssR[i]=dR*sx+eR*sy+.5f;ssG[i]=dG*sx+eG*sy+.5f;ssB[i]=dB*sx+eB*sy+.5f;
 }
 for(int y=top;y<=bt;y++,p0+=dy0,p1+=dy1,p2+=dy2,fZ+=eZ,fR+=eR,fG+=eG,fB+=eB){
  float e0=p0,e1=p1,e2=p2,r=fR,g=fG,blue=fB,z=fZ;
  IcSample*dst=icBuf+4*((y-ic.t)*ic.w+l-ic.l);
  for(int x=l;x<=rr;x++,dst+=4,e0+=dx0,e1+=dx1,e2+=dx2,z+=dZ,r+=dR,g+=dG,blue+=dB){
   int base=chZ[y*W+x];
   for(int i=0;i<ic.samples;i++){
    if(e0+ss0[i]<0||e1+ss1[i]<0||e2+ss2[i]<0)continue;
    int depth=(int)(z+ssZ[i]);if(depth<base||depth<dst[i].z)continue;
    dst[i]=(IcSample){depth,RGB(cl((int)(r+ssR[i]),0,255),cl((int)(g+ssG[i]),0,255),cl((int)(blue+ssB[i]),0,255))};
   }
  }
 }
}
static void ic_face(const ChVec*q,int n,U ink){
 IcVertex v[5];float ax=q[1].x-q[0].x,ay=q[1].y-q[0].y,az=q[1].z-q[0].z;
 float bx=q[2].x-q[0].x,by=q[2].y-q[0].y,bz=q[2].z-q[0].z;
 float nx=ay*bz-az*by,ny=az*bx-ax*bz,nz=ax*by-ay*bx,norm=sq(nx*nx+ny*ny+nz*nz);
 if(norm<.001f)return;
 if(nx*(ir.x-q[0].x)+ny*(ir.y-q[0].y)+nz*(ir.h-q[0].z)<0)norm=-norm;
 float nd=(nx*ic.lightX+ny*ic.lightY+nz*.81f)/norm;
 for(int i=0;i<n;i++){
  if(!ic_camera(q[i].x,q[i].y,q[i].z,v+i))return;
  float h=q[i].z-ic.floor;
  U shaded=nw_blend(ink,ic.which==10?0x213247:0x2c2637,cl((int)(37-nd*27-h*.19f),0,100));
  v[i].c=nw_blend(shaded,ic.lamp,cl((int)(11+nd*12+h*.67f),0,58));
 }
 for(int i=1;i+1<n;i++)ic_tri(v[0],v[i],v[i+1]);
}
static void ic_shadow(float x,float y,float z,float a,float scale){
 IrPoint anchor=ir_proj(x,y,z+.10f);if(!anchor.iz)return;
 float u=cs(a),v=sn(a),rx=3.43f*scale,ry=2.35f*scale,reach=(rx+ry)*anchor.scale;
 int l=max(0,(int)(anchor.x-reach)),r=min(W-1,(int)(anchor.x+reach));
 int t=max(ir.top,(int)(anchor.y-reach)),bt=min(ir.end-1,(int)(anchor.y+reach));
 for(int j=t;j<=bt;j++){
  float ray=(j+.5f-ir.mid)/IR_F,k=IR_S+ray*IR_C;if(k<=.001f)continue;
  float d=(ir.h-z-.10f)/k,wy=ir.y-d*(IR_C-ray*IR_S)-y,step=d/IR_F;
  int zi=(int)(IR_RES/d);float wx=ir.x+(l+.5f-W*.5f)*step-x;
  for(int i=l,pix=j*W+l;i<=r;i++,pix++,wx+=step){
   if(chZ[pix]>zi+4)continue;
   float f=(wx*u+wy*v)/rx,g=(wy*u-wx*v)/ry,p=f*f+g*g;
   if(p<1)pixels[pix]=nw_blend(pixels[pix],0x131422,(int)((1-p)*109));
  }
 }
}
static int ic_start(float x,float y,float z,float scale,int which){
 IrPoint foot=ir_proj(x,y,z),head=ir_proj(x,y,z+16.8f*scale);if(!foot.iz||!head.iz)return 0;
 float span=head.scale*scale;
 int half=(int)(span*10.5f)+5,feet=(int)(span*7.f)+4,cap=(int)(span*6.0f)+4;
 int l=max(0,min(foot.x,head.x)-half),r=min(W,max(foot.x,head.x)+half),t=max(ir.top,min(head.y,foot.y)-cap),b=min(ir.end,max(foot.y,head.y)+feet);
 if(l>=r||t>=b||r-l>IC_W||b-t>IC_H)return 0;
 ic.l=l;ic.t=t;ic.w=r-l;ic.h=b-t;ic.floor=z;ic.which=which;ic.samples=4;
 ic.lamp=which==7?0xe0a7eb:which==10?0x8ce2ec:which==11?0xfcd7ad:0xffebce;
 ic.lightX=-.37f;ic.lightY=.42f;
 for(int i=0;i<ic.w*ic.h*4;i++)icBuf[i].z=0;
 ch_actor_target=ic_face;ch_actor_detail=2;return 1;
}
static void ic_finish(void){
 ch_actor_target=0;ch_actor_detail=0;
 const IcSample*src=icBuf;
 for(int y=ic.t;y<ic.t+ic.h;y++)for(int x=ic.l,w=y*W+x;x<ic.l+ic.w;x++,w++,src+=4){
  if(ic.samples==1){if(src[0].z){pixels[w]=src[0].c;chZ[w]=src[0].z;}continue;}
  int z=ic.samples==2?max(src[0].z,src[1].z):max(max(src[0].z,src[1].z),max(src[2].z,src[3].z));
  if(!z)continue;U under=pixels[w];int r=0,g=0,b=0;
  for(int i=0;i<ic.samples;i++){U c=src[i].z?src[i].c:under;r+=(c>>16)&255;g+=(c>>8)&255;b+=c&255;}
  int s=ic.samples==4?2:1;
  pixels[w]=RGB((r+s)>>s,(g+s)>>s,(b+s)>>s);chZ[w]=z;
 }
}
static void ic_body(StreetPerson*q,const Look*l,AvPose opt,int phase,float strike,int armed,int room){
 if(!ic_start(q->x,q->y,opt.floor,opt.scale,room))return;
 /* Thin silhouettes beyond the floor still use geometry; only sample
  * count and limb chamfers step down. The player never drops a tier. */
 if(!l&&phase==NPC_IDLE){
  float span=ir_proj(q->x,q->y,opt.floor+9.f*opt.scale).scale*opt.scale;
  ic.samples=span<2.54f?1:span<3.18f?2:4;
  ch_actor_detail=ic.samples==4?2:1;
 }
 ic_shadow(q->x,q->y,opt.floor,q->a,opt.scale);
 av_body(q,l,phase,strike,armed,opt.guard,opt.gait,&opt);
 ic_finish();
}
static void ir_hero(float x,float y,float a,int which){
 const Look*l=look_valid(&sv.look)?&sv.look:&defaultLook;
 int armed=(which==3||which==4||which==7||which==10)&&pistol_equipped();
 if(!armed&&which!=10){
  float u=cs(a),v=sn(a),m=17.5f;IrV left={x+u*m-v*2.3f,y+v*m+u*2.3f,.5f},right={x+u*m+v*2.3f,y+v*m-u*2.3f,.5f},tip={x+u*(m+6),y+v*(m+6),.5f};
  ir_face(left,right,tip,tip,0xf2f1df);
 }
 float gait=which==11?(nt_walking?sn(anim*12)*4.f:0):which==10?(nvs_rail_walk?sn(nvs_frames*.24f)*4.f:0):(walking?sn(footcycle)*5.f:0);
  /* Afterimage uses a suspendable show clock, including the hero's idle
  * breathing. Global anim intentionally continues while in Pause. */
 AvPose opt={.scale=1.78f,.activity=AV_STILL,.t=(which==NIGHT_CLUB_SCENE?nc_motion():anim)*4.f,.gait=gait};
 if(armed){
  PulseRoomSolid room=which==4?wi_solid:npc_room_solid();
  PulsePose g=room?npc_room_pulse(x,y,a,player_gait(),room):(PulsePose){0};
  opt.guard=g.hand;opt.recoil=npcTraceTicks>=5?.54f:0;
  opt.tuck=g.hand?0:g.extent<12.f?(12.f-g.extent)*.26f:0;
  if(opt.tuck>1.55f)opt.tuck=1.55f;
 }
 StreetPerson hero={x,y,a,0,MODEL_PLAYER,3,which!=11&&p.hit>0&&((int)(anim*20)&1)?PINK:charTop[l->part[TOP_TONE]]};
 ic_body(&hero,l,opt,NPC_IDLE,which!=11?punchFlash:0,armed,which);
}
static void ir_actor(float x,float y,float z,float a,U coat,int model,int hue,int gait){
 StreetPerson q={x,y,a,0,model,hue,coat};
 AvPose opt={.scale=1.67f,.floor=z,.activity=AV_STILL,.t=anim*3.25f+hue*.46f,.gait=gait};
 if(model==MODEL_WORKER&&ch_mode_active())opt.activity=gait?AV_STILL:AV_TEND;
 ic_body(&q,0,opt,NPC_IDLE,0,0,scene);
}
static void ir_encounter_actor(const NpcRef*r){
 int phase=npcEnc.phase;U coat=npcEnc.flash>0?0xffdfbc:r->coat;
 if(phase==NPC_DROP||phase==NPC_RECOVER){
  if(ic_start(r->x,r->y,0,1.67f,3)){av_draw_downed((ChRig){r->x,r->y,cs(r->a),sn(r->a),1.67f,0},r->coat);ic_finish();}
  if(npcEnc.prize){ir_box(r->x+5,r->y-2,7,5,.1f,2.7f,0x81a36c,0);
   ir_box(r->x+7.8f,r->y-2,1.7f,5,2.81f,.31f,0xd8ca94,0);ir_marker(r->x+8,r->y,GOLD);}
 }else{
  StreetPerson q={r->x,r->y,r->a,0,r->model,r->hue,coat};
  float t=scene==NIGHT_CLUB_SCENE?nc_motion():anim;
  AvPose opt={.scale=1.67f,.activity=AV_STILL,.t=t*3,.gait=phase==NPC_FIGHT?sn(t*18.f)*4.f:0};
  ic_body(&q,0,opt,phase,npcEnc.strike,0,3);ir_marker(r->x,r->y,phase==NPC_FIGHT?PINK:phase==NPC_YIELD?LIME:GOLD);
 }
}
static void ir_trace(void){
 if(npcTraceTicks<=0)return;
 float kick=npcTraceTicks>=5?.54f:0,extent=npcTracePose.extent;
 ChRig r={npcTracePlayerX,npcTracePlayerY,cs(npcTraceFacing),sn(npcTraceFacing),1.78f,0};
 ChVec grip=av_grip(kick,0,0);if(extent<12){float k=(12-extent)*.26f;grip.x-=k<1.55f?k:1.55f;}
 ChVec n=av_muzzle(grip,0,kick),a=ch_joint(r,n.x,n.y,n.z-.22f);
 float x=npcTraceEndX,y=npcTraceEndY,z=16.5f,dx=x-a.x,dy=y-a.y;
 int len=cl((int)(sq(d2(dx,dy))*2.5f),2,550);
 for(int i=0;i<=len;i++){float t=(float)i/len;IrPoint q=ir_proj(a.x+t*dx,a.y+t*dy,a.z+(z-a.z)*t);
  if(q.x>=0&&q.x<W&&q.y>=ir.top&&q.y<ir.end&&chZ[q.y*W+q.x]<=q.iz+7)pixels[q.y*W+q.x]=GOLD;}
 if(npcTraceTicks>=5&&ic_start(a.x,a.y,a.z-3,1.f,3)){
  ChVec base={a.x,a.y,a.z},tip=ch_joint(r,n.x+1.7f,n.y,n.z-.22f);
  ChVec q[]={base,{a.x-.74f,a.y,a.z+.62f},tip,{a.x+.61f,a.y,a.z-.42f}};
  ic_face(q,4,0xffe1a3);ic_finish();
 }
}
/* Implemented with the localized room materials and costume meshes. */
static void ic_club_cast(float x,float y,float z,const NcCast*a,int stance,float t,float heading,float mul,int stage);

static void ir_room_actors(void){
 for(int id=scene==3?N_ACT_FRONT:N_ACT_ROOM_BASE,end=scene==3?N_ACT_GROW+1:N_ACT_ROOM_END;id<end;id++){
  NpcRoomCondition*c=npc_room_condition(id);if(!c||c->down<=0)continue;
  const NpcRef*r=&c->body;if(ic_start(r->x,r->y,0,1.67f,scene)){
   av_draw_downed((ChRig){r->x,r->y,cs(r->a),sn(r->a),1.67f,0},r->coat);ic_finish();
  }
 }
 if(npcEnc.phase!=NPC_IDLE&&npcEnc.room==npc_room_key())ir_encounter_actor(&npcEnc.actor);
}
static void ir_club_actor(int id){
 RoomCast q;NpcRef r;if(!npc_club_actor_ref(id,&q,&r))return;
 NcCast skin=*q.costume;skin.cloth=r.coat;
 ic_club_cast(q.x,q.y,q.z,&skin,q.pose,q.t,r.a,q.mul*.01f,id>=NAR_STAGE&&id<NAR_DJ);
 if(id>=NAR_REGULAR&&id<NAR_DISPUTE&&ncr_near()==id-NAR_REGULAR)
  ir_label(r.x,r.y,29,r.name,id-NAR_REGULAR==NCR_PROMOTER?ncGold:ncIce,0);
}
