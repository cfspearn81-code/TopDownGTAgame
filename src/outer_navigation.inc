/* Add a small weighted road graph to the legacy grid router. Long routes
 * enter/exit at actual road joins; no straight line across cliffs/water. */
enum{OUT_ATTACH_CAP=4};
typedef struct{int node,count,dist;NavPoint via[8];} OuterAttach;
static const unsigned char outGates[]={0,14,29,52};
static NavPoint nav_outer_point(int i){return (NavPoint){outerNodes[i].x,outerNodes[i].y};}
static int nav_outer_attach(NavPoint p,OuterAttach att[OUT_ATTACH_CAP]){
 int n=0;
 if(!outer_is_new(p.x,p.y)){
  for(int k=0;k<LEN(outGates);k++){
   int index=outGates[k];nav_solve_base(p,nav_outer_point(index));
   if(!navCount&&nav_dist(p,nav_outer_point(index))>=8)continue;
   OuterAttach*a=att+n++;a->node=index;a->count=min(8,navCount);a->dist=navDist;
   for(int j=0;j<a->count;j++)a->via[j]=navNodes[j];
   if(!a->count){a->count=1;a->via[0]=p;}
  }
 }else{
  int px=p.x,py=p.y,i=outer_nearest(p.x,p.y,&px,&py,100);
  if(i<0)return 0;
  NavPoint mid={px,py};if(!nav_traversable(p,mid,5))return 0;
  const OuterRoad*r=outerRoads+i;
  int nodes[]={r->a,r->b};
  for(int k=0;k<2;k++){
   int e=nodes[k];NavPoint q=nav_outer_point(e);
   if(night_phone_lane_stop(mid.x,mid.y,q.x,q.y,4))continue;
   OuterAttach*a=att+n++;a->node=e;a->count=0;a->dist=nav_dist(p,mid)+nav_dist(mid,q);
   a->count=nav_append(a->via,a->count,p);a->count=nav_append(a->via,a->count,mid);
   a->count=nav_append(a->via,a->count,q);
  }
 }
 return n;
}
static int nav_outer_add(int n,NavPoint p){
 if(n>=LEN(navNodes))return n;
 return nav_append(navNodes,n,p);
}
static void nav_solve(NavPoint from,NavPoint to){
 if(!outer_is_new(from.x,from.y)&&!outer_is_new(to.x,to.y)){nav_solve_base(from,to);return;}
 OuterAttach start[OUT_ATTACH_CAP],end[OUT_ATTACH_CAP];
 int ns=nav_outer_attach(from,start),ne=nav_outer_attach(to,end);
 navCount=navDist=0;
 if(nav_traversable(from,to,10)&&!night_phone_lane_stop(from.x,from.y,to.x,to.y,4)){
  navNodes[0]=from;navNodes[1]=to;navCount=2;navDist=nav_dist(from,to);return;
 }
 if(!ns||!ne)return;
 enum{N=LEN(outerNodes),INF=100000};
 int d[N],prev[N],which[N];unsigned char done[N];
 for(int i=0;i<N;i++){d[i]=INF;prev[i]=which[i]=-1;done[i]=0;}
 for(int i=0;i<ns;i++){
  int a=start[i].node;
  if(start[i].dist<d[a]){d[a]=start[i].dist;which[a]=i;}
 }
 for(int i=0;i<N;i++){
  int u=-1,best=INF;
  for(int j=0;j<N;j++)if(!done[j]&&d[j]<best){best=d[j];u=j;}
  if(u<0)break;done[u]=1;
  for(int j=0;j<LEN(outerRoads);j++){
   const OuterRoad*r=outerRoads+j;
   int v=r->a==u?r->b:r->b==u?r->a:-1;
   if(v<0||done[v])continue;
   NavPoint a=nav_outer_point(u),b=nav_outer_point(v);
   if(night_phone_lane_stop(a.x,a.y,b.x,b.y,4))continue;
   int alt=d[u]+nav_dist(a,b);
   if(alt<d[v]){d[v]=alt;prev[v]=u;which[v]=which[u];}
  }
 }
 int best=INF,win=-1;
 for(int i=0;i<ne;i++){
  int a=end[i].node,cost=d[a]+end[i].dist;
  if(cost<best){best=cost;win=i;}
 }
 if(win<0||which[end[win].node]<0)return;
 OuterAttach*first=start+which[end[win].node],*last=end+win;
 for(int i=0;i<first->count;i++)navCount=nav_outer_add(navCount,first->via[i]);
 int back[N],count=0;
 for(int n=last->node;n>=0&&count<N;n=prev[n])back[count++]=n;
 for(int i=count-1;i>=0;i--)navCount=nav_outer_add(navCount,nav_outer_point(back[i]));
 for(int i=last->count-1;i>=0;i--)navCount=nav_outer_add(navCount,last->via[i]);
 navDist=0;for(int i=1;i<navCount;i++)navDist+=nav_dist(navNodes[i-1],navNodes[i]);
}
