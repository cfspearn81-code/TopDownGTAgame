/* C only. Keep the original address, crosswalk and light map; these repeated
 * civic details sit in road space rather than sliding around in screen space.
 * A tiny indexed overlay keeps branches and blends out of the per-pixel path. */
static unsigned char cfRoadInk[256*256];static int cfRoadInkReady;
static const U cfRoadColor[]={0,0x49565c,0x38454d,0x606767,0x726c57,0x7e8179,0x222e35,0x525d5e,0x39464a,0x303d44,0x86928a};
static void cf_road_prepare(void){
 if(cfRoadInkReady)return;cfRoadInkReady=1;
 for(int y=0;y<256;y++)for(int x=0;x<256;x++){
  int u=min(x,256-x),v=min(y,256-y),cross=min(u,v),len=u>v?x:y;
  int perp=u>v?y:x;if(perp>=128)perp-=256;
  int edge=absf(perp),c=0;
  if(max(u,v)>41){
   /* shoulder/gutter joints, storm grate and a faded service cover */
   if(edge==29)c=5;
   if(edge==27)c=6;
   if(edge==26&&len%45<16)c=2;
   if(edge>=23&&edge<28&&(len>104&&len<119)){
    c=(len==105||len==118||edge==23)?3:len%3?6:1;
   }
   if(edge==1&&(len%36)<18)c=4;
   if(edge>=30&&edge<=41&&(len%20==0))c=8;
   if(edge==41)c=2;
   int mx=len-165,my=perp-9,rr=mx*mx+my*my;
   if(rr<=20)c=rr>12?6:((mx+my)&3)==0?1:2;
   /* tar repair: sparse, low contrast and broken at the paint lines */
   if(len>59&&len<80&&perp< -8&&perp== -10-(len-59)/7)c=9;
  }else if(cross<20&&max(u,v)>39&&max(u,v)<42)c=7; /* stop line */
  cfRoadInk[y*256+x]=(unsigned char)c;
 }
}
static U cf_road_sample(U old,unsigned x,unsigned y){
 unsigned char c=cfRoadInk[((y&255)<<8)|(x&255)];return c?cfRoadColor[c]:old;
}
