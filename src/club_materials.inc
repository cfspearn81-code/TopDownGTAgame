/* Afterimage scene materials. Coordinates are in the club's original room
   space. Atlas rows 0..111 are upright scenery, never projected on the floor.
   The rest is an actor-free ground map shared by both cameras. */
static U ncRoomMaterial[NC_OFFICE+1][W*H];
static U *ncGround=ncRoomMaterial[0];
static unsigned ncMaterialReady;
static int nc_abs(int a){return a<0?-a:a;}
static U nc_tone(U c,int n){
 return RGB(cl((int)(c>>16&255)+n,0,255),cl((int)(c>>8&255)+n,0,255),cl((int)(c&255)+n,0,255));
}
static U nc_grain(int x,int y){
 U a=(U)x*2246822519u^(U)y*3266489917u;a=(a^(a>>15))*668265263u;return a^(a>>13);
}
static U nc_velvet(int x,int y,U dye){
 int fold=x%17,g=(int)(nc_grain(x/2,y/2)%5)-2;
 int fac=fold<3?-11:fold<10?fold-3:16-fold;
 return nc_tone(dye,fac+g+(y%3==0?2:0));
}
static U nc_walnut(int x,int y,U dye){
 int streak=(int)(4*sn(x*.083f+y*.024f)+3*sn(y*.31f+x*.054f));
 return nc_tone(dye,streak+(y%8==1?3:0)+(int)(nc_grain(x/5,y)%4)-2);
}
static U nc_marble(int x,int y,U dye,int gleam){
 U grain=nc_grain(x/3,y/3),speck=nc_grain(x+32,y);
 int vein=(int)(x*.45f-y*.61f+9.f*sn(y*.104f)+6.f*sn(x*.075f)+36.f*sn(y*.021f+x*.024f));
 int p=nc_abs(vein)%52,delta=nc_abs(26-p);
 U v=nc_tone(dye,(int)(grain%6)-3);
 if(delta<=1)v=nw_blend(v,0x8e859b,gleam?54:32);
 else if(delta<=3)v=nw_blend(v,0x597176,gleam?30:19);
 if(gleam&&((x+y*3)%17)<3)v=nc_tone(v,3);
 if(speck%41==6)v=nc_tone(v,gleam?6:3);
 return v;
}
static U nc_brass(int x,int y,U dye){
 int adj=(int)(nc_grain(x/2,y)%6)-3;
 return nc_tone(dye,adj+(x%7==3?11:0)+(y%6==1?9:0));
}
static U nc_rug(int x,int y,U dye,U trim){
 int w=nc_abs((x%28)-14),h=nc_abs((y%18)-9),k=w+2*h;
 U c=nc_tone(dye,(int)(nc_grain(x,y)%9)-4+(y&1?2:-1));
 if(k==12||k==13)c=nw_blend(c,trim,57);
 if(k<4)c=nw_blend(c,trim,37);
 return c;
}
static U nc_glass(int x,int y,U dye){
 int diag=(x-2*y+4096)%29;
 return nw_blend(nc_tone(dye,(int)(nc_grain(x,y)%5)-2),0xc3d2e5,
                  diag<2?55:diag<7?19:10);
}
enum { NC_M_VELVET,NC_M_WOOD,NC_M_MARBLE,NC_M_BRASS,NC_M_RUG,NC_M_GLASS };
static U nc_sample(int kind,int x,int y,U c){
 switch(kind){
 case NC_M_VELVET:return nc_velvet(x,y,c);
 case NC_M_WOOD:return nc_walnut(x,y,c);
 case NC_M_MARBLE:return nc_marble(x,y,c,1);
 case NC_M_BRASS:return nc_brass(x,y,c);
 case NC_M_RUG:return nc_rug(x,y,c,mix(c,PAPER,3));
 default:return nc_glass(x,y,c);
 }
}
static void nc_fill(int x,int y,int w,int h,int mat,U c){
 for(int j=max(y,clipTop);j<min(y+h,min(rh,clipBottom));j++){
  U*row=surface+j*rw;
  for(int i=max(x,clipLeft);i<min(x+w,min(rw,clipRight));i++)row[i]=nc_sample(mat,i-x,j-y,c);
 }
}
static U nc_ground_tex(int room,int x,int y){
 U c;
 if(room==NC_FLOOR){
  int xr=(x+(y/38&1)*21)%46,yr=y%38;
  c=nc_marble(x,y,0x282332,1);
  if(xr<1||yr<1)c=0x1c1e2e;
  else if(xr==1||yr==1)c=nw_blend(c,0x656277,30);
  if(x>=275&&x<685&&y>=255&&y<373){
   int border=min(min(x-275,684-x),min(y-255,372-y));
   if(border==0||border==4)return nc_brass(x,y,0x9e7860);
   if(border<5)return nc_marble(x,y,0x171b2d,1);
   int u=(x+2*y+25)/52,v=(x-2*y+2048)/52;
   int a=(x+2*y+25)%52,b=(x-2*y+2048)%52;
   c=nc_marble(x,y,(u+v)&1?0x34354e:0x22283e,1);
   if(a<2||b<2)c=nw_blend(c,0x735d78,67);
   else if(a>49||b>49)c=nw_blend(c,0x89798d,31);
   int dx=nc_abs(x-480),dy=nc_abs(y-314),diamond=dx+dy*2;
   if(diamond==34||diamond==36)c=nw_blend(c,ncGold,75);
   if(diamond<31&&(dx==dy*2||dx<2||dy<2))c=nw_blend(c,ncGold,55);
  }
  if(y>=392&&y<424&&x>=411&&x<=549){
   if(x<416||x>544||y==395)c=nc_brass(x,y,0x8f765f);
   else c=nc_rug(x,y,0x442d49,0xad7698);
  }
 }else if(room==NC_SUITE){
  int lane=x<344?0:x<609?1:2,lx=x-(91+lane*266);
  c=nc_walnut(x,y,0x412d39);
  if(y%22==0||((x+(y/22%2)*42)%86==0))c=nw_blend(c,0x171829,100);
  if(lx>=11&&lx<=221&&y>=201&&y<283){
   int border=min(lx-11,221-lx);
   c=nc_rug(x,y,mix(ncBoothUph[lane],0x211a2d,4),ncBoothInk[lane]);
   if(border==3||y==279)c=nw_blend(c,ncBoothInk[lane],91);
  }else if(x>=286&&x<675&&y>=292&&y<371){
   int z=min(min(x-286,674-x),min(y-292,370-y));
   c=nc_rug(x,y,0x492f49,0xb78395);
   if(z==2||z==6)c=nw_blend(c,ncGold,67);
  }
 }else{
  int a=(x+2*y)%41,b=(x-2*y+2048)%41;
  c=nc_walnut(x,y,((a/10+b/14)&1)?0x3f343a:0x49383a);
  if(!a||!b)c=nw_blend(c,0x161b27,77);
  if(x>=278&&x<690&&y>=283&&y<390){
   int z=min(min(x-278,689-x),min(y-283,389-y));
   c=nc_rug(x,y,0x2f4051,0x9ba5a7);if(z==3||z==6)c=nw_blend(c,ncGold,61);
  }
 }
 int edge=min(min(x-74,885-x),min(y-114,435-y));
 if(edge>=0&&edge<13){
  if(edge==1||edge==7||edge==11)c=nw_blend(c,ncGold,edge==7?91:42);
  else c=nw_blend(c,0x141a2b,61);
 }
 /* Static fall-off keeps the furniture separated from the ground, rather
    than using one uniform colored screen overlay. */
 int dark=cl((y-355)/4+nc_abs(x-480)/25,0,35);
 return nw_blend(c,0x141321,dark);
}
static void nc_build_materials(int room){
 /* Materials are lazily baked once per room. Decor upgrades rebuild only
    furniture and trim, and returning to a room reuses its textured ground
    and wall samples. Never use session RNG or write a career. */
 room=cl(room,NC_FLOOR,NC_OFFICE);ncGround=ncRoomMaterial[room];
 if(ncMaterialReady&(1u<<room))return;
 ncMaterialReady|=1u<<room;
 for(int y=0;y<H;y++)for(int x=0;x<W;x++){
  int bx=(x-76+19)/135;
  U wall=room==NC_FLOOR?0x362844:room==NC_SUITE?0x3b2c45:0x33414f;
  U c=y<112?nc_velvet(x,y,wall):0;
  if(y<112){
   int b=(x+31)%135;
   if(b<6)c=nc_brass(x,y,0xb29a7e);
   else if(y>74)c=nc_walnut(x,y,0x493743);
   else if(y==8||y==10||y==71||y==73)c=nc_brass(x,y,0x74677c);
   else if(y%19==0&&b>12&&b<122)c=nw_blend(c,ncRose,24+(bx&1)*5);
  }else if(x>=73&&x<=887&&y<=441)c=nc_ground_tex(room,x,y);
  else c=nw_blend(room==NC_FLOOR?0x261d32:room==NC_SUITE?0x2b2037:0x25323b,
                   0x131623,cl(y-300,0,117));
  ncGround[y*W+x]=c;
 }
}
static void nc_cloth_pads(int x,int y,int w,int h,U dye){
 nc_fill(x,y,w,h,NC_M_VELVET,dye);
 for(int j=4;j<h-1;j+=8)for(int i=4+(j%16?4:0);i<w-4;i+=8){
  dot(x+i,y+j,nw_blend(dye,0x21182b,131));dot(x+i+1,y+j-1,nw_blend(dye,ncRose,47));
 }
}
