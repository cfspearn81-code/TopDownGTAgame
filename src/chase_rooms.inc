/* ROOM LENS | render-only, north-up perspective dolls' houses.
   Inverse-map horizontal planes at their real elevation. North walls are
   texture-mapped vertically; raised counters occlude the actor on the far
   side, never move the interactable or simulate it a second time. */
#include "chase_room_rig.inc"
typedef struct{int x,y,iz;float scale;} IrPoint;
static struct{float x,y,h,mid;int top,end;const U*tex;} ir;
enum{IR_F=443,IR_RES=1080000};
#define IR_S .51449576f
#define IR_C .85749293f
static U irAux[W*H];
static void ir_begin(float x,float y,int top,int end,const U*tex){
 RoomLens m=ch_room_lens(x,y);
 ir.x=m.x;ir.y=m.y;ir.h=m.h;ir.mid=m.mid;ir.top=top;ir.end=end;ir.tex=tex;
 for(int j=top;j<end;j++){
  U c=nw_blend(0x233848,0x121d29,cl((j-top)*224/(end-top),0,224));
  for(int i=j*W;i<(j+1)*W;i++){pixels[i]=c;chZ[i]=0;}
 }
}
static IrPoint ir_proj(float x,float y,float z0){
 float b=ir.y-y,h=ir.h-z0,d=b*IR_C+h*IR_S;
 if(d<8)return(IrPoint){-10000,-10000,0,0};float f=IR_F/d;
 return(IrPoint){(int)(W*.5f+(x-ir.x)*f),(int)(ir.mid+(h*IR_C-b*IR_S)*f),(int)(IR_RES/d),f};
}
static void ir_location(int*x,int*y,float z0){IrPoint q=ir_proj(*x,*y,z0);*x=q.x;*y=q.y;}
/* Every horizontal y row has an exact inverse; the north-facing room rig
   means u then advances uniformly. No division or transforms per pixel. */
static void ir_hplane(float l,float t,float r,float b,float z0,U ink,int tex){
 IrPoint lo=ir_proj(l,t,z0),hi=ir_proj(l,b,z0);if(!lo.iz)return;
 int y0=max(ir.top,lo.y),end=hi.iz?min(ir.end-1,hi.y):ir.end-1;
 for(int y=y0;y<=end;y++){
  float v=(y+.5f-ir.mid)/IR_F,k=IR_S+v*IR_C;if(k<=.001f)continue;
  float d=(ir.h-z0)/k,wy=ir.y-d*(IR_C-v*IR_S),s=IR_F/d;
  int xl=max(0,(int)(W*.5f+(l-ir.x)*s)),xr=min(W-1,(int)(W*.5f+(r-ir.x)*s));
  if(xl>xr)continue;
  int zi=(int)(IR_RES/d),u=(int)((ir.x+(xl+.5f-W*.5f)/s)*65536.f),du=(int)(65536.f/s);
  int yy=cl((int)wy,0,H-1);
  const U*row=tex&&ir.tex?ir.tex+yy*W:0;
  for(int x=xl,i=y*W+xl;x<=xr;x++,i++,u+=du){
   if(zi>=chZ[i]){chZ[i]=zi;pixels[i]=row?row[cl(u>>16,0,W-1)]:ink;}
  }
 }
}
/* A wall face spans a fixed world Y. Optional U/V coordinates give a
   physical backdrop or display window authored for this venue, never HUD. */
static void ir_front(float l,float r,float y,float low,float high,U ink,int tx,int tw,int ty,int th,const U*tex){
 IrPoint a=ir_proj(l,y,high),b=ir_proj(l,y,low);if(!a.iz||!b.iz||r<=l||high<=low)return;
 for(int py=max(ir.top,a.y);py<=min(ir.end-1,b.y);py++){
  float ray=(py+.5f-ir.mid)/IR_F,den=IR_C-ray*IR_S;if(den<.01f)continue;
  float d=(ir.y-y)/den;if(d<8)continue;
  float sc=IR_F/d,z0=ir.h-d*(IR_S+ray*IR_C);
  int left=max(0,(int)(W*.5f+(l-ir.x)*sc)),right=min(W-1,(int)(W*.5f+(r-ir.x)*sc));
  if(left>right)continue;
  int zi=(int)(IR_RES/d),u=0,step=0;const U*row=0;
  if(tex){int v=cl((int)(ty+(high-z0)*th/(high-low)),ty,ty+th-1);
   row=tex+v*W;step=(int)(65536.f*tw/((r-l)*sc));u=(int)(65536.f*(tx+(ir.x+(left+.5f-W*.5f)/sc-l)*tw/(r-l)));}
  for(int x=left,i=py*W+x;x<=right;x++,i++,u+=step)
   if(zi>=chZ[i]){chZ[i]=zi;pixels[i]=row?row[cl(u>>16,tx,tx+tw-1)]:ink;}
 }
}
static void ir_side(float x,float t,float b,float low,float high,U c){
 /* Each column intersects an X plane along a single vertical line. */
 if(absf(x-ir.x)<.1f)return;
 IrPoint a=ir_proj(x,t,low),bb=ir_proj(x,b,low),cc=ir_proj(x,b,high),dd=ir_proj(x,t,high);
 if(!a.iz||!bb.iz||!cc.iz||!dd.iz)return;
 int left=max(0,min(a.x,min(bb.x,min(cc.x,dd.x)))),right=min(W-1,max(a.x,max(bb.x,max(cc.x,dd.x))));
 for(int sx=left;sx<=right;sx++){
  float k=sx+.5f-W*.5f,d=IR_F*(x-ir.x)/k;if(d<8)continue;
  float h0=(d-(ir.y-t)*IR_C)/IR_S,h1=(d-(ir.y-b)*IR_C)/IR_S;
  if(h1<ir.h-high||h0>ir.h-low)continue;
  if(h0<ir.h-high)h0=ir.h-high;if(h1>ir.h-low)h1=ir.h-low;
  float sc=IR_F/(d*IR_C);
  int s=max(ir.top,(int)(ir.mid+(h0-d*IR_S)*sc)),e=min(ir.end-1,(int)(ir.mid+(h1-d*IR_S)*sc)),zi=(int)(IR_RES/d);
  for(int py=s,i=s*W+sx;py<=e;py++,i+=W)if(zi>=chZ[i]){chZ[i]=zi;pixels[i]=c;}
 }
}
static void ir_box(float x,float y,float w,float h,float z0,float ht,U c,int tex){
 if(w<=0||h<=0||ht<=0)return;
 U front=mix(c,0x101b29,3),side=mix(c,0x132231,4);if(ir.x<x)ir_side(x,y,y+h,z0,z0+ht,side);
 else if(ir.x>x+w)ir_side(x+w,y,y+h,z0,z0+ht,side);
 ir_front(x,x+w,y+h,z0,z0+ht,front,0,0,0,0,0);
 ir_hplane(x,y,x+w,y+h,z0+ht,c,tex);
}
static void ir_block(int x,int y,int w,int d,int h,U c){ir_box(x,y,w,d,0,h,c,0);}
typedef struct{float x,y,z;}IrV;
static void ir_face(IrV a,IrV b,IrV c,IrV d,U ink){
 IrPoint r[]={ir_proj(a.x,a.y,a.z),ir_proj(b.x,b.y,b.z),ir_proj(c.x,c.y,c.z),ir_proj(d.x,d.y,d.z)};
 if(!r[0].iz||!r[1].iz||!r[2].iz||!r[3].iz)return;
 /* Screen clipping in x and y, with reciprocal depth across each span. */
 for(int j=max(ir.top,min(min(r[0].y,r[1].y),min(r[2].y,r[3].y)));j<=min(ir.end-1,max(max(r[0].y,r[1].y),max(r[2].y,r[3].y)));j++){
  float left=1e8f,right=-1e8f,zl=0,zr=0;
  for(int k=0;k<4;k++){IrPoint u=r[k],v=r[(k+1)&3];if(u.y>v.y){IrPoint tmp=u;u=v;v=tmp;}
   if(j<u.y||j>=v.y)continue;float t=(j+.5f-u.y)/(v.y-u.y),x=u.x+t*(v.x-u.x),z0=u.iz+t*(v.iz-u.iz);
   if(x<left){left=x;zl=z0;}if(x>right){right=x;zr=z0;}}
  if(left>=right)continue;float step=(zr-zl)/(right-left);int l=max(0,(int)(left+.5f)),h=min(W-1,(int)(right-.5f));float dep=zl+(l-left)*step;
  for(int x=l,i=j*W+l;x<=h;x++,i++,dep+=step)if((int)dep>=chZ[i]){chZ[i]=(int)dep;pixels[i]=ink;}
 }
}
static void ir_display_car(const InteriorProp*f){
 /* The solid includes the presentation dais; the car itself has a pinched
    nose and tail, tapered cabin, glazing, lower sill and paired wheels. */
 int x=f->x,y=f->y,w=f->w,d=f->h;U p0=f->tint;
 ir_box(x,y,w,d,0,4,0x778588,1);
 float left=x+w*.05f,right=x+w*.95f,mid=y+d*.5f,hw=d*.36f;
 float ax[]={left,left+w*.10f,left+w*.35f,left+w*.71f,right-w*.055f,right};
 float aw[]={hw*.70f,hw*.95f,hw,hw*.98f,hw*.91f,hw*.62f};
 float az[]={10,15,17,16,13,11};
 for(int k=0;k<5;k++){
  float x1=ax[k],x2=ax[k+1],b1=aw[k],b2=aw[k+1],h1=az[k],h2=az[k+1];
  ir_face((IrV){x1,mid+b1,7},(IrV){x2,mid+b2,7},(IrV){x2,mid+b2,h2},(IrV){x1,mid+b1,h1},mix(p0,0x203040,3));
  ir_face((IrV){x1,mid-b1,h1},(IrV){x2,mid-b2,h2},(IrV){x2,mid+b2,h2},(IrV){x1,mid+b1,h1},mix(p0,0xe5dfbf,1));
 }
 for(int i=0;i<2;i++){float bx=x+w*(i?.75f:.24f);
  ir_box(bx-7,mid+hw-4,14,5,4,8,0x202733,0);ir_box(bx-3,mid+hw,6,2,6,4,0x98a6a3,0);}
 float x1=x+w*.27f,x2=x+w*.67f,l=x+w*.36f,r=x+w*.56f,g=hw*.82f,h=hw*.69f;
 ir_face((IrV){x1,mid-g,16},(IrV){l,mid-h,25},(IrV){l,mid+h,25},(IrV){x1,mid+g,16},0x486776);
 ir_face((IrV){r,mid-h,25},(IrV){x2,mid-g,16},(IrV){x2,mid+g,16},(IrV){r,mid+h,25},0x6c9196);
 ir_face((IrV){x1,mid+g,16},(IrV){x2,mid+g,16},(IrV){r,mid+h,25},(IrV){l,mid+h,25},0x314d5d);
 ir_face((IrV){l,mid-h,25},(IrV){r,mid-h,25},(IrV){r,mid+h,25},(IrV){l,mid+h,25},mix(p0,PAPER,2));
 for(int k=-1;k<=1;k+=2)ir_box(right-1,mid+k*hw*.50f-2,2,5,9,2,0xffdfa4,0);
}
static void ir_rail(int x,int y,int w,int height,U c){
 ir_front(x,x+w,y,0,height,c,0,0,0,0,0);
 ir_front(x,x+w,y+.12f,height-2,height,mix(c,PAPER,3),0,0,0,0,0);
}
static void ir_shell(int type,int top,int end,int l,int r,int wh,U wall,U neon,const U*ground){
 const U*old=ir.tex;ir.tex=ground;
 int lastL=l,lastR=r,step=type==1?8:1;
 for(int y=top;y<end;){
  int b=type==0?(y<216?216:y<294?294:y<370?370:end):type==1?min(end,y+step):end;
  if(type==0){l=venue_limit(y,0);r=venue_limit(y,1);}
  else if(type==1){l=wi_shell_edge(y,0);r=wi_shell_edge(y,1);}
  if(y==top){lastL=l;lastR=r;}
  ir_hplane(l,y,r,b,0,wall,1);
  /* Low cutaway wall on the camera side; high trade walls at the rear.
     Side silhouettes terminate precisely at each surveyed footprint step. */
  int sh=type==1?23:27;
  ir_side(l,y,b,0,sh,mix(wall,0xb6c6bd,2));ir_side(r,y,b,0,sh,mix(wall,0x171f2c,3));
  ir_hplane(l-3,y,l,b,sh,mix(wall,neon,3),0);
  ir_hplane(r,y,r+3,b,sh,mix(wall,neon,3),0);
  if(lastL!=l)ir_rail(min(lastL,l),y,max(lastL,l)-min(lastL,l),sh,wall);
  if(lastR!=r)ir_rail(min(lastR,r),y,max(lastR,r)-min(lastR,r),sh,wall);
  if(y==top){
   ir_front(l,r,y,0,wh,wall,l,r-l,max(0,y-wh),wh,old);
   ir_front(l,r,y+.3f,wh-3,wh,mix(wall,neon,4),0,0,0,0,0);
   ir_front(l,r,y+.35f,0,3,mix(wall,0x131c2b,5),0,0,0,0,0);
  }
  y=b;lastL=l;lastR=r;
 }
 /* Cut through the south wall so nearby doors and people can never vanish
    behind a roof. The plinth, sill and four steps indicate the boundary. */
 ir_front(lastL,lastR,end, -13,0,mix(wall,0x17202b,4),0,0,0,0,0);
 ir_front(lastL,lastR,end+.1f,-3,-1,mix(wall,neon,3),0,0,0,0,0);
 int door=cl(480,lastL+34,lastR-34);
 for(int j=0;j<4;j++){
  int y=end+j*7,z0=-3-j*3;
  ir_box(door-40-j,y,80+j*2,7,z0-3,3,mix(wall,0xabb4ad,2),0);
  ir_front(door-39-j,door+39+j,y+7.1f,z0-2,z0-1,mix(wall,neon,3),0,0,0,0,0);
 }
 ir.tex=old;
}
static void ir_label(float x,float y,int z0,const char*s,U c,int always){
 IrPoint q=ir_proj(x,y,z0);if(!q.iz||q.x<12||q.x>W-12||q.y<ir.top+6||q.y>ir.end-6)return;
 if(!always&&chZ[q.y*W+q.x]>q.iz+125)return;
 int w=textw(s,0),left=cl(q.x-w/2-5,3,W-w-13),top=cl(q.y-24,ir.top+2,ir.end-22);
 box(left,top,w+10,21,0x17272e,mix(c,0x24313e,4));textat(left+5,top,s,c,0);
}
static void ir_marker(float x,float y,U c){
 const float u=7,z0=.45f;ir_hplane(x-u,y-u,x+u,y-u+1.6f,z0,c,0);
 ir_hplane(x-u,y+u-1.5f,x+u,y+u,z0,c,0);
 ir_hplane(x-u,y-u,x-u+1.5f,y+u,z0,c,0);ir_hplane(x+u-1.5f,y-u,x+u,y+u,z0,c,0);
}
#include "chase_room_cast.inc"
#include "chase_room_detail.inc"
static void ir_named(void){
 int n=interior_active_loc,k=loc[n].kind,li=legal_index_for_kind(k);
 const InteriorTheme*t=interior_theme(k);U a=t->neon;
 ir_begin(interior_x,interior_y,79,483,interior_back);
 ir_shell(0,145,440,146,814,45,t->wall,a,irNamedFloor);
 id_named_scene(k,li,t);
 for(int i=0;i<interior_nprop;i++)id_fixture(interior_props+i,a,k,1);
 if(k==1&&!interior_subroom){
  id_bottle_row(544,155,103,34,11,2);
  if(v39.tick>=V39_POUR&&v39.tick<V39_HANDOFF&&!v39.playerTick&&!npc_replaces(N_ACT_MIKA))
   id_round(v39Px[v39.patron],V39_BAR_TOP+15,2.5f,2.4f,21,4,.5f,v39.patron?0xe0ab87:0xaee6cf);
 }
 /* Harvest display uses the live unlocked pots and warehouse, not room level. */
 if(k==4&&interior_subroom){
  GrowBatch b=grow_batch(sv.b);float pct=growth_fraction(0);
  for(int j=0;j<b.count;j++){int x=b.pot[j].x,y=b.pot[j].y;
   id_round(x,y,4.3f,4.f,9,4,1.2f,0x9d7e68);
   id_round(x,y,5.f,4.6f,13,.7f,0,0x6a544e);
   if(sv.b[0].timer<=0)continue;
   int h=3+(int)(pct*18.f);U leaf=b.product?0xb9a4d1:0x95b477;
   ir_box(x-.6f,y-.6f,1.2f,1.2f,13.7f,h,mix(leaf,0x6f916c,4),0);
   for(int m=0;m<6;m++){
    float a0=m*1.6183f+j*.77f;
    id_leaf(x,y,13.7f+h*(.22f+.13f*m),a0,2.3f+pct*(4.4f+(m%3)),1.4f+pct*.8f,mix(leaf,0x668673,m%4));
   }
  }
  int cnt=grow_case_count(warehouse_units(0));for(int i=0;i<min(5,cnt);i++){
   int xx=GROW_RACK_X+3+(i%3)*34,zz=20+i/3*11;ir_box(xx,GROW_RACK_Y+4,27,13,zz,11,0xae956a,0);
   ir_box(xx+12,GROW_RACK_Y+4,3,13,zz+11,.6f,0xe5ca8f,0);}
  char s[48];char*q=number(s,warehouse_units(0));append(q," IN STOCK");ir_label(GROW_RACK_X+57,GROW_RACK_Y,38,s,0xe0d5ad,0);
 }
 if(wf_shop_id()>=0){
  ir_block(WF_BENCH_X-33,WF_BENCH_Y-51,66,27,17,0x748566);
  for(int i=0;i<2;i++){int x=WF_BENCH_X-19+i*38,crop=wf_bed_crop(wf_shop_id(),i);
   ir_box(x-8,WF_BENCH_Y-43,16,14,17,4,0x816a51,0);
   if(crop>=0)ir_box(x-4,WF_BENCH_Y-39,8,7,21,wf_bed_left(wf_shop_id(),i)?4:9,0x98b783,0);}
  ir_marker(WF_BENCH_X,WF_BENCH_Y,TEAL);if(wf_bench_near())ir_label(WF_BENCH_X,WF_BENCH_Y,22,"ENTER / PLANTERS",TEAL,1);
 }
 ir_marker(W/2,425,a);ir_marker(interior_desk_x,interior_desk_y,a);
 if(!interior_subroom&&(li>=0||k==4))ir_marker(GROW_GATE_X,GROW_GATE_Y,a);
 if(k==1&&!interior_subroom){ir_marker(V39_STAFF_X,V39_STAFF_Y,a);
  if(v39_staff_visible())ir_label(V39_STAFF_X,V39_STAFF_Y,19,"EMPLOYEES ONLY",a,1);
  if(v39.playerServed)id_round(V39_PLAYER_X,V39_BAR_TOP+V39_BAR_H-8,2.6f,2.45f,21,5,.5f,0xedba96);
 }
 if(k==15){ir_marker(CHIEF_X,CHIEF_Y,a);ir_marker(CUSTODY_X,CUSTODY_Y,GOLD);}
 for(int id=N_ACT_FRONT;id<=N_ACT_GROW;id++){NpcRef r;if(npc_replaces(id)||!npc_ref(id,&r))continue;
  int step=0;if(id>=N_ACT_DANCE&&id<N_ACT_DANCE+3)step=(int)(sn(anim*5+id)*3);
  npc_present_pose(id,&r.x,&r.y,&r.a,&step);
  ir_actor(r.x,r.y,0,r.a,r.coat,r.model,r.hue,step);
 }
 ir_room_actors();
 if(k==4&&interior_subroom&&!npc_replaces(N_ACT_GROW)){
  GrowBatch b=grow_batch(sv.b);GrowPose g=grow_operator_pose(sv.b,&b,growth_fraction(0),grow_pack_left,anim);
  if(g.phase!=GROW_OFF&&!npc_holds(N_ACT_GROW)){
   if(g.phase==GROW_PACK)ir_box(g.x+2,g.y-5,10,7,7,8,0xb18e62,0);
   else if(g.pot>=0&&g.pot<b.count&&!g.walk){
    GrowPot pt=b.pot[g.pot];U c=g.phase==GROW_WATER?0x8ecfd9:g.phase==GROW_FEED?0xd5bf80:0xa6c592;
    for(int j=0;j<4;j++){float z0=(j+(int)(anim*9)%4)%4;ir_box(pt.x-1+(j&1)*4,pt.y+4,2,2,14+z0*2,2,c,0);}
   }
  }
 }
 ir_hero(interior_x,interior_y,interior_a,3);
 if(npcTraceTicks>0)ir_trace();
 if(!int_at_door())ir_label(interior_desk_x,interior_desk_y-13,18,t->counter,a,0);

 if(li>=0&&interior_subroom&&int_at_desk())ir_label(interior_desk_x,interior_desk_y,29,sv.legit[li].pending>0?(sv.legit[li].cooldown>0?"PROCESSING":"COLLECTION READY"):"ACCOUNT DESK",a,1);
}
static void ir_shop(void){
 const WiRole*r=wiRoles+wi_role;U a=r->accent;
 ir_begin(wi_x,wi_y,74,477,wi_back);
 /* UI overlays share the original north-up title and interaction rows. */
 rect(0,0,W,74,0x111e28);rect(0,477,W,H-477,0x111e28);
 ir_shell(1,wi_top,440,wi_left,wi_right,44,wiFinish[wi_role].wall,a,wi_back);
 id_arch(wi_shell_edge(wi_top,0),wi_shell_edge(wi_top,1),wi_top,44,wiFinish[wi_role].wall,a,1,0);
 int lit=wi_sw[walkin_active];
 id_floor_light(wi_f[0].x+wi_f[0].w*.5f,wi_f[0].y+wi_f[0].h+12,105,75,lit?a:0xb1a99b,lit?68:46);
 id_floor_light(480,422,91,39,mix(a,0xc4bca1,4),39);
 id_room_shadows(wi_f,wi_n);id_exit(480,429,a);
 for(int i=0;i<wi_n;i++)id_fixture(wi_f+i,a,100+wi_role,i==0||wi_f[i].type==WI_SCREEN?lit:0);
 ir_marker(W/2,429,a);ir_marker(wi_plaque_x(),418,GOLD);
 ir_hero(wi_x,wi_y,wi_angle,4);
}
static void ir_club_prop(int l,int t,int r,int b,int h,U c){
 ir_box(l,t,r-l,b-t,0,h,c,1);
 ir_front(l+1,r-1,b+.2f,h-2,h,mix(c,ncRose,3),0,0,0,0,0);
}
#include "club_perspective.inc"
static void ir_trap(void){
 /* Authored pixel art is the material atlas. Actors and service labels from
    the overhead pass aren't baked into the playable floor. */
 U*old=surface;surface=irAux;nt_show_floor();surface=old;
 rect(0,0,W,H,0x101a25);ir_begin(nt_x,nt_y,108,453,irAux);
 ir_shell(2,220,448,100,857,85,0x465460,nt_amber,0);
 /* Rectangular warehouse tiles, overhead work-light strips, the real number
    of shelves, shutter banks and relay hardware. */
 for(int i=0;i<12;i++)ir_hplane(101,222+i*19,856,223+i*19,.12f,0x505856,0);
 for(int i=0;i<14;i++)ir_hplane(110+i*55,220,111+i*55,447,.13f,0x3d4d50,0);
 id_arch(100,857,220,78,0x465460,nt_amber,2,480);
 id_floor_light(207,266,114,78,0xb3b096,78);id_floor_light(488,259,116,83,0xc4a877,76);
 id_floor_light(716,259,111,83,0x86ac9f,78);id_exit(480,437,nt_amber);
 for(int i=0;i<4;i++)id_floor_strip(nt_sz[i]-19,nt_sy[i]-11,38,22,0x738179);
 int rack=nt_level(NT_RACK);
 for(int i=0;i<4;i++){int x=120+i*57;
  ir_block(x,215,3,27,16+rack*13,0x536665);
  id_front(x,3,242.15f,0,15+rack*13,0x84928b);}
 for(int j=0;j<=rack;j++){ir_box(119,214,176,28,6+j*13,3,0x8e836a,0);
  for(int i=0;i<5;i++)id_parcel(125+i*32,219,19+i%3,16,9+j*13,6+(i*7+j)%5,i&1?0x7e9488:0xaf9169);}
 ir_club_prop(397,209,564,236,21,0x63767a);ir_club_prop(658,209,817,230,21,0x52666d);
 id_trap_kit(463,215,42,21,0);id_trap_kit(691,215,36,21,1);
 for(int i=0;i<4;i++)ir_marker(nt_sz[i],nt_sy[i],i==nt_nearest()?nt_mint:nt_amber);
 ir_hero(nt_x,nt_y,nt_angle,11);
 for(int i=0;i<3;i++)ir_label(nt_sz[i],nt_sy[i]-23,26,nt_station_name(i),i==nt_nearest()?nt_amber:0xc5c3b5,0);
}
static void ir_aqua(void){
 /* The animated marine habitat is a vertical observation window, not the
    floor. Keep the exhibit's live fish and wildlife clock while walking
    the actual rail; the guided panel mechanics remain regular UI. */
 for(int y=84;y<332;y++)for(int x=42;x<918;x++)irAux[y*W+x]=pixels[y*W+x];
 ir_begin(nvs_rail_point(),332,73,347,0);
 ir_hplane(30,220,940,374,0,0x465e5e,0);ir_front(30,940,221,0,111,0x344b53,0,0,0,0,0);
 ir_front(41,919,221.5f,22,108,0,42,876,84,248,irAux);
 for(int i=0;i<5;i++){
  int x=40+i*219;ir_block(x,220,5,7,109,0x728883);
  id_front(x+1,1,227.15f,24,82,0x9ba99d);
  if(i<4)id_floor_light(x+105,263,103,67,0x68a0a6,73);
 }
 for(int i=0;i<15;i++)id_lid(31+i*61,243,1,130,.16f,0x4c6464);
 for(int i=0;i<5;i++)id_lid(31,251+i*25,910,1,.17f,0x4e6766);
 ir_box(38,222,884,4,19,3,0xa0ab9a,0);ir_hplane(33,363,938,365,.2f,0xcfbe8c,0);
 for(int j=0;j<4;j++){
  int x=119+j*222;U c=nvs_bonus&(1<<j)?TEAL:GOLD;
  ir_box(x-2,257,4,4,0,21,0x849e98,0);
  ir_box(x-12,255,24,6,21,10,0x3d6066,0);id_front(x-10,20,261.2f,23,6,mix(c,0x648481,3));
  for(int i=0;i<=j;i++)id_front(x-8+i*4,2,261.38f,25,2,0x334f59);
  id_floor_strip(x-13,324,27,16,mix(c,0x608181,5));
 }
 id_sofa(709,249,77,16,0x9c8c6f,3);
 NpcRef r;if(!npc_replaces(NAR_INEZ)&&npc_ref(NAR_INEZ,&r))ir_actor(r.x,r.y,0,r.a,r.coat,r.model,r.hue,0);
 ir_room_actors();ir_hero(nvs_rail_point(),332,nvs_aim_angle,10);if(npcTraceTicks>0)ir_trace();
 int s=cl((nvs_position-8)/222,0,3);const char*names[]={"INEZ","LIGHTWELL","NURSERY","EAST BENCH"};
 ir_label(119+s*222,258,35,names[s],nvs_bonus&(1<<s)?TEAL:GOLD,0);
}
