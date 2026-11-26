/* Port Vanta Afterhours: shared fixed-step weather, skyline and shoreline art. */
static float nwWet, nwStorm, nwFog;
static int night_world_is_night(void){return sv.dayClock<355.f||sv.dayClock>=1110.f;}
static int night_world_weather(void){
 const int minute=(int)sv.dayClock;
 if(nv.world[0])return NIGHT_CLEAR;
 if((minute>=245&&minute<344)||(minute>=1100&&minute<1290))return NIGHT_RAIN;
 if((minute>=344&&minute<431)||(minute>=1290&&minute<1430))return NIGHT_FOG;
 return NIGHT_CLEAR;
}
static float nw_ease(float x){return x<=0?0:x>=1?1:x*x*(3-2*x);}
static float nw_dark(void){
 return (sv.dayClock<660.f?1.f-nw_ease((sv.dayClock-330.f)/105.f):nw_ease((sv.dayClock-1040.f)/125.f));
}
static void night_world_reset(void){nwWet=nwStorm=nwFog=0;}
static void night_world_tick(float dt){
 if(nv.world[0])nv.world[0]--;
 int weather=night_world_weather();
 nwStorm+=((weather==NIGHT_RAIN)-nwStorm)*dt*.82f;
 nwFog+=((weather==NIGHT_FOG)-nwFog)*dt*.46f;
 nwWet+=((weather==NIGHT_RAIN)-nwWet)*dt*(weather==NIGHT_RAIN?.35f:.055f);
}
static float night_world_grip(void){return 1.f-.15f*nwWet;}
static float night_world_brake(void){return 1.f-.19f*nwWet;}
static float night_world_foot(void){return 1.f-.10f*nwWet;}
static void night_rest_clock(unsigned hours,int clear_weather){
 if(hours>12)hours=12;
 /* Use the real economy clock rather than changing its day label alone. */
 for(unsigned tick=0;tick<hours*60;tick++){
  production_tick(1.f);
  if(sv.charges){
   sv.sweepTicks=max(1,sv.sweepTicks-59);
   revenue_tick();
  }
 }
 night_phone_rest((int)hours);
 if(clear_weather)nv.world[0]=120*60;
}
static int nw_seawall_y(int x){
 return 1999+(int)(sn(x*.008f)*3.5f);
}
static int nw_ridge_y(int x){
 return 256+(int)(sn(x*.006f)*5.5f);
}
static void nw_road_graphics(void){
 /* These center lines fit the actual vehicle collision hull. The Seawall
  * corridor links the lower ends of the street grid along the waterfront. */
 for(int x=48;x<BASE_ROOM-48;x++){
  int y=nw_seawall_y(x);
  for(int k=-13;k<=13;k++){
   int py=y+k;
   if(roofMask[py*ROOM+x])continue;
   U ink=k==-12?0x72808a:k==12?0x669899:k==0&&x%42<19?0xa1a376:
      (k>-8&&k<8)?0x354553:0x32414a;
   dot(x,py,ink);
  }
 }
 for(int roadX=256;roadX<=1792;roadX+=256){
  for(int y=1830;y<nw_seawall_y(roadX);y+=25){
   rect(roadX-21,y,1,11,0x7e8c83);rect(roadX+21,y,1,11,0x7e8c83);
  }
  rect(roadX-9,nw_seawall_y(roadX)-6,19,2,0x8c9182);
  rect(roadX-9,nw_seawall_y(roadX)+6,19,2,0x8c9182);
 }
 for(int x=280;x<997;x++){
  int y=nw_ridge_y(x);
  if(x%256<=47||x%256>=209)continue;
  if(x%36<17){dot(x,y,0x968d78);dot(x,y-1,0x827a6d);}
  dot(x,y-24,0x687586);dot(x,y+24,0x687586);
 }
 for(int y=288;y<1760;y++){
  if(y%256<=42||y%256>=214)continue;
  dot(1792-25,y,0x696f8d);dot(1792+25,y,0x696f8d);
  if(y%54<16){dot(1791,y,0x9e9773);dot(1792,y,0x9e9773);}
 }
 /* Ground stencils show up in the city below, not only in a guide. */
 textat(653,1988,"SEAWALL ROAD",0x91b8ad,0);
 textat(624,229,"GLASS RIDGE   /   WATCH THE BENDS",0xacc1c5,0);
 textat(1801,1320,"EAST FREIGHT",0xa5a5bd,0);
 textat(1801,1338,"EXPRESS",0xa5a5bd,0);
}
static void night_world_bake(void){
 nw_road_graphics();
 /* New docks and bluff silhouettes are in the non-colliding border;
  * existing drivable routes, building entrances and legacy pins are unchanged. */
 for(int y=0;y<2;y++){
  const int yy=y?BASE_ROOM-27:6;
  for(int x=16;x<BASE_ROOM-16;x+=8){
   int g=(x/64+y*5)%7;
   rect(x,yy,8,17, y?0x193544:0x242e3e);
   rect(x,yy+(y?4:1),8,1,y?0x295161:0x333f56);
   if(g>2)rect(x+1,yy+7+g/2,5,1,y?0x244756:0x3d4759);
  }
 }
 for(int i=0;i<7;i++){
  int x=108+i*267;
  if(i%3==1)x+=43;
  for(int j=0;j<3;j++){rect(x+j*15,BASE_ROOM-26,12,17-(i+j)%6,0x4b4d4b);rect(x+2+j*15,BASE_ROOM-24,7,2,0x7b6151);}
  rect(x-7,BASE_ROOM-28,53,2,0x8e7b6b);
  rect(x+19,BASE_ROOM-12,3,11,0x69554c);
  rect(x,BASE_ROOM-31,42,1,0x9c6651);
 }
 /* Dashed parking-bay paint by the established shoreline road. */
 for(int x=112;x<1940;x+=104){
  int y=1792+17;
  if(!roofMask[y*ROOM+x])rect(x,y,26,1,0x666659);
 }
}
static U nw_blend(U a,U b,int weight){
 U k=(U)weight,m=256-k;
 U rb=(((a&0x00ff00ffu)*m+(b&0x00ff00ffu)*k)>>8)&0x00ff00ffu;
 U g=(((a&0x0000ff00u)*m+(b&0x0000ff00u)*k)>>8)&0x0000ff00u;
 return rb|g;
}
/* Recover illumination already baked into the historic tint, rather than
 * adding a differently colored second pool on top. Fog tints the lens
 * without ever changing its shape or drawing through a roof. */
static void nw_fixture(const StreetLamp*s,int veilWeight,int tint){
 int x=sx(s->x),y=sy(s->y),r=s->radius,r2=r*r;
 if(x+r<0||x-r>=W||y+r<0||y-r>=H)return;
 U haze=tint>55?0x40526b:0x949fa1;
 for(int yy=max(y-r,0);yy<min(y+r,H);yy++){
  int sy0=(yy+cy)*ROOM+cx,veil=(veilWeight*(H-yy)+H/2)/H;
  for(int xx=max(x-r,0);xx<min(x+r,W);xx++){
   int dx=xx-x,dy=yy-y,d2=dx*dx+dy*dy;
   if(d2>=r2||roofMask[sy0+xx])continue;
   int lens=dx>=s->lensX&&dx<s->lensX+s->lensW&&dy>=s->lensY&&dy<s->lensY+s->lensH;
   int pole=dx>=s->poleX&&dx<s->poleX+s->poleW&&dy>=s->poleY&&dy<s->poleY+s->poleH;
   U ink=city[sy0+xx];
   int weight=lens?0:pole?tint:tint*(38+162*d2/r2)/256;
   U original=ink;
   if(weight)ink=nw_blend(ink,0x121727,weight);
   if(veil)ink=nw_blend(ink,haze,veil);
   int i=yy*W+xx;
   if(!lens){
    /* Preserve tire streaks and other ground layers already composited. */
    U prior=nw_blend(original,0x121727,tint);if(veil)prior=nw_blend(prior,haze,veil);
    U current=pixels[i];
    if(current!=prior){
     int rgb[3];for(int k=0;k<3;k++){
      unsigned shift=(unsigned)k*8;
      rgb[k]=cl((int)((current>>shift)&255)+(int)((ink>>shift)&255)-(int)((prior>>shift)&255),0,255);
     }
     ink=RGB(rgb[2],rgb[1],rgb[0]);
    }
   }
   pixels[i]=ink;
  }
 }
 if(nwWet>.22f){
  int h=9+(int)(nwWet*22),seed=(s->x+s->y)&3;
  for(int k=8;k<h;k+=3){
   int yy=y+k;
   if((U)yy>=H)continue;
   int width=max(1,(s->lensW+1)/2-k/7),x0=x+((seed+k*4)%5)-2;
   for(int xx=max(0,x0-width);xx<min(W,x0+width+1);xx++){
    if(roofMask[(cy+yy)*ROOM+cx+xx])continue;
    int i=yy*W+xx;pixels[i]=mix(pixels[i],s->pool,k>h/2?1:2);
   }
  }
 }
}
static void night_world_street_fx(void){
 float dark=nw_dark();
 int tint=(int)(92*dark+.5f);
 if(tint||nwFog>.08f){
  U haze=dark>.6f?0x40526b:0x949fa1;
  int fog=fastGfx?0:(int)(nwFog*88);
  for(int y=0;y<H;y++){
   int veil=(fog*(H-y)+H/2)/H;
   for(int x=0;x<W;x++){
    U c=pixels[y*W+x];
    if(tint)c=nw_blend(c,0x121727,tint);
    if(veil)c=nw_blend(c,haze,veil);
    pixels[y*W+x]=c;
   }
  }
 }
 if(fastGfx)return;
 if(dark>.22f){
  const int fog=(int)(nwFog*88);
  StreetLamp fixture;
  for(int gy=max(1,(cy-85)/256);gy<=min(BASE_ROOM/256-1,(cy+H+85)/256);gy++)
   for(int gx=max(1,(cx-85)/256);gx<=min(BASE_ROOM/256-1,(cx+W+85)/256);gx++)
    for(int k=0;k<2;k++)if(street_lamp_legacy(gx,gy,k,&fixture))nw_fixture(&fixture,fog,tint);
  for(int i=0;i<LEN(outerNodes);i+=2)if(street_lamp_outer(i,&fixture))nw_fixture(&fixture,fog,tint);
  /* Late buses and clubgoers read immediately as short moving silhouettes. */
  static const int aw[3][2]={{777,1789},{1526,523},{1270,1041}};
  for(int k=0;k<3;k++){
   int x=sx(aw[k][0]),y=sy(aw[k][1]);
   if((unsigned)x>=W||(unsigned)y>=H)continue;
   for(int j=0;j<4;j++){
    int pace=(int)(anim*(5+j))%31;
    person(x+j*11-16,y+(k&1?31-pace:pace)-15,j&1?0x8b6381:0x53898e,1,(k&1)?-1.57f:1.57f,0,j);
   }
  }
 }
 const int streaks=(int)(nwStorm*145.f);
 for(int i=0;i<streaks;i++){
  int x=(i*197+((int)(anim*61))%W)%W,y=(i*233+((int)(anim*211))%H)%H;
  U c=mix(pixels[y*W+x],0x92abc7,2);
  line(x,y,x-3,y+5+(i%4),c);
 }
 if(nwWet>.50f){
  int x=(int)(anim*147)%W,y=(int)(anim*79)%H;
  for(int i=0;i<16;i++){
   int px=(x+i*157)%W,py=(y+i*251)%H;
   if(roofMask[(cy+py)*ROOM+cx+px])continue;
   dot(px,py,0x729eae);dot(px+1,py-1,0x6b8294);
  }
 }
}
static void night_world_hud(void){
 int w=night_world_weather();
 if(p.dead||atmOpen>=0||menu>=0)return;
 U accent=w==NIGHT_RAIN?0x8cbee2:w==NIGHT_FOG?0xc0b0d4:0xffcf8b;
 const char *weather=w==NIGHT_RAIN?"RAIN":w==NIGHT_FOG?"FOG":nwWet>.12f?"WET STREETS":"CLEAR";
 char b[50];
 char *q=append(b,weather);q=append(q,night_world_is_night()?" / NIGHT":" / DAY");
 (void)q;
 const int width=textw(b,0)+15;
 box(129,16,width,24,0x18202c,0x48596a);
 textat(136,17,b,accent,0);
 if(scene==1)textat(18,483,night_phone_toggle(PHONE)?"P  SIGNAL":"F7  SIGNAL",0x9ad7b9,0);
}
