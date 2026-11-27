/* Port Vanta franchises: clean passive income and a one-button fictional
 * credit exchange, settled only by collecting at the same rear account desk.
 * Funds here are units in the game; no other accounting is simulated. */
enum {
 LEGAL_MENU_FIRST=16,LEGAL_REAR_FIRST=19,
 LEGAL_OPEN=100,LEGAL_BUY_FIRST=101,LEGAL_DEPOSIT_FIRST=104,LEGAL_COUNTER_BACK=107,
 LEGAL_CLAIM_FIRST=108,
 LEGAL_INTERVAL=120,LEGAL_MIN_WINDOW=45,LEGAL_MIN_TRANSFER=5
};
typedef struct {
 int kind,cost,payout,limit;
 const char *name,*buyNote;
} LegalBusinessSpec;
static const LegalBusinessSpec legal_specs[]={
 {0,360,24,220,"MERCURY COURIER FRANCHISE","Buy with clean credits. Earn $24 clean every 2 minutes of active play."},
 {2,710,36,320,"FERRY MARKET FRANCHISE","Buy with clean credits. Earn $36 clean every 2 minutes of active play."},
 {10,1200,49,480,"WRECKER RECOVERY FRANCHISE","Buy with clean credits. Earn $49 clean every 2 minutes of active play."}
};
static int legal_index_for_kind(int kind){
 for(int i=0;i<LEN(legal_specs);i++)if(legal_specs[i].kind==kind)return i;
 return -1;
}
static int legal_menu_index(int kind){
 int i=kind-LEGAL_MENU_FIRST;
 return i>=0&&i<LEN(legal_specs)?i:-1;
}
static int legal_rear_index(int kind){
 int i=kind-LEGAL_REAR_FIRST;
 return i>=0&&i<LEN(legal_specs)?i:-1;
}
static int legal_seconds(float value){
 return value>0.f?(int)value+((float)(int)value<value):0;
}
static char*legal_duration(char*label,float value){
 int seconds=legal_seconds(value);
 label=number(label,seconds/60);*label++=':';if(seconds%60<10)*label++='0';
 return number(label,seconds%60);
}
static int legal_amount(int i){return min(sv.dirty,legal_specs[i].limit);}
static int legal_net(int amount){return amount-((amount+3)/4);}
static void legal_make_menu(int i){
 if(i<0||i>=LEN(legal_specs))return;
 const LegalBusinessSpec*s=legal_specs+i;
 const LegalBusiness*b=sv.legit+i;
 if(!b->owned){
  if(sv.charges)add("FRANCHISE BUYING ON HOLD","Resolve the current Revenue case before buying property.",0);
  else priced("BUY FRANCHISE",s->cost,s->buyNote,LEGAL_BUY_FIRST+i);
 }else{
  char*status=append(strings[nopt],"NEXT CLEAN PAYOUT  $");
  status=number(status,s->payout);status=append(status," IN ");legal_duration(status,LEGAL_INTERVAL-b->elapsed);
  add_colored(strings[nopt],"Earned automatically as you play. The payout clock is saved.",0,CASH_GREEN);
  add("REAR ACCOUNT DESK","Walk into the staff room to deposit or collect game credits.",0);
 }
 add("RETURN TO COUNTER","Back to regular services at this venue.",LEGAL_COUNTER_BACK);
}
static void legal_counter_menu(int i){
 if(i<0||i>=LEN(legal_specs))return;
 const LegalBusiness*b=sv.legit+i;
 if(!b->owned)add("OWNER ACCESS ONLY","Buy this franchise from the public counter first.",0);
 else if(sv.charges)add("ACCOUNT ON HOLD","Resolve the Revenue case to use the account desk.",0);
 else if(b->pending>0){
  char*label;
  if(b->cooldown>0.f){
   label=append(strings[nopt],"COLLECT $");label=number(label,b->pending);label=append(label," IN ");legal_duration(label,b->cooldown);
   add_colored(strings[nopt],"Return to this desk after closing to collect. No automatic payout.",0,CASH_GREEN);
  }else{
   label=append(strings[nopt],"COLLECT $");label=number(label,b->pending);append(label," CLEAN");
   add_colored(strings[nopt],"Your deposit has cleared. Claim these credits here.",LEGAL_CLAIM_FIRST+i,CASH_GREEN);
  }
 }else{
  const int amount=legal_amount(i);
  char*label=append(strings[nopt],"TODAY CLOSES IN ");legal_duration(label,DAY_SECONDS-sv.dayClock);
  add(strings[nopt],"Deposits made in the final 45 seconds clear at tomorrow's closing.",0);
  if(amount>=LEGAL_MIN_TRANSFER){
   label=append(strings[nopt],"DEPOSIT $");label=number(label,amount);append(label," DIRTY MONEY");
   char*note=append(strings[LEN(strings)-1],"25% game fee, rounded up. Collect $");note=number(note,legal_net(amount));append(note," clean at this desk after closing.");
   add_colored(strings[nopt],strings[LEN(strings)-1],LEGAL_DEPOSIT_FIRST+i,CASH_RED);
  }else add_colored("DEPOSIT DIRTY MONEY","Bring at least $5 in dirty money. Only one deposit per desk at a time.",0,CASH_RED);
 }
 add("LEAVE DESK","Close the account and keep walking.",LEGAL_COUNTER_BACK);
}
static int legal_opt(int action,int*currentMenu){
 if(action==LEGAL_OPEN){
  int idx=legal_index_for_kind(*currentMenu);
  if(idx>=0)active(LEGAL_MENU_FIRST+idx);
  return 1;
 }
 if(action<LEGAL_BUY_FIRST||action>=LEGAL_CLAIM_FIRST+LEN(legal_specs))return 0;
 const int front=legal_menu_index(*currentMenu),rear=legal_rear_index(*currentMenu);
 const int i=front>=0?front:rear;
 if(i<0)return 1;
 const LegalBusinessSpec*s=legal_specs+i;LegalBusiness*b=sv.legit+i;
 if(action==LEGAL_COUNTER_BACK){
  if(front>=0)active(s->kind);else *currentMenu=-1;
  return 1;
 }
 if((front>=0&&action!=LEGAL_BUY_FIRST+i)||(rear>=0&&action!=LEGAL_DEPOSIT_FIRST+i&&action!=LEGAL_CLAIM_FIRST+i))return 1;
 if(sv.charges){tell("Franchise transaction blocked by Revenue. Visit the Annex.");return 1;}
 if(front>=0){
  if(b->owned){tell("This franchise is already yours.");return 1;}
  if(sv.cash<s->cost){tell("Not enough clean credits to buy this franchise.");return 1;}
  if(!spend(s->cost))return 1;
  *b=(LegalBusiness){.owned=1};
  tell("Franchise acquired. The rear staff room is now open.");
  request_save();
  return 1;
 }
 if(!b->owned){tell("Buy this franchise before using its account desk.");return 1;}
 if(action==LEGAL_CLAIM_FIRST+i){
  if(b->pending<=0||b->cooldown>0.f){tell("No settled deposit to collect at this desk.");return 1;}
  int paid=credit_wallet(1,b->pending);
  if(!paid){tell("Clean wallet full. Your account balance stays at this desk.");return 1;}
  b->pending-=paid;b->cooldown=0.f;
  cashNote(b->pending?"Partial collection; the rest stays at this desk.":"Account collected.",paid);
  request_save();
  return 1;
 }
 if(b->pending>0){tell("One deposit per desk. Collect this one first.");return 1;}
 const int amount=legal_amount(i);
 if(amount<LEGAL_MIN_TRANSFER){tell("Bring at least $5 in dirty money to deposit.");return 1;}
 sv.dirty-=amount;b->pending=legal_net(amount);b->cooldown=DAY_SECONDS-sv.dayClock;
 if(b->cooldown<LEGAL_MIN_WINDOW){
  b->cooldown+=DAY_SECONDS;tell("Deposit recorded. Collect here after tomorrow's closing.");
 }else tell("Deposit recorded. Collect here after the end of the game day.");
 request_save();
 return 1;
}
static void legal_seize(void){
 for(int i=0;i<LEN(legal_specs);i++){sv.legit[i].pending=0;sv.legit[i].cooldown=0.f;}
}
static float legal_advance(float elapsed,float dt){
 /* Native play uses fixed 60 Hz ticks; keep that counter frame-aligned so
  * the day closes exactly on frame 86,400, including after save. */
 if(absf(dt-1.f/60.f)<.000001f)return (float)((int)(elapsed*60.f+.5f)+1)/60.f;
 return elapsed+dt;
}
/* The game clock does not advance offline. Rather than letting a floating
 * countdown drift away from closing, peg it to the same day boundary. */
/* Preserve the predecessor's numeric stamps so saves still round-trip.
 * Civil midnight, including sleep, controls both daily rewards. A playtime
 * boundary which is *not* midnight carries today's claims forward. */
static int daily_rewards_tick(int closed,float dt);
static int legal_tick_mode(float dt,int active_work){
 if(!(dt>0.f))return 0;
 const float previousEnd=DAY_SECONDS-sv.dayClock,clock=legal_advance(sv.dayClock,dt);
 const int closed=(int)(clock/DAY_SECONDS);
 sv.dayClock=clock-closed*DAY_SECONDS;
 int reset=daily_rewards_tick(closed,dt)||(closed&&sv.dirtySpent>0);
 if(closed)sv.dirtySpent=0;
 int income=0,ready=0;
 for(int i=0;i<LEN(legal_specs);i++){
  LegalBusiness*b=sv.legit+i;
  if(!b->owned)continue;
  if(b->pending>0&&b->cooldown>0.f){
   const int closings=b->cooldown>previousEnd+DAY_SECONDS*.5f?2:1;
   if(closed>=closings){b->cooldown=0.f;ready++;}
   else b->cooldown=(closings-closed)*DAY_SECONDS-sv.dayClock;
  }
  /* Civil midnight still matures a deposit while the owner sleeps. But
   * franchises promise income for active play, not for repeated naps. */
  if(!active_work)continue;
  b->elapsed=legal_advance(b->elapsed,dt);
  if(b->elapsed>=LEGAL_INTERVAL){
   int periods=(int)(b->elapsed/LEGAL_INTERVAL);
   b->elapsed-=periods*LEGAL_INTERVAL;
   income+=periods*legal_specs[i].payout;
  }
 }
 if(income)income=credit_wallet(1,income);
 if(p.noticeT<.5f){
  if(ready)tell("Account cleared. Collect your credits at the same rear desk.");
  else if(income)cashNote("Clean franchise income.",income);
 }
 return income>0||ready>0||reset;
}

static int legal_tick(float dt){return legal_tick_mode(dt,1);}
