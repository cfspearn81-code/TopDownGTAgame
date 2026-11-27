/* Harbor caseboard. These are scripted arcade objectives involving a nonliving
 * security automaton, a fictional prototype and an uncrewed training float. No NPC
 * identity, ordinary combat, law/heat/evasion or real inventory is reused. */
enum { HJ_PHASE=13,HJ_DONE=14,HJ_FLOAT=15 };
enum { HJ_IDLE,HJ_RELAY_TAG,HJ_RELAY_LIFT,HJ_RELAY_TRUNK,HJ_RELAY_QUAY,HJ_RELAY_BUOY,
       HJ_BLUE_CLUE,HJ_BLUE_FIND,HJ_BLUE_RETURN,HJ_BUMP_TAG,HJ_BUMP_LOOT,HJ_BUMP_RETURN };
enum { HJ_RELAY,HJ_BLUE,HJ_BUMP,HJ_CANCEL,HJ_NCARD };
enum { HJ_OFFICE_X=2896,HJ_OFFICE_Y=2096,HJ_ALLEY_X=2848,HJ_ALLEY_Y=2114,
       HJ_CUE_X=2824,HJ_CUE_Y=2145,HJ_QUAY_X=2824,HJ_QUAY_Y=2192 };
static const NavPoint hj_office={HJ_OFFICE_X,HJ_OFFICE_Y},hj_alley={HJ_ALLEY_X,HJ_ALLEY_Y},hj_cue={HJ_CUE_X,HJ_CUE_Y},hj_quay={HJ_QUAY_X,HJ_QUAY_Y};
static const NavPoint hj_nominal[]={{2840,2420},{2718,2390},{2635,2440}};
static const char *const hj_name[]={"Harbor / Relay recovery","Harbor / Blue parcel","Harbor / Breakwater bump","Harbor / Cancel assignment"};
static const int hj_score[]={185,150,105};
static int hj_panel,hj_row,hj_confirm;
static float hj_attention,hj_interference; /* temporary arcade encounter, never saved */
static char hj_note[166];
static void harbor_jobs_reset(void){hj_panel=hj_row=hj_confirm=0;hj_attention=hj_interference=0;hj_note[0]=0;}
static int harbor_jobs_panel_active(void){return hj_panel;}
static void hj_msg(const char *text){
 if(!hj_panel){np_msg(text);return;}
 int n=0;while(n<(int)sizeof(hj_note)-1&&text[n]){hj_note[n]=text[n];n++;}hj_note[n]=0;
}
static int hj_phase(void){U v=nv.phone[HJ_PHASE];return v<=HJ_BUMP_RETURN?(int)v:HJ_IDLE;}
static int hj_case(int phase){return phase>=HJ_BUMP_TAG?HJ_BUMP:phase>=HJ_BLUE_CLUE?HJ_BLUE:phase>=HJ_RELAY_TAG?HJ_RELAY:-1;}
static int hj_complete(int which){return !!(nv.phone[HJ_DONE]&(1u<<which));}
static int hj_near(NavPoint d,int radius){return d2(p.x-d.x,p.y-d.y)<radius*radius;}
static NavPoint hj_automaton(void){
 NavPoint q={hj_alley.x+(int)(sn(anim*2.4f)*11.f),hj_alley.y+(int)(cs(anim*2.4f)*7.f)};
 return solid(q.x,q.y,10)?hj_alley:q;
}
static int hj_other_job(void){
 int x,y;return job.kind>=0||vdActive>=0||ncr_active()||ng_route_goal(&x,&y);
}
static int hj_open_world(void){return scene==1&&!mapOn&&menu<0&&!p.dead&&!vdPanel&&atmOpen<0&&!hj_other_job();}
static int hj_own(int cond){return sv.owned&&nc>0&&cars[0].owner==1&&(!cond||cars[0].cond>0);}
static NavPoint hj_rear(void){const Car*c=&cars[0];return (NavPoint){(int)(c->x-cs(c->a)*22.f),(int)(c->y-sn(c->a)*22.f)};}
static NavPoint hj_dock_walk(int dock){int x=2824,y=2234;harbor_dock_point(dock,&x,&y);return (NavPoint){x,y};}
static NavPoint hj_water(int task){
 NavPoint q=hj_nominal[cl(task,0,HJ_BUMP)];
 if(harbor_nav_water(q.x,q.y,20))return q;
 /* Fail soft if another build edited the authored coast. Search only
  * nearby open virtual tiles; never place an objective on land. */
 for(int r=24;r<=216;r+=24)for(int i=0;i<8;i++){
  int x=q.x+(int)(cs(.7854f*i)*r),y=q.y+(int)(sn(.7854f*i)*r);
  if(harbor_nav_water(x,y,20))return (NavPoint){x,y};}
 int x,y;
 if(harbor_dock_point(3,&x,&y)&&harbor_nav_water(x,y+60,14))return (NavPoint){x,y+60};
 return q;
}
static NavPoint hj_trainer(void){
 NavPoint q=hj_water(HJ_BUMP);
 int x=q.x+(int)(sn(anim*.36f)*62),y=q.y+(int)(cs(anim*.36f)*36);
 return harbor_nav_water(x,y,14)?(NavPoint){x,y}:q;
}
static U hj_pack(NavPoint q){return (U)cl(q.x,0,4095)|((U)cl(q.y,0,4095)<<12);}
static NavPoint hj_drift(void){
 U v=nv.phone[HJ_FLOAT];NavPoint q={(int)(v&4095u),(int)((v>>12)&4095u)};
 return harbor_nav_water(q.x,q.y,12)?q:hj_water(HJ_BUMP);
}
static int hj_save_phase(int phase,const char*s,int show){
 nv.phone[HJ_PHASE]=(U)phase;navGoalKind=0;request_save();
 if(show)tell(saveFlash<0?"Harbor claim pending on this device. F5 retries the save.":s);return 1;
}
static int hj_finish(int which,int pay){
 if(hj_complete(which)){nv.phone[HJ_FLOAT]=0;return hj_save_phase(HJ_IDLE,"Harbor / That commission already paid.",1);}
 if(sv.cash>WALLET_LIMIT-pay){tell("Harbor / Wallet full. Your commission remains ready.");return 1;}
 sv.cash+=pay;nv.phone[HJ_DONE]|=1u<<which;nv.phone[HJ_FLOAT]=0;
 int r=hj_save_phase(HJ_IDLE,"Harbor commission paid in clean cash.",1);
 if(saveFlash>=0)cashNote("Harbor commission logged.",pay);return r;
}
static const char *hj_step_desc(int p){
 switch(p){
 case HJ_RELAY_TAG:return "Approach the nonhuman security automaton in the marked alley. E disables it in this scripted arcade encounter.";
 case HJ_RELAY_LIFT:return "Press E again at the post to carry the disabled automaton's inert shell.";
 case HJ_RELAY_TRUNK:return "Use E at your stopped registered car's marked trunk. The case travels with that registered model.";
 case HJ_RELAY_QUAY:return !hj_own(1)||nv.phone[HJ_FLOAT]!=(U)(cars[0].type+1)?
  "The loaded car changed or was wrecked. Return to the service alley and use E to reset the sealed shell.":
  "Park that car beside the cargo inspection point, exit, then press E at the quay marker.";
 case HJ_RELAY_BUOY:return "Board a skiff at the cargo dock; use E at the marked floating survey receiver.";
 case HJ_BLUE_CLUE:return "Read the blue gantry at the cargo pier. It reveals a coded clue for a launch with a fictional contraband Starcoil.";
 case HJ_BLUE_FIND:return "Look for the blue cabin launch southwest of the cargo berth. Use E aboard a skiff for the boat-to-boat Starcoil exchange.";
 case HJ_BLUE_RETURN:return "The fictional Starcoil is filed with your case. Moor, walk to Harbor Office and use E to finish.";
 case HJ_BUMP_TAG:return "Take a skiff to the orange uncrewed trainer. A deliberate arcade bump releases one floating scoring chip.";
 case HJ_BUMP_LOOT:return "E at the floating chip for a $45 bonus, or return to Harbor Office for the base commission.";
 case HJ_BUMP_RETURN:return "Return to Harbor Office on foot to file your training result and claim the chip bonus.";
 default:return "Open the Signal Night Wire to choose a harbor commission.";
 }}
static void harbor_jobs_card(int row,char *label,char *suffix,char *detail){
 row=cl(row,0,HJ_CANCEL);append(label,hj_name[row]);
 int phase=hj_phase(),task=hj_case(phase);
 if(row==HJ_CANCEL){append(suffix,task>=0?"RESET":"NO ACTIVE CASE");append(detail,"Select twice to release the commission and clear any mission-only cargo. Your other inventory and jobs stay untouched.");return;}
 if(hj_complete(row)){append(suffix,"FILED");append(detail,"This commission's clean payout was collected. The other Harbor Office cards are independent.");return;}
 if(task==row){append(suffix,"IN PROGRESS");append(detail,hj_step_desc(phase));return;}
 number(append(suffix,"$"),hj_score[row]);
 append(detail,row==HJ_RELAY?"Disable the alley's nonhuman security automaton, carry its inert shell to your registered trunk, then take it to the survey receiver.":
   row==HJ_BLUE?"Read a clue, meet a hidden blue launch boat to boat, then file its fictional contraband Starcoil at Harbor Office.":
   "Bump an uncrewed training float, optionally take its scoring chip, and return to Harbor Office. Bonus $45 if collected.");
}
static int hj_raw_goal(int*x,int*y){
 int phase=hj_phase(),boat=harbor_boat_active();NavPoint q={0,0};
 switch(phase){
 case HJ_RELAY_TAG:q=hj_automaton();break;
 case HJ_RELAY_LIFT:q=hj_alley;break;
 case HJ_RELAY_TRUNK:q=hj_own(0)?hj_rear():hj_office;break;
 case HJ_RELAY_QUAY:q=hj_own(1)&&nv.phone[HJ_FLOAT]==(U)(cars[0].type+1)?hj_quay:hj_alley;break;
 case HJ_RELAY_BUOY:q=boat?hj_water(HJ_RELAY):hj_dock_walk(2);break;
 case HJ_BLUE_CLUE:q=hj_cue;break;
 case HJ_BLUE_FIND:
  q=boat?hj_water(HJ_BLUE):hj_dock_walk(2);
  if(boat&&!hj_near(q,240))return 0;
  break;
 case HJ_BLUE_RETURN:case HJ_BUMP_RETURN:q=boat?hj_dock_walk(1):hj_office;break;
 case HJ_BUMP_TAG:q=boat?hj_trainer():hj_dock_walk(2);break;
 case HJ_BUMP_LOOT:q=boat?hj_drift():hj_office;break;
 default:return 0;
 }
 *x=q.x;*y=q.y;return 1;
}
static int harbor_jobs_route(int*x,int*y){return hj_raw_goal(x,y);}
static void harbor_jobs_phone(int row){
 int phase=hj_phase(),task=hj_case(phase),*arm=hj_panel?&hj_confirm:&np_confirm;
 if(row==HJ_CANCEL){
  if(task<0){hj_msg("No harbor commission active.");return;}
  if(!*arm){*arm=1;hj_msg("Cancel and unload all mission-only harbor cargo? Enter again confirms.");return;}
  *arm=0;nv.phone[HJ_FLOAT]=0;hj_save_phase(HJ_IDLE,0,0);
  hj_msg(saveFlash<0?"Save failed; F5 retries the cancellation.":"Harbor assignment released. Start another case at any time.");return;
 }
 if(row<0||row>=HJ_CANCEL)return;
 if(hj_complete(row)){hj_msg("This commission is already filed.");return;}
 if(task==row){hj_msg(hj_step_desc(phase));return;}
 if(task>=0){hj_msg("Finish or cancel the current Harbor Office assignment first.");return;}
 if(hj_other_job()){hj_msg("Finish or cancel the current timed city job first.");return;}
 if(row==HJ_RELAY&&!hj_own(1)){hj_msg("Relay recovery requires a working registered car. Nightglass Motors is your starting garage.");return;}
 if(!*arm){*arm=1;hj_msg("Accept this Harbor Office commission? Press Enter again to sign on.");return;}
 *arm=0;nv.phone[HJ_FLOAT]=0;int next=row==HJ_RELAY?HJ_RELAY_TAG:row==HJ_BLUE?HJ_BLUE_CLUE:HJ_BUMP_TAG;
 hj_save_phase(next,0,0);hj_msg(saveFlash<0?"Save failed; F5 retries.":hj_step_desc(next));
}
static void harbor_jobs_tick(float dt){
 if(dt<=0||!hj_open_world())return;
 if(hj_phase()==HJ_RELAY_TAG&&!harbor_boat_active()&&p.car<0&&hj_near(hj_automaton(),61)){
  if(hj_interference>0){hj_interference-=dt;if(hj_interference<0)hj_interference=0;}
  else if((hj_attention+=dt)>=2.25f){
   hj_attention=0;hj_interference=.68f;
   tell("Security automaton / Arcade interference burst. Its E prompt returns in a moment.");
  }
 }else hj_attention=hj_interference=0;
 if(!harbor_boat_active()||hj_phase()!=HJ_BUMP_TAG)return;
 NavPoint q=hj_trainer();
 if(harbor_boat_hit(q.x,q.y,17.f,45.f)){
  nv.phone[HJ_FLOAT]=hj_pack(q);
  hj_save_phase(HJ_BUMP_LOOT,"Orange trainer tagged. E at the floating scoring chip for an optional bonus.",1);
 }
}
static int harbor_jobs_use(U tap){
 if(!(tap&USE)||!hj_open_world())return 0;
 int phase=hj_phase();
 if(harbor_boat_active()){
  if(phase==HJ_RELAY_BUOY&&hj_near(hj_water(HJ_RELAY),38)){nv.phone[HJ_FLOAT]=0;return hj_finish(HJ_RELAY,hj_score[HJ_RELAY]);}
  if(phase==HJ_BLUE_FIND&&hj_near(hj_water(HJ_BLUE),34))return hj_save_phase(HJ_BLUE_RETURN,"Boat-to-boat Starcoil exchange recorded. Return to Harbor Office.",1);
  if(phase==HJ_BUMP_LOOT&&hj_near(hj_drift(),36))return hj_save_phase(HJ_BUMP_RETURN,"Scoring chip recorded. Return to Harbor Office for the bonus.",1);
  return 0;
 }
 if(p.car>=0)return 0;
 switch(phase){
 case HJ_RELAY_TAG:
  if(hj_near(hj_automaton(),26)){
   if(hj_interference>0){tell("Arcade interference. Wait for the E prompt.");return 1;}
   hj_attention=hj_interference=0;
   return hj_save_phase(HJ_RELAY_LIFT,"Automaton disabled. Press E again to carry its inert shell.",1);
  }break;
 case HJ_RELAY_LIFT:if(hj_near(hj_alley,26))return hj_save_phase(HJ_RELAY_TRUNK,"Relay case carried. Walk to the marked trunk of your registered car.",1);break;
 case HJ_RELAY_TRUNK:
  if(hj_own(0)&&hj_near(hj_rear(),30)){
   if(!hj_own(1)||absf(cars[0].s)>8.f){tell("Harbor / The registered car must be working and stopped.");return 1;}
   nv.phone[HJ_FLOAT]=(U)(cars[0].type+1);
   return hj_save_phase(HJ_RELAY_QUAY,"Relay loaded in the marked trunk. Drive to the cargo quay and stop.",1);
  }break;
 case HJ_RELAY_QUAY:
  if(hj_near(hj_quay,29)){
   if(!hj_own(1)||!nv.phone[HJ_FLOAT]||(int)nv.phone[HJ_FLOAT]!=cars[0].type+1||absf(cars[0].s)>8.f||d2(cars[0].x-hj_quay.x,cars[0].y-hj_quay.y)>87*87){
    tell("Harbor / Bring the registered car that holds the case to the quay.");return 1;}
   nv.phone[HJ_FLOAT]=0;
   return hj_save_phase(HJ_RELAY_BUOY,"Relay transferred to the skiff registry. Board at the nearby cargo dock.",1);
  }
  /* A different/disabled registered model can release the cargo back to
   * the scripted alley. This is a reset, not payout or an inventory grant. */
  if(hj_near(hj_alley,24)&&(!hj_own(1)||nv.phone[HJ_FLOAT]!=(U)(cars[0].type+1))){
   nv.phone[HJ_FLOAT]=0;return hj_save_phase(HJ_RELAY_LIFT,"Relay recovered to its alley stand. E to carry it again.",1);}
  break;
 case HJ_BLUE_CLUE:
  if(hj_near(hj_cue,28))return hj_save_phase(HJ_BLUE_FIND,"Blue gantry: find the crescent insignia on a cabin launch southwest of the cargo berth.",1);
  break;
 case HJ_BLUE_RETURN:if(hj_near(hj_office,30))return hj_finish(HJ_BLUE,hj_score[HJ_BLUE]);break;
 case HJ_BUMP_LOOT:case HJ_BUMP_RETURN:
  if(hj_near(hj_office,30))return hj_finish(HJ_BUMP,hj_score[HJ_BUMP]+(phase==HJ_BUMP_RETURN?45:0));break;
 default:break;
 }
 if(hj_near(hj_office,29)){
  hj_panel=1;hj_row=hj_case(phase)>=0?hj_case(phase):0;hj_confirm=0;hj_msg("Choose a case, then Enter twice to accept it.");return 1;
 }
 return 0;
}
static void harbor_jobs_panel_tick(U keys,U tap){
 (void)keys;
 if(tap&ESC){harbor_jobs_reset();return;}
 if(tap&SAVE)request_save();
 if(tap&(UP|DOWN)){hj_row=(hj_row+HJ_NCARD+((tap&UP)?-1:1))%HJ_NCARD;hj_confirm=0;hj_note[0]=0;return;}
 if(tap&(ENTER|USE))harbor_jobs_phone(hj_row);
}
static const U hj_gold=0xffc683,hj_blue=0x89cee8,hj_buoy=0xffa679;
static void hj_buoy_art(int x,int y,U c,int style){
 if(x<-23||x>W+23||y<-24||y>H+25)return;
 for(int i=0;i<3;i++){
  int d=13+i*5+(int)(anim*2)%4;
  line(x-d,y+4+i*4,x-d+9,y+4+i*4,0x406570);
  line(x+d-9,y+4+i*4,x+d,y+4+i*4,0x406570);
 }
 if(style==1){
  tri(x-18,y+8,x+17,y+8,x,y-21,0x213a52);
  box(x-11,y-8,23,18,0x568db0,0x9ed5ea);box(x-7,y-8,15,11,0x296481,c);
  rect(x-5,y-6,5,4,0x96d9f2);rect(x+2,y-6,5,4,0xb4e8f2);
  line(x-11,y+10,x+11,y+10,c);rect(x-4,y+4,9,3,0xbcadd7);
  disc(x+1,y-17,4,0xc2edff);disc(x+3,y-19,4,0x213a52);
  return;
 }
 if(style==2){
  tri(x-17,y+6,x,y-17,x+18,y+5,0x29424a);box(x-10,y-6,21,12,0xa66c55,0x2b2f39);
  rect(x-6,y-7,11,5,c);line(x,y-10,x,y-20,c);disc(x,y-20,2,c);return;
 }
 if(style==3){diamond(x,y-2,8,0x425f67);diamond(x,y-5,5,c);dot(x,y-5,PAPER);return;}
 disc(x,y+2,8,0x2b464f);disc(x,y-2,7,c);rect(x-3,y-4,7,5,0x1d3642);rect(x-1,y-16,3,12,c);
 disc(x,y-16,2,(int)(anim*3)%2?PAPER:c);
}
static void hj_land_art(NavPoint q,int kind){
 int x=sx(q.x),y=sy(q.y);if(x<-62||x>W+65||y<-55||y>H+46)return;
 if(kind==0){box(x-41,y-39,83,22,0x1b3039,hj_blue);center(x,y-39,"HARBOR OFFICE",hj_blue,0);
  box(x-18,y-16,36,12,0x685646,0x9b896a);rect(x-14,y-14,11,4,0x91a2a3);
  person(x+24,y-3,0x718da0,0,1.57f,1,2);return;}
 if(kind==1){
  int disabled=hj_phase()==HJ_RELAY_LIFT;
  if(!disabled){
   int beat=((int)(anim*9))&3;
   disc(x,y+2,13,0x263942);
   line(x-6,y-6,x-11-(beat&1),y,0x98afaa);line(x+6,y-6,x+10+(beat&1),y-1,0x98afaa);
   box(x-8,y-11,17,13,0x506877,0x758f97);rect(x-4,y-8,8,2,hj_gold);
   box(x-7,y-23,15,11,0x364f61,0xaaa9a2);rect(x-4,y-20,9,3,hj_interference>0?0xffa68c:beat==0?PAPER:hj_gold);
   rect(x-6,y+1,4,4+(beat&1),0x657a7d);rect(x+2,y+1,4,5-(beat&1),0x657a7d);
   line(x+5,y-23,x+7,y-28,0x799b9d);dot(x+7,y-28,hj_blue);
  }else{
   disc(x,y+2,12,0x263942);box(x-11,y-5,22,10,0x445b66,hj_blue);
   box(x-15,y-5,9,8,0x3a4b60,0x7e8e98);dot(x-13,y-2,0x8db3c1);
  }
 }else if(kind==2){
  box(x-16,y-20,33,20,0x28495f,hj_blue);line(x-12,y-10,x+12,y-10,0xbcdcda);disc(x,y-10,5,0x7196ad);disc(x+2,y-12,5,0x28495f);
 }else{
  for(int j=0;j<4;j++){rect(x-18,y-3+j*2,36,1,j&1?hj_gold:0x4e5149);}
  box(x+11,y-24,12,15,0x354d5d,hj_blue);rect(x+14,y-19,6,2,hj_blue);
 }
}
static void harbor_jobs_world_paint(void){
 if(cx+W<2300||cy+H<1980)return;
 int ph=hj_phase();hj_land_art(hj_office,0);
 if(ph==HJ_RELAY_TAG||ph==HJ_RELAY_LIFT)hj_land_art(ph==HJ_RELAY_TAG?hj_automaton():hj_alley,1);
 if(ph==HJ_BLUE_CLUE)hj_land_art(hj_cue,2);
 if(ph==HJ_RELAY_QUAY)hj_land_art(hj_quay,3);
 if(ph==HJ_RELAY_TRUNK&&p.car<0&&!harbor_boat_active()){
  int px=sx(p.x),py=sy(p.y);box(px-6,py-26,13,8,0x4a5868,hj_gold);rect(px-3,py-24,6,1,hj_blue);
  if(hj_own(0)){NavPoint r=hj_rear();int bx=sx(r.x),by=sy(r.y);diamond(bx,by,8,hj_gold);disc(bx,by,2,hj_blue);}
 }
 if(ph==HJ_RELAY_BUOY){NavPoint q=hj_water(HJ_RELAY);hj_buoy_art(sx(q.x),sy(q.y),hj_gold,0);}
 if(ph==HJ_BLUE_FIND){NavPoint q=hj_water(HJ_BLUE);if(hj_near(q,265))hj_buoy_art(sx(q.x),sy(q.y),hj_blue,1);}
 if(ph==HJ_BUMP_TAG){NavPoint q=hj_trainer();hj_buoy_art(sx(q.x),sy(q.y),hj_buoy,2);}
 if(ph==HJ_BUMP_LOOT){NavPoint q=hj_drift();hj_buoy_art(sx(q.x),sy(q.y),hj_gold,3);}
}
static const char *hj_hot_hint(void){
 int ph=hj_phase();
 if(harbor_boat_active()){
  if(ph==HJ_RELAY_BUOY&&hj_near(hj_water(HJ_RELAY),41))return "E  /  SURVEY RECEIVER";
  if(ph==HJ_BLUE_FIND&&hj_near(hj_water(HJ_BLUE),38))return "E  /  BLUE LAUNCH RENDEZVOUS";
  if(ph==HJ_BUMP_LOOT&&hj_near(hj_drift(),40))return "E  /  FLOATING SCORING CHIP";
  if(ph==HJ_BUMP_TAG&&hj_near(hj_trainer(),154))return "ORANGE TRAINER / ARCADE BUMP";
  return 0;
 }
 if(p.car>=0)return 0;
 if(ph==HJ_RELAY_TAG&&hj_near(hj_automaton(),32))return hj_interference>0?"ARCADE INTERFERENCE  /  WAIT":"E  /  DISABLE AUTOMATON";
 if(ph==HJ_RELAY_LIFT&&hj_near(hj_alley,32))return "E  /  CARRY INERT SHELL";
 if(ph==HJ_RELAY_TRUNK&&hj_own(0)&&hj_near(hj_rear(),35))return "E  /  LOAD REGISTERED TRUNK";
 if(ph==HJ_RELAY_QUAY&&hj_near(hj_quay,36))return "E  /  TRANSFER TO SKIFF REGISTRY";
 if(ph==HJ_BLUE_CLUE&&hj_near(hj_cue,34))return "E  /  READ THE BLUE GANTRY";
 if(hj_near(hj_office,35))return ph==HJ_BLUE_RETURN||ph==HJ_BUMP_LOOT||ph==HJ_BUMP_RETURN?"E  /  FILE HARBOR COMMISSION":"E  /  HARBOR OFFICE";
 return 0;
}
static void harbor_jobs_hint(void){
 if(hj_panel){
  box(164,73,632,391,0x132734,0x668a94);rect(165,74,630,3,hj_blue);
  textat(186,84,"PORT VANTA / PIER SERVICES",hj_blue,0);textat(185,105,"Harbor Office",PAPER,1);
  textat(186,148,"COMMISSION BOARD / SAME CASES AS SIGNAL NIGHT WIRE",MUTED,0);
  char label[94],status[60],desc[174],note[130];const char*msg=hj_note;
  for(int i=0;i<HJ_NCARD;i++){
   label[0]=status[0]=desc[0]=0;harbor_jobs_card(i,label,status,desc);
   int y=181+i*34,selected=i==hj_row;box(185,y,588,28,selected?0x344d57:0x1e3740,selected?hj_gold:0x38616d);
   textat(198,y+3,label,selected?hj_gold:PAPER,0);textat(758-textw(status,0),y+3,status,selected?hj_gold:hj_blue,0);
  }
  /* Rebuild selected detail, never retain a prior loop's buffer. */
  if(!*hj_note){label[0]=status[0]=desc[0]=0;harbor_jobs_card(hj_row,label,status,desc);msg=desc;}
  rect(185,326,589,76,0x1b353e);
  for(int i=0;*msg&&i<3;i++){msg=hud_line(msg,553,note);textat(198,332+i*18,note,PAPER,0);}
  textat(185,424,"UP / DOWN  CHOOSE     E / ENTER  ACT     ESC  EXIT",hj_blue,0);
  return;
 }
 if(!hj_open_world())return;
 const char *hint=hj_hot_hint();if(!hint)return;int w=textw(hint,0)+26;
 int y=harbor_boat_active()?153:482;
 box((W-w)/2,y,w,26,0x142632,hj_gold);center(W/2,y+2,hint,hj_gold,0);
}
