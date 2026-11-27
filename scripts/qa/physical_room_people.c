/* Play the real room inputs, optionally export them for replay by SDL.
   Only the initial known-position career is seeded; all doors, dialogue,
   movement, punches and pulse fire below are ordinary key events. */
#ifdef ROOM_REPLAY
#define DC_LINUX_NATIVE
#else
#define DC_TEST
#endif
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef ROOM_REPLAY
#include "../../src/linux_save.h"
#endif
static U tape[4600];static int used;
static void play(U k){assert(used<LEN(tape));tape[used++]=k;tick(k);}
static void press(U k){play(k);play(0);}
static void wait_frames(int n){while(n--){play(0);}}
static U photo(void){U h=2166136261u;paint();for(int i=0;i<W*H;i++)h=(h^pixels[i])*16777619u;return h;}
static void init(const char*room,int c,int armed){
 set_session_seed(17);init_city();starter_save();sv.intro=4;sv.cash=70;sv.dayClock=670;sv.gear=armed?0:-1;
 nv.garage[1]=1;nv.garage[60]=c;sv.hp=100;scene=1;p.car=-1;p.hp=100;
 int b=!strcmp(room,"aquarium")?74:ncFrontId;
 p.x=ordinaryDoorX[b];p.y=ordinaryDoorY[b];
 if(b==74)nv.world[NVS_STAGE]=5;
#ifdef ROOM_REPLAY
 assert(linux_disk_init()&&linux_disk_lock());disk_slot(0);request_save();assert(saveFlash>0);linux_disk_shutdown();
#endif
 sv.x=p.x;sv.y=p.y;profiles_activate();used=0;tape[used++]=0;tape[used++]=0;tape[used++]=ENTER;wait_frames(5);
}
static int announce(int id,int change){
 NpcRef pinned;assert(npc_ref(id,&pinned)&&!npc_replaces(id));
 press(USE|BRAKE);assert(npcTalkOn&&npcTalk.id==id);
 int dialogue=used;float x=npcTalk.x,y=npcTalk.y;U panel=photo();
 wait_frames(10);NpcRef parked;assert(npc_ref(id,&parked)&&parked.x==x&&parked.y==y);
 press(UP);press(ENTER);assert(!npcTalkOn&&npcLetters[id]==1&&strstr(notice,pinned.name));
 if(change)assert(photo()!=panel);
 return dialogue;
}
static void attack(int id,U key){
 assert(!npc_room_condition(id));int last=0;
 for(int i=0;i<3;i++){
  press(key);last=used;
  NpcRoomCondition*c=npc_room_condition(id);assert(c&&c->damage==i+1);
  if(i<2)wait_frames(25);
 }
 assert(npc_room_downed(id)&&sv.cash==70&&sv.dirty==0&&strstr(notice,"Knockdown"));
 printf("# down %d\n",last);
}
static void club(int c,int armed){
 init("club",c,armed);press(USE);assert(scene==NIGHT_CLUB_SCENE&&ncRoom==NC_FLOOR);
 int bodies=0;for(int id=NAR_STAGE;id<NAR_SUITE;id++){
  RoomCast q;NpcRef r;if(!npc_club_blueprint(id,&q))continue;
  assert(npc_ref(id,&r)&&r.x==q.x&&r.y==q.y&&r.name[0]);bodies++;
 }
 assert(bodies==21);
 for(int i=0;i<150&&ncX<568.f;i++)play(RIGHT);
 assert(ncX>=568.f&&ncX<571.f);
 for(int i=0;i<150&&ncY>398.f;i++)play(UP);
 assert(ncY<399.f&&ncY>394.f);
 wait_frames(1);int at=announce(NAR_DOOR,1);printf("# talk %d\n",at);
 attack(NAR_DOOR,armed?FIRE:PUNCH);
#ifdef ROOM_REPLAY
 press(SAVE);wait_frames(2);return;
#endif
 int won=used;U fallen=photo();
 for(int i=0;i<740;i++)play(0);
 NpcRef back;assert(npc_ref(NAR_DOOR,&back)&&!npc_replaces(NAR_DOOR)&&photo()!=fallen);
 printf("# recovered %d\n",used);
 assert(used-won==740);press(SAVE);wait_frames(2);
}
static void aquarium(int c){
 init("aquarium",c,0);press(USE);assert(scene==4);press(ENTER);
 assert(scene==NIGHT_VENUES_SCENE);press(FIRE);assert(nvs_mode==4);
 int at=announce(NAR_INEZ,1);printf("# talk %d\n",at);
 press(USE);assert((nvs_bonus&1)&&!npcTalkOn);
 attack(NAR_INEZ,PUNCH);
 for(int i=1;i<4;i++){
  for(int n=0;n<400&&nvs_position<119+i*222;n++)play(RIGHT);
  wait_frames(1);press(USE);
 }
 assert(nvs_bonus==15&&sv.cash==100&&sv.dirty==0);
 press(ESC);assert(nvs_mode==0);press(SAVE);wait_frames(2);
}
int main(int argc,char**argv){
 int c=argc>2?atoi(argv[2]):0;
 if(argc>1&&!strcmp(argv[1],"aquarium"))aquarium(c);else club(c,argc>3?atoi(argv[3]):0);
 printf("%d 0\n",used);
 for(int i=0;i<used;){int j=i+1;while(j<used&&tape[j]==tape[i])j++;printf("%d %u\n",j-i,tape[i]);i=j;}
 return 0;
}
