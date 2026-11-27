/* Local 3D art for The Afterimage. All surface calls use Room Lens depth; no
   room collision rectangles, interaction coordinates, or global shaders move. */
static void ir_nc_shell(U accent){
 U wall=ncRoom==NC_FLOOR?0x473146:ncRoom==NC_SUITE?0x45344c:0x384552;
 ir_hplane(74,137,887,435,0,0,1);
 ir_side(74,137,435,0,27,mix(wall,accent,1));
 ir_side(887,137,435,0,27,mix(wall,accent,1));
 ir_front(74,887,137,0,63,wall,74,813,4,96,ncGround);
 ir_front(74,887,137.2f,62,64,nw_blend(wall,accent,151),0,0,0,0,0);
 ir_front(74,887,137.2f,10,12,mix(wall,ncGold,4),0,0,0,0,0);
 for(int i=0;i<7;i++){
  float x=101+i*126;
  ir_box(x,137,6,7,0,61,0x73586a,0);
  ir_box(x+1,143,3,2,30,17,mix(accent,ncGold,2),0);
 }
 ir_front(75,886,435,-11,0,0x453549,0,0,0,0,0);
 ir_front(75,886,435.22f,-2,0,0xa3757b,0,0,0,0,0);
 for(int i=0;i<4;i++){
  int y=436+i*6;float z0=-2-i*3;
  ir_box(446-i*3,y,68+i*6,6,z0-3,3,0x412c44,0);
  ir_front(447-i*3,513+i*3,y+6.1f,z0-2,z0-1,accent,0,0,0,0,0);
 }
}
static void ir_nc_frieze(int l,int r,float y,float z0,float zh,int artY,U tint){
 ir_front(l,r,y,z0,zh,tint,l,r-l,artY,min(96,H-artY),ncBackdrop);
}
static void ir_nc_piece(int l,int t,int r,int b,int h,U c,U edge){
 ir_box(l,t,r-l,b-t,0,h,c,0);
 /* Upright texture stands on the furniture, rather than lying under the
    actor. Each material arrangement is cached and independent of frame time. */
 int z=min(h,6);
 ir_front(l+1,r-1,b+.12f,1,h-z,mix(c,edge,2),l,r-l,76,28,ncGround);
 ir_front(l+1,r-1,b+.18f,h-2,h,mix(edge,ncGold,2),0,0,0,0,0);
 ir_front(l+2,r-2,b+.2f,1,3,mix(c,0x141724,6),0,0,0,0,0);
 const U *old=ir.tex;ir.tex=ncBackdrop;
 ir_hplane(l,t,r,b,h+.06f,c,1);
 ir.tex=old;
}
static void ir_nc_round(float x,float y,float rx,float ry,float h,U c){
 IrV center={x,y,h};
 for(int i=0;i<12;i++){
  float a=i*.5235988f,b=(i+1)*.5235988f;
  IrV p={(x+cs(a)*rx),(y+sn(a)*ry),h},q={(x+cs(b)*rx),(y+sn(b)*ry),h};
  ir_face(center,p,q,center,c);
  float base=h-2.f;
  ir_face((IrV){p.x,p.y,base},(IrV){q.x,q.y,base},q,p,mix(c,ncGold,i<5?3:1));
 }
}
static void ir_nc_bottle(float x,float y,float z0,U c,int h){
 ir_box(x-2,y-1.5f,4,3,z0,h,c,0);ir_box(x-.8f,y-1,.0f+1.7f,2,z0+h,2,mix(c,ncGold,1),0);
 ir_front(x-1,x+.35f,y+1.54f,z0+1,z0+h-1,nw_blend(c,0xe9ccaf,171),0,0,0,0,0);
 ir_front(x-1.5f,x+1.3f,y+1.55f,z0+1,z0+2,0xbda280,0,0,0,0,0);
}
static void ir_nc_sofa(int x,int y,int w,int d,U cloth){
 ir_box(x,y,w,d,0,10,mix(cloth,0x281c2f,2),0);
 ir_box(x,y,w,min(d/2,9),10,12,cloth,0);
 ir_box(x,y,5,d,10,7,nw_blend(cloth,ncGold,35),0);
 ir_box(x+w-5,y,5,d,10,7,nw_blend(cloth,ncGold,35),0);
 for(int i=10;i<w-11;i+=16){
  ir_box(x+i,y+8,min(15,w-i-5),d-11,10,2,cloth,0);
  ir_front(x+i,x+i+10,y+9.1f,14,20,nw_blend(cloth,ncRose,31),0,0,0,0,0);
 }
 ir_front(x+4,x+w-4,y+d+.13f,3,4,mix(cloth,ncGold,5),0,0,0,0,0);
}
static void ir_nc_spot(float x,float y,float rx,float ry,U c,int strength){
 if(fastGfx)return;
 IrPoint t=ir_proj(x,y-ry,0),b=ir_proj(x,y+ry,0);if(!t.iz||!b.iz)return;
 for(int sy=max(ir.top,t.y);sy<min(ir.end,b.y+1);sy++){
  float ray=(sy+.5f-ir.mid)/IR_F,den=IR_S+ray*IR_C;if(den<.007f)continue;
  float d=ir.h/den,v=(ir.y-d*(IR_C-ray*IR_S)-y)/ry;
  int zi=(int)(IR_RES/d)+7,mid=(int)(W*.5f+(x-ir.x)*IR_F/d),rx0=(int)(rx*IR_F/d);
  if(rx0<1)continue;
  for(int sx=max(0,mid-rx0);sx<=min(W-1,mid+rx0);sx++){
   float u=(sx-mid)/(float)rx0,r=u*u+v*v;
   if(r>=1||chZ[sy*W+sx]>zi)continue;
   float a=1-r;a*=a;
   pixels[sy*W+sx]=nw_blend(pixels[sy*W+sx],c,(int)(strength*a));
  }
 }
}
static void ir_nc_ray(float x,float y,float h,float tx,float ty,float zz,float width,U c){
 if(fastGfx)return;
 IrPoint s=ir_proj(x,y,h),e=ir_proj(tx,ty,zz);int rows=e.y-s.y;
 if(!s.iz||!e.iz||rows<2)return;
 for(int py=max(ir.top,s.y);py<=min(ir.end-1,e.y);py++){
  float t=(py-s.y)/(float)rows;int w=max(1,(int)(width*e.scale*t)),center=s.x+(int)((e.x-s.x)*t),iz=s.iz+(int)((e.iz-s.iz)*t);
  for(int px=max(0,center-w);px<=min(W-1,center+w);px++){
   int i=py*W+px;if(chZ[i]>iz+40)continue;
   int v=nc_abs(px-center),a=(w-v)*(37+(int)(17*(1.f-t)))/w;
   pixels[i]=nw_blend(pixels[i],c,a);
  }
 }
}
static void ir_nc_stage(U left,U right,float time){
 ir_nc_frieze(288,672,151,8,79,112,0x6f4871);
 for(int i=0;i<2;i++){
  int x=i?650:291;
  ir_box(x,147,22,15,0,73,0x663d59,0);
  ir_front(x+3,x+19,162.3f,7,72,nw_blend(0x77516b,ncGold,99),0,0,0,0,0);
  ir_box(x+4,151,13,8,72,5,0xca967a,0);
 }
 ir_front(304,654,159,71,76,0xc29783,0,0,0,0,0);
 ir_front(321,640,159.2f,69,71,0xf1c794,0,0,0,0,0);
 ir_nc_piece(290,159,671,218,20,0x6b4055,0xba8b7e);
 for(int i=0;i<3;i++){
  int y=218+i*3,h=18-5*i;
  ir_box(296+i*3,y,369-i*6,3,0,h,0x634759,0);
  ir_front(296+i*3,665-i*3,y+3.1f,h-2,h,0xc48989,0,0,0,0,0);
 }
 for(int j=0;j<2;j++){
  int x=j?672:265;
  ir_box(x,171,22,20,0,30,0x272439,0);ir_front(x+2,x+20,191.2f,4,28,0x3c3146,0,0,0,0,0);
  ir_front(x+5,x+17,191.4f,18,25,0x171b2a,0,0,0,0,0);
  ir_front(x+5,x+17,191.4f,6,13,0x171b2a,0,0,0,0,0);
 }
 for(int i=0;i<13;i++)ir_box(305+i*28,217,5,2,19,2,i%2?left:right,0);
 int lv=nc_get(NC_DECOR);
 if(lv>0)for(int j=0;j<2;j++){
  int x=j?744:95;
  ir_box(x,146,121,3,38,22,0x694d69,0);
  ir_front(x+3,x+118,149.2f,41,58,0x68647d,x,121,132,27,ncBackdrop);
 }
 if(lv>1)for(int i=0;i<7;i++){
  int x=312+i*56;ir_box(x-3,158,6,7,66,5,0xa0bbb3,0);
  ir_box(x-1,161,2,2,59,7,0x6b5a76,0);
  ir_box(x-2,160,4,4,57,3,sn(time*1.6f+i)>.15f?right:left,0);
 }
}
static void ir_nc_bar_and_dj(void){
 ir_nc_piece(86,209,231,247,16,0x473951,0xbe94af);
 for(int i=0;i<2;i++){
  int x=113+i*87;
  ir_nc_round(x,226,18,10,18,0x1a2235);
  ir_nc_round(x,226,14,8,19,0x3c3752);
  ir_box(x-2,224,4,4,19,1,i?ncRose:ncIce,0);
  ir_box(x+10,220,2,3,20,2,0xd9b18b,0);
 }
 for(int j=0;j<7;j++)ir_box(142+j*5,218,2,9,16,2,j%2?0x83bbbb:0xa978a4,0);
 ir_nc_piece(732,208,876,247,17,0x704959,0xd7a583);
 for(int i=0;i<2;i++){
  float z0=16+i*12;
  ir_box(736,154+i*10,131,13,z0,3,0x735d73,0);
  ir_front(738,865,167.2f+i*10,z0+1,z0+3,0xc7a17f,0,0,0,0,0);
  for(int j=0;j<9-i;j++)ir_nc_bottle(744+j*13,161+i*10,z0+3,j%3==0?0x7eb5aa:j%3==1?0xb981a3:0xbba081,5+(j&1)*2);
 }
 for(int j=0;j<5;j++)ir_nc_bottle(770+j*15,223,17,j%2?0xca9782:0x8ea9af,3+j%2);
}
static void ir_nc_lounge(void){
 for(int i=0;i<3;i++){
  int x=210+i*266;U cloth=ncBoothUph[i],ink=ncBoothInk[i];
  ir_nc_frieze(x-114,x+111,153,11,58,126,cloth);
  ir_front(x-109,x+108,153.2f,55,57,mix(ink,ncGold,2),0,0,0,0,0);
  ir_nc_sofa(x-105,184,211,36,cloth);
  for(int d=-1;d<=1;d+=2){
   int bx=x+d*100;ir_box(bx-3,174,6,6,37,7,0x93627b,0);
   ir_box(bx-2,175,4,4,39,4,nw_blend(ink,PAPER,121),0);
  }
  ir_box(x-6,220,12,10,0,12,0x64545b,0);
  ir_nc_round(x,227,43-i*4,11+i*2,15,i==0?0x754957:i==1?0x54566c:0x4d777b);
  ir_nc_bottle(x-16,226,15,0x8b9f92,6);
  ir_nc_bottle(x+17,227,15,nc_get(NC_TABLES)&(1<<i)?ink:0xb39eaa,4);
  if(nc_get(NC_SHOWCASE)&(1<<i))ir_box(x-2,222,5,5,15,5,ink,0);
  ir_nc_spot(x,254,100,38,ink,59);
  for(int j=0;j<2;j++)ir_club_actor(NAR_SUITE+i*2+j);
  ir_marker(x,259,ink);
  ir_label(x,280,3,ncBooths[i],ink,0);
 }
 ir_nc_piece(86,305,207,353,18,0x65506b,0xc6a18a);
 ir_nc_piece(747,306,875,350,23,0x5e6a78,0xa6bab9);
 for(int j=0;j<6;j++)ir_nc_bottle(761+j*17,325,23,j&1?0xb4809b:0x82aaa2,6);
}
static void ir_nc_office(void){
 ir_nc_piece(95,156,237,191,26,0x454754,0xb2a58d);
 for(int z=9;z<=25;z+=8)for(int i=0;i<11;i++)ir_box(104+i*11,190,5,3,z,5,i%3==0?0x776678:0x586771,0);
 ir_box(712,154,146,9,10,32,0x334656,0);
 ir_front(717,854,163.2f,14,38,0x4c5e69,705,152,127,68,ncBackdrop);
 ir_nc_piece(305,198,655,237,20,0x6e5148,0xc4a188);
 ir_box(452,212,45,4,23,15,0x3b4c55,0);
 ir_front(456,494,216.2f,25,36,0x719491,0,0,0,0,0);
 ir_box(373,218,21,9,20,2,0xcab493,0);
 ir_nc_bottle(585,214,20,0x698b86,6);
 ir_nc_sofa(100,309,138,32,0x5c5b70);ir_nc_sofa(728,309,135,32,0x5c5b70);
 ir_nc_spot(480,264,175,45,0xdeb891,44);
 ir_club_actor(NAR_OFFICE);
 ir_marker(480,267,ncGold);
}

/* Mesh accessories have actual silhouette and depth. Floor actors share the
   street rig and the roster's original footholds; nothing here can move or
   replace an NPC selected by the encounter picker. */
static void ir_nc_ornament(ChRig r,ChVec a,ChVec b,ChVec c,U ink){
 av_q(r,a,b,c,c,ink);
}
static void ir_nc_costume(ChRig r,const NcCast*a,float t){
 float wave=sn(t),bob=.13f*sn(t*.24f);
 U metal=nw_blend(a->trim,0xfff1cc,48),satin=nw_blend(a->cloth,a->trim,79);
 U seam=mix(a->cloth,0x251e35,3);
 /* Opaque inner layer, raised belt, front embroidery and split silk seams.
    Several thin layers catch the room's normal-based light separately. */
 av_round(r,(ChVec){-.13f,0,6.35f+bob},(ChVec){-.13f,0,6.65f+bob},1.61f,1.62f,1.13f,1.16f,metal);
 for(int side=-1;side<=1;side+=2){
  float f=(float)side;
  av_q(r,(ChVec){1.33f,1.45f*f,10.41f+bob},(ChVec){1.35f,1.17f*f,10.42f+bob},
    (ChVec){1.08f,.20f*f,7.41f+bob},(ChVec){1.08f,.44f*f,7.55f+bob},metal);
  float heel=side*(a->fit==CA_FLARES?1.06f:.98f);
  U boot=a->fit==CA_FLARES?satin:mix(a->leg,0x212136,3);
  float shaft=a->fit==CA_FLARES?2.4f:3.15f;
  av_round(r,(ChVec){.19f,heel,.21f},(ChVec){.19f,heel,.48f},.69f,.68f,1.f,.94f,metal);
  av_round(r,(ChVec){0,heel,.51f},(ChVec){.12f,heel,shaft},a->fit==CA_FLARES?.80f:.64f,.63f,.70f,.61f,boot);
  av_round(r,(ChVec){.12f,heel,shaft},(ChVec){.12f,heel,shaft+.13f},.64f,.64f,.63f,.64f,metal);
  ch_link(r,(ChVec){.22f,1.30f*f,12.50f+bob},(ChVec){.22f,1.30f*f,12.86f+bob},.15f,.16f,.16f,.15f,metal);
 }
 for(int i=0;i<4;i++){
  float z=7.87f+i*.62f+bob,u=1.11f+.056f*i,v=(i&1)?.28f:-.24f;
  ir_nc_ornament(r,(ChVec){u,v-.15f,z},(ChVec){u+.04f,v+.04f,z+.19f},(ChVec){u,v+.21f,z-.03f},i==2?0xfff5e8:metal);
 }
 switch(a->fit){
 case CA_FRINGE:
  for(int j=-3;j<=3;j++){
   float v=j*.44f,d=v*1.24f+.17f*wave,z=5.13f+bob+.33f*sn(t+j*.88f);
   av_q(r,(ChVec){.94f,v-.089f,6.4f+bob},(ChVec){.94f,v+.089f,6.4f+bob},
     (ChVec){1.04f,d+.11f,z},(ChVec){1.04f,d-.11f,z},j&1?satin:metal);
  }
  break;
 case CA_FLARES:
  for(int side=-1;side<=1;side+=2)for(int j=0;j<4;j++){
   float v=(float)side,tip=(2.72f+j*.25f)*v,z=11.49f+bob+j*.31f;
   ir_nc_ornament(r,(ChVec){-.09f,(1.80f+j*.22f)*v,10.82f+bob},
     (ChVec){.19f,(2.08f+j*.23f)*v,10.47f+bob},(ChVec){-.18f,tip,z},j&1?metal:satin);
  }
  break;
 case CA_FEATHER:{
  float v=-3.57f,zz=11.45f-wave*1.4f;
  for(int j=0;j<7;j++){
   float k=j*.36f-.94f,tip=v-1.65f-j*.12f;
   ir_nc_ornament(r,(ChVec){1.64f,v,zz},(ChVec){1.56f,tip,zz+1.21f+k},
     (ChVec){1.63f,tip+.18f,zz+1.48f+k},j==3||j==5?metal:satin);
  }
  break;}
 case CA_STAR:
  for(int j=-2;j<2;j++){
   float v=j*.61f,h=4.77f+bob,tw=.61f*wave;
   av_q(r,(ChVec){-.99f,v,11.14f+bob},(ChVec){-.99f,v+.62f,11.14f+bob},
     (ChVec){-1.48f,(v+.62f)*1.84f+tw,h+.24f*j},(ChVec){-1.49f,v*1.84f+tw,h+.24f*j},j&1?seam:satin);
  }
  ir_nc_ornament(r,(ChVec){1.35f,-1.5f,9.44f+bob},(ChVec){1.49f,0,10.29f+bob},(ChVec){1.35f,1.5f,9.44f+bob},metal);
  break;
 case CA_RIBBON:{
  float zz=11.45f+wave*1.4f;
  for(int j=0;j<3;j++){
   float u=1.64f-j*.07f,v=3.55f+j*.77f,up=zz+sn(t*.75f-j*.6f)*j*.29f-j*.34f;
   float b=zz+sn(t*.75f-(j+1)*.6f)*(j+1)*.29f-(j+1)*.34f;
   av_q(r,(ChVec){u,v,up},(ChVec){u+.15f,v+.08f,up+.30f},
     (ChVec){u+.07f,v+.85f,b+.25f},(ChVec){u-.07f,v+.77f,b},j&1?satin:metal);
  }
  break;}
 }
}
static void ic_club_cast(float x,float y,float z,const NcCast*a,int stance,float t,float heading,float mul,int stage){
 static const int style[]={5,8,1,2,7,8,9,6};
 int motion=stance==CA_SWAY||stance==CA_PRIVATE?AV_DANCE:stance==CA_APPLAUSE?AV_CLAP:stance==CA_SEATED?AV_SEATED:stance==CA_WORKING?AV_SERVICE:AV_STILL;
 AvPose opt={.scale=(1.53f+a->tall*.054f)*mul,.floor=z,.activity=motion,.t=t,.styled=1,
   .skin=a->skin,.hair=a->hair,.pants=a->leg,.trim=a->trim,.style=style[a->style],
   .top=stage?0:a->fit==CA_BAR||a->fit==CA_DJ?5:a->fit==CA_TRACK?4:a->fit==CA_DUSTER?3:2,
   .face=a->fit==CA_LEDGER?1:0};
 StreetPerson q={x,y,heading,0,stage?MODEL_CIVILIAN:MODEL_STAFF,1,a->cloth};
 if(!stage){ic_body(&q,0,opt,NPC_IDLE,0,0,7);return;}
 if(!ic_start(x,y,z,opt.scale,7))return;
 ic_shadow(x,y,z,heading,opt.scale);
 av_body(&q,0,NPC_IDLE,0,0,0,0,&opt);
 ir_nc_costume((ChRig){x,y,cs(heading),sn(heading),opt.scale,z},a,t);
 ic_finish();
}

static void ir_club(void){
 float time=nc_motion();ir_begin(ncX,ncY,78,455,ncGround);
 ir_nc_shell(ncRoom==NC_FLOOR?ncRose:ncRoom==NC_SUITE?ncGold:ncIce);
 if(ncRoom==NC_FLOOR){
  int track=nc_music();
  const U left[]={0xf09aad,0xecc084,0xb9a7ec},right[]={0x8bd5dc,0x95cca3,0x8bbfe6};
  ir_nc_stage(left[track],right[track],time);
  ir_nc_bar_and_dj();
  ir_nc_sofa(146,310,91,33,0x573d62);ir_nc_sofa(725,310,92,33,0x674357);
  for(int s=0;s<2;s++){
   int x=s?856:104;U c=s?ncRose:ncGold;
   ir_box(x-17,318,34,12,0,24,0x4d3c54,0);ir_front(x-13,x+13,330.2f,2,20,mix(c,0x262536,5),0,0,0,0,0);
   ir_box(x-18,320,36,11,24,2,mix(c,PAPER,1),0);
   int xx=s?767:193;ir_box(xx-3,352,6,6,0,11,0x775d6b,0);
   ir_nc_round(xx,354,20,9,13,0x5f4758);ir_nc_bottle(xx-4,354,13,0x8e9a91,5);
  }
  float x=389+sn(time*.71f)*46,xx=565+sn(time*.83f+1.4f)*46;
  ir_nc_spot(x,274,96,43,left[track],87);ir_nc_spot(xx,277,91,44,right[track],91);
  ir_nc_spot(480,342,143,38,ncRose,37);
  ir_nc_ray(320,158,66,x,226,0,28,left[track]);ir_nc_ray(642,158,66,xx,226,0,29,right[track]);
  if(nc_get(NC_DECOR)==2){float a=480+sn(time*.52f)*67;ir_nc_ray(481,153,74,a,273,0,35,0xe1b3e7);ir_nc_spot(a,282,78,27,0xd6adde,71);}
  /* Same simulated cast, heels, interaction anchors and condition as overhead.
     The reciprocal-depth buffer handles their position against physical props. */
  for(int id=NAR_STAGE;id<NAR_SUITE;id++)ir_club_actor(id);
  ir_marker(160,354,ncRose);ir_marker(793,354,ncGold);
 }else if(ncRoom==NC_SUITE)ir_nc_lounge();
 else ir_nc_office();
 ir_room_actors();ir_marker(480,418,ncIce);ir_hero(ncX,ncY,ncAngle,7);
 if(npcTraceTicks>0)ir_trace();
}
