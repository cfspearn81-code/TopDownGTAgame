/* Play the production movement/interaction paths; start fixtures do not use
   real career files. An optional output folder receives the actual frame. */
#define main historic_room_routes
#include "interior_routes_test.c"
#undef main
#include <assert.h>
static const char *captures;static int renders;
static void shot(const char*n){
 paint();renders++;if(!captures||!n)return;char f[512];snprintf(f,sizeof f,"%s/%s.ppm",captures,n);
 FILE*o=fopen(f,"wb");assert(o);fprintf(o,"P6\n%d %d\n255\n",W,H);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc(c>>16,o);fputc(c>>8,o);fputc(c,o);}fclose(o);
}
static void reps(int index){
 scene=1;menu=-1;open_interior(index);assert(scene==3);g_kind=loc[index].kind;room_name=loc[index].label;
 shot(g_kind==13?"01-apartment":g_kind==1?"02-vault39":g_kind==3?"03-showroom":0);
 go(FEATURE_G);press_key(USE);assert(interior_inspection==g_kind+1);
 if(g_kind==1){
  go(DESK_G);press_key(USE);for(int i=0;i<V39_CYCLE+94;i++)tick(0);assert(v39.playerServed);
  press_key(USE);assert(!v39.playerServed);go(GATE_G);press_key(USE);assert(interior_subroom);
  shot("04-ivo");{
   NpcRef r;int id=N_ACT_REAR+index;assert(npc_ref(id,&r));IrPoint q=ir_proj(r.x,r.y,0);
   U before[72*72];int changed=0;
   for(int yy=0;yy<72;yy++)for(int xx=0;xx<72;xx++)before[yy*72+xx]=pixels[(q.y-60+yy)*W+q.x-36+xx];
   NpcEncounter keep=npcEnc;npcEnc.phase=NPC_DROP;npcEnc.actor=r;npcEnc.actor.x=20;npcEnc.actor.y=20;npcEnc.room=npc_room_key();paint();
   for(int yy=0;yy<72;yy++)for(int xx=0;xx<72;xx++)changed+=before[yy*72+xx]!=pixels[(q.y-60+yy)*W+q.x-36+xx];
   npcEnc=keep;printf("Ivo pixels visible behind the counter: %d\n",changed);assert(changed>=28);
  }go(DESK_G);press_key(USE);assert(menu==1);shot(0);press_key(ESC);
  go(EXIT_G);press_key(USE);assert(scene==3&&!interior_subroom);
 }else if(g_kind==4||legal_index_for_kind(g_kind)>=0){int li=legal_index_for_kind(g_kind);
  if(g_kind==4){sv.b[0]=(Business){.own=1,.tier=2};sv.units=8;}else sv.legit[li].owned=1;
  go(GATE_G);press_key(USE);assert(interior_subroom);go(DESK_G);press_key(USE);assert(menu>=0);
  if(g_kind==4){
   press_key(ENTER);assert(sv.b[0].timer>0.f);sv.b[0].crew=1;float before=sv.b[0].timer;
   for(int i=0;i<1800;i++)tick(0);assert(sv.b[0].timer<before-29);shot(0);
  }
  press_key(ESC);shot(g_kind==4?"05-grow-floor":0);
  go(EXIT_G);press_key(USE);assert(scene==3&&!interior_subroom);
 }else if(g_kind==15){
  go(CHIEF_G);press_key(USE);assert(strstr(notice,"Rivas"));go(CELLS_G);press_key(USE);assert(strstr(notice,"Custody"));shot(0);
 }else if(g_kind==13){go(DESK_G);press_key(USE);shot(0);}
 go(EXIT_G);press_key(USE);assert(scene==1);
}
static void special(void){
 /* Door fixtures; thereafter controls are normal. */
 scene=1;end_job();menu=-1;p.car=-1;sv.intro=4;p.x=ordinaryDoorX[ncFrontId];p.y=ordinaryDoorY[ncFrontId];tick(0);
 press_key(USE);assert(scene==NIGHT_CLUB_SCENE);shot("07-afterimage");
 for(int j=0;j<41;j++)tick(UP);for(int j=0;j<60;j++)tick(LEFT);tick(0);
 /* Contacts, doors and ledger are reached by the ordinary room logic. */
 ncX=160;ncY=355;press_key(USE);assert(ncRoom==NC_OFFICE);shot("08-box-office");
 ncX=475;ncY=267;press_key(USE);assert(ncPanel==NC_DESK);shot(0);press_key(ESC);
 ncX=480;ncY=418;press_key(USE);assert(ncRoom==NC_FLOOR);
 ncX=798;ncY=354;press_key(USE);assert(ncRoom==NC_SUITE);shot(0);
 ncX=481;ncY=264;press_key(USE);assert(ncPanel!=0);press_key(ESC);
 p.x=NT_DOOR_X;p.y=NT_DOOR_Y;scene=1;menu=-1;end_job();clear_input();press_key(ENTER);
 assert(scene==NIGHT_TRAP_SCENE&&nt_pane>=0);sv.cash=900;press_key(ENTER);press_key(ENTER);assert(night_trap_owned());if(nt_pane>=0)press_key(ESC);
 for(int j=0;j<150&&nt_nearest()!=1;j++)tick(UP);tick(0);press_key(USE);assert(nt_pane==NT_BUILD);shot(0);press_key(ESC);shot("09-trap");
 scene=1;p.x=ordinaryDoorX[74];p.y=ordinaryDoorY[74];menu=-1;nv.world[NVS_STAGE]=5;tick(0);press_key(USE);assert(scene==4);
 press_key(ENTER);assert(scene==NIGHT_VENUES_SCENE);press_key(FIRE);assert(nvs_mode==4);
 press_key(USE);for(int j=1;j<4;j++){int stop=119+j*222;while(nvs_position<stop)tick(RIGHT);tick(0);press_key(USE);}
 assert(nvs_bonus==15);shot("10-reef-walk");press_key(ESC);assert(nvs_mode!=4);
}
int main(int argc,char**argv){
 init_city();scene=1;sv.intro=4;sv.legal=1;p.car=-1;end_job();mapOn=0;menu=-1;nv.garage[60]=1;fastGfx=0;assert(ch_mode_active());captures=argc>1?argv[1]:0;
 for(int i=0;i<LEN(loc);i++)reps(i);
 int ordinary=0;for(int b=0;b<nb;b++)if(ordinaryDoorY[b]&&b!=ncFrontId){
  scene=1;menu=-1;walkin_open(b);if(scene!=4)continue;ordinary++;room_name=walkin_name(b);
  shot(b==31?"06-tram-hall":0);go(STATION_G);int old=wi_sw[b];press_key(USE);
  if(!tram_hall(b))assert(wi_sw[b]!=old);go(PLAQUE_G);press_key(USE);assert(!strcmp(notice,wiPlaque[b]));
  go(EXIT_G);press_key(USE);assert(scene==1);
 }
 special();printf("PASS: %d named venues + gated rooms, %d public street floors; %d actual walking ticks across %d paths, %d perspective renders; Vault counter/rear, planter menu time, police, Afterimage/Cabaret ledger, Bell Lane purchase/bench, aquarium four-stop award.\n",LEN(loc),ordinary,steps_taken,route_count,renders);
 return 0;
}
