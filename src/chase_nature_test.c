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
static void materials(void){
 U safe=0xabcdef;
 for(unsigned y=35;y<ROOM-35;y+=41){
  unsigned x=ROOM-1;
  if(outerLand[y*ROOM+x]!=TILE_TIDE||outerLane[y*ROOM+x])continue;
  U a=wl_terrain_sample(0x1c3546,x,y,140,22);
  U b=wl_terrain_sample(0x203540,x+1,y,140,22);
  assert(absf((int)(a>>16)-(int)(b>>16))<6);
  assert(absf((int)((a>>8)&255)-(int)((b>>8)&255))<6);
  assert(absf((int)(a&255)-(int)(b&255))<6);
 }
 int channel=0,channelChanged=0,land=0,harborChanged=0;
 for(unsigned y=2026;y<3080;y+=3)for(unsigned x=2004;x<ROOM;x+=7){
  U orig=y<ROOM?city[y*ROOM+x]:safe;
  U color=wl_terrain_sample(orig,x,y,90,40);
  if(y<ROOM&&outerLane[y*ROOM+x]){channel++;channelChanged+=color!=orig;}
  if(y<ROOM&&(outerLand[y*ROOM+x]&127)==TILE_HARBOR){land++;harborChanged+=color!=orig;}
 }
 /* Working pier a few vertical decorations are deliberately removed.
  * The overwhelming majority of the quay paving must stay identical. */
 /* An overhead-only stop billboard used to spill onto drivable ground. */
 assert(channel>100&&channelChanged<channel/25);
 assert(land>100&&harborChanged>0&&harborChanged<land/5);
}
static void floor_separation(void){
 assert(city==cityAtlas&&!bakeFloorOnly&&surface==pixels);
 int crowns=0,treeShadows=0,woodStems=0,crates=0,lamps=0,pads=0;
 for(int i=0;i<LEN(scenery);i++)for(int j=scenery[i].treesFrom;j<scenery[i].count;j++){
  StreetBounds t=scenery[i].walls[j];
  int at=(t.y-15)*ROOM+t.x+3;
  if(city[at]!=chaseFloor[at])crowns++;
  at=(t.y+7)*ROOM+t.x+16;
  if(city[at]==0x172823&&chaseFloor[at]==city[at])treeShadows++;
 }
 for(int i=0;i<woodPropN;i++)if(woodProps[i].kind==WOOD_TREE){
  const WoodProp *t=&woodProps[i];int at=t->y*ROOM+t->x;
  if(city[at]==0x694e3a&&chaseFloor[at]!=city[at])woodStems++;
 }
 for(int i=0;i<LEN(harborPieces);i++)if(harborPiecePlaced[i]){
  const HarborPiece*s=&harborPieces[i];
  int at=(s->y+s->h/4)*ROOM+s->x+s->w/2;
  if(city[at]!=chaseFloor[at])crates++;
  assert((outerLand[at]&TILE_BLOCK)&&roofMask[at]); /* Only appearance differs. */
 }
 for(int y=1;y<8;y++)for(int x=1;x<8;x++){
  StreetLamp s;if(!street_lamp_legacy(x,y,0,&s))continue;
  int at=(s.y+s.lensY+1)*ROOM+s.x+s.lensX+1;
  if(city[at]==s.lens&&chaseFloor[at]!=s.lens)lamps++;
 }
 for(int i=0;i<LEN(loc);i++){
  const Building*b=buildings+zone_idx[i];
  int at=(b->y-b->ht+15)*ROOM+b->x+b->w/2;
  if(city[at]!=chaseFloor[at])pads++;
 }
 assert(crowns>40&&treeShadows>40&&woodStems>40&&crates>=4&&lamps>35&&pads>12);
 /* Genuine painted loading stripes and sign/boarding footprints remain. */
 int at=2155*ROOM+2850;
 assert(chaseFloor[at]==city[at]); /* Stripe ink may inherit the adjacent lamp pool. */
 at=2206*ROOM+2948;
 assert(chaseFloor[at]==city[at]); /* Keep the shared dock marker, including lamp pool tint. */
 printf("Separated %d crowns, %d roots, %d harbor structures, %d lamp sprites, %d named-building stamps; preserved %d sampled tree shadows and physical quay paint.\n",
   crowns,woodStems,crates,lamps,pads,treeShadows);
}
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
 U safe=0xabcdef;assert(wl_ocean_edge(safe,0xffffffffu,2391,0,200)==safe);
 assert(wl_ocean_edge(safe,150,ROOM,0,200)!=safe);assert(wl_ocean_edge(safe,ROOM+1,2600,0,200)!=safe);
 assert(wl_ocean_edge(safe,ROOM-1,2600,0,200)!=safe);
 floor_separation();materials();
 key(GFX);assert(!ch_mode_active()&&ch_pref());key(GFX);assert(ch_mode_active());
 puts("PASS chase nature: forest landmarks, quest state/gathering, wildlife, shore and ocean edge, repeatable paint; C, E, M, Escape, F2.");
 return 0;
}
