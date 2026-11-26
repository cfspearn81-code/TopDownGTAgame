/* Port Vanta cartography and navigation. Included after the game drawing helpers. */
#define NAV_INK 0x101522u
#define NAV_SEA 0x12283bu
#define NAV_GRID 0x667483u
#define NAV_CURB 0x465264u
#define NAV_RIM 0x47566du
#define NAV_VIOLET 0xc68affu
#define NAV_DARK 0x251331u
#define NAV_WALK 0xe4c8f8u
enum{NAV_MAP_X=256,NAV_MAP_Y=50,NAV_MAP_SIZE=448};
typedef struct{int x,y;} NavPoint;
typedef struct{int x,y,w,h,wx,wy,span;} NavView;
typedef struct{NavPoint a,via;int axis,walk,bent;} NavAnchor;
static NavPoint navCursor,navManual,navGoal,navOrigin,navCurb,navNodes[8];
static int navHasManual,navGoalKind,navCount,navDist,navSelected=-1,navReady,navManualLoc=-1,navVenue=-1,navLeg,navSlot=-1,navHover,navKeyboard,navPointerX=-999,navPointerY=-999;
static float navNext;
static int nav_abs(int x){return x<0?-x:x;}
static int nav_dist(NavPoint a,NavPoint b){return nav_abs(a.x-b.x)+nav_abs(a.y-b.y);}
static int nav_same(NavPoint a,NavPoint b){return a.x==b.x&&a.y==b.y;}
static int nav_road(int a){return cl((a+128)/256,1,7)*256;}
static NavPoint nav_player(void){return(NavPoint){(int)p.x,(int)p.y};}
static int nav_traversable(NavPoint a,NavPoint b,int radius){
 int d=nav_dist(a,b),steps=max(1,(d+7)/8);
 for(int i=1;i<=steps;i++)if(solid(a.x+(b.x-a.x)*i/(float)steps,a.y+(b.y-a.y)*i/(float)steps,(float)radius))return 0;
 return 1;
}
static int nav_anchors(NavPoint point,NavAnchor out[6]){
 int n=0,rx=nav_road(point.x),ry=nav_road(point.y);
 for(int axis=0;axis<2;axis++)for(int step=-1;step<=1;step++){
  int t=(axis?ry:rx)+step*256;if(t<256||t>1792)continue;
  NavPoint a=axis?(NavPoint){point.x,t}:(NavPoint){t,point.y};
  int d=nav_dist(point,a);if(d>270||!nav_traversable(point,a,10))continue;
  out[n++]=(NavAnchor){a,point,axis,d,0};
 }
 if(n)return n;
 // A building can hide all straight approaches from a courtyard or the
 // seawall. Pick a short, collision-tested dogleg before joining the grid.
 for(int axis=0;axis<2;axis++)for(int step=-1;step<=1;step++){
  int t=(axis?ry:rx)+step*256;if(t<256||t>1792)continue;
  int best=1<<20;NavAnchor found;
  for(int side=-1;side<=1;side+=2)for(int d=8;d<=208&&d<best;d+=8){
   NavPoint via=axis?(NavPoint){point.x+side*d,point.y}:(NavPoint){point.x,point.y+side*d};
   if(!nav_traversable(point,via,10))break;
   NavPoint road=axis?(NavPoint){via.x,t}:(NavPoint){t,via.y};
   int length=d+nav_dist(via,road);
   if(length<best&&nav_traversable(via,road,10)){best=length;found=(NavAnchor){road,via,axis,length,1};break;}
  }
  if(best<(1<<20))out[n++]=found;
 }
 return n;
}
static int nav_append(NavPoint route[8],int n,NavPoint q){if(!n||!nav_same(route[n-1],q))route[n++]=q;return n;}
static void nav_solve(NavPoint from,NavPoint to){
 NavAnchor a[6],b[6];int na=nav_anchors(from,a),nb=nav_anchors(to,b),best=32000;
 navCount=0;navDist=0;
 // A clear approach along the same bit of street needs no artificial detour.
 if(((from.x==to.x&&nav_abs(from.x-nav_road(from.x))<=24)||(from.y==to.y&&nav_abs(from.y-nav_road(from.y))<=24)||nav_dist(from,to)<60)&&nav_traversable(from,to,10)){
  navCount=2;navNodes[0]=from;navNodes[1]=to;navDist=nav_dist(from,to);best=navDist;
 }
 for(int i=0;i<na;i++)for(int j=0;j<nb;j++){
  NavPoint q[8],aa=a[i].a,bb=b[j].a;int n=0;
  n=nav_append(q,n,from);if(a[i].bent)n=nav_append(q,n,a[i].via);n=nav_append(q,n,aa);
  if(a[i].axis!=b[j].axis)n=nav_append(q,n,a[i].axis?(NavPoint){bb.x,aa.y}:(NavPoint){aa.x,bb.y});
  else if(a[i].axis&&aa.y!=bb.y){int cross=nav_road((aa.x+bb.x)/2);n=nav_append(q,n,(NavPoint){cross,aa.y});n=nav_append(q,n,(NavPoint){cross,bb.y});}
  else if(!a[i].axis&&aa.x!=bb.x){int cross=nav_road((aa.y+bb.y)/2);n=nav_append(q,n,(NavPoint){aa.x,cross});n=nav_append(q,n,(NavPoint){bb.x,cross});}
  n=nav_append(q,n,bb);if(b[j].bent)n=nav_append(q,n,b[j].via);n=nav_append(q,n,to);
  int d=0;for(int k=1;k<n;k++)d+=nav_dist(q[k-1],q[k]);
  // Favor center lines and fewer turns; do not route down a long sidewalk shortcut.
  int cost=d+(a[i].walk+b[j].walk)/2+n;
  if(cost<best){best=cost;navDist=d;navCount=n;for(int k=0;k<n;k++)navNodes[k]=q[k];}
 }
}
/* Nav state is session-only. A selected stop from one career must not
   leak into another career when the player changes slots. */
static void nav_slot(void){
 if(navSlot==sideSlot)return;
 navSlot=sideSlot;navReady=navHasManual=navGoalKind=navCount=navLeg=0;
 navSelected=navManualLoc=navVenue=-1;navHover=navKeyboard=0;
}
static NavPoint nav_door(int i){return (NavPoint){loc[i].x,loc[i].y};}
static int nav_objective(NavPoint *to,int *venue){
 *venue=-1;
 if(navHasManual){*venue=navManualLoc;*to=*venue>=0?nav_door(*venue):navManual;return 3;}
 if(job.kind>=0&&job.step>=0&&job.step<job.total){
  if(job.kind==0&&job.step<3){static const int drops[]={2,3,0};*venue=drops[job.step];*to=nav_door(*venue);}
  else *to=(NavPoint){job.xs[job.step],job.ys[job.step]};
  return 2;
 }
 if(sv.intro==1||sv.intro==2){*venue=0;*to=nav_door(0);return 1;}
 return 0;
}
static int nav_park(NavPoint door,NavPoint *curb){
 int south=((door.y>>8)+1)*256;
 // The authored facilities face south. Park inside the lane nearest their
 // sidewalk, clear of the zebra crossings and with a tested walk to the door.
 if(south<=1792&&south-door.y>=28&&south-door.y<=140&&nav_abs(door.x-nav_road(door.x))>=56){
  NavPoint stop={door.x,south-22},road={door.x,south};
  if(nav_traversable(door,stop,10)&&nav_traversable(road,stop,10)){*curb=stop;return 1;}
 }
 return 0;
}
static void nav_update(void){
 NavPoint here=nav_player(),goal={0,0},curb;
 nav_slot();
 if(navHasManual&&(navManualLoc<0||p.car<0)&&d2(p.x-navManual.x,p.y-navManual.y)<=27*27){
  navHasManual=0;if(scene==1&&!p.dead&&!mapOn)tell("GPS waypoint reached.");
 }
 int venue,kind=nav_objective(&goal,&venue);
 if(!kind){navGoalKind=navCount=navDist=navLeg=0;navVenue=-1;return;}
 int driving=venue>=0&&p.car>=0;
 if(kind!=navGoalKind||venue!=navVenue||!nav_same(navGoal,goal)||navLeg!=(driving?1:0)||anim<navNext-.25f||
    nav_dist(here,navOrigin)>88||(anim>=navNext&&nav_dist(here,navOrigin)>=12)){
  navGoalKind=kind;navVenue=venue;navGoal=goal;navOrigin=here;navNext=anim+.18f;
  navLeg=driving?1:0;navCurb=goal;
  if(driving&&nav_park(goal,&curb))navCurb=curb;
  nav_solve(here,navCurb);
 }
}
static NavPoint nav_pin(void){
 if(navSelected>=0)return nav_door(navSelected);
 int x=nav_road(navCursor.x),y=nav_road(navCursor.y);
 return nav_abs(x-navCursor.x)<=nav_abs(y-navCursor.y)?(NavPoint){x,navCursor.y}:(NavPoint){navCursor.x,y};
}
static void nav_clear(void){navHasManual=0;navManualLoc=-1;tell("GPS waypoint cleared.");}
static void nav_set(void){
 NavPoint to=nav_pin();
 if((navSelected<0||p.car<0)&&d2(p.x-to.x,p.y-to.y)<=27*27){navHasManual=0;navManualLoc=-1;tell("You're already at that waypoint.");return;}
 navHasManual=1;navManual=to;navManualLoc=navSelected;tell("GPS waypoint set.");
}
static void navigation_input(U tap){
 nav_slot();
 if(!navReady||(tap&MAP)){
  navReady=1;navHover=navKeyboard=0;navSelected=-1;navPointerX=navPointerY=-999;
 }
 // Optional non-pointer access. WASD/arrow keys are never captured by a map cursor.
 if(tap&FIRE){navKeyboard=navHover=1;navSelected=(navSelected+1)%LEN(loc);navCursor=nav_door(navSelected);}
 if((tap&ENTER)&&navHover){if(navHasManual&&nav_same(navManual,nav_pin()))nav_clear();else nav_set();}
}
static void navigation_pointer(int sx,int sy,U clicks){
 nav_slot();
 if(clicks&2){if(navHasManual)nav_clear();return;}
 if(!clicks&&sx==navPointerX&&sy==navPointerY)return;
 navPointerX=sx;navPointerY=sy;navKeyboard=0;navHover=0;navSelected=-1;
 if(sx<NAV_MAP_X||sy<NAV_MAP_Y||sx>=NAV_MAP_X+NAV_MAP_SIZE||sy>=NAV_MAP_Y+NAV_MAP_SIZE)return;
 navCursor=(NavPoint){(sx-NAV_MAP_X)*ROOM/NAV_MAP_SIZE,(sy-NAV_MAP_Y)*ROOM/NAV_MAP_SIZE};
 if(navCursor.x<39||navCursor.y<39||navCursor.x>2008||navCursor.y>2008)return;
 navHover=1;
 int closest=11*11;
 for(int i=0;i<LEN(loc);i++){
  int dx=sx-NAV_MAP_X-loc[i].x*NAV_MAP_SIZE/ROOM,dy=sy-NAV_MAP_Y-loc[i].y*NAV_MAP_SIZE/ROOM,d=dx*dx+dy*dy;
  if(d<closest){navSelected=i;closest=d;}
 }
 if(clicks&1)nav_set();
}
static int nav_x(const NavView*v,int x){return v->x+(x-v->wx)*v->w/v->span;}
static int nav_y(const NavView*v,int y){return v->y+(y-v->wy)*v->w/v->span;}
static int nav_inside(const NavView*v,int x,int y,int pad){return x>=v->x+pad&&x<v->x+v->w-pad&&y>=v->y+pad&&y<v->y+v->h-pad;}
static void nav_rect(const NavView*v,int x,int y,int w,int h,U c){
 int a=max(v->x,x),b=max(v->y,y),u=min(v->x+v->w,x+w),d=min(v->y+v->h,y+h);
 if(a<u&&b<d)rect(a,b,u-a,d-b,c);
}
static void nav_land(const NavView*v,int x,int y,int w,int h,U c){
 int a=nav_x(v,x),b=nav_y(v,y);nav_rect(v,a,b,nav_x(v,x+w)-a,nav_y(v,y+h)-b,c);
}
static U nav_district(int x,int y){
 if(y<545)return 0x363340;
 if(y>1160)return x<900?0x263c36:0x31404a;
 return x<850?0x413736:x<1400?0x343a48:0x363b3e;
}
static void nav_terrain(const NavView*v,int large){
 rect(v->x,v->y,v->w,v->h,NAV_SEA);
 for(int j=0;j<v->h;j+=large?11:9){int a=v->y+j;nav_rect(v,v->x+(j%3)*5,a,v->w,1,0x182d40);}
 nav_land(v,32,32,1984,1984,0x2e555d);
 nav_land(v,39,39,1970,1970,NAV_INK);
 for(int iy=0;iy<8;iy++)for(int ix=0;ix<8;ix++){
  int tx=ix*256,ty=iy*256,x=max(39,tx+27),y=max(39,ty+27),xe=min(2009,tx+230),ye=min(2009,ty+230);
  nav_land(v,x,y,xe-x,ye-y,nav_district(tx+128,ty+128));
  if(!lot[iy*8+ix][0]){
   nav_land(v,tx+48,ty+48,160,160,0x263e3d);
   nav_land(v,tx+48,ty+125,160,6,0x446359);
   nav_land(v,tx+125,ty+48,6,160,0x446359);
  }
 }
 for(int i=0;i<nb;i++){const Building*b=buildings+i;if(b->w<=0||b->h<=0)continue;
  nav_land(v,b->x-4,b->y-3,b->w+11,b->h+9,0x212a34);
  nav_land(v,b->x,b->y,b->w,b->h,0x536172);
  if(large)nav_land(v,b->x+4,b->y+4,b->w-8,b->h/3,0x647384);
 }
 for(int i=1;i<=7;i++){
  const int a=i*256;
  nav_land(v,a-25,39,50,1970,NAV_CURB);nav_land(v,39,a-25,1970,50,NAV_CURB);
 }
 for(int i=1;i<=7;i++){
  const int a=i*256;
  nav_land(v,a-16,39,32,1970,NAV_GRID);nav_land(v,39,a-16,1970,32,NAV_GRID);
  if(large)for(int z=102;z<1990;z+=33){
   if(z%256>38&&z%256<213){nav_land(v,a-2,z,4,12,0x97a3aa);nav_land(v,z,a-2,12,4,0x97a3aa);}
  }
 }
}
static int nav_clip(const NavView*v,int x,int y){
 return (x<v->x?1:x>=v->x+v->w?2:0)|(y<v->y?4:y>=v->y+v->h?8:0);
}
static void nav_seg(const NavView*v,int x,int y,int u,int w,U c){
 while(1){
  int a=nav_clip(v,x,y),b=nav_clip(v,u,w);
  if(!(a|b)){line(x,y,u,w,c);return;}if(a&b)return;
  if(!a){int t=x;x=u;u=t;t=y;y=w;w=t;a=b;}
  int xx,yy;
  if(a&12){yy=a&4?v->y:v->y+v->h-1;xx=x+(u-x)*(yy-y)/(w-y);}
  else{xx=a&1?v->x:v->x+v->w-1;yy=y+(w-y)*(xx-x)/(u-x);}
  x=xx;y=yy;
 }
}
static void nav_stroke(const NavView*v,NavPoint p0,NavPoint p1,int w,U c){
 int a=nav_x(v,p0.x),b=nav_y(v,p0.y),u=nav_x(v,p1.x),h=nav_y(v,p1.y);
 for(int i=-w;i<=w;i++)for(int j=-w;j<=w;j++)if(i*i+j*j<=w*w+1)nav_seg(v,a+i,b+j,u+i,h+j,c);
}
static int nav_progress(NavPoint here,NavPoint *join){
 int first=1;*join=navNodes[1];
 // The cached path is updated at most a few times a second. Trim its first
 // consumed section with a fast segment projection every draw.
 int closest=1<<30;
 for(int i=1;i<min(navCount,4);i++){
  NavPoint a=navNodes[i-1],b=navNodes[i];int dx=b.x-a.x,dy=b.y-a.y,dd=dx*dx+dy*dy;
  int t=dd?cl((int)((((long long)(here.x-a.x)*dx+(long long)(here.y-a.y)*dy)*1024)/dd),0,1024):0;
  NavPoint q={a.x+(dx*t)/1024,a.y+(dy*t)/1024};
  int x=here.x-q.x,y=here.y-q.y,dist=x*x+y*y;
  if(dist<closest&&(dist<12*12||nav_traversable(here,q,7))){closest=dist;*join=q;first=i;}
 }
 if(closest==(1<<30)){*join=navNodes[0];return 0;}
 return first;
}
static void nav_route(const NavView*v,int large){
 if(navLeg&&!nav_same(navCurb,navGoal)){
  int x=nav_x(v,navCurb.x),y=nav_y(v,navCurb.y),ex=nav_x(v,navGoal.x),ey=nav_y(v,navGoal.y);
  int steps=max(2,(nav_abs(ex-x)+nav_abs(ey-y))/(large?5:4));
  for(int i=1;i<steps;i++){
   int px=x+(ex-x)*i/steps,py=y+(ey-y)*i/steps;
   if(nav_inside(v,px,py,2))disc(px,py,large?2:1,NAV_WALK);
  }
 }
 if(navCount<2)return;
 NavPoint here=nav_player(),join;int first=nav_progress(here,&join);
 for(int pass=0;pass<2;pass++){
  int w=pass?(large?2:1):(large?4:3);U c=pass?NAV_VIOLET:NAV_DARK;
  if(first){nav_stroke(v,here,join,w,c);nav_stroke(v,join,navNodes[first],w,c);}
  for(int i=first+1;i<navCount;i++)nav_stroke(v,navNodes[i-1],navNodes[i],w,c);
 }
}
static void nav_icon(int x,int y,int i,int large){
 const U c=loc[i].tint;
 if(!large){disc(x,y,3,NAV_INK);disc(x,y,2,c);return;}
 disc(x,y,9,NAV_INK);disc(x,y,7,c);disc(x,y,5,NAV_INK);
 switch(loc[i].kind){
  case 0:case 10:rect(x-3,y-3,7,5,c);dot(x-2,y+2,c);dot(x+2,y+2,c);break;
  case 1:case 7:diamond(x,y,4,c);dot(x,y,c);break;
  case 2:case 11:rect(x-3,y-3,7,7,c);rect(x-2,y-1,5,1,NAV_INK);rect(x-2,y+1,5,1,NAV_INK);break;
  case 3:rect(x-4,y-1,9,4,c);rect(x-2,y-3,5,3,c);break;
  case 4:case 5:line(x-3,y+3,x+3,y-3,c);tri(x-3,y-2,x+4,y-4,x+1,y+3,c);break;
  case 6:line(x-3,y+4,x-3,y-4,c);rect(x-2,y-4,6,4,c);dot(x-1,y-3,NAV_INK);dot(x+1,y-2,NAV_INK);break;
  case 8:rect(x-1,y-4,3,9,c);rect(x-4,y-1,9,3,c);break;
  case 14:
   disc(x,y,4,c);disc(x,y,2,NAV_INK);line(x-4,y,x+4,y,c);line(x,y-4,x,y+4,c);dot(x,y,NAV_INK);break;
  case 15:
   rect(x-3,y-4,7,5,c);tri(x-3,y,x+3,y,x,y+4,c);line(x-2,y-2,x+2,y-2,NAV_INK);line(x,y-3,x,y+1,NAV_INK);break;
  default:tri(x-4,y,x,y-4,x+4,y,c);rect(x-3,y,7,4,c);dot(x,y+2,NAV_INK);break;
 }
}
static void nav_heading(int x,int y,int large){
 int n=large?10:8,w=large?6:4;
 float co=cs(p.a),si=sn(p.a);
 int ax=x+(int)(co*n),ay=y+(int)(si*n),bx=x-(int)(co*w+si*w),by=y-(int)(si*w-co*w),dx=x-(int)(co*w-si*w),dy=y-(int)(si*w+co*w);
 disc(x,y,large?10:8,0x202836);
 tri(ax,ay,bx,by,dx,dy,PAPER);
}
static void nav_label(const NavView*v,int x,int y,const char *s){
 int px=nav_x(v,x),py=nav_y(v,y),w=textw(s,0);
 if(px-w/2>v->x&&px+w/2<v->x+v->w)center(px,py,s,0xadb5bb,0);
}
static void nav_overlay(const NavView*v,int large){
 nav_terrain(v,large);
 if(large){
  nav_label(v,460,102,"GLASS MILE");nav_label(v,380,950,"TIDE MARKET");nav_label(v,1140,944,"NIGHTGLASS");
  nav_label(v,1680,638,"EAST FREIGHT");nav_label(v,565,1710,"LANTERN WARD");nav_label(v,1436,1748,"BREAKWATER");
 }
 nav_route(v,large);
 for(int i=0;i<LEN(loc);i++){int x=nav_x(v,loc[i].x),y=nav_y(v,loc[i].y);if(nav_inside(v,x,y,large?10:4))nav_icon(x,y,i,large);}
 NavPoint objective={0,0};
 int realGoal=0;U color=LIME;
 if(job.kind>=0&&job.step>=0&&job.step<job.total){
  realGoal=1;objective=(NavPoint){job.xs[job.step],job.ys[job.step]};color=job.kind<4?TEAL:PINK;
  if(job.kind==0&&job.step<3){static const int drops[]={2,3,0};objective=nav_door(drops[job.step]);}
 }else if(sv.intro==1||sv.intro==2){realGoal=1;objective=nav_door(0);}
 if(realGoal){int x=nav_x(v,objective.x),y=nav_y(v,objective.y);
  if(nav_inside(v,x,y,large?12:8)){diamond(x,y,large?11:7,color);dot(x,y,color);}
 }
 if(navHasManual){int x=nav_x(v,navManual.x),y=nav_y(v,navManual.y);
  if(nav_inside(v,x,y,large?10:7)){diamond(x,y,large?10:6,NAV_VIOLET);disc(x,y,large?3:2,NAV_VIOLET);}
 }
 if(navLeg&&!nav_same(navCurb,navGoal)){int x=nav_x(v,navCurb.x),y=nav_y(v,navCurb.y);
  if(nav_inside(v,x,y,large?8:5)){disc(x,y,large?7:4,NAV_DARK);diamond(x,y,large?6:3,NAV_WALK);if(large)dot(x,y,NAV_WALK);}
 }
 const int xp=nav_x(v,(int)p.x),yp=nav_y(v,(int)p.y);
 nav_heading(xp,yp,large);
 if(large&&navHover){NavPoint q=nav_pin();int x=nav_x(v,q.x),y=nav_y(v,q.y);
  diamond(x,y,14,PAPER);line(x-17,y,x-11,y,PAPER);line(x+11,y,x+17,y,PAPER);line(x,y-17,x,y-11,PAPER);line(x,y+11,x,y+17,PAPER);
 }
}
static void minimap(void){
 nav_update();
 char s[72];NavPoint here=nav_player();int d=0,atCurb=navLeg&&!nav_same(navCurb,navGoal)&&nav_dist(here,navCurb)<18;
 if(navCount>=2){NavPoint join;int first=nav_progress(here,&join);
  d=nav_dist(here,join)+(first?nav_dist(join,navNodes[first]):0);
  for(int i=first+1;i<navCount;i++)d+=nav_dist(navNodes[i-1],navNodes[i]);}
 if(!mapOn){
  const int w=170,h=120,x=W-w-24,y=H-h-40,span=742;
  box(x-4,y-4,w+8,h+30,NAV_INK,NAV_RIM);
  const NavView v={x,y,w,h,(int)p.x-span/2,(int)p.y-span*h/(2*w),span};
  nav_overlay(&v,0);box(x+5,y+4,22,21,NAV_INK,NAV_RIM);center(x+16,y+4,"N",PAPER,0);
  if(navGoalKind){if(atCurb)append(s,"PARK / WALK");else{char *a=append(s,"GPS  ");if(navCount>=2)append(number(a,d)," m");else append(a,nav_dist(here,navGoal)<30?"AT MARKER":"NO ROAD");}
   textat(x+4,y+h+1,s,NAV_VIOLET,0);}
  else textat(x+4,y+h+1,"PORT VANTA",MUTED,0);
  textat(x+w-30,y+h+1,"[M]",PAPER,0);return;
 }
 rect(0,0,W,H,0x0d1420);box(10,10,W-20,H-20,0x111b29,NAV_RIM);
 const NavView v={NAV_MAP_X,NAV_MAP_Y,NAV_MAP_SIZE,NAV_MAP_SIZE,0,0,ROOM};
 box(v.x-6,v.y-6,v.w+12,v.h+12,NAV_INK,NAV_RIM);nav_overlay(&v,1);
 textat(29,22,"PORT",PAPER,1);textat(29,55,"VANTA",PAPER,1);textat(31,98,"STREET ATLAS  /  09",NAV_VIOLET,0);
 textat(31,129,"WAYPOINT",PAPER,0);rect(30,154,183,1,NAV_RIM);
 textat(31,161,"L-CLICK   SET GPS",MUTED,0);
 textat(31,190,"R-CLICK   CLEAR GPS",MUTED,0);
 textat(31,219,"M / ESC   CLOSE MAP",MUTED,0);
 textat(31,253,"ENTER   TOGGLE PIN",MUTED,0);
 textat(31,276,"SPACE   NEXT VENUE",MUTED,0);
 textat(31,303,"LEGEND",PAPER,0);rect(30,327,183,1,NAV_RIM);
 line(37,345,51,345,NAV_VIOLET);textat(61,335,"GPS ROUTE",NAV_VIOLET,0);
 diamond(44,373,6,TEAL);textat(61,362,"ACTIVE CHECKPOINT",TEAL,0);
 disc(44,399,4,GOLD);textat(61,388,"LOCAL SERVICES",GOLD,0);
 tri(44,416,38,428,50,428,PAPER);textat(61,414,"YOUR HEADING",PAPER,0);
 dot(39,450,NAV_WALK);dot(44,450,NAV_WALK);dot(49,450,NAV_WALK);textat(61,440,"WALK FROM CURB",NAV_WALK,0);
 textat(31,479,"NORTH UP",MUTED,0);
 textat(732,53,navKeyboard?"SELECTED":"MAP POINTER",NAV_VIOLET,0);rect(732,79,190,1,NAV_RIM);
 if(navSelected>=0){
  const Place*l=loc+navSelected;
  wrapped(733,90,l->label,l->tint,20);wrapped(733,144,l->note,MUTED,20);
 }else if(navHover){
  textat(732,94,"STREET WAYPOINT",PAPER,0);wrapped(733,129,"Click to pin the highlighted road.",MUTED,20);
 }else{
  textat(732,94,"CITY ATLAS",PAPER,0);wrapped(733,129,"Point at a venue or road to see what you can pin.",MUTED,20);
 }
 textat(732,257,"GUIDANCE",NAV_VIOLET,0);rect(732,281,190,1,NAV_RIM);
 const char *kind=navGoalKind==3?"MANUAL WAYPOINT":navGoalKind==2?"ACTIVE CHECKPOINT":navGoalKind==1?"MERCURY DISPATCH":"NO ACTIVE ROUTE";
 wrapped(733,292,kind,PAPER,20);
 if(navGoalKind){
  if(atCurb)append(s,"PARK HERE / WALK");
  else if(navCount>=2)append(number(s,d),navLeg&&!nav_same(navCurb,navGoal)?" m TO CURB":" m REMAINING");
  else append(s,nav_dist(here,navGoal)<30?"AT DESTINATION":"NO ROAD ROUTE");
  textat(733,330,s,NAV_VIOLET,0);
  if(navLeg&&!nav_same(navCurb,navGoal))wrapped(733,357,"Park, press E to exit, and follow dots to the door.",MUTED,20);
 }
 textat(732,454,"CITY SIGNAL / ONLINE",MUTED,0);
}
#undef NAV_INK
#undef NAV_SEA
#undef NAV_GRID
#undef NAV_CURB
#undef NAV_RIM
#undef NAV_VIOLET
#undef NAV_DARK
#undef NAV_WALK
