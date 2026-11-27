/* C-view surface material. Collision and the overhead atlas still use the
 * survey's original single terrain bake. All marks here are world-anchored;
 * no visible retile at the rectangular limit of the chart. */
static U wl_terrain_hash(unsigned x,unsigned y){
 U t=x*2246822519u+y*3266489917u+0x7ba24655u;
 return (t^(t>>14))*668265263u;
}
static inline int wl_surface_scale(int a,int b){return (a*b)>>8;}
static int wl_swell(unsigned x,unsigned y,unsigned phase){
 /* Two oblique wavefronts, broad enough to survive 960 px nearest sampling.
  * Their continuous triangle crests drift on the presentation clock only. */
 int a=(int)((x+((y>>1)+(y>>2))+(x>>3)+phase)&127)-64;
 int b=(int)((y+(x>>3)-(x>>5)+phase)&63)-32;
 int swell=15-max(a,-a);
 return max(0,swell)*2+max(0,9-max(b,-b));
}
static U wl_bluewater(int x,int y,unsigned phase,int detail){
 /* Broad wave fronts carry across the chart; gradually leave the baked
  * in-chart flecks behind rather than retile their coarse hash at its edge. */
 int tone=wl_surface_scale(wl_swell((unsigned)x,(unsigned)y,phase),detail);
 return nw_blend(0x193948,0x548392,17+tone);
}
static U wl_ocean_edge(U ink,unsigned x,unsigned y,unsigned phase,int detail){
 /* Projected coordinates were produced as signed integer floor samples.
  * Outside the chart, extend only from a real surveyed tidal edge. This
  * lets the north/east water continue while retaining the legacy dry
  * block and all in-world boundaries for the hull and walking player. */
 int sx=(int)x,sy=(int)y;
 if(sx<=-8192||sy<=-8192||sx>=12000||sy>=12000)return ink;
 int edge=min(min(sx,sy),min(ROOM-1-sx,ROOM-1-sy));
 if(edge>=96)return ink;
 int ix=cl(sx,0,ROOM-1),iy=cl(sy,0,ROOM-1),i=iy*ROOM+ix;
 if(!outer_is_new(ix,iy)||outerLand[i]!=TILE_TIDE||outerLane[i])return ink;
 U water=wl_bluewater(sx,sy,phase,detail);
 return edge<0?water:nw_blend(ink,water,cl(256-(edge*256/95),0,256));
}
static U wl_material_ink(U ink,unsigned x,unsigned y){
 const int detail=215;const unsigned wave=0;
 int i=(int)(y*ROOM+x);
 if(outerLane[i]||roofMask[i]||!outer_is_new((int)x,(int)y))return ink;
 int ground=outerLand[i]&127;
 /* Do not write to licensed harbor furniture, pier markings, or real yards. */
 if(ground==TILE_HARBOR||ground==TILE_YARD)return ink;
 unsigned grain=wl_terrain_hash(x>>2,y>>2),tuft=wl_terrain_hash(x>>3,y>>3);
 if(ground==TILE_TIDE||ground==TILE_TARN){
  U ripple=ground==TILE_TARN?0x639e96:0x61989f;
  int strength=wl_surface_scale(wl_swell(x,y,wave),detail);
  if((int)x>=8&&(int)y>=8&&x<ROOM-8u&&y<ROOM-8u){
   /* Cross the actual shore mask (including lake), never the drawn road.
    * Lean into shallows using two nested bands and copper sunlit banks. */
   if((outerLand[i-6]&127)!=ground||(outerLand[i+6]&127)!=ground||
       (outerLand[i-6*ROOM]&127)!=ground||(outerLand[i+6*ROOM]&127)!=ground){
    U shallow=ground==TILE_TARN?0x598d75:0x6eaa9d;
    int width=(outerLand[i-2]&127)!=ground||(outerLand[i+2]&127)!=ground||
             (outerLand[i-2*ROOM]&127)!=ground||(outerLand[i+2*ROOM]&127)!=ground?65:39;
    ink=nw_blend(ink,shallow,width+strength);
    if(((x+(y>>1)+wave)&31)<6)strength+=13;
   }
  }
  return nw_blend(ink,ripple,strength);
 }
 int k=(int)(grain>>27)-(int)(tuft>>29)*3;
 if(ground==TILE_TRAIL){
  /* Compacted warm center, cream granules and irregular warm leaf scatter
     make a physical footpath instead of an orange overlay. */
  unsigned run=wl_terrain_hash(x>>4,y>>4);
  int speckle=(int)((grain>>16)&15);
  if(speckle<3) return nw_blend(ink,0xb3a388,wl_surface_scale(41+speckle*8,detail));
  return nw_blend(ink,(run&1u)?0xb28b5e:0x73614a,wl_surface_scale(14+max(k,0),detail));
 }
 if(ground==TILE_WOOD){
  U target=(tuft&8u)?0x5a7352:0x334d41;
  int leaves=((tuft>>4)&31u)<3?36:12;
  if(((grain>>8)&127u)<9)target=(tuft&16u)?0x9c8052:0x80936b;
  return nw_blend(ink,target,wl_surface_scale(leaves+max(k,0),detail));
 }
 /* Coast/bluff: distinguish the sand flecks from longer fractured shale. */
 if(ground==TILE_DUNE||ground==TILE_BLUSH){
  int strata=(int)((y+(x>>2)+(tuft>>28))&31);
  U dune=ground==TILE_BLUSH?0x8f8578:0xa79775;
  return nw_blend(ink,strata<4?dune:0x5e6f59,wl_surface_scale(8+max(0,k)+(strata<4?16:0),detail));
 }
 return ink;
}

/* The clean C base arrived without roof/sprite stamps. Microdetail is
 * applied in place to its outer arms; no third atlas or per-frame re-bake. */
enum{WL_BANK=BASE_ROOM-36};
static void wl_build_terrain(void){
 for(unsigned y=0;y<ROOM;y++)for(unsigned x=y<WL_BANK?WL_BANK:0;x<ROOM;x++){
  unsigned i=y*ROOM+x;chaseFloor[i]=wl_material_ink(chaseFloor[i],x,y);
 }
 /* Felt-soft tree shadow: take a little light out of the actual material
  * below the canopy, no flat polygon, road-covering overlap or depth fight. */
 for(int j=0;j<woodPropN;j++){
  const WoodProp*p=&woodProps[j];if(p->kind!=WOOD_TREE)continue;
  float h=p->height,lx=h*.55f,ly=h*.22f,rx=h*.33f,ry=h*.21f;
  for(int y=max(WL_BANK,(int)(p->y+ly-ry));y<min(ROOM,(int)(p->y+ly+ry+1));y++){
   float v=(y-p->y-ly)/ry;
   for(int x=max(0,(int)(p->x+lx-rx));x<min(ROOM,(int)(p->x+lx+rx+1));x++){
    float u=(x-p->x-lx)/rx,k=1.f-d2(u,v);
    int i=y*ROOM+x;if(k<=0.f||(outerLand[i]&127)!=TILE_WOOD||outerLane[i])continue;
    U *s=chaseFloor+i;
    *s=nw_blend(*s,0x182e29,(int)(92*k*k));
   }
  }
 }
}
static inline U wl_terrain_sample(U ink,unsigned x,unsigned y,float depth,unsigned wave){
 if(x<ROOM&&y<ROOM)ink=chaseFloor[y*ROOM+x];
 if(x<96u||y<96u||x>=ROOM-96u||y>=ROOM-96u)
  return wl_ocean_edge(ink,x,y,wave,cl((int)(271.f-depth*.26f),35,242));
 return ink;
}
