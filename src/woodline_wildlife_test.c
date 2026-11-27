/* Real fixed-step gameplay. Positions arrange deterministic approaches;
 * the strike, damage, flee and incursion all run through normal tick(). */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void wpress(U key){tick(key);tick(0);}
static void place(float x,float y){
 scene=1;mapOn=0;menu=-1;atmOpen=-1;vdPanel=0;sv.intro=4;p.car=-1;
 p.x=x;p.y=y;p.hp=100;p.dead=0;p.hit=p.heat=p.quiet=0;sv.armor=0;clear_input();
}
static void pause_integrity(WildActor *w){
 U before=ww_rng;float clock=w->clock,px=w->x,py=w->y;int state=w->state,hp=p.hp,elapsed=ww_inc.countdown;
 tick(ESC);tick(0);assert(menu==PAUSE_MENU);
 for(int n=0;n<185;n++)tick(0);
 assert(w->x==px&&w->y==py&&w->clock==clock&&w->state==state&&p.hp==hp&&elapsed==ww_inc.countdown&&before==ww_rng);
 tick(ESC);assert(menu<0);clear_input();
}
static void approach(int i){
 WildActor*w=ww_wild+i;const WildSpec*s=ww_spec+i;
 float bx=-999,by=-999,best=-1;
 for(int r=42;r<90;r+=3)for(int j=0;j<32;j++){
  float a=j*.19634954f,x=w->homeX+cs(a)*r,y=w->homeY+sn(a)*r;
  if(ww_refuge(x,y)||solid(x,y,10)||!street_line_open(w->homeX,w->homeY,x,y,s->radius+2))continue;
  if(!street_foot_car_clear(x,y,-1,7.f))continue;
  int route=1;
  for(int k=1;k<=14;k++)if(ww_refuge(w->homeX+(x-w->homeX)*k/14.f,w->homeY+(y-w->homeY)*k/14.f))route=0;
  if(!route)continue;
  float score=r;
  if(score>best){bx=x;by=y;best=score;}
 }
 if(bx<=0.f)fprintf(stderr,"No approach to %s %.2f/%.2f\n",s->name,w->homeX,w->homeY);
 assert(bx>0.f);place(bx,by);
 printf("%s: home %.2f,%.2f; forager starts %.2f,%.2f.\n",s->name,w->homeX,w->homeY,bx,by);
}
static void test_species(int i){
 woodline_wild_reset();
 WildActor*w=ww_wild+i;const WildSpec*s=ww_spec+i;
 assert(w->active&&ww_body(w->x,w->y,s->radius));
 approach(i);int frame=0,warned=0,escaped=0,before=p.hp;
 for(;frame<310&&p.hp==before;frame++){
  tick(0);
  if(!warned&&w->state==WW_WARN){
   warned=1;
   assert(w->clock>s->wait-.035f&&p.hp==before);
   if(i==WW_WOLF)pause_integrity(w);
  }
  assert(!p.dead&&(!warned||ww_body(w->x,w->y,s->radius)));
 }
 if(!(warned&&p.hp==before-s->damage))fprintf(stderr,"Attack %d stalled: %d hp %d warning %d at %.1f, %.1f\n",i,w->state,p.hp,warned,w->x,w->y);
 assert(warned&&p.hp==before-s->damage);
 printf(" hit after %d frames, %d HP.\n",frame,s->damage);
 /* From its attacked position, actually sprint to a natural refuge
  * along an unobstructed cardinal walk; no scene switch or warp. */
 static const U keys[]={UP,DOWN,LEFT,RIGHT};
 static const int mx[]={0,0,-1,1},my[]={-1,1,0,0};
 int route=-1;float startX=p.x,startY=p.y;
 for(int n=5;n<140&&route<0;n+=3)for(int k=0;k<4;k++){
  float x=startX+mx[k]*n,y=startY+my[k]*n;
  if(ww_refuge(x,y)&&street_line_open(startX,startY,x,y,10)){route=k;break;}
 }
 assert(route>=0);
 for(frame=0;frame<150;frame++){
  tick(keys[route]|BRAKE);
  if(ww_refuge(p.x,p.y)&&w->state!=WW_WARN&&w->state!=WW_STRIKE){escaped=1;break;}
 }
 assert(escaped&&d2(p.x-startX,p.y-startY)>9.f);int hp=p.hp;
 clear_input();for(int n=0;n<185;n++)tick(0);
 assert(p.hp==hp&&w->state!=WW_WARN&&w->state!=WW_STRIKE&&!p.dead);
 printf(" escaped via held Shift in %d frames; safe for 185 more.\n",frame);
}
static void test_safe(void){
 for(int i=0;i<WOOD_N_SITES;i++)
  assert(outer_walkable(woodline_sites[i].x,woodline_sites[i].y,10));
 for(int i=0;i<LEN(wf_stamps);i++){OuterPoint a=woodline_sites[wf_stamps[i]];assert(ww_refuge(a.x,a.y));}
 for(int i=0;i<LEN(wf_chests);i++){OuterPoint a=wf_point(wf_chests+i);assert(ww_refuge(a.x,a.y));}
 assert(ww_refuge(520,1995)&&ww_refuge(520,2060)&&ww_refuge(560,2100));
}
static void test_forage_risk(void){
 /* There must be a genuine vulnerable specimen approach, not just a
  * monster in unrelated empty grass. Preserve safe options via trails. */
 for(int patch=0;patch<LEN(wf_patches);patch++){
  OuterPoint q=wf_point(wf_patches+patch);
  for(int dx=-30;dx<=30;dx+=5)for(int dy=-30;dy<=30;dy+=5){
   if(dx*dx+dy*dy>30*30||ww_refuge(q.x+dx,q.y+dy)||solid(q.x+dx,q.y+dy,10))continue;
   woodline_wild_reset();place(q.x+dx,q.y+dy);
   if(wf_target().kind!=WF_OBJ_PATCH)continue;
   for(int i=0;i<WW_SPECIES;i++)if(ww_hot(ww_wild+i,ww_spec+i,0)&&ww_can_see(ww_wild+i,p.x,p.y)){
    int began=0;
    for(int t=0;t<140&&!began;t++){tick(0);began=ww_chosen>=0;}
    if(began){printf("Forage pressure: off-trail %s within E range at %.1f,%.1f.\n",wf_fungi[wf_patches[patch].type],p.x,p.y);return;}
   }
  }
 }
 assert(!"no off-trail specimen can be threatened");
}
static void test_incursion(void){
 woodline_wild_reset();
 place(250,1980);assert(!solid(p.x,p.y,10));
 int earliest=ww_inc.countdown;
 assert(earliest>=WW_RARE_MIN&&earliest<WW_RARE_MIN+WW_RARE_SPAN);
 U legacy=rng;woodline_wild_tick(1.f/60.f);assert(rng==legacy);
 /* It cannot run at a random city location. */
 place(512,768);ww_inc.countdown=1;woodline_wild_tick(1.f/60.f);
 assert(ww_inc.state==WW_QUIET&&ww_inc.countdown>13*60-2);
 place(250,1980);ww_inc.countdown=WW_RARE_MIN;
 int money=sv.cash,crime=sv.crime,legal=sv.legal,dirty=sv.dirty,hp=p.hp;
 assert(ww_edge_pos(ww_hikers[0].fromX,ww_hikers[0].fromY,ww_hikers[0].x,ww_hikers[0].y));
 /* Fast-forward with normal encounter updates for almost the full
  * minimum window, then use real street ticks for the visible event. */
 for(int n=0;n<WW_RARE_MIN-1;n++)woodline_wild_tick(1.f/60.f);
 assert(ww_inc.state==WW_QUIET&&ww_inc.countdown==1&&!ww_hikers[0].away&&!ww_hikers[1].away);
 tick(0);assert(ww_inc.state==WW_ENTERING);int victim=ww_inc.victim;
 assert(victim>=0&&victim<WW_HIKERS);
 int seen=0,a=0;
 for(;a<12*60&&ww_inc.state!=WW_QUIET;a++){
  tick(0);if(ww_inc.state==WW_TAKEN)seen=1;
 }
 assert(seen&&a<12*60&&ww_hikers[victim].away&&ww_inc.victim<0);
 assert(ww_inc.countdown>=WW_RARE_MIN&&ww_inc.countdown<WW_RARE_MIN+WW_RARE_SPAN);
 assert(p.hp==hp&&sv.cash==money&&sv.crime==crime&&sv.legal==legal&&sv.dirty==dirty);
 /* Hiker stays away even after the cooldown if the player is still there. */
 ww_hikers[victim].away=2;tick(0);assert(ww_hikers[victim].away==1);
 place(1000,1450);tick(0);assert(!ww_hikers[victim].away);
 assert(ww_hikers[victim].x==ww_hikers[victim].midX);
 puts("Natural minimum timer, urban exclusion, ambient-only take-away, no reward, delayed offscreen repopulation passed.");
}
int main(void){
 init_city();profiles_activate();sv.intro=4;sv.owned=0;sv.dayClock=800.f;nc=0;
 test_safe();
 for(int i=0;i<WW_SPECIES;i++)test_species(i);
 test_forage_risk();test_incursion();return 0;
}
