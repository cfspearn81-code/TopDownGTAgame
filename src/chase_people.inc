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
#include "chase_avatar.inc"
