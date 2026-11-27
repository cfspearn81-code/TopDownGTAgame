/* Dedicated Signal shortcut, separate from the app ring so settings, radio and
 * accessibility replays keep their original pointer/keyboard coordinates. */
enum { RR_SIGNAL_X=76,RR_SIGNAL_Y=408,RR_SIGNAL_W=211,RR_SIGNAL_H=30,RRC_ROWS=5 };
static const U rrc_ink=0xf3bf93;
static int rrc_previous,rrc_previous_row;
static void rrc_open(void){
 if(np_app!=NP_RECEIPTS){rrc_previous=np_app;rrc_previous_row=np_row;}
 np_app=NP_RECEIPTS;np_row=sv.charges?2:0;np_confirm=0;np_note[0]=0;
}
static void rrc_back(void){
 np_app=cl(rrc_previous,0,NP_NAPP-1);np_row=cl(rrc_previous_row,0,np_rows()-1);np_confirm=0;np_note[0]=0;
}
static void rrc_line(NPLine*l,int r){
 char*q;RevenueLog stamp;
 if(r==0||r==1){
  stamp=rr_read(r==0);
  const char*title=r==0?"Last major purchase":"Last seizure";
  if(!stamp.kind){
   np_label(l,title,"NO RECORD",r==0?"Successful purchases of $240 or more appear here. Your earlier purchases are not reconstructed.":"The most recent seizure will appear here. Unchanged and protected money is not subtracted.");
   return;
  }
  char when[6];rr_clock_text(when,stamp.minute);
  np_label(l,title,"","");
  if(r==0){q=number(append(l->suffix,"$"),stamp.clean+stamp.dirty);append(append(q," / "),when);}
  else append(append(l->suffix,stamp.kind==REVENUE_SWEEP?"WALLET SWEEP / ":"CASE OPENED / "),when);
  q=rr_wallet_delta(l->detail,"Clean",stamp.clean);
  q=rr_wallet_delta(append(q,"  |  "),"Dirty",stamp.dirty);
  q=append(q,"  |  Bank $0.");
  if(stamp.pending)append(rr_wallet_delta(append(q," "),"Unclaimed settlements",stamp.pending),".");
  else if(r==1)append(q," Protected bank funds were not seized.");
  return;
 }
 if(r==2){
  np_label(l,"Case balance",sv.charges?"":"CLEAR","");
  if(sv.charges)append(number(append(l->suffix,"$"),REV_CASE_FINE)," CLEAN DUE");
  q=l->detail;
  if(sv.charges){
   q=number(append(q,"$"),REV_CASE_FINE);
   q=append(q," clean due at the Annex, or finish ");
   q=number(q,max(0,RESTITUTION_SHIFTS-sv.restitution));
   q=append(q," unpaid shifts. ");
  }else q=append(q,"Nothing due. ");
  q=number(append(q,"Dirty spending today: $"),sv.dirtySpent);
  append(number(append(q," / $"),REVENUE_DIRTY_LIMIT),".");
  return;
 }
 if(r==3){
  np_label(l,"Next seizure",sv.charges?"":"NONE",sv.charges?
     "While the case is open, wallet sweeps repeat every five active minutes. Reading Signal pauses the clock. Funds in the bank are protected.":
     "There is no scheduled sweep while your case is clear.");
  if(sv.charges)revenue_time(l->suffix);
  return;
 }
 np_label(l,"Pin Revenue Annex","ENTER TO PIN","Place a GPS pin on the Revenue Annex. Leave Signal to follow the route. Reading this page never changes your current GPS pin.");
}
static void rrc_apply(void){
 if(np_row!=RRC_ROWS-1)return;
 np_venue(REV_ANNEX);np_msg("Revenue Annex pinned. Close Signal to follow your GPS.");
}
