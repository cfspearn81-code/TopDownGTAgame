/* World-space street fixtures and action roles. None supplies collision,
 * steering, mission progress, route targets or saved fields. */
typedef struct {float x,y,c,s;} CrProp;
static ChVec crp(const CrProp*g,float a,float b,float z){return (ChVec){g->x+a*g->c-b*g->s,g->y+a*g->s+b*g->c,z};}
static void cr_rect(const CrProp*g,float l,float r,float t,float b,float z,U ink,int shade){
 ch_quad(crp(g,l,t,z),crp(g,r,t,z),crp(g,r,b,z),crp(g,l,b,z),ink,shade);
}
static void cr_block(const CrProp*g,float l,float r,float t,float b,float z,float h,U base,U top){
 ChVec p[]={crp(g,l,t,z),crp(g,r,t,z),crp(g,r,b,z),crp(g,l,b,z),crp(g,l,t,h),crp(g,r,t,h),crp(g,r,b,h),crp(g,l,b,h)};
 for(int i=0;i<4;i++){int j=(i+1)&3;ch_quad(p[i],p[j],p[j+4],p[i+4],i&1?mix(base,DARK,2):base,1);}
 ch_face(p+4,4,top,1);
}
static int cr_visible(float x,float y,float z,float wide,float far){
 float dx=x-ch.x,dy=y-ch.y;if(d2(dx,dy)>far*far)return 0;
 ChVec p0=ch_local((ChVec){x,y,z});
 return p0.z>9.f-wide&&absf(p0.x)<p0.z*1.31f+wide;
}
static void cr_letters(const CrProp*g,float x,float face,float high,float px,const char*s,U ink,int mirror){
 int count=sl(s);float x0=x-(count*4-1)*px*.5f;
 for(int i=0;i<count;i++){
  U bits=zone_letter(s[i]);
  for(int row=0;row<5;row++)for(int k=0;k<3;){
   if(!(bits&(1u<<(14-row*3-k)))){k++;continue;}
   int j=k;do{k++;}while(k<3&&(bits&(1u<<(14-row*3-k))));
   float a=x0+(i*4+j)*px,b=x0+(i*4+k)*px,h=high-row*px;
   if(mirror){a=2*x-a;b=2*x-b;}
   ch_quad(crp(g,a,face,h),crp(g,b,face,h),crp(g,b,face,h-px),crp(g,a,face,h-px),ink,0);
  }
 }
}
/* New route props stand on the existing GPS and swept-checkpoint anchors.
 * Reflectors, markings and soft shells keep the crossing visually open. */
static void cr_beacon(const CrProp*g,float u,float v,U color,int tall,int flash){
 float h=tall?10.9f:6.7f;
 cr_block(g,u-1.65f,u+1.65f,v-1.42f,v+1.42f,.10f,.61f,0x2a393e,0x687077);
 cr_block(g,u-.87f,u+.87f,v-.74f,v+.74f,.61f,h-.68f,0x3c4548,0x7b8583);
 cr_block(g,u-1.05f,u+1.05f,v-.92f,v+.92f,h-.68f,h+.54f,mix(color,DARK,3),color);
 for(int i=-1;i<=1;i+=2)
  ch_quad(crp(g,u-.78f,v+i*.93f,h-1.49f),crp(g,u+.78f,v+i*.93f,h-1.49f),crp(g,u+.78f,v+i*.93f,h+.29f),crp(g,u-.78f,v+i*.93f,h+.29f),flash?mix(color,PAPER,3):color,0);
}
static void cr_gate(float x,float y,float dx,float dy,U tint,int finish){
 float len=sq(d2(dx,dy));if(len<.05f){dx=1;dy=0;len=1;}
 CrProp g={x,y,dx/len,dy/len};
 cr_rect(&g,-2.30f,2.30f,-21.7f,21.7f,.13f,0x273a46,0);
 for(int i=-5;i<5;i++)for(int row=0;row<2;row++)
  cr_rect(&g,-1.82f+row*1.82f,-.08f+row*1.82f,i*4.15f,(i+1)*4.15f-.06f,.186f,((i+row)&1)?0x344651:PAPER,1);
 for(int side=-1;side<=1;side+=2){
  float v=side*22.4f,ax=x-v*g.s,ay=y+v*g.c;
  if(!solid(ax,ay,1))cr_beacon(&g,0,v,tint,finish,((int)(anim*4.f)+(side>0))%4==0);
 }
}
static void cr_pullin(float x,float y,U color,int pickup){
 /* Long paint brackets identify a stopping zone, not an obstacle. */
 CrProp g={x,y,1,0};
 U rim=mix(color,0x304652,3);
 for(int side=-1;side<=1;side+=2){
  float b0=side*7.f;
  cr_rect(&g,-19,19,b0-.59f,b0+.59f,.21f,rim,0);
  for(int k=-1;k<=1;k+=2){
   float a=k*19.f;cr_rect(&g,a-.54f,a+.54f,side<0?b0:b0-2.75f,side<0?b0+2.75f:b0,.21f,rim,0);
  }
  if(!pickup)continue;
  for(int k=0;k<3;k++){
   float u=side*(3.8f+k*3.7f);
   ch_quad(crp(&g,u-.98f,2.6f,.22f),crp(&g,u-2.02f,0,.22f),crp(&g,u-.98f,-2.6f,.22f),crp(&g,u+.06f,0,.22f),rim,0);
  }
 }
}
static void cr_cache_case(float x,float y,U color){
 CrProp g={x,y,.8944272f,.4472136f};
 cr_block(&g,-7.6f,7.6f,-4.7f,4.7f,.15f,4.2f,0x625347,0x9b6f4f);
 for(int s=-1;s<=1;s+=2){float v=s*3.45f;
  cr_block(&g,-7.7f,7.7f,v-.31f,v+.31f,3.95f,4.66f,0x393b40,0x56514a);
  cr_block(&g,-2.8f,-1.7f,v-.41f,v+.41f,4.56f,4.85f,0x97794e,color);
 }
 cr_block(&g,-2.3f,2.3f,-1.05f,1.05f,4.65f,5.12f,0x55574e,0xb89960);
}
static int crGoalProp,crGoalX,crGoalY;
static void cr_job_props(void){
 crGoalProp=0;
 if(scene!=1||menu>=0||p.dead||job.kind<0||job.step<0||job.step>=job.total||job_vehicle_target()>=0)return;
 if(job.kind!=JOB_SPRINT&&job.kind!=JOB_TAXI&&job.kind!=JOB_RECOVERY&&job.kind!=JOB_HOT&&job.kind!=JOB_RIVALS)return;
 NavPoint goal=nav_job_goal();float x=goal.x,y=goal.y;
 if(!cr_visible(x,y,1.5f,27,680))return;
 if(job.kind==JOB_SPRINT&&job.total>1){
  int a=job.step?job.step-1:0,b=job.step?job.step:1;
  cr_gate(x,y,job.xs[b]-job.xs[a],job.ys[b]-job.ys[a],job.step==job.total-1?GOLD:TEAL,job.step==0||job.step==job.total-1);
 }else if(job.kind==JOB_RIVALS&&job.step==0&&rivalsDeployed){cr_cache_case(x,y,GOLD);return;}
 else if(job.kind==JOB_TAXI||(job.step==1&&(job.kind==JOB_RECOVERY||job.kind==JOB_HOT))){
  if(taxi_via_copper())return; /* The marquee itself is the detour landmark. */
  cr_pullin(x,y,job.kind==JOB_HOT?PINK:TEAL,job.kind==JOB_TAXI&&job.step==0);
 }else return;
 crGoalProp=1;crGoalX=goal.x;crGoalY=goal.y;
}
static int ch_job_prop_visible(int x,int y){
 if(!chProjected||!crGoalProp||x!=crGoalX||y!=crGoalY)return 0;
 ChVec v=ch_local((ChVec){x,y,.25f});
 if(v.z<9.5f) return 0;
 ChPlot q=ch_screen(v);
 if(q.x<32||q.x>W-33||q.y<142||q.y>H-70||(q.x>=745&&q.y>=328))return 0;
 return q.z+195>=chZ[q.y*W+q.x];
}
static void cr_garage_props(void){
 int x,y;
 if(ng_route_goal(&x,&y)){
  if(!cr_visible(x,y,4,27,650))return;
  if(ng.event==NG_STAGE||ng.event==NG_COUNT||ng.event==NG_RACE){
   const NgRun *r=ngRun+ng.bookedCourse;int b=ng.event==NG_RACE?ng.leg:1,a=b>0?b-1:0;
   cr_gate(x,y,r->gate[b].x-r->gate[a].x,r->gate[b].y-r->gate[a].y,GOLD,ng.event!=NG_RACE||ng.leg==4);
  }else if(ng.event==NG_DROP)cr_pullin(x,y,GOLD,0);
  /* FETCH still uses the actual vehicle, not a post over its hood. */
 }else if(night_garage_meet_site(&x,&y)>=0&&job.kind<0&&vdActive<0&&cr_visible(x,y,4,25,350)){
  cr_pullin(x,y,TEAL,0);CrProp g={x,y,1,0};
  for(int side=-1;side<=1;side+=2)if(!solid(x+side*22.f,y+11,1))cr_beacon(&g,side*22,11,TEAL,1,0);
 }
}
static void cr_road_closed(void){
 if(!night_phone_lane_stop(2410,1024,2500,1024,4)||!cr_visible(2464,1024,6,61,640))return;
 CrProp g={2464,1024,0,1};
 for(int n=-3;n<=3;n++)cr_beacon(&g,n*15.f,0,0xf1b263,0,((int)(anim*3.f)+n)%3==0);
 for(int i=-7;i<7;i++)
  cr_block(&g,i*7.f,(i+1)*7.f,-.53f,.53f,3.05f,5.09f,(i&1)?0xd59459:0x634c43,(i&1)?0xeac186:0x583f3a);
}
static void cr_copper_marquee(void){
 int id=CAB_COPPER_BUILDING;float x=ordinaryDoorX[id];
 StreetBounds f=building_bounds(buildings+id);float face=f.y+f.h;
 if(ch.y<face||!cr_visible(x,face,30,46,460))return;
 float distance=d2(ch.x-x,ch.y-face);
 CrProp g={x,face,1,0};
 cr_block(&g,-32.2f,32.2f,.09f,4.15f,21.7f,33.9f,0x843f43,0xb47761);
 cr_block(&g,-29.3f,29.3f,3.14f,4.45f,23.8f,31.9f,0x302a41,0x9a725f);
 if(distance<350.f*350.f){
  cr_letters(&g,0,4.77f,31.15f,.55f,"ON STAGE",0xf7d5a0,0);
  cr_letters(&g,0,4.77f,27.75f,.74f,"MINA VALE",0xf3a0ae,0);
 }
 for(int i=-6;i<=6;i++){float u=i*4.78f;U l=((i+6+(int)(anim*3.f))%4)?0xf0c788:0xb67951;
  cr_block(&g,u-.46f,u+.46f,4.20f,4.88f,22.4f,23.25f,l,l);
  cr_block(&g,u-.46f,u+.46f,4.20f,4.88f,32.25f,33.10f,l,l);
 }
}
static void cr_outer_signs(void){
 for(int i=0;i<LEN(outerStops);i++){
  const OuterLandmark*q=outerStops+i;
  float x=outerNodes[q->node].x+17,y=outerNodes[q->node].y+12;
  if(!cr_visible(x,y,15,28,530))continue;
  float w=i<6?14.f:17.f,h=20.f;
  CrProp g={x,y,1,0};
  for(int side=-1;side<=1;side+=2){
   float u=side*(w-2);cr_block(&g,u-.50f,u+.50f,-.48f,.48f,0,h,0x576c6c,0x8a9a8b);
   cr_block(&g,u-1.23f,u+1.23f,-1.14f,1.14f,0,1.4f,0x4c5c63,0x728074);
  }
  cr_block(&g,-w,w,-.52f,.52f,13.5f,20.3f,0x344e59,q->tint);
  float y0=ch.y>=y?.62f:-.62f;
  ch_quad(crp(&g,-w+.66f,y0,14),crp(&g,w-.66f,y0,14),crp(&g,w-.66f,y0,19.65f),crp(&g,-w+.66f,y0,19.65f),mix(q->tint,0x202d38,7),1);
  if(d2(ch.x-x,ch.y-y)<244*244){
   float px=(w*2-3)/(sl(q->label)*4.f);if(px>.64f)px=.64f;
   cr_letters(&g,0,y0*1.19f,18.50f,px,q->label,q->tint,ch.y<y);
  }
 }
}
static void cr_signal_cart(void){
 int x,y;if(!night_venue_vendor_pos(&x,&y)||!cr_visible(x,y,9,37,600))return;
 /* Same prop coordinates as the original Night Cart, fitted with its own
  * lift-up stall, intake lighting and cargo service side. */
 float u=x-21,v=y+21;
 Car van={.x=u,.y=v,.a=1.65f,.type=4,.cond=100};ch_car_painted(&van,0,0x66567e);
 CrProp g={u,v,cs(van.a),sn(van.a)};
 int signSide=(y-v)*g.c-(x-u)*g.s>0?1:-1; /* always toward the real service point */
 float side=8.35f*signSide;
 cr_block(&g,-11,5.6f,side-.31f,side+.31f,6.6f,10.65f,0x313c4c,0xa17597);
 cr_block(&g,-10.55f,5.32f,side-.49f,side+.49f,5.7f,6.7f,0x7d526d,0xbc8d9f);
 float outer=side+signSide*4.5f;
 for(int k=0;k<5;k++){
  float l=-11.f+k*3.4f,r=l+3.4f;
  ch_quad(crp(&g,l,side,10.6f),crp(&g,r,side,10.6f),crp(&g,r,outer,11.65f),crp(&g,l,outer,11.65f),k&1?0x855b7f:0xb88da1,1);
 }
 cr_block(&g,-10.2f,4.95f,outer-.22f,outer+.22f,10.75f,11.58f,0x915e8d,0xc689b5);
 if(d2(x-ch.x,y-ch.y)<245*245){
  cr_letters(&g,-2.4f,side+signSide*.60f,10.0f,.34f,"NIGHT SIGNAL",0xf6b8d9,signSide>0);
  CrProp fascia={u+3.9f*g.c,v+3.9f*g.s,-g.s,g.c};
  cr_block(&fascia,-5.55f,5.55f,-.68f,.2f,12.1f,14.3f,0x493d54,0xb573a2);
  if((ch.x-u)*g.c+(ch.y-v)*g.s>=0)
   cr_letters(&fascia,0,-.75f,14.03f,.22f,"NIGHT SIGNAL",0xffc6ec,1);
 }
 CrProp t={x,y,1,0};cr_block(&t,-2,2,-.72f,.72f,0,4.3f,0x5d4959,0xba83a8);
 cr_block(&t,-3.65f,3.65f,-.78f,.78f,4.15f,6.9f,0x72526c,0xfaace0);
}
