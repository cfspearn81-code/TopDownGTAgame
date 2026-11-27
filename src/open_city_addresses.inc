/* Public entrances for real addressable outer shells. Storage containers,
 * winches, signage, and other equipment are intentionally not addresses. */
enum { OC_BACK=0,OC_RIGHT=1,OC_FRONT=2,OC_LEFT=3 };
enum { OC_NONE,OC_NEIGHBOR,OC_QUAY,OC_WOOD };
typedef struct {
 const char *name,*feature,*plaque;
 unsigned char role,shell,edition;
} OcRoom;
typedef struct {
 const OcRoom *room;
 unsigned char family,face;
 short ax,ay,source;
} OcDoor;
static OcDoor ocDoor[LEN(buildings)];
static int ocLegacyCount;
static const OcDoor *oc_entry(int id){
 return id>=0&&id<LEN(ocDoor)&&ocDoor[id].room?ocDoor+id:0;
}
static const OcRoom ocQuay[]={
 {"LANTERN RIGGING SHED","THE RIGGING BENCH","The riggers keep their numbered winch spares on a blue rack behind the bay.",8,3,3},
 {"QUAY WATCH KIOSK","SIGN-IN LEDGER","A ledger records the hour the east floats first came in from the tide.",18,4,6},
 {"TIDE POST KIOSK","THE FAR PIER LOG","The signal keeper once sorted tide notices beside the green gangway lamp.",15,7,7}
};
static const OcRoom ocWoods={
 "GREYREED RANGER HUT","THE RANGER'S WALL MAP","The indoor map marks the dry paths around Mirror Tarn. Find the ranger at the outdoor station.",18,2,8
};
typedef struct {short x,y,w,h,d;U wall,roof;OcRoom room;} OcBlock;
static const OcBlock ocBlocks[]={
 {2475,200,62,36,18,0x796b5b,0x766357,{"NORTHPOINT PIE SHOP","THE STEEL OVEN","The crew baked these hand pies before the cliff road had a streetlight.",2,0,4}},
 {2564,205,62,33,22,0x53646d,0x4c595a,{"CROW RIDGE RECORDS","NORTH COAST PRESSINGS","The back shelf has test pressings from the old lookout transmitter.",1,1,6}},
 {2653,205,66,35,21,0x62776e,0x506460,{"MORNING ROOM","THE HILL COUNTER","The curved breakfast counter is cut from a salvaged ferry bench.",17,2,3}},
 {2750,208,61,37,20,0x82715e,0x635c4f,{"NORTHPOINT CYCLE CO-OP","BLUFF SERVICE STAND","Members leave a clean stand for whoever rides up the bluff next.",8,6,6}},
 {2254,181,72,40,24,0x696e78,0x5c565e,{"HIGH MEADOW READING ROOM","THE UPLAND ATLAS","The oldest atlas shows the grass road that used to end at the ridge.",18,4,5}},
 {2287,266,71,39,17,0x78766a,0x55615c,{"NORTHPOINT STONE GALLERY","THE COASTAL STUDY","Local limestone catches a violet band under the western skylight.",5,7,3}},
 {2500,69,63,33,22,0x656e76,0x4e5867,{"THE BLUFF RADIO CABIN","THE WEATHER FEED","Operators call wind reports from the roof when the coast goes quiet.",4,4,5}},
 {2624,67,68,37,18,0x656d67,0x545d56,{"SURVEYORS' SKY CLUB","THE HEADLAND SCOPE","A brass scope points beyond the cliff road to the north shoals.",14,5,7}},
 {2746,69,67,38,23,0x7a665c,0x625657,{"NORTHPOINT BOARDING HOUSE","THE TRAVELER'S BOARD","Room keys here carry the etched numbers of old ferry berths.",16,1,6}},
 {2316,78,69,34,23,0x756762,0x614f56,{"LOOKOUT PRINT SHOP","THE RIDGE PRESS","A two-color print separates the night road from its rocky coast.",13,8,6}}
};
static int oc_plot_reserved(int x,int y){
 if(y>423||y<35||x<2210||x>2840)return 0;
 for(int i=0;i<LEN(ocBlocks);i++){
  const OcBlock*a=ocBlocks+i;
  if(x>=a->x-20&&x<=a->x+a->w+20&&y>=a->y-20&&y<=a->y+a->h+28)return 1;
 }
 return 0;
}
static void oc_room_metadata(int,const OcRoom*);
static unsigned char oc_kind(int id){const OcDoor*d=oc_entry(id);return d?d->family:OC_NONE;}
static int oc_door_world(int id,int *x,int*y,int *side){
 const OcDoor*d=oc_entry(id);if(!d)return 0;
 *x=d->ax;*y=d->ay;*side=d->face;return 1;
}
static int oc_placed_door(int family,int source){
 for(int i=ocLegacyCount;i<nb;i++)if(ocDoor[i].family==family&&ocDoor[i].source==source)return i;
 return -1;
}
static void oc_wall_points(Building b,int face,int tangent,int *ax,int *ay,int *dx,int*dy){
 const int horiz=(face==OC_BACK||face==OC_FRONT);
 *ax=horiz?b.x+b.w/2+tangent:face==OC_LEFT?b.x:b.x+b.w;
 *ay=horiz?(face==OC_BACK?b.y:b.y+b.h):b.y+b.h/2+tangent;
 *dx=horiz?0:face==OC_LEFT?-1:1;
 *dy=horiz?(face==OC_BACK?-1:1):0;
}
static int oc_door_safe(int x,int y,int family){
 if(!outer_walkable(x,y,10)||solid(x,y,10))return 0;
 float keep=family==OC_NONE?22.f:26.f;
 for(int i=0;i<LEN(loc);i++)if(d2(loc[i].x-x,loc[i].y-y)<keep*keep)return 0;
 for(int i=0;i<nb;i++)if(ordinaryDoorY[i]&&d2(ordinaryDoorX[i]-x,ordinaryDoorY[i]-y)<keep*keep)return 0;
 for(int i=0;i<HARBOR_N_DOCK_POINTS;i+=2)if(d2(harborDockPoints[i].x-x,harborDockPoints[i].y-y)<36.f*36.f)return 0;
 if(d2(x-HARBOR_OFFICE_X,y-HARBOR_OFFICE_Y)<34.f*34.f)return 0;
 if(d2(x-woodline_sites[WOOD_RANGER].x,y-woodline_sites[WOOD_RANGER].y)<37.f*37.f)return 0;
 return 1;
}
static void oc_paint_exterior_door(int id,int ox,int oy){
 const OcDoor*d=oc_entry(id);if(!d)return;
 int x=d->ax-ox,y=d->ay-oy,face=d->face;
 U c=mix(buildings[id].wall,GOLD,4),wood=0x536b6a;
 if(face==OC_FRONT||face==OC_BACK){
  int front=face==OC_FRONT?1:-1;
  for(int j=1;j<=5;j++)rect(x-7,y+front*j,15,1,mix(wood,0x1c2b38,3+j%3));
  if(!bakeFloorOnly){
   if(front==1){box(x-6,y-10,13,11,0x213842,c);rect(x-4,y-8,9,4,0x648982);rect(x-6,y-11,13,2,c);}
   else rect(x-5,y-3,11,4,c);
  }
 }else{
  int side=face==OC_LEFT?-1:1;
  if(!bakeFloorOnly){rect(x-3,y-5,7,11,0x213842);rect(x-2,y-2,5,4,0x648982);}
  for(int j=1;j<5;j++)rect(x+side*j,y-5,1,11,c);
 }
 int z0=ordinaryDoorX[id]-ox,z1=ordinaryDoorY[id]-oy;rect(z0-3,z1-3,7,7,0x2e4147);dot(z0,z1,c);
}
static int oc_register(Building b,const OcRoom*name,int family,int source){
 if(nb>=LEN(buildings))return -1;
 static const int faces[]={OC_FRONT,OC_LEFT,OC_RIGHT,OC_BACK},off[]={0,-5,5,-10,10,-15,15};
 for(int i=0;i<LEN(faces);i++)for(int j=0;j<LEN(off);j++){
  if(absf(off[j])>((faces[i]%2?b.h:b.w)/2-4))continue;
  int ax,ay,u,v;oc_wall_points(b,faces[i],off[j],&ax,&ay,&u,&v);
  for(int dist=16;dist<=39;dist+=3){
   int x=ax+u*dist,y=ay+v*dist;
   if(!oc_door_safe(x,y,family)||!street_line_open(x,y,x+5*u,y+5*v,10))continue;
   int id=nb++;
   buildings[id]=b;ordinaryDoorX[id]=x;ordinaryDoorY[id]=y;
   ocDoor[id]=(OcDoor){name,(unsigned char)family,(unsigned char)faces[i],ax,ay,source};
   oc_room_metadata(id,name);return id;
  }
 }
 return -1;
}
static void oc_block_shell(Building b){
 StreetBounds sh=building_bounds(&b);
 for(int y=sh.y;y<sh.y+sh.h;y++)for(int x=sh.x;x<sh.x+sh.w;x++){
  int k=y*ROOM+x;
  if(outerLane[k])continue;
  outerLand[k]|=TILE_BLOCK;roofMask[k]=1;
 }
}
static void oc_extend_city(void){
 ocLegacyCount=nb;z(ocDoor,sizeof ocDoor);
 for(int i=0;i<LEN(ocBlocks);i++){
  const OcBlock *a=ocBlocks+i;
  Building b={a->x,a->y,a->w,a->h,a->d,a->roof,a->wall};
  StreetBounds sh=building_bounds(&b);int dry=1;
  for(int y=sh.y;y<sh.y+sh.h&&dry;y++)for(int x=sh.x;x<sh.x+sh.w;x++){
   int k=y*ROOM+x;if(outerLane[k]||!outer_step_at(x,y)){dry=0;break;}
  }
  if(!dry)continue;
  int id=oc_register(b,&a->room,OC_NEIGHBOR,i);
  if(id<0)continue;
  oc_block_shell(b);
 }
 int quay=0;
 for(int i=0;i<LEN(harborPieces);i++){
  const HarborPiece*p=harborPieces+i;
  if(p->kind!=HB_SHED&&p->kind!=HB_KIOSK)continue;
  const OcRoom*room=ocQuay+quay++;
  if(!harborPiecePlaced[i])continue;
  oc_register((Building){p->x,p->y,p->w,p->h,p->height,p->roof,p->body},room,OC_QUAY,i);
 }
 for(int i=0;i<woodPropN;i++){
  const WoodProp*p=woodProps+i;
  if(p->kind!=WOOD_HUT)continue;
  Building hut={p->x-p->radius,p->y-13,p->radius*2,26,17,0x596e61,0x71684d};
  /* The visible lodge has rectangular walls, not just its legacy prop's
     circular trunk mask. Do not let light or people through its corners. */
  if(oc_register(hut,&ocWoods,OC_WOOD,i)>=0)oc_block_shell(hut);
 }
}
static void oc_paint_city(void){
 for(int i=ocLegacyCount;i<nb;i++)if(oc_kind(i)==OC_NEIGHBOR)building(buildings+i);
 for(int i=ocLegacyCount;i<nb;i++)if(oc_kind(i)!=OC_WOOD)oc_paint_exterior_door(i,0,0);
}
