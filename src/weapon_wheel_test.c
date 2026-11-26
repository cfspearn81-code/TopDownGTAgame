/* Headless native engine tests with actual save serialization on a stub disk. */
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Save saved;static int wrote;
int disk_read(void*p,unsigned n){if(!wrote||n!=sizeof saved)return 0;cp(p,&saved,n);return 1;}
int disk_write(const void*p,unsigned n){assert(n==sizeof saved);cp(&saved,p,n);wrote++;return 1;}
static void photo(const char *path){
 paint();if(!path)return;
 FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U k=pixels[i];fputc(k>>16,f);fputc(k>>8,f);fputc(k,f);}fclose(f);
}
int main(int argc,char**argv){
 init_city();scene=1;sv.intro=4;sv.cash=250;sv.dirty=50;sv.bank=83;
 open_interior(2);active(2);makeMenu();
 int before=wrote;doOpt(22);doOpt(22);assert(saved.rev==11&&saved.medPatches==2&&saved.cash==200&&saved.hp==100&&wrote==before+2);
 load_save();assert(wason&&sv.medPatches==2&&sv.rev==11&&sv.cash==200&&sv.dirty==50&&sv.bank==83);
 p.hp=32;doOpt(22);assert(p.hp==82&&sv.medPatches==2&&sv.cash==175); /* Wounded buys still heal directly. */
 p.hp=100;sv.medPatches=99;before=wrote;doOpt(22);assert(sv.cash==175&&sv.medPatches==99&&wrote==before);
 sv.medPatches=2;open_interior(8);active(8);doOpt(24);assert(sv.cash==175&&sv.medPatches==2);
 p.hp=25;doOpt(24);assert(p.hp==100&&sv.cash==133&&sv.medPatches==2);
 /* Stable clocks, no backing panel navigation, and V gated by wheel. */
 tick(WHEEL|USE_PATCH);assert(!wheelOpen&&sv.medPatches==2);tick(0);menu=-1;
 p.hp=35;tick(USE_PATCH);assert(sv.medPatches==2&&p.hp==35);tick(0);
 scene=1;mapOn=0;pointer_input(WW_CX,WW_CY-WW_DIST,0);tick(WHEEL);assert(wheelOpen&&wheelPreviewFists);
 U steps=sv.playedTicks;float day=sv.dayClock;
 for(int i=0;i<11;i++)tick(WHEEL|LEFT|MAP|FIRE);
 assert(wheelOpen&&!mapOn&&sv.playedTicks==steps&&sv.dayClock==day&&p.trail<=0.f);
 tick(WHEEL|USE_PATCH);for(int i=0;i<120;i++)tick(WHEEL|USE_PATCH);
 assert(sv.medPatches==1&&p.hp==85&&saved.hp==85&&saved.medPatches==1&&saved.rev==11);
 tick(WHEEL);p.hp=100;tick(WHEEL|USE_PATCH);assert(sv.medPatches==1&&wheelFeedback==WW_FULL_HP);
 if(argc>1)photo(argv[1]);
 tick(0);assert(!wheelOpen&&!pistol_equipped());
 /* Owned pistol top, fists bottom, and an invalid pointer holds the hover. */
 active(14);doOpt(25);menu=-1;pointer_input(WW_CX+32,WW_CY+110,0);tick(WHEEL);assert(wheelPreviewFists&&pistol_equipped());
 pointer_input(-1,-1,0);tick(WHEEL|FIRE);assert(wheelPreviewFists&&p.trail<=0.f);
 tick(FIRE);for(int i=0;i<5;i++)tick(FIRE);
 assert(!wheelOpen&&!pistol_equipped()&&p.trail<=0.f&&p.shot<=0.f);
 tick(0);tick(FIRE);assert(p.shot>0&&p.trail<=0.f);tick(0);p.shot=0;
 pointer_input(WW_CX-40,WW_CY-52,0);tick(WHEEL);assert(!wheelPreviewFists&&!pistol_equipped());
 clear_input();tick(0);assert(!wheelOpen&&!pistol_equipped()); /* Canceled hover does not equip. */
 tick(WHEEL);assert(!wheelPreviewFists);
 int count=sv.medPatches;
 pointer_input(WW_MED_X,WW_MED_Y,0);tick(WHEEL);
 assert(wheel_med_hover()&&!wheelPreviewFists&&sv.medPatches==count);
 pointer_input(WW_MED_CARD_X+100,WW_MED_CARD_Y+24,1);tick(WHEEL);
 assert(wheel_med_hover()&&!wheelPreviewFists&&sv.medPatches==count);
 pointer_input(WW_CX-40,WW_CY-52,0);
 if(argc>2)photo(argv[2]);else paint();
 assert(textw("V USE ONE   HP 100/100",0)<WW_MED_CARD_W-28);
 assert(d2(WW_MED_X-WW_CX,WW_MED_Y-WW_CY)>(WW_OUT+WW_MED_R)*(WW_OUT+WW_MED_R));
 assert(pixels[WW_MED_Y*W+WW_MED_X+WW_MED_R]==0x85a795&&pixels[WW_MED_Y*W+WW_MED_X]==PAPER);
 assert(pixels[(WW_CY+141)*W+WW_CX-141]==0x62757e&&pixels[(WW_CY+141)*W+WW_CX+141]==0x62757e);
 tick(0);assert(pistol_equipped());
 pointer_input(WW_MED_X,WW_MED_Y,0);tick(WHEEL);assert(!wheelPreviewFists);tick(0);
 assert(pistol_equipped());
 tick(FIRE);assert(p.trail>0.f);tick(0);p.shot=0;
 p.hp=15;tick(WHEEL|USE_PATCH);assert(p.hp==65&&sv.medPatches==0&&saved.rev==10);
 tick(WHEEL);tick(WHEEL|USE_PATCH);assert(wheelFeedback==WW_NONE_LEFT&&p.hp==65&&sv.medPatches==0);tick(0);
 load_save();assert(wason&&sv.rev==10&&sv.hp==65&&sv.gear==0&&!sv.medPatches);
 Save valid=saved;
 for(int k=0;k<3;k++){
  sv=valid;sv.rev=k==0?10:11;sv.medPatches=k==0?1:k==1?100:0;sv.hash=savehash();saved=sv;
  load_save();assert(!wason&&!sv.medPatches); /* Reject noncanonical or overflow values with valid hashes. */
 }
 saved=valid;load_save();assert(wason&&sizeof(Save)==304);
 puts("Wheel controls, focus cancel, old/new saves, purchases, two weapons, separate accessory hover/geometry, patch cap and no repeat/full-HP spend passed.");
}
