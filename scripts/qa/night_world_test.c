/* Native disk round-trip of the legacy and Afterhours career formats. */
#define _POSIX_C_SOURCE 200809L
#define DC_LINUX_NATIVE
#include "../../src/game.c"
#include "../../src/linux_save.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void check_weather(void){
 Car coast={0};
 for(int x=54;x<BASE_ROOM-54;x++){
  coast.x=(float)x;coast.y=(float)nw_seawall_y(x);
  assert(!street_hits_wall(&coast));
 }
 sv.dayClock=250.f;
 night_world_reset();
 assert(night_world_weather()==NIGHT_RAIN && night_world_is_night());
 for(int i=0;i<240;i++)night_world_tick(1.f/60);
 assert(night_world_brake()<.90f&&night_world_grip()<.90f&&night_world_foot()<.95f);
 assert(nwStorm>.8f&&nwWet>.7f);
 U rainy;
 cx=540;cy=620;paint_world(1);rainy=night_hash(pixels,sizeof pixels);
 sv.dayClock=800.f;night_world_reset();paint_world(1);
 assert(rainy!=night_hash(pixels,sizeof pixels));
 sv.dayClock=1300.f;night_world_reset();
 assert(night_world_weather()==NIGHT_FOG&&night_world_is_night());
 for(int i=0;i<310;i++)night_world_tick(1.f/60.f);
 assert(nwFog>.85f&&nwStorm==0);
 sv.charges=0;sv.dayClock=1240;
 night_rest_clock(2,1);
 assert(sv.dayClock==1360&&nv.world[0]==7200&&night_world_weather()==NIGHT_CLEAR);
 for(int i=0;i<7200;i++)night_world_tick(1.f/60.f);
 assert(night_world_weather()==NIGHT_FOG);
 puts("World: storm, fog, night/day rendering, wet driving/walking, safehouse clear-sky duration");
}
static void record_test(void){
 starter_save();p.x=loc[12].x;p.y=loc[12].y;p.hp=100;p.dead=0;scene=1;sv.intro=4;sv.cash=123;
 disk_slot(0);
 request_save();assert(saveFlash>0);
 Save v;assert(disk_read(&v,sizeof v)&&v.rev==10&&v.cash==123&&v.hash==night_hash(&v,sizeof v-4));
 assert(!disk_present()||!disk_create(&v,sizeof v));
 /* Nonzero last words exercise the exact boundaries of every reserved family. */
 nv.club[31]=130;nv.garage[63]=91;nv.phone[63]=14;nv.world[23]=31;
 request_save();
 char backup[512];snprintf(backup,sizeof backup,"%s/DeadChannel/.pre-afterhours-career.dat-r10-%08x.bak",getenv("XDG_DATA_HOME"),v.hash);
 FILE *bk=fopen(backup,"rb");Save original;
 assert(bk&&fread(&original,1,sizeof original,bk)==sizeof original&&!memcmp(&original,&v,sizeof v)&&!fclose(bk));
 Save11 v11;assert(disk_read(&v11,sizeof v11)&&v11.base.rev==11&&v11.base.cash==123&&v11.hash==night_hash(&v11,sizeof v11-4));
 assert(v11.night.club[31]==130&&v11.night.garage[63]==91&&v11.night.phone[63]==14&&v11.night.world[23]==31);
 starter_save();
 assert(!nv.club[31]&&!nv.garage[63]&&!nv.phone[63]&&!nv.world[23]);
 load_save();
 assert(wason&&sv.rev==10&&sv.cash==123&&nv.club[31]==130&&nv.garage[63]==91&&nv.phone[63]==14&&nv.world[23]==31);
 /* An empty catalog scan and a foreign bad checksum cannot clobber this data. */
 disk_slot(2);v11.base.cash++;assert(disk_create(&v11,sizeof v11));disk_slot(0);
 profiles_scan();assert(cardCount==2&&cards[0].status&&!cards[1].status&&nv.club[31]==130);
 assert(disk_read(&v11,sizeof v11)&&v11.base.cash==123);
 disk_slot(2);assert(disk_delete());disk_slot(0);
 assert(exit_save_ok());assert(disk_read(&v11,sizeof v11)&&v11.hash==night_hash(&v11,sizeof v11-4));
 printf("Saves: Rev10 (%zu bytes) unchanged, Rev11 (%zu bytes) round-trip; isolation and corrupt record handling\n",sizeof v,sizeof v11);
}
int main(int argc,char **argv){
 assert(argc==2&&setenv("XDG_DATA_HOME",argv[1],1)==0&&linux_disk_init()&&linux_disk_lock());
 init_city();check_weather();record_test();linux_disk_shutdown();
 return 0;
}
