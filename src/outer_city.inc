/* Port Vanta Outer Loop. The 2048x2048 legacy city is never regenerated
 * differently. These surveyed roads are real extensions of its grid. */
typedef struct {int x,y;} OuterPoint;
typedef struct {unsigned char a,b,half,kind;} OuterRoad;
enum{OUT_HILL,OUT_HWY,OUT_DOCK,OUT_WORKS,OUT_COUNTRY};
enum{OUT_BL_BEGIN=0,OUT_HW_BEGIN=14,OUT_SH_BEGIN=29,OUT_S_BEGIN=49,OUT_W_BEGIN=52};
static const OuterPoint outerNodes[]={
 {1792,512},{2048,512},{2144,482},{2197,411},{2263,382},{2332,421},{2388,471},
 {2460,481},{2530,437},{2598,380},{2690,384},{2770,452},{2800,568},{2752,707},
 {1792,1024},{2060,1024},{2192,1024},{2464,1024},{2688,1024},{2824,1024},{2924,1024},
 {2688,816},{2688,1280},{2192,1280},{2368,1280},{2368,816},{2192,816},{2368,1460},{2696,1480},
 {1792,1792},{2048,1792},{2152,1766},{2272,1680},{2416,1663},{2512,1696},{2592,1756},
 {2704,1824},{2808,1858},{2864,1920},{2790,2016},{2682,2083},{2544,2176},{2384,2216},
 {2240,2280},{2200,2400},{2064,2496},{1920,2496},{1792,2432},{1792,2152},{1792,1792},
 {1712,2504},{1592,2464},{1024,1792},{1024,2144},{1136,2265},{1360,2321},{1520,2368}
};
#define OR(a,b,w,k) {(a),(b),(w),(k)}
static const OuterRoad outerRoads[]={
 OR(0,1,32,OUT_HILL),OR(1,2,33,OUT_HILL),OR(2,3,33,OUT_HILL),OR(3,4,36,OUT_HILL),
 OR(4,5,35,OUT_HILL),OR(5,6,35,OUT_HILL),OR(6,7,31,OUT_HILL),OR(7,8,32,OUT_HILL),
 OR(8,9,34,OUT_HILL),OR(9,10,39,OUT_HILL),OR(10,11,36,OUT_HILL),OR(11,12,35,OUT_HILL),
 OR(12,13,34,OUT_HILL),OR(13,21,36,OUT_HILL),
 OR(14,15,47,OUT_HWY),OR(15,16,56,OUT_HWY),OR(16,17,57,OUT_HWY),OR(17,18,57,OUT_HWY),
 OR(18,19,55,OUT_HWY),OR(19,20,47,OUT_HWY),
 OR(21,18,39,OUT_WORKS),OR(18,22,39,OUT_WORKS),OR(22,24,36,OUT_WORKS),OR(24,23,34,OUT_WORKS),
 OR(23,16,34,OUT_WORKS),OR(16,26,34,OUT_WORKS),OR(26,25,29,OUT_WORKS),OR(25,21,29,OUT_WORKS),
 OR(25,24,28,OUT_WORKS),OR(24,27,33,OUT_WORKS),OR(27,32,34,OUT_WORKS),OR(22,28,35,OUT_WORKS),OR(28,36,38,OUT_WORKS),
 OR(29,30,37,OUT_DOCK),OR(30,31,37,OUT_DOCK),OR(31,32,39,OUT_DOCK),OR(32,33,43,OUT_DOCK),
 OR(33,34,38,OUT_DOCK),OR(34,35,40,OUT_DOCK),OR(35,36,42,OUT_DOCK),OR(36,37,45,OUT_DOCK),
 OR(37,38,48,OUT_DOCK),OR(38,39,45,OUT_DOCK),OR(39,40,40,OUT_DOCK),OR(40,41,43,OUT_DOCK),
 OR(41,42,42,OUT_DOCK),OR(42,43,41,OUT_DOCK),OR(43,44,39,OUT_DOCK),OR(44,45,43,OUT_DOCK),
 OR(45,46,43,OUT_DOCK),OR(46,47,39,OUT_DOCK),OR(47,48,42,OUT_DOCK),OR(48,29,33,OUT_DOCK),
 OR(47,50,39,OUT_COUNTRY),OR(50,51,38,OUT_COUNTRY),OR(51,56,36,OUT_COUNTRY),
 OR(52,53,33,OUT_COUNTRY),OR(53,54,35,OUT_COUNTRY),OR(54,55,37,OUT_COUNTRY),OR(55,56,37,OUT_COUNTRY)
};
#undef OR
typedef struct{int node;const char *label,*detail;U tint;} OuterLandmark;
static const OuterLandmark outerStops[]={
 {10,"GLASS BLUFF","Hillside sweepers and a view of the city lights.",0xb8a5ee},
 {19,"PARKWAY OVERLOOK","The wide straight is marked for high-speed runs.",0xe8ca90},
 {25,"BELT WORKSHOPS","Tight turns under the industrial gantries.",0xfa9b87},
 {36,"HARBOR PIER","Tidewater bends, striped bollards and an open pier.",0x7fddd6},
 {45,"SALT REACH","South shore bridges and broad, clear runoffs.",0x91d8bc},
 {54,"WINDMILL CURVE","Rural turnoff from the older Seawall Road.",0xc3d590}
};
/* Four pixels in a packed lane index carry timed fixtures and forward map
   routing while scenery, shoulder and hull geometry use the same ribbon. */
static unsigned char outerLane[ROOM*ROOM];
static unsigned char outerInk[ROOM*ROOM];
static int outer_is_new(float x,float y){return x>BASE_ROOM-36||y>BASE_ROOM-36;}
static int outer_candidate(int x,int y){return x>=BASE_ROOM-58||y>=BASE_ROOM-58;}
static int outer_onroad(float x,float y){
 if(!(x>=35&&y>=35&&x<=ROOM-36&&y<=ROOM-36))return 0;
 if(!outer_is_new(x,y))return 1;
 return outerLane[(int)y*ROOM+(int)x] != 0;
}
static int outer_area(float x,float y,float r){
 if(!outer_onroad(x,y))return 0;
 if(!outer_is_new(x+r,y+r))return 1;
 return outer_onroad(x+r,y+r)&&outer_onroad(x+r,y-r)&&outer_onroad(x-r,y+r)&&outer_onroad(x-r,y-r);
}
static U outer_noise(int x,int y){
 U v=(U)x*3266489917u+(U)y*374761393u+0x9e3779b9u;v=(v^(v>>13))*1274126177u;return v^(v>>16);
}
static float out_proj(OuterPoint a,OuterPoint b,float x,float y){
 float dx=b.x-a.x,dy=b.y-a.y,n=dx*dx+dy*dy;
 float t=((x-a.x)*dx+(y-a.y)*dy)/n;return t<=0?0:t>=1?1:t;
}
static void outer_stamp(int lane,int *extent){
 const OuterRoad*r=outerRoads+lane;OuterPoint a=outerNodes[r->a],b=outerNodes[r->b];int w=r->half;
 int xl=max(min(a.x,b.x)-w,35),xr=min(max(a.x,b.x)+w,ROOM-37);
 int yl=max(min(a.y,b.y)-w,35),yr=min(max(a.y,b.y)+w,ROOM-37);
 int dx=b.x-a.x,dy=b.y-a.y;float l=sq(dx*dx+dy*dy);
 for(int y=yl;y<=yr;y++)for(int x=xl;x<=xr;x++){
  if(!outer_candidate(x,y))continue;
  float t=out_proj(a,b,x,y),nx=a.x+dx*t,ny=a.y+dy*t;
  float ds=d2(x-nx,y-ny);
  if(ds>w*w)continue;
  int i=y*ROOM+x;
  int margin=w-(int)sq(ds)+1;
  if(margin<outerInk[i])continue;
  outerLane[i]=(unsigned char)(lane+1);outerInk[i]=(unsigned char)margin;*extent+=margin>0;
  if(roofMask[i]&&!outer_is_new(x,y))continue;
  int run=(int)(t*l),edge=margin<=3;
  int dist=w+1-margin;
  U c=r->kind==OUT_DOCK?0x314655:r->kind==OUT_HILL?0x404350:r->kind==OUT_COUNTRY?0x454649:0x353f50;
  if(dist<=1&&run%29<15)c=r->kind==OUT_HWY?0xd3af62:0xa99a70;
  if(r->kind==OUT_HWY&&dist>=22&&dist<=23&&run%33<16)c=0x758b99;
  if(margin>=5&&margin<=10)c=r->kind==OUT_DOCK?0x50666a:0x4d5c63;
  if(edge)c=r->kind==OUT_DOCK?0x8c7761:0x70828e;
  U n=outer_noise(x,y)&1;city[i]=n?mix(c,0x728189,1):c;
 }
}
static void outer_bake(void){
 static U districtTint[]={0x292b3d,0x263749,0x263e48,0x39363a};
 z(outerLane,sizeof outerLane);z(outerInk,sizeof outerInk);
 for(int y=0;y<ROOM;y++){
  int start=y<BASE_ROOM?BASE_ROOM:0;
  for(int x=start;x<ROOM;x++){
   const U g=outer_noise(x/6,y/6),gr=outer_noise(x/128,y/118);
   U c;
   if(x>2030&&y<820){
    int cliff=(y+(int)(g%18)+((x/3)%81))%92;
    c=mix(districtTint[0],0x60576a,(cliff<4)?3:gr%3);
   }else if(x>2030&&y<1600){
    const int yard=x%142<114&&y%121<88;
    c=mix(districtTint[1],0x59626d,yard?gr%4:g%2);
    if(yard&&(x%142>100||y%121>76))c=mix(c,0x172432,4);
    if(yard&&y%121==77&&x%15<7)c=0x9e8577;
   }else if((x>2030&&y>=1600)||(y>2070&&x>950&&x<2300)){
    int surface=(y+(int)(g%18))%57;c=mix(districtTint[2],0x3e6970,surface<3?3:1);
    if(x>2030&&y>1610&&(x/42+y/100)%7==0)c=mix(c,0x755e51,2);
   }else c=mix(0x182e3a,0x354959,((x/17+y+(g%32))%53==0)?2:0);
   city[y*ROOM+x]=c;
  }
 }
 /* Draw the branches before the main boulevard so the latter keeps its
    lane markings at major crossings. Road cells remain a geometric union. */
 int extent=0;
 for(int i=0;i<LEN(outerRoads);i++)outer_stamp(i,&extent);
 for(int i=0;i<LEN(outerStops);i++){
  const OuterLandmark*s=outerStops+i;OuterPoint p=outerNodes[s->node];
  int w=textw(s->label,0)+30;
  box(p.x-w/2,p.y-43,w,22,0x192838,s->tint);
  center(p.x,p.y-44,s->label,s->tint,0);
  for(int d=-1;d<=1;d+=2){
    rect(p.x+d*19-2,p.y+14,5,17,0x233440);disc(p.x+d*19,p.y+14,3,s->tint);
  }
 }
 center(1966,493,"GLASS BLUFF",0xa7afd2,0);
 center(1966,990,"FREIGHT PARKWAY",0xe9c891,0);
 center(1954,1759,"HARBOR PIER",0x83cfc2,0);
 center(1791,2056,"SALT REACH",0x8fd5b7,0);
 center(1024,2056,"WINDMILL CURVE",0xc3d590,0);
}
static int outer_nearest(float x,float y,int *px,int *py,int maxd){
 float best=maxd*maxd;int k=-1;
 for(int i=0;i<LEN(outerRoads);i++){
  const OuterRoad*r=outerRoads+i;OuterPoint a=outerNodes[r->a],b=outerNodes[r->b];
  float t=out_proj(a,b,x,y),nx=a.x+(b.x-a.x)*t,ny=a.y+(b.y-a.y)*t;
  float d=d2(nx-x,ny-y);if(d<best){best=d;k=i;*px=(int)(nx+.5f);*py=(int)(ny+.5f);}
 }
 return k;
}
