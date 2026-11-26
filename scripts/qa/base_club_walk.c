/* No motion, animation, scene or career injection on the primary route.
   cc -O2 -std=c11 scripts/qa/base_club_walk.c src/linux_save.c -o /tmp/base-club
   /tmp/base-club "$(mktemp -d)" [optional screenshot directory]
   The final, independent old-address compatibility check positions its fixture
   at the historical door only after the primary fresh-career route completes. */
#define _POSIX_C_SOURCE 200809L
#define DC_LINUX_NATIVE
#include "../../src/game.c"
#include "../../src/linux_save.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void hit(U key){tick(key);tick(0);}
static void capture(const char*dir,const char*file){
 paint();if(!dir)return;
 char path[640];snprintf(path,sizeof path,"%s/%s.ppm",dir,file);
 FILE*f=fopen(path,"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc((c>>16)&255,f);fputc((c>>8)&255,f);fputc(c&255,f);}
 fclose(f);
}
int main(int argc,char**argv){
 assert(argc>=2&&setenv("XDG_DATA_HOME",argv[1],1)==0&&linux_disk_init()&&linux_disk_lock());
 const char*out=argc>2?argv[2]:NULL;
 init_city();assert(scene==0&&cardCount==0);
 int b=night_club_listing(),wx=ordinaryDoorX[b],wy=ordinaryDoorY[b];
 assert(b>NC_ID&&wx==179&&wy==676&&loc[12].x-wx==161&&loc[12].y==wy);
 hit(ENTER);assert(scene==5);hit(ENTER);
 assert(scene==3&&interior_active_loc==12&&sv.intro==0&&sv.cash==0);
 int steps=0;while(!int_at_door()&&steps++<100)tick(DOWN);
 tick(0);assert(int_at_door());hit(USE);
 assert(scene==1&&sv.intro==1&&sv.cash==0&&p.x==340&&p.y==676);
 capture(out,"from-apartment");
 hit(MAP);assert(mapOn);
 int px=NAV_MAP_X+wx*NAV_MAP_SIZE/nav_span(),py=NAV_MAP_Y+wy*NAV_MAP_SIZE/nav_span();
 pointer_input(px,py,1);
 assert(navSelected==LEN(loc)+b&&navManualLoc==LEN(loc)+b&&!strcmp(nav_name(navSelected),"THE AFTERIMAGE STRIP CLUB"));
 capture(out,"base-club-map");pointer_input(px,py,2);hit(MAP);assert(!mapOn);
 hit(PHONE);hit(RIGHT);hit(RIGHT);assert(np_app==NP_WIRE&&np_row==0);
 hit(ENTER);assert(navHasManual&&navManualLoc==LEN(loc)+b&&navManual.x==wx&&navManual.y==wy);
 hit(ESC);assert(scene==1);
 int frames=0;while(p.x>wx+6&&frames++<160)tick(LEFT);
 tick(0);assert(absf(p.x-wx)<8&&p.y==wy&&frames<130);
 assert(street_use_target(by,near).id==b);
 capture(out,"door-prompt");hit(USE);
 assert(scene==NIGHT_CLUB_SCENE&&walkin_active==b&&sv.cash==0&&!nc_owned());
 capture(out,"base-club-interior");
 hit(USE);assert(scene==1&&p.x==wx&&p.y==wy);
 hit(SAVE);load_save();assert(wason&&sv.x==wx&&sv.y==wy&&sv.cash==0);
 /* Compatibility only: E still visits the authentic Shadow Stage foyer.
    Enter opens the same club; the original E stage activator is preserved. */
 p.x=ordinaryDoorX[NC_ID];p.y=ordinaryDoorY[NC_ID];clear_input();
 hit(USE);assert(scene==4&&walkin_active==NC_ID);
 hit(ENTER);assert(scene==NIGHT_CLUB_SCENE&&walkin_active==NC_ID);
 hit(USE);assert(scene==1&&p.x==ordinaryDoorX[NC_ID]&&p.y==ordinaryDoorY[NC_ID]);
 printf("PASS: fresh profile -> Apt 4 -> %d LEFT frames -> direct E; atlas and Signal at (%d,%d), matching exit/save, original Shadow Stage alias verified\n",frames,wx,wy);
 linux_disk_shutdown();
}
