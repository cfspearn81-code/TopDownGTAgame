/* Pixel-level footer regression; screenshots optional. No player saves. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void dump(const char *path){
 if(!path)return;FILE *o=fopen(path,"wb");assert(o);fprintf(o,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc(c>>16,o);fputc(c>>8,o);fputc(c,o);}fclose(o);
}
int main(int argc,char**argv){
 init_city();starter_save();profiles_activate();sv.intro=4;scene=1;mapOn=0;menu=-1;end_job();
 p.x=2464;p.y=1024;p.hp=100;p.car=-1;p.dead=0;vd_reset();sv.dayClock=1300;
 navReady=navHasManual=0;
 const int x=W-170-24,y=H-120-40;minimap();dump(argc>1?argv[1]:0);
 const char*area=district((int)p.x,(int)p.y);assert(!strcmp(area,"FREIGHT PARKWAY"));
 U actual[22][170];
 for(int j=0;j<22;j++)memcpy(actual[j],&pixels[(y+121+j)*W+x],sizeof actual[j]);
 rect(x,y+121,170,22,0x101522u);textat(x+4,y+121,area,MUTED,0);
 int wrong=0;for(int j=0;j<22;j++)for(int i=0;i<170;i++)wrong+=(actual[j][i]!=pixels[(y+121+j)*W+x+i]);
 printf("Footer %s: %d pixels conflicted with expected full district label\n",area,wrong);fflush(stdout);
 assert(!wrong);
 /* Keep all route-state labels inside the 170px map, including
  * after-hours road distances, no-road and walk-to-door suffixes. */
 char label[72];NavPoint here={(int)p.x,(int)p.y};
 for(int kind=1;kind<=8;kind++)for(int mode=0;mode<4;mode++){
  navGoalKind=kind;navCount=mode==2?0:2;navGoal=(NavPoint){1300,2250};
  const char*f=nav_compact_footer(label,here,mode?8421:2,mode==3,162);
  assert(textw(f,0)<=162);
 }
 int badge=0;for(int j=0;j<20;j++)for(int i=30;i<=65;i++)badge+=pixels[(y+4+j)*W+x+i]==PAPER;
 assert(badge>10);
 /* Full atlas still opens/closes with the same key after the HUD change. */
 tick(MAP);assert(mapOn);paint();tick(0);tick(MAP);assert(!mapOn);
}
