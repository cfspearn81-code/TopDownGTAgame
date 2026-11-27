/* Actual terrain within the Outer Loop, distinct from vehicle lane ribbons.
 * Woods and other open land are walkable. The tide, tarn, and the visible
 * furniture/rocks/trunks are not. No save fields or legacy lot RNG change. */
enum{WOOD_RANGER,WOOD_SALVAGE,WOOD_FERN,WOOD_LOG,WOOD_GRANITE,WOOD_MEADOW,WOOD_RUINS,WOOD_RISE,WOOD_N_SITES};
static const OuterPoint woodline_sites[WOOD_N_SITES]={
 {560,2100},{326,2250},{182,2410},{453,2440},{760,2290},{659,2620},{322,2710},{858,2690}
};
static const char *const woodline_names[]={
 "GREYREED STATION","SURVEY CLEARING","FERN HOLLOW","OLD LOG GLADE","GRANITE FORK","MOTH MEADOW","STONE RING","EAST RISE"
};
static const char *const woodline_details[]={
 "Trailhead and ranger station. Walk south from the Seawall gate.",
 "An abandoned supply pallet at the west trail junction.",
 "Low cedar and curled ferns. The wooden trail posts continue south.",
 "A giant fallen tree marks the middle of the western wood.",
 "Exposed granite and two paths: Windmill Curve or the deep woods.",
 "The open ground south of Mirror Tarn.",
 "A stone remnant and clearing on the southern woodland loop.",
 "A ridge path above the eastern headland. Follow the trail back to the road."
};
enum{WOOD_WEST=WOOD_N_SITES,WOOD_NORTH,WOOD_BEND,WOOD_LOW,WOOD_TARN,
     WOOD_DOCK,WOOD_EAST,WOOD_MARSH,WOOD_PATH_COUNT};
static const OuterPoint woodline_path[]={
 {560,2100},{326,2250},{182,2410},{453,2440},{760,2290},{659,2620},{322,2710},{858,2690},
 {256,1995},{520,1995},{495,2252},{255,2575},{522,2500},{1034,2133},{913,2440},{516,2801}
};
static const unsigned char woodline_edges[][2]={
 {WOOD_WEST,WOOD_SALVAGE},{WOOD_NORTH,WOOD_RANGER},{WOOD_RANGER,WOOD_BEND},{WOOD_BEND,WOOD_SALVAGE},
 {WOOD_BEND,WOOD_LOG},{WOOD_SALVAGE,WOOD_FERN},{WOOD_FERN,WOOD_LOW},{WOOD_LOW,WOOD_RUINS},{WOOD_LOW,WOOD_LOG},
 {WOOD_LOG,WOOD_TARN},{WOOD_TARN,WOOD_GRANITE},{WOOD_RUINS,WOOD_MARSH},{WOOD_MARSH,WOOD_MEADOW},
 {WOOD_MEADOW,WOOD_RISE},{WOOD_RISE,WOOD_EAST},{WOOD_MEADOW,WOOD_EAST},{WOOD_EAST,WOOD_GRANITE},
 {WOOD_DOCK,WOOD_GRANITE},{WOOD_RANGER,WOOD_GRANITE}
};
enum{TILE_TIDE,TILE_BLUSH,TILE_YARD,TILE_DUNE,TILE_WOOD,TILE_TRAIL,TILE_TARN,TILE_HARBOR,TILE_BLOCK=128};
static unsigned char outerLand[ROOM*ROOM];
typedef struct{short x,y;unsigned char radius,kind,height,grade;} WoodProp;
enum{WOOD_TREE,WOOD_ROCK,WOOD_STUMP,WOOD_POST,WOOD_CRATE,WOOD_HUT,WOOD_LOGPROP,WOOD_UNDERGROWTH};
static WoodProp woodProps[1150];static int woodPropN;
static int woodline_coast_x(int y){
 static const short shore[][2]={{0,2890},{161,2975},{315,2976},{520,2977},{810,2979},
  {1045,2980},{1280,2984},{1470,2979},{1780,2980},{2008,3000},{2178,2850},
  {2307,2688},{2409,2530},{2569,2358},{2740,2074},{2930,1735},{3072,1362}};
 for(int i=1;i<LEN(shore);i++)if(y<shore[i][0])
  return shore[i-1][1]+(shore[i][1]-shore[i-1][1])*(y-shore[i-1][0])/(shore[i][0]-shore[i-1][0]);
 return 1362;
}
static int woodline_shore_y(int x){
 static const short south[][2]={{0,2850},{182,2993},{408,3023},{647,3034},{932,2992},
  {1172,2820},{1510,2758},{1760,2670},{2050,2634},{2380,2519},{2690,2294},{3072,1940}};
 for(int i=1;i<LEN(south);i++)if(x<south[i][0])
  return south[i-1][1]+(south[i][1]-south[i-1][1])*(x-south[i-1][0])/(south[i][0]-south[i-1][0]);
 return 1940;
}
static int woodline_ground_type(int x,int y){
 int coast=woodline_coast_x(y)+(int)(10.f*sn(y*.021f)+5.f*sn(y*.051f));
 int shore=woodline_shore_y(x)+(int)(12.f*sn(x*.015f)+6.f*sn(x*.048f));
 if(x>=coast||(y>=BASE_ROOM-36&&y>=shore))return TILE_TIDE;
 if(y>=BASE_ROOM-36&&x<1106+(int)(55*sn(y*.010f))){
  int u=x-492,v=y-2636;
  if(u>-90&&u<90&&v>-67&&v<67&&u*u*4489+v*v*8100<36360900)return TILE_TARN; /* actual visible water */
  return TILE_WOOD;
 }
 return y<810?TILE_BLUSH:y<1600&&x>BASE_ROOM-36?TILE_YARD:TILE_DUNE;
}
static int outer_tile(int x,int y){
 if((U)x>=ROOM||(U)y>=ROOM||!outer_is_new(x,y))return TILE_TIDE;
 return outerLand[y*ROOM+x];
}
static int outer_step_at(float x,float y){
 if(!(x>=35&&x<=ROOM-36&&y>=35&&y<=ROOM-36))return 0;
 int ix=(int)x,iy=(int)y;
 if(!outer_is_new(ix,iy))return 1; /* classify the same pixel that the atlas/bake filled */
 int i=iy*ROOM+ix;
 if(outerLane[i])return 1; /* road surfaces can be real bridges */
 int k=outerLand[i];
 return !(k&TILE_BLOCK)&&k!=TILE_TIDE&&k!=TILE_TARN;
}
static int outer_walkable(float x,float y,float r){
 if(!outer_step_at(x,y))return 0;
 if(r<=0.f||!outer_is_new(x+r,y+r))return 1;
 if(!outer_step_at(x-r,y)||!outer_step_at(x+r,y)||!outer_step_at(x,y-r)||!outer_step_at(x,y+r))return 0;
 const float diag=r*.707f;
 return outer_step_at(x-diag,y-diag)&&outer_step_at(x+diag,y-diag)&&
        outer_step_at(x-diag,y+diag)&&outer_step_at(x+diag,y+diag);
}
static int woodline_is_grove(int x,int y){
 int k=outer_tile(x,y)&127;return k==TILE_WOOD||k==TILE_TRAIL||k==TILE_TARN;
}
/* Continuous world-anchored tide colors. Used both before clipping the
 * map into place and by the chase renderer past the last surveyed cell. */
static U oc_ocean_ink(int x,int y){
 int wave=y+(int)(sn(x*.019f)*3),phase=(wave+((x>>5)&7))%19;
 if(phase<0)phase+=19;
 unsigned s=outer_noise(x/3,y/3);
 U base=mix(0x1a3547,0x2b4c5b,s%4);
 if(phase<2&&((x/13)+(wave/19)*3)%9<4)base=mix(base,0x63858d,2);
 return base;
}
static U woodline_ground_ink(int x,int y,int kind){
 U grain=outer_noise(x/2,y/2),fleck=outer_noise(x,y);
 int wave=y+(int)(sn(x*.019f)*3),coset=(wave+((x>>5)&7))%19;
 if(kind==TILE_TIDE)return oc_ocean_ink(x,y);
 if(kind==TILE_TARN){
  U water=0x183c40;
  if(coset<2&&((x/12)+(wave/19)*3)%9<4)return mix(water,0x78a4a6,2);
  return mix(water,0x467471,fleck%3);
 }
 if(kind==TILE_BLUSH){
  int terrace=(y+((x/3)%81)+(int)(6.f*sn(x*.007f)))%92;
  U ink=mix(0x343f41,0x64644c,(grain/7)%3);
  return terrace<4?mix(ink,0x706375,3):ink;
 }
 if(kind==TILE_YARD){
  int yard=x%142<114&&y%121<88;
  U ink=mix(0x394651,0x707772,yard?grain%4:grain%2);
  if(yard&&(x%142>100||y%121>76))ink=mix(ink,0x172432,4);
  if(yard&&y%121==77&&x%15<7)ink=0x9e8577;
  return ink;
 }
 if(kind==TILE_DUNE){
  int shelf=(wave+((x>>4)&7))%47;
  return mix(0x445e54,0x77846b,(shelf<2?3:grain%3));
 }
 int shade=(int)(2.5f+sn(x*.012f+y*.009f)*1.10f+sn(y*.009f-x*.004f)*1.25f);
 U ink=mix(0x203934,0x48634c,cl(shade,0,6));
 if(fleck%23==0)return mix(ink,0x7b8162,2);
 if(grain%11<3)return mix(ink,0x344937,2);
 return mix(ink,0x6b6850,grain%3);
}
static void woodline_dirt(int i){
 OuterPoint a=woodline_path[woodline_edges[i][0]],b=woodline_path[woodline_edges[i][1]];
 for(int y=max(BASE_ROOM-35,min(a.y,b.y)-22);y<=max(a.y,b.y)+22;y++)
  for(int x=min(a.x,b.x)-22;x<=max(a.x,b.x)+22;x++){
   float t=out_proj(a,b,x,y);
   int ds=d2(x-a.x-(b.x-a.x)*t,y-a.y-(b.y-a.y)*t);
   if(ds>22*22)continue;
   int j=y*ROOM+x,k=outerLand[j];
   if(k!=TILE_WOOD&&k!=TILE_TRAIL)continue;
   outerLand[j]=TILE_TRAIL;
   int ash=outer_noise(x/2,y/2)%3;
   U ink=ds>13*13?mix(city[j],0x967853,2):mix(0x675845,0x947f58,ash);
   city[j]=ink;
   if(x<BASE_ROOM&&y<BASE_ROOM)chaseFloor[j]=ds>13*13?mix(chaseFloor[j],0x967853,2):ink;
  }
}
static void woodline_prop(int x,int y,int r,int type,int height,int grade){
 if(woodPropN>=LEN(woodProps)||!outer_walkable(x,y,r+2))return;
 woodProps[woodPropN++]=(WoodProp){x,y,r,type,height,grade};
 if(type==WOOD_POST||type==WOOD_UNDERGROWTH)return;
 for(int yy=y-r;yy<=y+r;yy++)for(int xx=x-r;xx<=x+r;xx++){
  if(d2(xx-x,yy-y)>r*r)continue;
  int i=yy*ROOM+xx;if(outerLane[i])continue;
  outerLand[i]|=TILE_BLOCK;
 }
}
static void woodline_scatter(void){
 woodPropN=0;
 for(int y=2047;y<2994;y+=52)for(int x=73;x<1090;x+=51){
  U u=outer_noise(x,y);
  int xx=x+(int)((u>>2)%9)-4,yy=y+(int)((u>>6)%9)-4;
  int rx,ry;if((outer_tile(xx,yy)&127)!=TILE_WOOD||!outer_walkable(xx,yy,26)||outer_nearest(xx,yy,&rx,&ry,58)>=0)continue;
  int reserved=0;
  for(int j=0;j<WOOD_N_SITES;j++)if(d2(xx-woodline_sites[j].x,yy-woodline_sites[j].y)<45*45){reserved=1;break;}
  for(int j=0;!reserved&&j<LEN(woodline_edges);j++){
   OuterPoint a=woodline_path[woodline_edges[j][0]],b=woodline_path[woodline_edges[j][1]];
   float t=out_proj(a,b,xx,yy);if(d2(xx-(a.x+(b.x-a.x)*t),yy-(a.y+(b.y-a.y)*t))<34*34)reserved=1;
  }
  if(reserved)continue;
  if((u&15)<3)woodline_prop(xx,yy,6,WOOD_ROCK,14+(u>>18)%8,u>>26);
  else if((u&15)==3)woodline_prop(xx,yy,6,WOOD_STUMP,9,u>>26);
  else woodline_prop(xx,yy,4+(u>>18)%3,WOOD_TREE,34+(u>>13)%29,u>>26);
 }
 /* A few large rocks away from the carriageway identify real terrain
  * obstacles at Glass Bluff, Salt Reach and the foothills. */
 for(int y=73;y<2740;y+=77)for(int x=2070;x<3010;x+=83){
  U u=outer_noise(x/4,y/4);int xx=x+(int)(u%29),yy=y+(int)((u>>5)%27),rx,ry;
  int ground=outer_tile(xx,yy);
  if(!(ground==TILE_BLUSH||ground==TILE_DUNE)||oc_plot_reserved(xx,yy)||outer_nearest(xx,yy,&rx,&ry,65)>=0)continue;
  if(u%5<2)woodline_prop(xx,yy,6+(u>>7)%8,WOOD_ROCK,11+(u>>15)%15,u>>26);
 }
 for(int y=2057;y<2980;y+=42)for(int x=71;x<1101;x+=57){
  U q=outer_noise(x+198,y-173);
  int xx=x+(int)(q%17)-8,yy=y+(int)((q>>6)%15)-7;
  if((outer_tile(xx,yy)&127)!=TILE_WOOD||outerLane[yy*ROOM+xx]||q%5<2)continue;
  int reserve=0;for(int j=0;j<WOOD_N_SITES;j++)if(d2(xx-woodline_sites[j].x,yy-woodline_sites[j].y)<38*38)reserve=1;
  if(!reserve)woodline_prop(xx,yy,0,WOOD_UNDERGROWTH,6+q%7,q>>26);
 }
 woodline_prop(603,2083,16,WOOD_HUT,30,0);
 woodline_prop(293,2240,8,WOOD_CRATE,10,1);
 woodline_prop(412,2424,10,WOOD_LOGPROP,12,1);
 woodline_prop(779,2256,13,WOOD_ROCK,24,2);
 woodline_prop(293,2696,9,WOOD_ROCK,20,2);
 woodline_prop(348,2728,8,WOOD_ROCK,25,5);
 for(int i=0;i<WOOD_N_SITES;i++)woodline_prop(woodline_sites[i].x-22,woodline_sites[i].y+25,0,WOOD_POST,18,i);
}
static void woodline_bake(void){
 z(outerLand,sizeof outerLand);
 for(int y=0;y<ROOM;y++){
  int start=y>=BASE_ROOM-35?0:BASE_ROOM-35;
  for(int x=start;x<ROOM;x++){
   int k=woodline_ground_type(x,y);
   if(!outer_is_new(x,y))continue;
   int i=y*ROOM+x;
   outerLand[i]=(unsigned char)k;
   /* Start by darkening the edge of the old water band instead of
    * recoloring city curb paint. The new area continues seamlessly. */
   U ink=woodline_ground_ink(x,y,k);
   if(x<BASE_ROOM&&y<BASE_ROOM){
    int n=cl((max(x,y)-(BASE_ROOM-36))*2/5,0,8);
    city[i]=mix(city[i],ink,n);chaseFloor[i]=mix(chaseFloor[i],ink,n);
   }else city[i]=ink;
  }
 }
 /* Contrast solid shores and creeks without outlining an old-city seam. */
 for(int y=35;y<ROOM-36;y++)for(int x=y>=BASE_ROOM?35:BASE_ROOM;x<ROOM-36;x++){
  int i=y*ROOM+x,k=outerLand[i];
  if(k!=TILE_TIDE&&k!=TILE_TARN&&(outerLand[i+5]==TILE_TIDE||outerLand[i+5]==TILE_TARN||
     outerLand[i+ROOM*5]==TILE_TIDE||outerLand[i+ROOM*5]==TILE_TARN))city[i]=mix(city[i],0x9b9778,3);
 }
 for(int i=0;i<LEN(woodline_edges);i++)woodline_dirt(i);
 /* Two short, signed paths bridge the historic artwork boundary. */
 for(int x=0;x<2;x++){
  int cx0=x?520:256;
  for(int y=1990;y<=BASE_ROOM+8;y++)for(int j=-14;j<=14;j++){
   int px=cx0+j,i=y*ROOM+px;
   if(y<BASE_ROOM-35&&y<2009)continue; /* keep drivable legacy seawall */
   if(roofMask[i])continue;
   if(outer_is_new(px,y))outerLand[i]=TILE_TRAIL;
   city[i]=mix(0x695f4f,0x92805a,outer_noise(px,y)%3);
   if(y<BASE_ROOM)chaseFloor[i]=city[i];
  }
 }
}
static void woodline_shadows(void){
 for(int j=0;j<woodPropN;j++){
  const WoodProp*p=woodProps+j;
  if(p->kind==WOOD_POST||p->kind==WOOD_UNDERGROWTH)continue;
  int w=p->kind==WOOD_TREE?p->height/3:p->radius+2;
  for(int y=-5;y<8;y++)for(int x=-w;x<=w;x++){
   int i=(p->y+y)*ROOM+p->x+x;
   if(x*x*16+y*y*w<17*w*w&&!outerLane[i])city[i]=mix(city[i],0x101f20,2);
  }
  if(!bakeFloorOnly&&p->kind==WOOD_TREE)rect(p->x-2,p->y-2,5,4,0x694e3a);
 }
}
static U woodline_map_ink(int tile){
 static const U inks[]={0x12283b,0x405457,0x334650,0x415c59,0x354f43,0x887657,0x1c424b,0x71828a};
 int k=tile&127;return k<LEN(inks)?inks[k]:inks[0];
}
