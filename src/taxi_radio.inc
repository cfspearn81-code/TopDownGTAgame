/* Temporary dispatch menu, independent of Cab Rank/F6 and save layout.
 * Opening a quote only reads routes and cash; only accepting starts a job. */
enum {TR_CLOSED,TR_QUOTE,TR_RECEIPT};
typedef struct{int trips,base,tip,bonus,credit,bumps,ended;} TaxiReceipt;
static TaxiOffer taxiQuote;
static TaxiReceipt taxiReceipt;
static int taxiRadio,taxiToPickup;
static void modal(int x,int y,int w,int h,U c);
static void taxi_reset_radio(void){
 taxiRadio=TR_CLOSED;taxiReceipt=(TaxiReceipt){0};
}
static void taxi_sum(int *total,int amount){*total+=min(amount,WALLET_LIMIT-*total);}
static void taxi_start_receipt(void){
 if(taxiReceipt.ended)taxiReceipt=(TaxiReceipt){0};
}
static void taxi_record_ride(int base,int tip,int bonus,int credit,int bumps){
 taxi_start_receipt();
 taxi_sum(&taxiReceipt.trips,1);taxi_sum(&taxiReceipt.base,base);taxi_sum(&taxiReceipt.tip,tip);
 taxi_sum(&taxiReceipt.bonus,bonus);taxi_sum(&taxiReceipt.credit,credit);taxi_sum(&taxiReceipt.bumps,bumps);
}
static int taxi_radio_input(U tap){
 if(taxiRadio==TR_QUOTE){
  if(tap&(ESC|TAXI_QUOTE)){
   taxiSeed=taxiQuote.after;taxiRadio=TR_CLOSED;tell("Radio call declined. No fare was started.");
  }else if(tap&(ENTER|USE|TAXI_NEXT)){
   taxiRadio=TR_CLOSED;
   if(taxi_call_allowed()){taxi_commit_call(&taxiQuote);taxi_dispatch_accepted();}
  }
  return 1;
 }
 if(taxiRadio==TR_RECEIPT){
  if(tap&(ESC|TAXI_RECEIPT))taxiRadio=TR_CLOSED;
  else if((tap&ENTER)&&taxiReceipt.trips&&!taxiReceipt.ended&&job.kind!=JOB_TAXI){
   taxiReceipt.ended=1;tell("Taxi shift ended. Every completed ride was already paid.");
  }
  return 1;
 }
 if(!(tap&(TAXI_QUOTE|TAXI_RECEIPT))||scene!=1||menu>=0||mapOn||p.dead||atmOpen>=0)return 0;
 if(p.heat>0.f){tell("Lose the pursuit before opening taxi radio.");return 0;}
 if(p.car>=0&&absf(cars[p.car].s)>=5.f){
  tell("Stop safely before checking taxi radio. T still quick-accepts.");return 0;
 }
 if(tap&TAXI_RECEIPT){taxiRadio=TR_RECEIPT;return 1;}
 if(!taxi_call_allowed())return 1;
 if(!taxi_draft(&taxiQuote)){tell("No nearby fare available yet.");return 1;}
 taxiToPickup=taxi_dispatch_distance((int)p.x,(int)p.y,taxiQuote.job.xs[0],taxiQuote.job.ys[0]);
 taxiRadio=TR_QUOTE;return 1;
}
static void taxi_receipt_row(int y,const char*label,int amount,int cash,U tint){
 char value[24];number(append(value,cash?"$":""),amount);
 textat(236,y,label,PAPER,0);textat(722-textw(value,0),y,value,tint,0);
}
static void taxi_draw_radio(void){
 if(!taxiRadio)return;
 modal(212,101,536,320,TEAL);
 const TaxiReceipt *s=&taxiReceipt;
 if(taxiRadio==TR_QUOTE){
  const Taxi *t=&taxiQuote.taxi;char b[96],*q;
  textat(236,120,"MERCURY / RADIO OFFER",TEAL,1);
  q=append(b,riders[t->rider].name);append(q," / WAITING FOR PICKUP");textat(236,163,b,GOLD,0);
  textat(236,191,"FROM",MUTED,0);textat(310,191,taxi_stop_label(t->pickup),PAPER,0);
  textat(236,217,"TO",MUTED,0);textat(310,217,taxi_stop_label(t->dropoff),PAPER,0);
  if(taxiToPickup){q=number(append(b,"TO PICKUP  ~"),taxiToPickup);append(q," m");}
  else append(b,"TO PICKUP  /  GPS WHEN ACCEPTED");
  textat(236,247,b,MUTED,0);
  q=number(append(b,"WITH RIDER  ~"),t->meters);append(q," m");textat(236,270,b,MUTED,0);
  fmt(b,"BASE $",taxiQuote.job.price);textat(236,310,b,LIME,0);
  textat(390,310,"TIP EXTRA FOR SMOOTH DRIVING",MUTED,0);
  textat(236,368,"E / ENTER  ACCEPT",TEAL,0);textat(524,368,"ESC  DECLINE",PAPER,0);
  return;
 }
 textat(236,120,"MERCURY / TAXI RECEIPT",TEAL,1);
 if(!s->trips){
  if(job.kind==JOB_TAXI){
   textat(236,177,"FIRST FARE OF THIS SHIFT IN PROGRESS",PAPER,0);
   textat(236,207,"Your receipt updates when the rider gets out.",MUTED,0);
  }else{
   textat(236,177,"NO COMPLETED FARES THIS SHIFT",PAPER,0);
   textat(236,207,"Take a fare at Mercury Dispatch, or T",MUTED,0);
   textat(236,229,"from a licensed car after your first drop-off.",MUTED,0);
  }
 }else{
  textat(236,159,s->ended?"SHIFT ENDED / PAYMENTS COMPLETE":"CURRENT SHIFT / PAID AT EACH DROP-OFF",GOLD,0);
  taxi_receipt_row(184,"Completed fares",s->trips,0,PAPER);
  taxi_receipt_row(207,"Base fares earned",s->base,1,PAPER);
  taxi_receipt_row(230,"Tips earned",s->tip,1,PAPER);
  taxi_receipt_row(253,"Extra fare bonuses",s->bonus,1,PAPER);
  taxi_receipt_row(276,"Collisions with rider aboard",s->bumps,0,s->bumps?GOLD:PAPER);
  taxi_receipt_row(307,"Credited to clean cash",s->credit,1,LIME);
  if(!s->ended)textat(236,342,job.kind==JOB_TAXI?"Finish the current fare to end this shift.":"ENTER  END SHIFT (no additional payout)",MUTED,0);
 }
 textat(236,378,"ESC / R  BACK",TEAL,0);
}
