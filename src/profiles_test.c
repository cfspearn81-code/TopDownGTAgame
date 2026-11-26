/* Integration checks for catalog, creator, time tracking and migration, using a strict mock disk. */
#define _WIN_NATIVE
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned char saves[PROFILE_SLOTS][sizeof(Save)];
static unsigned sizes[PROFILE_SLOTS];
static int current,failCreate,failWrite,writes,failDelete,deletes;
void disk_slot(int n){assert(n>=0&&n<PROFILE_SLOTS);current=n;}
int disk_present(void){return sizes[current]>0;}
int disk_read(void*d,unsigned n){if(sizes[current]!=n)return 0;memcpy(d,saves[current],n);return 1;}
int disk_write(const void*d,unsigned n){if(failWrite)return 0;assert(n==sizeof(Save));memcpy(saves[current],d,n);sizes[current]=n;writes++;return 1;}
int disk_create(const void*d,unsigned n){if(failCreate||sizes[current])return 0;return disk_write(d,n);}
int disk_delete(void){if(failDelete||!sizes[current])return 0;sizes[current]=0;deletes++;return 1;}
static void press(U k){tick(k);tick(0);}
static void click(int x,int y){pointer_input(x,y,1);}
static void frame(const char*name){paint();FILE *f=fopen(name,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);for(int j=0;j<W*H;j++){U v=pixels[j];fputc(v>>16,f);fputc(v>>8,f);fputc(v,f);}fclose(f);}
int main(void){
 init_city();assert(scene==0&&!cardCount&&!wason&&!writes);
 press(ENTER);assert(scene==5&&!writes);press(ESC);assert(scene==0&&!writes);
 click(793,416);assert(scene==5&&!writes);
 click(580,151);assert(createWriting);for(int i=0;i<19;i++)text_input(8);
 for(const char*s="Riley Knox";*s;s++)text_input((U)*s);
 press(ENTER);assert(!createWriting&&scene==5&&!writes);
 click(550,151);text_input('X');press(ESC);assert(!strcmp(draftName,"Riley Knox"));
 unsigned hashes[LOOKBOOK_COUNT];
 for(int i=0;i<LOOKBOOK_COUNT;i++){
  press(GFX);assert(createPreset==i&&look_valid(&draft));rect(0,0,W,H,DARK);
  portrait(40,40,210,194,&draft,5,0);
  U h=2166136261u;for(int y=40;y<234;y++)for(int x=40;x<250;x++)h=(h^pixels[y*W+x])*16777619u;
  for(int j=0;j<i;j++)assert(h!=hashes[j]);hashes[i]=h;
 }
 press(GFX);assert(createPreset==0);click(703,417);assert(createPreset==LOOKBOOK_COUNT-1);
 click(579,138+28*(HAIR+1)+9);assert(createPreset==-1&&draft.part[HAIR]==0);
 click(335,138+28*(HAIR+1)+9);assert(draft.part[HAIR]==9);
 press(GFX);for(int i=0;i<34;i++)press(GFX);
 frame("build/character-creator.ppm");
 Look goal=draft;
 int torsoRows=0;U jacket=charTop[draft.part[TOP_TONE]];
 for(int y=154;y<346;y++){
  int n=0;for(int x=766;x<=816;x++)n+=pixels[y*W+x]==jacket;
  if(n>=12)torsoRows++;
 }
 assert(torsoRows>=20);

 failCreate=1;click(785,476);assert(scene==5&&!writes&&profileError&&sv.rev==7&&!memcmp(&draft,&goal,sizeof goal));
 failCreate=0;click(785,476);assert(scene==3&&interior_active_loc==12&&sideSlot==0&&writes==1);
 assert(!memcmp(&sv.look,&goal,sizeof goal)&&!strcmp(sv.name,"Riley Knox")&&sv.hp==100&&!sv.cash&&sv.gear==-1);
 for(int i=0;i<3600;i++)tick(0);
 assert(sv.playedTicks==3600);
 press(ESC);U time=sv.playedTicks;for(int i=0;i<120;i++)tick(0);assert(sv.playedTicks==time);
 sv.cash=4320;sv.dirty=1550;press(SLOT);assert(scene==0&&cardCount==1&&cards[0].ticks==time&&cards[0].cash==4320&&cards[0].dirty==1550);
 assert(!memcmp(&cards[0].look,&goal,sizeof goal));frame("build/career-cards.ppm");
 press(ESC);assert(scene==3&&menu==12);
 failWrite=1;press(NEW_SLOT);assert(scene==0&&profileError&&writes==2);press(NEW_SLOT);assert(scene==0);press(ESC);assert(scene==3);failWrite=0;
 press(NEW_SLOT);assert(scene==5);press(ESC);press(ESC);assert(scene==3&&sideSlot==0);
 press(NEW_SLOT);press(ENTER);assert(scene==3&&sideSlot==2&&!sv.cash&&!sv.dirty);
 init_city();assert(scene==0&&cardCount==2);cardSelected=0;press(ENTER);assert(sideSlot==0&&sv.cash==4320&&sv.dirty==1550&&sv.playedTicks>=time);
 assert(!memcmp(&sv.look,&goal,sizeof goal)&&!strcmp(sv.name,"Riley Knox"));
 /* Rev6 uses the same economy bytes, with the hash in the new clock position. */
 sv.rev=6;sv.playedTicks=hashbytes(LEGACY6_SIZE-4);memcpy(saves[1],&sv,LEGACY6_SIZE);sizes[1]=LEGACY6_SIZE;
 disk_slot(1);load_save();assert(wason&&sv.rev==7&&sv.cash==4320&&sv.legacyClock&&!sv.playedTicks&&look_valid(&sv.look));
 /* Corrupt file stays selectable as unreadable and is not overwritten. */
 for(int i=3;i<8;i++){memcpy(saves[i],saves[0],sizes[0]);sizes[i]=sizes[0];}
 saves[3][12]^=1;
 scene=0;profiles_scan();assert(cardCount==8&&!cards[3].status);
 cardSelected=0;click(627,466);assert(cardSelected==5&&profile_page()==1);
 click(89,466);assert(cardSelected==0);click(174,340);assert(cardSelected==3);
 press(ENTER);assert(scene==0&&profileError);
 press(NEW_SLOT);press(ENTER);assert(scene==3&&sideSlot==8&&saves[3][12]!=saves[0][12]);
 /* Delete only the confirmed slot, preserve others and the running career. */
 press(SLOT);assert(scene==0&&profileReturn==3);
 press(DELETE_SLOT);assert(!deleteOpen&&!deletes&&sizes[8]);
 cardSelected=2;click(786,460);assert(deleteOpen&&deleteCard.id==2&&!deleteArmed);
 frame("build/delete-save.ppm");
 click(150,155);assert(deleteOpen&&deleteCard.id==2);
 press(ENTER);assert(!deleteOpen&&!deletes&&sizes[2]);
 press(DELETE_SLOT);press(RIGHT);press(ESC);assert(!deleteOpen&&!deletes&&sizes[2]);
 failDelete=1;press(DELETE_SLOT);click(590,332);assert(!deleteOpen&&sizes[2]&&!deletes&&!profileSuccess&&current==8);
 failDelete=0;press(DELETE_SLOT);press(RIGHT|ENTER);assert(deleteOpen&&!deletes);
 press(ENTER);assert(!deleteOpen&&cardCount==8&&deletes==1&&!sizes[2]&&sizes[0]&&sizes[8]&&current==8);
 press(ESC);assert(scene==3&&sideSlot==8);
 init_city();assert(scene==0&&cardCount==8);cardSelected=0;
 press(DELETE_SLOT);assert(deleteOpen&&deleteCard.id==0);click(590,332);
 assert(deletes==2&&!sizes[0]&&sizes[8]&&sizes[3]);
 press(NEW_SLOT);press(ENTER);assert(sideSlot==0&&sizes[0]&&scene==3&&!sv.cash);
 /* Tall hair, north, full range: no arrow ink on the head. */
 sv.look.part[HAIR]=6;
 for(int i=0;i<128;i++){
  float a=i*6.283186f/128;rect(0,0,W,H,DARK);person(480,260,0,0,a,4,0);
  Pt h=rig(480,260,cs(a),sn(a),1,0,14);int whites=0;
  for(int y=211;y<=310;y++)for(int x=430;x<=530;x++)if(pixels[y*W+x]==0xffffff){assert(d2(x-h.x,y-h.y)>=121);whites++;}
  assert(whites>17);
 }
 puts("Passed: 360 distinct rasterized looks; keyboard and mouse creation, rename, cancel, disk failures, slot preservation, active playtime, colored cash, pagination, save deletion, upright portrait, Rev6 migration, and facing arrow at 128 angles.");
}
