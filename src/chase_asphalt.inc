/* C only. Ground details are world-locked. A small material atlas restores
 * visual flow at street speed without changing road or sidewalk collision. */
static unsigned char cfRoadInk[256*256];static int cfRoadInkReady;
static const U cfRoadColor[]={
 0,0x4a575a,0x3d4951,0x5d6668,0x8c7852,0x83877d,0x29363e,0x707b77,
 0x36464c,0x354249,0x5c696d,0x565f63,0x353e45,0x657474,0x556368
};
static void cf_road_prepare(void){
 if(cfRoadInkReady)return;cfRoadInkReady=1;
 for(int y=0;y<256;y++)for(int x=0;x<256;x++){
  int u=min(x,256-x),v=min(y,256-y),cross=min(u,v),len=u>v?x:y;
  int perp=u>v?y:x;if(perp>=128)perp-=256;
  int edge=absf(perp),c=0;
  if(max(u,v)>41){
   /* Split curbstone cap, underside, gutter and poured slab joints. The
    * inner lip has a soft line rather than the old harsh double edge. */
   if(edge==29)c=len%14!=0?5:8;
   if(edge==30)c=10;
   if(edge==28)c=6;
   if(edge==27)c=2;
   if(edge>=31&&edge<=40&&(len/9+((edge-30)>>2))%3)c=11;
   if(edge>=31&&edge<=41&&(len%18==0||edge==37))c=8;
   if(edge==41)c=14;
   if(edge==25&&len%45<16)c=9;
   if(edge>=23&&edge<28&&len>104&&len<119)
    c=(len==105||len==118||edge==23)?3:len%3?6:1;
   if(edge<=1&&(len%36)<18)c=4;
   int mx=len-165,my=perp-9,rr=mx*mx+my*my;
   if(rr<=31)c=rr>20?6:((mx+my)&3)==0?1:2;
   /* A visible curbside chip and repaired lane strip, not visual noise in
    * the far field. The color can be swapped by tile address cheaply. */
   if(len>62&&len<86&&perp>=-14&&perp<=-12+(len-62)/10)c=12;
   if(len>84&&len<91&&edge>=29&&edge<=31)c=14;
  }else if(cross<20&&max(u,v)>38&&max(u,v)<42)c=7; /* stop line */
  cfRoadInk[y*256+x]=(unsigned char)c;
 }
}
static const U cfRoadWard[4][3]={
 {0x525f64,0x69716e,0x46575d}, {0x665e61,0x78716d,0x594a52},
 {0x62615a,0x79766b,0x555951}, {0x4f6668,0x607475,0x40555a}
};
static inline U cf_road_sample(U old,unsigned x,unsigned y){
 unsigned s=x&255,t=y&255,c=cfRoadInk[(t<<8)|s];
 if(!c)return old;
 unsigned ward=y<600?1:x<820?2:x>1330?3:0;
 if(c==11||c==5||c==14)return cfRoadWard[ward][c==11?0:c==5?1:2];
 if(c==12&&(((x>>8)*13+(y>>8)*7)&1))return old;
 return cfRoadColor[c];
}
