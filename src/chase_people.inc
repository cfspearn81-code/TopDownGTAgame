/* The authored Afterimage night rider is not a generic road pedestrian.
 * Its true in-progress waiting/boarding/walking pose is shared by both
 * views after night_club_gig has defined the contract and cast. */
static int street_night_rider_pose(StreetPerson*out,int*mark){
#if __has_include("night_club.inc") && __has_include("night_club_gig.inc")
 float x,y;if(!ncr_ped(&x,&y))return 0;
 const NcRegular*r=ncr_cast+NCR_VOCALIST;
 *out=(StreetPerson){x,y,ncr.a,ncr.phase==NCR_WAIT?0:(int)(sn(anim*12.f)*4),r->model,r->hue,r->coat};
 if(mark)*mark=ncr.phase!=NCR_LEAVE;
 return 1;
#else
 (void)out;(void)mark;return 0;
#endif
}
/* Articulated figures in chase space, not screen rectangles or rotated floor
   sprites. Body and boots use the unchanged simulation foot anchor; solid
   geometry occludes their limbs via the same world z-buffer as cars. */
typedef struct {float x,y,co,si;} ChRig;
static ChVec ch_joint(ChRig r,float u,float v,float z){
 return(ChVec){r.x+u*r.co-v*r.si,r.y+u*r.si+v*r.co,z};
}
static void ch_link(ChRig r,ChVec a,ChVec b,float aw,float bw,float ad,float bd,U ink){
 ChVec p[8];static const int corner[][2]={{1,-1},{1,1},{-1,1},{-1,-1}};
 for(int i=0;i<4;i++){
  p[i]=ch_joint(r,a.x+ad*corner[i][0],a.y+aw*corner[i][1],a.z);
  p[4+i]=ch_joint(r,b.x+bd*corner[i][0],b.y+bw*corner[i][1],b.z);
 }
 for(int i=0;i<4;i++){
  int j=(i+1)&3;U c=i==0?mix(ink,PAPER,1):i==2?mix(ink,0x263042,2):mix(ink,0x263042,1);
  ch_quad(p[i],p[j],p[4+j],p[4+i],c,1);
 }
 ch_face(p+4,4,mix(ink,PAPER,2),1);
}
static void ch_figure_shadow(float x,float y){
 if(x<0||y<0||x>=ROOM||y>=ROOM||solid(x,y,1.f))return;
 U c=headlamps_ground(city[(int)y*ROOM+(int)x],(int)x,(int)y);
 ChVec v[5];
 for(int i=0;i<5;i++){float a=i*1.256637f;v[i]=(ChVec){x+cs(a)*4.7f,y+sn(a)*2.6f,.18f};}
 ch_face(v,5,mix(c,0x101924,4),1);
}
static void ch_character(const StreetPerson*p0){
 ChVec center=ch_local((ChVec){p0->x,p0->y,8});
 if(center.z<7||center.z>690||absf(center.x)>center.z*1.31f+18)return;
 ChRig r={p0->x,p0->y,cs(p0->a),sn(p0->a)};
 int hue=p0->hue%5,uniform=p0->role==MODEL_OFFICER||p0->role==MODEL_DISPATCH;
 static const U skins[]={0xdeb291,0xa16352,0x805248,0xe3c3a7,0xbd8571};
 static const U hairs[]={0x29323c,0x201720,0x45403f,0x66433f,0x34263b};
 U skin=skins[hue],coat=p0->coat,hair=hairs[hue],pants=uniform?0x394a63:p0->role==MODEL_RIVAL?0x5c4d62:0x56647a;
 /* Warm spot lights stay connected to their existing optical falloff at the
    actor's feet, rather than lighting leg pixels as road markings. */
 coat=headlamps_ground(coat,(int)p0->x,(int)p0->y);
 skin=headlamps_ground(skin,(int)p0->x,(int)p0->y);
 float step=p0->step*.54f,body=(absf(step))*.05f,head=13.05f-body;
 float wide=2.10f+(hue&1)*.19f;
 ch_figure_shadow(r.x,r.y);
 for(int side=-1;side<=1;side+=2){
  float stride=step*side,bootY=side*1.08f,lift=max(0,(int)(stride*.53f));
  ChVec hip={-.24f,bootY,6.2f-body},knee={stride*.54f,bootY,3.45f+lift*.25f},
        foot={stride,bootY,.28f+lift};
  ch_link(r,foot,(ChVec){stride-.22f,bootY,foot.z+.85f},.62f,.58f,.82f,.57f,0x283447);
  ch_link(r,(ChVec){stride-.22f,bootY,foot.z+.36f},knee,.58f,.60f,.46f,.56f,pants);
  ch_link(r,knee,hip,.60f,.74f,.56f,.63f,pants);
 }
 ch_link(r,(ChVec){-.17f,0,5.9f-body},(ChVec){.19f,0,11.4f-body},wide*.73f,wide,1.02f,1.07f,coat);
 for(int side=-1;side<=1;side+=2){
  float swing=-step*side;
  ChVec shoulder={.17f,(wide-.03f)*side,10.68f-body};
  ChVec elbow={swing*.48f,(wide+.54f)*side,8.2f},wrist={swing*.91f,(wide+.36f)*side,6.58f};
  ch_link(r,elbow,shoulder,.51f,.57f,.48f,.55f,mix(coat,0x283347,2));
  ch_link(r,wrist,elbow,.37f,.49f,.36f,.44f,coat);
  ch_link(r,(ChVec){wrist.x,wrist.y,wrist.z-.58f},wrist,.34f,.35f,.29f,.31f,skin);
 }
 ch_link(r,(ChVec){.23f,0,11.1f-body},(ChVec){.24f,0,head+.77f},.63f,1.10f,.59f,.88f,skin);
 ch_link(r,(ChVec){.24f,0,head+.77f},(ChVec){.24f,0,head+1.78f},1.13f,.90f,.90f,.75f,uniform?0x415a80:hair);
 if(uniform){
  ch_quad(ch_joint(r,1.13f,-1.14f,head+.54f),ch_joint(r,1.13f,1.14f,head+.54f),
          ch_joint(r,1.63f,.86f,head+.40f),ch_joint(r,1.63f,-.86f,head+.40f),0x778ab7,1);
  ch_quad(ch_joint(r,1.18f,.34f,9.65f),ch_joint(r,1.18f,.97f,9.65f),
          ch_joint(r,1.20f,.97f,10.33f),ch_joint(r,1.2f,.34f,10.33f),0xc6c8a5,1);
 }else if(p0->role==MODEL_RIVAL){
  ch_link(r,(ChVec){.18f,0,11.25f-body},(ChVec){.20f,0,11.66f-body},1.14f,1.1f,.96f,.94f,0xdd8cac);
 }else{
  ch_quad(ch_joint(r,1.28f,-.46f,head-.32f),ch_joint(r,1.28f,.48f,head-.32f),
          ch_joint(r,1.28f,.46f,head-.12f),ch_joint(r,1.28f,-.46f,head-.12f),mix(skin,0x403139,3),1);
 }
}
