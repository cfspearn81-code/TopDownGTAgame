/* Indoor lens changes remain render-only under real room controls.
   cc -std=c11 -O2 src/chase_room_rig_test.c -o build/room-lens */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void press(U key){tick(key);tick(0);}
static void prepare(int lens){
 starter_save();end_job();harbor_boats_reset();bank_ui_reset();vd_reset();npc_reset();clear_input();
 p=(Player){.x=loc[12].x,.y=loc[12].y,.hp=100,.car=-1};sv.intro=4;sv.gear=0;sv.cash=70;scene=1;
 menu=-1;mapOn=0;fastGfx=0;nv.garage[60]=lens;roomRig.key=0;
}
static void indoor(void){
 prepare(1);assert(ch_enabled());
 tick(USE);assert(scene==3&&interior_active_loc==12&&roomRig.key==25&&roomRig.entry==1.f);
 float firstY=interior_y;paint();assert(ir.h>=136.f);
 IrPoint start=ir_proj(interior_x,interior_y,16);
 tick(UP);assert(interior_y<firstY&&roomRig.entry<1.f);paint();
 float x=ir.x,y=ir.y,h=ir.h;for(int i=0;i<6;i++){paint();assert(ir.x==x&&ir.y==y&&ir.h==h);}
 float maxStep=0;int n=0;
 for(int phase=0;phase<5;phase++){
  const U directions[]={UP,RIGHT,LEFT,DOWN,0};U key=directions[phase];
  for(int i=0;i<32;i++){
   float beforeX=roomRig.x,beforeY=roomRig.y;
   tick(key);paint();IrPoint q=ir_proj(interior_x,interior_y,16);
   assert(q.iz&&q.x>100&&q.x<W-100&&q.y>145&&q.y<420);
   float d=sq(d2(roomRig.x-beforeX,roomRig.y-beforeY));if(d>maxStep)maxStep=d;n++;
  }
 }
 assert(maxStep>0.30f&&maxStep<5.f&&roomRig.entry==0);
 IrPoint final=ir_proj(interior_x,interior_y,16);assert(final.scale>start.scale*1.05f);
 press(HELP);assert(scene==2&&roomRig.key==25);x=roomRig.x;y=roomRig.y;
 press(CAMERA);assert(roomRig.x==x&&roomRig.y==y&&ch_mode_active());press(ESC);assert(scene==3);
 press(PHONE);assert(scene==NIGHT_PHONE_SCENE);x=roomRig.x;y=roomRig.y;press(CAMERA);assert(roomRig.key==25&&roomRig.x==x&&roomRig.y==y);press(ESC);
 press(CAMERA);assert(scene==3&&!roomRig.key&&!ch_mode_active());tick(CAMERA);assert(ch_mode_active()&&roomRig.entry==1.f);tick(0);
 press(GFX);assert(ch_pref()&&!ch_mode_active()&&!roomRig.key);tick(GFX);assert(ch_mode_active()&&roomRig.entry==1.f);tick(0);
 interior_x=480;interior_y=422;press(USE);assert(scene==1&&!roomRig.key&&ch_enabled());
 printf("Room reveal: entry %.3f to settled %.3f pixel/world unit; %d real movement samples, largest planar mount step %.2f units; six repeat renders identical; F1/Signal no snap and C/F2 work.\n",start.scale,final.scale,n,maxStep);
}
static void foreground(void){
 RoomMount m={.key=1,.view=3,.x=480,.y=300,.boom=150,.high=122,.mid=254};
 InteriorProp prop={468,305,24,12,INT_WALL,0};m.props=&prop;m.n=1;
 float yes=ch_room_cover(m,480,468);
 assert(yes>12&&yes<=22);prop.x=509;assert(ch_room_cover(m,480,468)==0);
 prop.x=468;prop.y=275;assert(ch_room_cover(m,480,468)==0);
 prop.y=305;prop.type=INT_BED;assert(ch_room_cover(m,480,468)==0);
 printf("Foreground guard lifts %.1f units for actual tall on-axis prop; low props, sideways props and props behind hero don't move camera.\n",yes);
}
static int fight(int lens){
 prepare(lens);open_interior(0);interior_x=interior_desk_x;interior_y=interior_desk_y;
 set_session_seed(0);for(U i=0;;i++){set_session_seed(i);npc_roll_bound(141);if(npc_roll_bound(100)>=35){set_session_seed(i);break;}}
 press(USE);assert(menu==0);doOpt(N_ACT_OPEN);assert(npcTalkOn);press(DOWN);tick(ENTER);assert(npcEnc.phase==NPC_DRAW);
 int prize=npcEnc.prize;for(int i=0;i<25;i++)tick(0);
 int fire=0;assert(npcEnc.phase==NPC_FIGHT);while(npcEnc.phase==NPC_FIGHT&&fire<100){tick(FIRE);paint();fire++;}
 assert(npcEnc.phase==NPC_DROP&&npcTraceTicks>0&&sv.dirty==0&&npcEnc.prize==prize);
 for(int i=0;i<20;i++)tick(0);
 for(int i=0;i<12&&npcEnc.phase==NPC_DROP;i++){
  U k=absf(npcEnc.actor.x-interior_x)>absf(npcEnc.actor.y-interior_y)?
   npcEnc.actor.x>interior_x?RIGHT:LEFT:npcEnc.actor.y>interior_y?DOWN:UP;
  tick(k);paint();
 }
 assert(sv.dirty==prize&&npcEnc.phase==NPC_RECOVER);
 for(int i=0;i<50;i++)tick(0);assert(sv.dirty==prize);
 return 100000*fire+1000*p.hp+prize;
}
static void rides(void){
 prepare(1);p.x=2948;p.y=2206;press(USE);assert(harbor_boat_active());paint();assert(chProjected&&ch_subject()==CH_HELM);
 float before=p.a;for(int i=0;i<45;i++)tick(UP|(i>20?RIGHT:0));
 paint();assert(p.a!=before&&ch_subject()==CH_HELM);
 press(GFX);paint();assert(ch_pref()&&!chProjected&&harbor_boat_active());
 press(GFX);paint();assert(chProjected&&harbor_boat_active());
}
static void rail(void){
 prepare(1);p.x=ordinaryDoorX[74];p.y=ordinaryDoorY[74];nv.world[NVS_STAGE]=5;tick(0);press(USE);assert(scene==4);
 press(ENTER);assert(scene==NIGHT_VENUES_SCENE);tick(FIRE);assert(nvs_mode==4&&roomRig.key==1041&&roomRig.entry==1.f);
 for(int i=0;i<40;i++)tick(RIGHT);paint();
 int saved=nvs_position;press(GFX);assert(!ch_mode_active()&&ch_pref()&&nvs_position==saved);
 press(GFX);assert(ch_mode_active()&&roomRig.key==1041&&nvs_position==saved);press(ESC);
 assert(nvs_mode!=4&&!roomRig.key);
}
int main(void){
 init_city();indoor();foreground();int a=fight(0),b=fight(1);assert(a==b);
 printf("Identical combat/receipt in both views: fire %d ticks, health %d, payout $%d (collected once).\n",a/100000,a/1000%100,a%1000);
 rides();rail();puts("F2 fallback and 3D restored while steering the original hull and exploring the aquarium.");
}
