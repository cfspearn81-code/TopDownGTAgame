/* Chase renderer, stock fleet and field-cast regressions. Optional capture
 * directory writes unretouched frames of the actual software raster target.
 * Uses synthetic in-process data, never the player's save. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <string.h>
static char *dir;
static U hashframe(void){U h=2166136261u;for(int i=0;i<W*H;i++)h=(h^pixels[i])*16777619u;return h;}
static int color(U c){int n=0;for(int i=0;i<W*H;i++)n+=pixels[i]==c;return n;}
static void savepic(const char*s){if(!dir)return;char path[800];snprintf(path,sizeof path,"%s/%s.ppm",dir,s);
 FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);for(int j=0;j<W*H;j++){
  U a=pixels[j];fputc(a>>16,f);fputc(a>>8,f);fputc(a,f);}assert(!fclose(f));}
static void stage(float x,float y,float distance,float height,float bearing){
 ch.co=cs(bearing);ch.si=sn(bearing);ch.x=x-distance*ch.co;ch.y=y-distance*ch.si;ch.h=height;
 float pitch=(height-6)/distance;chCos=1.f/sq(1+pitch*pitch);chSin=pitch*chCos;chDark=.09f;chFog=0;
 for(int i=0;i<W*H;i++){pixels[i]=0x324451;chZ[i]=0;}
}
static void fleet(void){
 Car c={1024,1024,0,0,0,0,100};
 U per[5];for(int i=0;i<5;i++)for(int a=0;a<12;a++){
  c.type=i;stage(c.x,c.y,63,24,a*6.2831853f/12);ch_car(&c,1);U h=hashframe();
  assert(color(0x324451)<W*H-1400);
  if(!a){per[i]=h;for(int j=0;j<i;j++)assert(per[j]!=h);assert(color(0xc84b44)>18);}
  if(a==1||a==5){char s[64];snprintf(s,sizeof s,"fleet-%d-%d",i,a);savepic(s);}
 }
 cars[0]=c;p.car=0;stage(c.x,c.y,63,24,0);playerBrake=1;ch_car(cars,1);assert(color(0xff5148)>18);
 cars[0].s=-10;ch_car(cars,1);assert(color(0xe2faff)>3);
 cars[0].cond=0;stage(c.x,c.y,63,24,0);ch_car(cars,1);assert(!color(0xe2faff)&&!color(0xff5148));
 for(int lod=0;lod<3;lod++){stage(c.x,c.y,85+lod*160,34,2.4f);Car v=c;ch_car(&v,0);U h=hashframe();
  stage(c.x,c.y,85+lod*160,34,2.4f);ch_car(&v,0);assert(hashframe()==h&&!memcmp(&v,&c,sizeof v));}
}
static U paintHero(float a){stage(p.x,p.y,44,19,a);ch_player_character();return hashframe();}
static void cast(void){
 p.car=-1;p.x=p.y=1024;p.a=0;p.dead=0;scene=1;sv.hp=p.hp=100;sv.look=defaultLook;sv.gear=-1;punchFlash=p.trail=0;
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.hp=0,.life=0};npcEnc.phase=NPC_IDLE;
 U idle=paintHero(.4f);assert(color(0x324451)<W*H-500);
 for(int i=0;i<CHAR_FIELDS;i++){sv.look=defaultLook;
  sv.look.part[i]=i==LEGS?2:i==SKIN?6:i==DETAIL?5:i==BUILD?2:i==HAIR?5:wardrobe[i].count-1;
  U a=paintHero(2.7f);sv.look=defaultLook;assert(a!=paintHero(2.7f));}
 walking=1;footcycle=1.44f;assert(paintHero(.4f)!=idle);walking=0;
 punchFlash=.21f;assert(paintHero(.4f)!=idle);punchFlash=0;
 sv.gear=0;wheelFists=0;p.shot=0;U armed=paintHero(.4f);assert(armed!=idle);savepic("cast-armed");
 pulseGuardView=1;pulseGuardTicks=PULSE_CLEAR_TICKS;assert(paintHero(.4f)!=armed);clear_input();pulse_guard_reset();
 shoot();assert(p.trail>0);paintHero(.4f);ch_street_encounter();assert(color(0xfff2bd)>5);savepic("cast-shot");p.trail=0;
 npcEnc.actor=(NpcRef){.id=0,.model=MODEL_RIVAL,.hue=4,.x=p.x+12,.y=p.y,.a=2.7f,.coat=0xb46791};npcEnc.room=1;
 for(int i=NPC_DRAW;i<=NPC_DROP;i++){npcEnc.phase=i;npcEnc.prize=66;npcEnc.strike=.17f;paintHero(.4f);ch_street_encounter();
  assert(color(i==NPC_FIGHT?PINK:i==NPC_YIELD?LIME:GOLD)>0);if(i==NPC_DROP){savepic("cast-collect");npcEnc.prize=0;paintHero(.4f);ch_street_encounter();assert(!color(GOLD));}}
}
int main(int argc,char**argv){if(argc>1)dir=argv[1];init_city();fleet();cast();puts("Chase cast: five chassis x 12 headings, stop/reverse/damage, distance tiers, wardrobe, stride, cover, fists, pulse, all street confrontation states and loot verified.");}
