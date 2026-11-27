/* Headwear and tailored layers are real chase-space volume meshes;
 * their colors/materials share the overhead editor's validated catalog. */
static void ward_brim_3d(ChRig r,float x0,float x1,float w0,float w1,float z0,float z1,U c){
 av_q(r,(ChVec){x0,-w0,z0},(ChVec){x0,w0,z0},(ChVec){x1,w1,z1},(ChVec){x1,-w1,z1},c);
}
static void ward_body_3d(ChRig r,float bob,float w,U bits,U cloth){
 int coat=ward_part(bits,CW_OUTERWEAR),accessory=ward_part(bits,CW_ACCESSORY);
 U trim=ward_trim_color(bits),dark=mix(cloth,0x1a2738,4),rim=mix(cloth,trim,2);
 if(coat){
  float bottom=coat==1?4.20f:coat==6?5.10f:5.72f,lower=w*(coat==1?.87f:.78f);
  av_round(r,(ChVec){-.17f,0,bottom+bob},(ChVec){-.07f,0,10.61f+bob},lower,w+.17f,1.29f,1.42f,coat==2?dark:cloth);
  av_round(r,(ChVec){0,0,10.57f+bob},(ChVec){.02f,0,11.35f+bob},w+.17f,w*.76f,1.42f,1.06f,rim);
  if(coat==1||coat==2||coat==6){
   av_front(r,1.32f,-.15f,.15f,bottom+bob,10.54f+bob,trim);
   for(int k=-1;k<=1;k+=2){
    av_q(r,(ChVec){1.37f,k*.99f,11.15f+bob},(ChVec){1.44f,k*w*.67f,10.76f+bob},(ChVec){1.44f,k*.20f,8.66f+bob},(ChVec){1.36f,k*.35f,10.21f+bob},coat==6?trim:rim);
    av_front(r,-1.33f,k*.76f,k*1.14f,8.74f+bob,9.10f+bob,rim);
   }
  }
  if(coat==3||coat==5)for(float y=7.05f;y<10.5f;y+=coat==5?.90f:1.35f)
   av_round(r,(ChVec){-.06f,0,y+bob},(ChVec){-.06f,0,y+.15f+bob},w*.92f,w,1.43f,1.46f,trim);
  if(coat==4){
   av_round(r,(ChVec){-.85f,0,9.43f+bob},(ChVec){-.98f,0,11.56f+bob},.88f,1.23f,.52f,.60f,dark);
   av_front(r,-1.60f,-.92f,.92f,9.91f+bob,10.20f+bob,trim);
  }
 }
 if(accessory==1||accessory==2||accessory==7){
  av_round(r,(ChVec){0,0,11.18f+bob},(ChVec){.05f,0,11.62f+bob},.75f,1.11f,.88f,1.0f,trim);
  if(accessory!=1)av_front(r,1.52f,accessory==7?.67f:-1.30f,accessory==7?1.30f:-.70f,8.18f+bob,11.22f+bob,trim);
  if(accessory==1)av_front(r,1.45f,-.15f,.15f,10.4f+bob,10.8f+bob,trim);
 }
 if(accessory==4){
  av_q(r,(ChVec){1.46f,-w*.69f,11.24f+bob},(ChVec){1.46f,-w*.52f,11.25f+bob},(ChVec){1.46f,w*.85f,6.18f+bob},(ChVec){1.46f,w*.66f,6.18f+bob},trim);
  ch_link(r,(ChVec){-.70f,w*.69f,5.36f+bob},(ChVec){-.73f,w*.73f,7.08f+bob},.61f,.65f,.68f,.64f,dark);
  av_front(r,-1.40f,w*.46f,w*.98f,6.40f+bob,6.67f+bob,trim);
 }
 if(accessory==5)av_front(r,1.48f,-.88f,-.52f,9.62f+bob,10.06f+bob,trim);
}
static void ward_head_3d(ChRig r,float z,U bits,U skin,U hair,int masked,int detail){
 int hat=ward_hat(bits),cut=ward_part(bits,CW_HAIR),face=ward_part(bits,CW_FACE);
 U c=ward_hat_color(bits),a=ward_trim_color(bits),dark=mix(c,0x1a2535,3),light=mix(c,PAPER,2);
 switch(cut){
 case 1:
  for(int k=-1;k<=1;k+=2){
   ch_link(r,(ChVec){-.95f,k*1.05f,z-.62f},(ChVec){-.65f,k*.83f,z+2.34f},.28f,.31f,.34f,.43f,hair);
   if(detail)ch_link(r,(ChVec){-1.f,k*1.05f,z-.50f},(ChVec){-1.f,k*1.05f,z-.34f},.31f,.32f,.37f,.39f,mix(hair,PAPER,2));
  }break;
 case 2:
  av_round(r,(ChVec){.04f,0,z+1.98f},(ChVec){.11f,-.12f,z+2.91f},1.44f,1.08f,1.03f,.91f,hair);
  if(detail)av_front(r,1.02f,-.58f,.90f,z+2.31f,z+2.49f,mix(hair,PAPER,3));break;
 case 3:
  for(int k=-1;k<=1;k+=2)av_round(r,(ChVec){-.31f,k*1.13f,z+2.04f},(ChVec){-.39f,k*1.15f,z+3.36f},.66f,.43f,.68f,.42f,hair);
  break;
 }
 if(detail){
  if(face==1&&!masked)av_front(r,1.36f,.62f,.87f,z+.48f,z+.70f,mix(skin,0xaa535d,4));
  if(face==2&&!masked)for(int k=-1;k<=1;k+=2)av_front(r,1.37f,k*.55f,k*.68f,z+.56f,z+.67f,mix(skin,hair,4));
  if(face==3&&!masked)av_front(r,1.40f,-.73f,.73f,z+.40f,z+.60f,mix(hair,skin,1));
  if(ward_part(bits,CW_ACCESSORY)==3)ch_link(r,(ChVec){-.10f,1.32f,z+.38f},(ChVec){-.13f,1.35f,z+.92f},.18f,.16f,.18f,.19f,a);
 }
 if(masked){
  av_round(r,(ChVec){.23f,0,z-.09f},(ChVec){.11f,0,z+2.02f},1.02f,1.40f,1.03f,1.29f,wardWrap);
  av_front(r,1.45f,-1.0f,1.0f,z+.93f,z+1.40f,mix(skin,wardWrap,2));
  if(detail)for(int k=-1;k<=1;k+=2)av_front(r,1.47f,k*.38f,k*.64f,z+1.05f,z+1.24f,0x151f2b);
 }
 if(!hat)return;
 switch(hat){
 case 1:
  av_round(r,(ChVec){0,0,z+1.72f},(ChVec){-.17f,0,z+2.83f},1.49f,1.16f,1.28f,1.00f,c);
  ward_brim_3d(r,.61f,2.75f,1.47f,.99f,z+1.88f,z+1.64f,dark);
  if(detail)av_front(r,1.23f,-.31f,.31f,z+2.10f,z+2.45f,a);break;
 case 2:
  av_round(r,(ChVec){0,0,z+1.30f},(ChVec){-.13f,0,z+2.94f},1.57f,.97f,1.24f,.83f,c);
  av_round(r,(ChVec){0,0,z+1.36f},(ChVec){-.05f,0,z+1.75f},1.62f,1.58f,1.30f,1.34f,dark);
  ch_link(r,(ChVec){-.08f,0,z+2.91f},(ChVec){-.12f,0,z+3.56f},.45f,.34f,.42f,.35f,a);break;
 case 3:
  av_round(r,(ChVec){0,0,z+1.63f},(ChVec){-.08f,0,z+2.90f},1.70f,1.32f,1.51f,1.08f,c);
  av_round(r,(ChVec){0,0,z+1.52f},(ChVec){0,0,z+1.69f},2.26f,1.59f,2.14f,1.50f,dark);
  av_round(r,(ChVec){0,0,z+1.71f},(ChVec){-.02f,0,z+1.91f},1.67f,1.63f,1.57f,1.52f,a);break;
 case 4:
  av_round(r,(ChVec){0,0,z+1.63f},(ChVec){0,0,z+1.82f},2.56f,2.51f,2.32f,2.22f,dark);
  av_round(r,(ChVec){0,0,z+1.81f},(ChVec){-.15f,0,z+3.35f},1.58f,1.15f,1.45f,1.24f,c);
  av_round(r,(ChVec){0,0,z+1.82f},(ChVec){-.03f,0,z+2.12f},1.59f,1.56f,1.53f,1.45f,a);
  if(detail)ward_brim_3d(r,-.94f,.66f,.17f,.17f,z+3.36f,z+3.37f,dark);break;
 case 5:
  av_round(r,(ChVec){0,0,z+1.61f},(ChVec){-.22f,0,z+2.55f},1.61f,1.31f,1.46f,1.38f,c);
  ward_brim_3d(r,.76f,2.09f,1.40f,.91f,z+1.76f,z+1.58f,dark);
  if(detail)ward_brim_3d(r,-1.05f,.53f,.16f,.13f,z+2.57f,z+2.57f,light);break;
 case 6:
  av_round(r,(ChVec){-.02f,0,z+1.46f},(ChVec){-.12f,0,z+3.00f},1.55f,1.04f,1.62f,1.05f,c);
  av_round(r,(ChVec){-.03f,0,z+1.48f},(ChVec){-.03f,0,z+1.63f},1.96f,1.83f,2.02f,1.86f,dark);
  ward_brim_3d(r,-1.13f,1.05f,.17f,.14f,z+2.94f,z+2.95f,a);break;
 case 7:
  av_round(r,(ChVec){-.47f,0,z+.33f},(ChVec){-.21f,0,z+2.76f},1.49f,1.44f,.73f,1.31f,c);
  for(int k=-1;k<=1;k+=2)ch_link(r,(ChVec){.20f,k*1.26f,z+.27f},(ChVec){.61f,k*1.07f,z+2.11f},.27f,.32f,.49f,.66f,dark);
  if(detail)av_front(r,-1.36f,-.47f,.47f,z+1.12f,z+1.36f,a);break;
 case 8:
  av_round(r,(ChVec){.04f,0,z+1.55f},(ChVec){0,0,z+1.96f},1.46f,1.39f,1.43f,1.38f,c);
  ch_link(r,(ChVec){-1.28f,.92f,z+.55f},(ChVec){-1.29f,.62f,z+1.76f},.27f,.33f,.32f,.41f,a);break;
 case 9:
  av_round(r,(ChVec){0,0,z+1.54f},(ChVec){0,0,z+1.73f},2.96f,2.91f,2.81f,2.71f,dark);
  av_round(r,(ChVec){0,0,z+1.70f},(ChVec){-.11f,0,z+2.77f},1.51f,1.28f,1.35f,1.13f,c);
  av_round(r,(ChVec){0,0,z+1.71f},(ChVec){-.03f,0,z+1.96f},1.59f,1.54f,1.43f,1.33f,a);break;
 }
}
