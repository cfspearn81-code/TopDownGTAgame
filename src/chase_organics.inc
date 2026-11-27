/* Forest material helpers. Scaled geometry and world-derived pigments are
 * stable between walking and driving without touching the terrain grid. */
static U wl_canopy_ink(float x,float y,U leaf,int broad,int city){
 if(city)return nw_blend(leaf,0x6f8778,51);
 /* Far Glen / Moth Meadow read as distinct habitat bands, not new seasons
  * on the save clock. Branches knit the gradients across both boundaries. */
 int marsh=cl((int)(y-2420.f)/2,0,112),west=cl((int)(610.f-x)/3,0,82);
 leaf=nw_blend(leaf,broad?0x959855:0x6d8c67,marsh);
 leaf=nw_blend(leaf,broad?0x6e8c6b:0x406c62,west);
 return leaf;
}
static void wl_bark(float x,float y,float h,float radius,const WlRing*f,float dist,U color){
 float fade=wl_sat((245.f-dist)/90.f);if(fade<.025f)return;
 float r=radius*(.96f+.24f*fade);
 for(int i=0;i<4;i++){
  /* Low roots radiate into the original ground footprint. Bark seams
   * follow world axes with asymmetric tapers rather than screen ribbons. */
  float dx=f->dx[i],dy=f->dy[i],q=radius*(1.1f+fade*.80f);
  ChVec A={x+r*dx,y+r*dy,h*(.20f-.012f*i)*fade};
  ChVec B={x+q*dx+r*.21f*dy,y+q*dy-r*.21f*dx,.14f};
  ChVec C={x+q*dx-r*.29f*dy,y+q*dy+r*.29f*dx,.14f};
  wl_tri(A,B,C,i&1?0x5c5243:0x79674e);
  float z0=h*(.03f+.03f*i),z1=h*(.37f+.03f*i);
  float sx0=radius*dx,sy0=radius*dy;
  ChVec a={x+sx0+dy*.12f,y+sy0-dx*.12f,z0},b={x+sx0+dy*.48f,y+sy0-dx*.48f,z0+h*.025f};
  ChVec c={x+sx0-dy*.16f,y+sy0+dx*.16f,z1};
  wl_tri(a,b,c,nw_blend(color,i%2?0xc09b70:0x3c3e38,(int)(148*fade)));
 }
}
static void wl_hanging_bough(const WlRing*f,float x,float y,float h,float w,U color,float dist,int broad,unsigned seed){
 float detail=wl_sat((435.f-dist)/145.f);if(detail<.03f)return;
 /* Broken profiles, arranged irregularly, never identically around a
  * trunk. Map silhouettes identify conifer and deciduous forms at a glance. */
 int j=(seed>>11)%6,k=(j+2+(seed>>18)%2)%6;
 float dx=f->dx[j]*w,dy=f->dy[j]*w,r=broad?.45f:.44f,ht=broad?.34f:.31f;
 U dim=nw_blend(color,0x284f3e,broad?58:31);
 wl_crown(f,x+dx*.62f,y+dy*.62f,h*ht,w*detail*r+.05f,.93f,h*detail*(broad?.23f:.17f),broad,dim);
 if(broad&&dist<295.f){
  float q=wl_sat((295.f-dist)/105.f);
  wl_crown(f,x+w*.55f*f->dx[k],y+w*.55f*f->dy[k],h*.64f,
    w*.29f*q+.05f,.93f,h*.20f*q,1,nw_blend(color,0xb3aa63,48));
 }
}
static void wl_strata_rock(float x,float y,float r,float h,unsigned seed,int region,float d){
 WlRing ring=wl_ring(seed);
 U low=region?0x666664:0x656e67,high=region?0x8c8980:0x83927e;
 ChVec foot[6],rim[6],tip={x-r*.24f,y+r*.17f,h*.79f};
 for(int j=0;j<6;j++){
  float u=r*ring.dx[j],v=r*ring.dy[j],jag=.85f+((seed>>(j*3))&3)*.067f;
  foot[j]=(ChVec){x+u,y+v*.81f,.15f};
  rim[j]=(ChVec){x+u*.81f*jag,y+v*.59f*jag,h*(.43f+(j%3)*.069f)};
 }
 for(int j=0;j<6;j++){
  int k=(j+1)%6;ChVec a=foot[j],b=foot[k],A=rim[j],B=rim[k];
  int lit=cl((int)(53+37*ring.dy[j]+24*ring.dx[k]),12,151);
  if((ch.x-x)*(ring.dx[j]+ring.dx[k])+(ch.y-y)*(ring.dy[j]+ring.dy[k])>=-.2f*r)
   ch_quad(a,b,B,A,nw_blend(low,0x293837,lit),1);
  wl_tri(A,B,tip,nw_blend(high,0x3d5350,lit));
  float fade=wl_sat((340.f-d)/145.f);
  if(fade>.04f&&((j+seed)%3u==0)){
   ChVec c={A.x+(tip.x-A.x)*.29f,A.y+(tip.y-A.y)*.29f,A.z+(tip.z-A.z)*.29f+.15f};
   ChVec e={B.x+(tip.x-B.x)*.39f,B.y+(tip.y-B.y)*.39f,B.z+(tip.z-B.z)*.39f+.16f};
   A.z+=.1f;B.z+=.14f;
   ch_quad(A,B,e,c,nw_blend(high,region?0xafa58e:0x91a65c,69+(int)(98*fade)),1);
  }
 }
}
static void wl_round_log(float x,float y,float r,float h,int fallen,unsigned seed){
 static const float u[]={.866f,.866f,0,-.866f,-.866f,0},v[]={.5f,-.5f,-1,-.5f,.5f,1};
 float k=fallen?h*.43f:r*.65f,head=fallen?r*1.28f:h*.74f;
 /* Keep the hit-location visible. The wood cap never extends the blocking
  * footprint; irregular height and the rings show age and broken form. */
 ChVec front[6],rear[6];
 for(int i=0;i<6;i++){
  float u0=u[i]*k,v0=v[i]*k,grain=.88f+((seed>>(i*4))&3)*.065f;
  if(fallen){
   front[i]=(ChVec){x+head,y+u0,h*.46f+v0*.85f*grain};
   rear[i]=(ChVec){x-head,y+u0*.84f,h*.56f+v0*.91f*grain};
  }else{
   front[i]=(ChVec){x+u0*grain,y+v0*grain,head+(i%3-1)*.39f};
   rear[i]=(ChVec){x+u0*1.11f,y+v0*1.11f,.16f};
  }
 }
 ChVec cap=fallen?(ChVec){x+head+.08f,y,h*.46f}:(ChVec){x,y,head+.28f};
 for(int i=0;i<6;i++){
  int j=(i+1)%6;ChVec a=front[i],b=front[j];
  U wood=i%3?0x826849:0x685842;
  ch_quad(rear[i],rear[j],b,a,wood,1);
  wl_tri(a,b,cap,i&1?0xaa8055:0xb89468);
  ChVec A={cap.x+(a.x-cap.x)*.62f,cap.y+(a.y-cap.y)*.62f,cap.z+(a.z-cap.z)*.62f},
        B={cap.x+(b.x-cap.x)*.61f,cap.y+(b.y-cap.y)*.61f,cap.z+(b.z-cap.z)*.61f};
  if(fallen){A.x+=.14f;B.x+=.14f;}else{A.z+=.14f;B.z+=.14f;}
  wl_tri(A,B,cap,i&1?0x927d5a:0x9c825c);
 }
}
