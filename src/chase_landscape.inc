/* Greyreed & promenade: all roots and footprints come from the baked map.
 * Meshes never enter collision or consume the simulation RNG. Covers and
 * foliage keep the same outline at every bearing; no camera-facing steps. */
static float wl_sat(float t){return t<0?0:t>1?1:t;}
static float wl_visible(float x,float y,float z,float radius,float far){
 ChVec v=ch_local((ChVec){x,y,z});
 if(v.z<9-radius||v.z>far+radius||absf(v.x)>v.z*(W*.5f/CH_F)+radius*2||
    v.y>v.z*(CH_MY/(float)CH_F)+radius*2||-v.y>v.z*((H-CH_MY)/(float)CH_F)+radius*2)return -9999.f;
 return v.z;
}
static void wl_tri(ChVec a,ChVec b,ChVec c,U ink){ChVec t[]={a,b,c};ch_face(t,3,ink,1);}
static void wl_box(float l,float t,float r,float b,float floor,float top,U wall,U lid){
 ChVec a={l,t,floor},u={r,t,floor},v={r,b,floor},w={l,b,floor};
 ChVec A={l,t,top},U0={r,t,top},V={r,b,top},W0={l,b,top};
 if(ch.y<t)ch_quad(a,u,U0,A,mix(wall,0x243442,1),1);
 if(ch.x>r)ch_quad(u,v,V,U0,mix(wall,0x1e2b36,2),1);
 if(ch.y>b)ch_quad(v,w,W0,V,wall,1);
 if(ch.x<l)ch_quad(w,a,A,W0,mix(wall,0xd1bf92,1),1);
 if(ch.h>=top)ch_quad(A,U0,V,W0,lid,1);
 else if(ch.h<floor)ch_quad(a,u,v,w,mix(wall,0x233039,3),1);
}
/* Silhouette segments (hexagonal skirt and peaked cap). Facet values are
 * derived from a fixed world light, not the viewing azimuth. */
typedef struct {float dx[6],dy[6];} WlRing;
static WlRing wl_ring(unsigned seed){
 static const float ux[]={1,.5f,-.5f,-1,-.5f,.5f},uy[]={0,.866f,.866f,0,-.866f,-.866f};
 static const float cr[]={1,.966f,.866f,.707f,.5f,.259f},sr[]={0,.259f,.5f,.707f,.866f,.966f};
 int ori=(int)(seed%6);WlRing ring;
 for(int i=0;i<6;i++){float k=.93f+((seed>>((i*3)%16))&3)*.04f;
  ring.dx[i]=k*(cr[ori]*ux[i]-sr[ori]*uy[i]);ring.dy[i]=k*(sr[ori]*ux[i]+cr[ori]*uy[i]);}
 return ring;
}
static void wl_crown(const WlRing*f,float x,float y,float z,float width,float yscale,float rise,int rounded,U color){
 ChVec p0[6],q[6],peak={x+.07f*width,y-.09f*width,z+rise};
 for(int i=0;i<6;i++){
  float dx=f->dx[i]*width,dy=f->dy[i]*width*yscale;
  p0[i]=(ChVec){x+dx,y+dy,z+(i%3-1)*.09f*width};
  q[i]=(ChVec){x+.70f*dx,y+.70f*dy,z+rise*.67f};
 }
 for(int i=0;i<6;i++){
  int j=(i+1)%6;
  float nx=f->dx[i]+f->dx[j],ny=f->dy[i]+f->dy[j];
  U panel=nw_blend(color,0x192d2d,cl(52+(int)(nx*19+ny*36),0,132));
  if(rounded){
   if((ch.x-x)*nx+(ch.y-y)*ny>=-.35f*width)ch_quad(p0[i],p0[j],q[j],q[i],panel,1);
   wl_tri(q[i],q[j],peak,nw_blend(color,0xc0c18a,cl(30-(int)(nx*7+ny*8),0,74)));
  }else wl_tri(p0[i],p0[j],peak,panel);
  if(ch.h<z+.10f*width)wl_tri(p0[j],p0[i],(ChVec){x,y,z-width*.115f},mix(color,0x1d302d,3));
 }
}
static const U wl_spruce[]={0x426954,0x537963,0x496f5a,0x52766c,0x3f6659,0x55745d};
static const U wl_broad[]={0x638362,0x839060,0x65816d,0x788c58,0x6d8860,0x6b8878};
static void wl_tree(float x,float y,float h,unsigned seed,int broad,int town,float dist){
 float far=wl_sat((910.f-dist)/120.f);
 if(far<=.012f)return;
 /* A crown immediately between a foot camera and the walker can otherwise
  * fill the entire image. Contract it smoothly, never hide the trunk. */
 float prox=wl_sat((sq(d2(x-ch.x,y-ch.y))-5.f)/25.f);
 float w=h*(broad?.34f:.29f)*far*(.16f+.84f*prox);
 float d=((int)(seed%5)-2)*.025f*h;
 WlRing f=wl_ring(seed);
 U c=broad?wl_broad[(seed>>5)%6]:wl_spruce[(seed>>4)%6];
 if(town)c=nw_blend(c,0x647e70,46);
 float stem=h*(town?.048f:.044f)*far;
 wl_box(x-stem,y-stem,x+stem,y+stem,0,h*(broad?.55f:.53f),broad?0x75634d:0x615243,0x957852);
 if(dist<265.f){
  /* A fork shows between the foliage and the bare trunk at street level.
   * Branches attach to the same world offsets in every view. */
  float leg=wl_sat((265.f-dist)/75.f)*w;
  for(int j=0;j<2;j++){
   int n=(j*3+(seed%3))%6;float dx=f.dx[n]*leg*.71f,dy=f.dy[n]*leg*.71f;
   float z0=h*(broad?.29f:.37f),z1=z0+h*(broad?.23f:.12f);
   ch_quad((ChVec){x-.62f,y,z0},(ChVec){x+.62f,y,z0},
           (ChVec){x+dx+.31f,y+dy,z1},(ChVec){x+dx-.31f,y+dy,z1},0x6d5b43,1);
   ch_quad((ChVec){x,y-.65f,z0},(ChVec){x,y+.65f,z0},
           (ChVec){x+dx,y+dy+.32f,z1},(ChVec){x+dx,y+dy-.32f,z1},0x79644d,1);
  }
 }
 if(broad){
  /* Distinct fork and a smaller off-axis crown: not a stack of discs. */
  wl_crown(&f,x-d*.65f-w*.26f,y+w*.13f,h*.40f,w*.94f,.91f,h*.36f,1,mix(c,0x334c3d,1));
  wl_crown(&f,x+d+w*.24f,y-w*.13f,h*.50f,w,.85f,h*.41f,1,c);
  /* One outward-facing limb breaks the spherical silhouette; it grows
   * continuously out of the primary crown rather than switching LOD. */
  if(dist<465.f){
   float k=wl_sat((465.f-dist)/120.f),dx=f.dx[1]*w,dy=f.dy[1]*w;
   wl_crown(&f,x+dx*.73f,y+dy*.73f,h*.47f,w*k*.48f+.05f,.89f,h*.23f*k,1,mix(c,0x869b71,1));
  }
 }else{
  wl_crown(&f,x,y,h*.25f,w,1,h*.41f,0,mix(c,0x244137,1));
  wl_crown(&f,x+d*.4f,y,h*.47f,w*.79f,.94f,h*.36f,0,c);
  wl_crown(&f,x+d,y,h*.66f,w*.51f,1.03f,h*.34f,0,mix(c,0x9aac85,1));
 }
}
static void wl_city_trees(void){
 for(int i=0;i<LEN(scenery);i++)for(int j=scenery[i].treesFrom;j<scenery[i].count;j++){
  const StreetBounds*t=&scenery[i].walls[j];float x=t->x+t->w*.5f,y=t->y+t->h*.5f;
  unsigned g=outer_noise((int)x,(int)y);float h=29+(g%9);
  float depth=wl_visible(x,y,h*.5f,h*.66f,910);if(depth<-9900.f)continue;
  wl_tree(x,y,h,g,g%5!=0,1,depth);
 }
}
static void wl_brush(float x,float y,float h,unsigned seed,float d){
 float lod=wl_sat((235-d)*.015f);
 if(lod<.04f)return;
 static const float u[]={-.86f,.38f,.89f,-.42f,.05f},v[]={-.43f,-.91f,.41f,.82f,.15f};
 float k=h*lod;U leaf=(seed&1)?0x678766:0x5c7954;float lean=sn(anim*1.3f+(seed&63)*.1f)*.7f*lod;
 for(int n=0;n<5;n++){
  float dx=u[n]*k,dy=v[n]*k,s=.065f*k;
  ChVec base={x,y,.15f},a={x+.58f*dx-v[n]*s,y+.58f*dy+u[n]*s,k*(n==4?.60f:.43f)},
      tip={x+.95f*dx+lean,y+.95f*dy,k*(n==4?.88f:.56f)},b={x+.48f*dx+v[n]*s,y+.48f*dy-u[n]*s,k*.38f};
  ch_quad(base,b,tip,a,n&1?leaf:mix(leaf,0xacac71,2),1);
 }
}
static void wl_wood(void){
 for(int j=0;j<woodPropN;j++){
  const WoodProp*q=&woodProps[j];float x=q->x,y=q->y,h=q->height,r=q->radius;
  float limit=q->kind==WOOD_UNDERGROWTH?235.f:q->kind==WOOD_TREE?910.f:660.f;
  float d=wl_visible(x,y,h*.45f,r+h*.75f,limit);if(d<-9900.f)continue;
  unsigned g=outer_noise(q->x,q->y);WlRing f;
  switch(q->kind){
   case WOOD_TREE: wl_tree(x,y,h,g,q->grade%3==1,0,d);break;
   case WOOD_UNDERGROWTH:wl_brush(x,y,h,g,d);break;
   case WOOD_ROCK:{
    f=wl_ring(g);U granite=q->grade%3?0x7a867b:0x849282;
    wl_crown(&f,x,y,r*.12f,r*1.12f,.78f,h*.67f,1,granite);
    if(d<230&&q->x>2000)wl_brush(x-r*.72f,y-r*.31f,7.f,g,d);break;}
   case WOOD_STUMP:{
    f=wl_ring(g);wl_box(x-r*.7f,y-r*.7f,x+r*.7f,y+r*.7f,0,h*.78f,0x695441,0xaa8c64);
    wl_crown(&f,x,y,h*.78f,r*.48f,.92f,.45f,0,0x90734e);break;}
   case WOOD_LOGPROP:
    wl_box(x-r,y-r*.42f,x+r,y+r*.42f,0,h*.65f,0x63523e,0x826b4a);
    wl_box(x+r-.6f,y-r*.37f,x+r+.1f,y+r*.37f,.6f,h*.60f,0xb99b6d,0xa4875c);break;
   case WOOD_POST:
    wl_box(x-.8f,y-.8f,x+.8f,y+.8f,0,19,0x786549,0xb19b73);
    wl_box(x-5.4f,y-.95f,x+5.4f,y+.95f,14,19,0xb29a65,0xc6b480);
    wl_box(x-2.1f,y-1.04f,x+1.9f,y+1.04f,16,17,0x4b6751,0x4b6751);break;
   case WOOD_CRATE:
    wl_box(x-r,y-r,x+r,y+r,0,h,0x786647,0x9a865d);
    for(int i=-1;i<=1;i+=2)wl_box(x+i*r*.64f-1,y-r-.12f,x+i*r*.64f+1,y+r+.12f,0,h+.12f,0x44594f,0x56675a);break;
   case WOOD_HUT:{
    float l=x-r,k=x+r,t=y-r*.82f,b=y+r*.82f;
    wl_box(l,t,k,b,0,17,0x706752,0x706752);
    ch_quad((ChVec){l-2,t-2,17},(ChVec){k+2,t-2,17},(ChVec){k+2,y,29},(ChVec){l-2,y,29},0x526959,1);
    ch_quad((ChVec){l-2,y,29},(ChVec){k+2,y,29},(ChVec){k+2,b+2,17},(ChVec){l-2,b+2,17},0x43594e,1);
    for(int side=-1;side<=1;side+=2){float u=x+side*r;wl_tri((ChVec){u,t,17},(ChVec){u,b,17},(ChVec){u,y,29},0x827458);}
    wl_box(x+2,t-.2f,x+8,b+.2f,0,13,0x3c5148,0x96876b);
    wl_box(l+3,t-.3f,l+10,b+.3f,7,13,0x9ab3a1,0xb1a58a);
    if(d<330){
     /* Sills / split glazing and a ridge cap keep the little ranger shed
      * readable at foot level rather than looking like another green box. */
     wl_box(l+2.8f,t-.47f,l+10.2f,b+.47f,6.65f,7.23f,0xa59065,0xbaab7e);
     wl_box(l+6.2f,t-.35f,l+6.9f,b+.35f,7,13,0x766952,0x766952);
     wl_box(l-2,y-.35f,k+2,y+.35f,28.6f,29.2f,0x7f8d72,0x899780);
     for(int n=0;n<3;n++)wl_box(l-.13f,t+2,l+.12f,b-2,3.0f+n*4.1f,3.35f+n*4.1f,0x645e4d,0x716957);
     wl_box(x+6.35f,t-.36f,x+6.9f,b+.36f,5.75f,6.6f,0xc4a473,0xc4a473);
    }
    break;}
  }
 }
}

/* Waist-low coast scrub is passable set dressing on surveyed, dry and clear
 * terrain only. Thin the clumps with a fixed windward stripe. Never spawn a
 * fake obstacle or random entity when a camera crosses the district edge. */
static void wl_shore_grass(void){
 if(ch.x<1720&&ch.y<1810)return;
 int l=cl((int)(ch.x-250)/34,0,ROOM/34),r=cl((int)(ch.x+250)/34,0,ROOM/34);
 int t=cl((int)(ch.y-250)/34,0,ROOM/34),b=cl((int)(ch.y+250)/34,0,ROOM/34);
 for(int gy=t;gy<=b;gy++)for(int gx=l;gx<=r;gx++){
  U seed=outer_noise(gx+977,gy+483);if((seed&7)>4)continue;
  int x=gx*34+3+(int)((seed>>4)%27),y=gy*34+3+(int)((seed>>12)%27);
  if((U)x>=ROOM-9||(U)y>=ROOM-9||!outer_is_new(x,y))continue;
  int i=y*ROOM+x,land=outerLand[i];
  if((land!=TILE_DUNE&&land!=TILE_BLUSH)||outerLane[i]||roofMask[i])continue;
  float d=wl_visible(x,y,4,11,230);if(d<-9900)continue;
  wl_brush(x,y,4.3f+((seed>>19)%4),seed,d);
  if(!outerLane[i+4*ROOM+4]&&outerLand[i+4*ROOM+4]==land)
   wl_brush(x+4,y+4,3.5f+((seed>>22)%3),seed>>2,d);
 }
}
