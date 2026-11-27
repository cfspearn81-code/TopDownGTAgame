/* Clean-cash ATM interface and nonblocking Revenue alert. Balance/transfer
 * policy is owned by the economy; these widgets never mutate the save. */
#define ATM_MINT 0x8cece0u
#define ATM_PANE 0x101f28u
#define ATM_RIM  0x3f6572u
typedef struct{int x,y;const char*label;} BankAtm;
static const BankAtm bankAtms[]={
 {388,723,"TIDE MARKET ATM"},
 {1622,467,"EAST FREIGHT ATM"},
 {598,1491,"LANTERN WARD ATM"}
};
enum{ATM_FRONT=13,ATM_REACH=26,SEIZURE_TICKS=264,SEIZURE_SHAKE=24};
static const int atmAmounts[]={1,10,25,50,100,250,500,1000,2500,5000,10000,25000};
static int atmOpen=-1,atmMode,atmStep,atmLastMove,atmRejected,assetAlert;
static RevenueReceipt assetSeizure;

static void bank_ui_reset(void){
 atmOpen=-1;atmMode=atmLastMove=atmRejected=assetAlert=0;assetSeizure=(RevenueReceipt){0};
 (void)revenue_seizure_notice(); /* Do not carry a pending event across careers. */
}
static int atm_near(void){
 if(p.car>=0)return -1;
 for(int i=0;i<LEN(bankAtms);i++){
  const BankAtm*a=bankAtms+i;int y=a->y+ATM_FRONT;
  if(d2(p.x-a->x,p.y-y)<=ATM_REACH*ATM_REACH&&
    street_line_open(p.x,p.y,a->x,y,4))return i;
 }
 return -1;
}
static void atm_place(void){
 for(int i=0;i<LEN(bankAtms);i++){
  const BankAtm*a=bankAtms+i;
  BlockScenery*b=scenery+a->y/256*8+a->x/256;
  if(b->count>=LEN(b->walls))continue;
  for(int j=b->count;j>b->treesFrom;j--)b->walls[j]=b->walls[j-1];
  b->walls[b->treesFrom++]=(StreetBounds){a->x-11,a->y-15,22,16};
  b->count++;
 }
}
static void atm_open(int i){
 atmOpen=i;atmMode=0;atmStep=4;atmLastMove=atmRejected=0;
}
static int atm_available(void){
 int balance=bank_balance();
 return atmMode?min(balance,WALLET_LIMIT-sv.cash):min(sv.cash,WALLET_LIMIT-balance);
}
static int atm_amount(void){
 int available=atm_available();
 return atmStep<LEN(atmAmounts)?min(atmAmounts[atmStep],available):available;
}
static void atm_adjust(int delta){
 /* Step between distinct displayed amounts rather than hidden preset
  * indices: the opening $100 step may already display a $9 MAX. A
  * single Left must move that $9 to $1, not show MAX three more times. */
 const int available=atm_available(),shown=atm_amount(),n=LEN(atmAmounts);
 if(delta>0){
  for(int i=0;i<n;i++)if(atmAmounts[i]>shown&&atmAmounts[i]<available){atmStep=i;return;}
  atmStep=n;
 }else{
  for(int i=n-1;i>=0;i--)if(atmAmounts[i]<shown){atmStep=i;return;}
  atmStep=0;
 }
}
static int bank_ui_tick(U tap){
 if(atmOpen<0)return 0;
 if(tap&(ESC|MAP)){atmOpen=-1;return 1;}
 if(tap&(UP|DOWN)){atmMode^=1;atmLastMove=atmRejected=0;}
 if(tap&(LEFT|RIGHT)){atm_adjust(tap&RIGHT?1:-1);atmLastMove=atmRejected=0;}
 if(tap&(ENTER|USE)){
  int amount=atm_amount();
  atmLastMove=amount?(atmMode?bank_withdraw(amount):bank_deposit(amount)):0;
  atmRejected=!atmLastMove;
 }
 return 1;
}
static void bank_warning_tick(void){
 RevenueReceipt receipt=revenue_seizure_notice();
 if(receipt.kind){assetSeizure=receipt;assetAlert=SEIZURE_TICKS;}
 else if(assetAlert>0)assetAlert--;
}
static int bank_warning_offset(void){
 int elapsed=SEIZURE_TICKS-assetAlert;
 return !fastGfx&&elapsed>=0&&elapsed<SEIZURE_SHAKE?
     (int)(sn(elapsed*.79f)*4*(SEIZURE_SHAKE-elapsed)/SEIZURE_SHAKE):0;
}
static const char *bank_alert_title(RevenueReceipt r){
 return r.clean||r.dirty||r.pending?"ASSETS SEIZED":"REVENUE NOTICE";
}
static void bank_warning_draw(void){
 if(!assetAlert||p.dead)return;
 int cx=W/2+bank_warning_offset(),x=cx-265,y=(H-122)/2;
 rect(x+5,y+5,530,122,0x24161c);box(x,y,530,122,0x211920,0xb85360);
 rect(x+1,y+1,528,3,CASH_RED);
 tri(cx-210,y+18,cx-227,y+49,cx-193,y+49,GOLD);
 rect(cx-212,y+28,4,11,0x422428);rect(cx-212,y+42,4,4,0x422428);
 center(cx+10,y+14,bank_alert_title(assetSeizure),PAPER,1);
 rect(x+24,y+62,482,1,0x75414d);
 center(cx,y+68,assetSeizure.kind==REVENUE_SWEEP?
     "Bank funds safe. Settle at Revenue Annex.":"Go to Revenue Annex to settle your debt.",GOLD,0);
 char s[70];
 if(assetSeizure.kind==REVENUE_SWEEP){
  char*q=s;
  if(assetSeizure.clean)q=number(append(q,"CLEAN -$"),assetSeizure.clean);
  if(assetSeizure.clean&&assetSeizure.dirty)q=append(q,"  /  ");
  if(assetSeizure.dirty)q=number(append(q,"DIRTY -$"),assetSeizure.dirty);
 }else if(assetSeizure.pending>0){
  char*q=s;if(assetSeizure.dirty){q=number(append(q,"DIRTY -$"),assetSeizure.dirty);q=append(q,"  /  ");}
  number(append(q,"PENDING -$"),assetSeizure.pending);
 }else if(assetSeizure.dirty>0){char*q=append(s,"DIRTY MONEY: -$");q=number(q,assetSeizure.dirty);append(q,"  /  CLEAN BANK SAFE");}
 else append(s,"NO FUNDS SEIZED  /  CLEAN BANK SAFE");
 center(cx,y+94,s,MUTED,0);
}
static void atm_kiosk(int x,int y){
 /* Cyan hood and bill slot read as a bank terminal even on a busy block. */
 rect(x-11,y+2,30,5,0x131e24);rect(x-10,y-1,22,3,0x68828c);
 box(x-11,y-25,23,26,0x243e4c,0x111e2a);
 rect(x-10,y-25,21,6,ATM_MINT);rect(x+9,y-19,3,19,0x3b5662);
 box(x-7,y-16,13,8,0x7bc7bb,0x102c36);
 rect(x-5,y-14,9,1,0xc0f1d8);rect(x-5,y-11,5,1,0x345862);
 rect(x-7,y-6,13,2,0x0c1922);rect(x-4,y-5,7,1,0x9aa393);
 rect(x-4,y-2,8,2,0x6b7f88);
}
static void atm_kiosk_hint(void){
 int i=atm_near();if(i<0||atmOpen>=0||menu>=0)return;
 const BankAtm*a=bankAtms+i;int x=a->x-cx,y=a->y-cy;if(chProjected)ch_project(&x,&y);
 if((unsigned)x>=W||(unsigned)y>=H)return;
 box(x-21,y-50,43,23,0x1b3640,ATM_MINT);
 center(x,y-49,"ATM",ATM_MINT,0);
}
static void atm_draw_balance(int x,int y,int w,const char*label,int value){
 char temp[24];fmt(temp,"$",value);
 box(x,y,w,60,0x152c36,ATM_RIM);textat(x+14,y+6,label,MUTED,0);
 textat(x+14,y+32,temp,CASH_GREEN,0);
}
static void atm_draw(void){
 char temp[76];const int x=(W-576)/2,y=(H-350)/2,w=576;
 rect(x+6,y+6,w,350,0x081219);box(x,y,w,350,ATM_PANE,ATM_RIM);
 rect(x+1,y+1,w-2,3,ATM_MINT);
 textat(x+19,y+16,"PORT VANTA BANK",ATM_MINT,0);
 textat(x+w-19-textw(bankAtms[atmOpen].label,0),y+16,bankAtms[atmOpen].label,MUTED,0);
 textat(x+18,y+41,"CASH TERMINAL",PAPER,1);
 atm_draw_balance(x+19,y+94,260,"CLEAN CASH ON HAND",sv.cash);
 atm_draw_balance(x+297,y+94,260,"BANK BALANCE",bank_balance());
 for(int i=0;i<2;i++){
  int bx=x+19+i*278,selected=atmMode==i;
  box(bx,y+173,260,34,selected?0x20414c:0x172b35,selected?ATM_MINT:ATM_RIM);
  textat(bx+14,y+180,i?"WITHDRAW CLEAN CASH":"DEPOSIT CLEAN CASH",selected?ATM_MINT:MUTED,0);
 }
 int amount=atm_amount();
 box(x+19,y+217,w-38,41,0x122630,ATM_RIM);
 textat(x+33,y+224,"AMOUNT",MUTED,0);
 char*a=number(append(temp,amount>0&&amount==atm_available()?"<  MAX  $":"<  $"),amount);
 append(a,"  >");center(W/2,y+224,temp,PAPER,0);
 if(atmLastMove){
  a=number(append(temp,atmMode?"Withdrew $":"Deposited $"),atmLastMove);append(a," clean cash.");
  textat(x+20,y+271,temp,CASH_GREEN,0);
 }else if(atmRejected)textat(x+20,y+271,saveFlash<0?"Save failed. No money was moved.":"No clean cash can be transferred.",CASH_RED,0);
 else textat(x+20,y+271,"Clean cash only. Dirty money cannot be deposited.",MUTED,0);
 rect(x+19,y+303,w-38,1,ATM_RIM);
 textat(x+20,y+315,"UP/DOWN   LEFT/RIGHT   ENTER/E OK   ESC EXIT",ATM_MINT,0);
}
