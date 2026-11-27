/* Fresh synthetic careers only. Compile this same test with -I pointing
 * at Crossroads or the update to replay identical encounter inputs. */
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
int main(void){
 init_city();int named=0,holds=0,sites=0,scheduled=0,onstage=0,edge=0;unsigned char ids[256]={0};
 for(int shift=0;shift<2;shift++)for(int k=0;k<LEN(outerStops);k++){
  point(k);sv.dayClock=shift?1140:690;anim=0;tick(0);
  NpcRef r;int unique=0;
  for(int t=0;t<160;t++){
   int wanted=k==1||k==2?(outNight()?2:4):outCrowdCount/2;
   scheduled+=wanted;
   OuterPoint at=outerNodes[outerStops[k].node];
   for(int i=0;i<wanted;i++){
    float swing=sn(anim*(outNight()?.7f:1.5f)+i*.81f);
    int ox=at.x+(i-wanted/2)*10+(int)(swing*14.f),oy=at.y+23+(i%2)*7;
    edge+=!!solid(ox,oy,5);
   }
   for(int j=0;j<N_ACT_MAX;j++){
    if(!npc_ref(j,&r)||r.id<N_ACT_OUTER||(j-N_ACT_OUTER)/N_OUTER_SEATS!=k)continue;
    int px=(int)r.x,py=(int)r.y,margin=outerInk[py*ROOM+px];
    assert(!solid(r.x,r.y,5)&&margin>=6&&margin<=12);
    onstage++;unique++;if(!ids[r.id]){named++;ids[r.id]=1;}
   }
   tick(0);
  }
  if(unique)sites++;
  int id=-1;
  for(int j=160;j<N_ACT_MAX;j++)if(npc_ref(j,&r)&&d2(r.x-p.x,r.y-p.y)<38*38&&npc_clear(p.x,p.y,r.x,r.y)){id=j;break;}
  if(id<0){
   for(int at=0;at<90&&id<0;at++){
    NpcRef goal;assert(npc_ref(N_ACT_OUTER+k*N_OUTER_SEATS,&goal));
    float dx=goal.x-p.x,dy=goal.y-p.y;U way=absf(dx)>3?(dx>0?RIGHT:LEFT):0;
    if(absf(dy)>3)way|=dy>0?DOWN:UP;
    tick(way);
    if(npc_near_ref(&r,0)&&r.id>=160)id=r.id;
   }
  }
  if(id<0){fprintf(stderr,"No reachable person shift %d site %d\n",shift,k);continue;}
  hit(USE|BRAKE);assert(npcTalkOn&&npcTalk.id>=160);
  id=npcTalk.id;float x=npcTalk.x,y=npcTalk.y;U before=sv.playedTicks;
  sv.dayClock=shift?690:1140;for(int i=0;i<120;i++)tick(0);
  assert(npcTalkOn&&npc_ref(id,&r)&&r.x==x&&r.y==y&&sv.playedTicks==before+120);
  hit(UP);hit(ENTER);assert(!npcTalkOn&&npcLetters[id]==1&&strstr(notice,r.name));
  holds++;
 }
 printf("Outer Loop / day and night: %d distinct addressable people, %d site/shift pairs, %d freeze/greet routes.\n",named,sites,holds);fflush(stdout);
 printf("Scheduled %d crowd poses; found %d addressable. Baseline formula left the shoulder %d times.\n",scheduled,onstage,edge);fflush(stdout);
 assert(named==24&&sites==12&&holds==12&&onstage==scheduled&&edge>0);
 point(1);sv.dayClock=690;anim=0;tick(0);
 for(int k=0;k<18;k++)tick(LEFT|DOWN);
 hit(USE|BRAKE);assert(npcTalkOn);int target=npcTalk.id;char name[32];strcpy(name,npcTalk.name);
 hit(DOWN);set_session_seed(0);hit(ENTER);assert(npcEnc.phase==NPC_DRAW&&npcEnc.actor.id==target&&!npcTalkOn&&sv.dirty==0);
 int prize=npcEnc.prize;for(int i=0;i<90;i++)tick(0);
 assert(npcEnc.phase==NPC_IDLE&&sv.dirty==prize&&npcSpent[target]&&prize==36);
 printf("Outer Loop / %s: surrender and one-time payout $%d passed.\n",name,prize);
}
