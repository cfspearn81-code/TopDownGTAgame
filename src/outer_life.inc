/* Seven stable physical chassis; no aliasing the city, owned or job range.
 * Detour decisions change a route at its entrance rather than relocating a car.
 * Floating point lane waypoints derive solely from the existing road survey. */
static const unsigned char outMain[]={16,18,22,24,23};
static const unsigned char outDetour[]={16,26,25,21,18,22,24,23};
static const unsigned char outBluff[]={2,3,4,5,6,7,8,9,10,11,12,13,21};
static const unsigned char outHarbor[]={31,32,33,34,35,36,37,38,39,40,41,42,43,44,45};
typedef struct{float x,y;} OutLanePoint;
typedef struct{int n;OutLanePoint point[64];} OutLane;
typedef struct{int route,next,driven,yielding;float outOfSight;} OutDriver;
static OutLane outLane[4];
static OutDriver outDriver[OUT_TRAFFIC_COUNT];
static int outCrowdCount;
static void out_prepare_lanes(void);
typedef struct{int damage;float x,y,down,flash;} OuterCondition;
static OuterCondition outBody[N_ACT_OUTER_END-N_ACT_OUTER];
#define DC_OUTER_WALK_POSES 1
/* Fixed world identities. Each walker stays on a surveyed shoulder/stroll
 * segment, then pauses at a distinct end point. No viewport-time loops. */
enum{OUT_WALK_COUNT=24};
typedef struct{
 float x,y,a,x0,y0,x1,y1,pace,pause,stride;
 int ready,leg;
} OuterWalker;
static OuterWalker outWalkers[OUT_WALK_COUNT];
static int outWalkReady;
_Static_assert(OUT_WALK_COUNT==N_ACT_OUTER_END-N_ACT_OUTER,"each surveyed walker keeps its original regional NPC identity");
static int outer_walk_count(void);
static int outer_walk_pose(int,float*,float*,float*,int*,U*);
static int outer_walk_clear(float x,float y){
 /* A side-road shoulder can cross the middle of a wider road.
  * The baked union's deepest asphalt band wins at every intersection;
  * walk paths cannot mistake that band for a shoulder. */
 return outer_is_new(x,y)&&outer_walkable(x,y,4.f)&&!solid(x,y,4.f)&&
        outerInk[(int)y*ROOM+(int)x]<=12;
}
static int outer_walk_check(float x,float y,float xx,float yy){
 float dx=xx-x,dy=yy-y;int n=1+(int)(sq(d2(dx,dy))/3.f);
 for(int i=0;i<=n;i++)if(!outer_walk_clear(x+dx*i/n,y+dy*i/n))return 0;
 return 1;
}
static int outer_walk_away_from_lane(float x,float y){
 /* The old decorative cars could pass through a shoulder card. The
  * reserved physical traffic must not turn into (or permanently yield
  * to) a stroller. Survey both the open and closed swept lane centers. */
 for(int i=0;i<LEN(outLane);i++){
  const OutLane*l=outLane+i;
  for(int j=0;j<l->n;j++){
   OutLanePoint a=l->point[j],b=l->point[(j+1)%l->n];
   const float left=a.x<b.x?a.x:b.x,right=a.x>b.x?a.x:b.x;
   const float top=a.y<b.y?a.y:b.y,bottom=a.y>b.y?a.y:b.y;
   if(x<left-24.f||x>right+24.f||y<top-24.f||y>bottom+24.f)continue;
   float dx=b.x-a.x,dy=b.y-a.y,d=d2(dx,dy),t=d>0?((x-a.x)*dx+(y-a.y)*dy)/d:0.f;
   if(t<0)t=0;if(t>1)t=1;
   if(d2(x-a.x-dx*t,y-a.y-dy*t)<24.f*24.f)return 0;
  }
 }
 return 1;
}
static void outer_walk_init(void){
 static const int sites[]={10,19,25,36,45,54};
 out_prepare_lanes();
 z(outWalkers,sizeof outWalkers);outCrowdCount=0;outWalkReady=1;
 for(int k=0;k<LEN(sites);k++)for(int j=0;j<4;j++){
  int id=k*4+j,side=(j&2)?-1:1;
  OuterWalker*w=outWalkers+id;
  float best=0;int site=sites[k];const OuterPoint*near=outerNodes+site;
  for(int n=0;n<LEN(outerRoads);n++){
   const OuterRoad*r=outerRoads+n;
   int r0=r->a,r1=r->b;
   float gap=sq(d2(outerNodes[r0].x-near->x,outerNodes[r0].y-near->y));
   float alt=sq(d2(outerNodes[r1].x-near->x,outerNodes[r1].y-near->y));
   if(alt<gap){int t=r0;r0=r1;r1=t;gap=alt;}
   if(gap>158.f)continue;
   const OuterPoint*a=outerNodes+r0,*b=outerNodes+r1;
   float dx=b->x-a->x,dy=b->y-a->y,length=sq(d2(dx,dy));if(length<56.f)continue;
   float u=dx/length,v=dy/length;
   static const float offsets[]={8.f,13.f,2.f,19.f,25.f,-5.f,-8.f,33.f};
   for(int off=0;off<LEN(offsets);off++){
    float yoff=side*(r->half+offsets[off]);
    float x=a->x-v*yoff,y=a->y+u*yoff;
    /* Only award this slot's half; adjacent walkers cannot lap or meet
     * head-to-head at the same station. Leave a full body space at the seam. */
    float lo=(j&1)?length*.5f+11.f:12.f,hi=(j&1)?length-12.f:length*.5f-11.f;
    if(hi-lo<30.f)continue;
    float bestStart=0,bestEnd=0,open=-1;
    for(float d=lo;d<=hi;d+=3.f){
     if(outer_walk_clear(x+u*d,y+v*d)&&outer_walk_away_from_lane(x+u*d,y+v*d)){
      if(open<0)open=d;
      if(d-open>bestEnd-bestStart){bestStart=open;bestEnd=d;}
     }else open=-1;
    }
    float span=bestEnd-bestStart;if(span<29.f)continue;
    float score=span-gap*.20f-off*4.f;
    if(score<=best||!outer_walk_check(x+u*bestStart,y+v*bestStart,x+u*bestEnd,y+v*bestEnd))continue;
    /* Other station shoulders may share a real road bend; reject
     * overlapping templates once, not by blinking out passing actors. */
    int overlap=0;
    for(int other=0;other<id;other++){
     const OuterWalker*q=outWalkers+other;if(!q->ready)continue;
     for(int m=0;m<=8&&!overlap;m++)for(int nn=0;nn<=8;nn++){
      float d=bestStart+span*m/8.f;
      float qx=q->x0+(q->x1-q->x0)*nn/8.f,qy=q->y0+(q->y1-q->y0)*nn/8.f;
      if(d2(x+u*d-qx,y+v*d-qy)<13.f*13.f){overlap=1;break;}
     }
    }
    if(overlap)continue;
    best=score;w->ready=1;w->x0=x+u*bestStart;w->y0=y+v*bestStart;w->x1=x+u*bestEnd;w->y1=y+v*bestEnd;
   }
  }
  if(w->ready){
   float begin=.15f+(id%7)*.105f;w->x=w->x0+(w->x1-w->x0)*begin;w->y=w->y0+(w->y1-w->y0)*begin;
   w->leg=id&1;w->pace=(24.f+(id%6)*2.3f)*FOOT_PACE;w->pause=(id%6)*.34f;
   w->a=look(w->leg?w->x1-w->x:w->x0-w->x,w->leg?w->y1-w->y:w->y0-w->y);outCrowdCount++;
  }
 }
}
static void outer_walk_tick(float dt){
 for(int i=0;i<LEN(outWalkers);i++){
  OuterWalker*w=outWalkers+i;int id=N_ACT_OUTER+i;
  if(!w->ready||outBody[i].damage==3||npc_holds(id)||npc_replaces(id))continue;
  if(w->pause>0){w->pause-=dt;continue;}
  float x=w->leg?w->x1:w->x0,y=w->leg?w->y1:w->y0;
  float dx=x-w->x,dy=y-w->y,n=sq(d2(dx,dy));
  if(n<.03f){w->leg^=1;w->pause=1.7f+(i%7)*.47f;continue;}
  float m=w->pace*dt;if(m>n)m=n;
  x=w->x+dx*m/n;y=w->y+dy*m/n;
  float facing=look(dx,dy),turn=wrap(facing-w->a),steer=dt*5.4f;
  if(turn>steer)turn=steer;if(turn<-steer)turn=-steer;
  w->a=wrap(w->a+turn);
  /* Wait on the shoulder rather than squeezing through a stopped
   * car, the player or the walker at the next bend. */
  if(absf(wrap(facing-w->a))>.30f||!outer_walk_clear(x,y)){w->pause=.10f;continue;}
  if(!street_step_past_cars(w->x,w->y,x,y,3.f)){w->pause=.18f;continue;}
  int blocked=0;
  for(int k=0;k<LEN(outWalkers)&&!blocked;k++)if(k!=i&&outWalkers[k].ready&&outBody[k].damage!=3){
   float dd=d2(outWalkers[k].x-x,outWalkers[k].y-y);
   if(dd<11.f*11.f&&dd<d2(outWalkers[k].x-w->x,outWalkers[k].y-w->y))blocked=1;
  }
  if(p.car<0&&!p.dead&&d2(p.x-x,p.y-y)<12.f*12.f&&d2(p.x-x,p.y-y)<d2(p.x-w->x,p.y-w->y))blocked=1;
  if(blocked){w->pause=.19f;continue;}
  w->x=x;w->y=y;w->stride+=m;if(w->stride>=25.13274f)w->stride-=25.13274f;
 }
}
static int outer_walk_count(void){return outWalkReady?LEN(outWalkers):0;}
/* 3HP for unarmoured civilians, as on the city blocks. These short-lived
   states, like the original downtown bodies, are not career data. */
static int outer_injure(int id,int damage,float x,float y){
 if(id<N_ACT_OUTER||id>=N_ACT_OUTER_END)return street_extra_injure(id,damage,x,y);
 if(damage<=0)return 0;
 OuterCondition*b=outBody+(id-N_ACT_OUTER);
 if(b->damage>=3)return 0;
 b->damage+=damage;b->x=x;b->y=y;b->flash=.22f;
 if(b->damage<3)return 0;
 b->damage=3;b->down=12.f;return 1;
}
static void outer_condition_tick(float dt){
 for(int j=0;j<LEN(outBody);j++){
  OuterCondition*b=outBody+j;
  if(b->flash>0)b->flash-=dt;
  if(b->damage!=3)continue;
  if(b->down>0)b->down-=dt;
  if(b->down<=0&&!street_on_camera(b->x,b->y)&&d2(p.x-b->x,p.y-b->y)>660.f*660.f)*b=(OuterCondition){0};
 }
}
static int outNight(void){return sv.dayClock<348.f||sv.dayClock>=1120.f;}
static int outer_person_ref(int,NpcRef*,int*);
static void outer_life_reset(void){
 z(outBody,sizeof outBody);z(outDriver,sizeof outDriver);z(outWalkers,sizeof outWalkers);outCrowdCount=outWalkReady=0;
 for(int i=STREET_CITY_CAR_LIMIT;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};
}
static void outer_take_wheel(int id){
 if(id>=STREET_CITY_CAR_LIMIT&&id<LEN(cars)){OutDriver*d=outDriver+(id-STREET_CITY_CAR_LIMIT);d->driven=1;d->outOfSight=0;}
}
static void out_lane_bake(OutLane*l,const unsigned char *route,int n,int loop){
 const int lines=loop?n:(n-1)*2;
 for(int j=0;j<lines;j++){
  int i=j<n?j:2*n-2-j,k=j+1<n?j+1:2*n-3-j;
  if(loop)k=(j+1)%n;
  OuterPoint a=outerNodes[route[i]],b=outerNodes[route[k]];
  float dx=b.x-a.x,dy=b.y-a.y,r=sq(d2(dx,dy)),u=dx/r,v=dy/r;
  const float lead=22.f,offset=l==outLane+2?10.4f:9.f;
  l->point[l->n++]=(OutLanePoint){a.x+u*lead-v*offset,a.y+v*lead+u*offset};
  l->point[l->n++]=(OutLanePoint){b.x-u*lead-v*offset,b.y-v*lead+u*offset};
  if(!loop&&(k==0||k==n-1)){
   l->point[l->n++]=(OutLanePoint){b.x+u*9.f-v*7.f,b.y+v*9.f+u*7.f};
   l->point[l->n++]=(OutLanePoint){b.x+u*9.f+v*7.f,b.y+v*9.f-u*7.f};
  }
 }
 if(l==outLane+2){
  /* At Glass Bluff the crowd uses the lookout's south shoulder. A
     surveyed inlet at the north curb keeps both lanes off that walk. */
  for(int i=0;i<l->n;i++){
   OutLanePoint*q=l->point+i;
   float z=(170.f-sq(d2(q->x-2690.f,q->y-384.f)))/90.f;
   if(z>0)q->y-=11.f*(z>1.f?1.f:z);
  }
 }
}
static void out_prepare_lanes(void){
 if(outLane[0].n)return;
 out_lane_bake(outLane,outMain,LEN(outMain),1);
 out_lane_bake(outLane+1,outDetour,LEN(outDetour),1);
 out_lane_bake(outLane+2,outBluff,LEN(outBluff),0);
 out_lane_bake(outLane+3,outHarbor,LEN(outHarbor),0);
}
static int out_seen(const Car*c){
 /* Vehicle remains intact well beyond both the overhead and chase views. */
 return d2(p.x-c->x,p.y-c->y)<830.f*830.f;
}
static OutLanePoint outFeet[1+LEN(peds)+N_ACT_OUTER_END-N_ACT_OUTER];
static int outFeetCount,outPlayerFoot;
static void out_sample_feet(void){
 outFeetCount=0;outPlayerFoot=p.car<0&&!p.dead;
 if(outPlayerFoot)outFeet[outFeetCount++]=(OutLanePoint){p.x,p.y};
 for(int i=0;i<LEN(peds);i++)if(peds[i].hp>0&&outer_is_new(peds[i].x+60.f,peds[i].y+60.f))
  outFeet[outFeetCount++]=(OutLanePoint){peds[i].x,peds[i].y};
 for(int id=N_ACT_OUTER;id<N_ACT_OUTER_END;id++){
  NpcRef q;if(outer_person_ref(id,&q,0))outFeet[outFeetCount++]=(OutLanePoint){q.x,q.y};
 }
}
static int out_yield(const Car*c,float u,float v,float limit,float radius){
 for(int j=0;j<outFeetCount;j++){
  OutLanePoint q=outFeet[j];float dx=q.x-c->x,dy=q.y-c->y;
  if(d2(dx,dy)>=limit*limit+radius*radius)continue;
  float along=dx*u+dy*v,side=absf(dy*u-dx*v);
  if(along>=-8.f&&along<limit&&side<radius)return 1;
 }
 return 0;
}
static int out_body_blocked(const Car*old,const Car*next){
 for(int i=0;i<outFeetCount;i++){
  OutLanePoint q=outFeet[i];
  if(d2(q.x-next->x,q.y-next->y)>28.f*28.f)continue;
  float radius=i==0&&outPlayerFoot?7.f:3.f;
  float depth=street_foot_car_depth(next,q.x,q.y,radius);
  if(depth>0.f&&depth>=street_foot_car_depth(old,q.x,q.y,radius))return 1;
 }
 return 0;
}
static int out_seed(int slot,int hidden){
 int id=STREET_CITY_CAR_LIMIT+slot;
 int district=slot%3,route=district==1?2:district==2?3:np_closed()?1:0;
 OutLane*l=outLane+route;
 for(int i=0;i<l->n;i++){
  int j=(i+1+(l->n*(slot/3))/3+district*3)%l->n,k=(j+1)%l->n;
  OutLanePoint a=l->point[j],b=l->point[k];float dx=b.x-a.x,dy=b.y-a.y;
  Car c={a.x+dx*.22f,a.y+dy*.22f,look(dx,dy),25,(slot+1)%LEN(specs),0,100};
  float co=cs(c.a),si=sn(c.a);
  if((hidden?out_seen(&c):d2(c.x-p.x,c.y-p.y)<84.f*84.f)||!street_car_clear(id,0,&c)||
      out_yield(&c,co,si,54.f,13.f)||night_phone_lane_stop(a.x,a.y,c.x,c.y,17))continue;
  cars[id]=c;outDriver[slot]=(OutDriver){.route=route,.next=k};return 1;
 }
 return 0;
}
static void out_drive(int slot,float dt){
 const int id=STREET_CITY_CAR_LIMIT+slot;
 Car*c=cars+id;OutDriver*d=outDriver+slot;
 OutLane*l=outLane+d->route;OutLanePoint q=l->point[d->next];
 float dx=q.x-c->x,dy=q.y-c->y,dist=sq(d2(dx,dy));
 if(dist<.04f){
  /* Route 0 and 1 share the same lane well before their fork. */
  if(++d->next>=l->n){d->next=0;if(d->route<2)d->route=np_closed();}
  return;
 }
 float u=dx/dist,v=dy/dist,angle=look(dx,dy),turn=wrap(angle-c->a);
 float cap=3.6f*dt,pace=39.f+slot%3*3.f;
 float newAngle=wrap(c->a+(turn>cap?cap:turn< -cap?-cap:turn));
 if(absf(turn)>.19f)pace=absf(turn)>.89f?9.f:22.f;
 Car probe=*c;float lead=8.f+c->s*.24f;
 if(d->yielding&&lead<11.f)lead=11.f;
 if(lead>dist)lead=dist;
 probe.a=newAngle;probe.x+=u*lead;probe.y+=v*lead;
 float footLook=23.f+c->s*.57f;
 if(d->yielding&&footLook<29.f)footLook=29.f;
 d->yielding=!street_car_clear(id,c,&probe)||night_phone_lane_stop(c->x,c->y,probe.x,probe.y,17)||
    out_yield(c,u,v,footLook,11.f);
 if(d->yielding)pace=0;
 float accel=(pace<c->s?155.f:63.f)*dt;
 c->s=c->s<pace?(c->s+accel<pace?c->s+accel:pace):(c->s-accel>pace?c->s-accel:pace);
 float step=c->s*dt;if(step>dist)step=dist;
 if(step<=0)return;
 Car next=*c;next.a=newAngle;next.x+=u*step;next.y+=v*step;
 if(!street_car_clear(id,c,&next)||night_phone_lane_stop(c->x,c->y,next.x,next.y,17)||
    out_body_blocked(c,&next)){d->yielding=1;c->s=0;return;}
 *c=next;
}
static void outer_life_tick(float dt){
 outer_condition_tick(dt);
 const int nearby=p.x>=1610.f||p.y>=1600.f;
 if(nearby&&!outWalkReady)outer_walk_init();
 if(outWalkReady)outer_walk_tick(dt);
 if(nearby)out_prepare_lanes();
 if(nearby||street_next_car(nc-1)<LEN(cars))out_sample_feet();
 for(int i=0;i<OUT_TRAFFIC_COUNT;i++){
  int id=STREET_CITY_CAR_LIMIT+i;Car*c=cars+id;OutDriver*d=outDriver+i;
  if(d->driven){
   if(p.car==id||out_seen(c))d->outOfSight=0;
   else if((d->outOfSight+=dt)>75.f){
    /* The unregistered street car is retained while nearby, never overwritten
       under the player or given a fresh report while visible. */
    if(lastCar==id)lastCar=-1;
    *d=(OutDriver){.outOfSight=75.f};*c=(Car){.x=-100,.y=-100};
   }
   continue;
  }
  if(!nearby&&!street_car_exists(id))continue;
  if(c->cond>0&&street_car_exists(id)){out_drive(i,dt);continue;}
  if(i<(outNight()?4:OUT_TRAFFIC_COUNT)&&nearby)out_seed(i,d->outOfSight>0.f);
 }
}
/* Crossroads crowds use the same stable identities for drawing, conversation
 * and confrontations. A selected pedestrian waits even if a shift changes. */
static int outer_person_ref(int id,NpcRef*r,int*step){
 static const char *names[N_OUTER_SITES][N_OUTER_SEATS]={
  {"Ari Flint","Noa Kell","Lux Reed","Kim Ashe"},
  {"Pat Hollow","Leo Crane","Hal Lark","Toni Bay"},
  {"Ren Vale","Kit Neri","Dana Ash","Tao Blake"},
  {"Cleo Wade","Luca Ford","Dev Beck","Oona Pike"},
  {"Sid Bass","Lane Moon","Mae Cove","Jori Niles"},
  {"Wynn Hart","Rune Hale","Cal Firth","Emery Lin"}
 };
 _Static_assert(LEN(outerStops)>=N_OUTER_SITES,"preserve the six original regional identity groups");
 if(id<N_ACT_OUTER||id>=N_ACT_OUTER_END||scene!=1)return 0;
 if(outBody[id-N_ACT_OUTER].damage==3)return 0;
 if(npc_holds(id)){*r=npcTalk;if(step)*step=0;return 1;}
 /* Static shoulder routes have no time-of-day or viewport discontinuity.
  * The raw actor supplies the same coordinates for walkers, NPC contacts,
  * ambient traffic yielding and direct overlap checks. */
 if(!outWalkReady){
  if(p.x<1610.f&&p.y<1600.f)return 0;
  outer_walk_init();
 }
 int s=id-N_ACT_OUTER,k=s/N_OUTER_SEATS,i=s%N_OUTER_SEATS;
 const OuterWalker*w=outWalkers+s;
 if(!w->ready)return 0;
 static const U coats[]={0x758fbd,0xca9fa0,0x82b8ad,0xb89a79,0x907caa,0x6b8d96};
 U coat=outBody[s].flash>0?0xffdfbc:coats[s%LEN(coats)];
 if(step)*step=w->pause>0?0:(int)(sn(w->stride*.5f)*4.f);
 return npc_spot(r,id,0,names[k][i],w->x,w->y,w->a,MODEL_CIVILIAN,s+13,coat);
}
static int outer_walk_pose(int i,float*x,float*y,float*a,int*step,U*coat){
 if((U)i>=(U)outer_walk_count())return 0;
 int id=N_ACT_OUTER+i;NpcRef r;
 if(npc_replaces(id)||!outer_person_ref(id,&r,step))return 0;
 npc_present_pose(id,&r.x,&r.y,&r.a,step);
 *x=r.x;*y=r.y;*a=r.a;*coat=r.coat;return 1;
}

/* Melee ranks by squared distance, pulses by forward distance, matching
   their city equivalents so a nearer target from either group wins. */
static OuterHit outer_pick(float x,float y,float co,float si,int melee,float limit,float minimum,float mx,float my){
 OuterHit best={-1,0,0,limit};
 if(scene!=1)return best;
 if(outCrowdCount)for(int id=N_ACT_OUTER;id<N_ACT_OUTER_END;id++){
  NpcRef r;if(npc_replaces(id)||npc_holds(id)||!outer_person_ref(id,&r,0))continue;
  float dx=r.x-x,dy=r.y-y,f=dx*co+dy*si,l=absf(dy*co-dx*si),d=melee?d2(dx,dy):f;
  if(f<=0||f<minimum||d>=best.rank||(melee?l>13.f:l>=13.f+f*.12f))continue;
  if(!street_reach_open(x,y,r.x,r.y,1,-1)||
     (!melee&&(mx!=x||my!=y)&&!street_reach_open(mx,my,r.x,r.y,1,-1)))continue;
  best=(OuterHit){id,r.x,r.y,d};
 }
 OuterHit extra=street_extra_pick(x,y,co,si,melee,best.rank,minimum,mx,my);
 return extra.id>=0?extra:best;
}
static int outer_car_hit(float x,float y,float co,float si){
 if(scene!=1)return 0;
 if(outCrowdCount)for(int id=N_ACT_OUTER;id<N_ACT_OUTER_END;id++){
  NpcRef r;if(npc_replaces(id)||npc_holds(id)||!outer_person_ref(id,&r,0))continue;
  float dx=r.x-x,dy=r.y-y;
  if(absf(dx*co+dy*si)<15.f&&absf(dy*co-dx*si)<6.5f)
   return outer_injure(id,3,r.x,r.y);
 }
 return street_extra_car_hit(x,y,co,si);
}
static void outer_life_paint(void){
 if(outCrowdCount){
  for(int id=N_ACT_OUTER;id<N_ACT_OUTER_END;id++){
   OuterCondition*b=outBody+(id-N_ACT_OUTER);
   if(npc_replaces(id))continue;
   if(b->damage==3&&b->down>0){
    int x=sx(b->x),y=sy(b->y);if(x>=-12&&x<W+12&&y>=-10&&y<H+10){
     line(x-7,y-3,x+7,y+3,0x815763);disc(x-7,y-4,3,0x85757b);
    }continue;
   }

  }
 }
}
