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
typedef struct{int damage;float x,y,down,flash;} OuterCondition;
static OuterCondition outBody[N_ACT_MAX-N_ACT_OUTER];
/* 3HP for unarmoured civilians, as on the city blocks. These short-lived
   states, like the original downtown bodies, are not career data. */
static int outer_injure(int id,int damage,float x,float y){
 if(id<N_ACT_OUTER||id>=N_ACT_MAX||damage<=0)return 0;
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
 z(outBody,sizeof outBody);z(outDriver,sizeof outDriver);outCrowdCount=0;
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
static int out_seen(const Car*c){
 /* Vehicle remains intact well beyond both the overhead and chase views. */
 return d2(p.x-c->x,p.y-c->y)<830.f*830.f;
}
static OutLanePoint outFeet[1+LEN(peds)+N_ACT_MAX-N_ACT_OUTER];
static int outFeetCount,outPlayerFoot;
static void out_sample_feet(void){
 outFeetCount=0;outPlayerFoot=p.car<0&&!p.dead;
 if(outPlayerFoot)outFeet[outFeetCount++]=(OutLanePoint){p.x,p.y};
 for(int i=0;i<LEN(peds);i++)if(peds[i].hp>0&&outer_is_new(peds[i].x+60.f,peds[i].y+60.f))
  outFeet[outFeetCount++]=(OutLanePoint){peds[i].x,peds[i].y};
 for(int id=N_ACT_OUTER;id<N_ACT_MAX;id++){
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
 outCrowdCount=nearby?(outNight()?8:4):0;
 if(!outLane[0].n&&nearby){
  out_lane_bake(outLane,outMain,LEN(outMain),1);
  out_lane_bake(outLane+1,outDetour,LEN(outDetour),1);
  out_lane_bake(outLane+2,outBluff,LEN(outBluff),0);
  out_lane_bake(outLane+3,outHarbor,LEN(outHarbor),0);
 }
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
 _Static_assert(LEN(outerStops)==N_OUTER_SITES,"update Outer Loop actor identities");
 if(id<N_ACT_OUTER||id>=N_ACT_MAX||scene!=1)return 0;
 if(outBody[id-N_ACT_OUTER].damage==3)return 0;
 if(npc_holds(id)){*r=npcTalk;if(step)*step=0;return 1;}
 if(!outCrowdCount)return 0;
 int s=id-N_ACT_OUTER,k=s/N_OUTER_SEATS,i=s%N_OUTER_SEATS;
 OuterPoint site=outerNodes[outerStops[k].node];
 /* Distant actors do not need trigonometry or terrain queries. */
 if(absf(p.x-site.x)>W+120||absf(p.y-site.y)>H+140)return 0;
 int night=outNight(),n=k==1||k==2?(night?2:4):outCrowdCount/2;
 if(i>=n)return 0;
 float swing=sn(anim*(night?.7f:1.5f)+i*.81f);
 /* Surveyed walkable shoulder runs. Each tangent stays on the road's
    raised outer ribbon for the full swing, including curved sites; the
    highway's white lane dividers are NOT sidewalks. Do not move a held
    conversation when its shift changes (handled above). */
 static const struct{float x,y,u,v;} walks[N_OUTER_SITES]={
  {2644,350,.99746f,.06976f},{2794,1072,1,0},{2368,794,1,0},
  {2734,1754,.03490f,.99926f},{2032,2532,1,0},{1160,2301,.96123f,.27564f}
 };
 float along=(i-(n-1)*.5f)*13.f+swing*13.f;
 float x=walks[k].x+walks[k].u*along,y=walks[k].y+walks[k].v*along;
 float heading=look(walks[k].u,walks[k].v)+(cs(anim*(night?.7f:1.5f)+i*.81f)<0?3.141593f:0);
 U coat=i%3==0?0x758fbd:i%3==1?0xca9fa0:0x82b8ad;
 if(outBody[id-N_ACT_OUTER].flash>0)coat=0xffdfbc;
 if(step)*step=(int)(swing*3);
 return npc_spot(r,id,0,names[k][i],x,y,heading,MODEL_CIVILIAN,i+k,coat);
}
/* Melee ranks by squared distance, pulses by forward distance, matching
   their city equivalents so a nearer target from either group wins. */
static OuterHit outer_pick(float x,float y,float co,float si,int melee,float limit,float minimum,float mx,float my){
 OuterHit best={-1,0,0,limit};
 if(scene!=1||!outCrowdCount)return best;
 for(int id=N_ACT_OUTER;id<N_ACT_MAX;id++){
  NpcRef r;if(npc_replaces(id)||npc_holds(id)||!outer_person_ref(id,&r,0))continue;
  float dx=r.x-x,dy=r.y-y,f=dx*co+dy*si,l=absf(dy*co-dx*si),d=melee?d2(dx,dy):f;
  if(f<=0||f<minimum||d>=best.rank||(melee?l>13.f:l>=13.f+f*.12f))continue;
  if(!street_reach_open(x,y,r.x,r.y,1,-1)||
     (!melee&&(mx!=x||my!=y)&&!street_reach_open(mx,my,r.x,r.y,1,-1)))continue;
  best=(OuterHit){id,r.x,r.y,d};
 }
 return best;
}
static int outer_car_hit(float x,float y,float co,float si){
 if(scene!=1||!outCrowdCount)return 0;
 for(int id=N_ACT_OUTER;id<N_ACT_MAX;id++){
  NpcRef r;if(npc_replaces(id)||npc_holds(id)||!outer_person_ref(id,&r,0))continue;
  float dx=r.x-x,dy=r.y-y;
  if(absf(dx*co+dy*si)<15.f&&absf(dy*co-dx*si)<6.5f)
   return outer_injure(id,3,r.x,r.y);
 }
 return 0;
}
static void outer_life_paint(void){
 if(outCrowdCount){
  for(int id=N_ACT_OUTER;id<N_ACT_MAX;id++){
   NpcRef r;int step=0;OuterCondition*b=outBody+(id-N_ACT_OUTER);
   if(npc_replaces(id))continue;
   if(b->damage==3&&b->down>0){
    int x=sx(b->x),y=sy(b->y);if(x>=-12&&x<W+12&&y>=-10&&y<H+10){
     line(x-7,y-3,x+7,y+3,0x815763);disc(x-7,y-4,3,0x85757b);
    }continue;
   }
   if(!outer_person_ref(id,&r,&step))continue;
   int x=sx(r.x),y=sy(r.y);
   if(x<-32||x>W+32||y<-32||y>H+44)continue;
   npc_present_pose(id,&r.x,&r.y,&r.a,&step);
   person(x,y,r.coat,step,r.a,r.model,r.hue);
  }
 }
}
