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
#include "chase_rig.inc"
static void ch_motion(float speed,float err){
 float unit=speed/205.f;if(unit>1)unit=1;
 float bend=absf(err);if(bend>.8f)bend=.8f;
 float wanted=79.f+18*unit+30*ngCamBurst-20*bend;
 float high=38.f+5*unit+11*bend;
 if(ch.ride!=p.car||ngCamRide!=p.car||ch.range<1){ch.range=79;ch.height=38;}
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
#include "chase_asphalt.inc"
#include "chase_sea_edge.inc"
/* Solve ray=(b*sin(pitch)-height*cos(pitch))/(b*cos(pitch)+height*sin(pitch)).
 * Use the same orthonormal camera frame as the 3D models; otherwise the
 * road slides visibly under tires and ground-anchored contacts at steep pitch. */
static inline float ch_floor_run(float ray,float denom){return ch.h*(chCos+ray*chSin)/denom;}
static int chDecals;static inline U ch_decal_pixel(U,unsigned,unsigned);
static void ch_ground(void){
 cf_road_prepare();
 const U seam=chDark>.5f?0x343c52:0x9cb0af;
 int wet=(int)(cl((int)(nwWet*100.f)-11,0,80)*.75f);
 for(int y=0;y<H;y++){
  float ray=(CH_MY-y)*(1.f/CH_F),v=chSin-ray*chCos;
  if(v<=.048f){
   int k=cl((int)((y+15)*1.5f),0,235);
   U sky=nw_blend(chDark>.5f?0x11182d:0x486b84,seam,k);
   for(int j=y*W;j<y*W+W;j++){pixels[j]=sky;chZ[j]=0;}continue;
  }
  float f=ch_floor_run(ray,v),depth=f*chCos+ch.h*chSin,du=depth/CH_F;
  int xx=(int)(65536*(ch.x+f*ch.co+W*.5f*du*ch.si)),yy=(int)(65536*(ch.y+f*ch.si-W*.5f*du*ch.co));
  int ux=(int)(-65536*du*ch.si),uy=(int)(65536*du*ch.co),z0=(int)(CH_ZS/depth);
  unsigned tint=(unsigned)(100*chDark),fog=(unsigned)cl((int)(depth*(.14f+.29f*chFog)),0,227);
  for(int k=y*W;k<y*W+W;k++,xx+=ux,yy+=uy){
   unsigned x=(unsigned)(xx>>16),j=(unsigned)(yy>>16);
   U c=x<ROOM&&j<ROOM?city[j*ROOM+x]:0x203540;
   if(x>=ROOM-72||j>=ROOM-72)c=wl_ocean_edge(c,x,j);
   if(x>=35&&x<BASE_ROOM-26&&j>=35&&j<BASE_ROOM-26)c=cf_road_sample(c,x,j);
   if(chDecals)c=ch_decal_pixel(c,x,j);
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
   pixels[k]=headlamps_ground(nw_blend(c,seam,fog),(int)x,(int)j);chZ[k]=z0;
  }
 }
}
#include "chase_city.inc"
#include "chase_coachwork.inc"
static void ch_upright(float x,float y,float w,float bottom,float top,U ink){
 float u=ch.si*w,v=-ch.co*w;
 ch_quad((ChVec){x-u,y-v,bottom},(ChVec){x+u,y+v,bottom},(ChVec){x+u,y+v,top},(ChVec){x-u,y-v,top},ink,1);
}
#include "chase_people.inc"
static void ch_lamp(const StreetLamp*s){
 float x=s->x,y=s->y;
 if(d2(x-ch.x,y-ch.y)>765.f*765.f)return;
 /* Translate the old rectangular fixture rather than inventing a taller
  * street pole; both lenses and their palettes match the arcade view. */
 float foot=s->poleH+.25f,cap=s->lensH;
 ch_upright(x,y,s->poleW*.5f,0,foot,s->pole);
 ch_upright(x,y,s->lensW*.5f,foot,foot+cap,s->lens);
}
#include "chase_landscape.inc"
#include "chase_trail.inc"
static void ch_props(void){
 StreetPerson actor;
 for(int i=0;i<LEN(peds);i++)if(street_ped_pose(i,&actor))ch_character(&actor);
 if(street_rider_pose(&actor))ch_character(&actor);
 for(int i=0;i<min(SP_OUT_MAX,outer_walk_count());i++)if(street_outer_pose(i,&actor))ch_character(&actor);
 int marker;
 if(street_night_rider_pose(&actor,&marker)){
  ch_character(&actor);
  if(marker&&d2(ch.x-actor.x,ch.y-actor.y)<480.f*480.f){
   float x=actor.x,y=actor.y,u=3.f*ch.si,v=-3.f*ch.co;
   ch_quad((ChVec){x,y,18.8f},(ChVec){x-u,y-v,16.5f},(ChVec){x,y,14.2f},
      (ChVec){x+u,y+v,16.5f},0xf18acb,0);
  }
 }
 wl_city_trees();
 StreetLamp fixture;
 for(int j=1;j<BASE_ROOM/256;j++)for(int i=1;i<BASE_ROOM/256;i++)
  for(int k=0;k<2;k++)if(street_lamp_legacy(i,j,k,&fixture))ch_lamp(&fixture);
 for(int i=0;i<LEN(outerNodes);i+=2)if(street_lamp_outer(i,&fixture))ch_lamp(&fixture);
 for(int i=0;i<HARBOR_LAMPS;i++)if(street_lamp_harbor(i,&fixture))ch_lamp(&fixture);
 ch_field_sparks();
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
   float tmX=(x+xx)*.5f,tmY=(y+yy)*.5f;
   /* Land-side sidewalks are now open. Keep guard rails on actual banks
    * and drop-offs only; no phantom enclosure along walkable meadows. */
   if(outer_walkable(tmX+j*u*1.8f,tmY+j*v*1.8f,3))continue;
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
 wl_wood();wl_shore_grass();wl_field();wl_life();
#endif
}
#include "harbor_chase.inc"
#include "chase_jobs.inc"
static int ch_draw(void){
 chProjected=0;if(!ch_enabled())return 0;
 ch_frame_rig();
 chDark=nw_dark();chFog=nwFog;chProjected=1;
 headlamps_build(1,(int)ch.x-620,(int)ch.y-620,(int)ch.x+621,(int)ch.y+621);
 ch_collect_jobs();ch_ground();ch_buildings();ch_outer();ch_harbor_props();ch_props();
 for(int i=street_next_car(-1);i<LEN(cars);i=street_next_car(i))if(cars[i].x>=0&&d2(cars[i].x-ch.x,cars[i].y-ch.y)<755.f*755.f)
  ch_car(cars+i,i==p.car);
 ch_chalk_pixel_actor();ch_street_encounter();if(ch_subject()==CH_FOOT)ch_player_character();
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
 ch_job_labels();
 return 1;
}
static void ch_badge(void){
 if(!chProjected||scene!=1||menu>=0)return;
 box(467,16,164,23,0x101d25,0x425864);center(549,17,ch_subject()==CH_FOOT?"STREET 3D / C":ch_subject()==CH_HELM?"HELM 3D / C":"CHASE VIEW / C",0xa6c5c4,0);
 int x,y;if(ng.event==NG_IDLE&&job.kind<0&&vdActive<0&&night_garage_meet_site(&x,&y)>=0&&d2(p.x-x,p.y-y)<58*58){
  box(352,448,256,26,0x132931,TEAL);center(W/2,451,"NIGHT MEET / STOP + ENTER",TEAL,0);
 }
}
