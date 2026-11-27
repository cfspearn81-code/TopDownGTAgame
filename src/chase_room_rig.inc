/* Interior presentation follows real movement at 60Hz, never frame paints.
   One north-up profile serves the floor, people, markers, and collision-
   occluded material. HUD, inputs, shooting and saves remain in room space. */
typedef struct {int key,view;float x,y,boom,high,mid;const InteriorProp *props;int n;} RoomMount;
typedef struct {float x,y,h,mid;} RoomLens;
static struct {int key;float px,py,x,y,lookX,lookY,entry,lift;} roomRig;
/* Broad opaque fixtures only. Glass panes, open railings, tables and
   skinny displays shouldn't move the camera even when they have tall trim. */
static float ch_room_prop_crest(int type){
 switch(type){
 case INT_WALL:return 40.f;
 case WI_SCREEN:return 43.f;
 case WI_TIMETABLE:return 41.f;
 case INT_RACK:case INT_SORTER:case INT_GROW_STOCK:return 34.f;
 case WI_AQUA:return 35.f;
 case WI_WASH:return 34.f;
 case WI_GALLERY:case INT_LOCKER:case INT_COOLER:return 31.f;
 default:return 0.f;
 }
}
static RoomMount ch_room_mount(int view){
 RoomMount m={.view=view,.boom=150,.high=122,.mid=254};
 if(view==3&&interior_active_loc>=0&&interior_active_loc<LEN(loc)){
  m.key=1+2*interior_active_loc+!!interior_subroom;
  m.x=interior_x;m.y=interior_y;m.props=interior_props;m.n=interior_nprop;
 }else if(view==4&&walkin_active>=0&&walkin_active<nb){
  m.key=128+walkin_active;m.x=wi_x;m.y=wi_y;m.props=wi_f;m.n=wi_n;
  if(tram_hall(walkin_active)){m.boom=163;m.high=130;}
#if __has_include("night_club.inc")
 }else if(view==NIGHT_CLUB_SCENE){
  m.key=1024+ncRoom;m.x=ncX;m.y=ncY;m.boom=167;m.high=133;m.mid=248;
#endif
 }else if(view==NIGHT_TRAP_SCENE){
  m.key=1040;m.x=nt_x;m.y=nt_y;m.boom=166;m.high=135;m.mid=246;
#if __has_include("night_venues.inc")
 }else if(view==NIGHT_VENUES_SCENE&&nvs_room==NVR_GLASS&&nvs_mode==4){
  m.key=1041;m.x=nvs_rail_point();m.y=332;m.boom=178;m.high=132;m.mid=281;
#endif
 }
 return m;
}
static float ch_room_x(RoomMount m){return W*.5f+(m.x-W*.5f)*.82f;}
static float ch_room_y(RoomMount m){
 /* Show where the aisle leads without pinning feet to an exact pixel. Near
    the north side, study the front of a counter; at the exit see the sill. */
 return m.key==1041?m.y:m.y+ch_clampf((345.f-m.y)*.33f,-26.f,57.f);
}
static float ch_room_cover(RoomMount m,float x,float y){
 /* Only a fixture actually on the line from hero to camera can lift the
    lens. Normal side occlusion remains; props still keep their real depth. */
 float lift=0.f,dy=y-m.y,dx=x-m.x;
 if(dy<30.f)return 0.f;
 for(int i=0;i<m.n;i++){
  const InteriorProp*f=m.props+i;
  if(f->y+f->h<=m.y||f->y>m.y+37.f)continue;
  float along=ch_clampf((f->y-m.y)/dy,.015f,.33f),px=m.x+along*dx;
  if(px<f->x-3||px>f->x+f->w+3)continue;
  float need=(ch_room_prop_crest(f->type)+3.f-(22.f+along*(m.high-22.f)))*1.35f;
  if(need>lift)lift=need;
 }
 return ch_clampf(lift,0,22);
}
static void ch_room_step(void){
 if(!ch_mode_active()){roomRig.key=0;return;}
 /* Guide and Signal are presented over/from existing rooms. Don't replay
    the doorway reveal or move the room behind one of those static screens. */
 if(scene==2||scene==NIGHT_PHONE_SCENE||(scene==NIGHT_CLUB_SCENE&&menu==PAUSE_MENU))return;
 RoomMount m=ch_room_mount(scene);
 if(!m.key){roomRig.key=0;return;}
 float dx=m.x-roomRig.px,dy=m.y-roomRig.py;
 if(m.key!=roomRig.key||d2(dx,dy)>90.f*90.f){
  /* Enter already facing the correct room, with a short wide reveal.
     Never interpolate positions between a street and an indoor coordinate. */
  roomRig.key=m.key;roomRig.x=ch_room_x(m);roomRig.y=ch_room_y(m);
  roomRig.lookX=roomRig.lookY=0;roomRig.entry=1.f;
  roomRig.lift=ch_room_cover(m,roomRig.x,roomRig.y+m.boom);
 }else{
  roomRig.lookX+=(ch_clampf(dx*4.2f,-13.f,13.f)-roomRig.lookX)*.22f;
  roomRig.lookY+=(ch_clampf(dy*3.5f,-9.f,9.f)-roomRig.lookY)*.20f;
  roomRig.x+=(ch_room_x(m)+roomRig.lookX-roomRig.x)*.24f;
  roomRig.y+=(ch_room_y(m)+roomRig.lookY-roomRig.y)*.23f;
  float lift=ch_room_cover(m,roomRig.x,roomRig.y+m.boom);
  roomRig.lift+=(lift-roomRig.lift)*(lift>roomRig.lift?.38f:.12f);
  roomRig.entry*=.82f;if(roomRig.entry<.003f)roomRig.entry=0;
 }
 roomRig.px=m.x;roomRig.py=m.y;
}
static RoomLens ch_room_lens(float heroX,float heroY){
 int view=scene==2?helpBack:scene;
 RoomMount m=ch_room_mount(view);
 if(!m.key)return(RoomLens){W*.5f+(heroX-W*.5f)*.82f,heroY+167,135,258};
 int sample=m.key==roomRig.key&&d2(m.x-roomRig.px,m.y-roomRig.py)<=90.f*90.f;
 float x=sample?roomRig.x:ch_room_x(m),y=sample?roomRig.y:ch_room_y(m);
 float in=sample?roomRig.entry:1.f,lift=sample?roomRig.lift:ch_room_cover(m,x,y+m.boom);
 return(RoomLens){x,y+m.boom+in*23.f-lift*.39f,m.high+in*14.f+lift,m.mid};
}
