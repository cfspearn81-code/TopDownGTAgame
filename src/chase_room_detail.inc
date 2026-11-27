/* Room Lens | the geometry stays within the collision footprints surveyed by
   interiors/streetside. Everything here is render-only. Each fixture's task,
   clock, selection and ownership are still owned by the existing game. */
static const U idInk=0x17232d,idBrass=0xcaa275,idSteel=0x8cacae;

static void id_floor_light(float x,float y,float rx,float ry,U ink,int strength){
 /* True world-space pools and contact shadows, clipped to opaque floor.
    Squared range + x/y scan bounds keep their cost proportional to area. */
 IrPoint lo=ir_proj(x,y-ry,0),hi=ir_proj(x,y+ry,0);
 if(!lo.iz||rx<1||ry<1)return;
 int t=max(ir.top,lo.y),b=min(ir.end-1,hi.iz?hi.y:ir.end-1);
 for(int py=t;py<=b;py++){
  float v=(py+.5f-ir.mid)/IR_F,k=IR_S+v*IR_C;if(k<=.001f)continue;
  float d=ir.h/k,dy=(ir.y-d*(IR_C-v*IR_S)-y)/ry,sc=IR_F/d;
  if(dy<=-1.f||dy>=1.f)continue;
  float radius=rx*sq(1.f-dy*dy)*sc,cx=W*.5f+(x-ir.x)*sc;
  float du=1.f/(rx*sc);int l=max(0,(int)(cx-radius)),r=min(W-1,(int)(cx+radius)),z=(int)(IR_RES/d);
  float u=(l+.5f-cx)*du,base=1.f-dy*dy;
  for(int sx=l,i=py*W+l;sx<=r;sx++,i++,u+=du)if(chZ[i]==z){
   float n=base-u*u;if(n<=0)continue;
   int blend=(int)(strength*n*n);if(blend)pixels[i]=nw_blend(pixels[i],ink,blend);
  }
 }
}
static void id_contact(float x,float y,float w,float d){
 id_floor_light(x+w*.5f+5,y+d*.57f+5,w*.66f+7,d*.70f+11,idInk,124);
}
static void id_front(float x,float w,float y,float bot,float h,U c){
 ir_front(x,x+w,y,bot,bot+h,c,0,0,0,0,0);
}
static void id_lid(float x,float y,float w,float d,float z,U c){
 ir_hplane(x,y,x+w,y+d,z,c,0);
}
static void id_stroke(IrV a,IrV b,float r,U c){
 /* Ribbon on a solid surface; suitable for metallic joints and neon inlay. */
 ir_face((IrV){a.x-r,a.y-r,a.z},(IrV){b.x-r,b.y-r,b.z},
         (IrV){b.x+r,b.y+r,b.z},(IrV){a.x+r,a.y+r,a.z},c);
}
static void id_legs(float x,float y,float w,float d,int ht,U c){
 float s=w>90?4.f:3.f;
 for(int j=0;j<2;j++)for(int i=0;i<2;i++)
  ir_box(x+(i?w-s-3:3),y+(j?d-s-3:3),s,s,0,ht,c,0);
}
static void id_round(float x,float y,float rx,float ry,float z,float ht,float flare,U ink){
 /* Low-poly world mesh, not a dimensional sprite. True round silhouettes
    for pottery, drinkware, pillar lamps and small mechanical parts. */
 static const float ax[]={1,.707107f,0,-.707107f,-1,-.707107f,0,.707107f};
 static const float ay[]={0,.707107f,1,.707107f,0,-.707107f,-1,-.707107f};
 IrV bot[8],cap[8];
 for(int j=0;j<8;j++){
  bot[j]=(IrV){x+ax[j]*rx,y+ay[j]*ry,z};cap[j]=(IrV){x+ax[j]*(rx+flare),y+ay[j]*(ry+flare),z+ht};
 }
 for(int j=0;j<8;j++){
  int n=(j+1)&7;
  ir_face(bot[j],bot[n],cap[n],cap[j],mix(ink,j<3?0x243543:0xdde4c9,j<3?3:1));
 }
 for(int j=1;j<7;j++)ir_face(cap[0],cap[j],cap[j+1],cap[j+1],ink);
}
static void id_disc_face(float x,float y,float z,float rx,float rz,U ink){
 static const float q[]={1,.7071068f,0,-.7071068f,-1,-.7071068f,0,.7071068f};
 for(int i=0;i<8;i++){int j=(i+1)&7;
  IrV a={x+rx*q[i],y,z+rz*q[(i+6)&7]},b={x+rx*q[j],y,z+rz*q[(j+6)&7]},m={x,y,z};
  ir_face(m,a,b,b,ink);
 }
}
static void id_soft_box(float x,float y,float w,float d,float z,float ht,U c){
 /* Faceted piping + a bevel inset give upholstered parts actual roundover
    without triangles for the unseen back of the couch. */
 float b=w>22&&d>10?3.f:1.f;
 ir_box(x,y,w,d,z,ht-b,mix(c,0x38414c,2),0);
 ir_box(x+b,y+b,w-2*b,d-2*b,z+ht-b,b,c,0);
 ir_face((IrV){x,y+d,z+ht-b},(IrV){x+w,y+d,z+ht-b},
         (IrV){x+w-b,y+d-b,z+ht},(IrV){x+b,y+d-b,z+ht},mix(c,PAPER,1));
 id_front(x+b,w-2*b,y+d+.1f,z+ht-b-1,1,mix(c,0xc9b8a0,2));
}
static void id_sofa(float x,float y,float w,float d,U c,int rows){
 float back=d*.29f,arm=w>38?6.f:3.f;
 id_legs(x,y,w,d,5,0x685845);
 id_soft_box(x,y,w,d,4,6,mix(c,0x3c343f,4));
 id_soft_box(x,y,w,back,9,17,c);
 id_soft_box(x,y+back,arm,d-back,9,10,c);
 id_soft_box(x+w-arm,y+back,arm,d-back,9,10,c);
 float cw=(w-2*arm)/rows;
 for(int j=0;j<rows;j++){
  float u=x+arm+j*cw;U cap=mix(c,j%2?0xf2c5a8:0xb79e9a,2);
  id_soft_box(u+.8f,y+back,cw-1.6f,d-back-1,10,6,cap);
  if(cw>22&&d>26)id_soft_box(u+cw*.39f,y+back+1,cw*.42f,min((int)(d*.31f),11),16,5,mix(c,0xd7bd99,2+j%2));
 }
}
static void id_sleep(const InteriorProp*f,int clinic){
 float x=f->x,y=f->y,w=f->w,d=f->h;U c=f->tint;
 id_legs(x,y,w,d,6,clinic?idSteel:0x9e785d);
 ir_box(x,y,w,d,6,4,clinic?0x849cab:0x6e4e42,0);
 id_soft_box(x+1,y+1,w-2,d-2,10,6,clinic?0xd1dad3:0xc6b9a7);
 ir_box(x,y,w,4,11,clinic?9:15,clinic?0x91b1b5:0x8d6b5c,0);
 id_soft_box(x+w*.13f,y+5,w*.74f,d*.25f,16,5,clinic?0xc4d9db:0xead7bd);
 id_soft_box(x+2,y+d*.46f,w-4,d*.51f,16,5,c);
 id_lid(x+3,y+d*.61f,w-6,2,21.10f,mix(c,PAPER,3));
 for(int j=1;j<=3;j++)id_lid(x+w*.20f*j,y+d*.51f,1,d*.4f,21.12f,mix(c,idInk,2));
 if(clinic)for(int i=0;i<2;i++){
  float u=x+(i?w-3:2);ir_box(u,y+7,2,d-15,20,2,idSteel,0);
  ir_box(u,y+9,2,2,12,10,idSteel,0);ir_box(u,y+d-12,2,2,12,10,idSteel,0);
 }
}
static void id_books(float x,float y,int w,float z,U c,int seed){
 static const U palette[]={0xc69767,0x996b71,0xa5b08c,0x668f96,0xb0a4b6,0xd5c3a5};
 int d=0;while(d<w-5){
  int p=4+(seed+d*13)%5,h=7+(seed+d*7)%7;U c0=mix(palette[(d+seed)%6],c,2);
  ir_box(x+d,y,p-1,8,z,h,c0,0);id_front(x+d,p-1,y+8.15f,z+3,1,mix(c0,PAPER,4));
  if(p>=6)id_front(x+d+1,p-3,y+8.16f,z+h-3,1,mix(c0,PAPER,3));
  d+=p;
 }
}
static void id_shelves(const InteriorProp*f,U c,int style){
 float x=f->x,y=f->y,w=f->w,d=f->h,b=y+d;int bins=cl((int)(w/28),2,7);
 U wood=style==1?mix(c,0x6f777f,4):mix(c,0x705b4d,4);
 ir_box(x,y,w,4,0,34,wood,0);
 ir_box(x,y,3,d,0,34,wood,0);ir_box(x+w-3,y,3,d,0,34,wood,0);
 for(int j=0;j<3;j++){
  float z=3+j*15;ir_box(x+1,y+3,w-2,d-3,z,2,wood,0);
  id_front(x+2,w-4,b+.1f,z+1,1,mix(wood,idSteel,3));
  if(j==2)continue;
  if(!style){id_books(x+6,b-10,(int)w-12,z+2,c,(int)x+31*j);continue;}
  for(int i=0;i<bins;i++){float u=x+4+i*(w-8)/bins,v=(w-8)/bins-3;
   U ink=mix(c,(style==2?0xac9772:0x516f79),2+((i+j)&1));
   ir_box(u,b-18,v,15,z+2,8,ink,0);id_front(u+2,v-5,b-2.85f,z+5,2,mix(ink,PAPER,4));
   if(style==1)id_front(u+v*.38f,v*.22f,b-2.8f,z+4,1,idInk);
  }
 }
}
static void id_parcel(float x,float y,float w,float d,float z,int h,U c){
 ir_box(x,y,w,d,z,h,c,0);
 id_lid(x+w*.45f,y,w*.12f,d,z+h+.14f,mix(c,0xf3d49d,5));
 id_front(x+w*.45f,w*.12f,y+d+.15f,z+h-4,4,mix(c,0xe4c692,4));
 if(w>15){
  id_lid(x+2,y+2,w*.28f,4,z+h+.18f,0xddd1ab);
  id_front(x+2,w*.28f,y+d+.16f,z+4,3,mix(c,PAPER,4));
 }
}
static void id_glass(const InteriorProp*f,U c){
 float x=f->x,y=f->y,w=f->w,d=f->h;int longX=w>d;
 U trim=mix(c,idSteel,4),edge=mix(c,0x375561,5);
 ir_box(x,y,w,d,0,3,edge,0);ir_box(x,y,w,d,30,2,trim,0);
 if(longX){
  for(int i=0;i<3;i++)ir_box(x+i*(w-3)/2,y,3,d,2,29,trim,0);
  id_front(x+2,w-4,y+d+.07f,6,1,edge);
  id_stroke((IrV){x+4,y+d,10},(IrV){x+w*.32f,y+d,24},.54f,mix(trim,0xbacfd0,3));
 }else{
  for(int i=0;i<3;i++)ir_box(x,y+i*(d-3)/2,w,3,2,29,trim,0);
  ir_face((IrV){x,y+2,13},(IrV){x,y+d-2,23},(IrV){x,y+d-2,24},(IrV){x,y+2,14},trim);
 }
}
static void id_wallprop(const InteriorProp*f,U c){
 float x=f->x,y=f->y,w=f->w,d=f->h;
 if(f->type==INT_BARS){
  ir_box(x,y,w,d,0,3,0x3a4e59,0);
  for(int i=2;i<w-3;i+=9)ir_box(x+i,y+d*.65f,2,3,2,35,idSteel,0);
  ir_box(x,y+d*.65f,w,3,31,3,0x8c9ea6,0);ir_box(x,y+d*.65f,w,3,13,2,0x748a92,0);
  return;
 }
 ir_box(x,y,w,d,0,38,mix(c,0x273c4c,4),0);
 ir_box(x,y,w,d,0,3,mix(c,idInk,4),0);ir_box(x,y,w,d,38,2,mix(c,idSteel,4),0);
 if(w>d)for(int i=8;i<w-3;i+=33)id_front(x+i,1,y+d+.2f,4,33,mix(c,idInk,3));
 else for(int i=8;i<d-3;i+=33)ir_box(x,y+i,w,1,4,33,mix(c,idInk,3),0);
}
static void id_leaf(float x,float y,float z,float a,float len,float span,U ink){
 float u=cs(a),v=sn(a),tx=x+u*len,ty=y+v*len;
 IrV p={x,y,z},tip={tx,ty,z+len*.35f},crest={x+u*len*.48f,y+v*len*.48f,z+len*.66f},
     left={crest.x-v*span,crest.y+u*span,crest.z-1.7f},right={crest.x+v*span,crest.y-u*span,crest.z-1.7f};
 ir_face(p,left,tip,crest,ink);ir_face(p,crest,tip,right,mix(ink,0x33483d,3));
}
static void id_fern(float x,float y,float z,int k,float s,U green){
 float r=5*s;id_round(x,y,r*.74f,r*.67f,z,5*s,r*.25f,0xaa7b5a);
 id_round(x,y,r,r*.9f,z+5*s,1.2f*s,-.3f,0xc39067);
 id_round(x,y,r*.76f,r*.67f,z+6.24f*s,.2f,0,0x5d4f43);
 for(int j=0;j<7;j++){
  float a=j*.8976f+k*.43f;
  id_leaf(x,y,z+6.2f*s,a,(6+(j+k)%4)*s,(2.4f+(j%3)*.35f)*s,mix(green,j%2?0xb7ca7b:0x58835f,3));
 }
}
static void id_console(float x,float y,float z,U c,int enabled){
 ir_box(x-8,y-3,16,9,z,1.3f,0x394650,0);
 ir_box(x-2,y-1,4,3,z+1,5,0x54646c,0);
 ir_box(x-11,y-2,22,3,z+5,13,0x253644,0);
 id_front(x-9,18,y+1.12f,z+7,9,enabled?mix(c,0x557b84,4):0x253445);
 for(int j=0;j<3;j++)id_front(x-7,5+5*(j%2),y+1.18f,z+8+j*2,.65f,enabled?c:0x536772);
 ir_box(x-9,y+4,18,5,z,1.2f,0x46515a,0);
 for(int j=0;j<6;j++)id_lid(x-8+j*2.8f,y+5,1.75f,2,z+1.25f,mix(c,idInk,3));
}
static void id_counter(const InteriorProp*f,U c,int type,int live){
 float x=f->x,y=f->y,w=f->w,d=f->h,b=y+d;U wood=mix(c,0x705647,4),shell=mix(c,0x334958,5);
 ir_box(x+3,y+2,w-6,d-4,0,18,type==INT_DINER?wood:shell,0);
 ir_box(x,y,w,d,18,3,type==INT_DINER?mix(c,0x977052,4):mix(c,0xc6c4b2,4),1);
 id_front(x+2,w-4,b+.1f,18,1,mix(c,PAPER,2));
 int cols=cl((int)(w/22),3,12);float k=(w-12)/cols;
 for(int i=0;i<cols;i++){
  float u=x+6+i*k;
  id_front(u,k-2,b-1.85f,4,type==INT_DINER?11:10,mix(shell,type==INT_DINER?wood:0x728287,2+((i&1)*2)));
  id_front(u+2,k-6,b-1.8f,13,1,mix(c,idSteel,4));
 }
 if(type==INT_SETTLE){
  ir_box(x+w*.28f,y+d*.33f,34,15,21,9,0x46565d,0);
  ir_box(x+w*.28f+4,y+d*.33f+2,26,7,29,3,0x7c8885,0);
  id_front(x+w*.28f+5,24,y+d*.33f+15.15f,25,2,live?TEAL:idSteel);
  for(int j=0;j<3;j++)ir_box(x+w*.67f,y+d*.45f,21,10,21+1.2f*j,.8f,0xb5c4a4,0);
 }else if(type==INT_DINER){
  for(int i=0;i<3;i++){float u=x+8+i*7;ir_box(u,y+4,4,4,21,6+2*(i%2),mix(c,0xd8a67e,4),0);
   ir_box(u+1,y+5,2,2,27+2*(i%2),2,mix(c,0xf4dda8,4),0);}
  ir_box(x+w*.73f,y+5,21,13,21,3,0x34433f,0);
  id_front(x+w*.73f+3,14,y+18.1f,23,1,live?0xa9cf9f:0x668c82);
 }else{
  id_console(x+w*.67f,y+d*.36f,21,c,live);
  for(int j=0;j<3;j++)id_lid(x+8+j*2,y+7+j,17,10,21.2f+j*.09f,0xcac6ad);
 }
}
static void id_cooler(const InteriorProp*f,U c){
 float x=f->x,y=f->y,w=f->w,d=f->h,b=y+d;int n=cl((int)(w/26),2,6);
 ir_box(x,y,w,d,0,31,mix(c,idSteel,4),0);
 for(int j=0;j<n;j++){float u=x+3+j*(w-6)/n,v=(w-6)/n-2;
  id_front(u,v,b+.15f,4,21,0x304856);
  for(int k=0;k<2;k++){id_front(u+3,v-6,b+.23f,8+8*k,1,0x77948b);
   for(int q=0;q<4;q++){U cc=mix(c,(q&1)?0x9bc0b9:0xaca47d,4);
    id_front(u+4+q*(v-7)/4,v/8,b+.25f,9+8*k,4+(q%3),cc);}}
  id_front(u+1,2,b+.28f,13,6,idSteel);
 }
 id_front(x+3,w-6,b+.20f,27,1,mix(c,0xa1d9dc,4));
}
static void id_lockers(const InteriorProp*f,U c){
 float x=f->x,y=f->y,w=f->w,d=f->h,b=y+d;int n=cl((int)(w/19),2,9);
 ir_box(x,y,w,d,0,31,mix(c,0x61717d,3),0);
 for(int j=0;j<n;j++){float u=x+2+j*(w-4)/n,v=(w-4)/n-1;
  id_front(u,v,b+.13f,2,26,mix(c,j%2?0x354a58:0x617583,3));
  for(int k=0;k<3;k++)id_front(u+v*.24f,v*.51f,b+.20f,23-k*2,.65f,0x364956);
  id_front(u+v*.72f,1.8f,b+.30f,13,4,idSteel);
 }
}
static void id_reel(float x,float y,float z,float r,U c){
 id_disc_face(x,y,z,r,r,0xb7a58b);id_disc_face(x,y+.15f,z,r*.71f,r*.71f,0x3d4652);
 for(int j=0;j<3;j++){float a=j*2.0944f+.3f;id_disc_face(x+cs(a)*r*.48f,y+.25f,z+sn(a)*r*.48f,r*.17f,r*.17f,c);}
 id_disc_face(x,y+.3f,z,r*.15f,r*.15f,idBrass);
}
static void id_oven(const InteriorProp*f,U c,int live){
 float x=f->x,y=f->y,w=f->w,d=f->h,b=y+d;int n=w>155?3:2;
 ir_box(x,y,w,d,0,26,mix(c,idSteel,4),0);
 for(int i=0;i<n;i++){float u=x+4+i*(w-8)/n,v=(w-8)/n-3;
  id_front(u,v,b+.14f,5,15,0x39434a);id_front(u+3,v-6,b+.22f,7,10,live?0xba7d50:0x655149);
  for(int k=0;k<3;k++)id_front(u+5+k*(v-8)/3,(v-8)/6,b+.28f,9,4,live?0xe1ad60:0x8d6f53);
  id_front(u+3,v-6,b+.26f,18,2,idSteel);id_front(u+5,4,b+.3f,22,1,live?0xf7d277:0x919a87);
 }
}
static void id_stage(const InteriorProp*f,U c,int live){
 float x=f->x,y=f->y,w=f->w,d=f->h,b=y+d;
 ir_box(x,y,w,d,0,8,mix(c,0x705060,4),1);
 id_front(x+2,w-4,b+.12f,6,1,mix(c,0xdfbd92,3));
 ir_box(x+3,y+2,4,d*.36f,8,22,mix(c,0x715060,3),0);
 ir_box(x+w-7,y+2,4,d*.36f,8,22,c,0);
 ir_box(x+w*.47f,y+d*.68f,10,6,8,1,0x26303d,0);
 ir_box(x+w*.47f+4,y+d*.68f+2,1.7f,1.7f,9,13,idSteel,0);
 ir_box(x+w*.47f+2,y+d*.68f+2,6,2,22,3,live?GOLD:idInk,0);
}
static void id_bike(const InteriorProp*f,U c,int live) {
 float x=f->x,y=f->y,w=f->w,d=f->h,b=y+d*.64f,m=x+w*.51f,s=min((int)(w*.13f),16);
 float ax=x+w*.23f,bx=x+w*.77f;
 ir_box(x+4,y+4,w-8,d-8,0,2,0x475156,0);
 for(int i=0;i<2;i++){float u=i?bx:ax;
  ir_box(u-6,b-4,12,8,2,4,0x6f7a7b,0);
  id_disc_face(u,b,s+5,s,s,0x232630);id_disc_face(u,b+.18f,s+5,s-2,s-2,idSteel);
  id_disc_face(u,b+.28f,s+5,s-3,s-3,0x344850);
  for(int j=0;j<4;j++){float a=j*.785398f+(live?anim*.8f:0);
   float ca=cs(a)*(s-4),sa=sn(a)*(s-4);
   ir_face((IrV){u-ca,b+.32f,s+5-sa},(IrV){u-ca+1,b+.32f,s+5-sa+1},
           (IrV){u+ca+1,b+.32f,s+5+sa+1},(IrV){u+ca,b+.32f,s+5+sa},0x99aba7);
  }
 }
 float stem=m+s*.64f;U a=mix(c,0xab755d,3);
 ir_face((IrV){ax,b+.43f,s+5},(IrV){m-5,b+.43f,s+5},(IrV){m-12,b+.43f,2*s+7},(IrV){ax,b+.43f,s+7},a);
 ir_face((IrV){m-7,b+.44f,s+7},(IrV){stem,b+.44f,2*s+7},(IrV){stem,b+.44f,2*s+10},(IrV){m-9,b+.44f,s+8},c);
 ir_face((IrV){m-12,b+.45f,2*s+8},(IrV){stem,b+.45f,2*s+8},(IrV){stem,b+.45f,2*s+11},(IrV){m-12,b+.45f,2*s+11},a);
 ir_face((IrV){bx,b+.46f,s+5},(IrV){bx+2,b+.46f,s+5},(IrV){stem+2,b+.46f,2*s+16},(IrV){stem,b+.46f,2*s+16},idSteel);
 ir_box(m-20,b-2,15,5,2*s+8,2,0x44383b,0);ir_box(stem-3,b-3,13,6,2*s+15,2,0x617678,0);
}
static void id_washer(const InteriorProp*f,U c,int live){
 float x=f->x,y=f->y,w=f->w,d=f->h,b=y+d;int n=cl((int)(w/42),2,5);
 for(int i=0;i<n;i++){
  float u=x+i*w/n,v=w/n-2,m=u+v*.49f;
  ir_box(u,y,v,d,0,34,mix(c,0x96adaf,i&1?4:6),0);
  id_front(u+3,v-6,b+.16f,26,5,0x384c59);
  id_front(u+6,8,b+.20f,27,2,live?TEAL:GOLD);
  id_disc_face(m,b+.13f,15,11,9,0xa6bab6);id_disc_face(m,b+.30f,15,9,7,0x375964);
  float shift=live?anim*3:0;
  for(int k=0;k<3;k++){float a=k*2.0944f+shift;
   id_disc_face(m+cs(a)*5,b+.45f,15+sn(a)*4,2,1,live?0x88c8c4:0x627e85);}
 }
}
static void id_screen(const InteriorProp*f,U c,int live,int art){
 float x=f->x,y=f->y,w=f->w,d=f->h,b=y+d,high=art?33:43;
 U steel=mix(c,0x667580,5);
 ir_box(x+2,y+2,w-4,d-4,0,5,0x45535e,0);
 ir_box(x+6,y+6,4,d-12,4,high-4,steel,0);ir_box(x+w-10,y+6,4,d-12,4,high-4,steel,0);
 ir_box(x+3,y+d*.55f,w-6,4,10,high-10,0x3c424c,0);
 float fy=y+d*.55f+4.14f;
 id_front(x+7,w-14,fy,13,high-16,live?0xcba787:0x6d8491);
 if(art==1){
  id_disc_face(x+w*.63f,fy+.13f,25,5,4,live?0xe5cb98:0xb5c9b0);
  for(int k=0;k<3;k++){
   float u=x+8+(w-16)*k/3,r=x+8+(w-16)*(k+1)/3;
   ir_face((IrV){u,fy+.24f,13},(IrV){r,fy+.24f,13},(IrV){r-6,fy+.24f,21+k*3},
           (IrV){u+(r-u)*.52f,fy+.24f,18+k*2},mix(c,0x3c5961,3+k));
  }
  id_front(x+8,w-16,fy+.38f,13,3,0x637986);
 }else if(live){
  for(int k=0;k<6;k++)id_front(x+10+k*(w-22)/6,(w-23)/6,fy+.2f,14,9+k%3*3,mix(c,0x343c4f,(k%3)+3));
  id_front(x+w*.35f,w*.30f,fy+.3f,25,2,0xe0d3b4);
 }
}
static void id_transit(const InteriorProp*f,U c,int live){
 float x=f->x,y=f->y,w=f->w,d=f->h,b=y+d;
 ir_box(x+5,y+d*.53f,4,9,0,29,idSteel,0);ir_box(x+w-9,y+d*.53f,4,9,0,29,idSteel,0);
 ir_box(x,y+d*.41f,w,d*.34f,17,24,0x4b4b4d,0);
 float yy=y+d*.75f+.1f;
 id_front(x+3,w-6,yy,20,18,0x27343e);id_front(x+3,w-6,yy+.1f,39,2,mix(c,0xe8b87d,5));
 for(int j=0;j<3;j++)for(int i=0;i<12;i++){
  float u=x+8+i*(w-15)/12;U col=i<8?0xe0bd77:0x7cb4aa;
  id_front(u,2.4f+(j+i+(live?2:0))%5,yy+.2f,23+j*5,1.5f,col);
 }
 (void)b;
}
static void id_tank(const InteriorProp*f,U c,int night){
 float x=f->x,y=f->y,w=f->w,d=f->h,b=y+d,zi=10,hi=34;
 ir_box(x,y,w,d,0,10,0x415b60,0);
 ir_box(x+2,y+2,w-4,d-4,10,23,night?0x346979:0x518486,0);
 ir_box(x,y,w,d,33,2,night?0x659bb4:0x80aaac,0);
 float fy=b+.14f;
 id_front(x+3,w-6,fy,11,21,night?0x305e73:0x416e74);
 for(int i=0;i<7;i++){float xx=x+8+(w-16)*(i+.5f)/7,z=12+i%3;
  float u=xx+sn(anim*.8f+i*1.5f)*5;
  ir_face((IrV){xx,fy+.11f,12},(IrV){xx+2,fy+.11f,12},(IrV){u+3,fy+.11f,z+6},(IrV){u,fy+.11f,z+9},mix(c,0x4d796b,i%2?4:6));
 }
 for(int i=0;i<6;i++){
  float u=x+12+(w-24)*(i+.5f)/6+sn(anim*.9f+i*3.f)*min((int)(w/12),11),z=19+(i*7)%11+sn(anim*1.1f+i)*1.8f;
  float dir=(i&1)?1.f:-1.f;U ink=mix(night?0x8bb8b9:0xe0b785,c,2+i%3);
  id_disc_face(u,fy+.19f,z,3.2f,1.1f,ink);
  IrV t={u-1.6f*dir,fy+.21f,z},l={u-4.6f*dir,fy+.21f,z+1.5f},r={u-4.6f*dir,fy+.21f,z-1.5f};
  ir_face(t,l,r,r,mix(ink,idSteel,3));
 }
 for(int i=0;i<3;i++)id_front(x+1+i*(w-3)/2,2,fy+.33f,zi,hi-zi,mix(c,0x719699,5));
 id_front(x+1,w-2,fy+.36f,32,2,0x8ab2b0);
}
static void id_exhibit(const InteriorProp*f,int k,U c){
 float x=f->x,y=f->y,w=f->w,d=f->h,b=y+d,m=x+w*.5f;
 switch(k){
 case 0:case 6:case 11:case 15:id_transit(f,c,k==6);break;
 case 1:{
  /* Twin DJ platters, mixer and colored channels. */
  id_legs(x,y,w,d,15,0x38404c);ir_box(x,y,w,d,15,5,0x565061,0);
  for(int i=0;i<2;i++){float u=x+w*(.25f+i*.50f),v=y+d*.51f;
   id_round(u,v,12,11,20,1.7f,0,0x34323e);id_round(u,v,7,6,21.7f,.7f,0,0x5d4d5b);
   id_round(u,v,4,4,22.4f,.5f,0,i?0xe49ba6:0x86bdbb);id_stroke((IrV){u+14,v-10,21.8f},(IrV){u+3,v+1,22.3f},.85f,idSteel);
  }
  for(int i=0;i<6;i++)id_lid(m-8+i*3,y+8,1.4f,d-16,20.3f,mix(c,0xcdb1a1,i%3));
  break;
 }
 case 2:{
  ir_box(x,y,w,d,0,17,0x344e54,0);ir_box(x+2,y+2,w-4,d-4,17,2,0x87adad,0);
  for(int i=0;i<7;i++){float u=x+14+i*(w-24)/7,v=y+9+(i*11)%(int)(d-17);U f0=i%2?0x829c97:0x8db4b2;
   id_round(u,v,7,4,19,2,0,f0);
   ir_face((IrV){u-5,v,21},(IrV){u-11,v-3,21},(IrV){u-11,v+3,21},(IrV){u-5,v,21},mix(f0,idSteel,2));
   id_lid(u+3,v-1,1,1,21.14f,idInk);
  }id_front(x+1,w-2,b+.16f,16,2,idSteel);break;
 }
 case 3:case 10:{
  ir_box(x,y,w,d,0,5,0x4b5960,0);ir_box(x+w*.12f,y+8,5,d-16,5,34,0xaa9670,0);
  ir_box(x+w*.80f,y+8,5,d-16,5,34,0xb6a477,0);
  ir_box(x+w*.12f,y+9,w*.73f,5,36,4,0xc2a26b,0);
  if(k==3){ir_box(m-16,y+d*.45f,32,17,9,13,0x667780,0);for(int i=0;i<4;i++)ir_box(m-13+i*7,y+d*.43f,5,17,22,6,0x909c99,0);}
  else{ir_box(m-1,y+10,2,2,23,14,0x5c5554,0);id_disc_face(m,y+13,21,3,4,0xd4b76f);}
  break;
 }
 case 4:{
  id_legs(x,y,w,d,16,0x75654d);ir_box(x,y,w,d,16,3,0x8c8765,0);
  for(int i=0;i<4;i++)id_fern(x+(i+.5f)*w/4,y+d*.54f,19,i,.65f,mix(c,0xa0ad76,4));
  break;
 }
 case 5:{
  ir_box(x,y,w,d,0,17,0x465368,0);
  for(int i=0;i<3;i++){
   float u=x+20+i*(w-36)/3,v=y+d*.5f;
   id_round(u,v,11,10,17,20,1,0x667e89);id_round(u,v,12,11,36,4,-2,mix(c,0xa6b1be,4));
   id_round(u,v,6,6,40,2,0,mix(c,0x6e8c9c,4));id_front(u-4,8,v+10.5f,29,3,c);
  }
  break;
 }
 case 7:{
  ir_box(x,y,w,d,0,22,0x4f4c5d,0);
  id_front(x+6,w-12,b+.14f,8,10,0x333746);for(int i=0;i<5;i++)id_front(x+9,w-18,b+.28f,9+i*2,1,0x737082);
  ir_box(x+4,b-8,w-8,8,22,2,mix(c,0x9c876e,4),0);break;
 }
 case 8:{
  id_legs(x,y,w,d,15,idSteel);ir_box(x,y,w,d,15,3,0x8ea7a8,0);id_console(m,y+d*.49f,18,TEAL,1);
  for(int i=0;i<4;i++)ir_box(x+12+i*7,y+6,5,5,18,4+i%2,0x9eb0a4,0);break;
 }
 case 9:case 13:{
  ir_box(x,y,w,d,0,17,0x715c53,0);ir_box(x,y,w,d,17,3,0xc0a990,0);
  for(int i=0;i<4;i++){
   float u=x+5+i*(w-7)/4,v=(w-7)/4;
   id_front(u,v-2,b+.14f,2,12,i==1?0x667475:0x927260);id_front(u+3,v-8,b+.20f,11,1,idBrass);
  }
  id_lid(x+w*.15f,y+d*.28f,w*.25f,d*.48f,20.3f,0x555b59);
  id_round(x+w*.66f,y+d*.51f,5,5,20,7,1,0xa5a89a);id_round(x+w*.66f,y+d*.51f,5,5,27,3,-1,0xc6bba0);
  for(int i=0;i<2;i++)id_round(x+w*.81f+i*7,y+d*.68f,2,2,20,4,.6f,0xd9c19c);
  break;
 }
 case 14:{
  /* Exhibition target: painted plate, trolley cable and shallow plinth. */
  ir_box(x,y,w,d,0,9,0x475560,0);
  for(int i=0;i<3;i++){float u=x+20+i*(w-30)/3;
   ir_box(u-1,y+d*.33f,2,3,9,18,idSteel,0);
   id_disc_face(u,y+d*.35f,24,8,8,0x9ca79c);
   id_disc_face(u,y+d*.35f+.2f,24,5,5,0x536570);
   id_disc_face(u,y+d*.35f+.3f,24,2,2,0xc19378);}
  break;
 }
 default:id_counter(f,c,INT_DESK,1);break;
 }
}
static void id_fixture(const InteriorProp*f,U a,int k,int live){
 const float x=f->x,y=f->y,w=f->w,d=f->h,m=x+w*.5f,n=y+d*.5f,b=y+d;
 int t=f->type;U c=f->tint?f->tint:a;
 if(t==INT_CAR){ir_display_car(f);return;}
 switch(t){
 case INT_DESK:case INT_CONSOLE:case INT_DINER:case INT_SETTLE:
  if(k==13){id_legs(x,y,w,d,13,0x947457);ir_box(x,y,w,d,13,3,0xa2816a,0);
   id_lid(m-12,n-7,24,14,16.2f,0xe1d0a8);id_lid(m-9,n-3,14,1,16.4f,0xa0816a);id_lid(m-9,n+1,17,1,16.4f,0x8a725e);
   id_round(x+w-11,y+9,4,4,16,5,.5f,0xb5ad90);
  }else id_counter(f,c,t,live);break;
 case INT_SOFA:id_sofa(x,y,w,d,c,cl((int)(w/28),2,6));break;
 case INT_BED:case INT_CLINIC:id_sleep(f,t==INT_CLINIC);break;
 case INT_CRATES:
  {int n0=cl((int)(w/28),2,6);
   for(int i=0;i<n0;i++){
    float u=x+2+i*(w-3)/n0,v=(w-5)/n0-3;
    id_parcel(u,y+1,v,d*.48f,0,10+i%3,mix(c,0xb49a78,2+i%2));
    if(d>28){id_parcel(u+1,n+1,v-1,d*.44f,0,12-i%3,mix(c,0x947c64,2+i%2));
     if(i%3==0&&v>14)id_parcel(u+3,n+3,v-5,d*.34f,12,6,mix(c,0xc8a17a,5));}
   }break;}
 case INT_PLANT:case INT_FERNS:{
  if(t==INT_FERNS){id_legs(x,y,w,d,10,0x667768);ir_box(x,y,w,d,10,3,mix(c,0x748375,4),0);}
  int cols=t==INT_PLANT?1:cl((int)(w/31),2,7),rows=t==INT_FERNS&&d>43?2:1;
  for(int j=0;j<rows;j++)for(int i=0;i<cols;i++){
   float s=t==INT_PLANT?1.23f:.76f+((i+j)%3)*.14f;
   id_fern(x+(i+.5f)*w/cols,y+(j+.5f)*d/rows,t==INT_PLANT?0:13,i+j*2,s,c);}
  break;
 }
 case INT_RACK:case INT_SORTER:case INT_GROW_STOCK:id_shelves(f,c,t==INT_RACK&&(k==13||k==9||k==101||k==118)?0:t==INT_SORTER?1:2);break;
 case INT_LOCKER:id_lockers(f,c);break;
 case INT_GLASS:id_glass(f,c);break;
 case INT_BARS:case INT_WALL:id_wallprop(f,c);break;
 case INT_COOLER:id_cooler(f,c);break;
 case INT_FEATURE:id_exhibit(f,k,c);break;
 case WI_TIMETABLE:id_transit(f,c,live);break;
 case WI_WASH:id_washer(f,c,live);break;
 case WI_AQUA:id_tank(f,c,live);break;
 case WI_OVEN:id_oven(f,c,live);break;
 case WI_SCREEN:case WI_GALLERY:id_screen(f,c,live,t==WI_GALLERY);break;
 case WI_STAGE:id_stage(f,c,live);break;
 case WI_BICYCLE:id_bike(f,c,live);break;
 case WI_SEATS:{
  int nn=cl((int)(w/22),2,9);
  for(int i=0;i<nn;i++)id_sofa(x+i*w/nn,y,w/nn-1,d,mix(c,0x785667,i%2+3),1);
  break;
 }
 case INT_GROWBED:
  ir_box(x+1,y+2,w-2,d-4,0,7,mix(c,0x3b5960,4),0);
  ir_box(x,y,w,d,7,2,mix(c,0x849e9f,5),1);
  id_front(x+2,w-4,b+.15f,7,1,0x9dabaa);id_front(x+2,w-4,b+.16f,3,1,0x566b74);
  break;
 case INT_LAB:
  id_exhibit(f,5,c);break;
 case INT_TRAY:case WI_PARCELS:
  id_legs(x,y,w,d,14,0x4a595c);ir_box(x,y,w,d,14,3,mix(c,0x8d987c,4),0);
  for(int i=0;i<5;i++){
   float u=x+4+i*(w-7)/5,v=(w-7)/5-3;
   id_parcel(u,y+5,v,d-12,17,5+i%3,mix(c,0xb6a076,4));
  }break;
 case INT_REEL:case WI_PROJECTOR:
  ir_box(x+3,y+3,w-6,d-6,0,19,0x585765,0);
  for(int i=0;i<2;i++)id_reel(x+w*(.28f+.38f*i),b-6,28+i%2*5,6,c);
  ir_box(x+w*.7f,y+5,10,13,18,7,0x77797f,0);
  id_front(x+w*.7f+1,7,y+18.15f,20,3,live?0xd4c497:0x45545e);break;
 case WI_DECK:id_exhibit(f,1,c);break;
 case WI_RADIO:
  id_counter(f,c,INT_CONSOLE,live);for(int i=0;i<12;i++)id_lid(x+6+i*3,y+12,1.7f,3+(i*5+(live?(int)(anim*10):2))%10,21.3f,i<9?c:GOLD);break;
 case WI_POTTERY:{
  id_legs(x,y,w,d,14,0x695650);ir_box(x,y,w,d,14,3,0x8f725c,0);
  id_round(m,n,13,11,17,2,0,0x646364);id_round(m,n,7,6,19,5,4,0xb88366);
  id_round(m,n,10,9,24,1,-2,0xc39173);id_round(m,n,8,7,25.16f,.3f,0,0x795b56);
  if(live)id_round(m+cs(anim*4)*7,n+sn(anim*4)*6,1.2f,.9f,25.4f,1,0,0xa87a64);
  break;}
 case WI_BOOK:case WI_TABLE:
  id_legs(x,y,w,d,13,0x8c6e59);id_soft_box(x,y,w,d,13,4,mix(c,0xaa8d69,5));
  if(t==WI_TABLE){id_round(m,n,7,7,17,1,0,0xd1c8ad);id_round(m,n,5,5,18.2f,.3f,0,0xaba48f);id_round(m+14,n-3,2,2,17,4,1,0xc0ad8b);}
  else for(int i=0;i<2;i++){
   float u=m-22+i*22;ir_box(u,n-12,21,26,17,1,live?0xd8c498:0xc1c9b0,0);
   for(int j=0;j<6;j++)id_lid(u+3,n-9+j*3,10+(j+i)%4,.7f,18.2f,0x998d74);
  }break;
 case WI_KITCHEN:id_exhibit(f,9,c);break;
 case INT_GUNCASE:
  ir_box(x,y,w,d,0,18,mix(c,0x475963,6),0);ir_box(x,y,w,d,18,2,mix(c,0x7b9c9e,4),1);
  for(int i=0;i<3;i++)id_front(x+5+i*w/3,w/3-9,b+.16f,5,9,mix(c,0x506f78,5));
  id_front(x+4,w-8,b+.2f,15,2,mix(c,0xb8d2ce,5));break;
 case WI_FABRIC:
  id_legs(x,y,w,d,15,0x6f5a51);ir_box(x,y,w,d,15,4,0x90796b,0);
  for(int i=0;i<4;i++)id_soft_box(x+6,y+4+i*9,w*.26f,8,19,4,mix(c,i%2?0x888e9c:0xae927e,4));
  ir_face((IrV){m-25,n-18,19.2f},(IrV){m+14,n-15,19.2f},(IrV){m+49,n+13,19.2f},(IrV){m-16,n+24,19.2f},live?0xdcc9a4:0x8aa7b1);
  for(int i=0;i<4;i++)id_lid(m-14,n-9+i*5,21+i*2,1,19.4f,0xa69a8a);break;
 case WI_TELESCOPE:{
  ir_box(x+3,y+3,w-6,d-6,0,3,0x45616c,0);
  IrV high={m,n,24},south={m+15,n+13,3},east={m-20,n-6,3},west={m+10,n-13,3};
  id_stroke(high,south,1.8f,idSteel);id_stroke(high,east,1.8f,idSteel);id_stroke(high,west,1.8f,idSteel);
  float dir=live?-1.f:1.f,u=m+dir*29;
  ir_face((IrV){m-dir*20,n+2,23},(IrV){u,n+2,35},(IrV){u,n+2,44},(IrV){m-dir*20,n+2,27},0xa9b1ac);
  ir_face((IrV){m-dir*20,n-3,27},(IrV){u,n-7,44},(IrV){u,n+2,44},(IrV){m-dir*20,n+2,27},0x70868e);
  id_front(u-3,6,n+2.12f,36,6,mix(c,0x7b9baa,3));break;}
 case WI_PRINT:
  ir_box(x,y,w,d,0,18,0x54636a,0);ir_box(x+10,y+6,w*.54f,d*.64f,18,9,0x7d877f,0);
  for(int i=0;i<3;i++)ir_box(x+14+i*w*.16f,y+8,3,d*.57f,27,3,0x4b5c61,0);
  id_lid(m+19,n-4,w*.29f,d*.32f,18.2f,0xe4d4ad);
  for(int i=0;i<4;i++)id_lid(m+24,n-2+i*3,w*.2f-i,1,18.4f,live?c:0x728078);break;
 case WI_KEYS:
  ir_box(x,y,w,d,0,17,0x6a5551,0);
  ir_box(x+4,y+4,w-8,5,17,21,0x977761,0);
  for(int i=0;i<11;i++){float u=x+11+i*(w-22)/10;
   id_disc_face(u,y+9.2f,27+i%2*5,1.4f,1.3f,idBrass);
   id_front(u-.45f,.9f,y+9.3f,23+i%2*5,4,idSteel);
   id_front(u-.4f,1.6f,y+9.36f,23+i%2*5,1,idBrass);}
  break;
 case WI_WEIGHT:
  ir_box(x,y,w,d,0,4,0x4a5460,0);
  for(int i=0;i<2;i++)for(int j=0;j<3;j++){
   float u=x+12+j*(w-21)/3,v=y+(i+.5f)*d/2;int s=5+j;
   ir_box(u,v,17,2,5+s,2,idSteel,0);ir_box(u+1,v-3,2,9,4,2*s,0x3e4651,0);
   ir_box(u+14,v-3,2,9,4,2*s,0x3e4651,0);}
  id_front(x+3,w-6,b+.18f,1,1,live?0xca977b:0x83abae);break;
 case INT_TOOLS:
  id_legs(x,y,w,d,14,idSteel);ir_box(x,y,w,d,14,3,0x7b8d91,0);
  for(int i=0;i<3;i++){
   float u=x+6+i*(w-11)/3;ir_box(u,y+6,12,7,17,5,0x617783,0);
   id_front(u+2,8,y+13.2f,20,1,mix(c,idBrass,4));}
  ir_box(x+w*.73f,y+8,17,14,17,7,0x9a6766,0);
  id_front(x+w*.73f+4,9,y+22.18f,21,1,idSteel);break;
 default:ir_box(x,y,w,d,0,13,c,1);break;
 }
}
static void id_door(float x,float y,int w,int high,U c,int accessible){
 U rim=mix(c,0x60717b,4),fill=accessible?0x2f4650:0x3d4655;
 ir_box(x-w/2,y,w,5,0,high,mix(rim,0x313641,4),0);
 id_front(x-w/2+3,w-6,y+5.17f,3,high-6,fill);
 id_front(x-w/2+4,w-8,y+5.27f,high-13,6,mix(fill,c,3));
 for(int i=-1;i<=1;i+=2)ir_box(x+i*(w/2-2)-1,y+1,3,6,0,high,rim,0);
 id_front(x-w/2,w,y+6.16f,high-3,3,rim);
 id_front(x-w/2+6,w-12,y+6.22f,high-2,1,accessible?c:0xa66c65);
 id_front(x+w*.30f-2,4,y+5.25f,12,2,idBrass);
 id_lid(x-w/2+1,y+6,w-2,4,.39f,rim);
}
static void id_exit(float x,float y,U a){
 U metal=mix(a,0x47565f,5);
 id_lid(x-23,y-7,46,13,.44f,0x405258);
 id_lid(x-23,y+3,46,2,.56f,mix(metal,PAPER,2));
 for(int j=0;j<5;j++)id_lid(x-18+j*8,y-5,2,7,.55f,metal);
 /* Short jambs, rather than a foreground wall across the actor's path. */
 for(int i=-1;i<=1;i+=2){
  float u=x+i*26;
  ir_box(u-2,y-4,4,8,0,13,metal,0);id_front(u-1,2,y+4.12f,10,2,a);
 }
}
static void id_arch(int l,int r,int y,int height,U wall,U a,int pattern,int skip){
 float span=r-l;int n=span>410?5:3;
 for(int j=0;j<n;j++){
  float x=l+(j+.5f)*span/n;if(skip&&absf(x-skip)<49)continue;
  float w=pattern==2?22:16,top=height-9;
  /* Frame rails, triangular sconce and the falloff on the adjacent floor. */
  ir_box(x-w/2,y+1,w,5,top-10,11,mix(wall,0x2c414b,4),0);
  ir_box(x-w/2+2,y+6,w-4,3,top-5,4,mix(a,0xf7dcb2,4),0);
  id_front(x-w/2+3,w-6,y+9.2f,top-6,2,mix(a,PAPER,2));
  id_floor_light(x,y+43,47+(j%2)*14,67,mix(a,0xc6b799,3),34+pattern*7);
 }
 id_front(l+3,r-l-6,y+.16f,5,1,mix(wall,0x526771,4));
}
static void id_floor_strip(int x,int y,int w,int d,U c){
 id_lid(x,y,w,1,.34f,c);id_lid(x,y+d-1,w,1,.34f,c);
 id_lid(x,y,1,d,.34f,c);id_lid(x+w-1,y,1,d,.34f,c);
}
static void id_room_shadows(const InteriorProp*f,int n){
 for(int i=0;i<n;i++)id_contact(f[i].x,f[i].y,f[i].w,f[i].h);
}
static void id_named_scene(int k,int li,const InteriorTheme*t){
 U a=t->neon;int rear=interior_subroom;
 int gate=rear?0:k==1?V39_STAFF_X:k==4||li>=0?GROW_GATE_X:0;
 id_arch(venue_limit(146,0),venue_limit(146,1),145,45,t->wall,a,(k==1||k==5)?2:1,gate);
 /* The active service location guides the eye, while tint/shadows can
    never appear on furniture or over the navigation overlays. */
 id_floor_light(interior_desk_x,interior_desk_y-6,92,69,a,k==1?62:53);
 id_floor_light(480,421,68,32,0xa0af95,42);
 id_room_shadows(interior_props,interior_nprop);
 if(k==1&&!rear){
  id_floor_light(365,264,116,72,0x9866bd,84);
  id_floor_light(408,259,103,67,0x607aa8,59);
  id_floor_light(610,271,102,60,0xc37f8f,61);
  float motion=sn(anim*.68f);
  id_floor_light(331+31*motion,249,73,47,0x9d659b,54);
  id_floor_light(454-27*motion,283,55,53,0x617eac,41);
  id_floor_strip(267,214,226,90,0x8e6995);
 }else if(k==4&&rear){
  id_floor_light(281,255,113,54,0x7ab1ab,61);
  id_floor_light(680,255,113,54,0x8fb6b2,61);
  id_floor_light(281,364,113,50,0x92b596,45);
  id_floor_light(680,365,113,50,0x92b596,45);
 }else if(k==13){
  id_floor_light(320,225,95,84,0xe8c18a,75);
  id_floor_light(541,372,88,64,0xcbac81,63);
  id_floor_strip(417,358,126,42,0xae915f);
 }else if(k==15){
  id_floor_light(CHIEF_X,CHIEF_Y,112,79,0x8dabc4,45);
  id_floor_light(CUSTODY_X,CUSTODY_Y,94,54,0xa68b76,44);
 }
 if(gate)id_door(gate,146,k==1?35:43,33,a,k==4||(k==1?v39_staff_visible():li>=0&&sv.legit[li].owned));
 id_exit(480,426,a);
}
static void id_club_lighting(void){
 U back=ncRoom==NC_FLOOR?0xaf729e:ncRoom==NC_SUITE?0xc6a181:0x9c9db4;
 id_arch(74,887,155,55,0x534158,back,2,0);
 if(ncRoom==NC_FLOOR){
  float wave=sn(nc_motion()*.68f);
  id_floor_light(414+wave*36,257,173,97,0xa86d97,94);
  id_floor_light(543-wave*32,264,164,95,0x575d9f,78);
  id_floor_light(785,275,87,61,0xc5817e,56);
  id_floor_strip(315,235,338,133,0x977793);
  id_floor_strip(317,237,334,129,0x765b7b);
  id_contact(291,157,380,71);id_contact(78,158,156,105);id_contact(733,158,152,102);
  id_contact(143,305,96,40);id_contact(721,305,96,40);
 }else if(ncRoom==NC_SUITE){
  for(int i=0;i<3;i++){int x=210+i*266;id_floor_light(x,256,120,93,ncBoothInk[i],74);id_contact(x-106,154,198,94);}
 }else{
  id_floor_light(476,260,183,105,0xbaad93,66);
  id_contact(294,155,354,93);id_contact(84,155,181,56);id_contact(687,155,188,60);
 }
 id_floor_light(480,418,113,51,back,51);id_exit(480,419,back);
}
static void id_bottle_row(float x,float y,float w,float z,int n,int salt){
 for(int j=0;j<n;j++){
  static const U glass[]={0x647078,0x71918b,0x9d735e,0xb19870,0x937180,0x96a092};
  U c=glass[(j+salt)%6];float u=x+(j+.5f)*w/n;int h=4+(j*3+salt)%7;
  ir_box(u-2,y,4,4,z,h,c,0);ir_box(u-1,y+1,2,2,z+h,2,mix(c,PAPER,2),0);
  id_front(u-1.8f,3.6f,y+4.09f,z+2,2,mix(c,0xd8bf9c,4));
 }
}
static void id_club_counter(int l,int t,int r,int b,U ink,int isbar){
 int w=r-l,d=b-t;float z=20;
 ir_box(l+3,t+1,w-6,d-2,0,17,mix(ink,0x343241,4),0);
 ir_box(l,t,w,d,17,3,mix(ink,0x99797b,4),1);
 id_front(l+2,w-4,b+.17f,16,2,mix(ink,0xad8699,4));
 for(int j=7;j<w-4;j+=12)id_front(l+j,1,b-1+.14f,2,12,mix(ink,0x6c5366,4));
 if(isbar){
  id_bottle_row(l+5,t+7,w-10,z,11,1);
  ir_box(r-24,b-19,15,11,z,2,0x565360,0);
  for(int i=0;i<3;i++){id_round(l+12+i*9,b-14,2.5f,2.3f,z,4,.4f,0xb9a38a);id_front(l+10+i*9,4,b-11.6f,z+2,1,0xe6c19b);}
 }else{
  for(int i=0;i<2;i++)id_round(l+w*(.25f+.45f*i),b-21,13,10,z,2,0,0x534b64);
  for(int i=0;i<6;i++)id_lid(l+w*.50f+i*3,b-25,1.5f,2+i%3,z+.3f,0x97aab4);
 }
}
static void id_club_stage(float time){
 ir_box(290,157,381,71,0,20,0x5c3d4c,1);
 for(int i=0;i<2;i++){int x=i?646:294;
  ir_box(x,160,19,62,20,25,0x493446,0);
  for(int j=0;j<7;j++)ir_box(x+1+j*2,160+j,1.25f,61-j*2,20,25,mix(ncRose,0x573952,j%3+4),0);
  ir_box(x-5,205,13,19,20,15,0x343344,0);
  id_disc_face(x+1,224.15f,27,3.5f,4.5f,0x62566e);
 }
 for(int i=0;i<11;i++){
  U glow=mix(0x8d657c,(i&1)?ncRose:ncGold,3+(int)(sn(time*.55f+i)*2));
  id_front(299+i*34,14,228.15f,16,2,glow);
 }
}
static void id_club_office(void){
 const InteriorProp f[]={ {84,155,181,56,INT_LOCKER,0x697588},{294,155,354,93,INT_DESK,0x968778},
  {687,155,188,60,INT_SORTER,0x7a8190},{97,302,143,56,INT_SOFA,0x855d70},
  {730,303,137,57,INT_CRATES,0xa28778}};
 for(int i=0;i<LEN(f);i++)id_fixture(f+i,ncIce,-1,1);
 for(int i=0;i<3;i++){
  float u=350+i*75;
  id_lid(u,208,34,18,21.26f,0xc2b8a5);id_lid(u+3,210,27,14,21.39f,0xb09d8a);
  for(int j=0;j<4;j++)id_lid(u+5,212+j*3,14+j%2*5,.6f,21.6f,0x79796f);
 }
}
static void id_trap_kit(int x,int y,int w,int z,int variant){
 ir_box(x,y,w,4,z,20,0x485865,0);
 id_front(x+2,w-4,y+4.12f,z+4,13,0x293b44);
 for(int j=0;j<7;j++)id_front(x+4+j*(w-9)/7,1.5f,y+4.21f,z+5,3+((j*7+variant)%6),variant?0x8cbbb3:0xd3ba77);
 id_front(x+3,w-6,y+4.19f,z+15,1,0x617e82);
}
