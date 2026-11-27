/* City masonry for the perspective camera. Indexed, authored wall materials,
 * stamped once per real address. No screen-space signs, extra doors, RNG,
 * collision bodies, or save state. Four separate elevations share a trim kit.
 * The eye-level geometry can approach the near clip; UVs interpolate in 1/Z. */
enum { CF_W=128,CF_H=64,CF_N=20,CF_LO=CF_W*CF_H,
 CF_PITCH=CF_LO+CF_LO/4+CF_LO/16 };
enum{CF_STONE,CF_JOINT,CF_LITBRICK,CF_COURSE,CF_LIGHT,CF_TRIM,CF_INK,
 CF_FRAME,CF_PANE,CF_SKY,CF_LAMP,CF_ROOM,CF_SILL,CF_SIGN,CF_BOARD,
 CF_SHADE,CF_CANVAS,CF_GOLD,CF_DARKGLASS,CF_PAPER};
enum{CF_MARKET,CF_DECO,CF_YARD,CF_CIVIC,CF_FLATS,CF_GLASS,CF_QUAY};
typedef struct{const char *name;U wall,accent;int style,door;} CfAddress;
static CfAddress cf_address(int i){
 const Building *b=buildings+i; int area=zone_id[i]-1;
 if(area>=0){
  static const unsigned char kit[]={CF_YARD,CF_DECO,CF_MARKET,CF_GLASS,CF_MARKET,
   CF_YARD,CF_GLASS,CF_DECO,CF_CIVIC,CF_FLATS,CF_YARD,CF_CIVIC,CF_FLATS,CF_FLATS,CF_YARD,CF_CIVIC};
  const VenuePlan *v=zone_plan(area);
  return (CfAddress){v->name,v->brick,v->light,kit[loc[area].kind],loc[area].x};
 }
 if(i==ncFrontId)return(CfAddress){"AFTERIMAGE",0x735063,0xf5a3c4,CF_DECO,ordinaryDoorX[i]};
 if(i==28)return(CfAddress){"COPPER PLAYHOUSE",0x945d57,0xf4c38c,CF_DECO,ordinaryDoorX[i]};
 if(i==NIGHT_CLUB_LEGACY)return(CfAddress){"SHADOW STAGE",0x74616a,0xefa2ae,CF_DECO,ordinaryDoorX[i]};
 static const unsigned char plan[]={CF_DECO,CF_DECO,CF_MARKET,CF_MARKET,CF_GLASS,CF_CIVIC,
  CF_YARD,CF_MARKET,CF_YARD,CF_GLASS,CF_DECO,CF_YARD,CF_QUAY,CF_YARD,CF_GLASS,CF_QUAY,
  CF_FLATS,CF_MARKET,CF_CIVIC};
 static const U clay[]={0xa37861,0x958a75,0xad887b,0x71888d,0x9d8a7d,0x8b838a,0x719289};
 int w=wi_index(i);
 return(CfAddress){oc_kind(i)==OC_NEIGHBOR?walkin_name(i):wiRoles[w].noun,mix(clay[(i*7+i/3)%LEN(clay)],b->roof,2),wiRoles[w].accent,plan[w],ordinaryDoorX[i]};
}
static int cf_door_u(int i,const CfAddress *a){
 StreetBounds b=building_bounds(buildings+i);const OcDoor*d=oc_entry(i);
 if(d&&d->family==OC_NEIGHBOR){
  int side=d->face,u=side==OC_LEFT?d->ay-b.y:side==OC_RIGHT?b.y+b.h-d->ay:side==OC_BACK?b.x+b.w-d->ax:d->ax-b.x;
  return cl(u*CF_W/max(1,side%2?b.h:b.w),7,120);
 }
 return cl((a->door-b.x)*CF_W/max(1,b.w),7,120);
}
typedef struct{unsigned char t[CF_PITCH],ready;} CfSheet;
static CfSheet cfSheets[LEN(buildings)][4];
static void cf_fill(unsigned char*t,int x,int y,int w,int h,int c){
 for(int j=max(y,0);j<min(y+h,CF_H);j++)for(int i=max(x,0);i<min(x+w,CF_W);i++)t[j*CF_W+i]=(unsigned char)c;
}
static void cf_text(unsigned char*t,int x,int y,const char*s,int col){
 for(int j=0;s[j];j++,x+=4){unsigned g=zone_letter(s[j]);
  for(int k=0;k<15;k++)if(g&(1u<<(14-k)))cf_fill(t,x+k%3,y+k/3,1,1,col);
 }
}
static void cf_window(unsigned char*t,int x,int y,int w,int h,int seed,int form){
 cf_fill(t,x-1,y-1,w+2,h+3,CF_JOINT);cf_fill(t,x,y,w,h,CF_FRAME);
 cf_fill(t,x+1,y+1,w-2,h-2,CF_PANE);
 int glow=(seed%9<3),col=glow?CF_ROOM:CF_SKY;
 cf_fill(t,x+1,y+1,w-2,form==CF_GLASS?h/2:2,col);
 if(glow){cf_fill(t,x+2,y+3,w/2-1,h-4,CF_LAMP);cf_fill(t,x+w/2,y+3,1,h-4,CF_FRAME);}
 else if(form!=CF_YARD){
  cf_fill(t,x+w/2,y+2,1,h-3,CF_FRAME);
  for(int k=0;k<h-4;k++)if(k+2<w/2)cf_fill(t,x+2+k,y+2+k,2,1,CF_SHADE);
  if(form==CF_GLASS)cf_fill(t,x+1,y+h-3,w-2,1,CF_ROOM);
 }
 if(form==CF_CIVIC||form==CF_FLATS)cf_fill(t,x+1,y+h/2,w-2,1,CF_FRAME);
 cf_fill(t,x-1,y+h,w+2,1,CF_SILL);
}
static void cf_colors(CfAddress a,int side,U *pal){
 U wall=mix(a.wall,side==1?0x8e9e9e:side==3?0x353e53:side==0?0x556576:0xbea587,side==2?1:2);
 pal[CF_STONE]=wall;pal[CF_JOINT]=mix(wall,0x252d38,3);
 pal[CF_LITBRICK]=mix(wall,PAPER,1);pal[CF_COURSE]=mix(wall,0x374454,4);
 pal[CF_LIGHT]=mix(wall,PAPER,3);pal[CF_TRIM]=mix(wall,0x6c7f83,4);pal[CF_INK]=0x182833;
 pal[CF_FRAME]=0x30414d;pal[CF_PANE]=0x3c5665;pal[CF_SKY]=0x78959a;
 pal[CF_LAMP]=0xe7bb86;pal[CF_ROOM]=0xad927c;pal[CF_SILL]=mix(wall,0xadad95,4);
 pal[CF_SIGN]=a.accent;pal[CF_BOARD]=mix(a.accent,0x112633,7);
 pal[CF_SHADE]=mix(a.accent,0x253541,6);pal[CF_CANVAS]=mix(a.accent,wall,3);
 pal[CF_GOLD]=0xb99c76;pal[CF_DARKGLASS]=0x253a46;pal[CF_PAPER]=0xe0d5ba;
}
/* Material mip levels reduce high-frequency window/mortar crawl while driving.
 * Palette match uses a tiny deterministic pair table per style, not random
 * point samples or full RGBA textures on the frame hot path. */
static unsigned char cf_pair[CF_N][CF_N];static int cfPairReady;
static void cf_pair_init(void){
 if(cfPairReady)return;cfPairReady=1;
 U p0[CF_N];cf_colors((CfAddress){0,0x94857d,0xd5b68d,0,0},2,p0);
 for(int i=0;i<CF_N;i++)for(int j=i;j<CF_N;j++){
  U mid=mix(p0[i],p0[j],4);int best=0,score=99999999;
  for(int k=0;k<CF_N;k++){
   int r=(int)(mid>>16)-(int)(p0[k]>>16),g=(int)((mid>>8)&255)-(int)((p0[k]>>8)&255),b=(int)(mid&255)-(int)(p0[k]&255),d=2*r*r+4*g*g+b*b;
   if(d<score){best=k;score=d;}
  }cf_pair[i][j]=cf_pair[j][i]=(unsigned char)(i==j?i:best);
 }
}
static void cf_mip(const unsigned char *s,unsigned char *dst,int w,int h){
 for(int y=0;y<h;y+=2)for(int x=0;x<w;x+=2){
  const unsigned char *p=s+y*w+x;*dst++=cf_pair[cf_pair[p[0]][p[1]]][cf_pair[p[w]][p[w+1]]];
 }
}
static CfSheet *cf_bake(int id,int side,const CfAddress *a){
 CfSheet*sheet=&cfSheets[id][side];if(sheet->ready)return sheet;
 sheet->ready=1;unsigned char*t=sheet->t;int form=a->style;
 int front=side==(oc_kind(id)==OC_NEIGHBOR?ocDoor[id].face:2),profile=front?form:form==CF_MARKET?CF_FLATS:form,seed=id*17+side*11;
 /* Quiet, structural mortar; individual details are subordinate to entries. */
 for(int y=0;y<64;y++)for(int x=0;x<128;x++){
  int val=CF_STONE;
  if(profile==CF_YARD){if((x%8)==0)val=CF_JOINT;else if((x%8)==1)val=CF_LITBRICK;}
  else if(profile==CF_GLASS){if(y%11==1)val=CF_JOINT;}
  else{int brick=(x+(y/5%2)*6)%12;
   if((y%5==0||!brick)&&((x*5+y*7+seed)&7)>2)val=CF_JOINT;
   else if(((x/12+3*(y/5)+seed)%11)==0)val=CF_LITBRICK;
  }
  t[y*CF_W+x]=val;
 }
 cf_fill(t,0,0,128,2,CF_LIGHT);cf_fill(t,0,2,128,2,CF_COURSE);
 cf_fill(t,0,61,128,3,CF_COURSE);
 int bays=(side&1)?5:(form==CF_CIVIC||form==CF_YARD)?5:6;
 int step=116/bays,left=(128-bays*step)/2,ww=step-(profile==CF_GLASS?2:7);
 for(int j=0;j<(front?2:3);j++)for(int k=0;k<bays;k++){
  int x=left+k*step+(step-ww)/2,y=7+j*(front?13:18);
  cf_window(t,x,y,ww,front?8:12,(seed+k*13+j*5)%91,profile);
 }
 if(!front){
  if(profile==CF_DECO){ /* vertical sign blade around the corner, with stepped neon rails */
   cf_fill(t,5,6,13,30,CF_BOARD);cf_fill(t,5,6,2,26,CF_CANVAS);cf_fill(t,8,5,1,29,CF_SIGN);
   for(int y=10;y<31;y+=5)cf_fill(t,11,y,(y%3)+3,2,CF_GOLD);
  }
  if(profile==CF_CIVIC||profile==CF_QUAY){
   for(int k=0;k<=bays;k++)cf_fill(t,left+k*step-1,5,2,55,CF_TRIM);
   cf_fill(t,0,37,128,2,CF_COURSE);
  }else if(profile==CF_YARD||id%3==1){
   /* Utility flank: rain leader, brackets and louvred service grating. */
   cf_fill(t,119,4,2,57,CF_TRIM);for(int y=12;y<60;y+=18)cf_fill(t,118,y,4,1,CF_LIGHT);
   cf_fill(t,6,48,13,10,CF_FRAME);for(int y=50;y<=55;y+=2)cf_fill(t,7,y,11,1,CF_TRIM);
  }
 }else{
  int door=cf_door_u(id,a),center=64,sw=sl(a->name)*4+7;
  cf_fill(t,0,35,128,25,form==CF_YARD?CF_SHADE:CF_TRIM);
  cf_fill(t,1,37,126,22,CF_FRAME);
  for(int k=0;k<bays;k++){
   int x=left+k*step+1,w=step-2;
   cf_fill(t,x,39,w,18,CF_DARKGLASS);
   if(form==CF_YARD){
    if(absf(x+w*.5f-door)<15)continue;
    cf_fill(t,x,40,w,17,CF_TRIM);
    for(int r=42;r<57;r+=3)cf_fill(t,x,r,w,1,CF_COURSE);
   }else{
    cf_fill(t,x+1,40,w-2,5,form==CF_DECO?CF_SHADE:CF_SKY);
    cf_fill(t,x+2,46,w-4,7,((seed+k)%3)?CF_PANE:CF_ROOM);
    if(form==CF_MARKET){
     cf_fill(t,x+2,51,w-4,3,CF_GOLD);cf_fill(t,x+4,48,3,3,CF_CANVAS);
    }else if(form==CF_DECO){
     cf_fill(t,x+3,46,2,8,CF_CANVAS);cf_fill(t,x+w-5,46,1,8,CF_SHADE);
    }
    cf_fill(t,x+w/2,40,1,16,CF_FRAME);
    cf_fill(t,x,56,w,1,CF_SILL);
   }
  }
  cf_fill(t,0,31,128,7,CF_SHADE);
  cf_fill(t,0,31,128,1,CF_LIGHT);cf_fill(t,0,37,128,1,CF_INK);
  if(form==CF_MARKET)for(int x=0;x<128;x+=8)cf_fill(t,x,33,4,4,CF_CANVAS);
  if(form==CF_DECO){
   cf_fill(t,1,32,126,1,CF_SIGN);cf_fill(t,3,35,122,1,CF_GOLD);
   for(int x=5;x<127;x+=8)cf_fill(t,x,37,1,1,CF_PAPER);
  }
  if(form==CF_CIVIC||form==CF_QUAY)for(int k=0;k<=bays;k++){
   int tx=left+k*step-2;cf_fill(t,tx,38,4,22,CF_LIGHT);cf_fill(t,tx+2,39,2,20,CF_COURSE);
  }
  cf_fill(t,door-7,42,14,21,CF_SILL);cf_fill(t,door-6,43,12,20,CF_INK);
  cf_fill(t,door-5,44,10,18,CF_FRAME);cf_fill(t,door-4,45,8,10,CF_SKY);
  cf_fill(t,door-3,48,6,7,CF_DARKGLASS);cf_fill(t,door+2,55,2,2,CF_PAPER);
  cf_fill(t,door-7,62,14,2,CF_LIGHT);
  /* A real lettered sign in its own 3D plane, never mirrored by winding. */
  cf_fill(t,center-sw/2-1,27,sw+2,9,CF_SIGN);cf_fill(t,center-sw/2,28,sw,7,CF_BOARD);
  cf_text(t,center-(sl(a->name)*4-1)/2,29,a->name,CF_SIGN);
  if(id==28||id==ncFrontId||zone_id[id]){
   int site=zone_id[id]-1;
   const char *billing=id==28?"MINA VALE":id==ncFrontId?"STRIP CLUB":zone_plan(site)->sign;
   int len=sl(billing)*4+4,sx=64-len/2;
   cf_fill(t,sx-1,20,len+2,8,CF_GOLD);cf_fill(t,sx,21,len,6,CF_BOARD);
   cf_text(t,64-(sl(billing)*4-1)/2,21,billing,CF_PAPER);
  }
 }
 cf_pair_init();cf_mip(t,t+CF_LO,128,64);cf_mip(t+CF_LO,t+CF_LO+CF_LO/4,64,32);return sheet;
}
typedef struct{float x,y,z,u,v;} CfV;
typedef CfV CfP;
static float cf_facade_distance(ChVec a,ChVec b){
 /* Stable when a vertex crosses the near plane. The previous clipped-corner
  * average re-lit an entire building during a turn past it. */
 float dx=b.x-a.x,dy=b.y-a.y,sq0=d2(dx,dy);
 float t=((ch.x-a.x)*dx+(ch.y-a.y)*dy)/sq0;t=t<0?0:t>1?1:t;
 return sq(d2(a.x+dx*t-ch.x,a.y+dy*t-ch.y))+.19f*sq(sq0);
}
static void cf_draw_side(int id,int side,const CfAddress *ad,ChVec left,ChVec right,float ht){
 /* Left/right are the screen-reading order from outside this wall. */
 ChVec a=ch_local(left),b=ch_local(right);left.z=right.z=ht;
 ChVec c=ch_local(right),d=ch_local(left);
 CfV q[4]={{a.x,a.y,a.z,0,64},{b.x,b.y,b.z,128,64},{c.x,c.y,c.z,128,0},{d.x,d.y,d.z,0,0}},v[6],prev=q[3];
 int n=0;for(int i=0;i<4;i++){
  CfV w=q[i];if((w.z>=9)!=(prev.z>=9)){
   float f=(9-prev.z)/(w.z-prev.z);
   v[n++]=(CfV){prev.x+(w.x-prev.x)*f,prev.y+(w.y-prev.y)*f,9,
    prev.u+(w.u-prev.u)*f,prev.v+(w.v-prev.v)*f};
  }if(w.z>=9)v[n++]=w;prev=w;
 }
 if(n<3)return;float ymin=H,ymax=0,xmin=W,xmax=0,dist=cf_facade_distance(left,right);CfP p0[6];
 for(int j=0;j<n;j++){
  float inv=1.f/v[j].z,ss=CH_F*inv;
  float x=W*.5f+v[j].x*ss,y=CH_MY-v[j].y*ss;
  p0[j]=(CfP){x,y,CH_ZS*inv,v[j].u*inv*CH_ZS,v[j].v*inv*CH_ZS};
  if(y<ymin)ymin=y;if(y>ymax)ymax=y;if(x<xmin)xmin=x;if(x>xmax)xmax=x;
 }
 if(xmin>=W||xmax<0||ymin>=H||ymax<0)return;
 U pal[CF_N];cf_colors(*ad,side,pal);
 for(int k=0;k<CF_N;k++){
  U c0=pal[k];if(k==CF_SKY)c0=nw_blend(c0,0x304650,(int)(160*chDark));
  if(k==CF_LAMP||k==CF_ROOM)c0=nw_blend(0x697978,c0,(int)(70+185*chDark));
  U damp=ch_dye(c0,dist);
  if(k==CF_SIGN||k==CF_LAMP)damp=nw_blend(damp,c0,(int)(chDark*cl(160-(int)dist/7,0,160)));
  pal[k]=damp;
 }
 const unsigned char *tex=cf_bake(id,side,ad)->t;
 int yminI=cl((int)ymin,0,H-1),ymaxI=cl((int)ymax+1,0,H);
 for(int y=yminI;y<ymaxI;y++){
  float sy=y+.5f;CfP row[2];int count=0;
  for(int j=0,k=n-1;j<n;k=j++){
   CfP l=p0[j],r=p0[k];if((l.y>sy)==(r.y>sy))continue;
   float f=(sy-l.y)/(r.y-l.y);
   if(count<2)row[count++]=(CfP){l.x+f*(r.x-l.x),sy,l.z+f*(r.z-l.z),l.u+f*(r.u-l.u),l.v+f*(r.v-l.v)};
  }
  if(count!=2)continue;CfP l=row[row[0].x>row[1].x],r=row[row[0].x<=row[1].x];
  float span=r.x-l.x;if(span<.001f)continue;
  int start=cl((int)(l.x+.5f),0,W),end=cl((int)(r.x+.5f),0,W);if(end<=start)continue;
  float inv=1.f/span,stepZ=(r.z-l.z)*inv,stepU=(r.u-l.u)*inv,stepV=(r.v-l.v)*inv;
  float h=start+.5f-l.x,iz=l.z+stepZ*h,iu=l.u+stepU*h,iv=l.v+stepV*h;
  int pos=y*W+start;float stU=iu/iz,stV=iv/iz;
  for(int x=start;x<end;){
   int len=min(8,end-x);
   /* A long fixed span stretched texels sharply at grazing angles. Adapt
    * only at the steepest depths; regular city draws remain eight-wide. */
   float dz=absf(stepZ);
   if(dz*8>iz*.090f)len=min(len,dz*4>iz*.09f?2:4);
   float nextZ=iz+stepZ*len,ni=1.f/nextZ;
   float enU=(iu+stepU*len)*ni,enV=(iv+stepV*len)*ni;
   /* Oblique repeat detail uses a smaller atlas, not a maze of shimmering
    * one-pixel features. Latitude as well as depth matters on side walls. */
   float fp=absf(enU-stU)+absf(enV-stV);int mip=(fp>3.2f*len||iz<1500)?2:(fp>1.65f*len||iz<3080)?1:0;
   int scale=1024>>mip,du=(int)((enU-stU)*scale/len),dv=(int)((enV-stV)*scale/len);
   int u=(int)(stU*scale),vv=(int)(stV*scale),width=128>>mip,height=64>>mip;
   const unsigned char*t=tex+(mip==0?0:mip==1?CF_LO:CF_LO+CF_LO/4);
   for(int j=0;j<len;j++,u+=du,vv+=dv,pos++,iz+=stepZ){
    int depth=(int)iz;if(depth>=chZ[pos]){chZ[pos]=depth;pixels[pos]=pal[t[cl(vv>>10,0,height-1)*width+cl(u>>10,0,width-1)]];}
   }
   iz=nextZ;iu+=stepU*len;iv+=stepV*len;stU=enU;stV=enV;x+=len;
  }
 }
}
static void cf_block(float l,float t,float r,float b,float lo,float h,U wall,U roof){
 if(ch.y<t)ch_quad((ChVec){l,t,lo},(ChVec){r,t,lo},(ChVec){r,t,h},(ChVec){l,t,h},mix(wall,0x405160,2),1);
 if(ch.x>r)ch_quad((ChVec){r,t,lo},(ChVec){r,b,lo},(ChVec){r,b,h},(ChVec){r,t,h},mix(wall,0xaeb7ab,1),1);
 if(ch.y>b)ch_quad((ChVec){l,b,lo},(ChVec){r,b,lo},(ChVec){r,b,h},(ChVec){l,b,h},wall,1);
 if(ch.x<l)ch_quad((ChVec){l,t,lo},(ChVec){l,b,lo},(ChVec){l,b,h},(ChVec){l,t,h},mix(wall,0x243a4a,3),1);
 if(ch.h>=h)ch_quad((ChVec){l,t,h},(ChVec){r,t,h},(ChVec){r,b,h},(ChVec){l,b,h},roof,1);
}
static int cf_seen(float x,float y,float half,float top,float limit){
 float dx=x-ch.x,dy=y-ch.y,d=dx*ch.co+dy*ch.si,side=absf(dy*ch.co-dx*ch.si);
 /* Keep very close structures even when the eye pivots steeply down. */
 if(d2(dx,dy)>(limit+half)*(limit+half)||d< -half-top||side>1.5f*(absf(d)+top+half)+half)return 0;
 return 1;
}
static void cf_roof(int id,const CfAddress *a,StreetBounds f,float ht,float dist2){
 float l=f.x,r=l+f.w,t=f.y,b=t+f.h,w=f.w,h=f.h,x=l+.42f*w,y=t+.48f*h;
 U roof=buildings[id].roof,att=mix(a->wall,0x536771,3),flashing=mix(att,0xb3b6a6,2);
 if(ch.h>=ht)ch_quad((ChVec){l,t,ht},(ChVec){r,t,ht},(ChVec){r,b,ht},(ChVec){l,b,ht},roof,1);
 if(a->style==CF_DECO){ /* stepped crown with two ceramic fins */
  cf_block(l+.18f*w,t+.28f*h,r-.17f*w,b-.18f*h,ht,ht+9,att,flashing);
  cf_block(l+.32f*w,t+.37f*h,r-.29f*w,b-.30f*h,ht+9,ht+14,mix(a->wall,a->accent,2),flashing);
 }else if(a->style==CF_YARD||a->style==CF_QUAY){
  /* Twin sawtooth monitor roofs, direction fixed to the workshop. */
  for(int j=0;j<2;j++){
   float lo=l+w*(.18f+j*.30f),hi=lo+.22f*w,y0=t+.18f*h,y1=b-.17f*h;
   ch_quad((ChVec){lo,y0,ht},(ChVec){hi,y0,ht+9},(ChVec){hi,y1,ht+9},(ChVec){lo,y1,ht},flashing,1);
   if(ch.x>hi)ch_quad((ChVec){hi,y0,ht},(ChVec){hi,y1,ht},(ChVec){hi,y1,ht+9},(ChVec){hi,y0,ht+9},0x56717a,1);
   float yy=ch.y<(y0+y1)*.5f?y0:y1;ChVec g[]={{lo,yy,ht},{hi,yy,ht},{hi,yy,ht+9}};ch_face(g,3,att,1);
  }
 }else if(a->style==CF_GLASS||a->style==CF_CIVIC){
  cf_block(l+.22f*w,t+.18f*h,r-.21f*w,b-.24f*h,ht,ht+(a->style==CF_GLASS?12:8),a->style==CF_GLASS?0x47737c:att,flashing);
 }else if(a->style==CF_FLATS){
  cf_block(x-8,y-7,x+8,y+7,ht,ht+11,att,flashing);
  if(id%2==0)cf_block(r-22,t+16,r-15,t+22,ht,ht+13,0x966f61,flashing);
 }else cf_block(x-10,y-6,x+10,y+6,ht,ht+5,att,flashing);
 if(dist2<315.f*315.f&&ch.h>ht+14){ /* flashings and roof fan grille */
  cf_block(r-.22f*w-6,t+.39f*h-4,r-.22f*w+6,t+.39f*h+4,ht,ht+3,0x44545d,0x83918d);
 }
}
static void cf_simple_sign(float x,float y,float z0,float px,const char*s,U ink){
 float start=x-(sl(s)*4-1)*px*.5f;for(int c=0;s[c];c++){
  unsigned g=zone_letter(s[c]);
  for(int row=0;row<5;row++)for(int k=0;k<3;){
   if(!(g&(1u<<(14-row*3-k)))){k++;continue;}
   int j=k;do{k++;}while(k<3&&(g&(1u<<(14-row*3-k))));
   float l=start+(c*4+j)*px,r=start+(c*4+k)*px,h=z0-row*px;
   ch_quad((ChVec){l,y,h-px},(ChVec){r,y,h-px},(ChVec){r,y,h},(ChVec){l,y,h},ink,1);
  }
 }
}
static void cf_city_fixtures(void){
 for(int i=0;i<LEN(bankAtms);i++){
  float x=bankAtms[i].x,y=bankAtms[i].y;if(!cf_seen(x,y,13,24,430))continue;
  cf_block(x-10,y-14,x+10,y,0,22,0x344c5a,0x748c8a);
  cf_block(x-11,y-15,x+11,y+1,20,24,0x60877e,0x7a9b97);
  if(ch.y<=y)continue;
  #define CF_PLANE(L,R,Y,B,T,C) ch_quad((ChVec){(L),(Y),(B)},(ChVec){(R),(Y),(B)},(ChVec){(R),(Y),(T)},(ChVec){(L),(Y),(T)},(C),1)
  CF_PLANE(x-7,x+7,y+.2f,10,19,0x10282e);CF_PLANE(x-5.5f,x+5.5f,y+.3f,11.5f,17.8f,0x8ad7c8);
  CF_PLANE(x-6,x+6,y+.2f,5.3f,6.7f,0x172935);CF_PLANE(x-4,x+4,y+.3f,6,6.7f,0xc3cdb3);
  if(d2(x-ch.x,y-ch.y)<220*220)cf_simple_sign(x,y+1.25f,23.7f,.65f,"ATM",0xc9ffea);
 }
 /* Bell Lane's inset threshold is a small real-world annex, not a HUD box. */
 {float x=NT_DOOR_X,y=NT_DOOR_Y-9;
  if(cf_seen(x,y,15,19,400)){
   U accent=night_trap_owned()?nt_mint:nt_amber;
   cf_block(x-10,y-3,x+10,y,0,18,0x566069,0x89948c);
   if(ch.y>y){
    CF_PLANE(x-7,x+7,y+.2f,0,16,0x253844);CF_PLANE(x-5,x+5,y+.3f,7,13,0x415861);
    CF_PLANE(x+3,x+5,y+.4f,6,7.1f,accent);
    cf_block(x-14,y-3,x+14,y+.65f,16,22,0x343c42,0x7f806b);
    cf_simple_sign(x,y+.82f,20.7f,.66f,"TRAP HOUSE",accent);
   }
  }
 }
 #undef CF_PLANE
}
#include "chase_city_trim.inc"
static void ch_buildings(void){
 for(int i=0;i<nb;i++){
  int family=oc_kind(i);if(family==OC_QUAY||family==OC_WOOD)continue;
  const Building*b=buildings+i;if(b->w<=0||b->h<=0)continue;
  StreetBounds f=building_bounds(b);float l=f.x,r=l+f.w,t=f.y,bt=t+f.h;
  float x=(l+r)*.5f,y=(t+bt)*.5f,dd=d2(x-ch.x,y-ch.y);
  float ht=22.f+1.55f*b->ht+(i%11==6?19:0);
  if(!cf_seen(x,y,.5f*(f.w+f.h),ht,1000))continue;
  CfAddress a=cf_address(i);
  if(ch.y<t)cf_draw_side(i,0,&a,(ChVec){r,t,0},(ChVec){l,t,0},ht);
  if(ch.x>r)cf_draw_side(i,1,&a,(ChVec){r,bt,0},(ChVec){r,t,0},ht);
  if(ch.y>bt)cf_draw_side(i,2,&a,(ChVec){l,bt,0},(ChVec){r,bt,0},ht);
  if(ch.x<l)cf_draw_side(i,3,&a,(ChVec){l,t,0},(ChVec){l,bt,0},ht);
  cf_roof(i,&a,f,ht,dd);
  int serviceSide=i%3!=0?OC_RIGHT:OC_LEFT;
  if(dd<310.f*310.f&&(family==OC_NONE||ocDoor[i].face!=serviceSide))cf_service_wall(i,&a,f,ht);
  if(ch.y>bt-.3f&&(family==OC_NONE||ocDoor[i].face==OC_FRONT))cf_front_architecture(i,&a,f,ht);
 }
 cf_city_fixtures();
}

/* Both facade sides use exact door anchor from the registered footprint. */
static void ch_oc_small_door(int id,float maxh){
 const OcDoor*d=oc_entry(id);if(!d)return;
 float x=d->ax,y=d->ay,dx=d->face==OC_LEFT?-.45f:d->face==OC_RIGHT?.45f:0,dy=d->face==OC_BACK?-.45f:d->face==OC_FRONT?.45f:0;
 float u=dy!=0?1.f:0.f,v=dx!=0?1.f:0.f;
 x+=dx;y+=dy;
 for(int i=0;i<3;i++){
  float w=6.f-i*1.f,z=0.12f+i*.12f,h=maxh-i*.56f;U ink=i==0?0xbca986:i==1?0x4a6569:0x21343c;
  float a=x+dx*i*.16f,b=y+dy*i*.16f;
  ch_quad((ChVec){a-u*w,b-v*w,z},(ChVec){a+u*w,b+v*w,z},(ChVec){a+u*w,b+v*w,h},(ChVec){a-u*w,b-v*w,h},ink,1);
 }
 float a=x+dx*.61f,b=y+dy*.61f;
 ch_quad((ChVec){a-u*2,b-v*2,maxh*.52f},(ChVec){a+u*2,b+v*2,maxh*.52f},(ChVec){a+u*2,b+v*2,maxh*.74f},(ChVec){a-u*2,b-v*2,maxh*.74f},0x7caa9c,1);
}
