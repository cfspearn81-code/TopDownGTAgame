/* Shared live wardrobe rig. Portraits keep their established close-up art;
 * styling and animation use the same registered look and world anchors. */
enum {MODEL_CIVILIAN,MODEL_OFFICER,MODEL_RIVAL,MODEL_DISPATCH,MODEL_PLAYER,MODEL_STAFF,MODEL_WORKER};
static Pt model_ptmix(Pt a,Pt b,int k){return (Pt){a.x+(b.x-a.x)*k/8,a.y+(b.y-a.y)*k/8};}
/* Rounded, filled sleeves. Rasterizing perpendicular to the segment
 * keeps diagonal forearms as substantial as vertical or horizontal ones. */
static void model_sleeve(Pt a,Pt b,int radius,U color){
 int dx=b.x-a.x,dy=b.y-a.y,n=dx*dx+dy*dy,r2=radius*radius;
 for(int y=min(a.y,b.y)-radius;y<=max(a.y,b.y)+radius;y++)
  for(int x=min(a.x,b.x)-radius;x<=max(a.x,b.x)+radius;x++){
   int u=x-a.x,v=y-a.y,t=u*dx+v*dy,q=u*dy-v*dx;
   int fill=t<=0?u*u+v*v<=r2:t>=n?(x-b.x)*(x-b.x)+(y-b.y)*(y-b.y)<=r2:q*q<=r2*n;
   if(fill)dot(x,y,color);
  }
}
static void model_live_body(int x,int y,float co,float si,int step,int w,int wide,int top,int legs,
                           int style,int custom,int hero,U jacket,U skin,U pants,U shoe,U ink,const PulseSprite*gun){
 const int depthSide=co>=0?-1:1; /* nearest limb lands last */
 U seam=mix(jacket,ink,4),rim=mix(jacket,PAPER,3);
 for(int j=0;j<2;j++){
  int side=j?-depthSide:depthSide;
  Pt hip=rig(x,y,co,si,-2,side*3,5),boot=rig(x,y,co,si,-2+step*side,side*4,0);
  Pt knee=model_ptmix(hip,boot,4);
  U leg=j?pants:mix(pants,ink,2);
  int width=legs==1||legs==4?2:1;
  rod(hip,knee,width,mix(leg,ink,3));rod(knee,boot,width,legs==2?skin:leg);
  if(legs==2){rod(hip,knee,1,leg);dot(knee.x,knee.y,mix(leg,PAPER,2));}
  else if(legs==1||legs==4){dot(knee.x+side,knee.y-1,mix(leg,PAPER,2));dot(knee.x,knee.y,mix(leg,ink,4));}
  else if(legs==3)dot(boot.x,boot.y-2,mix(leg,PAPER,2));
  else dot(knee.x,knee.y,mix(leg,PAPER,2));
  disc(boot.x,boot.y,2,mix(shoe,ink,1));
  dot(boot.x,boot.y+1,ink);
  dot(boot.x+(co>.25f?1:co<-.25f?-1:0),boot.y-1,custom?mix(shoe,PAPER,3):0x7897a9);
 }
 /* Two fabric panels and a collar sit above the waist; the look still
  * supplies the cut, build width, materials and base palette. */
 Pt l=rig(x,y,co,si,3,-w,10),r=rig(x,y,co,si,3,w,10);
 Pt rb=rig(x,y,co,si,-4,4+wide,5),lb=rig(x,y,co,si,-4,-4-wide,5);
 Pt ls=rig(x,y,co,si,3,2-w,10),rs=rig(x,y,co,si,3,w-2,10);
 Pt rh=rig(x,y,co,si,-4,3+wide,6),lh=rig(x,y,co,si,-4,-3-wide,6);
 Pt ct=model_ptmix(ls,rs,4),cb=model_ptmix(lh,rh,4);
 quad(l,r,rb,lb,ink);
 quad(ls,ct,cb,lh,depthSide<0?mix(jacket,ink,1):mix(jacket,PAPER,1));
 quad(ct,rs,rh,cb,depthSide<0?mix(jacket,PAPER,1):mix(jacket,ink,1));
 line(lb.x,lb.y,rb.x,rb.y,seam);
 line(ls.x,ls.y,rs.x,rs.y,mix(jacket,PAPER,2));
 int back=si<-.34f;
 /* Directional detailing helps tell which way a person is facing without
  * changing their facing arrow or inventing an outline the collider lacks. */
 Pt collar=rig(x,y,co,si,2,0,9);
 if(back){
  line(ls.x,ls.y+1,rs.x,rs.y+1,seam);
  if(style==MODEL_OFFICER||style==MODEL_DISPATCH||style==MODEL_WORKER){
   Pt badge=model_ptmix(ct,cb,5);
   line(badge.x-2,badge.y,badge.x+2,badge.y,style==MODEL_WORKER?0xdbc477:0x83a4c7);
  }
 }else{
  if(top==2||top==3||top==4||top==5||style>=MODEL_STAFF){
   line(ct.x,ct.y,cb.x,cb.y,top==4?rim:seam);
   if(top==3){dot(collar.x-2,collar.y+1,rim);dot(collar.x+2,collar.y+1,rim);}
  }
  if(style==MODEL_OFFICER||style==MODEL_DISPATCH||style==MODEL_STAFF||style==MODEL_WORKER||top==5){
   Pt badge=rig(x,y,co,si,2,depthSide*2,8);
   line(badge.x,badge.y,badge.x+1,badge.y,style==MODEL_DISPATCH?0x9dbed4:style==MODEL_STAFF?PAPER:0xe8cb86);
  }
  if(style==MODEL_RIVAL){Pt scarf=model_ptmix(ct,cb,3);line(scarf.x-2,scarf.y,scarf.x+1,scarf.y,0xc88579);}
 }
 for(int j=0;j<2;j++){
  int side=j?-depthSide:depthSide;
  float offset=gun&&!gun->hand?pulse_shoulder(co,si):0;
  int gripSide=gun&&gun->hand?gun->hand:offset<-1.5f?-1:1;
  int punchSide=gun?-gripSide:1;
  float strike=hero?punchFlash:modelExtraPunch;
  int reach=side==punchSide&&strike>0?9+(int)(strike*28):0;
  Pt shoulder=rig(x,y,co,si,1,side*(w-1),10),hand=rig(x,y,co,si,reach-step*side,side*(reach?3:w),4);
  Pt bend=model_ptmix(shoulder,hand,5);
  if(gun&&!reach&&(!gun->hand||side==gun->hand)){
   /* The upper arm drops toward the ribs, not toward the muzzle.
      Only its bent forearm rises to the grip on either side. */
   hand=side==gripSide?gun->grip:gun->support;
   int crossChest=absf(offset)>1.5f&&side!=gripSide;
   bend=rig(x,y,co,si,-2+(step*side)/3,side*(w-(crossChest?4:2)),6);
   Pt cuff=model_ptmix(bend,hand,6);
   int shortSleeve=custom&&(top<2||top==5);
   U coat=j?mix(jacket,ink,1):mix(jacket,ink,3);
   U fore=shortSleeve?mix(skin,ink,j?1:3):coat;
   model_sleeve(shoulder,bend,2,ink);model_sleeve(bend,hand,2,ink);
   model_sleeve(shoulder,bend,1,top==1&&shortSleeve?fore:coat);
   model_sleeve(bend,hand,1,fore);
   if(!shortSleeve){
    disc(cuff.x,cuff.y,1,j?mix(coat,PAPER,2):seam);
    dot(shoulder.x,shoulder.y-1,j?rim:coat);
   }
   disc(hand.x,hand.y,1,mix(skin,ink,j?1:3));dot(hand.x,hand.y-1,skin);
   continue;
  }
  Pt cuff=model_ptmix(bend,hand,5);
  int shortSleeve=custom&&(top==0||top==1||top==5);
  U sleeve=j?mix(jacket,ink,2):seam;
  rod(shoulder,hand,1,ink);
  if(shortSleeve){
   line(shoulder.x,shoulder.y,bend.x,bend.y,top==1?skin:sleeve);
   line(bend.x,bend.y,hand.x,hand.y,mix(skin,ink,j?1:3));
  }else{
   line(shoulder.x,shoulder.y,cuff.x,cuff.y,sleeve);
   dot(shoulder.x,shoulder.y,j?rim:mix(jacket,PAPER,1));
   if(top==4)line(shoulder.x,shoulder.y,bend.x,bend.y,rim);
   if(style==MODEL_WORKER)dot(bend.x,bend.y,0xe1bf75);
   dot(cuff.x,cuff.y,seam);
  }
  disc(hand.x,hand.y,1,mix(skin,ink,j?2:3));dot(hand.x,hand.y-1,skin);
 }
}
static void model_head_lights(int hx,int hy,float co,float si,U ink,U skin,U hair,int style,int hue,int face){
 U warm=mix(skin,0xffd8b0,2),dim=mix(skin,0x2a2335,3);
 dot(hx-2,hy,dim);dot(hx+2,hy+1,dim);
 if(si<-.58f){
  /* Back of the head is read as hair first, with a narrow lit nape. */
  if(style!=MODEL_PLAYER){
   line(hx-2,hy,hx+2,hy,hair);line(hx-2,hy+1,hx+2,hy+1,mix(hair,ink,1));
  }
  dot(hx,hy+2,mix(skin,hair,2));
 }else{
  int profile=absf(co)>.61f,dir=co>=0?1:-1;
  dot(hx+(profile?dir:0),hy+1,warm);
  if(style!=MODEL_PLAYER){
   if(profile){dot(hx+dir,hy,ink);}
   else{dot(hx-1,hy,ink);dot(hx+1,hy,ink);}
   if(style==MODEL_STAFF&&(hue&1)){
    line(hx-2,hy-1,hx+2,hy-1,ink);dot(hx+1,hy,0x8db9bb);
   }else if(style==MODEL_RIVAL)line(hx-2,hy+1,hx+2,hy+1,0x756072);
  }else if(face==0){dot(hx+(profile?-dir:0),hy+2,dim);}
 }
}
