/* Pixel-art clothing details. Wearables use the existing articulated torso,
 * facing and foot anchors in editors, on the street and inside rooms. */
static void ward_body_2d(int x,int y,float co,float si,int stride,U bits,U top,U leg,U shoe,U skin,int width){
 int coat=ward_part(bits,CW_OUTERWEAR),item=ward_part(bits,CW_ACCESSORY),sole=ward_part(bits,CW_FOOT_TONE);
 U trim=ward_trim_color(bits),dark=mix(top,0x182536,4),light=mix(top,trim,2);
 if(coat){
  const int cut=coat==1?2:coat==6?3:5;
  int shoulder=width-1,hem=coat==1?5:coat==4?4:3;
  Pt a=rig(x,y,co,si,-1,-shoulder,10),b=rig(x,y,co,si,-1,shoulder,10);
  Pt c=rig(x,y,co,si,-4,hem,cut),d=rig(x,y,co,si,-4,-hem,cut);
  for(int k=-1;k<=1;k+=2){
   Pt sh=rig(x,y,co,si,1,k*(shoulder+1),9),hand=rig(x,y,co,si,-stride*k,k*(width-1),5);
   if(coat==1||coat==2||coat==4||coat>=5)rod(sh,hand,1,dark);
  }
  quad(a,b,c,d,dark);
  Pt aa=rig(x,y,co,si,2,-shoulder+1,10),bb=rig(x,y,co,si,2,shoulder-1,10);
  Pt cc=rig(x,y,co,si,1,hem,cut+1),dd=rig(x,y,co,si,1,-hem,cut+1);
  quad(aa,bb,cc,dd,coat==2?dark:top);
  rod(aa,bb,0,coat==6?mix(trim,PAPER,4):light);
  if(coat==1||coat==2||coat==6){
   Pt lapel=rig(x,y,co,si,3,0,6);rod(aa,lapel,0,coat==6?trim:light);rod(bb,lapel,0,coat==6?trim:light);
   dot(lapel.x,lapel.y+1,trim);
  }else if(coat==3){
   for(int z0=6;z0<=9;z0+=3)rod(rig(x,y,co,si,3,-4,z0),rig(x,y,co,si,3,4,z0),0,trim);
   Pt p0=rig(x,y,co,si,4,3,7);rect(p0.x-1,p0.y-1,3,2,dark);
  }else if(coat==4){
   Pt collar=rig(x,y,co,si,0,0,12);rod(a,collar,1,top);rod(b,collar,1,top);rod(d,c,0,trim);
  }else if(coat==5){
   for(int z0=6;z0<=9;z0+=2)rod(rig(x,y,co,si,3,-4,z0),rig(x,y,co,si,3,4,z0),0,trim);
  }
 }
 if(item==1){Pt a=rig(x,y,co,si,2,-3,11),b=rig(x,y,co,si,4,0,8),c=rig(x,y,co,si,2,3,11);rod(a,b,0,trim);rod(b,c,0,trim);}
 if(item==2||item==7){
  Pt a=rig(x,y,co,si,1,-5,11),b=rig(x,y,co,si,1,5,11);
  rod(a,b,item==7?2:1,trim);
  Pt c=rig(x,y,co,si,-3,item==7?6:-4,item==7?5:6);rod(item==7?b:a,c,1,mix(trim,top,2));
  dot(c.x,c.y,mix(trim,PAPER,2));
 }
 if(item==4){
  Pt a=rig(x,y,co,si,1,-5,11),b=rig(x,y,co,si,3,5,5);rod(a,b,1,0x252d3c);rod(a,b,0,trim);
  Pt c=rig(x,y,co,si,1,6,4);disc(c.x,c.y,2,dark);line(c.x-1,c.y,c.x+1,c.y,trim);
 }
 if(item==5){Pt a=rig(x,y,co,si,3,3,9);disc(a.x,a.y,1,trim);dot(a.x,a.y-1,PAPER);}
 if(item==6)for(int k=-1;k<=1;k+=2){
  Pt a=rig(x,y,co,si,-stride*k,k*(width-1),4);disc(a.x,a.y,1,0x283040);dot(a.x,a.y-1,trim);
 }
 if(sole)for(int k=-1;k<=1;k+=2){
  Pt a=rig(x,y,co,si,-2+stride*k,k*4,0);rod((Pt){a.x-1,a.y+1},(Pt){a.x+1,a.y+1},0,wardSole[sole]);
  dot(a.x,a.y-1,mix(wardSole[sole],shoe,2));
 }
 (void)leg;(void)skin;
}
static void ward_head_2d(int x,int y,float co,float si,U bits,U skin,U hair,int masked){
 int hat=ward_hat(bits),haircut=ward_part(bits,CW_HAIR),face=ward_part(bits,CW_FACE);
 int facing=co>.20f?1:co<-.20f?-1:0,front=si>-.58f;
 U c=ward_hat_color(bits),a=ward_trim_color(bits),dark=mix(c,0x0a1523,3),light=mix(c,PAPER,2);
 if(haircut){
  if(haircut==1){rect(x-3,y-4,7,2,hair);for(int k=-1;k<=1;k+=2){line(x+k*3,y-2,x+k*4,y+4,hair);dot(x+k*4,y+3,mix(hair,PAPER,2));}}
  if(haircut==2){rect(x-3,y-5,7,3,hair);for(int k=-2;k<=2;k+=2)dot(x+k,y-4+(k+2)%3,mix(hair,PAPER,3));dot(x-3,y-2,mix(skin,hair,2));}
  if(haircut==3){disc(x-3,y-5,2,hair);disc(x+3,y-5,2,hair);rect(x-3,y-3,7,1,hair);dot(x-4,y-5,mix(hair,PAPER,3));dot(x+2,y-6,mix(hair,PAPER,3));}
 }
 if(front&&!masked){
  if(face==1){dot(x+2,y,mix(skin,0x9b3c48,3));dot(x+3,y+1,mix(skin,PAPER,3));}
  if(face==2){dot(x-2,y+1,mix(skin,hair,3));dot(x+2,y+1,mix(skin,hair,3));}
  if(face==3){line(x-2,y+1,x+2,y+1,mix(hair,skin,1));dot(x,y+1,hair);}
 }
 if(ward_part(bits,CW_ACCESSORY)==3){dot(x+4*(facing?facing:-1),y+1,a);dot(x+3*(facing?facing:-1),y+2,light);}
 if(masked){
  if(front){
   rect(x-3,y-2,7,5,wardWrap);line(x-2,y-1,x+2,y-1,mix(skin,wardWrap,2));
   dot(x-1,y-1,0x151f2b);dot(x+1,y-1,0x151f2b);line(x-1,y+1,x+2,y+1,mix(a,wardWrap,6));
  }else rect(x-3,y-2,7,5,wardWrap);
 }
 if(!hat)return;
 switch(hat){
 case 1:
  rect(x-3,y-5,7,4,dark);rect(x-2,y-6,5,3,c);line(x-2,y-5,x+2,y-5,light);
  if(facing)line(x+facing,y-2,x+7*facing,y-2,c);else if(front)rect(x-4,y-2,9,2,c);
  dot(x+(facing?facing:0),y-4,a);break;
 case 2:
  disc(x,y-4,4,dark);disc(x,y-5,3,c);disc(x,y-8,1,a);
  rect(x-4,y-3,9,2,c);for(int k=-3;k<4;k+=2)dot(x+k,y-2,light);break;
 case 3:
  tri(x-4,y-3,x+4,y-3,x-3,y-7,c);tri(x-3,y-7,x+3,y-7,x+4,y-3,c);
  rect(x-4,y-4,9,1,a);line(x-5,y-3,x+5,y-3,dark);line(x-6,y-2,x+6,y-2,c);break;
 case 4:
  rect(x-4,y-7,9,4,dark);rect(x-3,y-8,7,4,c);line(x-2,y-8,x+2,y-8,light);
  rect(x-4,y-5,9,1,a);line(x-6,y-4,x+6,y-4,c);line(x-7,y-3,x+7,y-3,dark);break;
 case 5:
  disc(x,y-4,4,dark);rect(x-3,y-5,7,2,c);line(x-4,y-4,x+4,y-4,light);
  line(x-4,y-2,x+4,y-2,a);if(facing)line(x+3*facing,y-2,x+6*facing,y-2,dark);break;
 case 6:
  disc(x,y-5,4,dark);disc(x,y-5,3,c);rect(x-5,y-4,11,2,c);
  line(x-4,y-4,x+4,y-4,light);line(x,y-8,x,y-5,a);break;
 case 7:
  disc(x,y-2,5,dark);disc(x,y-4,4,c);
  if(front){disc(x,y,3,masked?wardWrap:skin);dot(x-1,y-1,0x293445);dot(x+1,y-1,0x293445);}
  line(x-4,y-1,x-3,y+3,a);line(x+4,y-1,x+3,y+3,a);dot(x,y-7,light);break;
 case 8:
  line(x-4,y-3,x+4,y-3,c);line(x-4,y-2,x+4,y-2,a);
  if(!front||facing)line(x-(facing?facing*4:4),y-2,x-(facing?facing*6:5),y+1,c);
  break;
 case 9:
  disc(x,y-5,3,c);rect(x-3,y-6,7,3,c);line(x-3,y-4,x+3,y-4,a);
  line(x-8,y-3,x+8,y-3,c);line(x-7,y-2,x+7,y-2,dark);dot(x-2,y-7,light);break;
 }
}
