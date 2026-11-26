/* Hand-built sets, drawn into wi_back once when entering or toggling the station.
   World space/mission anchors never depend on the raster cache. */
enum {WM_WOOD,WM_CHECK,WM_CARPET,WM_SLATE,WM_BRICK,WM_TILE,WM_RUBBER,WM_MARBLE,WM_METAL,WM_WATER};
typedef struct {U floor,zone,wall,light;unsigned char fm,zm;} WiFinish;
static const WiFinish wiFinish[]={
 {0x763e4e,0xbb925c,0x5b283b,0xffdd9a,WM_CARPET,WM_CARPET}, /* picture palace */
 {0x947e62,0x42334b,0x392f42,0xf1a77b,WM_WOOD,WM_RUBBER},
 {0xb6815f,0xc0a991,0x93634c,0xffdc91,WM_BRICK,WM_CHECK},
 {0x85654e,0x697577,0x62636a,0xead3ad,WM_WOOD,WM_CARPET},
 {0x514b60,0x77564e,0x333a4a,0x69e8c5,WM_RUBBER,WM_WOOD},
 {0xbdb4a0,0x88958b,0xcfc9b3,0xf5b883,WM_MARBLE,WM_WOOD},
 {0xc18c6f,0x9e7467,0x9e7362,0xf2c397,WM_TILE,WM_BRICK},
 {0xa8bdb5,0x62a7ac,0x54838b,0xc1eee0,WM_CHECK,WM_TILE},
 {0x746f60,0x435556,0x515b58,0xf2ad53,WM_SLATE,WM_RUBBER},
 {0x294657,0x336d77,0x254856,0x89e7dc,WM_WOOD,WM_WATER},
 {0x543849,0x92705a,0x502a3d,0xffc377,WM_CARPET,WM_WOOD},
 {0x9c7d55,0x486b69,0x4c6769,0xf9db90,WM_WOOD,WM_RUBBER},
 {0xc0aa83,0x777b79,0x50565f,0xffca66,WM_TILE,WM_CHECK},
 {0x928a75,0x79635c,0x745857,0xefaf7e,WM_SLATE,WM_WOOD},
 {0x2e364a,0x46556a,0x213449,0xb2caed,WM_RUBBER,WM_METAL},
 {0x88775f,0x4f6966,0x546561,0xf8cc78,WM_WOOD,WM_SLATE},
 {0x9c7b59,0x795055,0x765851,0xf4cc82,WM_WOOD,WM_CARPET},
 {0x95685f,0xb7b09b,0x5e7477,0xffd08f,WM_WOOD,WM_CHECK},
 {0x967c59,0x627869,0x705b4a,0xf2d690,WM_WOOD,WM_CARPET}
};
static int wi_abs(int x){return x<0?-x:x;}
static void wi_oval(int x,int y,int rx,int ry,U c){
 if(rx<1||ry<1)return;
 for(int j=-ry;j<=ry;j++){int w=(int)(rx*sq(1.f-(float)(j*j)/(ry*ry)));rect(x-w,y+j,2*w+1,1,c);}
}
static void wi_ellipse(int x,int y,int rx,int ry,U c){
 int ox=x+rx,oy=y;
 for(int i=1;i<=56;i++){float a=i*(6.283185f/56.f);int nx=x+(int)(rx*cs(a)),ny=y+(int)(ry*sn(a));line(ox,oy,nx,ny,c);ox=nx;oy=ny;}
}
static void wi_rivet(int x,int y,U c){rect(x-1,y-1,3,3,0x24313c);dot(x-1,y-1,c);}
static void wi_floor_pixel_pal(U c,U p[8]){
 const U tone=0x252c33,white=0xf6e8c9;
 p[0]=c;p[1]=mix(c,tone,1);p[2]=mix(c,tone,2);p[3]=mix(c,tone,3);
 p[4]=mix(c,white,1);p[5]=mix(c,white,2);p[6]=mix(c,white,3);p[7]=mix(c,tone,4);
}
static int wi_material(int m,int x,int y){
 int u=x+128,v=y+128;
 switch(m){
 case WM_WOOD:{int row=v/12,b=(u+(row%3)*31)%97;if(v%12==0||b<2)return 2;
  if(v%12==1)return 4;return (b+13*row)%64<16&&v%12==7?2:((row*7+u/97)%5==0);}
 case WM_CHECK:{int a=(u/20+v/16)&1;if(u%20==0||v%16==0)return 2;return a?5:1;}
 case WM_CARPET:return (u%18==v%18||u%18==17-v%18)?1:((u+v)%11==0?4:0);
 case WM_SLATE:return u%53==0||v%28==0?3:((u+v*3)%83==0?1:0);
 case WM_BRICK:{int row=v/12,b=(u+row%2*16)%33;return v%12<2||b<2?5:(b==3||v%12==3)?2:(row+u/33)%6==0?1:0;}
 case WM_TILE:return u%24==0||v%18==0?2:u%24==1||v%18==1?4:0;
 case WM_RUBBER:return u%52==0||v%42==0?2:(u+v)%9==0&&v%4==0?4:0;
 case WM_MARBLE:return (u+v/2+(v/8)%7)%85<2?2:(u+v/2)%96==0?4:0;
 case WM_METAL:return v%24==0||u%44==0?2:((u+v)%8==0&&u%8<3)?4:0;
 default:return (u+((v/15)%2)*(v%15)/2)%52<2&&v%15<9?5:(v%19==0&&u%37<12)?4:0;
 }
}
/* Trade circulation plans: tread, dance floor, prep crescent, reading islands,
   concourse, divided kiln floor, booth, long counter. All are walkable inlays. */
static int wi_zone(int role,int e,int x,int y){
 int u=wi_mirror?960-x:x,v=y-wi_top,A= e*23-23,dx=u-(441+A),dy=v-(157-e*9);
 switch(role){
 case 0:return wi_abs(u-480)<28+v/8||((v+e*16)%81)<10;
 case 1:return dx*dx/4+dy*dy<6400+e*420||u<210+e*30;
 case 2:return e==0?u<424+v/5:e==1?v<142||(u<340&&v<244):u+v/2<520;
 case 3:return e&1?wi_abs(u-460-v/4)<91:u-v/3<332+e*23;
 case 4:return wi_abs(u-(600-A))<95+((v>128&&e%2)?50:0)||u<240+e*35;
 case 5:return e==0?u-v/2<292:e==1?dy*dy+dx*dx/2<4600:e==2?wi_abs(v-117)<37:wi_abs(u-490)<75;
 case 6:return e==1?u+v/3>520:dx*dx/5+dy*dy<7500||u<191;
 case 7:return e==0?v<107:e==1?u-v/3<307||v>234:wi_abs(u-480)<126;
 case 8:return e==0?wi_abs(u+v/3-687)<125:wi_abs(v-123)<68;
 case 9:return u<330+(e%2?1:-1)*(v%130)/3||u>595+(e%2?-1:1)*(v%130)/3||v<65+e*7;
 case 10:return e==2?dx*dx/3+dy*dy<7400:e==1?u>280&&u<690&&v<142:v<102+e*12;
 case 11:return e==0?u>238&&u<710&&v>32&&v<250:e==1?wi_abs(dx)<133&&wi_abs(dy)<80:e==2?wi_abs(u-475)<115:wi_abs(dx)+2*wi_abs(dy)<217;
 case 12:return e==0?v<94||v>249:wi_abs(u-477)<105||v<64;
 case 13:return e==1?wi_abs(u-490-v/3)<100:e==0?v<146:u>572||v<89;
 case 14:return dx*dx+dy*dy*4<48000-e*5100||v<37;
 case 15:return e==0?u>466:e==1?wi_abs(v-155)<65:u+v/2<446;
 case 16:return e==0?wi_abs(u-478)<97:e==1?dx*dx/3+dy*dy<7700:e==2?wi_abs(v-150)<70:wi_abs(u-459)<100||v>254;
 case 17:return e==1?u>394:e==2?u+v/2<512:e==3?wi_abs(v-129)>57:u<444+v/4;
 default:return e==0?wi_abs(u-480)<140:e==1?wi_abs(u-391)+wi_abs(v-128)<164:wi_abs(v-154)<80;
 }
}
static void wi_shell(const WiFinish*f,int e){
 U pp[2][8];wi_floor_pixel_pal(f->floor,pp[0]);wi_floor_pixel_pal(f->zone,pp[1]);
 const int t=wi_top,b=442;U edge=0x23323a,outer=mix(f->wall,0x10212b,4),cap=mix(f->wall,f->light,3);
 /* Actual stepped cutaway geometry and grounded lip. Current row bounds are
    also used by wi_solid: lateral annexes are not painted unreachable space. */
 for(int y=t;y<=b;y++){
  int l=wi_shell_edge(y,0),r=wi_shell_edge(y,1);
  rect(l-7,y-3,r-l+14,15,0x15222a);rect(l-3,y,r-l+6,9,outer);
 }
 for(int y=t;y<=b;y++){
  int l=wi_shell_edge(y,0),r=wi_shell_edge(y,1);
  U*at=surface+y*rw;
  at[l-2]=edge;at[l-1]=cap;
  for(int x=l;x<r;x++){
   int a=wi_zone(wi_role,e,x,y);
   at[x]=pp[a][wi_material(a?f->zm:f->fm,x,y)];
  }
  at[r]=cap;at[r+1]=edge;
 }
}
/* Large trade silhouettes, all original. The same vocabulary can be printed as
   wall art or mounted over a service fixture; no expensive per-frame sprites. */
static void wi_vase(int x,int y,int w,int h,U a){
 int v=y-h;U rim=mix(a,0xf7dfbb,4);
 wi_oval(x,y-1,w+3,3,0x4f4845);
 for(int j=0;j<h;j++){int ww=1+(int)(w*(.30f+.69f*sn(j*(3.14159f/h))));if(j<h/4)ww=max(2,w/3);
  rect(x-ww,v+j,ww*2+1,1,a);if(j==h*2/3||j==h*2/3+2)rect(x-ww,v+j,ww*2+1,1,rim);}
 wi_oval(x,v,w/3+1,2,rim);rect(x-w/4,v,w/2,1,0x59494c);
}
static void wi_record(int x,int y,int r,U a){
 disc(x+2,y+3,r,0x302831);disc(x,y,r,0x20282f);
 for(int j=r-3;j>r/3;j-=4)wi_ellipse(x,y,j,j,0x48505a);
 disc(x,y,max(3,r/3),a);disc(x,y,2,0x3b3340);
}
static void wi_wave(int x,int y,int w,int h,int e,U a){
 for(int j=0;j<w;j+=4){int ht=2+(int)(h*absf(sn(j*.089f+e)*cs(j*.035f+1)));rect(x+j,y-ht,2,ht*2+1,a);}
}
static void wi_fish(int x,int y,int z,int kind,U a){
 if(kind==1){wi_oval(x,y,z,z*2/3,a);rect(x-z,y+1,z*2+1,z/3,a);
  for(int i=-2;i<=2;i++)for(int j=0;j<z;j++)dot(x+i*z/3+(int)(sn(j*.35f+i)*2),y+z/3+j,a);return;}
 if(kind==2){tri(x-z*2,y,x,y-z/2,x+z*2,y-2,a);tri(x-z*2,y,x+3,y+z/2,x+z*2,y-2,a);line(x+2,y+z/2,x+z,y+z+3,a);return;}
 wi_oval(x,y,z,z/2,a);tri(x-z+2,y,x-z*2,y-z/2,x-z*2,y+z/2,a);dot(x+z/2,y-1,0x2d5361);
}
static void wi_spot_star(int x,int y,int r,U c){rect(x-r,y,r*2+1,1,c);rect(x,y-r,1,r*2+1,c);if(r>4){line(x-2,y-2,x+2,y+2,c);line(x-2,y+2,x+2,y-2,c);}}
static void wi_wall(const WiFinish*f,int e){
 int t=wi_top,l=wi_shell_edge(t,0),r=wi_shell_edge(t,1),w=r-l;
 int h=42+(wiAddress[walkin_active].shell%3)*3,y=t-h;
 U dark=mix(f->wall,0x182632,5),lite=mix(f->wall,0xf7e4c2,4),a=f->light;
 box(l-3,y-3,w+6,h+7,dark,0x18262d);rect(l,y,w,h,f->wall);
 switch(wi_role){
 case 0:case 10:{ /* velvet pelmet: scalloped swags plus hanging folds */
  int span=wi_role==0?74+e*18:101+e*7;
  for(int x=l+2;x<r-3;x+=span){int ww=min(span,r-x-1);for(int j=0;j<ww;j++){
   int hang=5+(int)(17*sn(j*(3.14159f/ww)));rect(x+j,y,1,hang,((j/6)&1)?0x8d4052:0x702f45);
   dot(x+j,y+hang,0xd5a476);}for(int j=0;j<3;j++)rect(x+j*4,t-24,2,17+j*2,lite);}
  if(wi_role==0)for(int x=l+9;x<r-4;x+=18)disc(x,t-5,2,a);break;}
 case 1:{for(int x=l+9,j=0;x<r-38;x+=49,j++){U c=j%3?0xab7b72:0x7e9f96;
  if(e==1){box(x,y+4,38,32,c,dark);tri(x+3,y+31,x+19,y+8,x+34,y+31,lite);disc(x+25,y+12,4,a);}
  else wi_record(x+18,y+21,16,j%2?c:a);}break;}
 case 2:case 6:case 13:{ /* exposed masonry, arched ovens and proof rail */
  for(int j=0;j<h;j+=10){line(l,y+j,r-1,y+j,lite);for(int x=l+(j%20?13:1);x<r;x+=28)rect(x,y+j,1,min(10,h-j),lite);}
  for(int x=l+35,j=0;x<r-20;x+=86,j++){
   if(wi_role==2){wi_oval(x,y+26,23,21,dark);rect(x-23,y+26,47,14,dark);wi_oval(x,y+29,17,15,0x483943);rect(x-17,y+30,35,10,0x483943);
    for(int k=0;k<3;k++){wi_oval(x-11+k*10,y+34,6,4,0xd9a464);line(x-13+k*10,y+32,x-9+k*10,y+31,lite);}}
   else if(wi_role==6){wi_vase(x-9,t-5,8+(j%3)*2,18+j%3*4,e==1?0x628fa5:0xcc906d);wi_vase(x+12,t-5,6,15+(j%2)*8,e==2?0x7c9580:0xe0b38c);}
   else{rect(x-17,y+4,31,32,0xf3d7a8);if(e==1)disc(x,y+17,9,0xe88c71);
    tri(x-13,y+27,x-2,y+9,x+11,y+27,e==2?dark:0x4f7782);rect(x-12,y+30,21,2,dark);rect(x-3,y+1,6,5,dark);}
  }break;}
 case 3:{rect(l+10,y+6,w-20,3,a);for(int x=l+24,j=0;x<r-14;x+=39,j++){
   U cloth=(U[]){0x9ca99c,0xbe807e,0xc3af8e,0x667b95}[(j+e)%4];line(x,y+8,x-7,y+14,lite);line(x,y+8,x+7,y+14,lite);
   if(e==3){tri(x-15,y+15,x+14,y+12,x+14,t-4,cloth);line(x-12,y+15,x+12,t-6,lite);}
   else{tri(x,y+12,x-11,t-6,x+12,t-7,cloth);rect(x-6,y+15,13,15,cloth);line(x,y+17,x+1,t-9,lite);}}
  break;}
 case 4:{for(int x=l+4;x<r-5;x+=13){tri(x,y+3,x+10,y+3,x+4,t-5,dark);line(x+4,t-6,x+11,y+3,lite);}
  int start=l+17,end=r-17;rect(start,t-30,end-start,22,0x263a42);wi_wave(start+2,t-19,end-start-4,8,e,a);break;}
 case 5:{int size=48;for(int x=l+20,j=0;x<r-size;x+=85,j++){box(x,y+3,size,h-9,0xb48c73,dark);rect(x+4,y+7,size-8,h-17,0xe6d9b5);
   U ink=j%2?0x57817c:0xb57d68;tri(x+6,t-13,x+size/2,y+9,x+size-7,t-13,ink);disc(x+size*2/3,y+14,4,0x8d675b);}
  break;}
 case 7:case 17:{for(int j=0;j<h;j+=10){rect(l,y+j,w,1,lite);for(int x=l+(j%20?12:1);x<r;x+=24)rect(x,y+j,1,min(10,h-j),lite);}
  if(wi_role==7){for(int x=l+20;x<r-20;x+=40){wi_ellipse(x,y+20,12,12,0xa6c8c8);wi_ellipse(x+18,y+11,5,5,0xbfd4cd);}}
  else{rect(l+4,t-10,w-8,4,dark);for(int x=l+13;x<r-10;x+=31)rect(x,t-8,14,3,a);}
  break;}
 case 8:case 15:{for(int x=l+1;x<r;x+=24){rect(x,y+2,3,h-3,lite);line(x+1,t-2,min(x+22,r-1),y+3,dark);}
  if(wi_role==8){for(int x=l+31;x<r-24;x+=86){wi_ellipse(x,t-23,16,16,dark);for(int k=0;k<4;k++){float a0=k*.785398f;int dx=(int)(15*cs(a0)),dy=(int)(15*sn(a0));line(x-dx,t-23-dy,x+dx,t-23+dy,lite);}}}
  else{for(int x=l+8;x<r-15;x+=26){box(x,t-24,22,18,0xb58e67,dark);rect(x+9,t-23,4,15,0xe0cba3);}}
  break;}
 case 9:{rect(l+3,y+3,w-6,h-6,0x316779);for(int j=0;j<4;j++)line(l+4,y+6+j*3,r-4,y+6+j*3,mix(0x316779,0x9ddcce,3-j));
  for(int x=l+16,j=0;x<r-14;x+=47,j++){
   if(e==0)for(int s=0;s<4;s++)for(int k=0;k<19+j%4;k++)dot(x+s*3+(int)(5*sn(k*.19f+j)),t-3-k,s%2?0x668e70:0x90aa7c);
   wi_fish(x+7,y+20+(j%3)*3,5+((j+e)%3),e,e==1?0xd2aaca:0xabccc2);}
  for(int x=l+5;x<r;x+=122)rect(x,y,3,h,dark);break;}
 case 11:{rect(l+4,y+3,w-8,h-6,0x7d9994);for(int x=l+12;x<r-30;x+=48){line(x,y+4,x+12,t-4,0x99afa5);rect(x-4,y,2,h,dark);}
  rect(l+3,t-9,w-6,3,a);break;}
 case 12:{/* The tracks are behind the walkable platform, not painted traffic lanes. */
  rect(l,y,w,h-6,0x414953);for(int x=l+6;x<r-4;x+=14){rect(x,y+4,5,h-12,0x8b7560);rect(x,y+6,1,h-17,0xaa8a6a);}
  for(int j=0;j<2;j++){int yy=y+8+j*19;rect(l,yy,w,5,0x262e37);rect(l,yy,w,2,0xb4c2bc);rect(l,yy+2,w,1,0x7b8c91);}
  rect(l,t-9,w,9,0xc3aa72);for(int x=l+2;x<r-3;x+=6)rect(x,t-7,2,3,0xf5cf76);
  break;}
 case 14:{rect(l,y,w,h,0x263a53);for(int i=0;i<83;i++){U n=wi_hash((U)(i+45));int x=l+3+(int)(n%(U)(w-7)),yy=y+4+(int)((n>>11)%(U)(h-9));wi_spot_star(x,yy,1+(i%11==0),i%4?0xaabace:0xe0cda8);}
  for(int x=l+7;x<r-6;x+=84){rect(x,y,3,h,dark);line(x,t-2,min(x+36,r-2),y,lite);}
  break;}
 case 16:{for(int x=l+12,j=0;x<r-38;x+=85,j++){box(x,y+3,39,h-4,dark,lite);box(x+3,y+8,33,h-9,mix(f->wall,e&1?0x687f78:0x805946,3),dark);box(x+9,y+12,19,11,f->wall,lite);disc(x+29,t-10,2,a);}break;}
 default:{for(int x=l+4,j=0;x<r-8;x+=8,j++){int bh=12+(j*11)%19;U bc=(U[]){0x988b65,0x8e6461,0x667d73,0xc3a17e}[j%4];
 rect(x,t-6-bh,6,bh,bc);rect(x+1,t-11,4,1,lite);}rect(l+2,t-6,w-4,5,dark);break;}
 }
 /* Double-faced lintel/card on a wall, never on the walkable floor. */
 const char*txt=wiAddress[walkin_active].feature;int lw=textw(txt,0)+24;
 int cx=wi_role==12?(e?l+lw/2+12:r-lw/2-12):e%3==1?l+w/3:e%3==2?r-w/3:(l+r)/2;
 box(cx-lw/2,y-2,lw,22,0x233039,lite);rect(cx-lw/2+4,y+2,2,13,a);center(cx+2,y-2,txt,0xf2dfbc,0);
}
/* Walkable trade fittings and floor graphics, placed underneath collidable
   furniture. They give the store a room-sized composition, not scatter props. */
static void wi_grip(int x,int y,int w,int h,U c){
 for(int j=0;j<h;j+=10)for(int k=0;k<w;k+=12)line(x+k,y+j+3,x+k+3,y+j,c);
}
static void wi_mark(int x,int y,int n,U a){wi_ellipse(x,y,n,n*2/3,a);line(x-n-8,y,x+n+8,y,a);line(x,y-n*2/3-6,x,y+n*2/3+6,a);}
static void wi_inlay(const WiFinish*f,int e){
 const int t=wi_top,c=480+(e-1)*22;
 const U bright=mix(f->light,f->floor,3),mute=mix(f->floor,f->zone,4),edge=0x344348;
 const InteriorProp*g=wi_f;int gx=g->x+g->w/2,gy=g->y+g->h/2;
 switch(wi_role){
 case 0:{ /* gilded fan-shaped aisle and illuminated row numbers */
  for(int k=-3;k<=3;k++)line(c+k*8,t+39,480+k*48,405,0xae8566);
  for(int j=0;j<3;j++){int y=t+83+j*72;rect(192,y,165,2,0xb18b6c);rect(596,y,165,2,0xb18b6c);
   for(int i=0;i<5;i++){rect(198+i*31,y-2,4,2,0xf2ca8a);rect(600+i*31,y-2,4,2,0xf2ca8a);}}
  if(e==0)wi_spot_star(480,366,13,bright);else for(int j=0;j<4;j++)wi_ellipse(470,350,12+j*7,12+j*3,bright);
  break;}
 case 1:{int x=wi_mirror?555:405,y=308+e*11;for(int h=25;h<120;h+=9)wi_ellipse(x,y,h,h/2,mix(f->zone,bright,h%4+2));
  wi_oval(x,y,21,11,0x786553);wi_oval(x,y,18,8,e==0?0xa38c75:e==1?0xb27478:0x8ca591);dot(x,y,edge);
  if(e==2)for(int x0=260;x0<710;x0+=48)line(x0,384,x0-32,352,bright);break;}
 case 2:{/* Floor transition at a brick oven or pastry window. */
  for(int j=0;j<3;j++){wi_ellipse(gx,gy,105+j*13,53+j*6,0xd2a979);}
  int x=wi_mirror?290:661;for(int k=0;k<4;k++){line(x-33+k*18,352,x-27+k*18,333,0xb88d63);line(x-33+k*18,352,x-42+k*18,340,0xb88d63);}break;}
 case 3:{/* Flush measuring rule and broad fabric floor sample. */
  int y=357-e*13;rect(236,y,478,9,0xbb9d70);for(int i=0;i<476;i+=5)rect(236+i,y,1,i%25?3:7,0x675c51);
  for(int k=0;k<4;k++){int x=(wi_mirror?288:646)-20+k*10;line(x,245,x+24,312,bright);line(x-15,279,x+31,279,bright);}break;}
 case 4:{/* Real studio rug and looping signal cable under the control desk. */
  int x=wi_mirror?194:584;
  box(x,203,161,125,0x685660,0x93786a);box(x+5,208,151,115,0x63525a,0xb28b78);
  for(int j=0;j<5;j++)diamond(x+81,264,6+j*8,0xb18578);
  if(e<2){line(gx,g->y+g->h,gx,322,edge);line(gx,322,x+48,322,edge);wi_ellipse(x+49,322,14,7,edge);}
  else{for(int j=0;j<3;j++)wi_ellipse(x+79,275,25+j*18,10+j*8,bright);}break;}
 case 5:{/* Museum sightlines and large custom plinth outlines. */
  for(int j=0;j<3;j++){int x=(e&1)?252+j*173:280+j*189,y=(e==1?337:301)+j%2*31;wi_ellipse(x,y,39,14,bright);
   line(x-25,y+6,x+25,y+6,mute);}
  for(int j=0;j<3;j++){int y=385+j*3;line(252,y,711,y,mute);}break;}
 case 6:{int x=wi_mirror?645:315,y=291;
  for(int k=0;k<6;k++)wi_ellipse(x,y,61+k*6,31+k*3,mix(f->zone,bright,k%3+2));
  for(int j=0;j<8;j++){wi_oval(x+39+j*7,322+(j%3)*5,3+j%4,2,mix(f->zone,bright,3));}break;}
 case 7:{int x=wi_mirror?296:664;/* Tiled stylized bubbles and inset drain. */
  for(int i=0;i<7;i++){int xx=x+(i%4)*21-31;wi_ellipse(xx,351-(i/4)*25+(i%3)*3,9+(i%3)*3,8+(i%3)*3,bright);}
  rect(248,307-e*12,455,6,0x6a898b);for(int k=0;k<451;k+=7)rect(250+k,308-e*12,3,4,edge);break;}
 case 8:{int x=wi_mirror?240:561;box(x,203,162,132,0x4d5b59,0xdfa358);wi_grip(x+3,206,154,126,0x69716b);
  for(int k=0;k<13;k++)line(x+k*12,331,x+k*12+7,325,bright);
  if(e==0){for(int k=0;k<3;k++)line(282,386+k*4,693,386+k*4,bright);}else{for(int k=0;k<5;k++)diamond(270+k*35,373,7,bright);}break;}
 case 9:{/* Basin glass and an uninterrupted central timber jetty. */
  int l=wi_mirror?349:342,r=617;
  for(int k=0;k<6;k++){line(l+2+k*3,197,l-24+k*3,392,0x7ba596);line(r-3-k*3,202,r+18-k*3,391,0x779f96);}
  for(int j=0;j<4;j++){int x=(j&1)?677:275,y=235+j*43;wi_ellipse(x,y,38,13,0x579197);wi_ellipse(x,y,31,9,0x579197);}
  if(e==2)wi_fish(274,319,14,2,0x547f81);break;}
 case 10:{int x=g->x-20,y=g->y+g->h+3;
  if(e==2){wi_oval(480,256,127,46,0x463947);wi_oval(480,252,126,42,0x997a62);wi_ellipse(480,252,112,34,0xc8a178);}
  else {for(int j=0;j<3;j++)box(x-3*j,y+j*5,g->w+40+6*j,5,0xa18167,0x604d4c);}
  for(int k=0;k<11;k++)disc(x+11+k*(g->w+22)/11,y-2,2,0xf8c779);break;}
 case 11:{U chalk=0xd5c394;
  if(e==0){box(256,183,443,178,0xa38462,chalk);rect(258,185,439,174,0xa38462);
   for(int y=194;y<356;y+=15)line(260,y,690,y,0x9c7b59);line(477,184,477,358,chalk);wi_ellipse(477,269,43,36,chalk);wi_ellipse(257,269,57,57,chalk);wi_ellipse(698,269,57,57,chalk);}
  else if(e==1){box(347,212,263,127,0x667e79,0x9e7a61);for(int k=0;k<3;k++){int n=k*5;line(347+n,212+n,609-n,212+n,k==1?0xebe4c7:0xc18c73);line(347+n,337-n,609-n,337-n,k==1?0xebe4c7:0xc18c73);}wi_mark(479,274,22,chalk);}
  else if(e==2){for(int k=0;k<6;k++){int x=341+k*42;line(x,165,x,390,chalk);rect(x+3,358-k*8,36,3,chalk);}}
  else for(int j=0;j<6;j++){wi_ellipse(310+j*60,298+(j%2)*46,21,13,chalk);line(330+j*54,308,346+j*54,316,chalk);}
  break;}
 case 12:{/* Tactile platform strip and coarse arrows, visible at game zoom. */
  int y=t+17;rect(170,y,620,10,0xc5ab70);for(int k=0;k<612;k+=6)rect(173+k,y+3,2,3,0xf4cf77);
  for(int j=0;j<3;j++){int x=e==0?396+j*75:525-j*58,yy=330+(j%2)*26;
   rect(x-17,yy-2,21,5,bright);tri(x+4,yy-7,x+15,yy,x+4,yy+7,bright);}
  break;}
 case 13:{int x=wi_mirror?586:270,y=233;rect(x,y,126,81,0xc8ae89);
  wi_mark(x+4,y+4,8,0x596e75);wi_mark(x+120,y+77,8,0xa96d65);
  for(int k=0;k<3;k++){int xx=wi_mirror?325:615;wi_mark(xx+k*22,353,19,k%2?0xba766a:0x667a80);}break;}
 case 14:{int x=480+(e-1)*29,y=284;
  for(int k=0;k<5;k++)wi_ellipse(x,y,54+k*26,25+k*13,k%2?0x8c8f91:0xaf976c);
  for(int k=0;k<12;k++){float a=k*(6.283185f/12.f);int u=x+(int)(154*cs(a)),v=y+(int)(72*sn(a));wi_spot_star(u,v,k%3?3:6,bright);}
  line(x-156,y,x+156,y,0x918675);line(x,y-71,x,y+69,0x918675);
  if(e==1){wi_oval(x,y,21,13,0xc6a169);}break;}
 case 15:{int x=e&1?267:567;/* Sort zones painted into the factory floor. */
  for(int k=0;k<5;k++){int xx=x+k*21;line(xx,344,xx+18,324,bright);line(xx+1,344,xx+19,324,bright);}
  if(e==2){line(340,349,446,266,bright);line(340,349,419,314,bright);line(419,314,446,266,bright);}
  break;}
 case 16:case 18:{int x=e==1?402:e==2?512:475,y=e==0?315:284;
  if(wi_role==16){int wx=e==1?117:87,hy=e==1?63:47;wi_oval(x,y,wx,hy,0xb49374);wi_oval(x,y,wx-4,hy-3,0x795b5c);wi_ellipse(x,y,wx-12,hy-7,bright);
   for(int k=1;k<4;k++){wi_ellipse(x,y,14*k,9*k,bright);}}
  else{box(x-104,y-54,208,105,0x737f67,0xc1ac81);box(x-98,y-48,196,93,0x596d60,0xa9a07c);
   for(int k=0;k<4;k++)diamond(x-56+k*37,y,15,0xb4a37f);}
  break;}
 case 17:{int x=wi_mirror?339:598;for(int j=0;j<3;j++){wi_ellipse(x,237+j*65,44,16,bright);}
  if(e==2){for(int k=0;k<6;k++)wi_ellipse(275+k*18,364+(k%2)*8,10,6,bright);}
  else{int y=376;rect(240,y,468,2,bright);rect(240,y+5,468,2,bright);}break;}
 }
}
static void wi_bench(const InteriorProp*g,U c,int seats){
 int x=g->x,y=g->y,w=g->w,h=g->h;U dark=mix(c,0x182633,5),soft=mix(c,0xf2d6ad,3);
 rect(x+8,y+h-7,w-8,9,0x38424a);
 for(int k=0;k<seats;k++){int xx=x+k*w/seats+2,ww=w/seats-4;
  box(xx+2,y+3,ww-4,h-12,c,dark);rect(xx+4,y+4,ww-8,4,soft);
  box(xx,y+h/2,ww,h/2-5,mix(c,soft,2),dark);rect(xx+2,y+h/2+2,ww-4,2,soft);
  rect(xx+3,y+h-7,3,6,dark);rect(xx+ww-6,y+h-7,3,6,dark);}
}
static void wi_artwork(int x,int y,int w,int h,int ed,int slot,U a,int on){
 int m=x+w/2,n=y+h/2,bs=max(4,min(w/6,h/4));U clay=on?0xb9806c:0x93634f,pale=0xe1cfa6,metal=0x52646a;
 wi_oval(m,y+h-4,w/3,5,0x817c70);
 /* No tabletop footprint: sculptural silhouette with an actual plinth. */
 box(m-w/4,y+h-10,w/2,9,0xa19a87,0x666d67);rect(m-w/4+1,y+h-9,w/2-2,2,pale);
 if(ed==0){for(int k=0;k<3;k++){int xx=m-bs+bs*k,yy=y+3+(k*7+slot*3)%max(8,h/5);
  tri(xx-bs/2,yy+3,xx+bs/2,yy,xx+bs/2,y+h-11,k==1?clay:metal);
  tri(xx-bs/2,yy+3,xx+bs/2,y+h-11,xx-bs/2,y+h-11,k==1?clay:metal);
  line(xx+bs/2-1,yy+2,xx+bs/2-1,y+h-12,pale);}}
 else if(ed==1){rect(m-3,n,6,h/2-9,metal);tri(m-bs*2,n-8,m+bs*2,n-1,m-bs,n+3,clay);disc(m+bs,n-bs/2,bs/2+2,pale);disc(m-bs,n-bs,bs/2,0x426a68);}
 else if(ed==2){for(int j=0;j<4;j++){int xx=m-bs*2+j*bs,yy=n-8+(j%2)*8;tri(xx,yy,xx+bs/2,y+4,xx+bs,n+7,j%2?clay:metal);tri(xx,yy,xx+bs,n+7,xx+bs/2,y+h-13,j%2?0xbe966c:0x678e83);}}
 else{line(m,y+1,m,n-9,metal);line(m-bs*2,n-9,m+bs*2,n-14,metal);for(int k=-1;k<=1;k++){int xx=m+k*bs*2,yy=n+(k+2)*2;line(xx,n-11,xx,yy,metal);disc(xx,yy+3,max(3,bs/2),k<0?clay:k>0?metal:a);}}
}
static void wi_shelf(const InteriorProp*g,int e){
 int x=g->x,y=g->y,w=g->w,h=g->h;U maple=0xb39169,ink=0x574647;
 rect(x+3,y+7,w,h-4,0x384148);box(x,y,w,h-4,0x6f5a4b,ink);
 if(wi_role==1){/* front-facing bins of actual illustrated sleeves */
  for(int i=0;i<max(2,w/31);i++){int xx=x+5+i*(w-7)/max(2,w/31),ww=min(28,w-i*30-3);if(ww<6)continue;
   rect(xx,y+9,ww,h-18,(U[]){0xc49073,0x7e9994,0xae8ca2,0x98a7bb}[(i+e)%4]);
   if(i%2)wi_record(xx+ww/2,y+h/2,7,0xe7cd99);else{tri(xx+2,y+h-12,xx+ww/2,y+13,xx+ww-2,y+h-12,0x65616a);disc(xx+ww-7,y+16,3,0xe4c093);}}
 }else if(wi_role==6){for(int j=0;j<2;j++)for(int i=0;i<max(2,w/29);i++){int xx=x+14+i*(w-9)/max(2,w/29),yy=y+(j+1)*(h-5)/2;
  wi_vase(xx,yy-2,5+(i+j)%4,10+(i+j)%3*3,e==1?0x7896ab:(i+j)%2?0xc98972:0xc6ad82);}
 }else if(wi_role==3){rect(x+4,y+6,w-8,2,0xd6c3a1);for(int i=6;i<w-13;i+=14){U cloth=(U[]){0x9386a6,0x82999b,0xb98176,0xb8a684}[(i/14+e)%4];line(x+i+6,y+8,x+i+1,y+15,0xc1b397);
  tri(x+i+4,y+12,x+i-1,y+h-10,x+i+12,y+h-8,cloth);}}
 else{for(int j=0;j<2;j++){int yy=y+(j+1)*(h-7)/2;
  for(int k=5,i=0;k<w-5;i++){int bw=3+(i*7+j*3)%5,bh=max(4,(h-12)/2-(i*5)%6);
   U bc=(U[]){0x9b635b,0x688e83,0xc6a37a,0x81839d,0xe0c6a2,0x8396a0}[(i+j+e)%6];rect(x+k,yy-bh,bw,bh,bc);rect(x+k,yy-3,bw,1,0xd9c9a7);k+=bw+1;}
  rect(x+2,yy,w-4,2,maple);}}
 rect(x+1,y+h-8,w-2,4,maple);rect(x+2,y+h-3,4,3,ink);rect(x+w-6,y+h-3,4,3,ink);
}
static void wi_tank(const InteriorProp*g,int e,int on){
 int x=g->x,y=g->y,w=g->w,h=g->h,m=x+w/2;U rim=0x9ebfb6,base=0x3a555c;
 wi_oval(m,y+h-7,w/2,8,base);box(x,y+2,w,h-10,rim,0x314652);
 for(int j=0;j<h-15;j++)rect(x+5,y+5+j,w-10,1,mix(on?0x416487:0x51969d,on?0x243c64:0x295363,j*7/max(1,h-15)));
 for(int j=8;j<w-10;j+=11){int h0=5+(j*17)%max(7,h/3),u=x+j;
  for(int k=0;k<h0;k++){int off=(int)(3*sn(k*.23f+j));dot(u+off,y+h-12-k,e==0?0x8faf78:0x6d9a88);dot(u+off+1,y+h-12-k,0x7eae91);}}
 for(int i=0;i<max(3,w/28);i++){int xx=x+15+(i*39)%(w-30),yy=y+17+(i*17)%max(12,h-42);wi_fish(xx,yy,4+(e+i)%3,e,i%3?0xd9d4af:on?0xb790ae:0xdeab82);}
 rect(x+8,y+6,w-16,2,0xb7d6cf);line(x+13,y+10,x+5,y+23,0x86b6b8);line(x+w-15,y+8,x+w-23,y+21,0x87aeb0);
 wi_ellipse(m,y+h-9,w/2-3,4,0x74969a);
}
static void wi_stage_set(const InteriorProp*g,int e,int on){
 int x=g->x,y=g->y,w=g->w,h=g->h,m=x+w/2;U wood=on?0xb08b65:0x957660;
 rect(x+2,y+6,w,h-1,0x4c4149);box(x,y,w,h-3,wood,0x51444b);
 for(int j=6;j<h-7;j+=7)line(x+2,y+j,x+w-3,y+j,0x876951);
 int dr=min(70,w/4);for(int jj=0;jj<dr;jj++){int h0=max(7,(h-5)*(dr-jj)/dr);
  rect(x+2+jj,y+2,1,h0,((jj/5)&1)?0x93485a:0x6c374c);
  rect(x+w-3-jj,y+2,1,h0,((jj/5)&1)?0xa15560:0x74394b);}
 for(int j=8;j<w-6;j+=18)disc(x+j,y+h-6,2,0xeacc8b);
 if(e==3){for(int i=-1;i<=1;i++){disc(m+i*18,y+17+(i+1)*3,5,0x474351);tri(m+i*18,y+22,m+i*18-6,y+h-11,m+i*18+8,y+h-11,0x474351);}}
 else{wi_oval(m,y+h*2/3,13,4,0x6b5f5b);rect(m,y+h/3,2,h/3,0x434a50);line(m-2,y+h/3-1,m+6,y+h/3-3,0x373d46);}
}
static void wi_service(const InteriorProp*g,int e,int slot,int on){
 int x=g->x,y=g->y,w=g->w,h=g->h,m=x+w/2,n=y+h/2,t=g->type;
 U pale=0xe6d2a7,ink=0x2d3742,metal=0x9badaa;
 if(t==WI_STAGE){wi_stage_set(g,e,on);return;}
 if(t==WI_GALLERY&&wi_role==5){wi_artwork(x,y,w,h,e,slot,g->tint,on);return;}
 if(t==WI_AQUA){wi_tank(g,e,on);return;}
 if(t==INT_RACK&&(wi_role==1||wi_role==3||wi_role==6||wi_role==18)){wi_shelf(g,e);return;}
 if(t==INT_SOFA){wi_bench(g,mix(wiFinish[wi_role].zone,g->tint,3),3);return;}
 if(t==WI_SEATS&&wi_role==12){if(slot==2){/* waist-high gate line separating paid/unpaid halls */
   int count=4;for(int j=0;j<count;j++){int xx=x+5+j*(w-4)/count;box(xx,y+2,13,h-6,0x809894,ink);rect(xx+2,y+4,9,4,j==1?0xeb8e78:0x84c9a3);
    line(xx+10,n+3,xx+26,n-4,metal);line(xx+10,n+4,xx+26,n-3,pale);line(xx+10,n+3,xx+21,n+13,metal);}}
  else wi_bench(g,0x718783,4);return;}
 if(t==WI_SEATS){wi_bench(g,wi_role==0?0x9e5868:0x955661,max(3,w/24));return;}
 if(t==WI_OVEN){/* vaulted kiln, or brick bake oven with rack and lit mouth */
  int cnt=w>170?3:2;
  for(int k=0;k<cnt;k++){int cw=(w-4)/cnt,xx=x+2+k*cw,rr=cw/2-3;
   wi_oval(xx+cw/2,y+h-7,rr,6,0x655149);wi_oval(xx+cw/2,y+h/2,rr,h/2-3,wi_role==6?0xb37961:0xac755b);rect(xx+3,n,cw-6,h/2-7,0xaa775d);
   wi_oval(xx+cw/2,y+h/2+6,rr-7,max(7,h/3-5),ink);rect(xx+10,y+h/2+7,cw-20,max(2,h/3-8),ink);
   for(int j=1;j<8;j++){float a0=3.14159f+j*(3.14159f/8);int rx=(int)((rr-3)*cs(a0)),ry=(int)((h/2-7)*sn(a0));line(xx+cw/2+rx,y+h/2+ry,xx+cw/2+rx*4/5,y+h/2+ry*4/5,pale);}
   if(wi_role==6){wi_vase(xx+cw/2,y+h-13,6,11,0xd8b991);}
   else{for(int j=-1;j<=1;j++)wi_oval(xx+cw/2+j*9,y+h-15,5,3,on?0xeab569:0xb38d62);}
   rect(xx+4,y+h-9,cw-8,4,0xc59d74);}
  return;}
 if(t==WI_BICYCLE||t==WI_TELESCOPE||t==WI_WEIGHT){
  wi_oval(m+4,y+h-4,w/2-4,5,0x575950);
  if(t==WI_BICYCLE){int r=min(w/6-1,h/3),u=x+w/4,v=x+3*w/4,cy=n+4;
   for(int j=0;j<2;j++){int cc=j?v:u;for(int k=0;k<6;k++){float aa=k*.523599f;int xx=(int)((r-2)*cs(aa)),yy=(int)((r-2)*sn(aa));line(cc-xx,cy-yy,cc+xx,cy+yy,metal);}for(int k=0;k<3;k++)wi_ellipse(cc,cy,r-k,r-k,ink);}
   U c=e?0x9fc9ad:0xe4b177;for(int j=0;j<3;j++){line(u,cy+j,m-9,cy-r+j,c);line(m-9,cy-r+j,m+2,cy+3+j,c);line(m+2,cy+3+j,u,cy+j,c);line(m+2,cy+3+j,v-10,cy-r+j,c);line(v-10,cy-r+j,v,cy+j,c);}
   rect(m-17,cy-r-3,13,4,ink);rect(v-13,cy-r-4,12,3,metal);line(m,cy+4,m-5,y+h-4,ink);rect(m-13,y+h-5,17,3,ink);
  }else if(t==WI_TELESCOPE){int sign=on?-1:1;for(int k=0;k<3;k++){int foot=m+(-1+k)*23;
   for(int j=0;j<3;j++)line(m+j,n-2,foot+j,y+h-6-(k%2)*7,metal);rect(foot-3,y+h-6-(k%2)*7,7,3,ink);}
   for(int j=-6;j<7;j++)line(m-28*sign,n+10+j,m+29*sign,n-18+j,j%3?0xb8b4a4:0x718897);
   wi_oval(m+29*sign,n-18,5,8,0x445b72);rect(m-4,n-2,8,6,0xbf986c);
  }else{box(m-10,y+10,20,h-16,0x8e6d5d,ink);rect(m-7,y+12,14,h-22,0xbd9475);
   for(int a=0;a<2;a++){int yy=y+11+a*(h-27);rect(x+12,yy-1,w-24,4,metal);
    for(int j=0;j<3;j++){int size=13-j*2;rect(x+18+j*5,yy-size/2,4,size,ink);rect(x+w-23-j*5,yy-size/2,4,size,ink);}}}
  return;}
 if(t==WI_POTTERY){/* wheel, thrown pot and a loose test bowl on a kickwheel bench */
  wi_oval(m,y+h-6,w/2-4,6,0x776653);wi_oval(m,n+8,w/3,h/3,0x8b7260);wi_oval(m,n+3,w/3,h/3,0xba9b7b);
  wi_ellipse(m,n+2,w/3-5,h/3-4,0x816858);wi_vase(m,n+6,h/6,on?27:19,e==1?0x83a4ad:0xcd997e);
  wi_vase(x+w-13,y+h-9,7,11,0xaf8775);return;}
 if(t==WI_PARCELS){/* parcel conveyor instead of packages sitting on a featureless table */

  box(x,y+6,w,h-8,0x455b5e,ink);for(int u=5;u<w-5;u+=8){rect(x+u,y+11,3,h-19,0x91a7a0);rect(x+u+3,y+11,2,h-19,0x5c706f);}
  for(int j=0;j<4;j++){int ww=20+j%2*8,xx=x+12+j*(w-24)/4,yy=y+15+(j%2)*8;
   box(xx,yy,ww,17,0xb68e69,0x735a49);rect(xx+ww/2-2,yy+1,4,15,0xddc499);rect(xx+3,yy+3,5,4,pale);}
  rect(x+3,y+6,w-6,3,metal);rect(x+3,y+h-5,w-6,3,metal);return;}
 if(t==WI_BOOK||t==WI_TABLE){
  U top=wi_role==2?0xb08361:wi_role==17?(e==3?0xb47f79:0xcdad84):0x9b7758;
  int rx=w/2-3,ry=h/2-4;wi_oval(m+3,n+5,rx,ry,0x56544a);wi_oval(m,n,rx,ry,0x695c50);wi_oval(m,n-2,rx-2,ry-2,top);
  if(t==WI_BOOK){int yy=n-10;for(int j=0;j<2;j++){box(m-22+j*22,yy,22,21,pale,0x826d58);
    for(int k=0;k<4;k++)rect(m-18+j*22,yy+4+k*3,13-k%2*4,1,0xa59377);}
   if(w>100){wi_oval(x+21,n+3,7,3,0x936e55);rect(x+20,n-6,2,8,metal);wi_oval(x+21,n-7,9,4,0x557d68);}}
  else{wi_oval(m-6,n-1,12,7,0xebe0c7);wi_oval(m-6,n-1,8,4,0xb9c4b9);
   wi_oval(m+13,n-3,4,4,0xe4d0b0);wi_oval(m+13,n-3,2,2,0x705b52);line(m-22,n-7,m-22,n+5,metal);}
  return;}
 if(t==WI_RADIO){/* stepped console, stereo meter bridge, fader channels */
  box(x+3,y+10,w-6,h-10,0x424a59,ink);box(x+9,y+2,w-18,h/3+5,0x6c7377,ink);
  for(int i=0;i<4;i++){int xx=x+15+i*(w-28)/4;box(xx,y+6,(w-35)/4,9,0x273e43,ink);
   for(int j=2;j<(w-40)/4;j+=3)rect(xx+j,y+9+(j%2),2,3+(j%3),j<17?0xa1cc94:0xe1ae74);}
  for(int i=0;i<11;i++){int xx=x+12+i*(w-22)/11,yy=y+h/2;
   rect(xx,yy,2,h/2-7,0xa4aca8);rect(xx-2,yy+2+(i*7+on*3)%max(3,h/2-13),7,4,0xdbc3a1);}
  return;}
 if(t==WI_FABRIC){box(x+2,y+4,w-3,h-6,0x7a604e,ink);box(x+4,y+5,w-8,h-12,0xac9678,0x6d5c4d);
  for(int j=0;j<4;j++){int xx=x+11+j*9;rect(xx,y+10,7,h-22,(U[]){0x718388,0xb78279,0x8b7a94,0xb5aa82}[(e+j)%4]);wi_oval(xx+3,y+10,4,3,pale);}
  int xx=m-13;U cloth=on?0xdacbb1:0xb2bfad;tri(xx,n-18,xx+34,n-12,xx+17,n+13,cloth);tri(xx,n-18,xx+17,n+13,xx-5,n+17,cloth);
  for(int j=0;j<24;j+=4)rect(xx+j,n-13+j/6,2,1,0x766b60);
  wi_ellipse(x+w-32,n+10,4,4,ink);wi_ellipse(x+w-23,n+12,4,4,ink);line(x+w-29,n+8,x+w-17,n-8,metal);line(x+w-21,n+8,x+w-33,n-6,metal);
  return;}
 if(t==WI_PRINT){rect(x+6,y+7,w-10,h-8,0x4e5958);box(x+2,y+8,w-4,h-13,0x657471,ink);
  for(int j=0;j<3;j++){int xx=x+9+j*w/8;box(xx,y+7,w/10,h-15,0x9eaaa3,ink);rect(xx+3,y+8,4,h-17,0xbac0aa);}
  rect(m+6,y+11,w/2-19,h-22,pale);tri(m+9,y+h-16,m+w/5,n-10,x+w-20,y+h-16,0x5b8885);if(on||e==1)disc(x+w-29,y+22,6,0xc37c68);
  wi_ellipse(x+8,n,11,11,ink);line(x-2,n,x+17,n,metal);return;}
 if(t==WI_KEYS){box(x,y+2,w,h-5,0x906f57,0x584b43);box(x+5,y+6,w-10,h-14,0x706153,pale);
  for(int j=0;j<2;j++)for(int k=0;k<7;k++){int xx=x+14+k*(w-21)/7,yy=y+14+j*max(10,h/3);
   rect(xx-3,yy-5,8,2,0xc4ac89);wi_ellipse(xx,yy+1,2,2,pale);line(xx,yy+3,xx,yy+8,0xe2c284);rect(xx,yy+6,4,2,0xe2c284);}
  return;}
 if(t==INT_DINER){int topH=h*2/3;box(x,y,w,topH,wi_role==2?0xc39b70:0xaeb9af,ink);rect(x+2,y+2,w-4,3,pale);
  if(wi_role==2){for(int q=0;q<max(4,w/22);q++){int xx=x+12+q*21,yy=y+topH/2;
   wi_oval(xx,yy,8,5,e==2?0xce9677:0xd7ac72);line(xx-4,yy-2,xx-1,yy-4,pale);line(xx+1,yy-1,xx+4,yy-3,pale);}
   rect(x,y+topH-5,w,2,0x93b4b3);}
  else{for(int q=0;q<4;q++){int xx=x+18+q*(w-20)/4;wi_oval(xx,y+topH/2,7,5,0xe2d8bc);wi_oval(xx,y+h-7,10,5,0x71585a);wi_oval(xx,y+h-9,10,5,0xac776f);}}
  return;}
 wi_draw(g,on);
}
