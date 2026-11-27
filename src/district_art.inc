/* Port Vanta address book: each venue owns a real city lot. No draw below
   this property grid touches a vehicle lane, traffic footprint, or crosswalk. */
typedef struct {
 int gx,gy,x,y,w,h,depth,door;
 U brick,roof,light;
 const char *name,*sign;
} VenuePlan;
static const VenuePlan zone_plans[16]={
 /* Address (block), physical shell, canopy height, entrance offset. */
 {1,1,48,85,142,108,25,46,0x486b63,0x344d52,0x8de8cb,"MERCURY","COURIER"},
 {2,2,57,82,99,118,30,29,0x614258,0x45384c,0xea8ec1,"VAULT 39","MEMBERS"},
 {3,1,45,86,149,111,24,38,0x867251,0x534c46,0xf7cb75,"FERRY","MARKET"},
 {4,2,47,90,146,110,25,48,0x536c89,0x3c4d63,0x9bcffc,"NIGHTGLASS","MOTORS"},
 {2,4,48,87,143,104,24,42,0x638163,0x354f51,0xc9ef90,"LANTERN","GROW LOFT"},
 {5,4,48,79,150,119,29,45,0x596287,0x43495a,0xbcb0f6,"CINDER","WORKS"},
 {4,0,46,86,151,105,24,39,0x66738a,0x3b435b,0x91dcf0,"GLASS MILE","CIRCUIT"},
 {6,2,57,103,91,91,20,30,0x785265,0x423646,0xf2a2ce,"KIOSK 09","EXCHANGE"},
 {1,5,48,80,145,115,30,46,0x667b7d,0x4d5a68,0xf5dea3,"TIDEWAY","CLINIC"},
 {6,5,55,88,107,102,27,35,0x775f59,0x4d4753,0xf7c58d,"BREAKWATER","SAFEHOUSE"},
 {7,3,46,95,150,98,23,52,0x7a6b56,0x4e5151,0xf2c286,"WRECKER","RECOVERY"},
 {3,0,47,88,149,109,34,51,0x697084,0x50596d,0xe0c89b,"REVENUE","CIVIL ANNEX"},
 {0},
 {1,2,49,75,119,71,31,35,0x8a635d,0x565465,0xf8c897,"TIDE FLATS","APT 4"},
 {3,2,51,89,129,106,28,39,0x53667a,0x354d60,0xeab483,"VANTA","ARMORY"},
 {5,1,49,85,148,110,33,53,0x61769a,0x4b597a,0xa8c7ff,"VANTA POLICE","HQ / 04"}
};
static int zone_idx[LEN(loc)],zone_owner[64],zone_id[LEN(buildings)];
static const VenuePlan *zone_plan(int n){return zone_plans+loc[n].kind;}
static int zone_is_lot(int i){return zone_owner[i]>=0;}
static int zone_is_building(int i){return zone_id[i]>0;}
static U zone_hash(int x,int y){
 U u=(U)x*374761393u+(U)y*668265263u;
 return (u^(u>>13))*1274126177u;
}
static void zone_venues(void){
 for(int i=0;i<LEN(zone_owner);i++)zone_owner[i]=-1;
 z(zone_id,sizeof zone_id);
 for(int n=0;n<LEN(loc);n++){
  const VenuePlan *r=zone_plan(n);
  int tile=r->gy*8+r->gx,ox=r->gx*256,oy=r->gy*256,*a=lot[tile];
  int i=a[0]?a[1]:nb++;
  for(int j=2;j<=a[0];j++)buildings[a[j]]=(Building){0};
  zone_owner[tile]=n;zone_idx[n]=i;zone_id[i]=n+1;
  buildings[i]=(Building){ox+r->x,oy+r->y,r->w,r->h,r->depth,r->roof,r->brick};
  a[0]=1;a[1]=i;a[2]=0;
  loc[n].x=ox+r->x+r->door;loc[n].y=oy+r->y+r->h+18;
  // Remove the former generic park/lot footprint while preserving nearby
  // sidewalks, markings and asphalt exactly as setupCity laid them out.
  for(int y=43;y<214;y++)for(int x=43;x<214;x++){
   U noise=zone_hash(ox+x,oy+y);int v=(noise>>28)&7;U c;
   if(x%19==0||y%14==0)c=0x2d3a41;
   else if(loc[n].kind==8||loc[n].kind==11||loc[n].kind==15)c=RGB(48+v,57+v,66+v);
   else c=RGB(40+v,50+v,54+v);
   city[(oy+y)*ROOM+ox+x]=c;
  }
 }
}
/* A miniature pixel-letterpress for text fixed to masonry at its own
   physical size. Signs never depend on the transient street UI label. */
static unsigned zone_letter(char ch){
 static const unsigned ABC[]={0x2bed,0x6bae,0x3923,0x6b6e,0x79a7,0x79a4,0x396b,0x5bed,0x7497,0x126a,0x5bad,0x4927,0x5fed,0x6b6d,0x2b6a,0x6ba4,0x2b51,0x6bad,0x388e,0x7492,0x5b6f,0x5b6a,0x5bfd,0x5aad,0x5a92,0x72a7};
 static const unsigned nums[]={0x7b6f,0x2c97,0x62a7,0x62ce,0x5bc9,0x798e,0x39ef,0x7292,0x7bef,0x7bce};
 if(ch>='A'&&ch<='Z')return ABC[ch-'A'];
 if(ch>='0'&&ch<='9')return nums[ch-'0'];
 return ch=='/'?0x12a4:0;
}
static void zone_lettering(int cx0,int y,const char*s,U c,int px){
 int len=sl(s),x=cx0-(len*4*px-px)/2;
 for(int k=0;k<len;k++,x+=4*px){
  unsigned g=zone_letter(s[k]);
  for(int i=0;i<15;i++)if(g&(1u<<(14-i)))rect(x+(i%3)*px,y+(i/3)*px,px,px,c);
 }
}
static void zone_shadow(int x0,int y0,int x1,int y1,int dx,int dy,U c){
 int xx=x0+2+dx,yy=y0+3+dy;int endx=x1+dx,endy=y1+dy;
 if(endx<=xx||endy<=yy)return;
 for(int y=yy;y<endy;y++)for(int x=xx;x<endx;x++){
  U*p0=city+y*ROOM+x;*p0=mix(*p0,c,4);
 }
}
static void zone_glow(const VenuePlan*r,int x,int y,int radius,U tint,int weight){
 int xx=r->gx*256,yy=r->gy*256,d=radius*radius;
 for(int v=max(yy+42,y-radius);v<min(yy+225,y+radius);v++){
  for(int u=max(xx+42,x-radius);u<min(xx+215,x+radius);u++){
   int a=(d-d2(x-u,y-v))*weight/d;
   if(a>0){U*p0=city+v*ROOM+u;*p0=mix(*p0,tint,min(7,a));}
  }
 }
}
static void zone_window(int x,int y,int w,int h,U a,int lit){
 box(x,y,w,h,lit?mix(a,0x334b60,4):0x253849,0x22313e);
 if(w>5){rect(x+1,y+1,w-2,1,lit?mix(a,PAPER,3):0x688495);rect(x+w/2,y+2,1,h-3,0x364a55);}
 if(h>7)rect(x+2,y+h-4,w-4,2,lit?mix(a,0x786258,3):0x344657);
}
static void zone_vent(int x,int y,int w,int h,U a){
 rect(x+2,y+3,w,h,0x263442);box(x,y,w,h,0x365063,mix(a,0x788891,4));
 for(int i=3;i<h-1;i+=3)rect(x+2,y+i,w-4,1,0x78929a);
}
static void zone_hvac(int x,int y,U r){
 box(x,y,22,18,mix(r,0x87999e,4),0x263945);
 disc(x+10,y+8,6,0x263946);
 disc(x+10,y+8,4,0x4e6971);
 line(x+7,y+7,x+13,y+9,0x8a9da1);line(x+12,y+5,x+9,y+11,0x839095);dot(x+10,y+8,0xadada3);
}
static void zone_stripe(int x,int y,int w,int h,U c,U a){
 rect(x,y,w,h,c);
 for(int j=0;j<h;j++)for(int i=0;i<w;i++)if(((i+j)>>2)&1)dot(x+i,y+j,a);
}
static void zone_icon(int x,int y,int kind,U a){
 switch(kind){
 case 0: /* shipping mark */
  line(x-9,y+5,x-4,y,a);line(x-4,y,x+8,y,a);line(x+4,y-4,x+8,y,a);line(x+8,y,x+4,y+4,a);line(x-10,y+7,x+4,y+7,a);break;
 case 7:case 1: /* cropped neon ring */
  disc(x,y,9,a);disc(x,y,7,0x373d50);line(x-6,y+5,x+5,y-5,a);break;
 case 9:case 13: /* a house number and chevron */
  line(x-10,y+1,x,y-7,a);line(x,y-7,x+10,y+1,a);box(x-5,y,11,8,0x354951,a);break;
 case 8: /* walk-in clinic */
  rect(x-2,y-9,5,18,a);rect(x-9,y-2,19,5,a);break;
 case 11: /* civic triangle */
  tri(x-10,y-2,x,y-8,x+10,y-2,a);for(int i=-1;i<2;i++)rect(x+5*i-1,y,3,8,a);break;
 case 14:
  line(x-9,y-7,x-9,y+1,a);line(x-9,y+1,x,y+8,a);line(x,y+8,x+9,y+1,a);line(x+9,y+1,x+9,y-7,a);line(x-9,y-7,x+9,y-7,a);
  line(x-5,y,x+5,y,a);line(x-3,y,x-4,y+4,a);break;
 case 15: /* precinct shield */
  tri(x-10,y-7,x+10,y-7,x,y+10,a);tri(x-7,y-5,x+7,y-5,x,y+6,0x465e7e);rect(x-4,y-2,9,3,a);break;
 default:
  line(x-7,y-6,x+1,y-6,a);line(x+1,y-6,x+7,y,a);line(x+7,y,x+1,y+6,a);line(x+1,y+6,x-7,y+6,a);line(x-7,y+6,x-7,y-6,a);break;
 }
}
static void zone_entrance(const VenuePlan*r,Building*b,int kind){
 int x=b->x+r->door,y=b->y+b->h;
 U a=r->light,m=r->brick,ink=0x182b3b;
 int w=kind==13?30:kind==1||kind==7?32:40;
 int h=min(19,b->ht-2),left=x-w/2;
 // Lit threshold and flush stairs end before the frontage sidewalk.
 zone_glow(r,x,y+9,26,a,3);
 for(int i=0;i<3;i++){int ww=w+6+i*4;rect(x-ww/2,y+1+2*i,ww,2,mix(m,0x35434f,3+i));}
 if(bakeFloorOnly)return;
 rect(left,y-h,w,h+1,mix(m,ink,3));
 box(left+5,y-h+3,w-10,h-3,ink,mix(m,a,3));
 rect(left+7,y-h+6,w/2-8,h-8,0x48596a);rect(x+2,y-h+6,w/2-9,h-8,0x3a5264);
 rect(left+7,y-h+6,w/2-8,2,mix(a,PAPER,4));rect(x+2,y-h+6,w/2-9,2,mix(a,PAPER,4));
 rect(x-2,y-8,1,3,a);rect(x+2,y-8,1,3,a);
 rect(left,y-h,w,2,mix(a,PAPER,3));
 rect(left-4,y-2,5,2,0x879ba2);rect(x+w/2,y-2,5,2,0x879ba2);
 for(int i=-1;i<=1;i+=2){rect(x+i*(w/2+4)-1,y-12,3,5,0x2e4353);rect(x+i*(w/2+4)-1,y-12,3,2,a);}
}
static void zone_art(int n){
 const VenuePlan*r=zone_plan(n);Building*b=buildings+zone_idx[n];
 int kind=loc[n].kind,ox=r->gx*256,oy=r->gy*256,x=b->x,y=b->y,w=b->w,h=b->h,d=b->ht;
 int top=y-d,edge=y+h,front=edge-d,s=d/2;
 U a=r->light,roof=r->roof,c=r->brick,darkwall=mix(c,0x182a36,4);
 int on=max(ox+43,x-3),off=min(ox+210,x+w+s+9);
 zone_shadow(on,y,off,min(oy+216,edge+2),3,6,0x112230);
 if(!bakeFloorOnly){
 // North roof, projected east wall and the actual street-facing wall.
 quad((Pt){x+w-1,top},(Pt){x+w+s,y-3},(Pt){x+w+s,edge-2},(Pt){x+w-1,front},darkwall);
 rect(x,front,w,d,c);
 rect(x,front,w,2,0x303d47);rect(x,edge-3,w,3,mix(c,0x2b343e,4));
 for(int j=front+7;j<edge-3;j+=5){
  line(x+1,j,x+w-2,j,mix(c,0x253541,2));for(int i=(j%10)*2;i<w-1;i+=15)rect(x+i,j+1,1,3,mix(c,0x263947,2));
 }
 box(x,top,w,front-top+1,roof,mix(c,0x879c9f,4));
 rect(x+3,top+3,w-6,2,mix(roof,0x9ba5aa,3));rect(x+3,top+4,2,h-6,mix(roof,0x9ba5aa,3));
 rect(x+5,front-4,w-10,2,mix(roof,0x14232c,5));
 for(int j=top+8;j<front-7;j+=8)for(int i=x+6;i<x+w-5;i+=3){
  U q=zone_hash(i,j);if((q&7)<2)rect(i,j,2,1,mix(roof,0x72838d,2));
 }
 for(int j=top+10;j<front-1;j+=10)line(x+w+3,j+7,x+w+s-2,j+14,mix(c,0x243645,4));
 // Windows and piers bracket the physical entrance. Keep all art within the lot.
 int door=x+r->door;
 for(int i=x+8;i<x+w-11;i+=15)if(i<door-31||i>door+29)
  zone_window(i,front+4,9,min(d-9,kind==13?18:14),a,((i+n)%3)!=0);
 if(kind==11||kind==8||kind==15){
  for(int i=x+5;i<x+w-5;i+=19){if(i>door-24&&i<door+24)continue;
   rect(i,front+1,4,d-2,mix(c,0x97a4ae,5));rect(i+1,front+2,1,d-4,mix(c,PAPER,4));}
 }
 // Street-level glazing and verandas are different from the motifs overhead.
 int right0=door+30,span=x+w-right0-8;
 if(span>=23){
  switch(kind){
  case 0:case 10:
   box(right0,front+4,span,d-7,0x48595c,0x2b3f4c);
   for(int j=front+6;j<edge-4;j+=3)rect(right0+2,j,span-4,1,mix(c,0x83928d,3));
   zone_stripe(right0,edge-5,span,3,mix(c,0x394956,3),mix(a,0x394956,4));break;
  case 2:
   zone_window(right0+1,front+7,span-2,d-9,a,1);
   for(int i=0;i<span;i+=5)rect(right0+i,front+2,min(5,span-i),6,i%10?0x8b6449:0xd3ac76);
   break;
  case 3:
   zone_window(right0,front+4,span,d-10,a,1);
   for(int i=10;i<span-2;i+=11)rect(right0+i,front+6,1,d-14,0x577491);
   line(right0+4,edge-9,right0+span/2,front+7,0x759ca9);break;
  case 4:
   zone_window(right0,front+4,span,d-10,a,1);
   rect(right0+2,edge-9,span-4,5,0x45675c);
   for(int i=5;i<span-3;i+=7){disc(right0+i,edge-9,2,0x94ad72);dot(right0+i,edge-10,0xc3d093);}break;
  case 14:
   box(right0,front+5,span,d-9,0x364d60,0x253743);
   for(int i=5;i<span-1;i+=5)rect(right0+i,front+6,1,d-11,0x67818e);
   rect(right0+3,front+7,span-6,2,mix(a,c,4));break;
  case 15:
   rect(right0+1,front+7,span-2,2,0x455975);
   rect(right0+4,front+7,11,2,0xa8c7ff);rect(right0+16,front+7,11,2,0xc995b1);break;
  }
 }
 // The roof's working equipment and silhouettes identify the business before reading.
 int hw=x+w-31,ht=top+12;
 switch(kind){
 case 0: /* courier depot */
  for(int i=0;i<3;i++)zone_vent(x+12+i*26,top+15,20,30,c);
  zone_hvac(hw,ht,roof);rect(x+7,top+55,w-14,5,0x293744);rect(x+10,top+56,w-20,2,0x74948c);
  break;
 case 1: /* basement vault, double recessed clerestory */
  box(x+12,top+12,w-24,42,0x322e42,0x947e91);
  box(x+19,top+19,w-38,28,0x49394e,0x665671);
  zone_icon(x+w/2,top+32,1,a);
  for(int i=x+8;i<x+w-6;i+=9)rect(i,front-6+(i%3),4,1,mix(a,roof,2));
  break;
 case 2: /* striped market canopy */
  zone_window(x+14,top+12,w-28,29,c,1);
  for(int i=x+8;i<x+w-8;i+=7)rect(i,top+46,min(6,x+w-8-i),12,((i-(x+8))/7)%2?0xb3945d:0x7c5b47);
  zone_vent(hw,top+19,17,12,roof);break;
 case 3: /* glass display roof */
  box(x+8,top+9,w-16,54,mix(c,0x2f556e,2),0x849fb2);
  for(int i=14;i<w-8;i+=11)rect(x+i,top+11,2,50,0x405b75);
  for(int i=10;i<61;i+=11)rect(x+10,top+i,w-20,1,0x7896aa);
  line(x+10,top+61,x+84,top+11,0x8caab4);
  line(x+w-65,top+61,x+w-9,top+22,0x8caab4);
  break;
 case 4: /* louvered greenhouse, plants visible under the panes */
  box(x+9,top+8,w-18,56,0x3c6868,0x8baf9a);
  for(int i=0;i<3;i++)rect(x+11,top+12+i*15,w-22,8,mix(c,0x37614f,2));
  for(int i=13;i<w-11;i+=10)for(int j=0;j<3;j++){
   int yy=top+17+j*15;
   disc(x+i,yy,3,(i+j)%3?0x77a86c:0xa5b377);dot(x+i,yy-1,0xc2d197);
  }
  for(int i=9;i<w-8;i+=15)rect(x+i,top+10,2,51,0x819c96);
  break;
 case 5: /* stacks, condenser and copper piping */
  for(int i=0;i<3;i++){
   disc(x+24+i*28,top+43,12,0x273844);disc(x+23+i*28,top+40,10,0x6d7b89);
   disc(x+23+i*28,top+39,7,0x3a4964);rect(x+20+i*28,top+36,8,1,0x91a2ae);
  }
  rect(x+16,top+19,110,4,0x9a8184);rect(x+115,top+20,4,33,0x83727d);
  zone_hvac(x+w-31,top+46,roof);zone_stripe(x+8,front-10,31,3,0x60586c,0xa69b83);
  break;
 case 6: /* checkerboard race office + stripe */
  zone_stripe(x+8,top+9,w-16,6,0x66737b,0xb0a791);
  box(x+13,top+24,w-26,28,0x38465d,0x5a7689);
  for(int i=0;i<4;i++)zone_window(x+18+i*28,top+29,18,18,a,i%2==0);
  for(int i=14;i<w-11;i+=15)rect(x+i,top+58,8,2,0xadcdd5);
  break;
 case 7: /* radio exchange */
  zone_vent(x+12,top+11,w-24,19,c);zone_icon(x+w/2,top+44,7,a);
  for(int i=0;i<5;i++)rect(x+16+i*11,top+59,6,1,a);
  break;
 case 8: /* medical wing's cross */
  box(x+13,top+18,w-26,40,0x546975,0x889ba4);
  zone_icon(x+w/2,top+37,8,a);zone_hvac(x+18,top+26,roof);zone_hvac(hw-9,top+26,roof);
  break;
 case 9: /* pitched roof and solar tube */
  tri(x+5,top+8,x+5,front-8,x+w-5,top+(front-top)/2,0x555368);
  line(x+7,top+(front-top)/2,x+w-7,top+(front-top)/2,0x8c8184);
  for(int i=x+11;i<x+w-12;i+=8)line(i,top+12,i,front-12,0x7c7580);
  box(x+w-35,top+12,24,31,0x466577,0x828f99);line(x+w-23,top+13,x+w-23,top+41,0x91a5ae);
  break;
 case 10: /* service shop */
  for(int i=12;i<w-10;i+=7)line(x+i,top+9,x+i,front-6,0x727c7a);
  for(int i=0;i<3;i++)zone_vent(x+13+i*45,top+21,30,27,roof);
  zone_stripe(x+12,top+59,w-25,3,0x887550,0xc5a677);
  break;
 case 11: /* revenue civic pediment and shallow glass lobby */
  box(x+12,top+13,w-24,39,0x7b8490,0x9d9b9c);
  zone_icon(x+w/2,top+31,11,a);
  for(int i=x+16;i<x+w-15;i+=9)rect(i,top+45,3,9,0xb2ada2);
  break;
 case 13: /* apartments: steel stairs and rooftop dormers */
  for(int i=0;i<3;i++){int u=x+12+35*i;box(u,top+10,25,26,0x66566c,0xb08d8d);zone_window(u+5,top+13,15,14,a,i!=1);}
  // Rooftop chimneys.
  rect(x+10,top+41,8,14,0x866d72);rect(x+8,top+41,12,2,0x9e8787);
  rect(x+w-16,top+39,8,16,0x866d72);rect(x+w-18,top+39,12,2,0x9e8787);
  break;
 case 14: /* reinforced armory vault door and equipment cages */
  box(x+13,top+12,w-26,45,0x3c5465,0x738b99);zone_icon(x+w/2,top+32,14,a);
  for(int i=x+20;i<x+42;i+=6)rect(i,top+18,2,26,0x728593);
  for(int i=x+w-40;i<x+w-18;i+=6)rect(i,top+18,2,26,0x728593);
  break;
 case 15: /* precinct badge and radio tower */
  box(x+11,top+12,w-22,43,0x5a7293,0x8f9fb5);zone_icon(x+w/2,top+33,15,a);
  for(int i=0;i<2;i++){int u=i?x+w-34:x+20;
   line(u,top+47,u,top+14,0x9bafc3);line(u-6,top+25,u+6,top+25,a);
   line(u-6,top+19,u+6,top+19,a);rect(u-2,top+12,4,3,0xd7829a);}
  break;
 }
 // Illuminated business signature. All boards are inset into the roof fascia.
 int bw=min(w-12,max(63,sl(r->name)*8+10)),yy=front-30;
 box(x+w/2-bw/2,yy,bw,24,mix(roof,0x172631,5),mix(c,a,4));
 zone_lettering(x+w/2,yy+3,r->name,a,2);
 zone_lettering(x+w/2,yy+16,r->sign,mix(PAPER,a,4),1);
 }
 zone_entrance(r,b,kind);
 if(kind==3||kind==10){ // Painted service lanes on private forecourt only.
  for(int i=x+w-22;i<x+w-5;i+=7){rect(i,edge+4,3,9,0x8e8b69);}
 }
 if(!bakeFloorOnly&&(kind==1||kind==7||kind==13)){ // Planted pots safely behind the sidewalk.
  int xx=x+w-13;disc(xx+3,edge+5,5,0x24343b);disc(xx,edge+4,5,0x516861);disc(xx-1,edge+2,3,0x769271);
 }
}
