/* Focused regression: real world fixtures and controls; render cannot tick
 * wildlife/play state; all physical assets write z; frame files are optional.
 * Build: cc -std=c11 -O1 src/chase_nature_test.c -o /tmp/chase-nature */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <string.h>
static U image_hash(void){
 U a=2166136261u;for(int j=0;j<W*H;j+=3)a=(a^pixels[j])*16777619u;return a;
}
static int painted(void){int n=0;for(int i=0;i<W*H;i++)n+=chZ[i]!=0;return n;}
static void reset_canvas(float x,float y,int dir){
 ch.x=x;ch.y=y;ch.h=21;ch.si=dir;ch.co=0;chSin=.20f;chCos=.98f;chDark=.2f;chFog=0.f;
 z(pixels,sizeof pixels);z(chZ,sizeof chZ);
}
static void key(U k){tick(0);tick(k);tick(0);}
int main(void){
 init_city();starter_save();sv.intro=4;sv.dayClock=730;scene=1;menu=-1;mapOn=0;fastGfx=0;end_job();vd_reset();
 const OuterPoint q=woodline_sites[WOOD_RANGER];
 p=(Player){.x=q.x,.y=q.y,.hp=100,.car=-1};
 clear_input();if(!ch_pref())key(CAMERA);assert(ch_pref());
 reset_canvas(q.x,q.y-68,1);wl_field();assert(painted()>150);
 reset_canvas(q.x,q.y-68,1);wl_wood();assert(painted()>800);
 const U first=image_hash();reset_canvas(q.x,q.y-68,1);wl_wood();assert(image_hash()==first);
 int secs=sv.playedTicks;U run=rng,wild=ww_rng;paint();paint();assert(run==rng&&ww_rng==wild&&sv.playedTicks==secs);
 key(USE);assert(scene==NIGHT_FOREST_SCENE);paint();key(USE);assert(wf_has(WF_SURVEY_BEGIN));key(MAP);assert(navHasManual);
 key(ESC);assert(scene==1);
 OuterPoint cap=wf_point(wf_patches);p.x=cap.x;p.y=cap.y;
 reset_canvas(cap.x,cap.y-56,1);wl_field();U before=image_hash();key(USE);assert(wf_inv(wf_patches[0].type)==1);
 key(USE);assert(wf_inv(wf_patches[0].type)==1);
 reset_canvas(cap.x,cap.y-56,1);wl_field();assert(image_hash()!=before);
 for(int i=0;i<WW_SPECIES;i++){
  const WildActor*w=&ww_wild[i];assert(w->active);
  reset_canvas(w->x,w->y-72,1);wl_animal(w,i,0);assert(painted()>180);
 }
 reset_canvas(2824,2126,1);ch_harbor_props();assert(painted()>140);
 U safe=0xabcdef;assert(wl_ocean_edge(safe,0xffffffffu,2391)==safe);
 assert(wl_ocean_edge(safe,150,ROOM)==safe);assert(wl_ocean_edge(safe,ROOM+1,2600)!=safe);
 assert(wl_ocean_edge(safe,ROOM-1,2600)!=safe);
 key(GFX);assert(!ch_mode_active()&&ch_pref());key(GFX);assert(ch_mode_active());
 puts("PASS chase nature: forest landmarks, quest state/gathering, wildlife, shore and ocean edge, repeatable paint; C, E, M, Escape, F2.");
 return 0;
}
