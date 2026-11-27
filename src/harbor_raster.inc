/* Screen-space harbor detail; relies on the water/land mask used by boats.
   Weather movement never changes collision, pixels cover only real sea. */
static void harbor_paint_world(void){
 if(cx+W<2310||cx>ROOM-36||cy+H<2010||cy>2790)return;
 static const short buoys[][2]={{2913,2257},{2906,2316},{2872,2354},{2796,2291},
   {2781,2347},{2708,2432},{2615,2480},{2517,2571},{2448,2585},{3018,2209}};
 const U teal=0x83bab2,amber=0xe5b580;
 for(int i=0;i<LEN(buoys);i++){
  int wx=buoys[i][0],wy=buoys[i][1];
  if(!harbor_nav_water(wx,wy,3))continue;
  int x=sx(wx),y=sy(wy)+(int)(sn(anim*1.5f+i)*2);
  if(x<-12||x>W+12||y<-12||y>H+12)continue;
  U lens=i%2?amber:teal;
  line(x-8,y+3,x+9,y+3,0x344e5c);
  rect(x-3,y-2,7,4,0x354350);rect(x-2,y-6,5,4,0x7b7d6a);
  dot(x,y-7,lens);dot(x+1,y-7,lens);
  if(fastGfx)continue;
  for(int j=2;j<18;j+=2){
   int u=x+(int)(sn(anim*1.2f+j*.8f+i)*2),v=y+j;
   if((unsigned)u>=W||(unsigned)v>=H||!harbor_ocean_cell(u+cx,v+cy))continue;
   dot(u,v,mix(pixels[v*W+u],lens,j>10?1:2));
  }
 }
 static const struct {short x,y;const char*name;U ink;} signs[]={
  {2900,2017,"LANTERN QUAY",0x9edfc9},{2757,2132,"FREIGHT BERTH",0xe0ad84},{2432,2414,"TIDE SIGNAL PIER",0xa6bbd9}
 };
 for(int i=0;i<LEN(signs);i++){
  const char*name=signs[i].name;int x=sx(signs[i].x),y=sy(signs[i].y),w=textw(name,0)+14;
  if(x+w/2<0||x-w/2>=W||y<91||y>H-72)continue;
  box(x-w/2,y,w,22,0x192833,mix(signs[i].ink,0x374750,2));center(x,y,name,signs[i].ink,0);
  rect(x+w/2-9,y+21,2,5,0x889294);rect(x-w/2+7,y+21,2,5,0x889294);
 }
 if(fastGfx)return;
 /* Wakelets are world-sampled; camera moves do not slide foam along roads. */
 int time=(int)(anim*13.f);
 for(int j=0;j<24;j++){
  int wx=2400+(j*83)%613,wy=2150+(j*41)%461;
  wx+=(time+j*7)%22-11;
  for(int k=0;k<7+j%4;k++){
   int x=sx(wx+k),y=sy(wy+(k+k%3)/6);
   if((unsigned)x>=W||(unsigned)y>=H||!harbor_ocean_cell(x+cx,y+cy))continue;
   dot(x,y,mix(pixels[y*W+x],0x9bb5aa,k%4?1:2));
  }
 }
}
