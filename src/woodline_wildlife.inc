/* Greyreed wildlife is a session-local encounter, not part of the city
 * NPC pool. It cannot overwrite quest actors, loadouts, wages or save data. */
enum{WW_BEAR,WW_WOLF,WW_BOBCAT,WW_SPECIES};
enum{WW_ROAM,WW_ALERT,WW_FOLLOW,WW_WARN,WW_STRIKE,WW_BACKOFF,WW_RETURN};
typedef struct {
 const char *name,*watch,*warning;
 short x,y,leash;
 unsigned char radius,notice,damage,reach;
 float speed,wait,dash,dashTime,recover;
 U top,shade,light;
} WildSpec;
static const WildSpec ww_spec[WW_SPECIES]={
 {"BEAR","Bear: heavy tracks nearby","BEAR / CHARGE",
  381,2618,161,12,126,24,29,57.f,.94f,194.f,.37f,1.95f,0x9a7458,0x4c3f3d,0xd0a77b},
 {"WOLF","Wolf: circling in the trees","WOLF / LUNGE",
  313,2378,157,8,151,15,23,87.f,.54f,184.f,.30f,1.45f,0x7c8990,0x384b59,0xc7c1a3},
 {"BOBCAT","Bobcat: low in the brush","BOBCAT / POUNCE",
  886,2538,148,7,120,12,22,75.f,.69f,216.f,.28f,1.55f,0xc58861,0x694644,0xf4c58a}
};
typedef struct {
 float x,y,homeX,homeY,a,phase,clock,goalX,goalY,glide;
 int state,hit,active,side;
} WildActor;
static WildActor ww_wild[WW_SPECIES];
static U ww_rng;
static int ww_chosen,ww_coolNotice;
#define WW_STAMP_SAFE 34
#define WW_CHEST_SAFE 41
#define WW_RANGER_SAFE 126
static U ww_random(void){ww_rng^=ww_rng<<13;ww_rng^=ww_rng>>17;ww_rng^=ww_rng<<5;return ww_rng;}
static int ww_zone(float x,float y){
 return y>=2214&&y<2980&&x>=58&&x<1041&&woodline_is_grove((int)x,(int)y);
}
static int ww_refuge(float x,float y){
 if(!ww_zone(x,y))return 1;
 if((outer_tile((int)x,(int)y)&127)==TILE_TRAIL||outer_area(x,y,5))return 1;
 if(d2(x-woodline_sites[WOOD_RANGER].x,y-woodline_sites[WOOD_RANGER].y)<=WW_RANGER_SAFE*WW_RANGER_SAFE)return 1;
 for(int i=0;i<LEN(wf_stamps);i++){
  OuterPoint a=woodline_sites[wf_stamps[i]];
  if(d2(x-a.x,y-a.y)<=WW_STAMP_SAFE*WW_STAMP_SAFE)return 1;
 }
 for(int i=0;i<LEN(wf_chests);i++){
  OuterPoint a=wf_point(wf_chests+i);
  if(d2(x-a.x,y-a.y)<=WW_CHEST_SAFE*WW_CHEST_SAFE)return 1;
 }
 return 0;
}
static int ww_body(float x,float y,int r){
 /* Keep the whole pounce/strike out of the trails and quest circles. */
 return !ww_refuge(x,y)&&outer_walkable(x,y,2)&&outer_walkable(x,y,r);
}
static int ww_can_see(const WildActor*w,float x,float y){
 return street_line_open(w->x,w->y,x,y,2);
}
static int ww_try(WildActor*w,const WildSpec*s,float dx,float dy,int leash){
 float x=w->x+dx,y=w->y+dy;
 if(d2(x-w->homeX,y-w->homeY)>leash*leash||!ww_body(x,y,s->radius))return 0;
 w->x=x;w->y=y;return 1;
}
static int ww_step(WildActor*w,const WildSpec*s,float angle,float speed,float dt,int sweeping){
 static const float steer[]={0,.43f,-.43f,.82f,-.82f,1.26f,-1.26f};
 float r=speed*dt;int count=sweeping?LEN(steer):1;
 for(int i=0;i<count;i++){
  float a=wrap(angle+steer[i]*w->side),dx=cs(a)*r,dy=sn(a)*r;
  if(ww_try(w,s,dx,dy,s->leash)){
   w->a=a;w->glide=speed;return 1;
  }
 }
 w->glide=0;return 0;
}
static int ww_home(WildActor*w,int i){
 const WildSpec*s=ww_spec+i;
 for(int radius=0;radius<=92;radius+=9)for(int j=0;j<(radius?16:1);j++){
  float a=j*.3926991f,x=s->x+cs(a)*radius,y=s->y+sn(a)*radius;
  if(!ww_body(x,y,s->radius+2))continue;
  /* A den tucked in a one-tile niche traps a wide creature. Pick a
   * nearby dry home with a genuine visible runway into the woods. */
  int open=0;
  for(int k=0;k<16;k++){
   float ang=k*.3926991f,xx=x+cs(ang)*53,yy=y+sn(ang)*53;
   if(!ww_body(xx,yy,s->radius)||!street_line_open(x,y,xx,yy,s->radius))continue;
   int all=1;
   for(int t=1;t<=6;t++)if(ww_refuge(x+(xx-x)*t/6.f,y+(yy-y)*t/6.f)){all=0;break;}
   if(all){open=1;break;}
  }
  if(!open)continue;
  *w=(WildActor){.x=x,.y=y,.homeX=x,.homeY=y,.a=i*1.25f,.phase=i*.23f,.state=WW_ROAM,.active=1,.side=i%2?1:-1};
  return 1;
 }
 return 0;
}
static int ww_hot(const WildActor*w,const WildSpec*s,int extra){
 if(scene!=1||p.car>=0||p.dead||ww_refuge(p.x,p.y))return 0;
 int r=s->notice+extra,leash=s->leash+9;
 return d2(p.x-w->x,p.y-w->y)<r*r&&d2(p.x-w->homeX,p.y-w->homeY)<leash*leash;
}
static void ww_begin(WildActor*w,int state,float sec){w->state=state;w->clock=sec;w->glide=0;}
static void ww_hunter(WildActor*w,int i,float dt){
 const WildSpec*s=ww_spec+i;
 if(!w->active)return;
 int alert=ww_hot(w,s,44),fresh=ww_hot(w,s,0);
 if(w->state>=WW_ALERT&&w->state<=WW_BACKOFF&&!alert)ww_begin(w,WW_RETURN,0);
 w->clock-=dt;
 w->phase=wrap(w->phase+dt*(w->glide>125?14.f:w->glide>55?10.f:3.4f));
 if(w->state==WW_ROAM){
  if(fresh&&ww_can_see(w,p.x,p.y)){ww_begin(w,WW_ALERT,i==WW_BEAR?.52f:i==WW_WOLF?.42f:.36f);w->a=look(p.x-w->x,p.y-w->y);return;}
  if(w->clock<=0||d2(w->x-w->goalX,w->y-w->goalY)<11*11){
   float a=(float)(ww_random()%360)*(.01745329f),r=20+(ww_random()%42);
   w->goalX=w->homeX+cs(a)*r;w->goalY=w->homeY+sn(a)*r;w->clock=2.7f+(ww_random()%110)*.019f;
  }
  ww_step(w,s,look(w->goalX-w->x,w->goalY-w->y),i==WW_BEAR?21.f:i==WW_WOLF?35.f:28.f,dt,1);return;
 }
 if(w->state==WW_RETURN){
  if(d2(w->x-w->homeX,w->y-w->homeY)<15*15){ww_begin(w,WW_ROAM,0);return;}
  ww_step(w,s,look(w->homeX-w->x,w->homeY-w->y),s->speed*.68f,dt,1);
  /* If a permanent obstacle trapped its previous route, re-seed only
   * after it is well outside the viewer's playable neighbourhood. */
  if(w->clock<-6.f&&d2(p.x-w->x,p.y-w->y)>630*630)ww_home(w,i);
  return;
 }
 float aim=look(p.x-w->x,p.y-w->y),distance=d2(p.x-w->x,p.y-w->y);
 if(w->state==WW_ALERT){
  w->a=aim;w->glide=0;
  if(w->clock<=0)ww_begin(w,WW_FOLLOW,2.7f);return;
 }
 if(w->state==WW_FOLLOW){
  int near=s->reach+(i==WW_BEAR?43:i==WW_WOLF?32:43);
  if(distance<near*near&&ww_can_see(w,p.x,p.y)){
   ww_begin(w,WW_WARN,s->wait);w->a=aim;w->goalX=p.x;w->goalY=p.y;w->hit=0;return;
  }
  if(w->clock<=0){ww_begin(w,WW_RETURN,0);return;}
  /* A wolf approaches on an arc; the bear walks straight in. */
  float flank=i==WW_WOLF&&distance>50*50?sn(w->phase*.56f)*.40f:0.f;
  ww_step(w,s,wrap(aim+flank*w->side),s->speed,dt,1);return;
 }
 if(w->state==WW_WARN){
  /* Only commit once the warning finishes. The marked direction then
   * remains fixed until the short strike window expires. */
  if(w->clock>s->wait*.49f&&ww_can_see(w,p.x,p.y)){
   w->goalX=p.x;w->goalY=p.y;w->a=aim;
  }
  if(w->clock<=0){ww_begin(w,WW_STRIKE,s->dashTime);w->a=look(w->goalX-w->x,w->goalY-w->y);}return;
 }
 if(w->state==WW_STRIKE){
  float locked=w->a;
  if(w->clock>0)ww_step(w,s,locked,s->dash,dt,0);
  w->a=locked;
  int reach=s->reach;
  if(!w->hit&&alert&&d2(p.x-w->x,p.y-w->y)<=reach*reach&&ww_can_see(w,p.x,p.y)){
   w->hit=1;
   if(p.hit<=0){
    health(s->damage);
    if(!p.dead)tell(i==WW_BEAR?"Bear charge hit.":i==WW_WOLF?"Wolf lunge hit.":"Bobcat pounce hit.");
   }
   ww_begin(w,WW_BACKOFF,s->recover);return;
  }
  if(w->clock<=0){ww_begin(w,WW_BACKOFF,s->recover);return;}
  return;
 }
 if(w->state==WW_BACKOFF){
  if(w->clock>s->recover*.52f)
   ww_step(w,s,wrap(aim+(i==WW_WOLF?1.61f*w->side:3.141593f)),s->speed*.65f,dt,1);
  else w->a=aim;
  if(w->clock<=0)ww_begin(w,WW_FOLLOW,2.8f);
 }
}

/* A separate occasional outskirts story. Its two hikers are intentionally
 * ambient set dressing, not reusable citizen, Ranger, taxi or job slots. */
enum{WW_HIKERS=2,WW_QUIET,WW_ENTERING,WW_TAKEN,WW_WITHDRAW,
     WW_RARE_MIN=11*60*60,WW_RARE_SPAN=5*60*60,WW_HIKER_REST=105*60};
typedef struct{float x,y,midX,midY,fromX,fromY;int away;} WildHiker;
static WildHiker ww_hikers[WW_HIKERS];
typedef struct{float x,y,phase,a;int state,countdown,victim,species,ticks;} WildIncursion;
static WildIncursion ww_inc;
static void ww_delay(void){ww_inc.countdown=WW_RARE_MIN+(int)(ww_random()%WW_RARE_SPAN);}
static int ww_edge_pos(float x,float y,float tx,float ty){
 return street_line_open(x,y,tx,ty,6)&&(!outer_is_new(tx,ty)||outer_walkable(tx,ty,7));
}
static void ww_edge_init(void){
 ww_hikers[0]=(WildHiker){.x=250,.y=1980,.midX=250,.midY=1980,.fromX=260,.fromY=2071};
 ww_hikers[1]=(WildHiker){.x=815,.y=1983,.midX=815,.midY=1983,.fromX=802,.fromY=2060};
}
static int ww_inc_begin(void){
 if(p.car>=0||p.dead||p.y<1810||p.y>2140)return 0;
 int first=(int)(ww_random()%WW_HIKERS);
 for(int k=0;k<WW_HIKERS;k++){
  int who=(first+k)%WW_HIKERS;
  WildHiker*h=ww_hikers+who;
  if(h->away||npc_holds(N_ACT_OUT_HIKER+who)||npc_replaces(N_ACT_OUT_HIKER+who)||
     street_extra_down(N_ACT_OUT_HIKER+who)||d2(h->x-p.x,h->y-p.y)>270*270||!ww_edge_pos(h->fromX,h->fromY,h->x,h->y))continue;
  ww_inc.victim=who;ww_inc.species=(int)(ww_random()%WW_SPECIES);ww_inc.state=WW_ENTERING;
  ww_inc.ticks=0;ww_inc.x=h->fromX;ww_inc.y=h->fromY;ww_inc.phase=0.f;return 1;
 }
 return 0;
}
static void ww_edge_step(float targetX,float targetY,float rate,float dt){
 float a=look(targetX-ww_inc.x,targetY-ww_inc.y),x=ww_inc.x+cs(a)*rate*dt,y=ww_inc.y+sn(a)*rate*dt;
 if(ww_edge_pos(ww_inc.x,ww_inc.y,x,y)){
  ww_inc.x=x;ww_inc.y=y;ww_inc.a=a;ww_inc.phase=wrap(ww_inc.phase+10.f*dt);
 }
}
static void ww_inc_tick(float dt){
 for(int i=0;i<WW_HIKERS;i++){
  WildHiker*h=ww_hikers+i;
  if(h->away){
   if(h->away>1)h->away--;
   if(h->away==1&&d2(h->midX-p.x,h->midY-p.y)>425*425){h->away=0;h->x=h->midX;h->y=h->midY;}
   continue;
  }
  if((ww_inc.victim==i&&ww_inc.state!=WW_QUIET)||npc_holds(N_ACT_OUT_HIKER+i)||
     npc_replaces(N_ACT_OUT_HIKER+i)||street_extra_down(N_ACT_OUT_HIKER+i))continue;
  float w=sn(p.secs*.56f+i*2.5f)*24.f;
  if(ww_edge_pos(h->midX,h->midY,h->midX+w,h->midY))h->x=h->midX+w;
 }
 if(ww_inc.state==WW_QUIET){
  if(ww_inc.countdown>0&&--ww_inc.countdown>0)return;
  if(!ww_inc_begin())ww_inc.countdown=13*60+(int)(ww_random()%(10*60));
  return;
 }
 WildHiker*h=ww_hikers+ww_inc.victim;ww_inc.ticks++;
 if(ww_inc.state==WW_ENTERING){
  int id=N_ACT_OUT_HIKER+ww_inc.victim;
  if(npc_holds(id)||npc_replaces(id)||street_extra_down(id)){ww_inc.state=WW_WITHDRAW;ww_inc.ticks=0;return;}
  if(d2(h->x-ww_inc.x,h->y-ww_inc.y)<13*13){
   h->away=WW_HIKER_REST;ww_inc.state=WW_TAKEN;ww_inc.ticks=0;
   if(d2(h->x-p.x,h->y-p.y)<280*280)tell("A predator carried a Seawall hiker into the woods.");return;
  }
  ww_edge_step(h->x,h->y,69.f,dt);
  if(ww_inc.ticks>6*60){ww_inc.state=WW_WITHDRAW;ww_inc.ticks=0;}
 }else{
  ww_edge_step(h->fromX,h->fromY,ww_inc.state==WW_TAKEN?63.f:78.f,dt);
  if(d2(h->fromX-ww_inc.x,h->fromY-ww_inc.y)<8*8||ww_inc.ticks>6*60){
   ww_inc.state=WW_QUIET;ww_inc.victim=-1;ww_delay();
  }
 }
}
static void woodline_wild_reset(void){
 ww_rng=0x7385a471u^(U)(sideSlot*0x39856e21u);
 if(!ww_rng)ww_rng=1;
 for(int i=0;i<WW_SPECIES;i++){ww_wild[i]=(WildActor){0};ww_home(ww_wild+i,i);}
 ww_chosen=-1;ww_coolNotice=0;ww_inc=(WildIncursion){.state=WW_QUIET,.victim=-1};ww_delay();ww_edge_init();
}
static void woodline_wild_tick(float dt){
 if(scene!=1||p.dead||menu>=0||mapOn||vdPanel||atmOpen>=0||wheelOpen)return;
 if(ww_coolNotice>0)ww_coolNotice--;
 if(p.x<1480&&p.y>1700){
  int picked=-1;
  for(int i=0;i<WW_SPECIES;i++){
   ww_hunter(ww_wild+i,i,dt);if(p.dead)break;
   const WildActor*w=ww_wild+i;
   if(w->active&&w->state>=WW_ALERT&&w->state<=WW_BACKOFF&&(picked<0||
     d2(w->x-p.x,w->y-p.y)<d2(ww_wild[picked].x-p.x,ww_wild[picked].y-p.y)))picked=i;
  }
  if(ww_chosen>=0&&picked<0&&ww_refuge(p.x,p.y)&&!ww_coolNotice&&!p.dead){
   tell("Wildlife backed off. Marked trails and ranger sites are safe.");ww_coolNotice=120;
  }
  ww_chosen=picked;
 }else{
  /* A long trip out of the forest releases any earlier pursuit. */
  for(int i=0;i<WW_SPECIES;i++)if(ww_wild[i].state!=WW_ROAM)ww_begin(ww_wild+i,WW_RETURN,0);
  ww_chosen=-1;
 }
 if(!p.dead)ww_inc_tick(dt);
}

typedef struct{int x,y;float co,si,k;} WildInk;
typedef struct{int x,y;} WildPt;
static WildPt ww_project(WildInk c,float u,float v,float h){
 return (WildPt){c.x+(int)((u*c.co-v*c.si)*c.k),c.y+(int)(((u*c.si+v*c.co)*.61f-h)*c.k)};
}
static void ww_disc(WildInk c,float u,float v,float h,int r,U ink){
 WildPt a=ww_project(c,u,v,h);disc(a.x,a.y,max(1,(int)(r*c.k)),ink);
}
static void ww_line(WildInk c,float u,float v,float h,float a,float b,float d,U ink,int thickness){
 WildPt s=ww_project(c,u,v,h),e=ww_project(c,a,b,d);
 for(int w=0;w<thickness;w++)line(s.x+w-thickness/2,s.y,e.x+w-thickness/2,e.y,ink);
}
static void ww_tri(WildInk c,float x,float y,float z,float u,float v,float w,float a,float b,float d,U ink){
 WildPt p=ww_project(c,x,y,z),q=ww_project(c,u,v,w),r=ww_project(c,a,b,d);tri(p.x,p.y,q.x,q.y,r.x,r.y,ink);
}
static void ww_body_art(const WildActor*w,int i,int x,int y,float k,int display){
 const WildSpec*s=ww_spec+i;
 float alert=w->state==WW_WARN||w->state==WW_STRIKE,move=w->glide>=10.f?1.f:0.f;
 float run=sn(w->phase)*move,h=i==WW_BEAR?14.f:i==WW_WOLF?10.f:9.f;
 float crouch=i==WW_BOBCAT&&w->state==WW_WARN?3.5f:0.f;
 float raise=i==WW_BEAR&&w->state==WW_WARN?(sn(w->clock*2.6f)+1.f)*3.f:0.f;
 U core=mix(s->top,0x213833,(int)(nw_dark()*3.f)),dark=mix(s->shade,0x1b272d,(int)(nw_dark()*3.f));
 U fur=mix(s->light,s->top,(int)(nw_dark()*4.f));
 WildInk c={x,y,cs(w->a),sn(w->a),k};
 for(int yy=-4;yy<=4;yy++){
  int rx=(i==WW_BEAR?24:19)-yy*yy/3;
  rect(x-(int)(rx*k),y+yy,(int)(rx*2*k),1,0x273c37);
 }
 for(int side=-1;side<=1;side+=2)for(int pair=-1;pair<=1;pair+=2){
  int u=pair*(i==WW_BEAR?10:i==WW_WOLF?9:7),v=side*(i==WW_BEAR?5:3);
  float rhythm=pair*side*run*(i==WW_BEAR?3.4f:5.2f);
  ww_line(c,u,v,h-2.f-crouch,u+rhythm,v+side,0,dark,i==WW_BEAR?4:2);
  ww_line(c,u+rhythm,v+side,0,u+rhythm+2.2f,v+side,0,core,i==WW_BEAR?3:2);
 }
 if(i!=WW_BEAR){
  int tail=i==WW_WOLF?23:16;float wag=sn(w->phase*.64f)*3.1f;
  ww_line(c,-9,0,h-2,-tail,wag,i==WW_WOLF?6:9,dark,i==WW_WOLF?4:3);
  ww_line(c,-tail+3,wag*.77f,i==WW_WOLF?6:9,-tail,wag,i==WW_WOLF?6:9,fur,i==WW_WOLF?2:1);
 }
 for(int u=-10;u<=9;u+=4){
  int r=i==WW_BEAR?(u>2?10:8):i==WW_WOLF?(u>2?6:5):5;
  ww_disc(c,u,0,h-crouch,r,dark);
  ww_disc(c,u+1,-.8f,h+2.f-crouch,r-1,core);
 }
 if(i==WW_BEAR){
  ww_disc(c,10,0,h+4+raise,10,core);
  ww_disc(c,15,0,h+6+raise,7,core);
  ww_disc(c,18,0,h+7+raise,6,fur);
  ww_disc(c,13,-4,h+13+raise,3,dark);ww_disc(c,13,4,h+13+raise,3,dark);
  ww_disc(c,22,0,h+7+raise,2,dark);
  ww_disc(c,17,3,h+10+raise,1,alert?PAPER:GOLD);
 }else if(i==WW_WOLF){
  ww_disc(c,11,0,h+4,5,core);
  ww_tri(c,11,-4,h+6,15,-3,h+6,12,-4,h+14,dark);
  ww_tri(c,11,4,h+6,15,3,h+6,12,4,h+14,core);
  ww_tri(c,11,-4,h+2,11,4,h+2,22,0,h+1,fur);
  ww_disc(c,21,0,h+2,2,dark);
  ww_disc(c,15,3,h+6,1,alert?PAPER:GOLD);
 }else{
  ww_disc(c,10,0,h+3-crouch,5,core);
  ww_tri(c,7,3,h+4-crouch,12,3,h+4-crouch,9,5,h+11-crouch,dark);
  ww_tri(c,7,-3,h+4-crouch,12,-3,h+4-crouch,9,-5,h+11-crouch,dark);
  ww_disc(c,14,1,h+2-crouch,2,fur);
  for(int l=0;l<5;l++)ww_disc(c,-8+l*3,l%2?3:-3,h+3-crouch,1,dark);
  ww_disc(c,12,3,h+5-crouch,1,alert?PAPER:0xf5d981);
 }
 if(display){
  U tone=w->state==WW_STRIKE?PINK:0xffcb7a;
  int span=i==WW_BEAR?42:34,top=y-(i==WW_BEAR?41:32);
  box(x-span/2,top,span,5,0x22333a,tone);
  int fill=w->state==WW_WARN?cl(span-4-(int)(w->clock/s->wait*(span-4)),0,span-4):span-4;
  rect(x-span/2+2,top+2,fill,1,tone);
  int lx=x+(int)(cs(w->a)*21),ly=y+(int)(sn(w->a)*17);
  if(w->state==WW_WARN||w->state==WW_STRIKE)diamond(lx,ly,4,tone);
 }
}
static int ww_visible(float x,float y){return x>=cx-54&&x<=cx+W+54&&y>=cy-54&&y<=cy+H+72;}
static void woodline_wild_paint(void){
 if(cy+H<1880||cx>1250)return;
 if(ww_inc.state!=WW_QUIET&&ww_visible(ww_inc.x,ww_inc.y)){
  int x=sx(ww_inc.x),y=sy(ww_inc.y);
  WildActor w={.x=ww_inc.x,.y=ww_inc.y,.a=ww_inc.a,.phase=ww_inc.phase,.glide=64};
  ww_body_art(&w,ww_inc.species,x,y,.85f,0);
  if(ww_inc.state==WW_TAKEN){int u=x+(int)(cs(w.a)*-14),v=y+(int)(sn(w.a)*-14);
   line(u,v,x,y,0x675a4b);box(u-6,v-5,10,7,0x6f6851,0x9e9a76);
   /* The hiker is taken into the woodland without a graphic animation. */
  }
 }
 for(int i=0;i<WW_SPECIES;i++){
  const WildActor*w=ww_wild+i;if(!w->active||!ww_visible(w->x,w->y))continue;
  ww_body_art(w,i,sx(w->x),sy(w->y),i==WW_BEAR?1.10f:1.f,
              i==ww_chosen&&(w->state==WW_WARN||w->state==WW_STRIKE));
 }
}
static void woodline_wild_hint(void){
 if(scene!=1||p.dead||p.car>=0||menu>=0||mapOn||vdPanel||atmOpen>=0||wheelOpen)return;
 int i=ww_chosen;if(i<0)return;
 const WildActor*w=ww_wild+i;const WildSpec*s=ww_spec+i;
 int warn=w->state==WW_WARN||w->state==WW_STRIKE;
 U ink=warn?0xffbe76:0xd0d6a8;
 const char*title=warn?s->warning:s->watch;
 box(257,93,446,55,0x192630,ink);center(W/2,94,title,ink,0);
 center(W/2,119,"SHIFT sprint / marked trails and clearings are safe",PAPER,0);
}
