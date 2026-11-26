/* Behind-the-car street view. An optional render-only perspective of the
 * existing city, not a second simulation. The same street paths, collisions,
 * car selection, fixtures and saved careers drive both views. */
enum{CH_F=378,CH_MY=206,CH_ZS=1100000};
typedef struct{float x,y,z;} ChVec;
typedef struct{int x,y,z;} ChPlot;
static struct {float x,y,h,si,co,range,height;int ride;} ch={.ride=-1};
static int chProjected;
static float chDark,chFog;
static int chZ[W*H];
static float chSin=.249f,chCos=.9685f;
static float chFit;
static int ch_pref(void){return nv.garage[60]==1u;}
static int ch_enabled(void){return ch_pref()&&!fastGfx&&p.car>=0&&p.car<nc&&!p.dead;}
static void ch_motion(float speed,float err){
 float unit=speed/205.f;if(unit>1)unit=1;
 float bend=absf(err);if(bend>.8f)bend=.8f;
 float wanted=79.f+18*unit+30*ngCamBurst-20*bend;
 float high=38.f+5*unit+11*bend;
 if(ch.ride!=p.car||ch.range<1){ch.range=79;ch.height=38;}
 ch.ride=p.car;ch.range+=(wanted-ch.range)*.073f;ch.height+=(high-ch.height)*.071f;
}
static ChVec ch_local(ChVec q){
 float dx=q.x-ch.x,dy=q.y-ch.y,dz=q.z-ch.h,b=dx*ch.co+dy*ch.si;
 return (ChVec){dy*ch.co-dx*ch.si,dz*chCos+b*chSin,b*chCos-dz*chSin};
}
static ChPlot ch_screen(ChVec a){
 float f=CH_F/a.z;return (ChPlot){W/2+(int)(a.x*f),CH_MY-(int)(a.y*f),(int)(CH_ZS/a.z)};
}
static void ch_project(int*x,int*y){
 ChVec v=ch_local((ChVec){*x+cx,*y+cy,3});
 if(v.z<13){*x=v.x<0?-44:W+44;*y=244;return;}
 ChPlot q=ch_screen(v);*x=q.x;*y=q.y;
}
static U ch_dye(U c,float distance){
 if(chDark>.01f)c=nw_blend(c,0x141d30,(int)(100*chDark));
 int fog=cl((int)(distance*.14f+chFog*distance*.29f),0,213);
 return nw_blend(c,chDark>.5f?0x303648:0x9cb0af,fog);
}
static long long ch_edge(ChPlot a,ChPlot b,int x,int y){return (long long)(b.x-a.x)*(y-a.y)-(long long)(b.y-a.y)*(x-a.x);}
static void ch_triangle(ChPlot a,ChPlot b,ChPlot c,U color){
 long long area=ch_edge(a,b,c.x,c.y);if(!area)return;
 int lo=max(0,min(a.x,min(b.x,c.x))),hi=min(W-1,max(a.x,max(b.x,c.x)));
 int top=max(0,min(a.y,min(b.y,c.y))),bottom=min(H-1,max(a.y,max(b.y,c.y)));
 if(lo>hi||top>bottom)return;
 int sign=area>0?1:-1;
 long long q=ch_edge(b,c,lo,top)*sign,r=ch_edge(c,a,lo,top)*sign,s=ch_edge(a,b,lo,top)*sign;
 int qx=(b.y-c.y)*sign,rx=(c.y-a.y)*sign,sx1=(a.y-b.y)*sign;
 int qy=(c.x-b.x)*sign,ry=(a.x-c.x)*sign,sy1=(b.x-a.x)*sign;
 float f=1.f/absf(area),sz=(a.z*(float)q+b.z*(float)r+c.z*(float)s)*f;
 float zx=(a.z*(float)qx+b.z*(float)rx+c.z*(float)sx1)*f,zy=(a.z*(float)qy+b.z*(float)ry+c.z*(float)sy1)*f;
 for(int y=top;y<=bottom;y++,q+=qy,r+=ry,s+=sy1,sz+=zy){
  long long aa=q,bb=r,cc=s;int i=y*W+lo;float zz=sz;
  for(int x=lo;x<=hi;x++,aa+=qx,bb+=rx,cc+=sx1,zz+=zx,i++)
   if((aa|bb|cc)>=0&&(int)zz>=chZ[i]){chZ[i]=(int)zz;pixels[i]=color;}
 }
}
static void ch_face(const ChVec *p0,int n,U c,int shade){
 ChVec i[5],o[6];for(int j=0;j<n;j++)i[j]=ch_local(p0[j]);
 float d=0;int m=0;ChVec a=i[n-1];
 /* Clip in view space first: behind-camera geometry cannot invert across
  * screen and a close street wall cannot cover the opposite horizon. */
 for(int j=0;j<n;j++){
  ChVec b=i[j];if((a.z>=9.f)!=(b.z>=9.f)){
   float f=(9.f-a.z)/(b.z-a.z);o[m++]=(ChVec){a.x+(b.x-a.x)*f,a.y+(b.y-a.y)*f,9.f};
  }
  if(b.z>=9.f)o[m++]=b;
  a=b;
 }
 if(m<3)return;for(int j=0;j<m;j++)d+=o[j].z;
 if(shade)c=ch_dye(c,d/m);
 ChPlot p[6];for(int j=0;j<m;j++)p[j]=ch_screen(o[j]);
 for(int j=1;j<m-1;j++)ch_triangle(p[0],p[j],p[j+1],c);
}
static void ch_quad(ChVec a,ChVec b,ChVec c,ChVec d,U k,int shade){ChVec q[]={a,b,c,d};ch_face(q,4,k,shade);}
static void ch_ground(void){
 const U seam=chDark>.5f?0x343c52:0x9cb0af;
 int wet=(int)(cl((int)(nwWet*100.f)-11,0,80)*.75f);
 for(int y=0;y<H;y++){
  float ray=(CH_MY-y)*(1.f/CH_F),v=chSin-ray*chCos;
  if(v<=.048f){
   int k=cl((int)((y+15)*1.5f),0,235);
   U sky=nw_blend(chDark>.5f?0x11182d:0x486b84,seam,k);
   for(int j=y*W;j<y*W+W;j++){pixels[j]=sky;chZ[j]=0;}continue;
  }
  float f=ch.h*(chCos-ray*chSin)/v,depth=f*chCos+ch.h*chSin,du=depth/CH_F;
  int xx=(int)(65536*(ch.x+f*ch.co+W*.5f*du*ch.si)),yy=(int)(65536*(ch.y+f*ch.si-W*.5f*du*ch.co));
  int ux=(int)(-65536*du*ch.si),uy=(int)(65536*du*ch.co),z0=(int)(CH_ZS/depth);
  unsigned tint=(unsigned)(100*chDark),fog=(unsigned)cl((int)(depth*(.14f+.29f*chFog)),0,227);
  for(int k=y*W;k<y*W+W;k++,xx+=ux,yy+=uy){
   unsigned x=(unsigned)(xx>>16),j=(unsigned)(yy>>16);
   U c=x<ROOM&&j<ROOM?city[j*ROOM+x]:0x203540;
   if(tint)c=nw_blend(c,0x141d30,tint);
   /* Small elongated lamp reflections remain on the pavement after the
    * rain particles stop. Use ground coordinates, not screen noise. */
   if(wet&&x<BASE_ROOM&&j<BASE_ROOM){
    /* The primary anchor is at the north-east corner of each crossing,
     * and the alternate teal lens belongs to its south-west corner. */
    int gx=(int)(x>>8),gy=(int)(j>>8),u=(int)(x&255),v=(int)(j&255);
    int dx=u-31,dy=v-222,second=0;
    if(u>=213&&v>=8&&v<=68){dx=u-222;dy=v-38;second=1;gx++;}
    else gy++;
    int ax=absf(dx),ay=absf(dy);
    if(ax<9&&ay<31){
     StreetLamp lamp;
     if(street_lamp_legacy(gx,gy,second,&lamp)){
      int glint=wet*(9-ax)*(31-ay)/279;
      if((j+(x>>1))&3)glint/=2;
      c=nw_blend(c,lamp.pool,glint);
     }
    }
   }
   pixels[k]=nw_blend(c,seam,fog);chZ[k]=z0;
  }
 }
}
static void ch_facade(float x,float y,float xx,float yy,float ht,U c,int id,int side,int detail){
 ch_quad((ChVec){x,y,0},(ChVec){xx,yy,0},(ChVec){xx,yy,ht},(ChVec){x,y,ht},c,1);
 float dx=xx-x,dy=yy-y,l=sq(d2(dx,dy));
 if(!detail||l<11)return;
 int spans=cl((int)(l/19),1,8),floors=cl((int)(ht/19),1,5);
 float ox=dy/l*.8f,oy=-dx/l*.8f; /* winding has exterior on its right */
 x+=ox;y+=oy;
 for(int k=0;k<spans;k++){
  float t=((float)k+.23f)/spans,tt=((float)k+.74f)/spans;
  float sx0=x+dx*t,sy0=y+dy*t,tx=x+dx*tt,ty=y+dy*tt;
  for(int j=0;j<floors;j++){
   float lo=6.f+j*17.f,hi=lo+7.f;if(hi>=ht-2)continue;
   U win=chDark>.35f&&((id*19+side*17+k*11+j*3)&7)<3?((k+j)%3==0?0x96d1ce:0xe5bb88):0x344c59;
   ch_quad((ChVec){sx0,sy0,lo},(ChVec){tx,ty,lo},(ChVec){tx,ty,hi},(ChVec){sx0,sy0,hi},win,1);
  }
 }
 ch_quad((ChVec){x,y,ht-3},(ChVec){xx+ox,yy+oy,ht-3},(ChVec){xx+ox,yy+oy,ht},(ChVec){x,y,ht},mix(c,PAPER,2),1);
}
static void ch_buildings(void){
 for(int i=0;i<nb;i++){
  const Building*b=buildings+i;StreetBounds f=building_bounds(b);
  float l=f.x,r=f.x+f.w,t=f.y,bt=f.y+f.h,dx=(l+r)*.5f-ch.x,dy=(t+bt)*.5f-ch.y;
  float dd=d2(dx,dy);if(dd>1010.f*1010.f||dx*ch.co+dy*ch.si< -102)continue;
  float ht=22.f+1.55f*b->ht+(i%11==6?19:0);
  int detail=dd<480.f*480.f;
  if(ch.y<t)ch_facade(l,t,r,t,ht,mix(b->roof,0x202b38,4),i,0,detail);
  if(ch.x>r)ch_facade(r,t,r,bt,ht,mix(b->roof,0x35465b,3),i,1,detail);
  if(ch.y>bt)ch_facade(r,bt,l,bt,ht,mix(b->roof,0x8c92a0,2),i,2,detail);
  if(ch.x<l)ch_facade(l,bt,l,t,ht,mix(b->roof,0x31475b,3),i,3,detail);
  ch_quad((ChVec){l,t,ht},(ChVec){r,t,ht},(ChVec){r,bt,ht},(ChVec){l,bt,ht},b->roof,1);
  if(detail){
   float x=l+(r-l)*.72f,y=t+(bt-t)*.39f;
   ch_quad((ChVec){x-6,y-4,ht+2},(ChVec){x+6,y-4,ht+2},(ChVec){x+6,y+4,ht+2},(ChVec){x-6,y+4,ht+2},0x788987,1);
  }
 }
}
static ChVec ch_v(const Car*c,float u,float v,float z0){
 float co=cs(c->a),si=sn(c->a);return (ChVec){c->x+u*co-v*si,c->y+u*si+v*co,z0};
}
static void ch_strip(const Car*c,float u,float v,float w,float wi,float z0,float zz,U color,int shade){
 ch_quad(ch_v(c,u,-w,z0),ch_v(c,v,-wi,zz),ch_v(c,v,wi,zz),ch_v(c,u,w,z0),color,shade);
}
static void ch_end(const Car*c,float u,float left,float right,float lower,float upper,U color,int shade){
 ch_quad(ch_v(c,u,left,lower),ch_v(c,u,right,lower),ch_v(c,u,right,upper),ch_v(c,u,left,upper),color,shade);
}
static void ch_car(const Car*c,int hero){
 int t=cl(c->type,0,4);const NgShape*s=ngShape+t;
 float rear=s->tail,front=s->nose,w=s->width,h=s->hood+.65f;
 NgMods m=c==cars&&c->owner==1?ng_installed(t):(NgMods){0};
 U paint=hero<0?0x8bdbd0:c->cond?(m.paint?ngPaint[m.paint]:specs[t].c):0x45535b,side=mix(paint,0x131e2c,4);
 float rt=s->roofTail,rn=s->roofNose,z0=s->roofHeight+.65f,wa=w-1.65f;
 ch_strip(c,rear-2,front+2,w+2,w+2,.35f,.35f,0x161d25,1);
 for(int sgn=-1;sgn<=1;sgn+=2){
  for(int axle=0;axle<2;axle++){
   float u=axle?front-5:rear+5,v=sgn*(w-.2f);
   ch_quad(ch_v(c,u-3.0f,v,.9f),ch_v(c,u+3.0f,v,.9f),ch_v(c,u+3.0f,v,5.7f),ch_v(c,u-3.0f,v,5.7f),0x121922,1);
   if(hero)ch_quad(ch_v(c,u-1.7f,v+.6f*sgn,2),ch_v(c,u+1.7f,v+.6f*sgn,2),ch_v(c,u+1.7f,v+.6f*sgn,4.5f),ch_v(c,u-1.7f,v+.6f*sgn,4.5f),ngRims[m.rim],0);
  }
  ch_quad(ch_v(c,rear,sgn*(w-1.0f),2.3f),ch_v(c,front,sgn*(w-1.5f),2.3f),ch_v(c,front,sgn*(w-1.5f),h),ch_v(c,rear,sgn*(w-1.0f),h),side,1);
  ch_quad(ch_v(c,s->trunk,sgn*(w-1.1f),h),ch_v(c,s->wind,sgn*(w-1.1f),h),ch_v(c,rn,sgn*wa,z0),ch_v(c,rt,sgn*wa,z0),0x223d51,hero?0:1);
 }
 ch_strip(c,rear,front,w-1.f,w-1.5f,h,h,paint,1);
 ch_strip(c,s->trunk,rt,w-1.1f,wa,h,z0,0x36596b,hero?0:1);
 ch_strip(c,rn,s->wind,wa,w-1.1f,z0,h,0x587c84,hero?0:1);
 ch_strip(c,rt,rn,wa,wa,z0,z0,mix(paint,PAPER,2),1);
 if(t==3||m.paint==7)ch_strip(c,rt+.1f,rn-.1f,.55f,.55f,z0+.07f,z0+.07f,0xf7ddb6,0);
 ch_end(c,rear,w*-1.f+1,w-1.f,2.35f,h,side,1);
 ch_end(c,front,w*-1.f+1.5f,w-1.5f,2.35f,h,mix(paint,0x182c39,4),1);
 ch_end(c,rear-.15f,-w+1.1f,w-1.1f,1.8f,2.9f,0x263541,1);
 ch_end(c,rear-.23f,-1.65f,1.65f,2.7f,3.8f,ngTags[m.tag],0);
 if(m.kit)ch_strip(c,rear-.7f,rear+1,w+1,w+1,h+(m.kit==2?2:1),h+(m.kit==2?2:1),side,1);
 int bulbs=road_lamps(c);
 if(c->cond)for(int i=-1;i<=1;i+=2){
  ch_end(c,front+.15f,i*(w-1.6f),i*(w-4.0f),h-1.0f,h-.1f,0xffe4a2,0);
  ch_end(c,rear-.21f,i*(w-1.0f),i*(w-3.8f),h-1.0f,h-.1f,bulbs&1?0xff5148:0xc84b44,0);
  if(bulbs&2)ch_end(c,rear-.23f,i*(w-4.3f),i*(w-5.1f),h-1.0f,h-.1f,0xe2faff,0);
 }
}
static void ch_upright(float x,float y,float w,float bottom,float top,U ink){
 float u=ch.si*w,v=-ch.co*w;
 ch_quad((ChVec){x-u,y-v,bottom},(ChVec){x+u,y+v,bottom},(ChVec){x+u,y+v,top},(ChVec){x-u,y-v,top},ink,1);
}
static void ch_person(float x,float y,int kind,int i){
 U jacket=kind==1?0x718ffc:kind==2?0xe08587:(i&1)?0x9ebdb7:0xc28b79;
 ch_upright(x,y,2.2f,.2f,5.3f,0x292e38);ch_upright(x,y,3.6f,5,11,jacket);ch_upright(x,y,2.1f,11,15,0xbe987f);
}
static void ch_lamp(const StreetLamp*s){
 float x=s->x,y=s->y;
 if(d2(x-ch.x,y-ch.y)>765.f*765.f)return;
 /* Translate the old rectangular fixture rather than inventing a taller
  * street pole; both lenses and their palettes match the arcade view. */
 float foot=s->poleH+.25f,cap=s->lensH;
 ch_upright(x,y,s->poleW*.5f,0,foot,s->pole);
 ch_upright(x,y,s->lensW*.5f,foot,foot+cap,s->lens);
}
static void ch_props(void){
 for(int i=0;i<LEN(peds);i++){
  Ped*q=peds+i;if(q->hp<=0||d2(ch.x-q->x,ch.y-q->y)>670.f*670.f)continue;
  ch_person(q->x,q->y,q->role,i);
 }
 if((job.kind==JOB_TAXI&&taxi.phase!=TAXI_RIDING)||(taxi.phase==TAXI_WALKING&&taxi.linger>0))
  ch_person(taxi.x,taxi.y,0,taxi.rider);
 for(int i=0;i<LEN(scenery);i++){
  const BlockScenery*b=scenery+i;for(int j=b->treesFrom;j<b->count;j++){
   const StreetBounds*t=b->walls+j;
   float x=t->x+t->w*.5f,y=t->y+t->h*.5f;
   if(d2(x-ch.x,y-ch.y)>710.f*710.f)continue;
   ch_upright(x,y,1.7f,0,18,0x54514b);
   ch_upright(x,y,7,12,20,0x294540);
   ch_upright(x,y,9,19,26,0x34544b);
   ch_upright(x,y,6,25,32,0x446057);
  }
 }
 StreetLamp fixture;
 for(int j=1;j<BASE_ROOM/256;j++)for(int i=1;i<BASE_ROOM/256;i++)
  for(int k=0;k<2;k++)if(street_lamp_legacy(i,j,k,&fixture))ch_lamp(&fixture);
 for(int i=0;i<LEN(outerNodes);i+=2)if(street_lamp_outer(i,&fixture))ch_lamp(&fixture);
 for(int i=0;i<LEN(fx);i++)if(fx[i].t>0&&d2(fx[i].x-ch.x,fx[i].y-ch.y)<580.f*580.f)
  ch_upright(fx[i].x,fx[i].y,1.2f,3.f,5.5f,fx[i].c);
}
static void ch_road_glyph(float x,float y,U color){
 float w=12,h=30;
 ch_quad((ChVec){x-w,y-2,.25f},(ChVec){x+w,y-2,.25f},(ChVec){x+w,y+2,.25f},(ChVec){x-w,y+2,.25f},color,0);
 ch_quad((ChVec){x-1.25f,y,0},(ChVec){x+1.25f,y,0},(ChVec){x+1.25f,y,h},(ChVec){x-1.25f,y,h},color,0);
}
static void ch_outer(void){
#ifdef BASE_ROOM
 /* The same sampled ribbons place actual rails and reflector posts in the
  * new country and shore roads as in the expanded collision/atlas graph. */
 for(int i=0;i<LEN(outerRoads);i++){
  const OuterRoad*r=outerRoads+i;
  if(r->kind!=OUT_HILL&&r->kind!=OUT_DOCK&&r->kind!=OUT_COUNTRY)continue;
  const OuterPoint *a=outerNodes+r->a,*b=outerNodes+r->b;
  float x=a->x,y=a->y,xx=b->x,yy=b->y;
  if(d2((x+xx)*.5f-ch.x,(y+yy)*.5f-ch.y)>830.f*830.f)continue;
  float dx=xx-x,dy=yy-y,len=sq(d2(dx,dy));if(len<1)continue;
  float u=dy*(r->half-3.f)/len,v=-dx*(r->half-3.f)/len;
  for(int j=-1;j<=1;j+=2){
   float x0=x+j*u,x1=xx+j*u,y0=y+j*v,y1=yy+j*v;
   U rail=r->kind==OUT_HILL?0x84998f:r->kind==OUT_DOCK?0x9aa49b:0x708578;
   ch_quad((ChVec){x0,y0,4.3f},(ChVec){x1,y1,4.3f},(ChVec){x1,y1,6.3f},(ChVec){x0,y0,6.3f},rail,1);
   for(float d=0;d<len;d+=28.f){
    float t=d/len;ch_upright(x0+dx*t,y0+dy*t,.75f,.2f,6.8f,rail);
   }
  }
 }
 for(int i=0;i<LEN(outerStops);i++){
  const OuterPoint*q=outerNodes+outerStops[i].node;
  if(d2(q->x-ch.x,q->y-ch.y)>500.f*500.f)continue;
  float x=q->x+17,y=q->y+12;
  ch_upright(x,y,.85f,0,19.6f,0x80998e);
  ch_upright(x,y,7.5f,13.5f,20.5f,outerStops[i].tint);
  ch_upright(x,y,4.7f,16.9f,17.8f,0x204453);
 }
#endif
}
static int ch_draw(void){
 chProjected=0;if(!ch_enabled())return 0;
 const Car*c=cars+p.car;
 float a=ngCamRide==p.car?ngCamAngle:c->a;
 ch.co=cs(a);ch.si=sn(a);
 /* Wall sweep trims the boom as the rear swings toward masonry. If there
  * is no space, pivot close and rise; the target never disappears into a
  * wall even after full steering or reversing near a roof edge. */
 float dist=ch.ride==p.car&&ch.range>1?ch.range:79,high=ch.ride==p.car&&ch.height>1?ch.height:38;
 float fit=dist;
 for(float q=19;q<=dist;q+=5.f){
  float xx=c->x-q*ch.co,yy=c->y-q*ch.si;
  if(solid(xx,yy,4)){fit=q-5.f;break;}
 }
 /* Keep steering visible as the boom shortens. Pitch down just enough to
  * retain the car at its normal screen anchor, instead of clipping it at
  * the bottom when backing into the wall next to a driveway. */
 if(fit<34.f)high+=min(24,(int)(34.f-fit))*1.10f;
 chFit=fit;
 float hAim=high-4,diag=sq(fit*fit+hAim*hAim),target=.303f;
 chSin=(hAim*.953f-fit*target)/diag;
 chCos=(fit*.953f+hAim*target)/diag;
 ch.x=c->x-fit*ch.co;ch.y=c->y-fit*ch.si;ch.h=high;
 chDark=nw_dark();chFog=nwFog;chProjected=1;
 ch_ground();ch_buildings();ch_outer();ch_props();
 for(int i=0;i<nc;i++)if((i||sv.owned)&&cars[i].x>=0&&d2(cars[i].x-ch.x,cars[i].y-ch.y)<755.f*755.f)
  ch_car(cars+i,i==p.car);
 for(int i=0;i<outCarCount;i++)if(outCars[i].cond>0&&d2(outCars[i].x-ch.x,outCars[i].y-ch.y)<710.f*710.f)
  ch_car(outCars+i,0);
 Car ghost;if(ng_ghost_car(&ghost)&&d2(ghost.x-ch.x,ghost.y-ch.y)<730.f*730.f)ch_car(&ghost,-1);
 int x,y,meet;
 if(ng_route_goal(&x,&y)){if(d2(x-ch.x,y-ch.y)<645.f*645.f)ch_road_glyph(x,y,GOLD);}
 else if((meet=night_garage_meet_site(&x,&y))>=0&&job.kind<0&&vdActive<0&&d2(x-ch.x,y-ch.y)<350.f*350.f)ch_road_glyph(x,y,TEAL);
 if(night_phone_lane_stop(2410,1024,2500,1024,4)&&d2(ch.x-2464,ch.y-1024)<685.f*685.f){
  for(int y=974;y<1080;y+=17){
   ch_upright(2464,y,1.7f,0,8,0x77504a);
   ch_upright(2464,y,2.3f,7,9,0xf4bd66);
   ch_quad((ChVec){2464,y,3},(ChVec){2464,y+13,3},(ChVec){2464,y+13,6},(ChVec){2464,y,6},0xb65f59,0);
  }
 }
 if(nwStorm>.10f){
  int drops=(int)(nwStorm*132),u=(int)(anim*263)%W,v=(int)(anim*296)%H;
  for(int i=0;i<drops;i++){
   int x=(i*197+u)%W,y=(i*233+v)%H;
   line(x,y,x-2,y+6+i%3,mix(pixels[y*W+x],0x9db4ca,2));
  }
 }
 return 1;
}
static void ch_badge(void){
 if(!chProjected||scene!=1||menu>=0)return;
 box(467,16,164,23,0x101d25,0x425864);center(549,17,"CHASE VIEW / C",0xa6c5c4,0);
 int x,y;if(ng.event==NG_IDLE&&job.kind<0&&vdActive<0&&night_garage_meet_site(&x,&y)>=0&&d2(p.x-x,p.y-y)<58*58){
  box(352,448,256,26,0x132931,TEAL);center(W/2,451,"NIGHT MEET / STOP + ENTER",TEAL,0);
 }
}
