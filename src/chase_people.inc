/* Articulated figures in chase space, not screen rectangles or rotated floor
   sprites. Body and boots use the unchanged simulation foot anchor; solid
   geometry occludes their limbs via the same world z-buffer as cars. */
typedef struct {float x,y,co,si,scale,floor;} ChRig;
/* Local draw target for the room lens, null on streets. All actor faces
 * still come from the same costume and articulated rig. */
static void (*ch_actor_target)(const ChVec*,int,U);
static int ch_actor_detail;
static void ch_actor_face(const ChVec *p0,int n,U c){
 if(ch_actor_target)ch_actor_target(p0,n,c);else ch_face(p0,n,c,1);
}
static void ch_actor_quad(ChVec a,ChVec b,ChVec c,ChVec d,U ink){
 ChVec q[]={a,b,c,d};ch_actor_face(q,4,ink);
}
static ChVec ch_joint(ChRig r,float u,float v,float z){
 float s=r.scale>0?r.scale:1.f;
 return(ChVec){r.x+s*(u*r.co-v*r.si),r.y+s*(u*r.si+v*r.co),r.floor+s*z};
}
static void ch_link(ChRig r,ChVec a,ChVec b,float aw,float bw,float ad,float bd,U ink){
 /* Eight rounded sections upstairs, four cheaper ones on street. Room
  * pieces use the original endpoints, including actual wrist/hand poses. */
 ChVec p[16];static const float corner[][2]={{1,-1},{1,1},{-1,1},{-1,-1}};
 static const float bevel[][2]={{1,-.53f},{1,.53f},{.56f,1},{-.56f,1},{-1,.53f},{-1,-.53f},{-.56f,-1},{.56f,-1}};
 int n=ch_actor_detail>1?8:4;const float (*shape)[2]=n==8?bevel:corner;
 for(int i=0;i<n;i++){
  p[i]=ch_joint(r,a.x+ad*shape[i][0],a.y+aw*shape[i][1],a.z);
  p[n+i]=ch_joint(r,b.x+bd*shape[i][0],b.y+bw*shape[i][1],b.z);
 }
 for(int i=0;i<n;i++){
  int j=(i+1)%n;U c=i==0?mix(ink,PAPER,1):i==n/2?mix(ink,0x263042,2):mix(ink,0x263042,1);
  ch_actor_quad(p[i],p[j],p[n+j],p[n+i],c);
 }
 if(n==4)ch_actor_face(p+4,4,mix(ink,PAPER,2));
 else for(int side=0;side<2;side++){ChVec q[5];for(int j=0;j<5;j++)q[j]=p[n+((side*4+j)&7)];ch_actor_face(q,5,mix(ink,PAPER,1));}
}
static void ch_figure_shadow(float x,float y){
 if(x<0||y<0||x>=ROOM||y>=ROOM||solid(x,y,1.f))return;
 U c=headlamps_ground(chaseFloor[(int)y*ROOM+(int)x],(int)x,(int)y);
 ChVec v[5];
 for(int i=0;i<5;i++){float a=i*1.256637f;v[i]=(ChVec){x+cs(a)*4.7f,y+sn(a)*2.6f,.18f};}
 ch_face(v,5,mix(c,0x101924,4),1);
}
#include "chase_avatar.inc"
