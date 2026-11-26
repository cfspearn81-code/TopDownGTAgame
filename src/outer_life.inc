/* New outer-city traffic and pedestrians. They are independent of the
 * original numbered cars and mission-reserved slot. They follow actual
 * plotted roads and take the new district's detour when the gate closes. */
static const unsigned char outMain[]={16,18,22,24,23,16};
static const unsigned char outDetour[]={16,26,25,21,18,22,24,23,16};
static const unsigned char outBluff[]={2,3,4,5,6,7,8,9,10,11,12,13,21};
static const unsigned char outHarbor[]={31,32,33,34,35,36,37,38,39,40,41,42,43,44,45};
static float outTime;
static Car outCars[7];
static int outCarCount,outCrowdCount;
static int outNight(void){return sv.dayClock<348.f||sv.dayClock>=1120.f;}
static void outer_life_reset(void){outTime=0;outCarCount=outCrowdCount=0;}
static Car outer_traffic_at(const unsigned char*route,int count,int loop,float along,int slot){
 int lens[32],total=0;
 for(int k=1;k<count;k++){
  OuterPoint a=outerNodes[route[k-1]],b=outerNodes[route[k]];
  lens[k-1]=max(1,(int)sq(d2(a.x-b.x,a.y-b.y)));
  total+=lens[k-1];
 }
 int span=loop?total:total*2;
 if(span<1)return (Car){.x=-100,.y=-100};
 int u=(int)along%span,dir=1;
 if(u>=total){u=span-u-1;dir=-1;}
 int leg=0;while(leg<count-2&&u>=lens[leg])u-=lens[leg++];
 OuterPoint a=outerNodes[route[leg]],b=outerNodes[route[leg+1]];
 float f=u/(float)lens[leg],dx=b.x-a.x,dy=b.y-a.y,v=dir/(float)lens[leg];
 /* Oncoming cars use separate lanes; retain the widest corridor in the
    center for the player and keep backgrounds out of approach gateways. */
 float offset=slot&1?24.f:27.f;
 float x=a.x+dx*f-dy*v*offset,y=a.y+dy*f+dx*v*offset;
 if(!outer_is_new(x,y)||!outer_area(x,y,8.f))return(Car){.x=-100,.y=-100};
 return (Car){x,y,look(dx*dir,dy*dir),0,(slot+1)%LEN(specs),0,100,0};
}
static void outer_life_tick(float dt){
 outTime+=dt;if(outTime>3600.f)outTime-=3600.f;
 const int want=outNight()?4:7;
 int oldNear=p.x<1610.f&&p.y<1600.f;
 if(oldNear){outCarCount=outCrowdCount=0;return;}
 int closed=night_phone_lane_stop(2410,1024,2500,1024,4);
 outCarCount=want;outCrowdCount=outNight()?8:4;
 for(int i=0;i<want;i++){
  const unsigned char *r=i%3==1?outBluff:i%3==2?outHarbor:closed?outDetour:outMain;
  int n=i%3==1?LEN(outBluff):i%3==2?LEN(outHarbor):closed?LEN(outDetour):LEN(outMain);
  outCars[i]=outer_traffic_at(r,n,i%3==0,110.f*i+outTime*(43.f+6.f*(i%3)),i);
 }
}
static int outer_vehicle_clear(const Car*old,const Car*next){
 if(!outer_is_new(next->x,next->y))return 1;
 for(int i=0;i<outCarCount;i++){
  const Car*c=outCars+i;
  if(c->cond>0&&street_overlap(c,next)&&(!old||!street_overlap(c,old)||
     d2(next->x-c->x,next->y-c->y)<d2(old->x-c->x,old->y-c->y)))return 0;
 }
 return 1;
}
static void outer_life_paint(void){
 for(int i=0;i<outCarCount;i++){
  const Car*q=outCars+i;int x=sx(q->x),y=sy(q->y);
  if(q->cond<=0||x<-42||x>W+42||y<-42||y>H+42)continue;
  ng_model(x,y,q->a,1.f,0,q->type,specs[q->type].c,(NgMods){0},100,0,0);
 }
 if(outCrowdCount){
  static const int extSites[]={10,19,25,36,45,54};
  for(int k=0;k<LEN(extSites);k++){
   OuterPoint p0=outerNodes[extSites[k]];
   int x=sx(p0.x),y=sy(p0.y);
   if(x<-50||x>W+50||y<-50||y>H+50)continue;
   int n=k==1||k==2?(outNight()?2:4):outCrowdCount/2;
   for(int i=0;i<n;i++){
    float swing=sn(anim*(outNight()?.7f:1.5f)+i*.81f);
    int wx=p0.x+(i-n/2)*10+(int)(swing*14.f),wy=p0.y+23+(i%2)*7;
    if(!solid(wx,wy,5))
     person(sx(wx),sy(wy),i%3==0?0x758fbd:i%3==1?0xca9fa0:0x82b8ad,
            (int)(swing*3),swing>=0?0:3.14159f,0,i+k);
   }
  }
 }
}
