/* A fixed city roster. Calls are routed from where a unit actually is.
 * Transient world state only: no changes to the save layout. */
enum{DISPATCH_FIRST=44,DISPATCH_UNITS=8,DISPATCH_SIDE=62,DISPATCH_CELLS=62*62,DISPATCH_PATH=144};
enum{D_POST,D_RESPONSE,D_SEARCH,D_RETURN,D_DOWN};
typedef struct{float x,y;} DispatchPoint;
typedef struct{
 DispatchPoint home,beat,goal,path[DISPATCH_PATH];
 int state,station,turn,len,next;
 float rethink,atPost,down;
} DispatchUnit;
static DispatchUnit dispatchUnits[DISPATCH_UNITS];
static unsigned char dispatchRoad[DISPATCH_CELLS],dispatchEdges[DISPATCH_CELLS];
static short dispatchCost[DISPATCH_CELLS],dispatchFrom[DISPATCH_CELLS],dispatchPosition[DISPATCH_CELLS],dispatchHeap[DISPATCH_CELLS],dispatchReverse[DISPATCH_CELLS];
static int dispatchHeapCount,dispatchEnd;
static int dispatchCase,dispatchSaw,dispatchReported,dispatchLost,dispatchLevel;
static float dispatchWait,dispatchCooldown,dispatchQuiet,dispatchSightAge,dispatchLastHeat,dispatchStrike;
static DispatchPoint dispatchCall;

static float dispatch_px(int key){return 48.f+(key%DISPATCH_SIDE)*32.f;}
static float dispatch_py(int key){return 48.f+(key/DISPATCH_SIDE)*32.f;}
static int dispatch_cell(float x,float y){return cl((int)((y-32.f)/32),0,DISPATCH_SIDE-1)*DISPATCH_SIDE+cl((int)((x-32.f)/32),0,DISPATCH_SIDE-1);}
static int dispatch_line(float x,float y,float tx,float ty,float r){
 return street_line_open(x,y,tx,ty,r);
}
static void dispatch_prepare(void){
 static const int steps[]={1,-1,DISPATCH_SIDE,-DISPATCH_SIDE};
 for(int k=0;k<DISPATCH_CELLS;k++){
  float x=dispatch_px(k),y=dispatch_py(k);int xr=(int)x%256,yr=(int)y%256;
  dispatchRoad[k]=(unsigned char)(solid(x,y,8)?0:(xr<=48||xr>=208||yr<=48||yr>=208)?1:3);
  dispatchEdges[k]=0;
 }
 for(int k=0;k<DISPATCH_CELLS;k++)if(dispatchRoad[k]){
  int c=k%DISPATCH_SIDE,r=k/DISPATCH_SIDE;
  for(int j=0;j<4;j++){
   if((j==0&&c==DISPATCH_SIDE-1)||(j==1&&c==0)||(j==2&&r==DISPATCH_SIDE-1)||(j==3&&r==0))continue;
   const int b=k+steps[j];
   if(dispatchRoad[b]&&dispatch_line(dispatch_px(k),dispatch_py(k),dispatch_px(b),dispatch_py(b),8))
    dispatchEdges[k]|=(unsigned char)(1u<<j);
  }
 }
}
static int dispatch_near(float x,float y){
 const int ref=dispatch_cell(x,y),cx=ref%DISPATCH_SIDE,cy=ref/DISPATCH_SIDE;
 float best=1e9f;int chosen=-1;
 for(int j=max(0,cy-3);j<=min(DISPATCH_SIDE-1,cy+3);j++)for(int i=max(0,cx-3);i<=min(DISPATCH_SIDE-1,cx+3);i++){
  int k=j*DISPATCH_SIDE+i;if(!dispatchRoad[k]||!dispatchEdges[k])continue;
  const float tx=dispatch_px(k),ty=dispatch_py(k),d=d2(x-tx,y-ty);
  if(d<best&&dispatch_line(x,y,tx,ty,8)){chosen=k;best=d;}
 }
 return chosen;
}
/* A narrow clearance or the tip of a facade can hide every coarse grid
 * center even though the point is reachable. Try a short clear dogleg. */
static int dispatch_anchor(float x,float y,DispatchPoint*via,int*bent){
 int direct=dispatch_near(x,y);*bent=0;
 if(direct>=0)return direct;
 static const unsigned char radii[]={8,14,22,34,51};
 int choice=-1;float best=1e9f;
 for(int r=0;r<LEN(radii)&&radii[r]<best;r++)for(int n=0;n<16;n++){
  float angle=n*.392699082f,xx=x+cs(angle)*radii[r],yy=y+sn(angle)*radii[r];
  if(!dispatch_line(x,y,xx,yy,8))continue;
  int k=dispatch_near(xx,yy);if(k<0)continue;
  float cost=radii[r]+sq(d2(dispatch_px(k)-xx,dispatch_py(k)-yy));
  if(cost<best){best=cost;choice=k;*via=(DispatchPoint){xx,yy};*bent=1;}
 }
 return choice;
}
static int dispatch_h(int key){
 int x=key%DISPATCH_SIDE-dispatchEnd%DISPATCH_SIDE,y=key/DISPATCH_SIDE-dispatchEnd/DISPATCH_SIDE;
 return max(x,-x)+max(y,-y);
}
static int dispatch_better(int a,int b){
 int x=dispatch_h(a),y=dispatch_h(b),r=dispatchCost[a]+x,s=dispatchCost[b]+y;
 return r<s||(r==s&&x<y);
}
static void dispatch_insert(int key){
 int k=dispatchPosition[key];
 if(k<0)k=dispatchHeapCount++;
 while(k>0){const int j=(k-1)/2,a=dispatchHeap[j];if(!dispatch_better(key,a))break;dispatchHeap[k]=(short)a;dispatchPosition[a]=(short)k;k=j;}
 dispatchHeap[k]=(short)key;dispatchPosition[key]=(short)k;
}
static int dispatch_pop(void){
 int result=dispatchHeap[0],last=dispatchHeap[--dispatchHeapCount],i=0;
 dispatchPosition[result]=-2;
 if(dispatchHeapCount==0)return result;
 while(i*2+1<dispatchHeapCount){
  int j=i*2+1,key=dispatchHeap[j];
  if(j+1<dispatchHeapCount&&dispatch_better(dispatchHeap[j+1],key)){key=dispatchHeap[++j];}
  if(!dispatch_better(key,last))break;
  dispatchHeap[i]=(short)key;dispatchPosition[key]=(short)i;i=j;
 }
 dispatchHeap[i]=(short)last;dispatchPosition[last]=(short)i;return result;
}
static void dispatch_plan(DispatchUnit*u,Ped*q,DispatchPoint to,float wait){
 u->next=u->len=0;u->goal=to;u->rethink=wait;
 const float dx=q->x-to.x,dy=q->y-to.y;
 if(d2(dx,dy)<150.f*150.f&&dispatch_line(q->x,q->y,to.x,to.y,8)){u->path[u->len++]=to;return;}
 DispatchPoint entry,exit;int bentEntry,bentExit;
 const int from=dispatch_anchor(q->x,q->y,&entry,&bentEntry),target=dispatch_anchor(to.x,to.y,&exit,&bentExit);
 if(from<0)return;
 dispatchEnd=target>=0?target:dispatch_cell(to.x,to.y);dispatchHeapCount=0;
 for(int i=0;i<DISPATCH_CELLS;i++){dispatchCost[i]=32767;dispatchFrom[i]=-1;dispatchPosition[i]=-1;}
 dispatchCost[from]=0;dispatch_insert(from);
 static const int steps[]={1,-1,DISPATCH_SIDE,-DISPATCH_SIDE};
 int nearest=from;
 while(dispatchHeapCount){
  int k=dispatch_pop();
  if(dispatch_h(k)<dispatch_h(nearest))nearest=k;
  if(k==dispatchEnd){nearest=k;break;}
  int edges=dispatchEdges[k];
  for(int j=0;j<4;j++)if(edges&(1u<<j)){
   const int a=k+steps[j],cost=dispatchCost[k]+dispatchRoad[a];
   if(dispatchPosition[a]!=-2&&cost<dispatchCost[a]){dispatchCost[a]=(short)cost;dispatchFrom[a]=(short)k;dispatch_insert(a);}
  }
 }
 /* Collapse collinear subdivisions into turn points, preserving the
  * walkable segments chosen by the road-biased grid. */
 int n=0,delta=0;
 for(int k=nearest;k>=0&&n<LEN(dispatchReverse);k=dispatchFrom[k]){
  const int parent=dispatchFrom[k],nextDelta=parent-k;
  if(!n||parent<0||nextDelta!=delta)dispatchReverse[n++]=(short)k;
  delta=nextDelta;
 }
 if(bentEntry)u->path[u->len++]=entry;
 while(n&&u->len<LEN(u->path)-2){int k=dispatchReverse[--n];u->path[u->len++]=(DispatchPoint){dispatch_px(k),dispatch_py(k)};}
 if(!n&&nearest==target){if(bentExit)u->path[u->len++]=exit;u->path[u->len++]=to;}
}
static void dispatch_step(DispatchUnit*u,Ped*q,float dt,float speed){
 q->s=0;
 while(u->next<u->len){
  const DispatchPoint t=u->path[u->next];const float dx=t.x-q->x,dy=t.y-q->y,d=sq(d2(dx,dy));
  if(d<.22f){u->next++;continue;}
  const float amount=speed*dt<d?speed*dt:d,oldX=q->x,oldY=q->y;
  const float a=look(dx,dy),turn=wrap(a-q->a),cap=dt*12.f;
  q->a=wrap(q->a+(turn>cap?cap:turn<-cap?-cap:turn));
  move(&q->x,&q->y,dx*amount/d,dy*amount/d,8);
  q->s=sq(d2(q->x-oldX,q->y-oldY))/dt;
  if(q->s<1)u->rethink=0;
  return;
 }
}
static int dispatch_engaged(void){
 int live=0;for(int i=0;i<DISPATCH_UNITS;i++)if((dispatchUnits[i].state==D_RESPONSE||dispatchUnits[i].state==D_SEARCH)&&peds[DISPATCH_FIRST+i].hp>0)live++;return live;
}
static int dispatch_visible(void){return dispatchSaw;}
static int dispatch_posted(int i){return i>=DISPATCH_FIRST&&i<DISPATCH_FIRST+DISPATCH_UNITS&&dispatchUnits[i-DISPATCH_FIRST].state==D_POST;}
static DispatchPoint dispatch_station_post(int index,DispatchPoint door){
 /* The venue door may be beside a facade, not in the center of the road.
  * Lay out distinct, reachable positions on the approach sidewalks. */
 static const unsigned char radii[]={28,36,49,63,82,106,134,171,216};
 DispatchPoint best=door;float score=1e9f;
 for(int r=0;r<LEN(radii);r++)for(int n=0;n<16;n++){
  float angle=(n+(index-4)*4)*.3926990817f;
  DispatchPoint q={door.x+cs(angle)*radii[r],door.y+sn(angle)*radii[r]};
  if(solid(q.x,q.y,8)||dispatch_near(q.x,q.y)<0)continue;
  int free=1;
  for(int j=0;j<index;j++)if(d2(dispatchUnits[j].home.x-q.x,dispatchUnits[j].home.y-q.y)<26.f*26.f){free=0;break;}
  if(!free)continue;
  float x=absf(q.x-street_nearest_line(q.x)),y=absf(q.y-street_nearest_line(q.y));
  float street=x<y?x:y;
  float cost=radii[r]+(street<28.f?38.f:absf(street-STREET_WALK)*.27f);
  if(cost<score){best=q;score=cost;}
 }
 return best;
}
static void dispatch_reset(void){
 /* Beat offices opposite the first jobs, around freight and in the wards;
  * remaining units start visibly parked near the physical station. */
 static const short posts[][4]={{512,558,512,731},{256,990,419,990},{1536,1216,1700,1216},{798,1568,965,1568}};
 int sx=1280,sy=512;
 for(int j=0;j<LEN(loc);j++)if(loc[j].kind==15){sx=loc[j].x;sy=loc[j].y;break;}
 dispatch_prepare();
 for(int i=0;i<DISPATCH_UNITS;i++){
  DispatchUnit*u=dispatchUnits+i;z(u,sizeof *u);u->station=i>=LEN(posts);
  if(!u->station){u->home=(DispatchPoint){posts[i][0],posts[i][1]};u->beat=(DispatchPoint){posts[i][2],posts[i][3]};}
  else{u->home=dispatch_station_post(i,(DispatchPoint){sx,sy});u->beat=u->home;}
  if(solid(u->home.x,u->home.y,8))u->home=dispatch_station_post(i,(DispatchPoint){sx,sy});
  if(solid(u->beat.x,u->beat.y,8))u->beat=u->home;
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
  if(q->hp<=0){if(u->state!=D_DOWN){u->state=D_DOWN;u->down=24.f;}continue;}
  if(wanted&&u->state!=D_RETURN&&d2(p.x-q->x,p.y-q->y)<177.f*177.f&&dispatch_line(q->x,q->y,p.x,p.y,2)){
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
   if(q->hp<=0||(u->state!=D_POST&&u->state!=D_RETURN))continue;
   float score=absf(q->x-dispatchCall.x)+absf(q->y-dispatchCall.y)+(u->station&&wanted<3?88.f:0.f)+(needStation&&!u->station?5000.f:0.f);
   if(score<best){best=score;choice=i;}
  }
  if(choice>=0){
   DispatchUnit*u=dispatchUnits+choice;u->state=D_RESPONSE;u->rethink=0;u->len=u->next=0;u->atPost=0;
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
  if(u->state==D_DOWN){
   /* The same slot comes back on another quiet shift, out of the camera's
    * vicinity. A downed officer never replenishes the active case. */
   if(!dispatchCase&&p.heat<=0){u->down-=dt;if(u->down<=0&&!street_on_camera(u->home.x,u->home.y)&&street_foot_clear(u->home.x,u->home.y,-1)){
    *q=(Ped){u->home.x,u->home.y,0,0,u->station?3:2,1,0,0,0};u->state=D_POST;u->atPost=2.f;u->len=u->next=0;
   }}continue;
  }
  q->cool-=dt;u->rethink-=dt;
  if((u->state==D_RESPONSE||u->state==D_SEARCH)&&p.heat<=0){u->state=D_RETURN;u->rethink=0;}
  if(u->state==D_POST||u->state==D_RETURN){
   DispatchPoint dest=u->state==D_RETURN||!u->turn?u->home:u->beat;
   if(d2(dest.x-q->x,dest.y-q->y)<1.f){
    q->s=0;
    if(u->state==D_RETURN){u->state=D_POST;u->turn=0;u->atPost=3.f;}
    else if(!u->station&&(u->atPost-=dt)<=0){u->turn=!u->turn;u->atPost=2.0f;u->rethink=0;}
   }else{
    if(u->rethink<=0)dispatch_plan(u,q,dest,1.1f);
    dispatch_step(u,q,dt,u->state==D_RETURN?96.f:37.f);
   }
   continue;
  }
  const float d=d2(p.x-q->x,p.y-q->y);
  const int eyes=wanted&&d<177.f*177.f&&dispatch_line(q->x,q->y,p.x,p.y,2);
  if(eyes&&d<65.f*65.f&&q->cool<=0&&dispatchStrike<=0){
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
  if((eyes&&d<25.f*25.f)||d2(dest.x-q->x,dest.y-q->y)<3.f*3.f){q->s=0;continue;}
  if(u->rethink<=0&&(u->next>=u->len||d2(dest.x-u->goal.x,dest.y-u->goal.y)>17.f*17.f))
   dispatch_plan(u,q,dest,.54f+i*.035f);
  dispatch_step(u,q,dt,u->state==D_SEARCH?77.f:speeds[max(1,wanted)]+(u->station?7.f:0.f));
 }
}
