/* Afterimage costume shop. Art uses the native raster, independent of player
   wardrobe, save fields and street actors. Heights/widths are silhouette cuts,
   not random palette swaps. All figures wear opaque costume layers. */
enum { CA_FRINGE,CA_FLARES,CA_FEATHER,CA_STAR,CA_RIBBON,
       CA_DJ,CA_BAR,CA_DOOR,CA_LEDGER,
       CA_VARSITY,CA_DUSTER,CA_DENIM,CA_TUNIC,CA_TRACK,CA_MOTO,CA_KNIT };
enum { CA_COILS,CA_BRAID,CA_CROP,CA_QUIFF,CA_BOB,CA_TAIL,CA_WAVES,CA_HAWK };
enum { CA_APPLAUSE,CA_SWAY,CA_SEATED,CA_WAITING,CA_WORKING,CA_PRIVATE };
typedef struct {
 U cloth,trim,leg,skin,hair;
 unsigned char fit,style,wide,tall;
} NcCast;
static const NcCast ncStageCast[]={
 /* Velvet Minx: rose fringe over a covered leotard and opaque tights. */
 {0xd95388,0xffcb97,0x483451,0xb47058,0x24192b,CA_FRINGE,CA_COILS,4,1},
 /* Orbit: teal reflective jumpsuit, wing seams, extra-wide hems. */
 {0x3faaa9,0xa4e8e6,0x4b7891,0xe3ab83,0x353045,CA_FLARES,CA_QUIFF,5,2},
 /* Sable: violet fan, ostrich-feather comb, braided silhouette. */
 {0x9975d7,0xebbbef,0x4d3975,0x925c4c,0x2b1e2f,CA_FEATHER,CA_BRAID,4,3},
 /* Comet: amber star-tail costume and stacked cuffs. */
 {0xeaa14f,0xffe0aa,0x723c58,0xc9896e,0x4c2d2f,CA_STAR,CA_BOB,4,0},
 /* Prism: emerald ribbon costume with techno inserts. */
 {0x72b98e,0xb9fff0,0x2d6269,0xe1b6a1,0x76405d,CA_RIBBON,CA_HAWK,5,1}
};
static const char*const ncStageNames[]={"Velvet Minx","Orbit","Sable","Comet","Prism"};
static const NcCast ncHouseCast[]={
 {0xa464bc,0x9ae4e6,0x41335a,0xbf896d,0x262037,CA_DJ,CA_TAIL,5,1},
 {0x42878c,0xaee5cb,0x354e62,0xe2a984,0x783d36,CA_BAR,CA_BRAID,5,0},
 {0x566185,0xc9c2e9,0x292d4a,0x9a6454,0x302435,CA_DOOR,CA_CROP,6,3},
 {0xb6858e,0xffd3a6,0x463f5d,0xc89476,0x55516b,CA_LEDGER,CA_WAVES,5,0},
 {0x724666,0xf4c999,0x48375a,0xd59877,0x5f3d41,CA_DOOR,CA_HAWK,6,2},
 {0x435b78,0xa9d3e3,0x35324c,0xb4826d,0x312533,CA_DOOR,CA_COILS,5,1}
};
static const NcCast ncGuestCast[]={
 {0x7584bf,0xe3a5b3,0x373e65,0xd39b7e,0x4d2d43,CA_DENIM,CA_TAIL,4,2},
 {0xa44e72,0xf2c97f,0x564568,0x895a49,0x2c2131,CA_FRINGE,CA_COILS,4,0},
 {0x6e9b71,0xaacdb2,0x464957,0xe5ad8f,0x724840,CA_DUSTER,CA_WAVES,5,3},
 {0xc57359,0xffc18f,0x4b5367,0xc18a6d,0x2e3440,CA_VARSITY,CA_CROP,6,1},
 {0x7863aa,0xe9b0db,0x3b466e,0xc88e73,0x754456,CA_TRACK,CA_BOB,4,0},
 {0x77a9ac,0xe3d4a5,0x435b73,0xa6705b,0x323148,CA_MOTO,CA_QUIFF,6,2},
 {0xc384a5,0xffceb7,0x473c61,0xe5b7a5,0x574052,CA_KNIT,CA_WAVES,5,1},
 {0x9c786b,0xf2d49f,0x455969,0x9e6b55,0x3e2940,CA_TUNIC,CA_BRAID,4,3},
 {0x75a0cd,0xadcbe0,0x404967,0xd6a288,0x714731,CA_VARSITY,CA_HAWK,5,1},
 {0x7f7fa5,0xd7cfec,0x4a3767,0x8d5e58,0x2e2636,CA_DUSTER,CA_TAIL,5,2},
 {0xaa695f,0xf8c685,0x4f4661,0xe4a382,0x4a2533,CA_DENIM,CA_BRAID,5,1},
 {0xba798e,0xe0aeb7,0x47425c,0xbf8c73,0x402a35,CA_TRACK,CA_COILS,4,0},
 {0x7ca2a0,0xc8e1bd,0x435265,0xd6ab92,0x402d31,CA_KNIT,CA_CROP,6,3},
 {0x997eaf,0xe3bedc,0x4f486c,0x9f6358,0x472f44,CA_MOTO,CA_HAWK,5,2},
 {0xbd9d5b,0xf2d597,0x655367,0xd9a085,0x5f3242,CA_TUNIC,CA_BOB,4,1},
 {0x7ab1af,0xb0dfd9,0x444367,0xa27768,0x322c3e,CA_DENIM,CA_QUIFF,5,0}
};
typedef struct{int x,y,scale,flip;} NcSprite;
static Pt nc_pt(NcSprite g,int x,int y){return (Pt){g.x+x*g.flip*g.scale/100,g.y+y*g.scale/100};}
static int nc_px(NcSprite g,int d){return max(1,d*g.scale/100);}
static void nc_stroke(NcSprite g,int x,int y,int x2,int y2,U ink){
 Pt a=nc_pt(g,x,y),b=nc_pt(g,x2,y2);line(a.x,a.y,b.x,b.y,ink);
}
static void nc_cast_dot(NcSprite g,int x,int y,U ink){Pt a=nc_pt(g,x,y);rect(a.x,a.y,nc_px(g,1),nc_px(g,1),ink);}
static void nc_cast_disc(NcSprite g,int x,int y,int r,U ink){Pt a=nc_pt(g,x,y);disc(a.x,a.y,nc_px(g,r),ink);}
static void nc_patch(NcSprite g,int x,int y,int w,int h,U ink){
 Pt a=nc_pt(g,x,y),b=nc_pt(g,x+w,y+h);rect(min(a.x,b.x),a.y,max(1,max(a.x,b.x)-min(a.x,b.x)),max(1,b.y-a.y),ink);
}
static void nc_edge(NcSprite g,Pt a,Pt b,int r,U ink){
 a=nc_pt(g,a.x,a.y);b=nc_pt(g,b.x,b.y);rod(a,b,nc_px(g,r),ink);
}
static void nc_shape(NcSprite g,Pt a,Pt b,Pt c,Pt d,U ink){
 quad(nc_pt(g,a.x,a.y),nc_pt(g,b.x,b.y),nc_pt(g,c.x,c.y),nc_pt(g,d.x,d.y),ink);
}
static void nc_costume_head(NcSprite g,const NcCast*a,int x,int y,int style){
 U edge=0x211928,light=mix(a->hair,a->cloth,3),skin=a->skin;
 /* Hair at back (lengths behind shoulders), then face, then fringe. */
 switch(style){
 case CA_BRAID:
  for(int i=0;i<5;i++)nc_cast_disc(g,x-4+i%2,y-4+i*3,2,i&1?a->hair:light);
  nc_cast_disc(g,x-3,y+9,1,a->trim);break;
 case CA_TAIL:
  nc_edge(g,(Pt){x-2,y-4},(Pt){x-6,y+8},2,a->hair);
  nc_cast_dot(g,x-5,y,a->trim);break;
 case CA_WAVES:case CA_BOB:
  nc_cast_disc(g,x-3,y,4,a->hair);nc_cast_disc(g,x+3,y+1,3,a->hair);
  nc_stroke(g,x-6,y-1,x-6,y+4,light);break;
 default:break;
 }
 nc_cast_disc(g,x,y,4,edge);nc_cast_disc(g,x,y,3,skin);
 nc_stroke(g,x-1,y+3,x+1,y+3,mix(skin,edge,3));
 switch(style){
 case CA_COILS:
  for(int i=-3;i<=3;i+=2)nc_cast_disc(g,x+i,y-3-((i&2)!=0),2,a->hair);
  nc_cast_dot(g,x-2,y-5,light);nc_cast_dot(g,x+2,y-4,light);break;
 case CA_QUIFF:
  nc_patch(g,x-4,y-5,6,3,a->hair);nc_stroke(g,x-3,y-6,x+2,y-4,light);break;
 case CA_HAWK:
  nc_edge(g,(Pt){x-1,y-3},(Pt){x+1,y-7},1,a->hair);nc_cast_dot(g,x+1,y-7,light);break;
 case CA_CROP: nc_patch(g,x-3,y-4,6,2,a->hair);nc_cast_dot(g,x-2,y-4,light);break;
 default: nc_patch(g,x-4,y-5,7,3,a->hair);nc_stroke(g,x-4,y-3,x,y-5,light);break;
 }
 nc_cast_dot(g,x-1,y,edge);nc_cast_dot(g,x+2,y,edge);nc_cast_dot(g,x+2,y+2,mix(skin,a->trim,3));
 if(a->fit<=CA_RIBBON||a->fit==CA_DUSTER||a->style==CA_BRAID)nc_cast_dot(g,x-4,y+2,a->trim);
 if(a->fit==CA_LEDGER){nc_stroke(g,x-3,y-1,x+3,y-1,0xbfc7df);nc_cast_dot(g,x,y,0xd0bec8);}
 if(a->fit==CA_DOOR){nc_cast_dot(g,x+4,y,a->trim);nc_stroke(g,x+4,y+1,x+4,y+3,a->trim);}
 if(a->fit==CA_DJ){
  nc_stroke(g,x-4,y-6,x+3,y-6,a->trim);
  nc_patch(g,x-6,y-3,2,5,a->trim);nc_patch(g,x+4,y-3,2,5,a->trim);
 }
 if(a->fit==CA_FEATHER){
  nc_stroke(g,x+3,y-4,x+5,y-12,a->trim);nc_stroke(g,x+5,y-9,x+7,y-11,a->cloth);
  nc_stroke(g,x+5,y-8,x+8,y-9,a->trim);nc_cast_dot(g,x+3,y-5,ncGold);
 }else if(a->fit==CA_STAR){
  nc_stroke(g,x-4,y-4,x+3,y-5,a->trim);nc_cast_dot(g,x+2,y-6,ncGold);
 }else if(a->fit==CA_RIBBON)nc_stroke(g,x-5,y-3,x+3,y-4,a->trim);
}
static void nc_cast_actor(int x,int y,int scale,const NcCast*a,int pose,float t,int face){
 NcSprite g={x,y,scale,face<0?-1:1};
 U edge=0x261b2f,low=mix(a->cloth,edge,3),hi=mix(a->cloth,a->trim,4),skin=a->skin,trim=a->trim;
 int s=(int)(sn(t)*2),wave=(int)(cs(t)*3),off=(int)(sn(t*.53f)*2);
 int seated=pose==CA_SEATED,dance=pose==CA_SWAY||pose==CA_PRIVATE,working=pose==CA_WORKING;
 int hipY=seated?-8:-13,sy=hipY-10-a->tall,wide=a->wide,hy=sy-5,bias=seated?0:off;
 int spread=dance?4+(int)(cs(t)*2):4;
 Pt lh={-spread-s,0},rh={spread+s,-(dance?max(0,wave):0)};
 if(seated){lh=(Pt){-6,-1};rh=(Pt){4,-1};}
 if(dance&&a->fit==CA_FLARES){lh.x-=2;rh.x+=2;}
 nc_oval(x,y+nc_px(g,2),nc_px(g,seated?9:10),nc_px(g,2),0x1d1829);
 nc_edge(g,(Pt){-3+bias,hipY},lh,2,edge);nc_edge(g,(Pt){3+bias,hipY},rh,2,edge);
 nc_edge(g,(Pt){-3+bias,hipY},lh,1,mix(a->leg,a->trim,1));nc_edge(g,(Pt){3+bias,hipY},rh,1,a->leg);
 if(a->fit==CA_FLARES||a->fit==CA_TRACK){
  nc_shape(g,(Pt){lh.x-3,lh.y},(Pt){lh.x+1,lh.y},(Pt){-1,hipY+3},(Pt){-4,hipY+3},a->leg);
  nc_shape(g,(Pt){rh.x-1,rh.y},(Pt){rh.x+3,rh.y},(Pt){4,hipY+3},(Pt){1,hipY+3},a->leg);
  nc_stroke(g,3,hipY+2,rh.x+2,rh.y,trim);
 }
 nc_patch(g,lh.x-3,lh.y-1,5,2,edge);nc_patch(g,rh.x-1,rh.y-1,5,2,edge);
 nc_stroke(g,lh.x-3,lh.y,lh.x,lh.y,a->fit<CA_DJ?trim:hi);
 nc_stroke(g,rh.x+1,rh.y,rh.x+3,rh.y,a->fit<CA_DJ?trim:hi);
 Pt l={-wide+bias,sy},r={wide+bias,sy},le={-wide-2+bias,sy+5},re={wide+2+bias,sy+5};
 Pt la={-wide-1+bias,hipY+2},ra={wide+1+bias,hipY+1};
 if(dance){
  le=(Pt){l.x-3,sy+1+wave};re=(Pt){r.x+4,sy+1-wave};
  la=(Pt){le.x-3,sy-3+wave};ra=(Pt){re.x+3,sy-3-wave};
  if(a->fit==CA_FRINGE||a->fit==CA_RIBBON){la.y=sy+5-wave;ra.y-=2;}
  if(a->fit==CA_FLARES||a->fit==CA_STAR)la.y=sy-6+wave;
 }else if(seated||working){
  le.y=sy+4;re.y=sy+4;la=(Pt){-4,hipY-1+s};ra=(Pt){6,hipY-1-wave/2};
  if(a->fit==CA_DJ)ra=(Pt){9+wave,hipY+4+s};
  if(a->fit==CA_BAR)ra=(Pt){11,sy+2-wave};
 }else if(a->fit==CA_DOOR){
  la=(Pt){2,hipY-1};ra=(Pt){-2,hipY-3};
 }else if(pose==CA_APPLAUSE){
  la.y=sy-wave;ra.y=sy+2+wave;le.x-=1;re.x+=1;
 }
 nc_edge(g,l,le,2,edge);nc_edge(g,le,la,1,a->fit==CA_FEATHER?low:a->cloth);
 nc_edge(g,r,re,2,edge);nc_edge(g,re,ra,1,a->fit==CA_MOTO?a->leg:hi);
 nc_cast_disc(g,la.x,la.y,1,skin);nc_cast_disc(g,ra.x,ra.y,1,skin);
 Pt lb={-wide+1+bias,hipY+2},rb={wide-1+bias,hipY+2};
 nc_shape(g,l,r,rb,lb,a->cloth);
 nc_stroke(g,l.x,l.y,lb.x,lb.y,low);nc_stroke(g,r.x,r.y,rb.x,rb.y,edge);
 switch(a->fit){
 case CA_FRINGE:{
  nc_shape(g,(Pt){-3+bias,hipY-2},(Pt){3+bias,hipY-2},(Pt){7+bias,hipY+6},(Pt){-7+bias,hipY+6},low);
  nc_stroke(g,-wide+1+bias,sy+1,wide-1+bias,hipY-1,trim);
  for(int i=-6;i<=6;i+=2)nc_stroke(g,i/2+bias,hipY,i+bias,hipY+4+((s+i)&2),i&2?trim:hi);
  nc_cast_dot(g,bias,hipY-2,trim);break;}
 case CA_FLARES:
  nc_shape(g,(Pt){l.x-2,sy},(Pt){r.x+2,sy},(Pt){r.x-1,sy+4},(Pt){l.x+1,sy+4},hi);
  nc_stroke(g,-3,sy+2,2,hipY,trim);nc_stroke(g,2,sy+2,2,hipY,ncGold);
  nc_patch(g,-3,hipY,7,2,edge);break;
 case CA_FEATHER:
  nc_stroke(g,l.x+1,sy+1,1,hipY+1,trim);
  if(dance)for(int i=0;i<5;i++){
   nc_stroke(g,la.x,la.y,la.x-2-i,la.y-3-(i&1),mix(a->cloth,trim,i+2));
   nc_cast_dot(g,la.x-2-i,la.y-3-(i&1),trim);
  }
  nc_patch(g,-3,hipY-1,7,2,ncGold);break;
 case CA_STAR:case CA_DUSTER:
  nc_shape(g,(Pt){l.x,sy+3},(Pt){r.x,sy+3},(Pt){rb.x+2,hipY+8},(Pt){lb.x-2,hipY+7},low);
  nc_stroke(g,l.x+1,sy+3,lb.x,hipY+6,hi);nc_stroke(g,r.x-2,sy+2,rb.x,hipY+5,trim);
  if(a->fit==CA_STAR){nc_stroke(g,-3,sy+4,3,sy+4,trim);nc_stroke(g,0,sy+1,0,sy+8,trim);nc_cast_dot(g,0,sy+4,PAPER);}
  break;
 case CA_RIBBON:
  nc_stroke(g,l.x,sy+1,rb.x,hipY,trim);nc_stroke(g,l.x,sy+3,rb.x,hipY+2,hi);
  if(dance){nc_stroke(g,ra.x,ra.y,ra.x+3,ra.y-2,trim);nc_stroke(g,ra.x+3,ra.y-2,ra.x+7,ra.y+1,hi);}
  break;
 case CA_DJ:
  nc_stroke(g,-3,sy+3,3,sy+3,trim);nc_stroke(g,-3,sy+6,4,sy+6,low);
  nc_cast_dot(g,-3,sy+4,ncGold);nc_cast_dot(g,3,sy+4,ncGold);break;
 case CA_BAR:
  nc_stroke(g,-3,sy,2,sy+4,trim);nc_stroke(g,3,sy,2,sy+4,trim);
  nc_patch(g,-4,sy+4,9,9,low);nc_patch(g,-2,sy+7,5,3,hi);
  nc_patch(g,ra.x-1,ra.y-5,3,4,trim);nc_cast_dot(g,ra.x,ra.y-6,PAPER);break;
 case CA_DOOR:
  nc_stroke(g,-wide,sy+2,wide,sy+2,hi);nc_stroke(g,0,sy+2,0,hipY,trim);
  nc_patch(g,2,sy+4,3,2,trim);
  nc_edge(g,le,la,1,low);nc_edge(g,re,ra,1,hi);break;
 case CA_LEDGER:
  nc_stroke(g,-2,sy+1,1,hipY,trim);nc_stroke(g,3,sy+1,1,hipY,trim);
  nc_patch(g,ra.x-3,ra.y-3,6,5,0xd8ba95);nc_stroke(g,ra.x-2,ra.y-1,ra.x+1,ra.y-1,low);break;
 case CA_DENIM:case CA_MOTO:
  nc_stroke(g,-wide,sy+3,wide,sy+3,hi);nc_stroke(g,-3,sy+6,2,hipY,trim);
  nc_cast_dot(g,3,sy+5,trim);nc_patch(g,2,sy+6,3,2,low);break;
 case CA_VARSITY:
  nc_stroke(g,-wide,sy+2,wide,sy+2,trim);nc_stroke(g,-wide+1,hipY,wide-1,hipY,trim);
  nc_cast_dot(g,2,sy+4,ncGold);break;
 case CA_KNIT:
  for(int i=2;i<10;i+=3)nc_stroke(g,-wide+2,sy+i,wide-2,sy+i,i==5?trim:hi);
  break;
 case CA_TUNIC:
  nc_stroke(g,l.x,sy+1,0,sy+5,trim);nc_stroke(g,0,sy+5,r.x,sy+1,trim);
  nc_stroke(g,lb.x,hipY-1,rb.x,hipY-1,hi);break;
 default: nc_stroke(g,-wide+1,sy+2,wide-1,sy+2,trim);break;
 }
 nc_cast_dot(g,bias,sy-1,skin);
 nc_costume_head(g,a,bias,hy,a->style);
 if(dance&&a->fit<=CA_RIBBON&&wave>1){
  nc_cast_dot(g,ra.x-1,ra.y-2,trim);nc_cast_dot(g,la.x+1,la.y-2,trim);
 }
}
static void nc_stage_actor(int x,int y,int index,float time){
 float phase=time*(3.35f+nc_music()*.45f)+index*1.73f;
 nc_cast_actor(x+(int)(sn(phase*.62f)*5),y,142,&ncStageCast[index],CA_SWAY,phase,1);
}
static void nc_guest_actor(int x,int y,int guest,int pose,float time,int face,int scale){
 const NcCast*a=&ncGuestCast[guest%LEN(ncGuestCast)];
 nc_cast_actor(x,y,scale,a,pose,time,face);
}
