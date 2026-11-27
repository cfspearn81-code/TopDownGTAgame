/* ROOM LENS | render-only, north-up perspective dolls' houses.
   Inverse-map horizontal planes at their real elevation. North walls are
   texture-mapped vertically; raised counters occlude the actor on the far
   side, never move the interactable or simulate it a second time. */
typedef struct{int x,y,iz;float scale;} IrPoint;
static struct{float x,y,h,mid;int top,end;const U*tex;} ir;
enum{IR_F=443,IR_RES=1080000,IR_TILE=112,IR_FOOT=78};
#define IR_S .51449576f
#define IR_C .85749293f
static U irTile[IR_TILE*IR_TILE],irAux[W*H];
static void ir_begin(float x,float y,int top,int end,const U*tex){
 ir.x=480+(x-480)*.82f;ir.y=y+167;ir.h=135;ir.mid=258;ir.top=top;ir.end=end;ir.tex=tex;
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
static int ir_height(int t){
 switch(t){
 case INT_PLANT:case INT_FERNS:case INT_GROWBED:return 9;
 case INT_BED:case INT_CLINIC:case INT_SOFA:case WI_SEATS:case WI_STAGE:return 12;
 case INT_CAR:return 14;case INT_GLASS:return 34;case INT_WALL:case INT_BARS:return 42;
 case INT_LOCKER:case INT_RACK:case WI_AQUA:case INT_COOLER:return 33;
 case INT_FEATURE:case WI_GALLERY:case WI_SCREEN:case WI_TIMETABLE:return 30;
 default:return 20;
 }
}
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
static void ir_fixture(const InteriorProp*f,U a){
 int x=f->x,y=f->y,w=f->w,d=f->h,t=f->type,h=ir_height(t);U c=f->tint?f->tint:a;
 if(t==INT_CAR){ir_display_car(f);return;}
 ir_box(x,y,w,d,0,h,c,1);
 /* Edge bands/feet and individually modelled fixtures add vertical depth;
    authentic address-specific artwork remains on the furniture itself. */
 if(w>28&&h>9){
  ir_front(x+1,x+w-1,y+d+.12f,h-3,h-1,mix(c,PAPER,3),0,0,0,0,0);
  ir_front(x+4,x+w-4,y+d+.15f,1,3,mix(c,0x141a25,5),0,0,0,0,0);
 }
 if(t==INT_DESK||t==INT_CONSOLE||t==INT_SETTLE||t==WI_PRINT||t==INT_DINER){
  int xx=x+w*2/3;ir_box(xx-8,y+d/3,19,8,h,2,0x33414a,0);
  ir_box(xx-3,y+d/3,6,3,h+2,6,0x445560,0);ir_box(xx-11,y+d/3-2,24,4,h+6,14,0x384c57,0);
  ir_front(xx-9,xx+11,y+d/3+2.1f,h+8,h+18,mix(a,0x263b48,3),0,0,0,0,0);
 }else if(t==INT_SOFA||t==WI_SEATS){
  ir_box(x,y,w,min(d/4,8),h,10,c,0);
  ir_box(x,y,5,d,h,5,mix(c,PAPER,1),0);ir_box(x+w-5,y,5,d,h,5,c,0);
 }else if(t==INT_CAR){
  ir_box(x+w/3,y+d/4,w/3,d/2,h,8,mix(c,0x283e50,4),0);
  ir_box(x+w/3+4,y+d/4+4,w/3-8,d/2-8,h+8,2,c,0);
 }else if(t==INT_GUNCASE||t==WI_AQUA||t==INT_COOLER||t==INT_LAB){
  for(int i=4;i<w-9;i+=max(21,w/4)){
   int j=min(w-3,i+max(21,w/4)-4);
   ir_front(x+i,x+j,y+d+.2f,5,h-5,t==WI_AQUA?0x367d82:mix(c,0x29455a,4),0,0,0,0,0);
   ir_front(x+i+2,x+i+4,y+d+.25f,7,h-7,mix(c,0xc8e6d9,4),0,0,0,0,0);
  }
 }else if(t==INT_PLANT||t==INT_FERNS){
  int n=t==INT_PLANT?1:cl(w/38,2,6);
  for(int i=0;i<n;i++){float u=x+(i+.5f)*w/n,v=y+d*.5f;ir_box(u-5,v-4,10,8,h,7,0xa37157,0);
   ir_box(u-2,v-2,4,4,h+7,10,0x537456,0);
   for(int j=0;j<5;j++){float an=j*1.256637f+i*.7f;
    ir_box(u+cs(an)*5-3,v+sn(an)*4-2,7,5,h+10+(j&1)*4,4,j&1?0x80ac79:0x638c67,0);}}
 }else if(t==INT_RACK||t==INT_LOCKER||t==INT_SORTER||t==WI_OVEN){
  int n=cl(w/24,1,8);for(int j=0;j<n;j++){
   int l=x+3+j*(w-6)/n,r=x+3+(j+1)*(w-6)/n-2;if(r<=l)continue;
   ir_front(l,r,y+d+.2f,5,h-5,mix(c,0x293b48,j%2?3:4),0,0,0,0,0);
   ir_front(l+2,min(r,l+9),y+d+.3f,h/2,h/2+2,mix(c,PAPER,4),0,0,0,0,0);
  }
 }
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
static void ir_scratch(void){irCardPass=1;surface=irTile;rw=rh=IR_TILE;for(int i=0;i<IR_TILE*IR_TILE;i++)irTile[i]=0;}
static void ir_put_scaled(float x,float y,float z0,float mul,float tall){
 surface=pixels;rw=W;rh=H;irCardPass=0;
 IrPoint q=ir_proj(x,y,z0);if(!q.iz)return;
 float scale=q.scale*mul;if(scale<.15f)return;
 int width=(int)(IR_TILE*scale),height=(int)(IR_TILE*scale*tall);
 int left=q.x-(int)(IR_TILE*.5f*scale),top=q.y-(int)(IR_FOOT*scale*tall),inv=(int)(65536.f/scale),iv=(int)(65536.f/(scale*tall));
 int xl=max(0,left),xr=min(W,left+width),yt=max(ir.top,top),yb=min(ir.end,top+height);
 for(int sy=yt;sy<yb;sy++){int v=min(IR_TILE-1,((sy-top)*iv)>>16);
  float depth=IR_RES/(float)q.iz,worldHeight=(q.y-sy)/(scale*IR_C);
  int zi=(int)(IR_RES/(depth-worldHeight*IR_S));
  int u=(xl-left)*inv;const U*row=irTile+v*IR_TILE;
  for(int sx=xl,i=sy*W+xl;sx<xr;sx++,i++,u+=inv){U c=row[min(IR_TILE-1,u>>16)];if(c&&zi>=chZ[i]){pixels[i]=c;chZ[i]=zi;}}
 }
}
static void ir_put(float x,float y,float z0,float mul){ir_put_scaled(x,y,z0,mul,1.f);}
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
static void ir_hero(float x,float y,float a,int which){
 if(!pistol_equipped()&&which!=10){
  float u=cs(a),v=sn(a),m=18;IrV l={x+u*m-v*3,y+v*m+u*3,.5f},r={x+u*m+v*3,y+v*m-u*3,.5f},tip={x+u*(m+7),y+v*(m+7),.5f};
  ir_face(l,r,tip,tip,0xf2f1df);
 }
 ir_scratch();
 if(which==3||which==4)player_in_room(x,y,(int)x-IR_TILE/2,(int)y-IR_FOOT,which==3?0xc6d979:0x56baae,player_gait(),a,which==3?3:1,which==3?int_solid:wi_solid);
 else person(IR_TILE/2,IR_FOOT,0x67b0a1,which==11?(nt_walking?((int)(anim*15)&3):0):which==10?(nvs_rail_walk?(int)(nvs_frames%16)/4:0):player_gait(),a,MODEL_PLAYER,1);
 ir_put_scaled(x,y,0,1.14f,1.48f);
}
static void ir_actor(float x,float y,float z0,float a,U c,int model,int hue,int gait){
 ir_scratch();person(IR_TILE/2,IR_FOOT,c,gait,a,model,hue);ir_put_scaled(x,y,z0,1.15f,1.70f);
}
static void ir_trace(float x,float y,float xx,float yy,U c){
 int n=cl((int)(sq(d2(xx-x,yy-y))*.9f),2,550);
 for(int i=0;i<=n;i++){float t=(float)i/n;IrPoint q=ir_proj(x+(xx-x)*t,y+(yy-y)*t,11.5f);
  if(q.x>=0&&q.x<W&&q.y>=ir.top&&q.y<ir.end&&chZ[q.y*W+q.x]<=q.iz+6)pixels[q.y*W+q.x]=c;}
}
static void ir_named(void){
 int n=interior_active_loc,k=loc[n].kind,li=legal_index_for_kind(k);
 const InteriorTheme*t=interior_theme(k);U a=t->neon;
 ir_begin(interior_x,interior_y,79,483,interior_back);
 ir_shell(0,145,440,146,814,45,t->wall,a,irNamedFloor);
 for(int i=0;i<interior_nprop;i++)ir_fixture(interior_props+i,a);
 /* Harvest display uses the live unlocked pots and warehouse, not room level. */
 if(k==4&&interior_subroom){
  GrowBatch b=grow_batch(sv.b);float pct=growth_fraction(0);
  for(int j=0;j<b.count;j++){int x=b.pot[j].x,y=b.pot[j].y;
   ir_box(x-6,y-5,12,10,9,4,0x785c4d,0);int h=3+(int)(pct*18);
   ir_box(x-1,y-1,2,2,13,h,0x86a276,0);
   for(int p0=0;p0<5;p0++){float f=p0*1.55f+j,k0=2+pct*4;
    ir_box(x+cs(f)*k0-2,y+sn(f)*k0-2,5,4,13+h*(.38f+.13f*p0),3,p0&1?0x78ad77:0x68926a,0);}
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
  if(v39.playerServed)ir_box(V39_PLAYER_X-3,V39_BAR_TOP+V39_BAR_H-8,6,5,20,7,0xedba96,0);
 }
 if(k==15){ir_marker(CHIEF_X,CHIEF_Y,a);ir_marker(CUSTODY_X,CUSTODY_Y,GOLD);}
 for(int id=N_ACT_FRONT;id<=N_ACT_GROW;id++){NpcRef r;if(npc_replaces(id)||!npc_ref(id,&r))continue;
  int step=0;if(id>=N_ACT_DANCE&&id<N_ACT_DANCE+3)step=(int)(sn(anim*5+id)*3);
  npc_present_pose(id,&r.x,&r.y,&r.a,&step);
  ir_actor(r.x,r.y,0,r.a,r.coat,r.model,r.hue,step);
 }
 if(npcEnc.phase!=NPC_IDLE&&npcEnc.room==npc_room_key()){
  NpcRef*r=&npcEnc.actor;
  if(npcEnc.phase==NPC_DROP||npcEnc.phase==NPC_RECOVER){
   ir_box(r->x-7,r->y-3,14,6,0,2,r->coat,0);if(npcEnc.prize){ir_box(r->x+2,r->y-4,10,5,2,3,LIME,0);ir_marker(r->x+7,r->y-1,GOLD);}
  }else{modelExtraPunch=npcEnc.phase==NPC_FIGHT?npcEnc.strike:0;
   ir_actor(r->x,r->y,0,r->a,npcEnc.flash>0?0xffdfbc:r->coat,r->model,r->hue,npcEnc.phase==NPC_FIGHT?(int)(sn(anim*18)*4):0);modelExtraPunch=0;
   ir_marker(r->x,r->y,npcEnc.phase==NPC_FIGHT?PINK:GOLD);}
 }
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
 if(npcTraceTicks>0)ir_trace(npcTracePose.mx,npcTracePose.my,npcTraceEndX,npcTraceEndY,GOLD);
 if(!int_at_door())ir_label(interior_desk_x,interior_desk_y-13,18,t->counter,a,0);

 if(li>=0&&interior_subroom&&int_at_desk())ir_label(interior_desk_x,interior_desk_y,29,sv.legit[li].pending>0?(sv.legit[li].cooldown>0?"PROCESSING":"COLLECTION READY"):"ACCOUNT DESK",a,1);
}
static void ir_shop(void){
 const WiRole*r=wiRoles+wi_role;U a=r->accent;
 ir_begin(wi_x,wi_y,74,477,wi_back);
 /* UI overlays share the original north-up title and interaction rows. */
 rect(0,0,W,74,0x111e28);rect(0,477,W,H-477,0x111e28);
 ir_shell(1,wi_top,440,wi_left,wi_right,44,wiFinish[wi_role].wall,a,wi_back);
 for(int i=0;i<wi_n;i++)ir_fixture(wi_f+i,a);
 ir_marker(W/2,429,a);ir_marker(wi_plaque_x(),418,GOLD);
 ir_hero(wi_x,wi_y,wi_angle,4);
}
static void ir_club_guest(float x,float y,float z0,int id,int stance,float time,int face){
 ir_scratch();nc_guest_actor(IR_TILE/2,IR_FOOT,id,stance,time,face,100);ir_put(x,y,z0,1.f);
}
static void ir_crew(float x,float y,float z0,int id,int stance,float time){
 ir_scratch();nc_cast_actor(IR_TILE/2,IR_FOOT,110,ncHouseCast+id,stance,time,1);ir_put(x,y,z0,1.f);
}
static void ir_club_prop(int l,int t,int r,int b,int h,U c){
 ir_box(l,t,r-l,b-t,0,h,c,1);
 ir_front(l+1,r-1,b+.2f,h-2,h,mix(c,ncRose,3),0,0,0,0,0);
}
static void ir_club(void){
 float time=nc_motion();ir_begin(ncX,ncY,78,455,ncBackdrop);
 ir_shell(2,155,435,74,887,55,0x46324b,ncRose,ncBackdrop);
 if(ncRoom==NC_FLOOR){
  ir_club_prop(290,157,671,228,20,0x614151);
  ir_club_prop(78,158,234,263,22,0x615569);ir_club_prop(733,158,885,260,22,0x764959);
  for(int i=0;i<2;i++){int x=i?721:143;ir_club_prop(x,305,x+96,345,12,0x80516a);x=i?836:82;ir_club_prop(x,302,x+42,374,24,0x4f4159);}
  for(int i=0;i<3+nc_get(NC_ACT);i++){
   ir_scratch();nc_stage_actor(IR_TILE/2,IR_FOOT,i,time);ir_put(480+(i*2-(3+nc_get(NC_ACT))+1)*30,209-(i&1)*5,20,1.f);
  }
  /* The bars are working counters, so their crew stand behind the service edge. */
  ir_crew(159,216,22,0,CA_WORKING,time*3.5f);ir_crew(800,216,22,1,CA_WORKING,time*2.9f);
  static const short crowd[][3]={{391,264,5},{554,269,0},{620,275,14},{374,283,1},
   {423,286,9},{529,294,7},{585,299,2},{324,306,11},{470,311,12},{608,320,8},
   {344,327,3},{446,335,6},{513,339,13},{359,348,10},{567,351,4},{645,357,15}};
  int guests=nc_owned()?cl(8+nc_get(NC_DOOR)/4,8,16):12;
  for(int i=0;i<LEN(crowd);i++){int id=crowd[i][2];if(id<guests)ir_club_guest(crowd[i][0]+sn(time*.83f+id*4.2f)*7,crowd[i][1]+sn(time*1.9f+id)*2,0,id,id%3==0?CA_APPLAUSE:id%3==1?CA_SWAY:CA_WAITING,time*(2.6f+id*.07f)+id*1.7f,id%4>=2?-1:1);}
  for(int i=0;i<LEN(ncr_cast);i++){if(i==NCR_VOCALIST&&ncr_active())continue;ir_scratch();
   nc_regular_sprite(i,IR_TILE/2,IR_FOOT,ncr_near()==i);ir_put(ncr_cast[i].x,ncr_cast[i].y,0,1.f);}
  for(int i=0;i<1+nc_get(NC_STAFF);i++)ir_crew(568+i*25,377+i*3,0,i?i+3:2,CA_WAITING,time*1.7f+i);
  if(nc_get(NC_DISPUTE))for(int i=0;i<1+nc_get(NC_DISPUTE);i++)ir_club_guest(610+i*18,396+i*3,0,12+i,CA_APPLAUSE,time*2.6f+i,-1);
  for(int i=0;i<7;i++)ir_box(300+i*60,222,7,2,20,1,(sn(time*2+i)>.2f)?ncIce:ncGold,0);
  ir_marker(160,354,ncRose);ir_marker(793,354,ncGold);
 }else if(ncRoom==NC_SUITE){
  for(int i=0;i<3;i++){
   int x=210+i*266;ir_club_prop(x-106,154,x+92,248,11,ncBoothUph[i]);
   ir_box(x-90,155,171,12,11,24,ncBoothUph[i],0);
   for(int j=0;j<2;j++)ir_club_guest(x+(j?49:-49),213,11,2+i*3+j,CA_SEATED,time*1.5f+i+j,j?-1:1);
   ir_box(x-26,218,52,13,11,4,0xa67b71,0);if(nc_get(NC_SHOWCASE)&(1<<i))ir_box(x-2,221,5,5,15,5,ncBoothInk[i],0);
  }
  ir_club_prop(80,304,210,372,16,0x7a546c);ir_club_prop(743,307,884,363,24,0x947785);
 }else{
  ir_club_prop(84,155,265,211,26,0x665d77);ir_club_prop(294,155,648,248,22,0x776260);
  ir_club_prop(687,155,875,215,28,0x6b6577);ir_club_prop(97,302,240,358,16,0x8b667c);ir_club_prop(730,303,867,360,16,0x8b667c);
  ir_crew(540,192,22,3,CA_WORKING,time*1.43f);
 }
 ir_marker(480,418,ncIce);ir_hero(ncX,ncY,ncAngle,7);
}
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
 int rack=nt_level(NT_RACK);
 for(int i=0;i<3;i++){int x=119+i*20;ir_block(x,215,3,27,29+rack*12,0x536665);}
 for(int j=0;j<=rack;j++){ir_box(119,214,176,28,6+j*13,3,0x8e836a,0);
  for(int i=0;i<5;i++)ir_box(125+i*31,218,19+i%3,16,9+j*13,6+(i*7+j)%5,i&1?0x7e9488:0xaf9169,0);}
 ir_club_prop(397,209,564,236,21,0x63767a);ir_club_prop(658,209,817,230,21,0x52666d);
 ir_box(463,215,42,5,21,17,0x40515d,0);ir_front(465,503,220.2f,25,36,nt_amber,0,0,0,0,0);
 ir_box(691,215,36,4,21,17,0x354650,0);ir_front(693,725,219.2f,26,36,nt_mint,0,0,0,0,0);
 for(int i=0;i<4;i++)ir_marker(nt_sz[i],nt_sy[i],i==nt_nearest()?nt_mint:nt_amber);
 ir_hero(nt_x,nt_y,nt_angle,11);
 for(int i=0;i<3;i++)ir_label(nt_sz[i],nt_sy[i]-23,26,nt_station_name(i),i==nt_nearest()?nt_amber:0xc5c3b5,0);
}
static void ir_aqua(void){
 /* The animated marine habitat is a vertical observation window, not the
    floor. Keep the exhibit's live fish and wildlife clock while walking
    the actual rail; the guided panel mechanics remain regular UI. */
 for(int y=84;y<332;y++)for(int x=42;x<918;x++)irAux[y*W+x]=pixels[y*W+x];
 ir_begin(nvs_position,332,73,347,0);ir.mid=281;ir.h=132;ir.y=510;
 ir_hplane(30,220,940,374,0,0x465e5e,0);ir_front(30,940,221,0,111,0x344b53,0,0,0,0,0);
 ir_front(41,919,221.5f,22,108,0,42,876,84,248,irAux);
 for(int i=0;i<5;i++)ir_block(40+i*219,220,4,5,109,0x728883);
 ir_box(38,222,884,4,19,3,0xa0ab9a,0);ir_hplane(33,363,938,365,.1f,0xcfbe8c,0);
 for(int j=0;j<4;j++){int x=119+j*222;ir_box(x-2,257,4,4,0,21,0x849e98,0);
  ir_box(x-11,256,22,4,21,9,nvs_bonus&(1<<j)?TEAL:GOLD,0);}
 ir_box(710,249,76,16,8,6,0x967b60,0);ir_block(715,251,5,9,8,0x687e7d);ir_block(776,251,5,9,8,0x687e7d);
 ir_actor(151,297,0,3.14f,0xbcb98a,MODEL_STAFF,1,0);
 ir_hero(nvs_position,332,nvs_rail_dir<0?3.14159f:0.f,10);
 int s=cl((nvs_position-8)/222,0,3);const char*names[]={"INEZ","LIGHTWELL","NURSERY","EAST BENCH"};
 ir_label(119+s*222,258,35,names[s],nvs_bonus&(1<<s)?TEAL:GOLD,0);
}
