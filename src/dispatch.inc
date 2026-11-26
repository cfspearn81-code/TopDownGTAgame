/* A fixed city roster. Calls are routed from where a unit actually is.
 * Transient world state only: no changes to the save layout. */
enum{D_POST,D_RESPONSE,D_SEARCH,D_RETURN,D_DOWN};
typedef struct{float x,y;} DispatchPoint;
typedef struct{
 DispatchPoint home,beat;
 int state,station,turn;
 float atPost,down;
} DispatchUnit;
static DispatchUnit dispatchUnits[DISPATCH_UNITS];
static int dispatchCase,dispatchSaw,dispatchReported,dispatchLost,dispatchLevel;
static float dispatchWait,dispatchCooldown,dispatchQuiet,dispatchSightAge,dispatchLastHeat,dispatchStrike;
static DispatchPoint dispatchCall;

/* A sidewalk address lies on one face of a block. Either end of that
 * face leads to the corner network used by civilian walkers. */
typedef struct{DispatchPoint point,ends[2];} DispatchAddress;
static DispatchAddress dispatch_address(float x,float y){
 int left=cl((int)x/256,0,7)*256,top=cl((int)y/256,0,7)*256;
 float x0=left+STREET_WALK,x1=left+256-STREET_WALK;
 float y0=top+STREET_WALK,y1=top+256-STREET_WALK;
 float sx=x<x0?x0:x>x1?x1:x,sy=y<y0?y0:y>y1?y1:y;
 float curbX=x-left<128?x0:x1,curbY=y-top<128?y0:y1;
 if(sx==curbX&&sy==curbY)
  return (DispatchAddress){{sx,sy},{{sx,sy},{sx,sy}}};
 if(absf(x-curbX)<absf(y-curbY))
  return (DispatchAddress){{curbX,sy},{{curbX,y0},{curbX,y1}}};
 return (DispatchAddress){{sx,curbY},{{x0,curbY},{x1,curbY}}};
}
static float dispatch_distance(DispatchPoint a,DispatchPoint b){
 return absf(a.x-b.x)+absf(a.y-b.y);
}
static int dispatch_walk_to(int i,DispatchPoint target,float dt,float speed){
 Ped*q=peds+DISPATCH_FIRST+i;StreetWalk*w=streetWalks+DISPATCH_FIRST+i;
 q->s=0;
 if(w->pause>0){w->pause-=dt;return 0;}
 if(d2(q->x-w->mx,q->y-w->my)<.18f){
  q->x=w->mx;q->y=w->my;w->cross=0;w->blocked=0;
  DispatchAddress from=dispatch_address(q->x,q->y),to=dispatch_address(target.x,target.y);
  DispatchPoint here={q->x,q->y},next=to.point;
  if(dispatch_distance(here,next)<.01f)return 1;
  int sameFace=(here.x==next.x&&(int)here.y/256==(int)next.y/256)||
               (here.y==next.y&&(int)here.x/256==(int)next.x/256);
  if(!sameFace){
   float best=1e9f;
   /* A corner can leave along either axis. A walker mid-block must reach
    * an end of their own sidewalk before turning. */
   DispatchPoint start=here,end=to.point;
   for(int a=0;a<2;a++)for(int b=0;b<2;b++){
    float length=dispatch_distance(here,from.ends[a])+dispatch_distance(from.ends[a],to.ends[b])+
                 dispatch_distance(to.ends[b],to.point);
    if(length<best){best=length;start=from.ends[a];end=to.ends[b];}
   }
   next=start;
   if(dispatch_distance(here,start)==0){
    if(here.x!=end.x)next.x=street_corner_step(here.x,end.x,&w->cross);
    else if(here.y!=end.y)next.y=street_corner_step(here.y,end.y,&w->cross);
   }
  }
  w->mx=next.x;w->my=next.y;
  if(!street_can_start_leg(q,w))return 0;
 }
 street_walk_step(q,w,dt,w->cross&&speed<62.f?62.f:speed);
 return 0;
}
static int dispatch_engaged(void){
 int live=0;for(int i=0;i<DISPATCH_UNITS;i++)if((dispatchUnits[i].state==D_RESPONSE||dispatchUnits[i].state==D_SEARCH)&&peds[DISPATCH_FIRST+i].hp>0)live++;return live;
}
static int dispatch_visible(void){return dispatchSaw;}
static int dispatch_posted(int i){return i>=DISPATCH_FIRST&&i<DISPATCH_FIRST+DISPATCH_UNITS&&dispatchUnits[i-DISPATCH_FIRST].state==D_POST;}
static void dispatch_reset(void){
 /* Four neighborhood beats and four posts on the station frontage. */
 static const short posts[][4]={{512,558,512,731},{256,990,419,990},{1536,1216,1700,1216},{798,1568,965,1568}};
 int hq=0;
 for(int j=0;j<LEN(loc);j++)if(loc[j].kind==15){hq=j;break;}
 StreetAddress station=street_venue_address(hq,0);
 for(int i=0;i<DISPATCH_UNITS;i++){
  DispatchUnit*u=dispatchUnits+i;z(u,sizeof *u);u->station=i>=LEN(posts);
  if(u->station){
   /* The HQ entrance faces the east/west sidewalk. Space the four posts
    * along its frontage, inside this block and clear of the crossings. */
   float offset=(i-LEN(posts)-1.5f)*28.f;
   u->home=dispatch_address(station.x+offset,station.y).point;u->beat=u->home;
  }else{
   u->home=dispatch_address(posts[i][0],posts[i][1]).point;
   u->beat=dispatch_address(posts[i][2],posts[i][3]).point;
  }
  streetWalks[DISPATCH_FIRST+i]=(StreetWalk){.mx=u->home.x,.my=u->home.y};
  u->atPost=1.f+i*.53f;peds[DISPATCH_FIRST+i]=(Ped){u->home.x,u->home.y,i<6?1.57f:-1.57f,0,i<4?2:3,1,0,0,0};
 }
 dispatchCase=dispatchSaw=dispatchReported=dispatchLost=dispatchLevel=0;
 dispatchWait=dispatchCooldown=dispatchQuiet=dispatchLastHeat=dispatchStrike=0;
 dispatchSightAge=99.f;
}
static void dispatch_tick(float dt){
 static const float speeds[]={63.f,86.f,110.f,124.f};
 const int wanted=(p.heat>15.f)+(p.heat>80.f)+(p.heat>150.f);
 dispatchSaw=0;dispatchCooldown-=dt;dispatchStrike-=dt;dispatchSightAge+=dt;
 if(wanted){
  if(!dispatchCase||dispatchQuiet>0.f){dispatchCase=1;dispatchWait=.82f;dispatchLevel=0;dispatchReported=dispatchLost=0;dispatchSightAge=99.f;dispatchCall=(DispatchPoint){p.x,p.y};}
  if(p.heat>dispatchLastHeat+.15f){dispatchCall=(DispatchPoint){p.x,p.y};dispatchSightAge=0;}
  dispatchQuiet=0;dispatchWait-=dt;
 }else if(p.heat<=0){
  dispatchQuiet+=dt;
  if(dispatchQuiet>=3.0f){dispatchCase=dispatchLevel=0;dispatchReported=dispatchLost=0;}
 }else dispatchQuiet=0;
 dispatchLastHeat=p.heat;
 /* Only an assigned unit with an unobstructed view, or a nearby officer
  * at the original call, updates the shared location. Walls break contact. */
 for(int i=0;i<DISPATCH_UNITS;i++){
  DispatchUnit*u=dispatchUnits+i;Ped*q=peds+DISPATCH_FIRST+i;
  if(npc_replaces(N_ACT_STREET+DISPATCH_FIRST+i)||npc_holds(N_ACT_STREET+DISPATCH_FIRST+i))continue;
  if(q->hp<=0){if(u->state!=D_DOWN){u->state=D_DOWN;u->down=24.f;}continue;}
  if(wanted&&u->state!=D_RETURN&&d2(p.x-q->x,p.y-q->y)<177.f*177.f&&street_line_open(q->x,q->y,p.x,p.y,2)){
   dispatchCall=(DispatchPoint){p.x,p.y};dispatchSightAge=0;
   if(u->state==D_RESPONSE||u->state==D_SEARCH){dispatchSaw=1;u->state=D_RESPONSE;}
  }
 }
 if(wanted&&dispatchCooldown<=0&&dispatchWait<=0&&dispatch_engaged()<wanted*2){
  float best=1e9f;int choice=-1,stationActive=0;
  for(int i=0;i<DISPATCH_UNITS;i++)if(dispatchUnits[i].station&&(dispatchUnits[i].state==D_RESPONSE||dispatchUnits[i].state==D_SEARCH))stationActive++;
  const int needStation=stationActive<wanted-1&&dispatch_engaged()>=wanted*2-(wanted-1);
  for(int i=0;i<DISPATCH_UNITS;i++){
   DispatchUnit*u=dispatchUnits+i;Ped*q=peds+DISPATCH_FIRST+i;
  if(npc_replaces(N_ACT_STREET+DISPATCH_FIRST+i)||npc_holds(N_ACT_STREET+DISPATCH_FIRST+i))continue;
   if(q->hp<=0||(u->state!=D_POST&&u->state!=D_RETURN))continue;
   float score=absf(q->x-dispatchCall.x)+absf(q->y-dispatchCall.y)+(u->station&&wanted<3?88.f:0.f)+(needStation&&!u->station?5000.f:0.f);
   if(score<best){best=score;choice=i;}
  }
  if(choice>=0){
   DispatchUnit*u=dispatchUnits+choice;u->state=D_RESPONSE;u->atPost=0;
   dispatchCooldown=wanted==1?1.55f:wanted==2?1.1f:.72f;
   if(!dispatchReported){dispatchReported=1;tell("DISPATCH: nearest patrol responding from its beat.");}
   else if(wanted>dispatchLevel&&wanted>=2)tell("DISPATCH: requesting station backup.");
   dispatchLevel=max(wanted,dispatchLevel);
  }
 }
 if(dispatchSaw&&p.quiet<1.3f)p.quiet=1.3f;
 if(dispatchReported&&!dispatchLost&&p.heat<=0&&dispatchQuiet<.06f){dispatchLost=1;tell("DISPATCH: no contact. Units returning to posts.");}
 for(int i=0;i<DISPATCH_UNITS;i++){
  DispatchUnit*u=dispatchUnits+i;Ped*q=peds+DISPATCH_FIRST+i;
  if(npc_replaces(N_ACT_STREET+DISPATCH_FIRST+i)||npc_holds(N_ACT_STREET+DISPATCH_FIRST+i))continue;
  if(u->state==D_DOWN){
   /* The same slot comes back on another quiet shift, out of the camera's
    * vicinity. A downed officer never replenishes the active case. */
   if(!dispatchCase&&p.heat<=0){u->down-=dt;if(u->down<=0&&!street_on_camera(u->home.x,u->home.y)&&street_foot_clear(u->home.x,u->home.y,-1)){
    *q=(Ped){u->home.x,u->home.y,0,0,u->station?3:2,1,0,0,0};u->state=D_POST;u->atPost=2.f;
    streetWalks[DISPATCH_FIRST+i]=(StreetWalk){.mx=u->home.x,.my=u->home.y};
   }}continue;
  }
  q->cool-=dt;
  if((u->state==D_RESPONSE||u->state==D_SEARCH)&&p.heat<=0){u->state=D_RETURN;}
  if(u->state==D_POST||u->state==D_RETURN){
   DispatchPoint dest=u->state==D_RETURN||!u->turn?u->home:u->beat;
   if(dispatch_walk_to(i,dest,dt,u->state==D_RETURN?96.f:37.f)){
    q->s=0;
    if(u->state==D_RETURN){u->state=D_POST;u->turn=0;u->atPost=3.f;}
    else if(!u->station&&(u->atPost-=dt)<=0){u->turn=!u->turn;u->atPost=2.0f;}
   }
   continue;
  }
  const float d=d2(p.x-q->x,p.y-q->y);
  const int eyes=wanted&&d<177.f*177.f&&street_line_open(q->x,q->y,p.x,p.y,2);
  if(eyes&&d<65.f*65.f&&q->cool<=0&&dispatchStrike<=0&&street_car_fraction(q->x,q->y,p.x,p.y,2,p.car)>=1.f){
   q->cool=1.36f;dispatchStrike=.66f;health(u->station?10:8);particle(p.x,p.y,5,PINK);
  }
  DispatchPoint dest=dispatchCall;
  if(dispatchSightAge>1.4f&&d2(q->x-dest.x,q->y-dest.y)<50.f*50.f)u->state=D_SEARCH;
  if(u->state==D_SEARCH&&!eyes){
   /* Spread through the immediate dispatch sector, not the live player
    * coordinates. Make responders visibly sweep before going off the call. */
   static const int search[4][2]={{-84,0},{0,-84},{84,0},{0,84}};
   int a=((int)(dispatchSightAge*.55f)+i)%4;
   dest.x=cl((int)dest.x+search[a][0],48,2000);dest.y=cl((int)dest.y+search[a][1],48,2000);
   if(!open_space(dest.x,dest.y))dest=dispatchCall;
  }
  if(eyes){dest=(DispatchPoint){p.x,p.y};u->state=D_RESPONSE;}
  if(eyes&&d<25.f*25.f&&!streetWalks[DISPATCH_FIRST+i].cross){q->s=0;continue;}
  dispatch_walk_to(i,dest,dt,u->state==D_SEARCH?77.f:speeds[max(1,wanted)]+(u->station?7.f:0.f));
 }
}
