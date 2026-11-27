/* Render-facing, distance-driven foot cycle. The C preference changes only
 * how fast on-foot input asks the existing sweeps to travel. It does not
 * change interaction radii, injury, clocks, steering, or any saved fields.
 * The Low FX renderer is a fallback, not a second movement mode. */
enum{GAIT_STREET=1,GAIT_NAMED=3,GAIT_PUBLIC=4,GAIT_CLUB=7,GAIT_RAIL=10,GAIT_TRAP=11};
typedef struct{float fore,side,lift,swing,bend;int planted;} GaitFoot;
typedef struct{GaitFoot foot[2];float bob,lean,sway,turn;int valid;} GaitPose;
static struct {
 U frame;
 int mode,key,paced;
 float ease,run,phase,weight,angle,x,y,dir;
} gait;
static U gaitFrame;
static float gait_lerp(float a,float b,float t){return a+(b-a)*t;}
static float gait_unit(float x){return x<0?0:x>1?1:x;}
static float gait_cycle(float run){return 36.4f+9.2f*run;}
static void gait_reset(void){gait.mode=gait.key=gait.paced=0;gait.frame=gaitFrame-60;}
static void gait_prepare(int mode,int key,float x,float y,float a){
 if(gait.mode!=mode||gait.key!=key||gait.frame+1!=gaitFrame||d2(gait.x-x,gait.y-y)>.04f){
  gait.mode=mode;gait.key=key;gait.ease=.26f;gait.run=0;gait.phase=.36f;
  gait.weight=0;gait.angle=gait.dir=a;gait.paced=0;
 }
}
static float gait_pace(int mode,int key,float x,float y,float angle,int moving,int sprint){
 if(!ch_pref())return 1.f;
 gait_prepare(mode,key,x,y,angle);
 float factor=mode==GAIT_STREET?(sprint?.75f:.69f):mode==GAIT_RAIL?.61f:(sprint?.72f:.65f);
 /* Ease total world pace, not just the lens modifier. Shift changes
    the ordinary base stride at each call site; divide that multiplier
    back out so neither engaging nor releasing it skips acceleration. */
 float ratio=!sprint?1.f:mode==GAIT_STREET?FOOT_SPRINT_SPEED/FOOT_WALK_SPEED:
    mode==GAIT_NAMED?NAMED_RUN_STEP/NAMED_WALK_STEP:mode==GAIT_TRAP?3.15f/2.05f:
    mode==GAIT_RAIL?1.f:WALKIN_RUN_STEP/WALKIN_WALK_STEP;
 gait.ease=gait_lerp(gait.ease,moving?factor*ratio:.26f,moving?.25f:.44f);
 gait.run=gait_lerp(gait.run,moving&&sprint?1.f:0.f,.23f);
 gait.paced=1;
 return gait.ease/ratio;
}
static void gait_finish(int mode,int key,float x,float y,float ox,float oy,float a){
 if(!ch_pref())return;
 if(!gait.paced)gait_prepare(mode,key,ox,oy,a);
 gait.paced=0;gait.frame=gaitFrame;gait.x=x;gait.y=y;
 float dx=x-ox,dy=y-oy,len=sq(d2(dx,dy));
 if(len<.018f){
  gait.weight*=.62f;gait.angle=wrap(gait.angle+.47f*wrap(a-gait.angle));
  if(gait.weight<.012f)gait.weight=0;return;
 }
 float heading=look(dx,dy),err=wrap(heading-gait.dir),ang=wrap(a-gait.angle);
 if(absf(err)>1.23f)gait.weight*=.53f; /* land the old swing before pivoting */
 gait.dir=heading;gait.angle=wrap(gait.angle+ang*.47f);
 gait.weight+=.26f*(1.f-gait.weight);if(gait.weight>.975f)gait.weight=1;
 float scale=mode==GAIT_STREET?1.f:1.78f;
 gait.phase+=len/(scale*gait_cycle(gait.run));if(gait.phase>=1)gait.phase-=1;
}
static GaitPose gait_pose(int mode,float x,float y,float heading){
 GaitPose out={0};if(!ch_pref())return out;
 out.valid=1;
 if(gait.mode!=mode||gait.frame!=gaitFrame||d2(x-gait.x,y-gait.y)>.09f)return out;
 float blend=gait.weight,pace=gait.run,period=gait_cycle(pace),stance=.57f-.07f*pace;
 float travel=wrap(gait.dir-heading),co=cs(travel),si=sn(travel);
 float contact=3.48f+.46f*pace;
 /* The transfer at each end of the stance rolls to heel or toes.
    Through the flat-foot portion the shoe counter-travels by exactly
    the collision-resolved root displacement. Ends shorten the swing
    arc without a frantic leg cycle disproportionate to scene scale. */
 float roll=.09f;
 float amp=contact+(period*stance*.5f-contact)*roll;
 out.turn=wrap(gait.angle-heading);out.lean=blend*(.12f+.36f*pace);
 out.bob=blend*(.12f+.33f*pace)*(-.4f-.6f*cs(gait.phase*12.5663706f));
 out.sway=blend*.21f*sn(gait.phase*6.2831853f);
 for(int i=0;i<2;i++){
  float f=gait.phase+i*.5f;if(f>=1.f)f-=1;
  float u=period*(stance*.5f-f),lift=0,bend=.42f;int planted=0;
  if(f<=stance){
   float d=absf(u)-contact;
   if(d<=0)planted=1;
   else {u=(u<0?-1.f:1.f)*(contact+d*roll);lift=(d/(period*stance*.5f-contact))*.19f;}
  }
  if(f>stance){float k=gait_unit((f-stance)/(1.f-stance)),s=k*k*(3.f-2.f*k);
   u=gait_lerp(-amp,amp,s);lift=.19f+sn(k*3.14159265f)*(1.05f+1.18f*pace);
   bend+=lift*.29f;
  }
  out.foot[i]=(GaitFoot){co*u*blend,si*u*blend,lift*blend,-u*blend*(.26f+.07f*pace),bend*blend,planted};
 }
 return out;
}
