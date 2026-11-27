/* Character raster checks for all room variants, no external assets. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
static U film(void){U h=2166136261u;for(int i=0;i<W*H;i++)h=(h^pixels[i])*16777619u;return h;}
static void verify_room(float x,float y,float angle,int which){
 ir_begin(x,y,65,485,0);ir_hplane(100,65,840,450,0,0x34445d,0);
 U before=film();ir_hero(x,y,angle,which);
 U normal=film();assert(normal!=before&&!ch_actor_target);
 int aa=0,full=0,raised=0;IrPoint q=ir_proj(x,y,0);
 for(int sy=max(65,q.y-88);sy<min(485,q.y+12);sy++)for(int sx=max(0,q.x-40);sx<min(W,q.x+41);sx++){
  int n=0;if(sx<ic.l||sx>=ic.l+ic.w||sy<ic.t||sy>=ic.t+ic.h)continue;
  const IcSample*ptr=icBuf+4*((sy-ic.t)*ic.w+sx-ic.l);
  for(int k=0;k<4;k++)n+=ptr[k].z>0;
  aa+=n>0&&n<4;full+=n==4;raised+=chZ[sy*W+sx]>q.iz+220;
 }
 assert(aa>45&&full>250&&raised>250); /* actual geometry plus partly covered edge samples */
 ir_begin(x,y,65,485,0);ir_hplane(100,65,840,450,0,0x34445d,0);ir_hero(x,y,angle,which);assert(normal==film());
 ir_begin(x,y,65,485,0);ir_hplane(100,65,840,450,0,0x34445d,0);ir_hero(x,y,angle+1.383f,which);assert(normal!=film());
}
int main(void){
 init_city();scene=1;end_job();menu=-1;p.car=-1;sv.intro=4;nv.garage[60]=1;
 sv.look=(Look){{2,4,8,5,1,3,8,1,4,2}};
 U saved=night_hash(&sv,sizeof sv),extended=night_hash(&nv,sizeof nv);
 for(int i=0;i<5;i++){int indoors[]={3,4,7,10,11};verify_room(480,375,1.22f,indoors[i]);}
 assert(night_hash(&sv,sizeof sv)==saved&&night_hash(&nv,sizeof nv)==extended);
 /* Raised opaque fixtures must win over the depth-tested avatar. */
 ir_begin(480,370,65,485,0);ir_hplane(100,65,840,450,0,0x34445d,0);
 ir_box(456,374,48,35,0,41,0x775151,0);
 U blocker=film();ir_hero(480,370,0,3);
 assert(film()==blocker); /* fully covered by this fixture */
 IrPoint q=ir_proj(480,368,15);int n=0;
 for(int sy=q.y-8;sy<q.y+8;sy++)for(int sx=q.x-6;sx<q.x+6;sx++)n+=pixels[sy*W+sx]==0x775151;
 assert(n>10);
 puts("PASS 3-D indoor character AA, geometry/depth, deterministic color, 5 room adapters, facing, opaque furniture and no save mutation.");
}
