/* Twin low beams in world space. One sparse 16x16 tile field serves both
 * cameras: paint only touched overhead tiles, and sample the same light
 * energy at each perspective ground intersection before drawing scenery.
 * No full-map light-buffer clear, extra animation state or save fields. */
enum{HL_EDGE=16,HL_SIDE=(ROOM+HL_EDGE-1)/HL_EDGE,HL_MAX_TILES=1536,HL_RAYS=12};
typedef struct{unsigned short x,y;unsigned char glow[HL_EDGE*HL_EDGE];} HlTile;
static HlTile hlTiles[HL_MAX_TILES];
static unsigned short hlIndex[HL_SIDE*HL_SIDE];
static int hlCount,hlGain,hlWet;
static int beam_block(int x,int y){
 return (U)x>=ROOM||(U)y>=ROOM||roofMask[y*ROOM+x]||(outer_is_new(x,y)&&!outer_walkable(x,y,0));
}
static U headlamps_shade(U c,int glow){
 int e=(glow*hlGain+16)>>5;
 if(!e)return c;
 int r=min(255,((c>>16)&255)+e*3);
 int g=min(255,((c>>8)&255)+(e*11>>2));
 int b=min(255,(c&255)+(e*(17+hlWet)>>4));
 return RGB(r,g,b);
}
static unsigned headlamps_energy(int x,int y){
 if(!hlCount||(U)x>=ROOM||(U)y>=ROOM)return 0;
 unsigned tile=hlIndex[(y/HL_EDGE)*HL_SIDE+x/HL_EDGE];
 return tile?hlTiles[tile-1].glow[(y&(HL_EDGE-1))*HL_EDGE+(x&(HL_EDGE-1))]:0;
}
static U headlamps_ground(U c,int x,int y){
 unsigned e=headlamps_energy(x,y);
 return e?headlamps_shade(c,e):c;
}
static void headlamps_add(int x,int y,int e){
 int tx=x/HL_EDGE,ty=y/HL_EDGE;
 unsigned short *dst=hlIndex+ty*HL_SIDE+tx;
 HlTile*t;
 if(!*dst){
  if(hlCount==HL_MAX_TILES)return;
  t=hlTiles+hlCount;*dst=++hlCount;t->x=tx*HL_EDGE;t->y=ty*HL_EDGE;
  z(t->glow,sizeof t->glow);
 }else t=hlTiles+(*dst-1);
 unsigned char*p=t->glow+(y&(HL_EDGE-1))*HL_EDGE+(x&(HL_EDGE-1));
 *p=min(93,*p+e);
}
static float headlamps_clamp_range(const Car*c){return 60.f+.23f*c->cond;}
/* Bounds and source describe only the fixture as seen by each renderer.
 * Direction always uses vehicle heading, never the velocity or camera. */
static void headlamps_pair(const Car*c,int style,int left,int top,int right,int bottom){
 if(c->cond<=0)return;
 float co=cs(c->a),si=sn(c->a),range=headlamps_clamp_range(c);
 const NgShape*s=ngShape+cl(c->type,0,LEN(ngShape)-1);
 float snout=style==2?s->nose+.15f:style==1?s->nose-1.f:13.f;
 float sep=style==2?s->width-2.8f:style==1?s->width-2.3f:5.f;
 float forward=c->x+snout*co,onward=c->y+snout*si;
 static const float flare=.087f,root=1.6f;
 for(int side=-1;side<=1;side+=2){
  float x=forward-side*sep*si,y=onward+side*sep*co;
  float yaw=side*.138f,end=root+range*flare;
  /* Narrow independent cones splay gently, with a dark center between
   * them from the nose to the limit: width never grows as fast as yaw. */
  float fx=x+range*(co-yaw*si),fy=y+range*(si+yaw*co);
  int lo=max(left,(int)(fx-end*absf(si))),hi=min(right,(int)(fx+end*absf(si))+2);
  int y0=max(top,(int)(fy-end*absf(co))),y1=min(bottom,(int)(fy+end*absf(co))+2);
  lo=max(left,min(lo,(int)(x-root*absf(si))));hi=min(right,max(hi,(int)(x+root*absf(si))+2));
  y0=max(top,min(y0,(int)(y-root*absf(co))));y1=min(bottom,max(y1,(int)(y+root*absf(co))+2));
  if(lo>=hi||y0>=y1)continue;
  float rays[HL_RAYS+1];
  for(int k=0;k<=HL_RAYS;k++){
   float q=k*(2.f/HL_RAYS)-1.f,d;
   for(d=0.f;d<range;d+=1.5f){
    float cross=yaw*d+q*(root+d*flare);
    if(beam_block((int)(x+d*co-cross*si),(int)(y+d*si+cross*co)))break;
   }
   rays[k]=d;
  }
  for(int py=y0;py<y1;py++){
   float dx=lo+.5f-x,dy=py+.5f-y,f=dx*co+dy*si,v=dy*co-dx*si;
   for(int px=lo;px<hi;px++,f+=co,v-=si){
    if(f<0.f||f>=range)continue;
    float width=root+f*flare,l=v-yaw*f,uv=absf(l);
    if(uv>=width)continue;
    float u=l/width,spot=(u+1.f)*(HL_RAYS*.5f);int ray=cl((int)spot,0,HL_RAYS-1);
    float cutoff=rays[ray]+(rays[ray+1]-rays[ray])*(spot-ray);
    if(f+1.f>=cutoff||beam_block(px,py))continue;
    float edge=1.f-absf(u),dist=1.f-f/range;
    int glow=(int)(74.f*dist*(.45f+.55f*dist)*edge*(.75f+.25f*edge));
    if(glow)headlamps_add(px,py,glow);
   }
  }
 }
}
static void headlamps_build(int chase,int left,int top,int right,int bottom){
 hlCount=0;if(fastGfx)return;
 z(hlIndex,sizeof hlIndex);
 hlGain=17+(int)(15*nw_dark());hlWet=cl((int)(nwWet*12),0,12);
 left=cl(left,0,ROOM);top=cl(top,0,ROOM);right=cl(right,0,ROOM);bottom=cl(bottom,0,ROOM);
 if(street_car_exists(p.car))
  headlamps_pair(cars+p.car,chase?2:p.car==0&&cars[0].owner==1&&sv.owned?1:0,left,top,right,bottom);
 /* Regional traffic is part of the canonical chassis array. A live
  * traffic driver keeps headlights at a stop; an unattended parked car
  * does not emit. The same index is used by collision, boarding and chase. */
 for(int i=street_next_car(-1);i<LEN(cars);i=street_next_car(i)){
  if(i==p.car||cars[i].cond<=0)continue;
  int active=i<STREET_CITY_CAR_LIMIT?streetDrivers[i].live:!outDriver[i-STREET_CITY_CAR_LIMIT].driven;
  if(active||absf(cars[i].s)>9.f)
   headlamps_pair(cars+i,chase?2:!i&&cars[0].owner==1?1:0,left,top,right,bottom);
 }
}
static void headlamps_overhead(void){
 headlamps_build(0,cx,cy,cx+W,cy+H);
 for(int i=0;i<hlCount;i++){
  const HlTile*t=hlTiles+i;int l=max(cx,t->x),r=min(cx+W,t->x+HL_EDGE);
  int u=max(cy,t->y),b=min(cy+H,t->y+HL_EDGE);
  if(l>=r||u>=b)continue;
  for(int y=u;y<b;y++){
   U *p=pixels+(y-cy)*W+l-cx;
   const unsigned char *g=t->glow+(y-t->y)*HL_EDGE+l-t->x;
   for(int x=l;x<r;x++,p++,g++)if(*g)*p=headlamps_shade(*p,*g);
  }
 }
}
