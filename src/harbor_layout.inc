/* The Lantern Quay working harbor occupies the southeastern waterline.
   All coordinates are world units. Roads, pedestrian decks, and charted
   sea are disjoint physical queries; boats do not use the land hull. */
enum { HARBOR_PUBLIC_FOOT,HARBOR_PUBLIC_BOAT,HARBOR_CARGO_FOOT,HARBOR_CARGO_BOAT,
       HARBOR_TIDE_FOOT,HARBOR_TIDE_BOAT,HARBOR_N_DOCK_POINTS };
enum { HARBOR_OFFICE_X=2896,HARBOR_OFFICE_Y=2096,HARBOR_YARD_X=2848,HARBOR_YARD_Y=2114 };
static const OuterPoint harborDockPoints[HARBOR_N_DOCK_POINTS]={
 {2948,2206},{2948,2244},{2824,2234},{2824,2270},{2478,2530},{2478,2572}
};
static int harbor_dock_point(int dock,int *x,int *y){
 if((unsigned)dock>=HARBOR_N_DOCK_POINTS||!x||!y)return 0;
 *x=harborDockPoints[dock].x;*y=harborDockPoints[dock].y;return 1;
}
enum {HB_QUAY,HB_BANK,HB_WALK,HB_ROCK};
typedef struct{short x,y,w,h;unsigned char kind;} HarborPaving;
/* Three individual fingers, with open water between their heads. The
   concrete necks meet native terrain; no invisible full-rectangle bridge. */
static const HarborPaving harborPaving[]={
 {2771,2026,201,102,HB_QUAY},{2777,2109,115,43,HB_BANK},
 {2786,2138,84,90,HB_QUAY},{2807,2206,34,41,HB_WALK},
 {2918,2115,55,39,HB_BANK},{2928,2146,41,72,HB_WALK},
 {2438,2423,86,56,HB_QUAY},{2461,2470,36,72,HB_WALK},
 {2990,2066,16,53,HB_ROCK},{2994,2113,15,45,HB_ROCK}
};
typedef struct{short x,y,w,h,height;unsigned char kind;U body,roof;} HarborPiece;
enum {HB_SHED,HB_BOX,HB_STACK,HB_KIOSK,HB_WINCH};
static const HarborPiece harborPieces[]={
 /* Their real ground footprints intentionally stay off every vehicle lane. */
 {2778,2073,26,18,11,HB_BOX,0xa86156,0xdc8470},
 {2778,2094,26,12,10,HB_BOX,0x4d8591,0x80b3b5},
 {2806,2073,22,18,17,HB_STACK,0x586a89,0x849cb6},
 {2808,2095,14,12,11,HB_BOX,0xa88a52,0xd9b07b},
 {2876,2110,43,16,15,HB_SHED,0x4c6971,0x668e87},
 {2903,2028,27,11,12,HB_KIOSK,0x806052,0xbb9270},
 {2960,2117,11,11,14,HB_BOX,0x476e75,0x73aaa0},
 {2787,2176,6,12,14,HB_WINCH,0x9c7950,0xd9bd87},
 {2864,2173,6,12,13,HB_WINCH,0x9c7950,0xd9bd87},
 {2504,2446,16,23,12,HB_KIOSK,0x606a74,0x8b9a9b}
};
static unsigned char harborPiecePlaced[LEN(harborPieces)];
static int harbor_paint_class(int x,int y){
 for(int i=0;i<LEN(harborPaving);i++){
  const HarborPaving*b=harborPaving+i;
  if(x<b->x||y<b->y||x>=b->x+b->w||y>=b->y+b->h)continue;
  int u=min(x-b->x,b->x+b->w-1-x),v=min(y-b->y,b->y+b->h-1-y);
  /* Quay corners are truncated, finger edges are squared timber. */
  if((b->kind==HB_QUAY||b->kind==HB_ROCK)&&u+v<5)continue;
  return b->kind;
 }
 return -1;
}
static int harbor_ocean_cell(float x,float y){
 if(!(x>=35.f&&y>=2010.f&&x<=ROOM-36.f&&y<=ROOM-36.f))return 0;
 int ix=(int)x,iy=(int)y,i=iy*ROOM+ix;
 return outerLand[i]==TILE_TIDE&&!outerLane[i];
}
static int harbor_nav_water(float x,float y,int radius){
 if(radius<0||radius>96||!harbor_ocean_cell(x,y))return 0;
 float r=radius,d=r*.7071068f;
 /* Marine hull may not clip the end of a wharf, the former public road
    bridges, land, Tarn, or the mapped sea's safe border in either view. */
 return !radius||(harbor_ocean_cell(x-r,y)&&harbor_ocean_cell(x+r,y)&&
  harbor_ocean_cell(x,y-r)&&harbor_ocean_cell(x,y+r)&&
  harbor_ocean_cell(x-d,y-d)&&harbor_ocean_cell(x-d,y+d)&&
  harbor_ocean_cell(x+d,y-d)&&harbor_ocean_cell(x+d,y+d));
}
static U harbor_paving_ink(int x,int y,int k){
 U bit=outer_noise(x,y);
 const U base[]={0x515961,0x56636b,0x747064,0x4e6970};
 U c=mix(base[k],0x9a9685,k==HB_ROCK?(int)(bit%3):(int)(bit%2));
 if(k==HB_WALK){
  if(y%7==0)c=0x4e5654;
  else if(x/12%2)c=mix(c,0xa39171,2);
  if(x%13==2&&y%7==3)c=0x455e63;
 }else if(k==HB_ROCK){
  if(x%7<2||y%8<2)c=mix(c,0x0f3747,3);
 }else if(x%32==0||y%24==0)c=0x414b54;
 return c;
}
static void harbor_bake_ground(void){
 /* Geometry, atlas and renderer share this material. New dry surfaces
    supplant only this empty eastern shore, not the city, woods or public pier. */
 for(int y=2026;y<2542;y++)for(int x=2438;x<3009;x++){
  int k=harbor_paint_class(x,y);if(k<0)continue;
  int i=y*ROOM+x;
  outerLand[i]=TILE_HARBOR|(k==HB_ROCK?TILE_BLOCK:0);
  city[i]=harbor_paving_ink(x,y,k);
 }
}
static void harbor_water_art(void){
 /* Broad cyan shallows, quiet blue channel, and highlights touching only a
    physical shoreline. Stay out of the old port road and the Greyreed map. */
 for(int y=2010;y<2710;y++)for(int x=2350;x<ROOM-36;x++){
  int i=y*ROOM+x;
  if(outerLand[i]!=TILE_TIDE||outerLane[i])continue;
  U grain=outer_noise(x/2,y/2),fine=outer_noise(x,y);
  U ink=mix(0x173a50,0x244b62,grain%3);
  if(!harbor_ocean_cell(x-12,y)||!harbor_ocean_cell(x+12,y)||!harbor_ocean_cell(x,y-12))ink=mix(ink,0x548f88,2);
  int wave=y+(int)(sn(x*.018f)*3);
  if((wave+(x>>5))%21==0&&fine%9<6)ink=mix(ink,0x6d9b9c,2);
  if(y%3==0&&fine%7<2&&(!harbor_ocean_cell(x-2,y)||!harbor_ocean_cell(x,y-2)))ink=mix(ink,0x99beb3,3);
  int blend=cl(min(min(x-2350,ROOM-36-x),min(y-2010,2710-y))/11,0,8);
  city[i]=mix(city[i],ink,blend);
 }
}
static void harbor_edge_art(void){
 for(int y=2026;y<2542;y++)for(int x=2438;x<3009;x++){
  int k=harbor_paint_class(x,y);
  if(k<0||outerLand[y*ROOM+x]==TILE_TIDE||outerLane[y*ROOM+x])continue;
  int shore=outerLand[(y+1)*ROOM+x]==TILE_TIDE||outerLand[y*ROOM+x+1]==TILE_TIDE||
            outerLand[y*ROOM+x-1]==TILE_TIDE;
  if(!shore)continue;
  dot(x,y,k==HB_WALK?0xcab896:0xa5bbb3);
  if(y%14<5||x%22<5)dot(x,y,k==HB_WALK?0x695340:0xc4b381);
  if(x%14==3||y%14==3){
   if(outerLand[(y+2)*ROOM+x]==TILE_TIDE)rect(x,y+1,2,3,0x263f4c);
   if(outerLand[y*ROOM+x+2]==TILE_TIDE)rect(x+1,y,3,2,0x263f4c);
  }
 }
}
static void harbor_place_piece(int id){
 const HarborPiece*b=harborPieces+id;
 int x=b->x,y=b->y,w=b->w,h=b->h;
 int clear=1;
 for(int yy=y;yy<y+h&&clear;yy++)for(int xx=x;xx<x+w;xx++)
  if(outerLane[yy*ROOM+xx]||outerLand[yy*ROOM+xx]!=TILE_HARBOR){clear=0;break;}
 harborPiecePlaced[id]=(unsigned char)clear;if(!clear)return;
 for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++){outerLand[yy*ROOM+xx]|=TILE_BLOCK;roofMask[yy*ROOM+xx]=1;}
}
static void harbor_draw_piece(int id){
 if(!harborPiecePlaced[id])return;
 const HarborPiece*b=harborPieces+id;
 int x=b->x,y=b->y,w=b->w,h=b->h,ht=b->height;
 U edge=0x283447,shade=mix(b->body,edge,3);
 rect(x+3,y+h-1,w+4,5,0x303e48);
 if(bakeFloorOnly)return;
 box(x,y-ht/2,w,h+ht/2,b->body,edge);
 rect(x+2,y+h-4,w-4,2,mix(b->body,0xf0dcc4,3));
 box(x,y-ht/2,w,h,b->roof,edge);
 if(b->kind==HB_SHED){
  for(int yy=y-ht/2+2;yy<y-ht/2+h-3;yy+=4)rect(x+1,yy,w-2,1,mix(b->roof,edge,3));
  for(int j=0;j<4;j++)box(x+4+j*11,y+h-8,8,6,0xcbce9d,shade);
  rect(x+4,y-ht/2+1,w-8,1,0xbed2bd);
 }else if(b->kind==HB_WINCH){
  disc(x+w/2,y+h/2,4,0x3e3f43);disc(x+w/2,y+h/2,2,0xc6ae83);
  for(int j=1;j<w-1;j+=3)rect(x+j,y+h-4,1,4,0xe3bb67);
 }else{
  for(int j=3;j<w-1;j+=3){
   rect(x+j,y-ht/2+2,1,h-4,shade);
   dot(x+j+1,y-ht/2+3,mix(b->roof,PAPER,3));
  }
  rect(x+4,y+h-7,min(10,w-5),3,0xc8b28d);
  if(b->kind==HB_STACK){rect(x+3,y-ht/2-4,w-6,6,0x8eaaa8);rect(x+4,y-ht/2-4,w-8,1,0xc7d3b4);}
 }
}
static void harbor_dock_bollards(int which,int half){
 int x=harborDockPoints[which].x,y=harborDockPoints[which].y;
 /* Stripes and lanterns lie on the deck but do not block boarding. */
 for(int row=0;row<2;row++)for(int side=-1;side<=1;side+=2){
  int a=x+side*half,b=y+row*7-17;
  disc(a,b+1,4,0x344354);
  if(bakeFloorOnly)continue;
  box(a-2,b-3,5,7,0x43404a,0xcab18b);rect(a-1,b-1,3,1,0xf0d195);
  if(!row)rect(a-1,b-5,3,2,0xed8c7d);
 }
 box(x-4,y-4,9,9,0x2d454d,0x9cd3ba);line(x-4,y+6,x+4,y+6,0xaec8be);
 for(int i=0;i<4;i++)dot(x-3+i*2,y+9,0xd1cb9a);
}
static void harbor_bake_structures(void){
 harbor_water_art();
 harbor_edge_art();
 for(int i=0;i<LEN(harborPieces);i++)harbor_draw_piece(i);
 harbor_dock_bollards(HARBOR_PUBLIC_FOOT,15);harbor_dock_bollards(HARBOR_CARGO_FOOT,13);
 harbor_dock_bollards(HARBOR_TIDE_FOOT,13);
 /* Walking lane and loading rules painted in the port's own art language. */
 for(int y=2119;y<2187;y+=11){
  rect(2797,y,3,6,0xd5a663);rect(2857,y,3,6,0xd5a663);
 }
 /* Painted foot bypass continues past a parked Mote before curling into
    the gangway. The paved shoulder remains physically clear underneath. */
 for(int y=2155;y<2220;y+=7){rect(2850,y,2,4,0xa9b9a6);rect(2856,y,1,4,0xa9b9a6);}
 for(int x=2829;x<2854;x+=6)rect(x,2215,3,2,0xa9b9a6);
 for(int x=2927;x<2969;x+=7)rect(x,2138,4,2,0xd9b175);
 for(int x=2463;x<2495;x+=7)rect(x,2480,4,2,0xd9b175);
 rect(2865,2097,18,4,0x596c76);rect(2866,2098,16,1,0xa6ac95);
}
