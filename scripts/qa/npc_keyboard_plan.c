/* NPC input-route generator. Game internals are read to assert transitions
 * and derive framebuffer crops. The only write before the real input chain
 * is a fresh owned-pistol Rev10 save, never an NPC or encounter state. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {KEY_CAP=4800};
static U tape[KEY_CAP];
static int frames;
static FILE *report;
static void play(U keys){
 assert(frames<KEY_CAP);
 tape[frames++]=keys;
 tick(keys);
}
static void wait_ticks(int n){while(n-->0)play(0);}
static void tap(U keys){play(keys);play(0);}

static void shot_test(const char*label,int x,int y,int w,int h,U ink,U minimum){
 assert(w>0&&h>0&&x>=0&&y>=0&&x+w<=W&&y+h<=H);
 U check=2166136261u;
 U hits=0;
 for(int yy=y;yy<y+h;yy++)for(int xx=x;xx<x+w;xx++){
  U pix=pixels[yy*W+xx]&0xffffffu;
  check=(check^pix)*16777619u;hits+=pix==ink;
 }
 assert(!ink||hits>=minimum);
 fprintf(report,"%d %d %d %d %d %08x %08x %s\n",frames,x,y,w,h,ink,ink?minimum:check,label);
}
static void shot(const char*label,int x,int y,int w,int h){shot_test(label,x,y,w,h,0,0);}
static void panel(const char*label){
 paint();
 /* Title with the actual identity and every available choice; verify the
  * selection background from the rendered modal, not just its state. */
 assert(npcTalkOn&&pixels[(127+87+npcChoice*31+12)*W+700]==0x2a4950u);
 shot(label,195,127,570,277);
}
static void toast(const char*label){
 paint();
 assert(p.noticeT>0);
 int rows=0,width=0;
 char row[130];const char *line=notice;
 while(*line){line=hud_line(line,711,row);width=max(width,textw(row,0));rows++;}
 assert(rows>=1);int h=28+(rows-1)*21;
 shot(label,16,466-h,width+24,h);
}
static void state(const char*label){
 paint();
 assert(npcEnc.phase!=NPC_IDLE);
 int x=sx(npcEnc.actor.x),y=sy(npcEnc.actor.y);U sample=0;
 if(npcEnc.phase==NPC_FIGHT)sample=PINK;
 if(npcEnc.phase==NPC_DRAW)sample=GOLD;
 if(npcEnc.phase==NPC_YIELD)sample=LIME;
 if(npcEnc.phase==NPC_DROP)sample=0x4a7547u;
 /* Bind the foreground marker, not incidental wet-road pixels, to
  * the native replays. The optional crop expands around this probe. */
 assert(sample);
 if(npcEnc.phase==NPC_DROP)shot_test(label,x+1,y-10,15,11,sample,5);
 else shot_test(label,x-8,y-32,17,15,sample,5);
}
static int open_dialogue(int hold){
 assert(scene==1&&menu<0&&!p.dead&&p.car<0);
 assert(street_use_target(placeNear(),carNear()).kind==STREET_USE_NONE);
 play(USE);
 assert(npcTalkOn&&npcTalk.kind==0&&npcChoice==1);
 int id=npcTalk.id;for(int i=0;i<hold;i++)play(USE);play(0);
 assert(npcTalkOn&&npcTalk.id==id);
 return id;
}
static int choose_rob(void){
 int id=open_dialogue(0);panel("rob-menu");
 tap(DOWN);assert(npcTalkOn&&npcChoice==2);
 panel("rob-selected");
 tap(ENTER);assert(npcEnc.actor.id==id&&npcEnc.phase==NPC_DRAW&&!npcTalkOn);
 assert(pistol_equipped()&&p.trail<=0&&p.shot<=0&&sv.dirty==0&&p.heat==0.f);
 assert(d2(npcEnc.actor.x-p.x,npcEnc.actor.y-p.y)>0);
 float a=look(npcEnc.actor.x-p.x,npcEnc.actor.y-p.y);
 assert(absf(p.a-a)<.01f&&npcEnc.prize>=15&&npcEnc.prize<=155);
 state("drawn-without-firing");
 paint();
 PulsePose g=pulse_visible_pose();PulseSprite s=pulse_project(g,cx,cy);
 assert(!g.hand&&pixels[s.muzzle.y*W+s.muzzle.x]==PULSE_MUZZLE_INK);
 shot_test("drawn-pistol",s.muzzle.x-2,s.muzzle.y-2,5,5,PULSE_MUZZLE_INK,1);
 return id;
}
static void conversation(void){
 int id=open_dialogue(7);panel("quiet-default");
 /* Holding E does not immediately confirm the dialogue it also opened. */
 play(USE); /* This later fresh press is allowed to confirm Quiet. */
 assert(!npcTalkOn&&!npcLetters[id]&&!sv.dirty);
 for(int i=0;i<7;i++)play(USE);
 assert(!npcTalkOn&&!npcLetters[id]);play(0);
 id=open_dialogue(0);tap(UP);assert(npcChoice==0);
 panel("hello-selected");
 NpcRef r=npcTalk;
 play(ENTER);
 char greet[112];snprintf(greet,sizeof greet,"%s: %s",r.name,npc_greet_line(r.kind,id,0));
 assert(!strcmp(notice,greet)&&npcLetters[id]==1&&!npcTalkOn);
 for(int i=0;i<7;i++)play(ENTER);
 assert(!strcmp(notice,greet)&&npcLetters[id]==1&&!npcTalkOn&&!sv.dirty);
 toast("first-greeting");play(0);
 id=open_dialogue(0);tap(ESC);
 assert(!npcTalkOn&&!sv.dirty&&!p.heat&&p.hp==100);
 toast("cancel-no-extra-greeting");
}
static int wait_for_choice(int surrender){
 int i=0;
 while(npcEnc.phase==NPC_DRAW&&i++<60){
  play(FIRE);assert(p.trail<=0&&p.shot<=0);
 }
 assert(i<60&&npcEnc.phase==(surrender?NPC_YIELD:NPC_FIGHT));
 /* A Space held through the transition cannot fire; release it first. */
 play(FIRE);assert(p.trail<=0&&p.shot<=0);play(0);
 return i;
}
static void give_cash(void){
 int id=choose_rob(),amount=npcEnc.prize;
 assert(npcEnc.surrender);
 wait_for_choice(1);
 assert(sv.dirty==amount&&sv.cash==70&&p.hp==100&&npcEnc.prize==0&&npcSpent[id]);
 assert(strstr(notice,"hand over their cash")&&p.trail<=0);
 state("hands-raised");toast("surrender-pays");
 wait_ticks(50);
 assert(npcEnc.phase==NPC_IDLE&&sv.dirty==amount&&!p.dead);
}
static void modal_roundtrip(void){
 /* Genuine launcher key chain; opening a non-world view must not spend a
    target, discard ground loot, or restart a battle. Do not test save slots
    here: the two live launchers use different native card inventories. */
 U mods[]={HELP,PHONE}; int back=scene;
 for(int j=0;j<LEN(mods);j++){
  int phase=npcEnc.phase,prize=npcEnc.prize;
  tap(mods[j]);assert(scene!=back);wait_ticks(12);tap(ESC);
  assert(scene==back&&npcEnc.phase==phase&&npcEnc.prize==prize);
 }
}
static void fight(int flee,int modal){
 int id=choose_rob(),amount=npcEnc.prize;
 assert(!npcEnc.surrender);
 wait_for_choice(0);
 assert(!sv.dirty&&npcSpent[id]&&npcEnc.hp==3);
 state("refuses");toast("fight-or-leave");
 if(flee){
  float oldx=p.x,oldy=p.y;
  for(int i=0;i<160&&npcEnc.phase==NPC_FIGHT;i++)play(RIGHT|BRAKE);
  assert(npcEnc.phase==NPC_IDLE&&!sv.dirty&&!p.dead&&d2(p.x-oldx,p.y-oldy)>100*100);
  assert(strstr(notice,"got away"));toast("fled-no-take");
  play(0);
  return;
 }
 if(modal)modal_roundtrip();
 for(int i=0;i<75&&p.hp==100;i++)play(0);
 assert(npcEnc.phase==NPC_FIGHT&&p.hp<100&&p.hp>=76);
 state("npc-strike");
 assert(npcEnc.strike>0);
 int ax=sx(npcEnc.actor.x),ay=sy(npcEnc.actor.y);
 shot_test("npc-swing-hand",ax-15,ay-22,31,28,0xffd4aau,2);
 play(FIRE);assert(p.trail>0&&npcEnc.hp==2);
 paint();PulseSprite s=pulse_project(firedPulse,cx,cy);
 assert(s.muzzle.x==sx(trailX)&&s.muzzle.y==sy(trailY)&&pixels[s.muzzle.y*W+s.muzzle.x]==0xfff2bdu);
 shot_test("fight-muzzle-flash",s.muzzle.x-2,s.muzzle.y-2,5,5,0xfff2bdu,1);
 for(int i=0;i<85&&npcEnc.phase==NPC_FIGHT;i++)play(FIRE);
 assert(npcEnc.phase==NPC_DROP&&!sv.dirty&&npcEnc.prize==amount);
 state("cash-drop");toast("walk-over-cash");
 /* Continue holding long enough to cross the original pistol cooldown.
  * A defeated encounter must not leak that hold into the normal shot path. */
 float drop_heat=p.heat;
 for(int i=0;i<22;i++)play(FIRE);
 play(0);
 assert(npcEnc.phase==NPC_DROP&&!sv.dirty&&p.trail<=0&&p.shot<=0&&p.heat==drop_heat);
 if(modal)modal_roundtrip();
 for(int i=0;i<24&&npcEnc.phase==NPC_DROP;i++){
  float dx=npcEnc.actor.x-p.x,dy=npcEnc.actor.y-p.y;
  U dir=0;if(absf(dx)>2)dir|=dx>0?RIGHT:LEFT;
  if(absf(dy)>2)dir|=dy>0?DOWN:UP;
  play(dir);
 }
 assert(npcEnc.phase==NPC_RECOVER&&npcEnc.prize==0&&sv.dirty==amount&&sv.cash==70);
 assert(strstr(notice,"Dropped cash collected")&&!p.dead);
 toast("collected-once");wait_ticks(101);
 assert(npcEnc.phase==NPC_IDLE&&sv.dirty==amount&&npcSpent[id]);
}
static void guide_hotfix(void){
 tap(HELP);assert(scene==2&&helpBack==1);paint();
 /* Shift+E and the new driving-view row both fit above the legacy legend. */
 int step=guide_step(LEN(guideStreet)),talk_y=183+8*step;
 assert(LEN(guideStreet)==10&&talk_y+step+20<395);
 assert(pixels[talk_y*W+92]==0x162931u);
 assert(pixels[395*W+92]==0x39545eu);
 shot("guide-complete",64,37,832,466);
 shot("shift-e-row",92,talk_y,367,step);
 shot("separate-legend",92,395,775,29);
 tap(ESC);assert(scene==1&&menu<0);
}
static void held_body(const char*label,int id){
 Ped *q=peds+id;
 paint();int x=sx(q->x),y=sy(q->y);
 U coat=(id&1)?0x92b7b4:0xca8c71;
 /* The original fixture stood beside the old east camera boundary.
    The extended map now centers the same curb; the dialogue covers the
    near actor in that view. Preserve motion assertions below and probe
    the selected identity when it is under the panel, the actual
    model pixels whenever it is unobstructed. */
 int left=max(773,x-8);
 if(npcTalkOn&&x<773&&x>=186&&y>130&&y<409){
  shot(label,221,160,480,53);
  return;
 }
 if(!npcTalkOn)left=max(0,x-8);
 assert(x>=9&&x<W-9&&left<x+9);
 U ink=mix(coat,0x111a27,1);
 if(npcTalkOn)ink=((ink>>1)&0x007f7f7fu)+0x050a0du;
 shot_test(label,left,y-22,x+9-left,24,ink,1);
}
static void conversation_hold(void){
 /* East Freight's normal spawn gives an exposed rendering beside the
    modal: both peds start at the same curb, but one works a later shift. */
 tap(GFX);wait_ticks(34);
 assert(peds[25].s>0&&peds[10].x==1890.f);
 tap(BRAKE|USE);
 assert(npcTalkOn&&npcTalk.id==25&&!npcEnc.phase);
 float sx0=peds[25].x,sy0=peds[25].y,ox=peds[10].x;
 held_body("selected-at-open",25);
 U clock=sv.playedTicks;
 wait_ticks(53);assert(npcTalkOn&&npcTalk.id==25);
 #ifdef DC_NPC_HOLD_BASELINE
 assert(peds[25].x<sx0-17.f);
 held_body("selected-drifted-mid-menu",25);
 #else
 assert(peds[25].x==sx0&&peds[25].y==sy0&&npc_holds(25));
 held_body("selected-held-mid-menu",25);
 #endif
 wait_ticks(52);assert(npcTalkOn&&npcTalk.id==25);
 assert(peds[10].x<ox-14&&sv.playedTicks==clock+105);
 #ifdef DC_NPC_HOLD_BASELINE
 assert(peds[25].x<sx0-30.f);
 #else
 assert(peds[25].x==sx0&&peds[25].y==sy0&&npc_holds(25));
 float x,y,a0;int step=-1;
 assert(npc_present_pose(25,&x,&y,&a0,&step)==1&&x==sx0&&y==sy0&&step==0);
 held_body("selected-held-throughout",25);
 #endif
 held_body("other-ped-moved",10);
 tap(ESC);
 assert(!npcTalkOn);
 float resume=peds[25].x;
 wait_ticks(21);assert(peds[25].x<resume-5.f);
 held_body("selected-resumed",25);
}

/* No teleports: walk along the open Parkway, stop a passerby, and walk home. */
static void outer_sidewalk(int strike){
 tap(GFX);
 for(int i=0;i<750&&p.x<2817.f;i++)play(RIGHT|BRAKE);
 assert(p.x>=2817.f&&p.x<2821.f);
 for(int k=0;k<17;k++)play(DOWN);
 tap(USE);assert(npcTalkOn&&npcTalk.id>=160);
 int id=npcTalk.id;float x=npcTalk.x,y=npcTalk.y;
 panel("parkway-passerby");
 wait_ticks(90);NpcRef r;assert(npc_ref(id,&r)&&r.x==x&&r.y==y);
 tap(UP);panel("parkway-hello");tap(ENTER);
 assert(!npcTalkOn&&npcLetters[id]==1&&strstr(notice,r.name));toast("parkway-reply");
 if(strike){
  /* No phase injection. Turn down from the highway and use ordinary Fire
     on the group we actually walked to. No Rob interaction was selected. */
  tap(DOWN);int down=-1;NpcRef latest[N_OUTER_SEATS];int present[N_OUTER_SEATS]={0};
  int group=N_ACT_OUTER+(id-N_ACT_OUTER)/N_OUTER_SEATS*N_OUTER_SEATS;
  for(int k=0;k<N_OUTER_SEATS;k++)present[k]=npc_ref(group+k,latest+k);
  for(int j=0;j<200&&down<0;j++){
   play(FIRE);
   for(int k=0;k<N_OUTER_SEATS;k++)if(present[k]){
    NpcRef now;if(npc_ref(group+k,&now))latest[k]=now;else{down=k;break;}
   }
  }
  assert(down>=0&&npcEnc.phase==NPC_IDLE&&p.heat>50&&!sv.dirty);paint();
  /* Neighbors are still moving on the same shoulder. Verify the fallen
     actor's guaranteed sprite ink instead of hashing nearby footfalls. */
  shot_test("parkway-contact",sx(latest[down].x)-18,sy(latest[down].y)-14,37,29,0x85757bu,1);
  play(0);
  if(strike==2){
   for(int t=0;t<2100&&!dispatch_visible();t++)play(FIRE);
   assert(dispatch_visible()&&dispatch_engaged()>0);
   int seen=0;
   for(int n=0;n<DISPATCH_UNITS;n++){
    Ped*q=peds+DISPATCH_FIRST+n;
    if(q->hp>0&&dispatchUnits[n].state==D_RESPONSE&&d2(q->x-p.x,q->y-p.y)<177.f*177.f&&
       street_line_open(q->x,q->y,p.x,p.y,2)){
     paint();
     /* A moving patrol alternates its uniform flash and walk frame.
        Check an actual uniform/badge pixel near the unit rather than the
        hash of an entire moving group, which varies by host presentation. */
     U colors[]={0xe8cb86,0x83a4c7,0x9dbed4,mix(0x668df0,0x111a27,1),mix(PINK,0x111a27,1)};
     int xx=sx(q->x),yy=sy(q->y);
     for(int c=0;c<LEN(colors)&&!seen;c++){
      int hits=0;for(int b=yy-24;b<=yy+4;b++)for(int a=xx-12;a<=xx+12;a++)hits+=(pixels[b*W+a]&0xffffffu)==colors[c];
      if(hits){shot_test("regional-responder",xx-28,yy-32,57,45,colors[c],1);seen=1;}
     }
     if(seen)break;
    }
   }
   assert(seen);play(0);return;
  }
 }
 for(int k=0;k<(strike?18:17);k++)play(UP);
 for(int i=0;i<750&&p.x>1985.f;i++)play(LEFT|BRAKE);
 assert(p.x<1986.f&&!outer_is_new(p.x,p.y));play(0);
}

int main(int argc,char **argv){
 assert(argc==4&&"usage: plan START.DAT PROBE_FILE conversation|surrender|defeat|flee|guide|hold|outer");
 int case_id=!strcmp(argv[3],"conversation")?0:!strcmp(argv[3],"surrender")?1:
             !strcmp(argv[3],"defeat")?2:!strcmp(argv[3],"flee")?3:!strcmp(argv[3],"guide")?4:!strcmp(argv[3],"hold")?5:!strcmp(argv[3],"outer")?6:!strcmp(argv[3],"modal")?7:!strcmp(argv[3],"outer-strike")?8:!strcmp(argv[3],"outer-response")?9:-1;
 assert(case_id>=0);
 /* Seed0 first roll: $36, 34 (surrenders). Seed2: $17, 44 (fights).
  * The walking route approaches the normal Tide Market sidewalk actors
  * at the southeast corner of the old apartment block. */
 set_session_seed(case_id==2||case_id==3||case_id==7?2u:0u);init_city();starter_save();
 sv.intro=4;sv.gear=0;sv.cash=70;sv.x=(case_id==6||case_id==8||case_id==9)?1980.f:case_id==5?1880.f:336.f;sv.y=(case_id==6||case_id==8||case_id==9)?1024.f:case_id==5?1006.f:762.f;stamp_save();
 FILE *career=fopen(argv[1],"wb");assert(career&&fwrite(&sv,sizeof sv,1,career)==1&&!fclose(career));
 report=fopen(argv[2],"w");assert(report);
 profiles_activate();tape[frames++]=0;tape[frames++]=ENTER;
 wait_ticks(4);
 if(case_id==6||case_id==8||case_id==9)outer_sidewalk(case_id==9?2:case_id==8);else if(case_id!=5){
 float oldy=p.y;
 for(int i=0;i<4;i++)play(UP);
 assert(scene==1&&p.y<oldy-6.f&&p.y>oldy-7.f);
 wait_ticks(2);
 if(case_id==0)conversation();
 if(case_id==1)give_cash();
 if(case_id==2||case_id==3||case_id==7)fight(case_id==3,case_id==7);
 if(case_id==4)guide_hotfix();
 }else conversation_hold();
 assert(sv.cash==70&&sv.dirty==(case_id==1?36:(case_id==2||case_id==7)?17:0)&&!npcTalkOn&&!p.dead&&scene==1);
 play(SAVE);assert(sv.hash==savehash()&&sv.x==p.x&&sv.y==p.y&&sv.hp==p.hp);
 U digest=sv.hash;
 if(case_id==9){
  Save11 blob={0};blob.base=sv;blob.night=nv;blob.base.rev=NIGHT_SAVE_REV;
  outer_safe_anchor(&blob.base);blob.night.world[NW_SAVE_X]|=0x80000000u;
  blob.base.hash=night_hash(&blob.base,sizeof(Save)-4);digest=night_hash(&blob,sizeof blob-4);
 }
 wait_ticks(3);
 fprintf(stderr,"%s: %d frames, %d dirty/%d clean, %d HP, saved %.2f/%.2f.\n",argv[3],frames,sv.dirty,sv.cash,p.hp,sv.x,sv.y);
 assert(!fclose(report));
 printf("%d %08x\n",frames,digest);
 for(int i=0;i<frames;){int j=i+1;while(j<frames&&tape[j]==tape[i])j++;printf("%d %u\n",j-i,tape[i]);i=j;}
}
