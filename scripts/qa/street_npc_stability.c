/* Render and stress the physical street simulation. No career is opened.
   clang -O1 -fsanitize=address,undefined -fno-sanitize-recover=all -no-pie
      scripts/qa/street_npc_stability.c -o build/street-npc && build/street-npc
   Pass --observe to run the same measured checks on the old renderer. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const float frame=.016666667f;
static void init_neutral(float x,float y){
 init_city();
 starter_save();sv.intro=4;sv.dayClock=710;scene=1;menu=-1;end_job();clear_input();
 p=(Player){.x=x,.y=y,.car=-1,.a=-1.5707963f,.hp=100};
}
static void clear_entities(void){
 for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=999};
 z(streetDrivers,sizeof streetDrivers);z(streetWalks,sizeof streetWalks);
 nc=1;npc_reset();
}
static void pose_pair(int i){
 StreetPerson v,w;assert(street_ped_pose(i,&v));
 U a=night_hash(peds,sizeof peds),c=night_hash(streetWalks,sizeof streetWalks);
 /* The two production renderers get the same pose/clock/foot anchor. */
 nv.garage[60]=1;paint();assert(chProjected&&street_ped_pose(i,&w));
 assert(v.x==w.x&&v.y==w.y&&v.a==w.a&&v.step==w.step);
 nv.garage[60]=0;paint();assert(!chProjected&&street_ped_pose(i,&w));
 assert(v.x==w.x&&v.y==w.y&&v.a==w.a&&v.step==w.step);
 assert(a==night_hash(peds,sizeof peds)&&c==night_hash(streetWalks,sizeof streetWalks));
}
static void fixture(int observe){
 init_neutral(1308,792);clear_entities();
 peds[0]=(Ped){.x=1330.20f,.y=730.f,.a=3.141593f,.hp=2};
 streetWalks[0]=(StreetWalk){.mx=1318,.my=730,.tx=1318,.ty=730,.leg=STREET_EN_ROUTE,.ax=1318,.ay=730};
 peds[1]=(Ped){.x=1318.f,.y=754.f,.a=-1.5707963f,.hp=2};
 streetWalks[1]=(StreetWalk){.cross=1,.crossX=1318,.crossY=806,.mx=1318,.my=730,.tx=1318,.ty=730};
 assert(!solid(peds[0].x,peds[0].y,4)&&!solid(peds[1].x,peds[1].y,4));
 float maxTurn=0;int backward=0;
 for(int n=0;n<72;n++){
  float x=peds[0].x,a=peds[0].a;street_tick_civilian(0,frame);
  float turn=absf(wrap(peds[0].a-a));if(turn>maxTurn)maxTurn=turn;
  if(peds[0].x<x-.015f)backward++;
  assert(!solid(peds[0].x,peds[0].y,4));
 }
 printf("72 ticks at an occupied curb: largest turn %.1f°, backward steps %d.\n",maxTurn*57.29578f,backward);
 if(!observe){assert(maxTurn<.1301f&&backward==0&&peds[0].s==0);pose_pair(0);}
 /* Do not step through a stationary car even after the wait clears. */
 peds[1].hp=0;streetWalks[1].cross=0;
 for(int n=0;n<20;n++)street_tick_civilian(0,frame);
 if(!observe){
  float x=peds[0].x;
  for(int n=0;n<6;n++)street_tick_civilian(0,frame);
  assert(peds[0].x<x); /* planned route now resumes */
 }
 peds[0]=(Ped){.x=730,.y=730,.a=0,.hp=2};
 streetWalks[0]=(StreetWalk){.mx=730,.my=730,.tx=806,.ty=730,.ax=806,.ay=730,.leg=STREET_EN_ROUTE};
 nc=2;cars[1]=(Car){.x=768,.y=730,.a=1.5707963f,.cond=100};
 for(int n=0;n<50;n++){street_tick_civilian(0,frame);assert(peds[0].x==730&&peds[0].y==730);}
 cars[1]=(Car){.x=-100,.y=-100};
 for(int n=0;n<145&&peds[0].x<805.9f;n++)street_tick_civilian(0,frame);
 assert(peds[0].x>=805.9f&&peds[0].y==730);
 puts("Parked car holds pedestrian at the curb; once clear, pedestrian traverses the original 76-unit zebra.");
}
static void retreat(void){
 /* A returning walker needs the full 12-unit gap at the old curb. Two
    previously observed corners jammed forever when yield was capped at
    8.4 units even though room to take a wider safe step existed. */
 static const int sites[][3]={{294,474,502},{806,550,523}};
 for(int j=0;j<LEN(sites);j++){
  clear_entities();
  int x=sites[j][0],y=sites[j][1],from=sites[j][2],sign=from>y?1:-1;
  peds[0]=(Ped){.x=x,.y=y,.hp=2};
  streetWalks[0]=(StreetWalk){.mx=x,.my=y,.tx=x,.ty=y,.ax=x,.ay=y,.leg=STREET_EN_ROUTE,.pause=2.9f};
  peds[1]=(Ped){.x=x,.y=from,.a=-sign*1.5707963f,.hp=2};
  streetWalks[1]=(StreetWalk){.cross=2,.crossX=x,.crossY=y,.mx=x,.my=y,.tx=x,.ty=y,.ax=x,.ay=y};
  int done=0;
  for(int n=0;n<125;n++){
   street_tick_civilian(0,frame);street_tick_civilian(1,frame);
   if(!streetWalks[1].cross&&absf(peds[1].y-y)<.5f){done=1;break;}
  }
  assert(done&&absf(peds[0].x-x)>=12.f&&peds[0].hp&&peds[1].hp);
 }
 puts("Two recorded curb jams now let the retreating walker finish the crossing.");
}
static void natural(int observe){
 init_neutral(256,512);outer_life_reset();
 enum{TICKS=3600};
 int last[STREET_CIVILIANS],reverse[STREET_CIVILIANS],flips=0,shuffles=0,moved=0,idle=0;
 float vx[STREET_CIVILIANS]={0},vy[STREET_CIVILIANS]={0};
 float prev[STREET_CIVILIANS],x[STREET_CIVILIANS],y[STREET_CIVILIANS],maxTurn=0;
 for(int i=0;i<STREET_CIVILIANS;i++)last[i]=reverse[i]=-99;
 for(int t=0;t<TICKS;t++){
  for(int i=0;i<STREET_CIVILIANS;i++){prev[i]=peds[i].a;x[i]=peds[i].x;y[i]=peds[i].y;}
  tick(t==1||t==885?CAMERA:0);
  if(t==150){assert(ch_mode_active());paint();assert(chProjected);}
  if(t==950){assert(!ch_mode_active());paint();assert(!chProjected);}
  for(int i=0;i<STREET_CIVILIANS;i++)if(peds[i].hp>0){
   float a=absf(wrap(peds[i].a-prev[i])),d=d2(peds[i].x-x[i],peds[i].y-y[i]);
   if(a>maxTurn)maxTurn=a;
   if(a>1.04f){flips+=t-last[i]<=2;last[i]=t;}
   float dx=peds[i].x-x[i],dy=peds[i].y-y[i],prior=d2(vx[i],vy[i]);
   if(d>.035f&&prior>.035f&&dx*vx[i]+dy*vy[i]<-.92f*sq(d*prior)){
    shuffles+=t-reverse[i]<=2;reverse[i]=t;
   }
   vx[i]=dx;vy[i]=dy;
   if(d>.0004f)moved++;else idle++;
  }
 }
 printf("%d ticks on natural city schedules: %d repeated >60° turns, %d repeated travel reversals; %.1f° max; %d moving and %d idle actor samples.\n",
  TICKS,flips,shuffles,maxTurn*57.29578f,moved,idle);
 if(!observe)assert(!flips&&!shuffles&&maxTurn<.1301f&&moved>10000&&idle>3000);
 /* East-road expansions retain all physical identities and smooth headings
    on the surveyed shoulders. Both poses read the same actor owner. */
 p.x=2770;p.y=1120;clear_input();tick(CAMERA);tick(0);
 assert(ch_mode_active()&&outer_walk_count()==24&&outCrowdCount==24);
 float distance=0,walked[OUT_WALK_COUNT]={0};
 for(int t=0;t<600;t++){
  OuterWalker old[OUT_WALK_COUNT];memcpy(old,outWalkers,sizeof old);tick(0);
  for(int i=0;i<OUT_WALK_COUNT;i++){
   float a=absf(wrap(outWalkers[i].a-old[i].a));
   assert(a<.095f&&outer_walk_away_from_lane(outWalkers[i].x,outWalkers[i].y));
   walked[i]+=sq(d2(outWalkers[i].x-old[i].x,outWalkers[i].y-old[i].y));
  }
 }
 int active=0;for(int i=0;i<OUT_WALK_COUNT;i++){distance+=walked[i];active+=walked[i]>5.f;}
 assert(active>=12&&distance>350.f);paint();assert(chProjected);
 printf("All 24 expanded-road pedestrians remain on surveyed shoulders; %.0f world units walked in 600 ticks.\n",distance);
}
int main(int argc,char**argv){
 int observe=argc>1&&!strcmp(argv[1],"--observe");
 fixture(observe);if(!observe)retreat();natural(observe);
 return 0;
}
