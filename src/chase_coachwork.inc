/* Port Vanta coachwork. Same five silhouettes/upgrade slots as the garage.
 * Stations and glass are in local chassis coordinates; no mutable model
 * state, simulation effects or dependencies. Far traffic loses subpixel trim. */
typedef struct {float x,y,c,s,eyeU,eyeV,depth;int lod;} CvRig;
static ChVec cv_p(const CvRig*r,float x,float y,float z){
 return (ChVec){r->x+x*r->c-y*r->s,r->y+x*r->s+y*r->c,z};
}
static void cv_quad(const CvRig*r,ChVec a,ChVec b,ChVec c,ChVec d,U ink,int tint){
 ch_quad(cv_p(r,a.x,a.y,a.z),cv_p(r,b.x,b.y,b.z),cv_p(r,c.x,c.y,c.z),cv_p(r,d.x,d.y,d.z),ink,tint);
}
static void cv_roof(const CvRig*r,float x,float xx,float l,float rr,float ll,float r2,float h,float hh,U ink,int tint){
 cv_quad(r,(ChVec){x,l,h},(ChVec){xx,ll,hh},(ChVec){xx,r2,hh},(ChVec){x,rr,h},ink,tint);
}
static void cv_end(const CvRig*r,float x,float l,float rr,float h,float hh,U ink,int tint){
 cv_quad(r,(ChVec){x,l,h},(ChVec){x,rr,h},(ChVec){x,rr,hh},(ChVec){x,l,hh},ink,tint);
}
static void cv_side(const CvRig*r,int s,float x,float xx,float w,float ww,float low,float high,U ink){
 cv_quad(r,(ChVec){x,s*w,low},(ChVec){xx,s*ww,low},(ChVec){xx,s*ww,high},(ChVec){x,s*w,high},ink,1);
}
/* Elliptical contact shadow multiplies the road already on screen instead
 * of covering lane stripes and pools of light with a black rectangle.
 * Z test preserves any geometry already closer than ground at this pixel. */
static void cv_contact(const CvRig*r,float tail,float nose,float w){
 static const float corner[8][2]={{-1,-.68f},{-.79f,-1},{.77f,-1},{1,-.66f},{1,.66f},{.77f,1},{-.79f,1},{-1,.68f}};
 ChPlot p[8];int top=H,bt=-1;float mid=(tail+nose)*.5f,len=(nose-tail)*.5f+.7f;
 for(int j=0;j<8;j++){
  ChVec v=ch_local(cv_p(r,mid+corner[j][0]*len,.32f+corner[j][1]*(w+.76f),.18f));
  if(v.z<9.5f)return;p[j]=ch_screen(v);top=min(top,p[j].y);bt=max(bt,p[j].y);
 }
 for(int y=max(0,top);y<min(bt,H);y++){
  int l=W,rt=-1;
  for(int j=0;j<8;j++){ChPlot a=p[j],b=p[(j+1)&7];
   if((y>=a.y)==(y>=b.y))continue;int x=a.x+(b.x-a.x)*(y-a.y)/(b.y-a.y);l=min(l,x);rt=max(rt,x);}
  float ray=(CH_MY-y)*(1.f/CH_F);
  int ground=(int)(CH_ZS*(chSin-ray*chCos)/(ch.h*(chSin*chSin+chCos*chCos)))+190;
  for(int x=max(0,l),end=min(rt,W);x<end;x++){int k=y*W+x;
   if(chZ[k]<=ground)pixels[k]=mix(pixels[k],0x101720,3);}
 }
}
static void cv_wheel(const CvRig*r,float u,float w,int s,float rad,U rim){
 static const float dx[]={-.56f,.56f,1,1,.56f,-.56f,-1,-1};
 static const float dz[]={-1,-1,-.51f,.51f,1,1,.51f,-.51f};
 float v=s*(w+.04f),inner=s*(w-1.34f),zc=rad+.05f;
 /* Face away from the eye is skipped, but the tire tread is visible even
  * looking directly along the axle or straight down the boot. */
 for(int j=0;j<8;j++){
  int k=(j+1)&7;float x=u+rad*dx[j],xx=u+rad*dx[k],h=zc+rad*dz[j],hh=zc+rad*dz[k];
  if(((dx[j]+dx[k])*.5f*(r->eyeU-u)+(dz[j]+dz[k])*.5f*(ch.h-zc))<0)continue;
  cv_quad(r,(ChVec){x,v,h},(ChVec){xx,v,hh},(ChVec){xx,inner,hh},(ChVec){x,inner,h},j==3||j==4?0x373d44:0x1e252d,1);
 }
 if(s*r->eyeV<w)return;
 /* Octagons split down the middle stay within the near clipper's 5-vertex input bound. */
 for(int ring=0;ring<(r->lod==2?3:r->lod?2:1);ring++){
  float scale=ring==0?1: ring==1?.66f:.37f,plane=v+s*(ring*.12f);
  U color=ring==0?0x18222a:ring==1?rim:0x394952;
  for(int half=0;half<2;half++){ChVec pts[5];for(int k=0;k<5;k++){int j=(half*4+k)&7;pts[k]=cv_p(r,u+rad*dx[j]*scale,plane,zc+rad*dz[j]*scale);}
   ch_face(pts,5,color,1);}
 }
}
/* Fitted shell/windscreens share all four corners. Trims are inset in
 * surface coordinates (not screen direction), so no floating window bands. */
static void cv_glass(const CvRig*r,float a,float b,float w,float ww,float h,float hh,U base,int isVan){
 cv_roof(r,a,b,-w,w,-ww,ww,h,hh,base,1);
 if(!r->lod)return;
 float dx=b-a,dz=hh-h;
 float s=.12f,e=.89f,wa=w+(ww-w)*s-.60f,wb=w+(ww-w)*e-.60f;
 float u=a+dx*s,v=a+dx*e,z1=h+dz*s+.13f,z2=h+dz*e+.13f;
 if(isVan){
  cv_roof(r,u,v,-wa,-.45f,-wb,-.45f,z1,z2,0x35515c,1);
  cv_roof(r,u,v,.45f,wa,.45f,wb,z1,z2,0x43606b,1);
 }else{
  cv_roof(r,u,v,-wa,wa,-wb,wb,z1,z2,0x345667,1);
  cv_roof(r,u,v,-wa+.14f,-wa*.36f,-wb+.14f,wb*.22f,z1+.065f,z2+.065f,0x577a87,1);
  if(r->lod==2)cv_roof(r,u+dx*.59f,u+dx*.63f,-wb*.75f,wb*.74f,-wb*.75f,wb*.74f,z1+dz*.59f+.12f,z1+dz*.63f+.12f,0x6a8389,1);
 }
}
static void ch_car(const Car*c,int hero){
 ChVec view=ch_local((ChVec){c->x,c->y,6});if(view.z< -20||absf(view.x)>view.z*1.31f+29)return;
 int t=cl(c->type,0,4);const NgShape *g=ngShape+t;float co=cs(c->a),si=sn(c->a),dx=ch.x-c->x,dy=ch.y-c->y;
 CvRig rig={c->x,c->y,co,si,dx*co+dy*si,dy*co-dx*si,view.z,hero||view.z<155?2:view.z<315?1:0};const CvRig*r=&rig;
 NgMods m=c==cars&&c->owner==1?ng_installed(t):(NgMods){0};
 int live=c->cond>0,bits=road_lamps(c);U paint=hero<0?0x8bdbd0:!live?0x49555b:m.paint?ngPaint[m.paint]:specs[t].c;
 U ink=0x24343c,edge=mix(paint,0xf6eee1,3),waist=mix(paint,0x28384b,3),deep=mix(paint,0x172532,5);
 U glass=live?0x1c323f:0x35434c,lip=mix(paint,0x607586,2);
 float w=g->width,tail=g->tail,nose=g->nose,h=g->hood+1.04f,rt=g->roofTail,rn=g->roofNose;
 float back=g->trunk,wind=g->wind,rr=w-(t==4?1.12f:t==3?2.36f:1.80f),roof=g->roofHeight+1.25f;
 /* Five authored stations. Nightjar's wedge nose, Mote's clipped hatch and
    Courier's squared cargo portal have different front/rear rake. */
 float x[]={tail,tail+2.35f,0,nose-3.7f,nose};
 float span[]={w-(t==4?.55f:t==3?1.05f:1.45f),w,w,w-.20f,w-(t==3?3.10f:1.32f)};
 float ht[]={h-.32f,h,h+.08f,h,h-(t==3?1.04f:.39f)};
 if(live&&c->cond<45)ht[4]-=(45-c->cond)*.021f;
 float sh=(t==3?.42f:.64f);
 cv_contact(r,tail,nose,w);
 for(int n=0;n<4;n++){
  float u=x[n],v=x[n+1],a=span[n],b=span[n+1],z=ht[n],zz=ht[n+1];
  for(int s=-1;s<=1;s+=2){
   if(s*r->eyeV< -1.2f)continue;
   cv_quad(r,(ChVec){u,s*(a-.45f),1.34f},(ChVec){v,s*(b-.45f),1.34f},(ChVec){v,s*b,zz-sh},(ChVec){u,s*a,z-sh},s<0?waist:deep,1);
   cv_quad(r,(ChVec){u,s*a,z-sh},(ChVec){v,s*b,zz-sh},(ChVec){v,s*(b-.62f),zz},(ChVec){u,s*(a-.62f),z},s<0?edge:lip,1);
  }
  cv_roof(r,u,v,-a+.62f,a-.62f,-b+.62f,b-.62f,z,zz,paint,1);
 }
 for(int s=-1;s<=1;s+=2){
  if(s*r->eyeV< -w-2)continue;
  cv_wheel(r,tail+5.0f,w,s,t==3?2.35f:2.64f,ngRims[m.rim]);cv_wheel(r,nose-5.1f,w,s,t==3?2.35f:2.64f,ngRims[m.rim]);
  cv_quad(r,(ChVec){back,s*(w-.66f),h},(ChVec){wind,s*(w-.66f),h},(ChVec){rn,s*rr,roof},(ChVec){rt,s*rr,roof},waist,1);
  if(r->lod){
   float mid=t==3?-.75f:-1.4f,wb=w-.66f-.10f,ww=rr+.28f;
   /* Side glazing separated by a body-color B-pillar; solid cargo walls. */
   if(t!=4)cv_quad(r,(ChVec){back+1.28f,s*wb,h+.86f},(ChVec){mid-.47f,s*wb,h+.86f},(ChVec){mid-.47f,s*ww,roof-.54f},(ChVec){rt+.65f,s*ww,roof-.54f},glass,1);
   cv_quad(r,(ChVec){mid+.36f,s*wb,h+.86f},(ChVec){wind-1.23f,s*wb,h+.86f},(ChVec){rn-.51f,s*ww,roof-.54f},(ChVec){mid+.36f,s*ww,roof-.54f},0x476675,1);
   cv_side(r,s,-2,t==3?.85f:1.5f,w+.10f,w+.10f,2.51f,2.85f,edge);
   cv_side(r,s,wind-2.2f,wind-.88f,w+.77f,w+.94f,h+.33f,h+.91f,waist);
  }
 }
 cv_glass(r,back,rt,w-.65f,rr,h+.09f,roof, t==4?paint:glass,t==4);
 cv_glass(r,wind,rn,w-.65f,rr,h+.12f,roof,glass,0);
 cv_roof(r,rt,rn,-rr,rr,-rr,rr,roof+.05f,roof+.05f,mix(paint,0xe2e8db,1),1);
 if(r->lod){
  U trim=mix(paint,0x394953,3);float z=roof+.13f;
  if(t==0){ /* short hatch, moulded liftback lip */
   cv_roof(r,rt-.55f,rt+.42f,-rr-.28f,rr+.28f,-rr,rr,z+.20f,z+.34f,lip,1);
  }else if(t==1){ /* saloon sunroof and revealing weather seal */
   cv_roof(r,rt+1.25f,rn-.70f,-rr+.97f,rr-.97f,-rr+.97f,rr-.97f,z,z,ink,1);
   cv_roof(r,rt+1.54f,rn-1.06f,-rr+1.23f,rr-1.23f,-rr+1.23f,rr-1.23f,z+.08f,z+.08f,0x526d78,1);
  }else if(t==2){for(int s=-1;s<=1;s+=2)cv_roof(r,rt+.2f,rn-.1f,s*(rr-.23f),s*(rr-.88f),s*(rr-.23f),s*(rr-.88f),z+.30f,z+.30f,0xb7c1b8,1);
  }else if(t==4){for(int j=-1;j<=1;j++)cv_roof(r,rt+1.f,rn-1.45f,2.45f*j-.21f,2.45f*j+.21f,2.45f*j-.21f,2.45f*j+.21f,z,z,trim,1);}
  /* Pressed hood contours converge toward the grille instead of a flat
   * board between lamps. The van uses cowl slots; the coupe has vents. */
  for(int side=-1;side<=1;side+=2){
   float a=w*(t==3?.66f:.73f)*side,b=w*(t==4?.71f:.43f)*side;
   cv_roof(r,wind+.65f,nose-3.7f,a,a+.24f*side,b,b+.24f*side,h+.18f,h+.11f,edge,1);
   if(t==3)cv_roof(r,wind+2.05f,wind+4.90f,side*2.6f,side*3.86f,side*2.72f,side*3.56f,h+.17f,h+.17f,deep,1);
   else if(t==4)cv_roof(r,wind+.43f,wind+1.1f,side*.44f,side*3.57f,side*.44f,side*3.57f,h+.19f,h+.19f,deep,1);
  }
  if(t==3||m.paint==7){
   for(int s=-1;s<=1;s+=2){float l=s*.28f,v=s*1.08f;
    cv_roof(r,rt+.1f,rn-.1f,l,v,l,v,z+.07f,z+.07f,0xf7ddb6,0);
    cv_roof(r,wind+.5f,nose-3.7f,l,v,l,v,h+.14f,h+.14f,0xe4d1ad,1);
    cv_roof(r,tail+2.4f,back-.14f,l,v,l,v,h+.14f,h+.14f,0xe4d1ad,1);}
  }
  if(m.kit||t==3)cv_roof(r,tail-.12f,tail+1.20f,-w+.63f,w-.63f,-w+.72f,w-.72f,h+.40f+m.kit*.60f,h+.46f+m.kit*.60f,m.kit?ink:lip,1);
 }
 for(int end=-1;end<=1;end+=2){
  if(end*r->eyeU< -5)continue;
  int rear=end<0,idx=rear?0:4;float u=x[idx],a=span[idx],z=ht[idx],e=u+end*.13f;
  cv_end(r,u,-a+.1f,a-.1f,1.48f,z,waist,1);
  cv_end(r,e,-a+.18f,a-.18f,1.30f,2.25f,ink,1);
  cv_end(r,e+end*.09f,-a+.72f,a-.72f,2.13f,2.45f,mix(paint,0xafc0be,3),1);
  if(r->lod){
   cv_end(r,e,-(rear?2.18f:a*.44f),rear?2.18f:a*.44f,2.48f,z-.42f,ink,1);
   if(rear){cv_end(r,e+end*.16f,-1.64f,1.64f,2.60f,3.48f,ngTags[m.tag],1);
    if(t==4)cv_end(r,e+end*.11f,-.19f,.19f,3.68f,z-.14f,deep,1);}
   else if(t==0){cv_end(r,e+end*.12f,-a*.34f,a*.34f,2.83f,3.1f,0x758e8b,1);}
   else if(t==2){for(int sl=-2;sl<=2;sl++)cv_end(r,e+end*.14f,sl*.78f-.13f,sl*.78f+.13f,2.59f,z-.47f,0xb4aba0,1);}
   else if(t==4){cv_end(r,e+end*.12f,-a*.39f,a*.39f,2.88f,3.22f,0x758581,1);}
   else cv_end(r,e+end*.12f,-a*.28f,a*.28f,2.62f,2.88f,0x81928f,1);
   if(!rear||t!=3)cv_end(r,e+end*.20f,-.39f,.39f,z-.72f,z-.32f,0xb6beb5,1);
  }
  for(int s=-1;s<=1;s+=2){
   float outer=a-.40f,inner=a-(t==3?3.25f:t==4?1.82f:2.72f),low=z-(t==4?1.93f:1.24f);
   U lamp=!live?(rear?0x663e42:0x738080):rear?(bits&1?0xff5148:0xc84b44):0xffe4a2;
   cv_end(r,e+end*.11f,s*outer,s*inner,low,z-.13f,lamp,!live);
   if(live&&r->lod==2){
    if(t==3)cv_end(r,e+end*.19f,s*(inner+1.38f),s*(inner+1.71f),low,z-.11f,ink,1);
    if(rear&&(bits&2))cv_end(r,e+end*.20f,s*(inner+.10f),s*(inner+.80f),low,z-.12f,0xe2faff,0);
   }
  }
  if(rear&&live&&(bits&1)&&r->lod)cv_end(r,e+end*.17f,-1.42f,1.42f,z-.49f,z-.12f,0xff6555,0);
 }
}
