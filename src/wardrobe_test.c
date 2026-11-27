/* Actual game/editor tick + paint, and mock disk at the host contract. */
#define _WIN_NATIVE
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned char files[4][sizeof(Save11)];
static unsigned size[4];
static int diskSlot,broken;
void disk_slot(int n){diskSlot=n;}
int disk_present(void){return diskSlot<4&&size[diskSlot]>0;}
int disk_read(void*v,unsigned n){
 if(!disk_present()||size[diskSlot]!=n)return 0;
 memcpy(v,files[diskSlot],n);return 1;
}
int disk_write(const void*v,unsigned n){
 assert(diskSlot<4&&(n==sizeof(Save)||n==sizeof(Save11)));
 if(broken)return 0;
 memcpy(files[diskSlot],v,n);size[diskSlot]=n;return 1;
}
int disk_create(const void*v,unsigned n){return !disk_present()&&disk_write(v,n);}
int disk_delete(void){if(!disk_present())return 0;size[diskSlot]=0;return 1;}
static const char* shots;
static void press(U key){tick(key);tick(0);}
static void shot(const char*name){
 if(!shots)return;
 char path[512];assert(snprintf(path,sizeof path,"%s/%s.ppm",shots,name)<(int)sizeof path);
 paint();FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U v=pixels[i];fputc(v>>16,f);fputc(v>>8,f);fputc(v,f);}assert(!fclose(f));
}
static U region(int x,int y,int w,int h){
 U hsh=2166136261u;for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)hsh=(hsh^pixels[j*W+i])*16777619u;return hsh;
}
static void open_editor(void){
 press(ESC);assert(menu==PAUSE_MENU);press(UP);assert(opt[sel].action==CW_PAUSE_OPTION);press(ENTER);
 assert(scene==OPEN_CITY_WARDROBE_SCENE&&wardPage==0&&!createWriting&&menu<0);
 press(WHEEL);assert(wardPage&&createField==CW_HAT&&!wheelOpen);
}
static U outfit(void){
 U u=ward_set(0,CW_HAT,4);u=ward_set(u,CW_HAT_TONE,2);u=ward_set(u,CW_TRIM,5);
 u=ward_set(u,CW_ACCESSORY,4);u=ward_set(u,CW_HAIR,1);u=ward_set(u,CW_FACE,3);
 u=ward_set(u,CW_OUTERWEAR,1);u=ward_set(u,CW_FOOT_TONE,2);return u;
}
int main(int argc,char**argv){
 init_city();
 if(argc==3&&!strcmp(argv[1],"--fixture")){
  starter_save();sv.intro=4;sv.cash=2387;sv.dirty=315;sv.bank=815;sv.hp=93;sv.x=1024.f;sv.y=1024.f;sv.playedTicks=26402;
  memset(sv.name,0,sizeof sv.name);strcpy(sv.name,"Wardrobe Test");sv.hash=savehash();
  FILE*f=fopen(argv[2],"wb");assert(f&&fwrite(&sv,sizeof sv,1,f)==1&&!fclose(f));return 0;
 }
 shots=argc==2?argv[1]:NULL;
 press(ENTER);assert(scene==5&&!wardPage&&!draftWard);
 press(WHEEL);press(RIGHT);assert(ward_hat(draftWard)==1);
 press(DOWN);press(RIGHT);press(RIGHT);assert(ward_hat_color(draftWard)==wardDyes[2]);
 press(DOWN);press(RIGHT);press(DOWN);press(RIGHT);assert(ward_part(draftWard,CW_ACCESSORY)==1);
 /* Through paint, every hat's aggregate of four turn angles is distinct. */
 U hats[10],full=draftWard;
 for(int h=0;h<10;h++){
  U k=2166136261u;draftWard=ward_set(outfit(),CW_HAT,h);
  for(int a=0;a<4;a++){paint();k=(k^region(693,159,190,180))*16777619u;press(MAP);}
  for(int j=0;j<h;j++)assert(hats[j]!=k);
  hats[h]=k;
 }
 for(int field=1;field<CW_N;field++){
  draftWard=ward_set(outfit(),field,0);paint();U a=region(693,159,190,180);
  draftWard=ward_set(outfit(),field,ward_part(outfit(),field));paint();
  assert(region(693,159,190,180)!=a);
 }
 draftWard=outfit();shot("01-style-lab");press(MAP);shot("02-style-side");
 draftWard=full;press(ENTER);
 assert(scene==3&&sideSlot==0&&ward_live()==full&&!sv.cash&&size[0]==sizeof(Save11));
 assert(((Save11*)files[0])->hash==night_hash(files[0],sizeof(Save11)-4));
 init_city();assert(cardCount==1&&cards[0].style==full);shot("03-career-file");
 press(ENTER);assert(scene==3&&ward_live()==full);
 /* Put a real legacy-format career in slot zero and restore it. */
 starter_save();sv.intro=4;sv.cash=2387;sv.dirty=315;sv.bank=815;sv.hp=93;sv.x=1024.f;sv.y=1024.f;sv.hash=savehash();disk_slot(0);
 assert(disk_write(&sv,sizeof sv));init_city();assert(cardCount==1&&!cards[0].style);
 press(ENTER);assert(scene==1&&!ward_live()&&sv.cash==2387);
 nv.world[CW_WORD]=0x80000003u;nv.phone[53]=47; ; /* test RMW against unrelated bits */
 int cash=sv.cash,dirty=sv.dirty,bank=sv.bank,hp=p.hp;float x=p.x,y=p.y;
 int day=(int)sv.dayClock;
 open_editor();press(RIGHT);press(ESC);assert(scene==1&&!ward_live()&&size[0]==sizeof(Save));
 open_editor();press(RIGHT);broken=1;press(ENTER);assert(scene==OPEN_CITY_WARDROBE_SCENE&&profileError&&!ward_live()&&nv.world[CW_WORD]==0x80000003u);
 broken=0;draftWard=outfit();press(ENTER);
 assert(scene==1&&ward_live()==outfit()&&(nv.world[CW_WORD]&~CW_SAVE_MASK)==0x80000003u&&nv.phone[53]==47);
 assert(sv.cash==cash&&sv.dirty==dirty&&sv.bank==bank&&p.hp==hp&&p.x==x&&p.y==y&&(int)sv.dayClock==day);
 assert(size[0]==sizeof(Save11));
 disk_slot(0);load_save();assert(wason&&ward_live()==outfit()&&nv.phone[53]==47&&(nv.world[CW_WORD]&~CW_SAVE_MASK)==0x80000003u);
 assert(sv.cash==cash&&sv.dirty==dirty&&sv.bank==bank&&sv.hp==hp);
 assert(ward_hat(0xfu)==0&&ward_part(15u<<4,CW_HAT_TONE)==0&&ward_part(7u<<19,CW_OUTERWEAR)==0);
 /* Ground truth through both native draw pipelines, not a costume catalog mock. */
 scene=1;menu=-1;p.car=-1;p.a=1.570796f;fastGfx=0;
 ward_store(0);paint();U top=region(350,110,260,270);ward_store(outfit());paint();
 assert(region(350,110,260,270)!=top);shot("04-overhead");
 nv.garage[60]=1;ch_reset_spring();press(0);assert(ch_mode_active());
 ward_store(0);paint();U view=region(350,145,260,260);ward_store(outfit());paint();assert(chProjected);
 assert(region(350,145,260,260)!=view);shot("05-street-3d");
 open_interior(12);p.car=-1;
 ward_store(0);paint();view=region(330,130,290,310);ward_store(outfit());paint();
 assert(region(330,130,290,310)!=view);shot("06-apartment-3d");
 puts("Wardrobe: 10 distinct hats (all four angles checked), each of seven independent style controls, actual 2D/3D outfits, styled and legacy save/load, career preservation, failed-write rollback and no cross-slot writes.");
}
