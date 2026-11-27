/* Use rendered pixels: before purchase the rack really is locked, afterward
 * the new sign must look different. Its highlight must agree with actual E reach. */
#define DC_TEST
#include "game.c"
#include <stdio.h>
#include <assert.h>
static void key(U m){tick(0);tick(m);tick(0);}
static U signhash(void){paint();U v=2166136261u;for(int y=241;y<263;y++)for(int x=122;x<303;x++)v=(v^pixels[y*W+x])*16777619u;return v;}
int main(int argc,char**argv){
 int observe=argc>1;init_city();profiles_activate();sv.intro=4;sv.cash=1200;p.car=-1;scene=1;p.x=NT_DOOR_X;p.y=NT_DOOR_Y;
 key(ENTER);assert(nt_pane==NT_VISIT);key(ESC);assert(nt_pane==-1);U l=signhash();
 nt_x=nt_sz[NT_DESK_DOOR];nt_y=nt_sy[NT_DESK_DOOR];key(USE);key(ENTER);key(ENTER);assert(night_trap_owned()&&sv.cash==720);key(ESC);
 nt_x=480;nt_y=412;U o=signhash();nt_x=nt_sz[0];nt_y=nt_sy[0]+53;assert(nt_nearest()<0);U m=signhash();key(USE);assert(nt_pane==-1);
 nt_y=nt_sy[0]+50;assert(nt_nearest()==0);U k=signhash();key(USE);assert(nt_pane==NT_STOCK);
 printf("Bell Lane sign hashes: locked %08x; owned %08x; 53px(out of reach) %08x; 50px(in reach) %08x.\n",l,o,m,k);
 if(!observe){assert(l!=o&&o==m&&m!=k);}return 0;
}
