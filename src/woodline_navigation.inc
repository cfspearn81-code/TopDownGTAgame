/* Terrain-aware pedestrian paths. The original roadway Dijkstra remains
 * authoritative for vehicles; this grid is used only for unpaved pins. */
enum{WALK_GRID_STEP=24,WALK_GRID_SIDE=ROOM/WALK_GRID_STEP,WALK_GRID_N=WALK_GRID_SIDE*WALK_GRID_SIDE,
 WALK_GRID_INF=999999};
static int walkG[WALK_GRID_N],walkF[WALK_GRID_N],walkBack[WALK_GRID_N],walkSpot[WALK_GRID_N],walkHeap[WALK_GRID_N];
static int walkN;
static NavPoint walk_mid(int i){
 return (NavPoint){(i%WALK_GRID_SIDE)*WALK_GRID_STEP+WALK_GRID_STEP/2,
  (i/WALK_GRID_SIDE)*WALK_GRID_STEP+WALK_GRID_STEP/2};
}
static int walk_heur(NavPoint a,NavPoint b){
 int x=nav_abs(a.x-b.x),y=nav_abs(a.y-b.y);return max(x,y)+min(x,y)*2/5;
}
static void walk_insert(int id){
 int i=walkSpot[id];
 if(i<0)i=walkN++;
 while(i>0){
  int j=(i-1)/2,u=walkHeap[j];if(walkF[u]<=walkF[id])break;
  walkHeap[i]=u;walkSpot[u]=i;i=j;
 }
 walkHeap[i]=id;walkSpot[id]=i;
}
static int walk_pop(void){
 int id=walkHeap[0],end=walkHeap[--walkN],at=0;
 while(at*2+1<walkN){
  int j=at*2+1;if(j+1<walkN&&walkF[walkHeap[j+1]]<walkF[walkHeap[j]])j++;
  int u=walkHeap[j];if(walkF[u]>=walkF[end])break;
  walkHeap[at]=u;walkSpot[u]=at;at=j;
 }
 if(walkN){walkHeap[at]=end;walkSpot[end]=at;}
 walkSpot[id]=-2;return id;
}
static int walk_visible(NavPoint a,NavPoint b){
 return street_line_open(a.x,a.y,b.x,b.y,10)&&!night_phone_lane_stop(a.x,a.y,b.x,b.y,4);
}
static int nav_walk_solve(NavPoint from,NavPoint to){
 if(solid(to.x,to.y,10))return 0;
 if(walk_visible(from,to)){
  navNodes[0]=from;navNodes[1]=to;navCount=2;navDist=nav_dist(from,to);return 1;
 }
 for(int i=0;i<WALK_GRID_N;i++){walkG[i]=WALK_GRID_INF;walkBack[i]=walkSpot[i]=-1;}
 walkN=0;
 int gridX=from.x/WALK_GRID_STEP,gridY=from.y/WALK_GRID_STEP;
 for(int y=max(1,gridY-2);y<=min(WALK_GRID_SIDE-2,gridY+2);y++)
  for(int x=max(1,gridX-2);x<=min(WALK_GRID_SIDE-2,gridX+2);x++){
   int id=y*WALK_GRID_SIDE+x;NavPoint s=walk_mid(id);
   if(!walk_visible(from,s))continue;
   walkG[id]=walk_heur(from,s);walkF[id]=walkG[id]+walk_heur(s,to);walk_insert(id);
  }
 int winner=-1,best=WALK_GRID_INF;
 while(walkN){
  int u=walk_pop();if(walkF[u]>=best)break;
  NavPoint a=walk_mid(u);
  if(nav_dist(a,to)<=76&&walk_visible(a,to)){
   int cost=walkG[u]+walk_heur(a,to);
   if(cost<best){winner=u;best=cost;}
  }
  for(int j=-1;j<=1;j++)for(int i=-1;i<=1;i++){
   if(!i&&!j)continue;
   int x=u%WALK_GRID_SIDE+i,y=u/WALK_GRID_SIDE+j;
   if(x<=0||x>=WALK_GRID_SIDE-1||y<=0||y>=WALK_GRID_SIDE-1)continue;
   int v=y*WALK_GRID_SIDE+x,cost=walkG[u]+(i&&j?WALK_GRID_STEP*7/5:WALK_GRID_STEP);
   if(walkSpot[v]==-2||cost>=walkG[v])continue;
   NavPoint b=walk_mid(v);
   if(!walk_visible(a,b))continue;
   walkG[v]=cost;walkF[v]=cost+walk_heur(b,to);walkBack[v]=u;walk_insert(v);
  }
 }
 if(winner<0)return 0;
 static NavPoint path[WALK_GRID_N+1];int n=0;
 path[n++]=to;for(int i=winner;i>=0;i=walkBack[i])path[n++]=walk_mid(i);
 navNodes[0]=from;navCount=1;navDist=0;
 for(int i=n-1;i>=0&&navCount<LEN(navNodes);){
  NavPoint a=navNodes[navCount-1];int j=i;
  /* Keep true line-of-sight when straightening, no cuts across the tarn
   * or trunk cells. Limit the per-segment probes to local geography. */
  while(j>0&&nav_dist(a,path[j-1])<300&&walk_visible(a,path[j-1]))j--;
  NavPoint b=path[j];
  if(!nav_same(a,b)){navNodes[navCount++]=b;navDist+=nav_dist(a,b);}
  i=j-1;
 }
 return navCount>=2&&nav_same(navNodes[navCount-1],to);
}
