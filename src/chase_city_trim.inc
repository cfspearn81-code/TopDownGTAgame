/* Affixed architectural details stay attached to the original wall.
 * Roof trim overhangs and lintels don't become new painted obstacles. Close
 * inspection benefits from genuine depth; distant fronts keep their mips. */
static inline float cf_front_row(float ht,float row){return ht*(64.f-row)/64.f;}
static inline void cf_board(float l,float r,float y,float lo,float hi,U c,int shade){
 ch_quad((ChVec){l,y,lo},(ChVec){r,y,lo},(ChVec){r,y,hi},(ChVec){l,y,hi},c,shade);
}
static inline void cf_flat(float l,float r,float t,float b,float h,U c,int shade){
 ch_quad((ChVec){l,t,h},(ChVec){r,t,h},(ChVec){r,b,h},(ChVec){l,b,h},c,shade);
}
static U cf_lens(U ink){
 /* No pulsing highlights or abrupt day-night threshold; lamp glass
  * brightens continuously as the city lighting follows the game clock. */
 return nw_blend(mix(ink,0x537175,5),mix(ink,PAPER,2),(int)(60+195*chDark));
}
static void cf_front_architecture(int id,const CfAddress*a,StreetBounds f,float ht){
 float lt=f.x,rt=lt+f.w,y=f.y+f.h,dx=a->door-ch.x,dy=y-ch.y;
 float distance=d2(dx,dy);if(distance>520*520||!cf_seen(a->door,y,f.w*.5f,ht,360))return;
 float l=lt+3,r=rt-3,w=r-l,x=a->door,x0=x-8,x1=x+8;
 float fascia=cf_front_row(ht,37),skert=cf_front_row(ht,61),lintel=cf_front_row(ht,41);
 if(x0<l)x0=l;if(x1>r)x1=r;
 U warm=mix(a->wall,0xc4b79b,3),shade=mix(a->wall,0x222d3a,4),accent=a->accent;
 U rim=mix(a->wall,accent,3),light=cf_lens(accent);
 int near=distance<255*255;
 if(a->style==CF_MARKET){
  /* Stripe fabric on a sloped canopy. Light seen below gives a stall
   * clear depth even while the street wears its grey/rain palette. */
  float al=x-22>l?x-22:l,ar=x+30<r?x+30:r,room=ar-al;
  int n=near?9:3;
  for(int k=0;k<n;k++){
   float a0=al+k*room/n,a1=al+(k+1)*room/n;
   U fabric=(k&1)?mix(rim,0xeece93,5):mix(rim,0x354d55,2);
   ch_quad((ChVec){a0,y-.15f,fascia+.4f},(ChVec){a1,y-.15f,fascia+.4f},
    (ChVec){a1,y+6,fascia-2.8f},(ChVec){a0,y+6,fascia-2.8f},fabric,1);
   cf_board(a0,a1,y+6,fascia-4.2f,fascia-2.8f,mix(fabric,shade,1),1);
  }
  if(near){
   cf_block(al, y-.2f,al+1.4f,y+5.5f,fascia-5,fascia-3.9f,shade,warm);
   cf_block(ar-1.4f,y-.2f,ar,y+5.5f,fascia-5,fascia-3.9f,shade,warm);
  }
 }else if(a->style==CF_DECO){
  /* Marquee glazing is a self-lit two-layer band, not a billboard facing
   * the camera. Deco lettering is still the original façade atlas. */
  float offset=id==28||id==ncFrontId?6.1f:4.2f,c=(l+r)*.5f;
  cf_block(l,y-.14f,r,y+offset,fascia-1.9f,fascia+.6f,shade,rim);
  cf_board(l,r,y+offset+.1f,fascia-.72f,fascia-.15f,mix(light,0x6c455d,1),0);
  if(near){
   cf_board(l+4,r-4,y+offset+.15f,fascia-1.46f,fascia-1.12f,mix(light,0x254056,1),0);
   for(int i=-3;i<=3;i++){
    float k=c+i*w/8;cf_flat(k-.64f,k+.64f,y+offset-1.86f,y+offset-.85f,fascia-2.03f,light,0);
   }
   if(id==ncFrontId||id==28){
    float z0=cf_front_row(ht,18),s=(r-l)*.31f;
    cf_block(c-s,y-.08f,c+s,y+2.2f,z0,z0+.65f,shade,rim);
    cf_board(c-s,c+s,y+2.35f,z0+.04f,z0+.38f,light,0);
    cf_board(l+2,l+3,y+.23f,fascia+3,ht-3,light,0);
    cf_board(r-3,r-2,y+.23f,fascia+3,ht-3,light,0);
   }
  }
 }else if(a->style==CF_CIVIC||a->style==CF_QUAY){
  /* Plinth, square columns and caps frame without blocking entrances. */
  float ex=x+15<r?x+15:r,ax=x-14>l?x-14:l,lo=fascia-3.5f;
  cf_block(ax-1,y-.15f,ex+1,y+5,lo,fascia,shade,warm);
  if(near)for(int i=0;i<2;i++){
   float b=i?ex-1.8f:ax+1.8f;
   cf_block(b-1.2f,y-.08f,b+1.2f,y+2.2f,.8f,lo,warm,warm);
   cf_block(b-1.8f,y-.02f,b+1.8f,y+2.8f,.15f,1.15f,shade,warm);
   cf_board(b-1.8f,b+1.8f,y+2.8f,lo-1,lo,mix(warm,PAPER,1),1);
  }
 }else if(a->style==CF_GLASS){
  /* Narrow metal edge, lower smoked glass and pressure caps. */
  cf_block(l,y-.13f,r,y+1.4f,fascia-1.45f,fascia,shade,mix(rim,0x9fb9bb,3));
  if(near)for(int k=1;k<=5;k++){
   float b=l+k*w/6;
   if(absf(b-x)<9)continue;
   cf_board(b-.44f,b+.44f,y+.28f,skert,fascia-1,0x62787c,1);
  }
 }else{ /* stonemason's ledges on tenements, shed lintels on the yards */
  cf_block(l,y-.13f,r,y+1.6f,fascia-.85f,fascia,shade,warm);
  if(near&&a->style==CF_FLATS){
   float z1=cf_front_row(ht,31),step=w/6;
   for(int j=1;j<6;j+=2){
    float b=l+j*step;cf_block(b-step*.64f,y-.2f,b+step*.44f,y+1.56f,z1,z1+.8f,shade,warm);
   }
  }
 }
 if(!near)return;
 /* The original entrance is still in the visible atlas. Frame it with
  * real jambs, a recessed overhead trim and low-profile threshold plate. */
 cf_block(x0-.4f,y-.23f,x0+.67f,y+1.1f,skert,lintel,shade,warm);
 cf_block(x1-.67f,y-.23f,x1+.4f,y+1.1f,skert,lintel,shade,warm);
 cf_block(x0-.7f,y-.18f,x1+.7f,y+1.8f,lintel-1.16f,lintel,shade,warm);
 cf_flat(x0-.5f,x1+.5f,y,y+4,.19f,mix(shade,warm,3),1);
 if(chDark>.15f)cf_board(x0+4,x1-4,y+1.85f,lintel-1.02f,lintel-.66f,light,0);
}
/* Use side-facing closed walls as service alleys: one side can host a
 * fire ladder or rain leader, never more than one assembly per building. */
static void cf_service_wall(int id,const CfAddress*a,StreetBounds f,float ht){
 float left=f.x,right=f.x+f.w,front=f.y+f.h;
 int east=id%3!=0;
 float x=east?right:left,sign=east?1:-1,y=f.y+f.h*.61f;
 if(east?ch.x<=right:ch.x>=left)return;
 if(!cf_seen(x,y,12,ht,250))return;
 float dx=ch.x-x,dy=ch.y-y,dd=d2(dx,dy);if(dd>280*280)return;
 U steel=mix(a->wall,0x293744,6),cap=mix(steel,0x9a9b94,3);
 float a0=x+sign*.42f,a1=x+sign*4.4f;
 if((a->style==CF_FLATS||a->style==CF_DECO)&&front-y>10.f){
  /* Raised, flush-mounted ladder is a silhouette and cannot be climbed.
   * Nearby crossbars give parallax without remodelling the building. */
  float hi=ht*.76f,lo=7.5f,s0=y-2,s1=y+2;
  float lead=x+sign*2.9f;
  ch_quad((ChVec){lead,s0-.36f,lo},(ChVec){lead,s0+.36f,lo},
   (ChVec){lead,s0+.36f,hi},(ChVec){lead,s0-.36f,hi},cap,1);
  ch_quad((ChVec){lead,s1-.36f,lo},(ChVec){lead,s1+.36f,lo},
   (ChVec){lead,s1+.36f,hi},(ChVec){lead,s1-.36f,hi},cap,1);
  if(dd<215*215)for(float z=lo+.6f;z<hi;z+=2.2f)
   ch_quad((ChVec){lead,s0,z},(ChVec){lead,s1,z},(ChVec){lead,s1,z+.45f},(ChVec){lead,s0,z+.45f},steel,1);
  for(int i=1;i<=2;i++){
   float z=ht*(.33f+i*.18f),ys=y+i*2;
   cf_flat(a0<a1?a0:a1,a1>a0?a1:a0,ys-7,ys+5,z,cap,1);
   ch_quad((ChVec){a1,ys-7,z-1},(ChVec){a1,ys+5,z-1},(ChVec){a1,ys+5,z},
           (ChVec){a1,ys-7,z},steel,1);
  }
 }else if(a->style==CF_YARD||id%5==2){
  float z=ht*.81f,l=x+sign*1.3f;
  ch_quad((ChVec){l,y-1.08f,3},(ChVec){l,y+1.08f,3},(ChVec){l,y+1.08f,z},
          (ChVec){l,y-1.08f,z},steel,1);
  for(int j=1;j<=3;j++){
   float h=ht*(.18f*j);
   ch_quad((ChVec){l+sign*.14f,y-1.5f,h},(ChVec){l+sign*.14f,y+1.5f,h},
    (ChVec){l+sign*.14f,y+1.5f,h+.67f},(ChVec){l+sign*.14f,y-1.5f,h+.67f},cap,1);
  }
 }
}
/* Earlier chase views drew a grey camera-facing rectangle with a square on
 * top. Existing municipal lamp anchors now have a solid stem, road-facing
 * arm and lens. No change to a lamp's official lighting or ground position. */
static void cf_city_lamp(const StreetLamp*s){
 float x=s->x,y=s->y;
 int old=s->radius==27;float cap=old?21:32,reach=old?4:9;
 if(!cf_seen(x,y,10,cap,625))return;
 U steel=nw_blend(0x354553,0x56666e,64),metal=mix(steel,0xafab8a,2),glint=cf_lens(s->lens);
 /* The short historic pool faces north-west, the newer arm sits inward
  * from the south-east curb. Lens bottom is visible, not a black tile. */
 float ny=y+(old?-reach:reach),l=x-2,r=x+2,t=ny-2,b=ny+2;
 cf_block(x-1,y-1,x+1,y+1,0,cap-1,steel,metal);
 if(d2(ch.x-x,ch.y-y)<290.f*290.f){
  cf_block(x-1.65f,y-1.65f,x+1.65f,y+1.65f,.1f,3,steel,metal);
  cf_block(x-.76f,old?ny:y,x+.76f,old?y:ny,cap-1.85f,cap-.48f,steel,metal);
 }
 cf_block(l,t,r,b,cap-1.55f,cap,steel,metal);
 cf_flat(l+.4f,r-.4f,t+.28f,b-.28f,cap-1.63f,glint,0);
 if(chDark>.33f)cf_board(l+.42f,r-.42f,b+.03f,cap-1.50f,cap-1.17f,glint,0);
}
