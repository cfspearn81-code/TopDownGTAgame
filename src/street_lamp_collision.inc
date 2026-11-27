/* Small ground footprints for the exact fixtures in the world catalogue.
 * Renderer-only glass, light pools, and parallax mounts do not participate.
 * Kept out of BlockScenery: no fake roofs or tall camera blockers, and the
 * tagged curb lights at a tile corner still query correctly across tiles. */
enum { SL_GRID=64,SL_SIDE=ROOM/SL_GRID,SL_COUNT=2*7*7+LEN(outerNodes)+HARBOR_LAMPS };
_Static_assert(SL_COUNT<255&&ROOM%SL_GRID==0,"lamp footprint index is one byte");
typedef struct{short x,y;unsigned char next;} StreetLampFoot;
static StreetLampFoot streetLampFoot[SL_COUNT+1];
static unsigned char streetLampGrid[SL_SIDE*SL_SIDE];
static int streetLampFootCount;
static const float streetLampBase=1.65f;
static void street_lamp_insert(const StreetLamp*s){
 int at=(s->y/SL_GRID)*SL_SIDE+s->x/SL_GRID,n=++streetLampFootCount;
 streetLampFoot[n]=(StreetLampFoot){s->x,s->y,streetLampGrid[at]};
 streetLampGrid[at]=(unsigned char)n;
}
static void street_lamp_reset(void){
 z(streetLampGrid,sizeof streetLampGrid);streetLampFootCount=0;
}
static void street_lamp_index(void){
 street_lamp_reset();
 StreetLamp s;
 for(int y=1;y<BASE_ROOM/256;y++)for(int x=1;x<BASE_ROOM/256;x++)
  for(int side=0;side<2;side++)if(street_lamp_legacy(x,y,side,&s))street_lamp_insert(&s);
 for(int i=0;i<LEN(outerNodes);i+=2)if(street_lamp_outer(i,&s))street_lamp_insert(&s);
 for(int i=0;i<HARBOR_LAMPS;i++)if(street_lamp_harbor(i,&s))street_lamp_insert(&s);
}
static int street_lamp_bucket(float r){return cl((int)(r*(1.f/SL_GRID)),0,SL_SIDE-1);}
/* A person's full wall/body spacing is intentionally not the width of
 * their shoes. Keep the contact margin ground-sized against a small base;
 * in particular the real foot can still clear the narrow old crossings. */
static int street_lamp_foot(float x,float y,float r){
 const float reach=streetLampBase+(r>2.20f?2.20f:r);
 int left=street_lamp_bucket(x-reach),right=street_lamp_bucket(x+reach);
 int top=street_lamp_bucket(y-reach),bottom=street_lamp_bucket(y+reach);
 for(int j=top;j<=bottom;j++)for(int i=left;i<=right;i++)
  for(int k=streetLampGrid[j*SL_SIDE+i];k;k=streetLampFoot[k].next)
   if(d2(x-streetLampFoot[k].x,y-streetLampFoot[k].y)<reach*reach)return 1;
 return 0;
}
static float street_lamp_car(const Car*c,float co,float si,float rx,float ry){
 int l=street_lamp_bucket(c->x-rx-streetLampBase),r=street_lamp_bucket(c->x+rx+streetLampBase);
 int t=street_lamp_bucket(c->y-ry-streetLampBase),b=street_lamp_bucket(c->y+ry+streetLampBase);
 float depth=0.f;
 for(int j=t;j<=b;j++)for(int i=l;i<=r;i++)
  for(int k=streetLampGrid[j*SL_SIDE+i];k;k=streetLampFoot[k].next){
   const StreetLampFoot*s=&streetLampFoot[k];float dx=s->x-c->x,dy=s->y-c->y;
   if(absf(dx)>=rx+streetLampBase||absf(dy)>=ry+streetLampBase)continue;
   float u=absf(dx*co+dy*si)-STREET_HALF_L,v=absf(dy*co-dx*si)-STREET_HALF_W;
   if(u>=streetLampBase||v>=streetLampBase)continue;
   float d;
   if(u<=0.f&&v<=0.f)d=streetLampBase-(u>v?u:v);
   else d=streetLampBase-sq(d2(u>0?u:0,v>0?v:0));
   if(d>depth)depth=d;
  }
 return depth;
}
