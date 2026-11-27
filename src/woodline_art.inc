/* Small layered props share the usual software-rendered y-sort with the
 * walker. Only the collision footprint, not the canopy, blocks passage. */
static void woodline_draw_prop(const WoodProp*p,int x,int y){
 const U green[]={0x456f61,0x507656,0x5c7c5d,0x647f61,0x395f55,0x567a6e};
 int v=p->grade%6,r=p->radius; U c=green[v];
 int dark=(int)(nw_dark()*4.f);
 c=mix(c,0x1e2f31,dark);
 switch(p->kind){
 case WOOD_TREE:{
  int h=p->height,j=v%3;
  rect(x-2,y-h/2,5,h/2+4,mix(0x795f47,0x283834,dark));
  line(x+1,y-2,x+1,y-h/2,0x8d7856);
  if(v%3!=1){
   int reach=h/3;
   for(int q=0;q<3;q++){
    int yy=y-6-(q+1)*h/6,w=reach-q*3;
    tri(x-w,yy,x+w,yy,x+(q&1)*3-1,yy-h/3,mix(c,0x193f3a,2));
    tri(x-w+2,yy-1,x+1,yy+3,x-1,yy-h/3+3,mix(c,0xc6bd77,1));
    line(x-w+4,yy-1,x+3,yy-3,mix(c,0x83a891,2));
   }
  }else{
   disc(x-h/6,y-h/2,h/4,mix(c,0x192e2d,2));disc(x+h/5,y-h/2+3,h/4,mix(c,0x254843,2));
   disc(x+1,y-h/2-9,h/4+1,c);disc(x-4,y-h/2-12,h/6,mix(c,0xa6a278,2));
   for(int q=0;q<7;q++){int u=x-h/6+(q*9)%max(3,h/2),yy=y-h/2-9+(q*7)%13;rect(u,yy,2+j,1,mix(c,0xbda981,2));}
  }
  break;}
 case WOOD_ROCK:{
  int h=5+p->height/2;U a=mix(0x78867a,0x273a42,dark),b=mix(a,0x2a3c3c,3);
  tri(x-r-4,y+2,x+r+6,y+2,x-2,y-h-4,b);
  tri(x-r+2,y-2,x+r-3,y-5,x-2,y-h-4,a);
  tri(x-3,y+2,x+r+6,y+2,x+r-3,y-5,b);
  line(x-r+4,y-2,x-2,y-h-3,mix(a,0xaeac93,2));dot(x+2,y-4,0x698b70);
  break;}
 case WOOD_STUMP:
  rect(x-r,y-7,2*r,9,0x654c3a);disc(x,y-7,r,0xa88a62);disc(x,y-7,max(1,r-3),0x816b52);dot(x+1,y-7,0xa08866);break;
 case WOOD_UNDERGROWTH:{
  U tip=mix(c,0xb4ab6b,2);
  for(int j=-2;j<=2;j++){
   int h=p->height-absf(j),xx=x+3*j;
   line(x,y,xx,y-h,c);line(xx,y-h,xx+j,y-h+3,tip);dot(xx+1,y-h+2,tip);
  }
  if(v==3){dot(x-3,y-8,0xd4aa75);dot(x+4,y-8,0xa88c62);}
  break;}
 case WOOD_POST:
  rect(x-2,y-15,4,17,0x6a583d);box(x-9,y-17,19,8,0x746b4b,0xa39167);line(x-5,y-13,x+4,y-13,0xcedd9a);dot(x+3,y-15,0xcedd9a);break;
 case WOOD_CRATE:
  box(x-10,y-12,22,14,0x5f5c46,0x898364);line(x-7,y-9,x+8,y-1,0x7d7859);rect(x-1,y-11,4,11,0x535d53);break;
 case WOOD_LOGPROP:
  for(int i=-r;i<=r;i++){int yy=y+(i*5)/r;disc(x+i,yy-3,6,0x504735);line(x+i,yy-6,x+i+4,yy-4,0x74644b);}
  disc(x+r,y+2,5,0x9b8061);disc(x+r,y+2,2,0x6f604c);break;
 case WOOD_HUT:
  box(x-18,y-24,37,29,0x55544b,0x827960);rect(x-14,y-13,9,8,0xa3aa87);
  rect(x-17,y-22,35,2,0x8f7d64);
  tri(x-22,y-20,x+22,y-20,x,y-34,0x374840);line(x-22,y-20,x,y-34,0x7f8c6b);
  rect(x-18,y+5,35,2,0x8d8065);oc_paint_exterior_door(oc_placed_door(OC_WOOD,(int)(p-woodProps)),cx,cy);break;
 }
}
