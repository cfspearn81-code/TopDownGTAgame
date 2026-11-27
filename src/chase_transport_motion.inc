/* Fixed-step visual load and wheel travel. Samples are only for extant road
 * vehicles; never write back to Car/Player, the collision model or a career.
 * Pause and menus don't wind the spokes or compress the springs. */
typedef struct{
 float x,y,a,s,roll,pitch,heave,steer,phase,settle;
 U tick;int type,cond;
} CvMotion;
static CvMotion cvMotion[LEN(cars)];
static U cvLastRoad;
static float cv_clamp(float a,float lo,float hi){return a<lo?lo:a>hi?hi:a;}
static void cv_world_motion(void){
 if(scene!=1||!ch_pref()){cvLastRoad=0;return;}
 if(roadNow==cvLastRoad)return;
 if(!cvLastRoad||roadNow<cvLastRoad)for(int i=0;i<LEN(cars);i++)cvMotion[i].tick=0;
 U serial=roadNow;cvLastRoad=serial;
 for(int i=street_next_car(-1);i<LEN(cars);i=street_next_car(i)){
  const Car*c=cars+i;CvMotion*m=cvMotion+i;
  if(c->x<0)continue;
  float dx=c->x-m->x,dy=c->y-m->y;
  if(!m->tick||serial!=m->tick+1u||m->type!=c->type||d2(dx,dy)>6.4f*6.4f){
   *m=(CvMotion){.x=c->x,.y=c->y,.a=c->a,.s=c->s,.tick=serial,.type=c->type,.cond=c->cond};
   continue;
  }
  float da=wrap(c->a-m->a),ds=c->s-m->s,v=absf(c->s),roadSpeed=cv_clamp(v*.0125f,0,1.f);
  float targetRoll=cv_clamp(da*c->s*.015f,-.068f,.068f);
  float targetPitch=cv_clamp(-ds*.0104f,-.055f,.056f);
  float bump=(m->cond-c->cond)*.037f;if(bump<0)bump=0;
  if(bump>m->settle)m->settle=cv_clamp(bump,0,.96f);
  m->settle*=.85f;
  float targetHeave=-m->settle+sn(m->phase*2.f)*roadSpeed*.09f;
  m->roll+=(targetRoll-m->roll)*.19f;
  m->pitch+=(targetPitch-m->pitch)*.26f;
  m->heave+=(targetHeave-m->heave)*.25f;
  float turn=cv_clamp(da*11.f*(c->s<-2.f?-1.f:1.f),-.44f,.44f);
  m->steer+=(turn-m->steer)*.32f;
  float travel=dx*cs(c->a)+dy*sn(c->a);
  m->phase=wrap(m->phase+travel/(c->type==3?2.35f:2.64f));
  m->x=c->x;m->y=c->y;m->a=c->a;m->s=c->s;m->tick=serial;m->cond=c->cond;
 }
}
static const CvMotion*cv_motion(const Car*c){
 /* ch_car also draws a stack-local photo ghost and traveler props. Check
  * numerical address and alignment before indexing real motion slots. */
 __UINTPTR_TYPE__ key=(__UINTPTR_TYPE__)c,begin=(__UINTPTR_TYPE__)cars;
 if(key<begin||key-begin>=sizeof cars||(key-begin)%sizeof(Car))return 0;
 const CvMotion*m=cvMotion+(key-begin)/sizeof(Car);
 return m->tick==roadNow&&m->type==c->type&&d2(c->x-m->x,c->y-m->y)<.01f?m:0;
}
