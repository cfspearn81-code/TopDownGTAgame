/* Address census using actual engine entry, E fixtures, native projection,
 * and exit paths. Synthetic states only, with no ordinary job or save edits. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void press(U key){tick(key);tick(0);}
static int active_building(int i){return buildings[i].w>0&&buildings[i].h>0;}
static int spot(int x,int y,int venue,int building){
 p.x=x;p.y=y;p.car=-1;
 StreetUse t=street_use_target(placeNear(),-1);
 return street_foot_clear(x,y,-1)&&t.kind==(venue?STREET_USE_VENUE:STREET_USE_SHOP)&&t.id==(venue?zone_id[building]-1:building);
}
static void leave_address(int venue,int id){
 if(scene==3){assert(venue);interior_x=W/2;interior_y=420;press(USE);}
 else if(scene==4){assert(!venue&&walkin_active==id&&wi_at_door());press(USE);}
 else if(scene==NIGHT_CLUB_SCENE){assert(nc_target()==NCA_EXIT);press(USE);}
 else assert(!"unexpected room");
 assert(scene==1);
}
enum{QW=W/2,QH=H/2};
static int cells[QW*QH],prev_cell[QW*QH];
static int station_goal(float x,float y){
 const InteriorProp*f=wi_f;float dx=x<f->x?f->x-x:x>f->x+f->w?x-(f->x+f->w):0;
 float dy=y<f->y?f->y-y:y>f->y+f->h?y-(f->y+f->h):0;
 return dx*dx+dy*dy<=31*31&&d2(x-W/2,y-430)>=36*36;
}
static int room_goal(int door){return door?wi_at_door():wi_at_station()&&!wi_at_door();}
static int wi_probe(float x,float y){
 if(wi_solid(x,y))return 0;
 for(int n=-1;n<=1;n+=2)for(int m=-1;m<=1;m+=2)if(wi_solid(x+2*n,y+2*m))return 0;
 return 1;
}
static void walk_room(int b,int door){
 if(room_goal(door))return;
 const int sx=(int)wi_x/2,sy=(int)wi_y/2,seed=sy*QW+sx;
 const float ox=wi_x-2*sx,oy=wi_y-2*sy;
 int n=0,m=0,g=-1;cells[m++]=seed;
 for(int j=0;j<LEN(prev_cell);j++)prev_cell[j]=-1;
 prev_cell[seed]=seed;
 while(n<m){
  int j=cells[n++],x=j%QW,y=j/QW;
  float xx=2*x+ox,yy=2*y+oy;
  if(door?d2(xx-W/2,yy-430)<=29*29:station_goal(xx,yy)){g=j;break;}
  int neighbors[]={j+1,j-1,j+QW,j-QW};
  for(int k=0;k<4;k++){
   int h=neighbors[k];
   if(h<=0||h>=LEN(prev_cell)||prev_cell[h]>=0||absf((h%QW)-x)>1||
      !wi_probe((h%QW)*2+ox,(h/QW)*2+oy))continue;
   prev_cell[h]=j;cells[m++]=h;
  }
 }
 if(g<0){fprintf(stderr,"No %s route: %d %s\n",door?"exit":"station",b,walkin_name(b));assert(g>=0);}
 int i=0;while(g!=seed){cells[i++]=g;g=prev_cell[g];}
 while(i&&!room_goal(door)){
  int q=cells[--i],delta=q-g;g=q;
  while(i&&cells[i-1]-g==delta)g=cells[--i];
  int horizontal=delta==1||delta==-1,positive=delta>0;
  const float dest=horizontal?(g%QW)*2+ox:(g/QW)*2+oy;
  const float *actual=horizontal?&wi_x:&wi_y;
  U key=horizontal?(positive?RIGHT:LEFT):(positive?DOWN:UP);
  for(int j=0;j<1000&&!room_goal(door)&&(positive?dest-*actual:*actual-dest)>.2f;j++){
   float old=*actual;tick(key);if(*actual==old)break;
  }
 }
 tick(0);
 if(!room_goal(door))fprintf(stderr,"Input did not reach %s: %d %s at %.2f,%.2f\n",
   door?"exit":"station",b,walkin_name(b),wi_x,wi_y);
 assert(room_goal(door));
}
static void use_station(int b){
 assert(wi_x==480&&wi_y==416&&wi_n>0);
 walk_room(b,0);
 int was=wi_sw[b];press(USE);assert(scene==4&&wi_sw[b]!=was&&p.noticeT>0);
 walk_room(b,1);
}
static void northpoint_geometry(void){
 int ribbon=0;
 for(int i=OUT_ROADS_WITH_QUAY;i<LEN(outerRoads);i++){
  const OuterRoad*r=outerRoads+i;
  OuterPoint a=outerNodes[r->a],b=outerNodes[r->b];float dx=b.x-a.x,dy=b.y-a.y;
  Car c={.type=0,.cond=100,.a=look(dx,dy)};
  for(int t=0;t<=100;t++){
   c.x=a.x+dx*t/100;c.y=a.y+dy*t/100;
   assert(!street_hits_wall(&c)&&outer_walkable(c.x,c.y,10));
   ribbon++;
  }
 }
 assert(ribbon==1010);
 p.car=0;
 for(int i=0;i<LEN(outerStops)-1;i++){
  NavPoint a=nav_outer_point(outerStops[i].node),b=nav_outer_point(OUT_NP_GREEN);
  nav_solve(a,b);assert(navCount>=2&&nav_same(navNodes[navCount-1],b));
  for(int k=1;k<navCount;k++)assert(nav_ribbon(navNodes[k-1],navNodes[k],6));
 }
 p.x=2651;p.y=282;p.car=-1;
 press(MAP);assert(mapOn&&nav_span()==ROOM);
 int id=NAV_OUT_BASE+LEN(outerStops)-1;
 NavPoint q=nav_door(id);
 int ax=NAV_MAP_X+q.x*NAV_MAP_SIZE/nav_span(),ay=NAV_MAP_Y+q.y*NAV_MAP_SIZE/nav_span();
 pointer_input(ax,ay,1);assert(navHasManual&&navManualLoc==id&&nav_same(navManual,q));
 press(ESC);assert(!mapOn);nav_clear();
}
static void tide_geometry(void){
 assert(!harbor_nav_water(2900,199,10)&&!harbor_nav_water(520,2011,0));
 assert(!harbor_nav_water(492,2636,3)&&!harbor_nav_water(ROOM-37,1165,15));
 for(int y=245;y<=1980;y+=6)assert(harbor_nav_water(3018,y,15));
 const U dry=0x123456;
 const unsigned phase=19;const int detail=215;
 assert(wl_ocean_edge(dry,ROOM-1,1100,phase,detail)==wl_bluewater(ROOM-1,1100,phase,detail));
 assert(wl_ocean_edge(dry,ROOM,1100,phase,detail)==wl_bluewater(ROOM,1100,phase,detail));
 assert(wl_ocean_edge(dry,3018,0,phase,detail)==wl_bluewater(3018,0,phase,detail));
 assert(wl_ocean_edge(dry,3018,(unsigned)-1,phase,detail)==wl_bluewater(3018,-1,phase,detail));
 assert(wl_ocean_edge(dry,2400,(unsigned)-1,phase,detail)==dry); /* headland */
 assert(wl_ocean_edge(dry,1024,(unsigned)-1,phase,detail)==dry); /* legacy chart edge */
 assert(wl_ocean_edge(dry,ROOM+5,1100,0,detail)!=wl_ocean_edge(dry,ROOM+5,1100,83,detail));
 assert(!harbor_nav_water(3018,0,0)&&!harbor_nav_water(ROOM,1100,0));
 const NavPoint east={3012,1165};
 for(int dock=0;dock<3;dock++){
  NavPoint mooring;assert(harbor_dock_point(dock*2+1,&mooring.x,&mooring.y));
  for(int reverse=0;reverse<2;reverse++){
   NavPoint start=reverse?east:mooring,target=reverse?mooring:east;
   nav_solve_sea(start,target);
   assert(navCount>=3&&navCount<=5&&nav_same(navNodes[0],start)&&nav_same(navNodes[navCount-1],target)&&navDist>1000);
   for(int i=0;i<navCount;i++){
    assert(harbor_boat_nav_clear(navNodes[i].x,navNodes[i].y));
    if(i)assert(nav_sea_hull_segment(navNodes[i-1],navNodes[i]));
   }
  }
 }
 nav_solve_sea(east,(NavPoint){2808,1380});assert(!navCount); /* dry target */
}
static void material_geometry(void){
 cf_road_prepare();
 assert(cfRoadInk[95*256+31]==5&&cfRoadInk[95*256+32]==10); /* widened cap/gutter */
 unsigned new_lamps=0;StreetLamp s;
 for(int i=OUT_NP_PASS;i<LEN(outerNodes);i++)if(street_lamp_outer(i,&s)){
  assert(street_lamp_clear(&s)&&street_lamp_foot(s.x,s.y,0));
  new_lamps++;
 }
 assert(new_lamps>0);
 int id=oc_placed_door(OC_NEIGHBOR,0);assert(id>=0);
 const Building*b=buildings+id;
 unsigned x=b->x+20,y=b->y-b->ht+7,i=y*ROOM+x;
 U clean=wl_material_ink(woodline_ground_ink(x,y,TILE_BLUSH),x,y);
 assert(chaseFloor[i]==clean&&cityAtlas[i]!=clean); /* roof only in overhead painter */
 printf("Verified %u new Northpoint lamps clear the roads and door approaches; curb and roof/floor separation pass.\n",new_lamps);
}
static void restore_changed_ground(void){
 int id=oc_placed_door(OC_NEIGHBOR,0);assert(id>=0);
 const Building*b=buildings+id;
 int x=b->x+b->w/2,y=b->y+b->h/2;
 assert(solid(x,y,3));
 /* This coordinate was ordinary dry plateau before the Pie Shop existed. */
 nv.world[NW_SAVE_X]=(U)x*NW_SAVE_SCALE|0x80000000u;nv.world[NW_SAVE_Y]=(U)y*NW_SAVE_SCALE;
 sv.x=1792;sv.y=512;restore_outer_position();
 assert(outer_is_new(sv.x,sv.y)&&outer_walkable(sv.x,sv.y,10)&&d2(sv.x-x,sv.y-y)<80*80);
 assert(nv.world[NW_SAVE_X]==(U)(sv.x*NW_SAVE_SCALE+.5f));
 nv.world[NW_SAVE_X]=0xfffffffdu;nv.world[NW_SAVE_Y]=40000000u;
 sv.x=1792;sv.y=512;restore_outer_position();
 assert(sv.x==1792&&sv.y==512&&!nv.world[NW_SAVE_X]&&!nv.world[NW_SAVE_Y]);
}
int main(void){
 init_city();profiles_activate();sv.intro=4;sv.owned=0;sv.dayClock=625;nc=0;
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=9999};
 int known=0,ordinary=0,north=0,quay=0,hut=0;
 for(int s=0;s<2;s++)for(int i=0;i<nb;i++)if(active_building(i)){
  const int venue=zone_is_building(i),orig=venue?zone_id[i]-1:-1;
  int x=venue?loc[orig].x:ordinaryDoorX[i],y=venue?loc[orig].y:ordinaryDoorY[i];
  if(!(x>0&&y>0&&spot(x,y,venue,i))){
   fprintf(stderr,"Inaccessible address %d %s @%d,%d\n",i,walkin_name(i),x,y);return 1;
  }
  menu=-1;scene=1;mapOn=0;atmOpen=-1;clear_input();vd_reset();end_job();npc_reset();
  nv.garage[60]=s;
  const char *caption=venue?loc[orig].label:walkin_name(i);
  paint();
  tick(USE);tick(0);
  if(!((venue&&scene==3&&interior_active_loc==orig)||
       (!venue&&((scene==4&&walkin_active==i)||(i==ncFrontId&&scene==NIGHT_CLUB_SCENE))))){
   fprintf(stderr,"Wrong entry %d %s @%d,%d -> scene %d room %d\n",i,caption,x,y,scene,walkin_active);return 1;
  }
  if(oc_entry(i)){
   Building*b=buildings+i;assert(beam_block(b->x+2,b->y+3)&&solid(b->x+2,b->y+3,0));
   assert(wiAddress[i].feature&&wiPlaque[i]&&!tram_hall(i));
   if(s)paint();
   use_station(i);
  }
  leave_address(venue,i);
  if(!s){
   known+=venue;ordinary+=!venue;
   north+=oc_kind(i)==OC_NEIGHBOR;quay+=oc_kind(i)==OC_QUAY;hut+=oc_kind(i)==OC_WOOD;
  }
 }
 for(int i=0;i<LEN(ocBlocks);i++)assert(oc_placed_door(OC_NEIGHBOR,i)>=0);
 for(int i=0;i<LEN(harborPieces);i++)if(harborPiecePlaced[i]&&(harborPieces[i].kind==HB_SHED||harborPieces[i].kind==HB_KIOSK))
  assert(oc_placed_door(OC_QUAY,i)>=0);
 for(int i=0;i<woodPropN;i++)if(woodProps[i].kind==WOOD_HUT)assert(oc_placed_door(OC_WOOD,i)>=0);
 assert(north==LEN(ocBlocks)&&quay>0&&hut==1);
 northpoint_geometry();tide_geometry();restore_changed_ground();material_geometry();
 printf("Address census in both views: %d named + %d ordinary; ordinary includes %d Northpoint, %d Quay, %d Ranger Hut. Real fixture/exit checks passed.\n",known,ordinary,north,quay,hut);
 puts("Northpoint: 1,010 drive-hull samples, nine legacy Atlas connections, actual E/Atlas controls; continuous channel and mapped water limits.");
}
