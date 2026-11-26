/* Vault 39 art consumes the same InteriorProp plan as walking and staff
 * actions. Everything but four muted light steps and the DJ is cached.
 * There are no new walls, hotspots, global FX, RNG reads or save fields. */
static const U v39Neon[]={0xf065a9,0x6fbbf8,0x80e1db,0xe2a374};
static const U v39Brass=0xb99068u,v39Shadow=0x171522u;
static void v39_octagon(int x,int y,int rx,int ry,int cut,U c){
 for(int j=-ry;j<=ry;j++){int indent=max(0,cut-(ry-(j<0?-j:j)));rect(x-rx+indent,y+j,2*(rx-indent)+1,1,c);}
}
static void v39_stage_glass(void){
 const int x=419,y=294,rx=109,ry=65;
 v39_octagon(x+3,y+5,rx+6,ry+5,21,0x15121f);
 v39_octagon(x,y,rx+3,ry+2,20,0x906779);
 v39_octagon(x,y,rx,ry,19,0x31263d);
 v39_octagon(x,y,rx-4,ry-4,16,0x53405a);
 /* Black grouted tile in a circular spill, not a different walk plane. */
 for(int j=0;j<12;j++)for(int i=0;i<17;i++){
  const int px=x-102+i*12,py=y-60+j*10;
  if((j==0||j==11)&&(i<2||i>14))continue;
  if((j==1||j==10)&&(i==0||i==16))continue;
  const int d=absf((float)(i-8))*2+absf((float)(j-6))*3;
  const U hue=v39Neon[(i/5+(j/4+1))%4];
  const U bg=mix(0x252234,hue,max(1,5-d/8));
  rect(px,py,11,9,bg);
  rect(px+1,py,9,1,mix(bg,hue,3));
  if((i+j)%5==2)dot(px+8,py+6,mix(bg,PAPER,2));
 }
 for(int i=0;i<7;i++){
  const int px=x-90+i*30;
  rect(px,y+ry-2,3,2,i%2?0xb17ca7:0xde927b);
  rect(px,y-ry+3,3,2,i%2?0xad94bd:0xabb5c5);
 }
 /* Brass threshold leads down from the floor into the entry runner. */
 rect(x-31,y+ry+2,62,2,0x6e5262);rect(x-27,y+ry+5,54,2,0x533c5a);
}
static void v39_queue_inlay(int x,int y,int w){
 for(int j=0;j<w;j++){int z=4*j*(w-j)/max(1,w*w/5);rect(x+j,y+z,1,1,0x79566a);}
 for(int j=0;j<2;j++){ns_oval(x+j*w,y,4,2,0x292132);ns_oval(x+j*w,y,2,1,0xad8070);}
}
static void v39_sconce(int x,int y,U light,int l){
 rect(x-6,y-9,12,13,0x171925);
 box(x-5,y-8,10,11,0x322b40,0x775366);rect(x-2,y-6,4,6,light);
 if(l)for(int j=0;j<4;j++)rect(x-3-j,y+4+j,7+j*2,1,mix(0x35233c,light,4-j));
}
static void v39_exit_sign(int x,int y){
 box(x-24,y,48,14,0x232e31,0x638c79);
 /* Tiny EXIT is etched directly rather than using the 10px HUD font. */
 static const unsigned char letters[4][5]={{7,4,6,4,7},{5,5,2,5,5},{7,2,2,2,7},{7,2,2,2,2}};
 for(int i=0;i<4;i++)for(int j=0;j<5;j++)for(int k=0;k<3;k++)
  if(letters[i][j]&(4>>k))rect(x-15+i*8+k*2,y+2+j*2,2,2,0x94c4a3);
}
static void v39_staff_steel(void){
 const int x=V39_STAFF_X,y=117;
 box(x-21,y-3,43,49,0x292733,0x4d4659);
 box(x-18,y,37,44,0x423948,0x211c27);
 for(int j=2;j<40;j++){U c=j<24?0x51434a:0x3a3242;rect(x-16,y+j,33,1,mix(c,0x222432,j%7==0?2:0));}
 rect(x-16,y+2,2,39,0x70616a);rect(x+15,y+2,2,40,0x312737);
 /* A quiet, neutral metal plate. The context label is owned by entry code. */
 rect(x-13,y+6,25,7,0xaca392);rect(x-11,y+8,20,1,0x5c5553);
 rect(x-11,y+10,14,1,0x64574f);
 rect(x+7,y+25,7,2,0xad9174);dot(x+10,y+30,0x0e151e);
 rect(x-21,y+44,44,2,0x6b5754);rect(x-17,y+46,36,2,0x252529);
}
static void v39_label(int x,int y,const char*s,U ink){
 int w=textw(s,0)+14;box(x-w/2,y,w,21,0x211e2d,mix(ink,0x443b50,5));center(x,y,s,ink,0);
}
static void vault39_backdrop(int rear){
 const NsFinish floor=rear?(NsFinish){0x53413c,0x312f39,0x805d55,NS_WOOD}
                            :(NsFinish){0x30273d,0x221e2d,0x54425b,NS_CARPET};
 for(int y=145;y<433;y++)for(int x=venue_limit(y,0)+4;x<venue_limit(y,1)-4;x++)
  dot(x,y,ns_floor_pixel(x,y,&floor));
 const int l=venue_limit(142,0)+8,r=venue_limit(142,1)-7;
 if(!rear){
  ns_bricks(l,102,r-l,37,0x281f2c,0x473043,0x522e46);
  rect(l,137,r-l,5,0x4b3653);rect(l,139,r-l,1,0x93657b);
  for(int x=l+12;x<r-65;x+=23){rect(x,106,2,29,0x523248);rect(x+2,108,5,25,0x382838);}
  for(int x=l+24,n=0;x<r-65;x+=68,n++)v39_sconce(x,128,v39Neon[n%4],n%2);
  /* Behind the DJ: backlit identity and suspended dark blue truss. */
  box(240,102,121,29,0x25213a,0xbc7696);center(299,104,"VAULT 39",0xf10280,0);center(300,102,"VAULT 39",0xffadd4,0);
  rect(206,145,187,74,0x231e31);rect(206,212,187,6,0x573447);rect(209,214,181,1,0xa46386);
  for(int x=211;x<389;x+=14){
   line(x,149,x+11,157,0x74576b);line(x+11,149,x,157,0x544457);
   rect(x+2,150,6,2,v39Neon[(x/14)%4]);
  }
  /* Soft asymmetric pools, baked once. Colors are kept below text/chrome. */
  int_lamp(394,262,101,0x855293,3);int_lamp(482,316,82,0x555e9b,3);
  int_lamp(608,240,100,0x8b5a5c,3);int_lamp(264,312,75,0x613d81,2);
  v39_stage_glass();
  v39_queue_inlay(372,378,31);v39_queue_inlay(550,378,38);
  /* Rounded floor lighting directs traffic around the club's activity
   * instead of making patrons stand on a new invisible obstruction. */
  for(int j=0;j<5;j++){int yy=385+j*8;
   ns_oval(446-j*5,yy,2,1,0x9b6578);ns_oval(526+j*3,yy,2,1,0x916778);
  }
  v39_staff_steel();
 }else{
  ns_bricks(l,102,r-l,36,0x29222d,0x514143,0x514650);
  rect(l,136,r-l,4,0x3d353e);rect(l,136,r-l,1,0x836250);
  for(int i=0;i<3;i++)v39_sconce(l+28+i*149,124,i==1?0xd5a16e:0x91a9a1,1);
  /* Two tone glass-panel lamps, municipal tide map under security mesh. */
  ns_pipe(l+92,105,r-l-111,0,4,0x5c6e6c);
  box(372,110,183,32,0x2c3943,0x92908a);
  for(int i=0;i<14;i++){int x=376+i*12;
   line(x,112,x,139,0x455059);line(374,114+i%5*6,550,114+i%5*6,0x3f4a53);}
  for(int i=0;i<12;i++){int x=377+i*14,y=126-(i%4)*2;line(x,y,x+13,121+((i+1)%4)*2,0x9ab1aa);dot(x+1,y+2,0xc79168);}
  for(int i=0;i<4;i++)disc(406+i*33,121+i%2*5,2,0xbf9b76);
  ns_rug(348,271,264,89,0x2e292f,0x54434a,0x86685b);
  int_lamp(472,235,105,0xae7546,4);int_lamp(655,284,79,0x4a6f73,3);
  box(302,365,28,17,0x353131,0x5f5252);
  for(int i=0;i<6;i++)rect(305+i*4,368,1,10,0x6e5f57);
  v39_exit_sign(480,400);
 }
}
static void v39_leather(const InteriorProp*f,int rear){
 int x=f->x,y=f->y,w=f->w,h=f->h;
 U leather=mix(f->tint,rear?0x473136:0x602d58,4),rim=mix(leather,0xc2898e,3);
 rect(x+3,y+5,w+2,h+1,0x241c28);
 box(x,y,w,h,leather,0x241e2b);
 rect(x+3,y+2,w-6,6,rim);rect(x+5,y+9,w-10,5,mix(leather,DARK,2));
 for(int i=8;i<w-9;i+=17){
  const int n=min(16,w-8-i);
  box(x+i,y+15,n,h-18,mix(leather,0xb78a88,(i/17)%3+1),0x493047);
  rect(x+i+2,y+16,n-4,2,rim);dot(x+i+n/2,y+21,0x402c3c);
 }
 rect(x+3,y+7,6,h-10,leather);rect(x+w-9,y+7,6,h-10,rim);
 for(int i=7;i<w-6;i+=6)dot(x+i,y+h-3,0xbb8c68);
}
static void v39_platter(int x,int y,int stud,U light){
 disc(x+1,y+2,12,0x211b2a);disc(x,y,12,0x9d8793);disc(x,y,10,0x1d202f);
 ns_ring(x,y,8,8,1,0x51445a);ns_ring(x,y,6,6,1,0x4d3d4d);
 disc(x,y,4,light);dot(x,y,PAPER);
 line(x+12,y-10,x+10,y-3,0xc9b6a2);line(x+10,y-3,x+5,y+2,0xb1a5a1);
 dot(x+(int)(cs(stud*.2f)*9),y+(int)(sn(stud*.2f)*9),0xa991a3);
}
static void v39_speaker(int x,int y,int w,int h){
 box(x,y,w,h,0x242431,0x121523);rect(x+2,y+1,1,h-3,0x856a90);
 disc(x+w/2,y+h/2+4,min(8,w/2-2),0x181928);
 disc(x+w/2,y+h/2+4,min(5,w/2-4),0x4b4056);disc(x+w/2,y+5,2,0x9b7495);
}
static void v39_decks(const InteriorProp*f){
 const int x=f->x,y=f->y,w=f->w,h=f->h;
 int_solid_shape(x,y,w,h,0x504157);
 box(x+5,y+5,w-10,37,0x34283c,0x6b4c65);
 rect(x+6,y+h-9,w-12,5,0x2d253a);
 v39_speaker(x+7,y+9,21,29);v39_speaker(x+w-28,y+9,21,29);
 v39_platter(x+52,y+22,0,0xe871ab);v39_platter(x+w-50,y+22,12,0x8bd4cb);
 for(int i=0;i<4;i++){int u=x+w/2-15+i*7;
  rect(u,y+9,2,21,0x1c182d);rect(u-2,y+12+i*4,6,3,0xa384a0);
 }
 for(int i=0;i<7;i++)rect(x+w/2-17+i*5,y+34,3,2,i<5?0x65a69c:0xc2878f);
}
static void v39_bar(const InteriorProp*f){
 const int x=f->x,y=f->y,w=f->w,h=f->h;
 rect(x+4,y+5,w+1,h,0x261c26);
 box(x,y,w,h,0x48303a,0x201e22);
 box(x+2,y+1,w-4,25,0x65433c,0xac7d60);
 rect(x+4,y+3,w-8,3,0x7f574e);rect(x+4,y+h-8,w-8,3,v39Brass);
 for(int i=6;i<w-6;i+=5){
  dot(x+i,y+h-11,i%3?0x996269:0xca9567);
  int o=(i*37)%10;line(x+i,y+7+o,x+i+4,y+7+o,0x795044);
 }
 for(int i=0;i<2;i++){
  int u=x+15+i*(w-30);
  box(u-5,y+8,10,7,0x4e4e55,0x9d8c7f);line(u,y+5,u,y+12,0xe0bc83);
  line(u,y+5,u+3,y+5,0xb99c79);dot(u+3,y+7,0xcbb292);
 }
}
static void v39_bottles(const InteriorProp*f){
 const int x=f->x,y=f->y,w=f->w,h=f->h;
 box(x,y,w,h,0x2e293a,0x5d4059);rect(x+2,y+2,w-4,11,0x623b57);
 for(int i=8;i<w-5;i+=9){
  int u=x+i,ht=7+i%5;U a=mix(v39Neon[(i/9)%4],0x6f4f5a,2);
  rect(u,y+h-6-ht,5,ht,mix(a,0x40384f,3));
  rect(u+1,y+h-9-ht,3,4,a);rect(u+1,y+h-9,4,2,0xdda884);dot(u,y+h-11,a);
 }
 rect(x+2,y+h-5,w-4,2,0xa97b78);rect(x+2,y+h-2,w-4,1,0x4c314a);
}
static void v39_hideout_desk(const InteriorProp*f){
 const int x=f->x,y=f->y,w=f->w,h=f->h;
 int_solid_shape(x,y,w,h,0x795044);
 box(x+4,y+4,w-8,h-11,0x61493f,0xab8266);
 for(int j=7;j<h-8;j+=4)rect(x+5,y+j,w-10,1,0x73564b);
 /* Annotated dock route, ledger and still-wired dispatch receiver. */
 box(x+40,y+9,49,18,0xc2ac90,0x322f34);
 line(x+44,y+22,x+51,y+13,0x635273);line(x+51,y+13,x+70,y+16,0x635273);line(x+70,y+16,x+81,y+12,0x635273);
 for(int i=0;i<4;i++)disc(x+48+i*11,y+21-(i%3)*3,1,0x98634e);
 box(x+8,y+10,20,15,0x445360,0x353441);for(int i=0;i<4;i++)rect(x+11+i*4,y+15,2,7,0x8e9b9a);
 line(x+16,y+11,x+20,y+4,0x998d81);
 box(x+w-36,y+9,25,16,0x6a554f,0x3f3035);rect(x+w-32,y+11,19,11,0xaa937d);
 for(int j=0;j<3;j++)rect(x+w-30,y+13+j*3,14,1,0x7b654f);
 rect(x+4,y+h-5,w-8,2,0xb4896e);
}
static int vault39_prop(const InteriorProp*f,int rear){
 if(f->type==INT_SOFA){v39_leather(f,rear);return 1;}
 if(f->type==INT_DINER){if(rear)v39_hideout_desk(f);else v39_bar(f);return 1;}
 if(f->type==INT_FEATURE&&!rear){v39_decks(f);return 1;}
 if(f->type==INT_RACK&&!rear){v39_bottles(f);return 1;}
 if(f->type==INT_REEL&&!rear){v39_speaker(f->x,f->y,f->w,f->h);return 1;}
 if(f->type==INT_WALL&&!rear){box(f->x,f->y,f->w,f->h,0x3c3246,0x6a526b);rect(f->x+2,f->y,2,f->h,0x825e74);return 1;}
 return 0;
}
static void vault39_overlay(int rear){
 if(rear)return;
 /* Physical backbar and foyer signage. The staff door gets no spotlight,
  * marker or hover label from this art layer. */
 v39_label(599,106,"BAR",0xe6b997);
 v39_label(313,223,"DJ",0xda99bd);
 v39_exit_sign(480,407);
}
static void vault39_live(int rear,int dx,int dy){
 if(rear)return;
 const int phase=v39.tick/28;
 if(fastGfx)return;
 for(int j=0;j<5;j++)for(int i=0;i<8;i++){
  int t=(i+j+phase)%8;
  if(t>2)continue;
  int x=331+i*24-dx,y=247+j*22-dy;
  U c=mix(0x68546a,v39Neon[(i/2+j)%4],t==0?4:2);
  rect(x,y,11,2,c);dot(x+1,y+2,mix(c,PAPER,2));
 }
 for(int i=0;i<7;i++){
  int u=283+i*5-dx,v=191-dy;
  if((phase+i)%7<5)rect(u,v,3,2,(i<5?0x91c5b6:0xe9a28f));
 }
}
