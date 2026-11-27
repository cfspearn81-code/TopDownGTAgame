/* Current district walker regression, synthetic careers only. The stable
 * six stations have four identities each; added Harbor stops don't borrow
 * those identities or create poorly aligned duplicate crowd cards. */
#define DC_TEST
#include <game.c>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void hit(U key){tick(key);tick(0);}
static void point(int k){
 scene=1;menu=-1;mapOn=0;
 int node=outerStops[k].node;
 p=(Player){.x=outerNodes[node].x,.y=outerNodes[node].y,.hp=100,.car=-1};
 sv.intro=4;sv.gear=0;sv.dirty=0;
 clear_input();npc_reset();pulse_guard_reset();bank_ui_reset();vd_reset();end_job();
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900.f};
}
static int address_contact(int group){
 /* Start at an actually legible walking spot. Preserve the real Shift+E
  * interaction and range/line checks, not a hardcoded animator offset. */
 for(int seat=0;seat<N_OUTER_SEATS;seat++){
  int id=N_ACT_OUTER+group*N_OUTER_SEATS+seat;NpcRef r;
  if(!npc_ref(id,&r))continue;
  for(int radius=19;radius<=33;radius+=2)for(int k=0;k<12;k++){
   float a=k*.5235988f;
   p.x=r.x+cs(a)*radius;p.y=r.y+sn(a)*radius;p.a=look(r.x-p.x,r.y-p.y);
   NpcRef nearest;
   if(street_foot_clear(p.x,p.y,-1)&&npc_near_ref(&nearest,0)&&nearest.id==id)return id;
  }
 }
 return -1;
}
int main(void){
 init_city();int named=0,holds=0,pairs=0,poses=0;unsigned char ids[N_ACT_MAX]={0};
 for(int shift=0;shift<2;shift++)for(int k=0;k<N_OUTER_SITES;k++){
  point(k);sv.dayClock=shift?1140:690;anim=0;tick(0);
  assert(outer_walk_count()==N_OUTER_SITES*N_OUTER_SEATS&&outCrowdCount==OUT_WALK_COUNT);
  float moved[N_OUTER_SEATS]={0};
  for(int t=0;t<360;t++){
   for(int i=0;i<N_OUTER_SEATS;i++){
    int j=k*N_OUTER_SEATS+i,id=N_ACT_OUTER+j;NpcRef r;float x,y,a;int step;U coat;
    assert(npc_ref(id,&r)&&outer_walk_pose(j,&x,&y,&a,&step,&coat)&&r.id==id);
    assert(x==r.x&&y==r.y&&a==r.a&&coat==r.coat&&!solid(x,y,4)&&outer_walk_away_from_lane(x,y));
    assert(outerInk[(int)y*ROOM+(int)x]<=12); /* even at curved multi-road joins */
    const OuterWalker*w=outWalkers+j;
    assert(outer_walk_check(w->x0,w->y0,w->x1,w->y1));
    for(int u=0;u<OUT_WALK_COUNT;u++)if(u!=j)assert(d2(x-outWalkers[u].x,y-outWalkers[u].y)>=11.f*11.f-.01f);
    if(!ids[id]){named++;ids[id]=1;}
    if(w->pause<=0&&step!=0)moved[i]+=1.f;
    poses++;
   }
   tick(0);
  }
  for(int i=0;i<N_OUTER_SEATS;i++)assert(moved[i]>0.f);
  pairs++;
  int id=address_contact(k);assert(id>=N_ACT_OUTER);
  hit(USE|BRAKE);assert(npcTalkOn&&npcTalk.id==id);
  float x=npcTalk.x,y=npcTalk.y;U before=sv.playedTicks;int routeSeat=id-N_ACT_OUTER;
  sv.dayClock=shift?690:1140;for(int i=0;i<120;i++)tick(0);
  NpcRef r;int step=1;float u,v,a;U coat;
  assert(npcTalkOn&&npc_ref(id,&r)&&r.x==x&&r.y==y&&sv.playedTicks==before+120);
  assert(outWalkers[routeSeat].x==x&&outWalkers[routeSeat].y==y);
  assert(outer_walk_pose(routeSeat,&u,&v,&a,&step,&coat)&&u==x&&v==y&&step==0);
  hit(UP);hit(ENTER);assert(!npcTalkOn&&npcLetters[id]==1&&strstr(notice,r.name));
  holds++;
 }
 printf("Outer Loop: %d distinct people, %d site/shift pairs, %d physical poses and %d E/greet holds.\n",named,pairs,poses,holds);
 assert(named==24&&pairs==12&&holds==12&&poses==12*4*360);
 point(1);sv.dayClock=690;anim=0;tick(0);
 int target=address_contact(1);assert(target>=N_ACT_OUTER);
 hit(USE|BRAKE);assert(npcTalkOn&&npcTalk.id==target);char name[32];strcpy(name,npcTalk.name);
 hit(DOWN);set_session_seed(0);hit(ENTER);
 assert(npcEnc.phase==NPC_DRAW&&npcEnc.actor.id==target&&!npcTalkOn&&sv.dirty==0);
 int prize=npcEnc.prize;for(int i=0;i<90;i++)tick(0);
 assert(npcEnc.phase==NPC_IDLE&&sv.dirty==prize&&npcSpent[target]&&prize==36);
 printf("Outer Loop / %s: surrender and one-time payout $%d passed.\n",name,prize);
}
