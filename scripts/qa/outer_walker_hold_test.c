/* Both renderers read the same physical regional identity an E dialogue
 * captures; the route cannot drift while the dialogue is on screen. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>

int main(void){
 init_city();starter_save();sv.dayClock=605;sv.intro=4;
 end_job();vd_reset();npc_reset();clear_input();
 scene=1;menu=atmOpen=-1;mapOn=0;
 p=(Player){.x=2790,.y=1100,.hp=100,.car=-1};
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=3000};
 for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};
 nc=2;outer_life_reset();outer_life_tick(1.f/60.f);
 assert(outer_walk_count()==24&&outCrowdCount==24);
 for(int i=0;i<OUT_WALK_COUNT;i++){
  NpcRef r;float x,y,a;int step;U coat;
  assert(outer_person_ref(N_ACT_OUTER+i,&r,0)&&outer_walk_pose(i,&x,&y,&a,&step,&coat));
  assert(r.x==x&&r.y==y&&r.a==a&&r.coat==coat&&outer_walk_away_from_lane(x,y));
 }
 int i=6,id=N_ACT_OUTER+i,found=0;
 for(int radius=18;radius<37&&!found;radius+=3)for(int a=0;a<8&&!found;a++){
  float theta=a*.7853982f;OuterWalker*w=outWalkers+i;
  p.x=w->x+cs(theta)*radius;p.y=w->y+sn(theta)*radius;
  p.a=look(w->x-p.x,w->y-p.y);NpcRef target;
  found=street_foot_clear(p.x,p.y,-1)&&npc_near_ref(&target,0)&&target.id==id;
 }
 assert(found);tick(USE);tick(0);assert(npc_holds(id));
 const float holdX=outWalkers[i].x,holdY=outWalkers[i].y,neighbor=outWalkers[i+1].stride;
 for(int frame=0;frame<165;frame++)tick(UP);
 float x,y,a;int step;U coat;
 assert(outer_walk_pose(i,&x,&y,&a,&step,&coat)&&x==holdX&&y==holdY&&step==0);
 assert(outWalkers[i].x==holdX&&outWalkers[i].y==holdY&&neighbor!=outWalkers[i+1].stride);
 tick(ESC);tick(0);assert(!npc_holds(id));
 for(int frame=0;frame<350;frame++)tick(0);
 assert(d2(outWalkers[i].x-holdX,outWalkers[i].y-holdY)>9.f);
 puts("PASS: 24 persistent identities agree with the pose cache; real E/ESC holds and resumes the same walker while others keep moving.");
 return 0;
}
