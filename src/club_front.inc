/* Base Village frontage. Adds one shell to the otherwise empty west lot, after
   all old building and venue IDs have been assigned. No career state lives here. */
static void night_club_front_lot(void){
 ncFrontId=-1;
#if __has_include("night_club.inc")
 enum{TILE=2*8, L=58, T=568, DEPTH=28, WIDTH=146, HEIGHT=93};
 int *row=lot[TILE];
 if(row[0]||nb>=LEN(buildings))return; /* Never replace a pre-existing address. */
 ncFrontId=nb++;
 buildings[ncFrontId]=(Building){L,T,WIDTH,HEIGHT,DEPTH,0x4f3853,0x674553};
 row[0]=1;row[1]=ncFrontId;
 /* Reset only the park's square; sidewalk, pavement and zebra markings remain. */
 for(int y=554;y<726;y++)for(int x=42;x<214;x++){
  U hash=zone_hash(x,y);int v=(int)((hash>>27)&7);
  city[y*ROOM+x]=(x%18==0||y%12==0)?0x3c303d:RGB(45+v,41+v,48+v);
 }
#endif
}
static void paint_afterimage_front(void){
 if(ncFrontId<0)return;
 const Building*b=buildings+ncFrontId;
 const int l=b->x,r=l+b->w,top=b->y-b->ht,end=b->y+b->h,x=ordinaryDoorX[ncFrontId];
 const U court=0x251d31,rose=0xffa5ca,blue=0x92e0e5,gold=0xf9d7a0;
 /* Stacked Art Deco marquee: a physically legible purpose above the name. */
 box(l+13,top+14,b->w-26,51,0x342134,0x987385);
 box(l+16,top+17,b->w-32,45,0x2b2233,rose);
 zone_lettering((l+r)/2,top+22,"STRIP CLUB",rose,2);
 zone_lettering((l+r)/2,top+42,"THE AFTERIMAGE",gold,2);
 for(int i=0;i<9;i++){
  int bx=l+22+i*12;
  dot(bx,top+19,blue);dot(bx,top+57,rose);
 }
 line(l+19,top+36,r-19,top+36,0x9e6b84);
 for(int i=0;i<14;i++){
  int bx=l+9+i*10;
  rect(bx,top+68,2,9,i%3?0x845973:rose);
 }
 line(l+6,top+79,r-6,top+79,0x7e4e69);
 light(x,end+7,24,0xf093b4,4);
 for(int i=0;i<5;i++){int blade=9+i*2;rect(x-blade,end+i*3,blade*2+1,2,i&1?0x765475:0x644468);}
 box(x-15,end-21,31,23,court,blue);
 box(x-12,end-18,25,20,0x533849,rose);rect(x-10,end-16,9,13,0x755972);rect(x+2,end-16,9,13,0x755972);
 rect(x-3,end-8,2,3,gold);rect(x+3,end-8,2,3,gold);
 box(x-24,end-25,49,7,0x362434,gold);rect(x-19,end-22,39,2,rose);
 for(int i=-1;i<=1;i+=2){
  disc(x+i*23,end-7,2,gold);rect(x+i*23,end-6,2,9,0x7b5769);
  rect(x+i*25,end+9,2,9,0x5b4359);disc(x+i*25,end+8,2,0xd1a583);
 }
 /* Local sidewalk graphic points at the same E threshold used by collision. */
 for(int i=0;i<4;i++)line(x-i,end+24+i,x+i,end+24+i,rose);
 rect(x-1,end+27,3,6,rose);
}
