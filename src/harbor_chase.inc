/* Shore decor is anchored to the dry paving and true dock footprints.
 * Boat hulls, wakes and commissions live in the separate marine module. */
static void ch_sea_boats(void);
static void wl_beacon(float x,float y,int i){
 if(wl_visible(x,y,6,13,670)<-9900||!harbor_nav_water(x,y,3))return;
 float bob=.42f*sn(anim*1.55f+i*1.13f);
 WlRing f=wl_ring(6);
 U paint=i&1?0xb37958:0x5b9487,lamp=i&1?0xf5b46c:0xa3d9bb;
 wl_crown(&f,x,y,.60f+bob,3.5f,.99f,2.6f,1,paint);
 wl_box(x-.68f,y-.68f,x+.68f,y+.68f,2.2f+bob,8.2f+bob,0x4d615e,paint);
 wl_box(x-1.25f,y-1.25f,x+1.25f,y+1.25f,6.0f+bob,7.1f+bob,0xd2c9ab,0xbabd9d);
 wl_box(x-.83f,y-.83f,x+.83f,y+.83f,8.2f+bob,9.4f+bob,lamp,lamp);
}
/* Street paving meets the pier face at these authored deck tips. Braces,
 * tires and the boarding ladder remain decorative: all E and collision
 * queries continue to read the original berth and water masks. */
static void wl_dock_face(int id,float x,float y){
 static const float tip[]={12,13,12},half[]={20,16,17};
 int n=id/2;float edge=y+tip[n],l=x-half[n],r=x+half[n];
 wl_box(l,edge-1.2f,r,edge+.11f,-2.75f,.34f,0x766853,0xa79773);
 wl_box(l+.88f,edge-.20f,r-.86f,edge+.27f,-1.9f,-1.46f,0x544a42,0x726352);
 for(int i=-1;i<=1;i+=2){
  float u=x+i*(half[n]-3.f);
  wl_box(u-.93f,edge-1.65f,u+.93f,edge+.64f,-3.8f,1.35f,0x504942,0xa99372);
  float ox=x+i*8.3f;
  /* Tired rubber fender, inset lighter hub reveals the ring shape. */
  for(int j=0;j<8;j++){
   float t=j*.785398163f,f=t+.785398163f,a=cs(t),b=sn(t),c=cs(f),d=sn(f);
   ch_quad((ChVec){ox+2.25f*a,edge+.78f,-1.1f+2.48f*b},(ChVec){ox+2.25f*c,edge+.78f,-1.1f+2.48f*d},
           (ChVec){ox+.96f*c,edge+.85f,-1.1f+1.14f*d},(ChVec){ox+.96f*a,edge+.85f,-1.1f+1.14f*b},0x303b44,1);
  }
 }
 for(int s=-1;s<=1;s+=2){
  float u=x+s*2.58f;
  wl_box(u-.26f,edge+.25f,u+.26f,edge+.88f,-3.9f,2.66f,0x867844,0xd3ad69);
  wl_box(u-.26f,edge-.98f,u+.26f,edge+.51f,2.32f,2.68f,0xaa915b,0xe1c07f);
  wl_box(u-.28f,edge-1.02f,u+.28f,edge-.48f,.28f,2.37f,0x9f8b59,0xcdaf78);
 }
 for(int j=0;j<4;j++){float z=.12f-j*1.20f;
  wl_box(x-2.61f,edge+.51f,x+2.61f,edge+.89f,z-.18f,z+.09f,0xa08d5d,0xd4b47b);
 }
 /* Blunt anti-skid boarding strip reads from the pilot's back and
  * straight down the walking deck. */
 for(int j=0;j<5;j++){float u=x-9.f+j*3.8f;
  ch_quad((ChVec){u,edge-2.9f,.24f},(ChVec){u+1.4f,edge-2.9f,.24f},(ChVec){u+2.25f,edge-1.9f,.24f},(ChVec){u+.83f,edge-1.9f,.24f},0xc4a669,1);
 }
}
static void ch_harbor_props(void){
 /* The player can steer beyond the decorative quay's draw bounds. */
 ch_sea_boats();
 if(ch.x<1650||ch.y<1370)return;
 for(int i=0;i<LEN(harborPieces);i++){
  if(!harborPiecePlaced[i])continue;
  const HarborPiece*q=&harborPieces[i];float l=q->x,r=l+q->w,t=q->y,b=t+q->h,h=q->height,x=(l+r)*.5f,y=(t+b)*.5f;
  float d=wl_visible(x,y,h*.6f,q->w+q->h+20.f,755);
  if(d<-9900)continue;
  U panel=q->body,roof=q->roof,dark=mix(panel,0x243848,3);
  if(q->kind==HB_WINCH){
   wl_box(l,t,r,b,0,4,dark,roof);
   for(int side=-1;side<=1;side+=2){float yy=y+side*3;wl_box(x-1,yy-.75f,x+1,yy+.75f,4,h,0xb19062,0xd9bb88);}
   wl_box(x-1.75f,t,x+1.75f,b,h-1.8f,h,0x846e51,0xd1ba86);
   ch_quad((ChVec){x-1,b,h-1},(ChVec){x+1,b,h-1},(ChVec){x+9,b+7,h+8},(ChVec){x+7,b+7,h+8},0xb99a63,1);
   wl_box(x+7.9f,b+6.7f,x+8.35f,b+7.15f,h-2,h+8,0x536264,0x536264);continue;
  }
  wl_box(l,t,r,b,0,h,panel,roof);
  if(q->kind==HB_SHED||q->kind==HB_KIOSK){
   ch_quad((ChVec){l-1.2f,t-1.3f,h},(ChVec){r+1.2f,t-1.3f,h},(ChVec){r+1.2f,y,h+3},(ChVec){l-1.2f,y,h+3},roof,1);
   ch_quad((ChVec){l-1.2f,y,h+3},(ChVec){r+1.2f,y,h+3},(ChVec){r+1.2f,b+1.3f,h},(ChVec){l-1.2f,b+1.3f,h},mix(roof,0x314759,2),1);
   wl_tri((ChVec){l,t,h},(ChVec){l,b,h},(ChVec){l,y,h+3},panel);
   wl_tri((ChVec){r,t,h},(ChVec){r,b,h},(ChVec){r,y,h+3},panel);
   if(d<385){for(float xx=l+3;xx<r-5;xx+=9.f){
     wl_box(xx,t-.18f,xx+4.8f,b+.18f,5,h-2,0x9db6ae,0xb7c5ad);}
    ch_oc_small_door(oc_placed_door(OC_QUAY,i),h*.78f);}
  }else{
   if(q->kind==HB_STACK)wl_box(l+1.6f,t+1.7f,r-1.6f,b-1.7f,h,h+4.8f,0x7f9392,0xa3b2a8);
   if(d<345){
    for(float xx=l+2;xx<r-1;xx+=3.5f)wl_box(xx,t-.13f,xx+.50f,b+.13f,.8f,h-.7f,dark,mix(roof,PAPER,1));
    for(float yy=t+2;yy<b-1;yy+=3.2f)wl_box(l-.13f,yy,r+.13f,yy+.44f,1,h-.8f,mix(panel,0x344451,2),roof);
    wl_box(l+.8f,t-.22f,l+4.8f,b+.22f,h*.52f,h*.72f,0xd6c3a2,0xc6b898);
   }
  }
 }
 static const short buoys[][2]={{2913,2257},{2906,2316},{2872,2354},{2796,2291},{2781,2347},
  {2708,2432},{2615,2480},{2517,2571},{2448,2585},{3018,2209}};
 for(int i=0;i<LEN(buoys);i++)wl_beacon(buoys[i][0],buoys[i][1],i);
 for(int i=0;i<HARBOR_N_DOCK_POINTS;i+=2){
  float x=harborDockPoints[i].x,y=harborDockPoints[i].y;
  if(wl_visible(x,y,4,34,680)<-9900)continue;
  wl_dock_face(i,x,y);
  for(int k=-1;k<=1;k+=2){
   float u=x+(i?13:15)*k;
   for(int j=0;j<2;j++){float v=y-17+j*7;
    wl_box(u-1.4f,v-1.4f,u+1.4f,v+1.4f,0,5.3f,0x725b46,0xc3a681);
    wl_box(u-1.5f,v-1.5f,u+1.5f,v+1.5f,3.3f,4.0f,0xc5b995,0xc5b995);
   }
  }
 }
}
