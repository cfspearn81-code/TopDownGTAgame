/* Captures actual shared raster art in both cameras and exercises room input.
 * Standalone fixture: gcc -std=c11 -O1 scripts/qa/afterimage_art_test.c -o build/afterimage-art
 * Run build/afterimage-art [existing capture directory].
 * The upgrade showcase is seeded only after a real entry through the public door. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
static const char *out;
static void hit(U key){tick(key);tick(0);}
static U image(void){paint();return night_hash(pixels,sizeof pixels);}
static void cap(const char *name){
 paint();if(!out)return;
 char path[432];snprintf(path,sizeof path,"%s/%s.ppm",out,name);
 FILE *f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc(c>>16,f);fputc(c>>8,f);fputc(c,f);}
 assert(!fclose(f));
}
static void move(float x,float y){
 for(int n=0;n<500&&absf(ncX-x)>2.6f;n++)tick(ncX<x?RIGHT:LEFT);
 for(int n=0;n<500&&absf(ncY-y)>2.6f;n++)tick(ncY<y?DOWN:UP);
 tick(0);assert(absf(ncX-x)<4.f&&absf(ncY-y)<4.f);
}
static void pair(const char*a,const char*b){
 assert(!ch_mode_active());cap(a);U flat=image();
 hit(CAMERA);assert(ch_mode_active());cap(b);assert(image()!=flat);
 hit(CAMERA);assert(!ch_mode_active());
}
int main(int argc,char**argv){
 out=argc>1?argv[1]:0;init_city();scene=1;sv.intro=4;job.kind=-1;
 int b=night_club_listing();assert(b==ncFrontId&&b!=NC_ID);
 p.x=ordinaryDoorX[b];p.y=ordinaryDoorY[b];p.car=-1;sv.cash=900;sv.dayClock=22*60;
 clear_input();hit(USE);assert(scene==NIGHT_CLUB_SCENE&&ncRoom==NC_FLOOR&&walkin_active==b);
 pair("01-hall-overhead","02-hall-3d");
 hit(CAMERA);assert(ch_mode_active());
 hit(ESC);assert(menu==PAUSE_MENU);U still=image();
 for(int f=0;f<28;f++)tick(0);assert(image()==still);hit(ESC);assert(menu<0);
 hit(HELP);assert(scene==2);U frozen=image();for(int f=0;f<40;f++)tick(0);assert(image()==frozen);hit(ESC);
 assert(scene==NIGHT_CLUB_SCENE);hit(GFX);assert(fastGfx&&!ch_mode_active());
 cap("03-low-fx");hit(GFX);assert(!fastGfx&&ch_mode_active());
 hit(CAMERA);move(480,375);
 /* The owner upgrades decor and entertainment in the ordinary fixture. */
 nv.club[NC_OWN]=1;nv.club[NC_DECOR]=2;nv.club[NC_ACT]=2;nv.club[NC_DRINK]=4;
 pair("04-headline-overhead","05-headline-3d");
 /* The cast at this spot is the same person in the stage mesh and in
    Shift+E's physical-character picker, despite the service having E priority. */
 move(480,255);
 cap("13-stage-close-overhead");hit(CAMERA);
 for(int i=0;i<22;i++)tick(0);
 cap("14-stage-close-3d");
 hit(USE|BRAKE);assert(npcTalkOn&&npcTalk.id==NAR_STAGE+2);
 NpcRef held,seen;assert(npc_ref(npcTalk.id,&held));
 for(int i=0;i<26;i++)tick(0);
 assert(npc_holds(held.id)&&npc_ref(held.id,&seen)&&seen.x==held.x&&seen.y==held.y&&seen.a==held.a);
 hit(ESC);assert(!npcTalkOn);hit(CAMERA);assert(!ch_mode_active());
 move(480,375);
 move(168,361);hit(USE);assert(ncRoom==NC_OFFICE);
 pair("06-office-overhead","07-office-3d");
 move(480,416);hit(USE);assert(ncRoom==NC_FLOOR);
 move(808,361);hit(USE);assert(ncRoom==NC_SUITE);
 pair("08-lounge-overhead","09-lounge-3d");
 move(476,268);hit(USE);assert(ncPanel==NC_TABLE&&ncBooth==1);
 pointer_input(200,154+30*4,1);assert(ncPanel==NC_DANCE_CONFIRM);
 int wallet=sv.cash;pointer_input(200,184,1);assert(ncPanel==NC_DANCE_PLAY&&sv.cash==wallet-25);
 for(int n=0;n<92;n++)tick(0);cap("10-private-stage");hit(ESC);assert(ncPanel==NC_TABLE);
 /* Same exact camera/settings in a room this patch must not touch. */
 scene=1;p.car=-1;open_interior(12);assert(scene==3);
 pair("11-unrelated-overhead","12-unrelated-3d");
 puts("PASS: native game raster captures in both camera modes, real door and room actions, frozen 3D guide, Low FX fallback, actual VIP booking.");
}
