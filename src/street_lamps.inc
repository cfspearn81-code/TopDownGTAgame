/* Career Files (bd5d4e61) lamp palette, rectangular bodies and offsets.
 * The seven-by-seven original crossings share this catalogue with the
 * time/weather compositor and optional perspective camera. */
typedef struct {
 int x,y,radius,weight;
 int poleX,poleY,poleW,poleH,lensX,lensY,lensW,lensH;
 U pool,pole,lens;
} StreetLamp;
static StreetLamp street_lamp_style(int x,int y,U c,int secondary){
 if(secondary)return (StreetLamp){x,y,27,3,0,0,2,10,-2,-4,6,5,0x549e92,0x4b6364,TEAL};
 return (StreetLamp){x,y,46,4,-1,-1,3,13,-4,-4,8,4,c,0x293842,mix(c,PAPER,5)};
}
static int street_lamp_legacy(int gx,int gy,int secondary,StreetLamp *s){
 if(gx<1||gy<1||gx>=BASE_ROOM/256||gy>=BASE_ROOM/256)return 0;
 if(secondary&&(gx+gy)%2)return 0;
 if(secondary)*s=street_lamp_style(gx*256-34,gy*256+38,0,1);
 else{
  int x=gx*256,y=gy*256;
  *s=street_lamp_style(x+31,y-34,y<600?0x845987:x>1300?0x388b8c:0x8d7352,0);
 }
 return 1;
}
enum { HARBOR_LAMPS=3 };
static StreetLamp outerLamp[LEN(outerNodes)],harborLamp[HARBOR_LAMPS];
static unsigned char outerLampPlaced[LEN(outerNodes)],harborLampPlaced[HARBOR_LAMPS];
static int street_lamp_outer(int i,StreetLamp*s){
 if((unsigned)i>=LEN(outerLampPlaced)||!outerLampPlaced[i])return 0;
 *s=outerLamp[i];return 1;
}
static int street_lamp_harbor(int id,StreetLamp*s){
 if((unsigned)id>=HARBOR_LAMPS||!harborLampPlaced[id])return 0;
 *s=harborLamp[id];return 1;
}
/* Search only at scenery bake time, after the entire road union and all
 * solid terrain/structures exist. Midpoint offsets from a single edge
 * are unsafe at merges, bends and working harbor access lanes. */
static int street_lamp_surface(int x,int y){
 if(x<35||y<35||x>ROOM-36||y>ROOM-36||!outer_is_new(x,y))return 0;
 int j=y*ROOM+x,t=outerLand[j];
 return !outerLane[j]&&!roofMask[j]&&!(t&TILE_BLOCK)&&t!=TILE_TIDE&&t!=TILE_TARN;
}
static int street_lamp_rect(int x,int y,int w,int h){
 for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++)
  if(!street_lamp_surface(xx,yy))return 0;
 return 1;
}
static int street_lamp_clear(const StreetLamp*s){
 int x=s->x,y=s->y;
 if(!street_lamp_rect(x-3,y-3,7,7))return 0;
 /* The outer address pass reserves walk-up space before fixtures choose
  * an anchor. A lamp must not become the body in front of an open door. */
 for(int i=0;i<nb;i++)if(ordinaryDoorY[i]&&outer_is_new(ordinaryDoorX[i],ordinaryDoorY[i])&&
   d2(x-ordinaryDoorX[i],y-ordinaryDoorY[i])<20.f*20.f)return 0;
 /* Independently match the continuous road geometry with a safety
  * buffer, not just the road bitmap at this one integer point. */
 for(int i=0;i<LEN(outerRoads);i++){
  const OuterRoad*r=&outerRoads[i];const OuterPoint a=outerNodes[r->a],b=outerNodes[r->b];
  int w=r->half+8;
  if(x<min(a.x,b.x)-w||x>max(a.x,b.x)+w||y<min(a.y,b.y)-w||y>max(a.y,b.y)+w)continue;
  float t=out_proj(a,b,x,y);
  if(d2(x-a.x-(b.x-a.x)*t,y-a.y-(b.y-a.y)*t)<w*w)return 0;
 }
 /* The preserved overhead graphics are several pixels wider/longer
  * than the base, and can jut into a lane even with a clear center. */
 return street_lamp_rect(x+s->poleX,y+s->poleY,s->poleW,s->poleH)
     && street_lamp_rect(x+s->lensX,y+s->lensY,s->lensW,s->lensH);
}
static int street_lamp_separated(int x,int y,int upto){
 for(int i=0;i<upto;i++)if(outerLampPlaced[i]&&d2(x-outerLamp[i].x,y-outerLamp[i].y)<47*47)return 0;
 for(int i=0;i<HARBOR_LAMPS;i++)if(harborLampPlaced[i]&&d2(x-harborLamp[i].x,y-harborLamp[i].y)<30*30)return 0;
 return 1;
}
static void street_lamp_place_outside(void){
 z(outerLampPlaced,sizeof outerLampPlaced);z(harborLampPlaced,sizeof harborLampPlaced);
 for(int k=0;k<HARBOR_LAMPS;k++){
  const OuterPoint q=harborDockPoints[k*2];
  int x=q.x+(k==0?16:13),y=q.y-12,best=25*25;
  StreetLamp s=street_lamp_style(x,y,k==1?0xa38a5d:0x488f87,0);
  s.radius=27;s.weight=3;
  /* Original pier positions are preferred. If refitting ever changes
   * their dry space, keep the pole on the same dock or hide it. */
  for(int dy=-18;dy<=18;dy+=2)for(int dx=-18;dx<=18;dx+=2){
   int cost=dx*dx+dy*dy;if(cost>=best)continue;
   s.x=x+dx;s.y=y+dy;
   int type=harbor_paint_class(s.x,s.y);
   if(type<0||type==HB_ROCK||!street_lamp_clear(&s)||!street_lamp_separated(s.x,s.y,0))continue;
   best=cost;harborLamp[k]=s;harborLampPlaced[k]=1;
  }
 }
 for(int k=0;k<LEN(outerNodes);k+=2){
  const OuterPoint q=outerNodes[k];
  if(!outer_is_new(q.x,q.y))continue;
  int x=q.x+23,y=q.y-17;float best=100.f*100.f;
  StreetLamp s=street_lamp_style(x,y,q.y<810?0x845987:q.y<1520?0x8d7352:0x388b8c,0);
  for(int i=0;i<LEN(outerRoads);i++){
   const OuterRoad*r=&outerRoads[i];int next=r->a==k?r->b:r->b==k?r->a:-1;
   if(next<0)continue;
   float dx=outerNodes[next].x-q.x,dy=outerNodes[next].y-q.y,n=sq(d2(dx,dy));
   if(n<1.f)continue;
   float ux=dx/n,uy=dy/n;
   for(int run=0;run<=60&&run<n*.68f;run+=12)for(int shoulder=11;shoulder<=25;shoulder+=7){
    for(int sign=-1;sign<=1;sign+=2){
     float offset=sign*(r->half+shoulder);
     s.x=(int)(q.x+ux*run-uy*offset+.5f);
     s.y=(int)(q.y+uy*run+ux*offset+.5f);
     float cost=d2(x-s.x,y-s.y)+(shoulder-11)*38;
     if(cost>=best||!street_lamp_clear(&s)||!street_lamp_separated(s.x,s.y,k))continue;
     best=cost;outerLamp[k]=s;outerLampPlaced[k]=1;
    }
   }
  }
 }
}
static void street_lamp_bake(const StreetLamp*s){
 light(s->x,s->y,s->radius,s->pool,s->weight);
 if(bakeFloorOnly)return;
 rect(s->x+s->poleX,s->y+s->poleY,s->poleW,s->poleH,s->pole);
 rect(s->x+s->lensX,s->y+s->lensY,s->lensW,s->lensH,s->lens);
}
