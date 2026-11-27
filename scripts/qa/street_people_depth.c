/* Deterministic visual/geometry fixture for the actual shared raster.
   gcc -std=c11 -O1 scripts/qa/street_people_depth.c -o build/street-people
   build/street-people [existing screenshot directory]
   For before/after, use an older engine plus only the scene setup and capture.
   Does not seed/modify a real career or replace the separate host-SDL replay. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
static const char*out;
static U untouched[W*H];
static void snap(const char*n){
 if(!out)return;
 char path[384];snprintf(path,sizeof path,"%s/%s.ppm",out,n);FILE*f=fopen(path,"wb");assert(f);
 fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc((c>>16)&255,f);fputc((c>>8)&255,f);fputc(c&255,f);}
 assert(!fclose(f));
}
static void fixture(void){
 init_city();scene=1;sv.intro=4;sv.owned=1;sv.dayClock=720;job.kind=-1;
 z(peds,sizeof peds);nc=1;p.car=0;p.x=256;p.y=690;
 cars[0]=(Car){.x=256,.y=690,.a=-1.5707963f,.owner=1,.cond=100};
 peds[1]=(Ped){.x=226,.y=635,.a=.35f,.s=0,.hp=40};
 peds[2]=(Ped){.x=286,.y=610,.a=1.47f,.s=41,.hp=40};
 peds[3]=(Ped){.x=262,.y=565,.a=3.0f,.s=64,.hp=40,.role=1};
 peds[4]=(Ped){.x=297,.y=683,.a=-.54f,.s=41,.hp=40};
 peds[5]=(Ped){.x=244,.y=614,.a=.73f,.s=53,.hp=40,.role=2};
 anim=.42f;
}
static int delta(int x,int y,int w,int h){
 int n=0;for(int j=max(0,y);j<min(H,y+h);j++)for(int i=max(0,x);i<min(W,x+w);i++)
  n+=pixels[j*W+i]!=untouched[j*W+i];
 return n;
}
int main(int argc,char**argv){
 out=argc>1?argv[1]:0;fixture();
 U sim0=night_hash(peds,sizeof peds);nv.garage[60]=1;paint();assert(chProjected);
 snap("street-chase-day");anim=.64f;paint();snap("street-chase-stride");
 assert(night_hash(peds,sizeof peds)==sim0);
 StreetPerson actor;assert(street_ped_pose(4,&actor)&&actor.x==peds[4].x&&actor.y==peds[4].y);
 /* Unchanged 15-ish unit figure height, real depth-tested head/legs/shadow. */
 ChPlot foot=ch_screen(ch_local((ChVec){actor.x,actor.y,0.f}));
 ChPlot top=ch_screen(ch_local((ChVec){actor.x,actor.y,15.f}));
 assert(foot.y-top.y>25&&foot.y-top.y<92);
 ch_ground();cp(untouched,pixels,sizeof pixels);
 actor.step=0;ch_character(&actor);
 assert(delta(foot.x-25,top.y-2,50,11)>16&&delta(foot.x-25,foot.y-3,50,8)>14);
 cp(untouched,pixels,sizeof pixels);ch_ground();actor.step=4;ch_character(&actor);
 assert(delta(foot.x-25,foot.y-25,50,26)>30);
 cp(untouched,pixels,sizeof pixels);
 for(int i=0;i<W*H;i++)chZ[i]=CH_ZS;
 ch_character(&actor);assert(!delta(0,0,W,H)); /* nearer occluder wins */
 /* Become no NPC agent to render beneath a roof or alter the local anchor. */
 nv.garage[60]=0;paint();assert(!chProjected);snap("street-overhead");
 int fx=sx(actor.x),fy=sy(actor.y);
 assert((U)fx<W&&(U)fy<H);
 U groundBefore=0x7c8990;rect(0,0,W,H,groundBefore);cp(untouched,pixels,sizeof pixels);
 street_person_shadow((int)actor.x,(int)actor.y);
 street_person_draw(&actor);
 assert(delta(fx-16,fy-20,32,15)>10&&delta(fx-8,fy+2,16,6)>8);
 assert(!delta(fx-16,fy-28,32,5)); /* natural overhead rig, not magnification */
 peds[5].hp=0;peds[5].life=3.f;paint();snap("street-downed");
 assert(!street_ped_pose(5,&actor));
 /* Old hardcoded nighttime FX printed four people here even with zero peds.
    This exact coat was drawn by their art and is absent in a zero-actor street. */
 z(peds,sizeof peds);sv.dayClock=60.f;p.car=-1;p.x=1480;p.y=600;by=-1;sv.owned=0;anim=1.25f;
 paint();int coatCount=0;
 for(int y=max(0,sy(482));y<min(H,sy(554));y++)for(int x=max(0,sx(1485));x<min(W,sx(1564));x++)
  coatCount+=pixels[y*W+x]==0x53898e||pixels[y*W+x]==0x8b6381;
 assert(coatCount==0);snap("street-midnight-no-phantoms");
 puts("PASS: anchored upright chase actor, moving legs, depth occlusion, separate overhead contact shadow, same foot scale, readable downed state, removed render-only night crowds.");
}
