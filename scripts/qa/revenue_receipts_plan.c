/* Disposable end-to-end input fixture for both launchers. Reuses the named
 * shop movement solver; opening/checkout and Signal panels are production UI. */
#define main patch_market_plan
#include "wheel_buy_plan.c"
#undef main
static void rr_probe(const char*label){
 const char*path=getenv("DC_RECEIPT_PROBES");if(!path)return;
 FILE*f=fopen(path,"a");assert(f);paint();
 U hash=2166136261u;for(int y=90;y<425;y++)for(int x=324;x<891;x++)hash=(hash^(pixels[y*W+x]&0xffffffu))*16777619u;
 fprintf(f,"%d %08x %s\n",count,hash,label);assert(!fclose(f));
}
int main(int argc,char**argv){
 assert(argc==2);set_session_seed(17);init_city();starter_save();
 sv.intro=4;sv.cash=49;sv.dirty=901;sv.bank=207;sv.dayClock=540.f;
 sv.pending=37;sv.clearT=29.f;sv.legit[0]=(LegalBusiness){.owned=1,.pending=42,.cooldown=900.f};
 sv.x=loc[9].x;sv.y=loc[9].y;sv.hash=savehash();
 FILE*f=fopen(argv[1],"wb");assert(f&&fwrite(&sv,sizeof sv,1,f)==1&&!fclose(f));
 profiles_activate();sent[count++]=0;sent[count++]=ENTER;
 for(int i=0;i<4;i++)step(0);
 hit(USE);assert(scene==3&&interior_active_loc==9);front();hit(USE);assert(menu==9);
 hit(ENTER);assert(sv.home&&sv.charges&&sv.cash==0&&sv.dirty==0&&sv.bank==207);
 RevenueLog order=rr_read(1),loss=rr_read(0);
 assert(order.kind==RR_PURCHASE&&order.clean==49&&order.dirty==501&&order.minute==(int)sv.dayClock);
 assert(loss.kind==REVENUE_THRESHOLD&&!loss.clean&&loss.dirty==400&&loss.pending==79);
 assert(!revenue_seizure_notice().kind); /* HUD consumed it, ledger retained it. */
 assert(rr_read(0).dirty==400);
 hit(ESC);
 navHasManual=1;navSlot=sideSlot;navManualLoc=2;navManual=(NavPoint){loc[2].x,loc[2].y};
 hit(PHONE);assert(scene==NIGHT_PHONE_SCENE&&np_app==NP_INBOX);
 hit(LEFT);assert(np_app==NP_PREF);hit(RIGHT);assert(np_app==NP_INBOX);
 hit(MAP);assert(np_app==NP_RECEIPTS&&np_row==2&&navManualLoc==2);
 int before=sv.sweepTicks;for(int i=0;i<6;i++)step(0);assert(sv.sweepTicks==before);
 NPLine l;np_line(&l,2);assert(!strcmp(l.suffix,"$180 CLEAN DUE")&&strstr(l.detail,"3 unpaid shifts"));
 rr_probe("current-case");
 hit(UP);assert(np_row==1);np_line(&l,1);assert(strstr(l.detail,"Dirty -$400")&&strstr(l.detail,"Bank $0")&&strstr(l.detail,"Unclaimed settlements -$79"));
 rr_probe("last-seizure");
 hit(UP);assert(np_row==0);np_line(&l,0);assert(strstr(l.detail,"Clean -$49")&&strstr(l.detail,"Dirty -$501"));
 rr_probe("major-purchase");
 hit(UP);assert(np_row==4);hit(ENTER);assert(navManualLoc==REV_ANNEX&&navHasManual&&navManual.x==loc[REV_ANNEX].x);
 rr_probe("annex-confirm");
 hit(MAP);assert(np_app==NP_INBOX&&navManualLoc==REV_ANNEX);
 hit(ESC);assert(scene==3&&navManualLoc==REV_ANNEX&&sv.sweepTicks<before);hit(SAVE);
 printf("%d 0\n",count);
 for(int i=0;i<count;){int j=i+1;while(j<count&&sent[j]==sent[i])j++;printf("%d %u\n",j-i,sent[i]);i=j;}
 /* Distinct boundary checks never influence native tape or seeded career. */
 starter_save();assert(!rr_read(0).kind&&!rr_read(1).kind);
 sv.cash=1000;assert(canDirtyPurchase(239)&&!rr_read(1).kind);
 assert(canDirtyPurchase(240)&&rr_read(1).clean==240);
 assert(!canDirtyPurchase(99999)&&rr_read(1).clean==240);
 revenue_open_case();assert(!canDirtyPurchase(400)&&rr_read(1).clean==240);
 sv.cash=WALLET_LIMIT;sv.dirty=WALLET_LIMIT;sv.sweepTicks=1;revenue_tick();
 loss=rr_read(0);assert(loss.kind==REVENUE_SWEEP&&loss.clean==WALLET_LIMIT&&loss.dirty==WALLET_LIMIT);
 sv.sweepTicks=1;revenue_tick();assert(rr_read(0).kind==REVENUE_SWEEP);
 nv.garage[RR_PURCHASE_DIRTY]=~0u;assert(!rr_read(1).kind);
 nv.phone[DC_SEIZURE_META]|=1u<<13;assert(!rr_read(0).kind);
 revenue_close_case();rrc_open();np_line(&l,2);assert(!strcmp(l.suffix,"CLEAR"));
 np_line(&l,3);assert(!strcmp(l.suffix,"NONE"));
 pointer_input(RR_SIGNAL_X+3,RR_SIGNAL_Y+4,1);assert(np_app==NP_RECEIPTS);
}
