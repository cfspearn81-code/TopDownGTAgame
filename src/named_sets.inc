/* Named venues are little stage sets. Draw only into the room's cached backdrop;
   collision/interaction comes from the furniture plan in interiors.inc.
   'Plate' coordinates describe finishes laid flush with a floor (not new walls).
   Furniture art uses its actual solid footprint. No city RNG, assets or per-frame load. */
enum {NS_WOOD,NS_CARPET,NS_QUARRY,NS_MARBLE,NS_BRICK,NS_STEEL,NS_TARMAC,NS_LINO,NS_STONE,NS_GRATE};
typedef struct {U base,seam,trim;int surface;} NsFinish;
static const NsFinish ns_finish[]={
 {0x4e6a69,0x374d50,0xbba980,NS_QUARRY},{0x49304b,0x39213b,0xb08aa2,NS_CARPET},
 {0xa17d57,0x735d43,0xe4bd8f,NS_STONE},{0x496075,0x34445e,0xafc9d5,NS_MARBLE},
 {0xaa7151,0x765849,0xbbae72,NS_BRICK},{0x465365,0x283649,0xaeb6c2,NS_STEEL},
 {0x435158,0x344044,0xd5ae60,NS_TARMAC},{0x6d504f,0x463c41,0xc48e7c,NS_BRICK},
 {0x88a6a4,0x708f91,0xc8d5c4,NS_LINO},{0x806749,0x5b5041,0xbf9b67,NS_WOOD},
 {0x5d605d,0x3c494b,0xb99f71,NS_TARMAC},{0xa19880,0x756f66,0xd8c18a,NS_MARBLE},
 {0,0,0,0},{0xad7954,0x76563f,0xd3ae79,NS_WOOD},
 {0x5f665c,0x4a534c,0xafa58d,NS_QUARRY},{0x778894,0x596b76,0xbabcb4,NS_QUARRY},
 {0x617771,0x425650,0xa8bc90,NS_STEEL},{0x6f8b8b,0x4e6b71,0xacccc2,NS_QUARRY},
 {0x75685b,0x4b4a45,0xc69e65,NS_TARMAC},{0x4f6770,0x34484d,0x99b6bb,NS_GRATE}
};
static U ns_floor_pixel(int x,int y,const NsFinish*m){
 U h=int_hash(x,y),a=m->base,b=m->seam;int shade;
 switch(m->surface){
 case NS_WOOD:{int row=y/14,u=(x+19*(row%5))%100;shade=(row*3+x/100)%4;if(y%14<2||u==0)return b;
   if((h&31)==0||((x+row*13)%61<16&&(y+u/16)%7==0))return mix(a,b,3);break;}
 case NS_MARBLE:{int u=(x+((y/37)&1)*24)%59,v=y%37; if(!u||!v)return b;
   shade=((x/59+y/37)&1);if((x+y*2u+(h&3))%139<2)return mix(a,m->trim,3);break;}
 case NS_BRICK:{int v=y%11,u=(x+((y/11)&1)*14)%28;if(v==0||u==0)return b;
   shade=int_hash((x+((y/11)&1)*14)/28,y/11)%4;break;}
 case NS_LINO:{if(x%37==0||y%28==0)return mix(a,b,3);shade=(x/37+y/28)%2;break;}
 case NS_STEEL:{int u=x%58,v=y%35;if(!u||!v)return b;if((u==3||u==55)&&(v==3||v==32))return m->trim;shade=(x/58+y/35)%3;break;}
 case NS_QUARRY:{int u=x%36,v=y%24;if(!u||!v)return b;shade=(x/36+y/24)&1;break;}
 case NS_CARPET:shade=(x/17+y/17)&1; if((x+y)%19==0&&(x/19+y/19)%3==0)return mix(a,m->trim,1);break;
 case NS_STONE:{int row=y/21;int u=(x+row*22)%46;if(y%21<2||u<2)return b;shade=int_hash((x+row*22)/46,row)%4;break;}
 case NS_GRATE:shade=(x%10<2&&y%6<4)?5:1;break;
 default:shade=(h>>29)&3;if((h&511)==0)return mix(a,m->trim,4);break;
 }
 return mix(a,b,cl(shade+((h&7)==0),0,6));
}
static void ns_oval(int cx,int cy,int rx,int ry,U c){
 if(rx<=0||ry<=0)return;for(int j=-ry;j<=ry;j++){int n=(int)(rx*sq(1.f-(float)(j*j)/(float)(ry*ry)));rect(cx-n,cy+j,2*n+1,1,c);}
}
static void ns_ring(int x,int y,int a,int b,int w,U c){
 for(int j=-b;j<=b;j++){float z=1.f-(float)(j*j)/(b*b);
  int outer=(int)(a*sq(z)),inner=(j>-(b-w)&&j<b-w)?(int)((a-w)*sq(1.f-(float)(j*j)/((b-w)*(b-w)))):-1;
  rect(x-outer,y+j,outer-inner,1,c);rect(x+inner+1,y+j,outer-inner,1,c);}
}
static void ns_diamond(int x,int y,int w,int h,U c){
 tri(x-w,y,x,y-h,x+w,y,c);tri(x-w,y,x,y+h,x+w,y,c);
}
static void ns_stripe(int x,int y,int w,int h,U c,U edge){
 rect(x,y,w,h,c);for(int i=0;i<w-h;i+=h*2)for(int j=0;j<h;j++)rect(x+i+j,y+j,h,1,edge);
}
static void ns_rug(int x,int y,int w,int h,U dark,U mid,U light){
 for(int i=3;i<w-3;i+=4){rect(x+i,y-3,1,4,light);rect(x+i,y+h-1,2,4,light);}
 rect(x,y,w,h,dark);box(x+3,y+3,w-6,h-6,mid,light);box(x+9,y+8,w-18,h-16,mid,dark);
 int cy=y+h/2,cx=x+w/2;ns_diamond(cx,cy,w/4,h/2-10,dark);ns_diamond(cx,cy,w/5,h/2-14,light);
 if(h>43)ns_diamond(cx,cy,w/6,h/2-18,mid);
 for(int i=15;i<w-15;i+=15){ns_diamond(x+i,y+5,3,2,dark);ns_diamond(x+i,y+h-6,3,2,dark);}
 for(int i=0;i<2;i++)for(int j=0;j<2;j++)ns_diamond(x+(i?w-22:22),y+(j?h-20:21),7,5,dark);
}
static void ns_bricks(int x,int y,int w,int h,U mortar,U a,U b){
 rect(x,y,w,h,mortar);for(int j=0;j<h;j+=8)for(int i=-((j/8)%2)*10;i<w;i+=20){
  int l=max(i,0),r=min(i+19,w);if(r>l)rect(x+l,y+j,r-l,min(7,h-j),(int_hash(i+29,j)&3)?a:b);}
}
static void ns_awning(int l,int r,int y,U c,U d){
 for(int i=l,n=0;i<r;i+=16,n++){int w=min(16,r-i);
  rect(i,y,w,10,n&1?c:d);rect(i,y+10,w,4,mix(n&1?c:d,DARK,2));
  ns_oval(i+w/2,y+13,w/2,4,mix(n&1?c:d,DARK,2));}
}
static void ns_clock(int x,int y,int r,int t,U c){
 disc(x+1,y+2,r+1,0x242e31);disc(x,y,r,c);disc(x,y,r-2,0xeadfbd);for(int i=0;i<12;i++){
 float a=i*.523599f;dot(x+cs(a)*(r-3),y+sn(a)*(r-3),0x5c5951);}
 line(x,y,x+(int)(cs(t*.52f)*(r-4)),y+(int)(sn(t*.52f)*(r-4)),0x343c41);line(x,y,x,y-r+3,0x483e42);
}
/* Little factory window sets, grille shutters, padded curtains and corrugated doors
   are actual wall construction, not the old strip of identical skyline decals. */
static void ns_window(int x,int y,int w,int h,U sky,U frame,int bays){
 box(x,y,w,h,sky,frame);
 for(int i=1;i<h-1;i++){U c=mix(sky,0xe3cfa7,i*4/h);rect(x+1,y+i,w-2,1,c);}
 for(int i=x+6;i<x+w-15;i+=23){int z=4+int_hash(i,y)%8;rect(i,y+h-z-2,16,z, mix(sky,frame,4));}
 for(int i=1;i<bays;i++){rect(x+i*w/bays-1,y,3,h,frame);}
 rect(x,y+h*2/3,w,2,frame);rect(x-2,y+h,w+4,3,mix(frame,PAPER,2));
}
static void ns_shutter(int x,int y,int w,int h,U c){
 box(x,y,w,h,c,0x253742);for(int j=3;j<h-1;j+=4){rect(x+2,y+j,w-4,2,mix(c,DARK,3));rect(x+2,y+j+2,w-4,1,mix(c,PAPER,2));}
}
static void ns_pipe(int x,int y,int len,int vertical,int d,U c){
 int w=vertical?d:len,h=vertical?len:d;rect(x+2,y+2,w,h,0x323e43);rect(x,y,w,h,c);
 rect(x+1,y+1,vertical?1:w-2,vertical?h-2:1,mix(c,PAPER,4));
 for(int i=7;i<len-2;i+=26)rect(x+(vertical?-1:i),y+(vertical?i:-1),vertical?w+2:3,vertical?3:h+2,mix(c,DARK,2));
}
static void ns_wheel(int x,int y,int r,U hub){disc(x+2,y+3,r,0x242e32);disc(x,y,r,0x232c31);ns_ring(x,y,r-1,r-1,2,0x485053);
 disc(x,y,r*5/9,hub);disc(x,y,max(2,r/4),0x27353b);for(int i=0;i<5;i++){float a=i*1.25664f;dot(x+cs(a)*r*.39f,y+sn(a)*r*.39f,0xd9c5a2);}}
static void ns_pallet(int x,int y,int w,int h){rect(x+3,y+3,w,h,0x3b3e34);for(int i=0;i<h;i+=7){
 rect(x,y+i,w,min(6,h-i),i&1?0x97744f:0xae865b);dot(x+4,y+i+2,0x514a40);dot(x+w-5,y+i+2,0x514a40);}}
static void ns_parcel(int x,int y,int w,int h,U c){rect(x+2,y+3,w,h,0x48463b);
 box(x,y,w,h,c,mix(c,DARK,4));rect(x+w/2-2,y,4,h,0xd5b888);rect(x+3,y+3,min(10,w-6),5,0xe4d6b2);rect(x+5,y+4,min(w-9,6),1,0x8b8273);}
static void ns_wall(int id){
 int l=venue_limit(145,0)+9,r=venue_limit(145,1)-8,w=r-l;U a=ns_finish[id].trim;
 switch(id){
 case 0: /* parcel pigeonholes frame a tall central dispatch entrance */
 case 16:
  rect(l,101,w,40,0x263d40);
  for(int i=l+4,n=0;i<r-15;i+=20,n++){if(id==0&&i>410&&i<535)continue;for(int j=0;j<2;j++){
   int v=105+j*16;box(i,v,19,15,0x4b5145,0x9f8566);
   if((n+j)%4){rect(i+3,v+6,13,6,0xd3b994);line(i+3,v+7,i+9,v+10,0x9a8a76);line(i+9,v+10,i+15,v+7,0x9a8a76);}}}
  if(id==16){ns_shutter(383,101,196,36,0x84928d);ns_stripe(379,136,203,7,0x374b4c,0xc4aa6e);}break;
 case 1: /* scalloped drapes and brass fringe over the DJ stage */
  rect(l,101,w,40,0x251d33);
  for(int i=l;i<r;i+=44){for(int j=0;j<min(44,r-i);j++){int height=11+(int)(12*sn(j*.0714f));rect(i+j,101,1,height,0x93415e);dot(i+j,101+height,0xd7a772);}
   rect(i+2,104,4,30,0x79314f);line(i+2,135,i+5,129,0xd2a16f);}
  for(int i=l+55;i<r-18;i+=111){disc(i,125,8,0x332b48);disc(i,125,5,0xeac8c0);}break;
 case 2:
  ns_bricks(l,102,w,39,0x71817b,0xa9b1a0,0x8d9d92);ns_awning(l+1,411,107,0x317d88,0xe0ceaa);ns_awning(549,r-1,107,0x317d88,0xe0ceaa);break;
 case 3:
  ns_window(l+2,101,w-4,35,0x71a2b5,0x304860,7);for(int i=l+18;i<r-50;i+=82){line(i,131,i+40,104,0xc6d9d4);line(i+4,131,i+44,104,0xa4c9d3);}break;
 case 4:
  ns_window(l+1,103,w-2,34,0x95bca8,0x476856,10);
  for(int i=l+18,n=0;i<r-8;i+=38,n++){int s=(n*7)%10;line(i,101,i,111+s,0x505e4a);ns_oval(i,118+s,8,6,0x995f48);int_fern(i,115+s,11,0x628d58,n+17,0);}break;
 case 5:case 19:
  ns_shutter(l,101,w,39,id==5?0x526174:0x74818a);
  ns_pipe(l+2,105,w-4,0,6,id==5?0xc19a61:0x89b1a3);
  for(int i=l+21;i<r-9;i+=68){disc(i,125,10,0x263544);disc(i,125,8,0x60767e);for(int j=0;j<3;j++){float a0=j*2.0944f;tri(i,125,i+cs(a0)*7,125+sn(a0)*7,i+cs(a0+.9f)*6,125+sn(a0+.9f)*6,0x253744);}disc(i,125,2,0xb6bbb2);}break;
 case 6:
  rect(l,101,w,39,0x253947);
  for(int i=l+2;i<r-2;i+=11)for(int j=0;j<2;j++)rect(i,103+j*8,min(11,r-i),8,((i-l)/11+j)%2?0xe1d9bd:0x293c42);
  for(int i=l+9;i<r-40;i+=53){rect(i,125,38,7,0x101f2a);for(int j=0;j<5;j++)rect(i+2+j*7,127,4,3,j==4?0x65705a:0xe9b15e);}break;
 case 7:
  ns_bricks(l,101,w,39,0x493d46,0x805357,0x955d59);ns_shutter(l+19,111,85,26,0x83747a);ns_awning(r-125,r-9,101,0xd58c85,0x584056);
  for(int i=l+108;i<r-128;i+=18){line(i,104,i+9,119,0x31383e);line(i+9,119,i+18,104,0x31383e);}break;
 case 8:
  rect(l,101,w,39,0xb5c6bc);rect(l,127,w,11,0x6e9c98);ns_window(l+11,103,177,24,0xa9c7c0,0x577f82,5);
  for(int i=l+225;i<r-75;i+=70){box(i,105,25,24,0xe9e8d0,0x759294);rect(i+9,110,6,13,0x488c84);rect(i+5,114,14,5,0x488c84);}
  ns_clock(r-32,119,12,6,0x56777a);break;
 case 9:case 13:{
  U wallpaper=id==13?0xb49a78:0x657a67;rect(l,101,w,39,wallpaper);
  for(int x=l+10;x<r-3;x+=14){for(int y=105;y<138;y+=11)ns_diamond(x+(y%2?2:0),y,2,3,mix(wallpaper,0x694c4c,2));}
  int x=l+38;ns_window(x,103,106,30,id==13?0x71969d:0x476571,0x594d40,3);
  for(int j=0;j<2;j++){int u=x-7+j*103;rect(u,101,11,32,id==13?0x9b5b54:0x526456);rect(u+4,102,2,28,id==13?0xc7816d:0x788775);}
  for(int j=0;j<3;j++){int u=r-118+j*33;box(u,105,26,26,0xe6cba0,0x7a6144);box(u+3,108,20,20,j&1?0xab825f:0x77938d,0x514c46);
   tri(u+4,126,u+13,114,u+21,126,0x576c69);disc(u+17,113,2,0xe9ce8c);}
  break;}
 case 10:case 18:
  ns_shutter(l,102,w,37,0x716d61);for(int i=l+10;i<r-11;i+=40){line(i,103,i+24,134,0x514e44);line(i+1,103,i+25,134,0x9d8358);}
  ns_pipe(l,108,w,0,5,0xcc9f5a);break;
 case 11:
  rect(l,101,w,39,0x9e9283);for(int i=l+8;i<r-15;i+=43){rect(i,106,25,32,0x615c58);rect(i+3,108,19,29,0x716c64);rect(i-3,103,31,5,0xcbbfa0);}
  ns_clock((l+r)/2,119,15,8,0xc6a269);break;
 case 14:
  rect(l,102,w,37,0x3c463f);for(int i=l+6;i<r-18;i+=19){quad((Pt){i,105},(Pt){i+13,105},(Pt){i+5,133},(Pt){i-5,133},0x5f6860);}
  ns_stripe(l+1,134,w-2,5,0x38443f,0xcba164);break;
 case 15:
  ns_bricks(l,101,w,38,0x7d8e98,0xacb8b8,0xbfc7be);
  box(l+17,103,131,30,0x393f43,0x886e54);for(int i=0;i<6;i++){int u=l+22+i*20;box(u,108,16,18,0xc9b89a,0x71635a);disc(u+8,114,3,0x76685e);ns_oval(u+8,120,5,3,0x817465);}
  ns_window(366,104,233,24,0x77949b,0x4a6374,7);ns_shutter(r-153,105,145,28,0x617181);break;
 case 17:
  ns_window(l+6,105,172,27,0xa8c8c3,0x496873,6);ns_shutter(376,102,208,36,0x96ab9e);ns_pipe(r-170,113,163,0,9,0x7eabaa);break;
 }
 /* Narrow architectural plinth, deliberately interrupted by actual entrances drawn later. */
 rect(l,139,w,4,mix(a,0x192b32,5));rect(l,139,w,1,mix(a,PAPER,2));
}
/* Inlaid marks are part of the floor: route lanes in dispatch, a recessed stage,
   a wet hall by the ferry, clinical flooring, civic terrazzo and domestic textiles.
   They scale to the cutaway by using the same shell mask and door gap as collision. */
static void ns_ground(int id){
 switch(id){
 case 0:{
  /* A painted branching postal routing diagram leads from the front through the hall. */
  const int X[]={348,407,407,449,449,508,508,555,606},Y[]={302,302,338,338,399,399,354,354,306};
  for(int i=1;i<9;i++){for(int s=0;s<4;s++)line(X[i-1]+s,Y[i-1]+s,X[i]+s,Y[i]+s,0xbda777);}
  for(int i=1;i<8;i+=2){disc(X[i],Y[i],7,0x4a6769);ns_ring(X[i],Y[i],6,6,2,0xcfb887);}
  ns_stripe(175,214,167,4,0x596964,0xb6a87f);ns_stripe(594,310,135,4,0x596964,0xb6a87f);break;}
 case 1:{
  /* An octagonal luminous floor, with brass steps and scalloped inner edge. */
  int cx=417,cy=289;ns_oval(cx,cy+4,122,72,0x291d34);ns_oval(cx,cy,121,70,0xb2828a);
  ns_oval(cx,cy,118,67,0x5c3856);ns_oval(cx,cy,110,60,0xdab58b);ns_oval(cx,cy,107,56,0x383047);
  for(int j=-3;j<=3;j++)for(int i=-7;i<=7;i++)if(i*i*9+j*j*42<458){
   U hues[]={0xa46089,0x775c91,0x796b8e,0xb67d70,0x526e81};int x=cx+i*14+((j+3)%2)*6,y=cy+j*14;
   ns_diamond(x,y,11,6,hues[(i*i+j*j+i+8)%5]);}
  for(int i=0;i<3;i++){int y=353+i*5,w=79-i*8;rect(cx-w/2,y,w,3,0xb39587);}break;}
 case 2:
  rect(159,146,186,216,0x87998e);for(int j=155;j<357;j+=16){line(162,j,342,j,0x6e8177);for(int x=170+(j%3)*9;x<331;x+=31)rect(x,j,1,15,0x6e8177);}
  /* Working fish hall: long grated drain, scales of wet reflected light. */
  rect(347,150,10,209,0x4b605c);for(int y=153;y<357;y+=5)rect(349,y,5,2,0x9fac97);
  for(int i=0;i<52;i++){U h=int_hash(i,64);rect(170+h%171,155+(h>>12)%194,3+(h>>7)%9,1,0xb2b6a1);}
  ns_awning(577,755,145,0x378c95,0xdbc191);break;
 case 3:
  for(int j=0;j<2;j++){
   int cx=j?594:276,cy=j?348:274;ns_oval(cx+3,cy+6,102,57,0x334451);
   ns_oval(cx,cy,103,55,0x9eacae);ns_oval(cx,cy-3,100,53,0xccd4c6);
   ns_oval(cx,cy-4,96,49,0x6a8894);ns_ring(cx,cy-4,91,45,2,0xafd1cb);
   for(int i=0;i<8;i++){float a=i*.785398f;ns_oval(cx+cs(a)*98,cy-3+sn(a)*49,3,2,0xf4dfac);}
  }
  for(int i=0;i<3;i++){int x=394+i*9;line(x,147,x+78,215,0x81959b);line(x+1,147,x+79,215,0x81959b);}break;
 case 4:
  for(int j=0;j<13;j++)for(int i=0;i<3;i++){int x=450+i*29+((j&1)?6:0),y=156+j*21;
   ns_oval(x,y,14,8,0x705942);ns_oval(x,y-1,13,7,(j+i)%3?0xc1b99a:0xa0a98b);}
  /* Iron tracery seen in projection across the terracotta. */
  for(int i=0;i<6;i++){int x=177+i*100;for(int j=0;j<3;j++)line(x+j,147,x+29+j,203,0x796c55);}break;
 case 5:
  /* Two high-pressure hose feeds terminate at the round vault surround. */
  for(int j=0;j<2;j++){int x=545+j*12;ns_pipe(x,157,158,1,6,j?0xb49467:0x648b9c);ns_pipe(x,309,91,0,6,j?0xb49467:0x648b9c);}
  ns_ring(684,203,86,65,6,0x78848f);ns_ring(684,203,77,57,2,0xbb9da3);
  for(int i=0;i<8;i++){float a=i*.785398f;disc(684+cs(a)*81,203+sn(a)*60,3,0xc2b5a0);}
  ns_stripe(336,289,146,6,0x4e5961,0xb9986a);break;
 case 6:{
  /* Curving pit entry: the tire wall follows the bend; rumble strips at its apex. */
  for(int i=0;i<8;i++){int x=162+i*22;for(int j=0;j<3;j++)line(x,300+j,x+22,309+j,0xe1d5ac);}
  for(int j=0;j<4;j++)for(int i=0;i<13;i++)rect(380+i*14,352+j*8,14,8,((i+j)&1)?0xdeccaa:0x3b4249);
  for(int i=0;i<11;i++){int x=555+i*17,y=307-(i>4?(i-4)*2:0);line(x,y,x+11,y,0xe1c47e);line(x,y+1,x+11,y+1,0xe1c47e);}break;}
 case 7:
  ns_rug(395,345,144,68,0x59464a,0x724a59,0xb28b71);
  rect(346,276,267,7,0x493a41);for(int i=350;i<610;i+=13)rect(i,278,8,3,0x9c8878);
  /* Old glass block vestibule, edge light and a shoe-worn threshold. */
  for(int j=0;j<5;j++){int x=443+j*15;box(x,401,14,12,0x67827e,0x526767);}break;
 case 8:
  /* A continuous color-coded path to triage, distinct from the ambulance bay. */
  for(int j=0;j<2;j++){int x=490+j*7,y=260+j*7;rect(x,260,4,148,j?0xbecdb5:0x568d86);rect(336,y,x-332,4,j?0xbecdb5:0x568d86);}
  ns_ring(569,368,28,26,2,0x639a92);rect(566,350,7,36,0xd9ddc5);rect(551,365,36,7,0xd9ddc5);
  for(int j=0;j<2;j++){int x=167,y=147+j*122;rect(x,y,154,88,0x829d99);for(int i=0;i<7;i++)rect(x+3+i*22,y+2,1,85,0x758f8d);}break;
 case 9:
  ns_rug(296,292,192,89,0x664a40,0x8b684e,0xc1a677);
  /* Galley's checkered linoleum against a distinctly rough sleeping area. */
  for(int j=0;j<5;j++)for(int i=0;i<9;i++)rect(498+i*19,145+j*12,18,11,(i+j)%2?0x8c927b:0xb3a180);
  break;
 case 10:case 18:{
  int x=174,y=id==10?219:236,w=196,h=id==10?98:99;
  rect(x,y,w,h,0x414a49);box(x+4,y+4,w-8,h-8,0x343c3d,0xafa37d);
  for(int j=0;j<2;j++){ns_stripe(x,y+j*(h-7),w,7,0xccad72,0x3b4340);ns_pipe(x+7,y+17+j*(h-44),w-14,0,5,0x878b78);}
  for(int i=0;i<12;i++){int px=381+i*4,py=301+i*6;line(px,py,px+9,py-3,0x4c5350);line(px+16,py,px+25,py-3,0x4c5350);}
  break;}
 case 11:
  /* Marble compass in a rotunda, the municipal seal embedded in the floor beneath the teller. */
  ns_oval(481,314,76,54,0x827968);ns_ring(481,314,72,51,2,0xcabc96);ns_ring(481,314,63,45,2,0xbfb393);
  for(int i=0;i<8;i++){float z=i*.785398f;int u=481+cs(z)*53,v=314+sn(z)*37;
   tri(481,314,u,v,481+cs(z+.43f)*14,314+sn(z+.43f)*10,i&1?0x576d70:0xbeb295);}
  ns_ring(481,314,10,7,2,0xcab486);break;
 case 13:
  ns_rug(253,290,165,83,0x653f4d,0x9c6058,0xd4b27d);
  for(int i=0;i<3;i++)for(int j=0;j<3;j++)rect(543+i*41,146+j*23,40,22,(i+j)%2?0x829387:0xbdb899);
  ns_rug(439,371,84,42,0x486366,0x68847d,0xbcb58b);break;
 case 14:
  rect(169,147,177,95,0x485353);
  for(int i=0;i<3;i++){int x=178+i*55;rect(x,148,2,85,0xa09675);for(int j=0;j<6;j++)rect(x+11+j*6,227,4,2,0xa09675);}
  ns_stripe(169,232,175,6,0x46504d,0xceb77e);
  rect(524,198,176,72,0x505c56);for(int y=201;y<269;y+=8)for(int x=527;x<696;x+=12)line(x,y+3,x+4,y,0x687268);break;
 case 15:
  /* Restricted wing, investigation bullpen and lobby use three real floor surfaces. */
  for(int j=0;j<7;j++)for(int i=0;i<8;i++){int x=637+i*19,y=147+j*15;rect(x,y,18,14,(i+j)%2?0x8e9290:0x717b7a);}
  ns_rug(174,149,142,112,0x34495d,0x526273,0xb29e78);
  ns_ring(494,379,44,34,3,0xa2a18c);ns_ring(494,379,37,27,2,0x9a9c90);
  ns_diamond(494,379,22,21,0x8b968e);ns_diamond(494,379,9,9,0xbcb28f);break;
 case 16:
  /* Postal sorting conveyors, transfer spur and worn central loading walkway. */
  for(int j=0;j<2;j++){int v=j?352:258;rect(171,v,196,13,0x314c50);for(int x=174;x<364;x+=8)rect(x,v+2,3,9,0x8ba09a);}
  for(int i=0;i<6;i++){int y=288+i*20;line(414,y,428,y+5,0xb3ba96);line(428,y+5,441,y,0xb3ba96);}break;
 case 17:
  rect(161,146,215,96,0x95b6b7);for(int i=0;i<15;i++)rect(165+i*14,147,1,92,0x79999c);
  rect(167,244,606,6,0x577175);for(int x=170;x<771;x+=7)rect(x,245,3,4,0x9cb2a6);
  ns_stripe(612,373,130,4,0x6d8076,0xcdc494);break;
 case 19:
  /* One closed circuit around the beds is a graphic anchor even when no batch is planted. */
  for(int i=0;i<2;i++){int x=i?551:393;ns_pipe(x,156,228,1,6,0x79a097);ns_pipe(i?551:259,378,142,0,6,0x79a097);}
  for(int j=0;j<2;j++)for(int i=0;i<2;i++){int x=174+i*400,y=175+j*120;box(x,y,212,81,0x41575e,0x94a9ab);rect(x+4,y+4,204,2,0xdfc8f2);}
  break;
 }
}
static void ns_cushion(int x,int y,int w,int h,U c){
 rect(x+4,y,w-8,h,c);rect(x,y+4,w,h-8,c);rect(x+2,y+2,w-4,h-4,c);rect(x+5,y+2,w-10,2,mix(c,PAPER,3));
}
static void ns_sofaread(int x,int y,int w,int h,U cloth,int seats){
 ns_cushion(x+3,y+4,w,h,0x353936);ns_cushion(x,y,w,h, mix(cloth,DARK,3));ns_cushion(x+3,y+1,w-6,10,cloth);
 for(int i=0;i<seats;i++){int sw=(w-18)/seats;ns_cushion(x+9+i*sw,y+11,sw-2,h-15,cloth);}
 for(int side=0;side<2;side++)ns_cushion(x+side*(w-8),y+9,8,h-13,mix(cloth,PAPER,1));
}
static void ns_radio(int x,int y,int w,U c){
 box(x,y,w,18,0x354447,c);disc(x+8,y+9,5,0x88938c);disc(x+8,y+9,3,0x3f5052);rect(x+16,y+4,w-20,5,0xb4b491);
 for(int j=0;j<3;j++)rect(x+16,y+11+j*2,w-21,1,0x728681);line(x+w-7,y,x+w+4,y-10,0xaab6ac);
}
static void ns_slip(int x,int y,U paper){rect(x+1,y+2,11,15,0x605e52);rect(x,y,11,14,paper);for(int j=0;j<3;j++)rect(x+2,y+4+j*3,7-j,1,0x89918c);}
static void ns_speaker(int x,int y,int w,int h){
 ns_cushion(x+2,y+3,w,h,0x211f2b);box(x,y,w,h,0x322c3d,0x695364);int r=min(w/2-3,h/3-2);
 disc(x+w/2,y+h*2/3,r,0xa28a93);disc(x+w/2,y+h*2/3,r-2,0x484351);disc(x+w/2,y+h*2/3,r/3,0x726477);
 disc(x+w/2,y+h/4,r/2,0x262833);disc(x+w/2,y+h/4,max(2,r/4),0xbf9b7d);
}
static void ns_tank(int x,int y,int rx,int ry,U c){
 ns_oval(x+4,y+5,rx,ry,0x303c47);ns_oval(x,y,rx,ry,0x384455);ns_oval(x,y-2,rx-2,ry-2,c);
 ns_ring(x,y-2,rx-4,ry-3,2,mix(c,PAPER,3));ns_oval(x-2,y-4,rx/2,ry/2,mix(c,0x39435c,3));
 ns_oval(x-3,y-4,rx/3,ry/3,mix(c,0xab98ba,4));for(int i=0;i<8;i++){float a=i*.785398f;
 disc(x+cs(a)*(rx-5),y-2+sn(a)*(ry-5),1,0xf2dab6);}
}
static void ns_luggage(int x,int y,int w,int h,U c){
 ns_cushion(x+2,y+3,w,h,0x42413d);ns_cushion(x,y,w,h,c);box(x+w/2-5,y-3,11,4,0x5d5346,0x897c68);
 rect(x+5,y+2,3,h-4,0xa38a61);rect(x+w-9,y+2,3,h-4,0xa38a61);
}
/* Unlike a stack of rectangular palettes, furnishings have unoccupied background
   around their silhouette. Paint one themed object/group over each solid prop box. */
static int ns_furniture(const InteriorProp*f,int k){
 const int x=f->x,y=f->y,w=f->w,h=f->h,cx=x+w/2,cy=y+h/2,t=f->type;U c=f->tint;
 if(!interior_subroom&&k==0&&t==INT_CRATES){
  ns_pallet(x+2,y+h/3,w-6,h*2/3);for(int i=0;i<3;i++){int u=x+5+i*(w-8)/3;
  ns_parcel(u,y+h/3+3-(i&1)*5,w/3-6,h/2, i&1?0xbb8e65:0xc9ac7e);}
  ns_parcel(x+15,y+1,w/3-3,h/2,0xc8a671);return 1;}
 if(!interior_subroom&&k==0&&t==INT_DESK){
  ns_cushion(x+3,y+6,w,h,0x284148);ns_cushion(x,y,w,h,0x7e9b8f);rect(x+4,y+h-8,w-8,6,0x3d655d);
  ns_radio(x+9,y+9,39,0xb9b096);ns_slip(x+w/2-9,y+9,0xe4d4ae);ns_slip(x+w/2+4,y+12,0xbac8b2);
  for(int i=0;i<3;i++)ns_parcel(x+w-51+(i%2)*18,y+4+i/2*11,16,12,0xc79a67);return 1;}
 if(k==1){
  if(t==INT_SOFA){ns_sofaread(x,y,w,h,0x9c5367,3);for(int i=12;i<w-10;i+=13)dot(x+i,y+6,0xe0aa8c);return 1;}
  if(t==INT_DINER){
   ns_cushion(x+3,y+4,w,h,0x382c35);ns_cushion(x,y,w,h,0xa5846a);ns_cushion(x+5,y+2,w-10,h-9,0x573b4d);
   for(int i=0;i<7;i++){int u=x+12+i*12;disc(u,y+15,4,i%2?0xbca37d:0x7d9488);rect(u-1,y+11,2,5,0xe8d7b1);}
   ns_oval(x+w-32,cy,15,7,0xc29d7a);ns_oval(x+w-32,cy,13,5,0x725e58);return 1;}
  if(t==INT_REEL){ns_speaker(x+3,y,w-7,h);return 1;}
 }
 if(k==2&&!interior_subroom&&(t==INT_TRAY||t==INT_RACK||t==INT_CRATES)){
  int cnt=w<90?2:4;for(int j=0;j<cnt;j++){int bw=w/cnt-3,u=x+j*(w/cnt)+1;
   ns_pallet(u,y+2,bw,h-4);box(u+2,y+3,bw-4,h-6,0x695a40,0xb69a6f);
   for(int a=0;a<3;a++)for(int b=0;b<2;b++){int q=u+5+a*(bw-10)/3+2,r=y+9+b*(h-16)/2;
    U colors[]={0xd3a053,0x809359,0xb65d4c,0x948e64};U z=colors[(j+(t==INT_RACK?1:0))%4];
    ns_oval(q,r,4,4,mix(z,DARK,2));ns_oval(q-1,r-1,3,3,z);dot(q-1,r-3,0xb8b97b);}}
  return 1;}
 if(k==2&&!interior_subroom&&t==INT_DESK){
  int_solid_shape(x,y,w,h,0x8d755d);rect(x+4,y+4,w-8,h-14,0xc5b697);
  /* Round pan and offset dial of the grocer's scale. */
  ns_oval(x+30,cy+5,21,8,0x5f706c);ns_oval(x+30,cy+3,20,7,0xa6b2a0);disc(x+32,y+9,9,0x526e6c);
  disc(x+32,y+9,7,0xd8cfaa);line(x+32,y+9,x+36,y+5,0xa6684d);
  for(int i=0;i<3;i++)ns_parcel(x+60+i*15,y+10+(i%2)*3,12,13,0xb6996d);
  int_monitor(x+w-38,y+6,24,0x9fac94);return 1;}
 if(!interior_subroom&&(k==3||k==6)&&t==INT_CAR){
  /* Drive-on plinths are drawn on the floor. Sculpt tire, body and cabin with
     wheel arches instead of the previous desk-shaped showcase bezel. */
  int l=x+6,r=x+w-7,top=y+9,b=y+h-10;int cw=r-l,ch=b-top;
  ns_oval(cx+3,cy+6,cw/2+4,ch/2+4,0x293943);
  for(int i=0;i<2;i++)for(int j=0;j<2;j++)ns_cushion(l+19+i*(cw-52),top-3+j*(ch-8),16,11,0x202c31);
  ns_cushion(l,top,cw,ch,mix(c,DARK,2));ns_cushion(l+1,top+1,cw-2,ch-5,c);
  /* Precisely delineated windshield, sloping hood, trunk vents and twin lamps. */
  int u=l+cw/4,d=l+cw*2/3;
  quad((Pt){u-7,top+7},(Pt){d+3,top+7},(Pt){d+3,b-7},(Pt){u-7,b-7},0x31465a);
  quad((Pt){u-4,top+8},(Pt){u+7,top+10},(Pt){u+7,b-10},(Pt){u-4,b-8},0xa0bdc5);
  quad((Pt){d-10,top+10},(Pt){d+2,top+8},(Pt){d+2,b-8},(Pt){d-10,b-10},0x9ebec6);
  ns_cushion(u+7,top+8,d-u-17,ch-16,mix(c,0xedc79a,2));
  for(int i=0;i<2;i++){int yy=top+5+i*(ch-13);rect(r-12,yy,8,4,0xf1dcb1);rect(l+2,yy,7,3,0xce796c);
   line(d+9,yy+2,r-16,yy+4,mix(c,PAPER,3));}
  if(k==6){rect(d+5,cy-4,r-d-8,6,0xeee0b2);rect(l-1,top+3,6,ch-6,0x424b4d);disc(cx-2,cy,9,0xe7d5af);rect(cx-3,cy-5,3,10,0x484b4b);}
  return 1;}
 if(k==3&&t==INT_DESK){
  ns_oval(cx+2,cy+4,w/2,h/2,0x35424b);ns_oval(cx,cy,w/2,h/2,0xacb7b4);ns_oval(cx,cy-3,w/2-3,h/2-3,0xcec8af);
  for(int i=0;i<4;i++)ns_diamond(x+26+i*14,cy,6,6,0x486a77+i*0x111005);
  int_monitor(x+w-48,y+8,24,0x728c8d);return 1;}
 if(k==4&&!interior_subroom&&t==INT_FERNS){
  /* Tiered open timber stands, large leaves and round terracotta pots. */
  int rows=(h>52?2:1);for(int j=0;j<rows;j++){
   int yy=rows==1?cy: y+14+j*(h-27);ns_pallet(x+2,yy-4,w-4,13);
   for(int i=16;i<w-7;i+=26){U seed=int_hash(x+i,yy);int r=10+(int)(seed%8);
    ns_oval(x+i+2,yy+3,10,7,0x554a36);ns_oval(x+i,yy+1,9,7,(seed&2)?0xbb8058:0x937a5c);
    int_fern(x+i,yy-2,r,mix(c,(i%3)?0x8ea652:0x437c5a,3),seed,0);}
  }return 1;}
 if(k==5&&(t==INT_LAB||t==INT_FEATURE)){
  if(t==INT_FEATURE){ns_tank(x+w/2,cy,w/2-9,h/2-2,0x788c99);ns_tank(cx,cy,37,27,0xb299bc);
   ns_pipe(x+5,y+4,32,1,7,0xb28f78);ns_pipe(x+w-13,y+h/2+3,33,1,7,0x8e9baa);}
  else{int n=w>120?3:2;for(int i=0;i<n;i++){int u=x+(2*i+1)*w/(2*n);ns_tank(u,cy,w/(2*n)-4,h/2-4,i&1?0x85928a:0x8894ad);}}
  return 1;}
 if(k==6&&t==INT_TOOLS){for(int i=0;i<5;i++)ns_wheel(x+17+i*33,cy-1,13,0x90978c);return 1;}
 if(k==7&&(t==INT_REEL||t==INT_CRATES)){
  if(t==INT_REEL){ns_radio(x+1,y+3,w-2,0x998a89);ns_luggage(x+13,y+26,w-25,16,0x826959);}
  else{ns_luggage(x+2,y+5,w-25,h-9,0xa77763);ns_luggage(x+w-32,y+16,30,19,0x7a6267);}return 1;}
 if(k==7&&t==INT_DINER){
  ns_cushion(x+2,y+5,w,h,0x3d3841);rect(x,y,w,h-5,0x775f5e);
  for(int j=0;j<8;j++)rect(x+3,y+j*4,w-6,2,0x9b7b77);
  box(x+7,y+3,w-14,18,0x343c43,0xb8a08d);rect(cx-15,y+16,30,7,0xc4ad88);ns_slip(x+w-28,y+12,0xd0bba1);
  return 1;}
 if(k==8&&t==INT_CLINIC){
  /* Casters, segmented mattress, folded linen and an IV stand. */
  for(int i=0;i<2;i++)for(int j=0;j<2;j++)disc(x+13+i*(w-32),y+7+j*(h-12),4,0x43575a);
  ns_cushion(x+3,y+3,w-15,h-6,0x809394);ns_cushion(x+6,y+4,w-21,h-12,0xc7ceba);
  ns_cushion(x+8,y+8,27,h-21,0xe3dfca);ns_cushion(x+42,y+6,w-62,h-16,0x86b2a6);
  for(int i=0;i<4;i++)rect(x+46+i*14,y+9,1,h-22,0x7ca096);
  rect(x+4,y+1,w-21,2,0xb2c1bc);rect(x+4,y+h-7,w-21,2,0xb2c1bc);
  line(x+w-9,y+10,x+w-9,y+h-12,0x566f77);line(x+w-14,y+h-9,x+w-9,y+h-12,0x52696e);
  line(x+w-9,y+h-12,x+w-3,y+h-9,0x52696e);box(x+w-14,y+4,9,14,0xbcd1c6,0x61878b);rect(x+w-11,y+6,4,5,0xe1e4c9);
  return 1;}
 if(k==8&&t==INT_SOFA){for(int j=0;j<3;j++){int u=x+j*(w/3);rect(u+8,y+4,4,h-4,0x637875);
  ns_cushion(u,y+2,w/3-4,8,0x558583);ns_cushion(u,y+12,w/3-4,13,0x70928a);}return 1;}
 if((k==9||k==13)&&t==INT_BED){
  /* Safehouse is a sea berth with a rope runner; apartment is a patched quilt,
     book and bedside lamp. Distinct silhouettes and no clinic bed beading. */
  int end=x+w-19;ns_cushion(x+2,y+6,w-17,h-4,0x4f4742);ns_cushion(x,y,w-18,h-3,k==9?0x755c47:0x8e6550);
  ns_cushion(x+5,y+4,w-28,h-12,0xc5b89a);ns_cushion(x+7,y+10,28,h-24,k==13?0xe5cfb1:0xc0c1a9);
  for(int j=0;j<4;j++)for(int i=0;i<4;i++){U quilt[]={0xab685e,0xc79273,0x758986,0x8c6471};int u=x+37+i*(w-59)/4,v=y+4+j*(h-12)/4;
   rect(u,v,(w-59)/4+1,(h-12)/4+1,k==9?(i%2?0x738786:0x849695):quilt[(j*3+i)%4]);if(k==13){line(u+1,v+2,u+(w-59)/4-1,v+(h-12)/4-2,0xc99b85);}}
  rect(x+36,y+4,5,h-12,0xd0c6a9);ns_cushion(end,y+4,19,24,0x87694e);disc(end+9,y+15,7,0xa28861);
  if(k==13){disc(end+9,y+13,6,0xe7b86d);disc(end+8,y+12,4,0xf2cf88);ns_slip(end+1,y+h-21,0xd4b187);}
  else{disc(end+9,y+14,4,0x668889);}return 1;}
 if((k==13||k==9)&&t==INT_SOFA){
  ns_sofaread(x,y,w,h,k==13?0x69938b:0x8b7857,2);
  ns_diamond(x+16,y+17,8,8,k==13?0xc49667:0x9e9b76);
  rect(x+w-28,y+8,15,h-13, k==13?0xbc816e:0x6b8381);for(int i=0;i<4;i++)rect(x+w-27,y+12+i*5,13,1,0xd2b38c);return 1;}
 if(k==13&&t==INT_CONSOLE){ns_oval(cx+3,cy+4,w/2,h/2,0x6c5546);ns_oval(cx,cy,w/2,h/2,0xa9865b);
  ns_oval(cx,cy-2,w/2-2,h/2-2,0xbc986d);disc(x+w-17,cy-3,6,0x7b866e);disc(x+w-17,cy-4,4,0x5f5145);
  rect(x+12,y+6,19,14,0x8c7155);rect(x+11,y+5,19,13,0xf0ddad);line(x+12,y+7,x+20,y+12,0xc0a881);line(x+20,y+12,x+28,y+7,0xc0a881);
  return 1;}
 if((k==10&&!interior_subroom||k==10&&interior_subroom)&&t==INT_CAR){
  /* The yard has a wreck. The back workshop has a partially stripped chassis. */
  int l=x+6,r=x+w-5;U metal=interior_subroom?0x788686:0xa6946b;
  for(int i=0;i<2;i++)for(int j=0;j<2;j++)ns_cushion(x+13+i*(w-48),y+5+j*(h-20),22,13,0x232f34);
  quad((Pt){l,y+15},(Pt){l+27,y+8},(Pt){r-19,y+9},(Pt){r,y+20},metal);
  quad((Pt){l,y+15},(Pt){r,y+20},(Pt){r-8,y+h-13},(Pt){l+12,y+h-8},metal);
  box(l+21,y+14,w/2,h-28,0x485b5b,0xc0af89);
  for(int j=0;j<2;j++)ns_cushion(l+36,y+17+j*(h-43),23,13,interior_subroom?0x4b5c5b:0x8d7567);
  box(r-48,y+17,38,h-34,0x485957,0x695e4f);for(int i=0;i<5;i++)rect(r-42+i*6,y+22,3,h-45,0x93a19a);
  if(!interior_subroom){tri(l+43,y+12,l+74,y+14,l+61,y+27,0x9bb1aa);line(l+47,y+12,l+55,y+23,0x4d6364);rect(r-11,y+23,6,5,0xcdac7a);}
  else{ns_wheel(l+9,cy,10,0x84908a);rect(l+22,cy,w-48,3,0xa79f85);}
  return 1;}
 if(k==11&&(t==INT_LOCKER||t==INT_SOFA)){
  if(t==INT_LOCKER){for(int i=0;i<3;i++){int u=x+i*(w/3);box(u,y,w/3-2,h,0x907755,0x675944);
    for(int j=0;j<2;j++){box(u+2,y+3+j*(h/2-1),w/3-6,h/2-4,0xa48a63,0x7e674e);rect(u+9,y+7+j*(h/2-1),10,3,0xe1d1a5);}}}
  else{rect(x+7,y+h-8,6,9,0x625a50);rect(x+w-15,y+h-8,6,9,0x625a50);for(int i=0;i<4;i++)ns_cushion(x,y+i*7,w,5,0x8a7459);rect(x+7,y+2,3,h-8,0x716352);rect(x+w-12,y+2,3,h-8,0x716352);}return 1;}
 if(k==11&&t==INT_DINER){
  ns_cushion(x+3,y+4,w,h,0x5e5a53);ns_cushion(x,y,w,h,0xa49170);ns_cushion(x+4,y+3,w-8,h-10,0xb4ad99);
  for(int i=0;i<3;i++){int u=x+17+i*50;box(u,y+6,39,15,0x7d9494,0x645e54);rect(u+16,y+16,10,6,0x927f63);rect(u-3,y+1,3,28,0xc4ad7f);}
  return 1;}
 if(k==14&&t==INT_LOCKER){
  for(int i=0;i<4;i++){int u=x+3+i*29;quad((Pt){u+6,y+3},(Pt){u+19,y+3},(Pt){u+26,y+15},(Pt){u+23,y+h-4},0x48554e);
   quad((Pt){u+6,y+3},(Pt){u+22,y+7},(Pt){u+23,y+h-4},(Pt){u,y+h-4},0x788477);rect(u+8,y+1,9,5,0x485953);
   for(int j=0;j<3;j++)rect(u+4,y+12+j*5,16,2,0x465750);}return 1;}
 if(k==15&&t==INT_BED){box(x,y,w,h,0x546366,0x748286);
  rect(x+3,y+4,13,h-8,0xb4b4a2);rect(x+18,y+3,w-21,h-6,0x82959a);
  rect(x+w-9,y+3,3,h-6,0x6c7a83);return 1;}
 if(k==15&&t==INT_SOFA){int count=4;for(int i=0;i<count;i++){int u=x+i*w/count;int sw=w/count-3;
  rect(u+sw/2-2,y+4,4,h-5,0x77828a);ns_cushion(u,y+3,sw,9,0x58748a);ns_cushion(u+1,y+15,sw-2,17,0x70889a);}
  return 1;}
 if(interior_subroom&&k==0&&t==INT_RACK){
  /* Bundled delivery sacks on a loading trolley. */
  rect(x+2,y+h-8,w-4,5,0x687f7b);for(int i=0;i<4;i++){int u=x+15+i*(w-16)/4;
   ns_oval(u,y+h/2+2,14,16,0xb19e77);tri(u-9,y+8,u,y+15,u+6,y+7,0x9a8966);rect(u-6,y+11,11,2,0x5a6961);
   line(u-8,cy+5,u-3,cy-2,0x948665);}return 1;}
 if(interior_subroom&&k==2&&(t==INT_COOLER||t==INT_CRATES)){
  if(t==INT_COOLER){int_solid_shape(x,y,w,h,0x9dacaa);rect(x+4,y+5,w-8,h-12,0xa8bbb5);
   for(int i=0;i<2;i++){int u=x+23+i*(w-42)/2;disc(u,cy,12,0x607c82);disc(u,cy,10,0x98aaa7);
    for(int j=0;j<4;j++){float a=j*1.5708f;tri(u,cy,u+cs(a)*9,cy+sn(a)*9,u+cs(a+.84f)*8,cy+sn(a+.84f)*8,0x536e74);}}
   ns_pipe(x+w-44,y+5,h-12,1,6,0x789992);}
  else{ns_pallet(x,y+2,w,h-4);for(int i=0;i<3;i++)ns_parcel(x+5+i*(w-10)/3,y+6+(i%2)*4,w/3-7,h-18,0xaf916d);
   rect(x+w-28,y+6,14,8,0x6eaaa9);}return 1;}
 if(interior_subroom&&k==10&&t==INT_CRATES){for(int i=0;i<5;i++)ns_wheel(x+16+i*30,cy,15,0x7d8988);return 1;}
 return 0;
}
/* Additional set dressing shares the same rigid boundaries as the original props.
   Keep these substantial visual changes out of walking routes, especially the
   virtual cashier approach and the north staff gate/rail. */
static void ns_curtain(int x,int y,int w,int h){
 if(w>h){for(int i=0;i<w;i+=9){int m=min(9,w-i);rect(x+i,y,m,h+3,(i/9)&1?0x8cbcb3:0xa4cbc0);
   rect(x+i,y,2,h+2,0x72968f);}ns_pipe(x,y-1,w,0,3,0xa1b1a6);}
 else{for(int i=0;i<h;i+=9){int m=min(9,h-i);rect(x-3,y+i,w+4,m,(i/9)&1?0x80b1a8:0xa6c6b7);
   rect(x-1,y+i,w+2,2,0x6a9893);}ns_pipe(x+w-2,y,h,1,3,0xa5b7af);}
}
static void ns_badge(int x,int y,int r,U linec,U field){
 tri(x-r,y-r,x+r,y-r,x,y+r,linec);rect(x-r,y-r,r*2, r,linec);
 tri(x-r+2,y-r+1,x+r-2,y-r+1,x,y+r-3,field);rect(x-r+2,y-r+2,r*2-4,r-3,field);
 ns_diamond(x,y-1,r/2,r/2,linec);
}
static void ns_leafpot(int x,int y,int r,unsigned seed){
 ns_oval(x+1,y+r/3,r*3/4,r/2,0x7c6148);ns_oval(x,y+r/4,r*3/4,r/2,0xad8054);
 for(int i=0;i<7;i++){float t=i*.8976f+(seed%5)*.21f;int dx=cs(t)*r*.9f,dy=sn(t)*r*.82f,px=-dy/3,py=dx/3;
  int ex=x+dx,ey=y+dy;U c=i%2?0x58816b:0x799d62;
  quad((Pt){x,y},(Pt){x+dx/2+px,y+dy/2+py},(Pt){ex,ey},(Pt){x+dx/2-px,y+dy/2-py},c);
  line(x,y,ex,ey,i%2?0x8ab07e:0xa9ba7f);}
}
static int ns_setpiece(const InteriorProp*f,int k){
 int x=f->x,y=f->y,w=f->w,h=f->h,cx=x+w/2,cy=y+h/2,t=f->type;
 if(k==4&&t==INT_PLANT){ns_leafpot(cx,cy,min(w,h)/2+3,int_hash(x,y));return 1;}
 if(k==4&&t==INT_GROWBED){
  ns_cushion(x+3,y+4,w,h,0x2c444b);ns_cushion(x,y,w,h,0x79979a);box(x+4,y+10,w-8,h-18,0x3d5356,0x9db4a6);
  for(int s=0;s<5;s++){int u=x+25+s*37;ns_oval(u,cy+4,14,12,0x98aaa5);ns_oval(u,cy+3,11,10,0x58685d);
   ns_oval(u,cy+3,8,7,0x544d41);for(int j=0;j<12;j++){U z=int_hash(s*13+j,y);dot(u-6+z%13,cy-2+(z>>10)%9,0x8a7957);}}
  for(int j=0;j<2;j++){int v=y+2+j*(h-9);rect(x+7,v,w-14,4,0xb8abc6);rect(x+10,v+1,w-20,2,0xf3cfed);
   rect(x+4,v-1,5,6,0x51676f);rect(x+w-8,v-1,5,6,0x51676f);}return 1;}
 if(k==8&&(t==INT_WALL||t==INT_GLASS)){ns_curtain(x,y,w,h);return 1;}
 if(k==8&&t==INT_DESK){
  ns_cushion(x+3,y+4,w,h,0x54746e);ns_cushion(x,y,w,h,0x719790);ns_cushion(x+2,y+1,w-4,h-10,0xd1d3b8);
  box(x+9,y+6,32,19,0x5b8b84,0x53776f);rect(x+23,y+8,5,14,0xf0e6c8);rect(x+18,y+13,15,4,0xf0e6c8);
  ns_slip(cx-9,y+8,0xf2e7c7);ns_slip(cx+4,y+10,0xc9d1bd);int_monitor(x+w-41,y+5,28,0x708d83);
  for(int i=5;i<w-8;i+=7)rect(x+i,y+h-7,2,4,0x53827b);return 1;}
 if(k==15&&t==INT_DESK){
  if(x>=400&&y>260){/* Broad municipal front desk, lowered wheelchair-accessible section and badge. */
   ns_cushion(x+3,y+4,w,h,0x394958);ns_cushion(x,y,w,h,0x6a6055);ns_cushion(x+2,y+1,w-4,h-11,0xc1af91);
   for(int i=7;i<w-5;i+=13)rect(x+i,y+h-11,2,8,0x8d7b60);
   ns_slip(x+26,y+6,0xdaceb2);ns_radio(x+46,y+6,36,0x8f958a);int_monitor(x+w-48,y+5,34,0x809795);
   ns_badge(cx+13,y+h-11,8,0xd2bb7b,0x576878);
  }else{
   ns_cushion(x+2,y+3,w,h,0x4f504c);ns_cushion(x,y,w,h,x<300?0x97785a:0x8e8c78);
   if(x<300){ns_slip(x+12,y+7,0xdacbac);ns_radio(x+w-48,y+6,37,0x7a8b7f);ns_badge(cx,y+12,6,0xd9b87f,0x526776);}
   else{ns_slip(x+10,y+9,0xd6c9ac);ns_slip(x+24,y+11,0xb7c0ae);int_monitor(x+w-44,y+5,32,0x729394);}
  }return 1;}
 if(interior_subroom&&t==INT_SETTLE){
  if(k==0){/* Mailroom scale with a wide oval weighing pan, stamp and manifests. */
   int_solid_shape(x,y,w,h,0x869687);ns_oval(x+32,cy+4,25,13,0x4f6763);ns_oval(x+32,cy+1,23,12,0xaab9aa);
   ns_parcel(x+20,cy-12,24,16,0xcfa97c);ns_slip(cx-3,y+11,0xdfd7b9);int_monitor(x+w-42,y+8,28,0x9db59e);}
  else if(k==2){/* Receiving cashier: scuffed timber, a brass service bell and ledger. */
   ns_pallet(x,y,w,h-2);rect(x+4,y+5,w-8,h-14,0x9f8c66);box(x+11,y+8,40,25,0xc2b188,0x6d6655);rect(x+30,y+9,2,23,0x957c5b);
   for(int j=0;j<5;j++)rect(x+15,y+12+j*3,11,1,0x8c806b);disc(cx+1,cy,8,0x7d7864);ns_oval(cx,cy-3,7,5,0xcdbc8b);
   box(x+w-47,y+7,32,24,0x526864,0x8c967f);rect(x+w-43,y+10,23,5,0xbec5a2);for(int i=0;i<8;i++)rect(x+w-42+i%4*5,y+18+i/4*4,3,2,0xb2b398);}
  else{/* Fleet office drafting surface with a folded work order and steel hand lamp. */
   int_solid_shape(x,y,w,h,0x92795f);box(x+7,y+4,w-47,h-15,0x5d8992,0x9db5ab);
   rect(x+16,y+10,38,14,0x7499a0);for(int i=0;i<3;i++){line(x+17,y+11+i*4,x+49,y+11+i*4,0xc0c7b5);}
   ns_slip(x+w-54,y+13,0xd7c6a1);disc(x+w-21,y+19,9,0x666e64);disc(x+w-23,y+15,7,0xd1b47c);}
  return 1;}
 if(k==10&&interior_subroom&&t==INT_CAR){
  /* Yellow lifting gantry, hanging motor and a disassembled axle, rather than
     showing a second copy of the tow-floor car in its adjacent office. */
  ns_pallet(x+6,y+h/2+9,w-12,h/2-10);
  for(int j=0;j<2;j++){int u=x+9+j*(w-24);rect(u,y+5,7,h-12,0x715733);rect(u+1,y+4,4,h-13,0xd9ac61);
   rect(u-4,y+h-11,16,7,0x827558);}
  box(x+4,y+3,w-8,11,0xbc965b,0x6e5e43);for(int i=0;i<w-19;i+=17)line(x+8+i,y+12,x+16+i,y+4,0x705f44);
  rect(cx-9,y+1,18,17,0x596968);rect(cx-5,y+4,10,5,0x8f9a88);
  line(cx-4,y+17,cx-13,cy-1,0xd7c5a0);line(cx+3,y+17,cx+14,cy-1,0xc8baa3);
  box(cx-19,cy,39,23,0x7d8f89,0x4b5f62);for(int i=0;i<5;i++)rect(cx-14+i*6,cy+4,3,11,0x52686a);
  ns_wheel(x+29,y+h-21,11,0x87928a);ns_wheel(x+w-29,y+h-21,11,0x87928a);return 1;}
 return 0;
}
static void ns_fixed_details(int id){
 if(id==15){
  /* Fixed wall fixtures: sanitary pans in the cells and a framed precinct crest. */
  for(int i=0;i<2;i++){int u=684+i*76;box(u-6,239,13,5,0xa8b5b0,0x6b817f);ns_oval(u,247,7,5,0xc2cbc0);ns_oval(u,247,4,3,0x849694);}
  ns_badge(311,122,13,0xddc282,0x526f87);
 }else if(id==14){
  /* A carriage rail and lane divider rings make the little gunsmith read as a shooting gallery. */
  for(int i=0;i<3;i++){int u=210+i*42;rect(u,145,2,14,0xafb19f);ns_oval(u+1,147,4,2,0xb9c0ac);}
 }else if(id==10||id==18){
  /* Stowed flexible hoses on actual wall hooks alongside the lift. */
  for(int i=0;i<3;i++){int u=214+i*30;ns_ring(u,123,13,9,3,i==2?0xb08759:0x525d59);rect(u-1,111,3,5,0xc2b291);}
 }
}
