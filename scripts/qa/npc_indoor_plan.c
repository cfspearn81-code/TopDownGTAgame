/* Re-use the Vault walking solver, but only this new tape reaches the clients. */
#define main vault_previous_plan
#include "vault_keyboard_plan.c"
#undef main
static void aim_pixel(void){
 const char*probe=getenv("DC_VAULT_PROBES");
 PulsePose g=npc_room_pulse(interior_x,interior_y,interior_a,player_gait(),int_solid);
 PulseSprite s=pulse_project(g,(int)interior_cam_x,(int)interior_cam_y);
 paint();assert(!g.hand&&pixels[s.muzzle.y*W+s.muzzle.x]==PULSE_MUZZLE_INK);
 if(!probe)return;
 FILE*f=fopen(probe,"a");assert(f);
 U h=(2166136261u^PULSE_MUZZLE_INK)*16777619u;
 fprintf(f,"%d %d %d 1 1 %08x 05-Mika-draw-tip\n",count,s.muzzle.x,s.muzzle.y,h);
 assert(!fclose(f));
}
int main(int argc,char **argv){
 assert(argc==2);
 const char *seedText=getenv("DC_TRIP_SEED");
 set_session_seed(seedText?(U)atoi(seedText):17);
 init_city();starter_save();
 sv.x=loc[1].x;sv.y=loc[1].y;sv.intro=4;sv.gear=0;sv.hp=100;sv.cash=70;
 sv.hash=savehash();
 FILE*f=fopen(argv[1],"wb");assert(f&&fwrite(&sv,1,sizeof sv,f)==sizeof sv);
 assert(!fclose(f));
 moves[count++]=0;moves[count++]=ENTER;profiles_activate();
 play(0);play(0);play(0);enter_front();press(GFX);named_inside(0);
 press(USE);assert(v39.requested&&!npcTalkOn&&sv.dirty==0);frame("02-Mika-service");
 press(USE|BRAKE);assert(npcTalkOn&&npcTalk.id==N_ACT_MIKA);frame("05-Mika-dialogue");
 NpcRef pinned=npcTalk,other;assert(npc_ref(N_ACT_DANCE,&other));int barTick=v39.tick;
 for(int i=0;i<86;i++)play(0);
 NpcRef held,now;
 assert(npc_holds(N_ACT_MIKA)&&npc_ref(N_ACT_MIKA,&held)&&held.x==pinned.x&&held.y==pinned.y);
 assert(npc_ref(N_ACT_DANCE,&now)&&d2(now.x-other.x,now.y-other.y)>2.f&&v39.tick!=barTick);
 assert(npc_raw_ref(N_ACT_MIKA,&now)&&absf(now.x-held.x)>4.f);
 frame("05-Mika-held-while-bar-continues");
 press(UP);press(ENTER);assert(!npcTalkOn&&npcLetters[N_ACT_MIKA]==1);frame("02-Mika-greeting");
 press(USE|BRAKE);press(ENTER);assert(!npcTalkOn&&!npcEnc.phase);
 assert(sv.cash==70&&sv.dirty==0&&npcLetters[N_ACT_MIKA]==1);
 /* The user deliberately selects Rob; hands and held trigger stay separate. */
 press(USE|BRAKE);press(DOWN);play(ENTER|FIRE);
 assert(npcEnc.phase==NPC_DRAW&&p.trail<=0&&p.shot<=0);aim_pixel();
 int prize=npcEnc.prize;assert(prize>=15&&prize<=155&&npcEnc.surrender);
 for(int i=0;i<26;i++)play(ENTER|FIRE);
 assert(npcEnc.phase==NPC_YIELD&&sv.dirty==prize&&p.trail<=0);
 frame("02-Mika-surrender");
 for(int i=0;i<275;i++)play(0);
 assert(npcEnc.phase==NPC_IDLE&&v39.playerServed&&sv.dirty==prize);
 press(USE);assert(!v39.playerServed&&strstr(notice,"Enjoy"));
 frame("02-Mika-back-to-service");
 press(USE|BRAKE);press(DOWN);press(ENTER);assert(!npcEnc.phase&&sv.dirty==prize);
 frame("02-Mika-no-duplicate");
 leave_vault();press(SAVE);
 fprintf(stderr,"OUTCOME %d %d\n",sv.dirty,p.hp);
 printf("%d %08x\n",count,sv.hash);
 for(int i=0;i<count;){int j=i+1;while(j<count&&moves[i]==moves[j])j++;printf("%d %d\n",j-i,moves[i]);i=j;}
}
