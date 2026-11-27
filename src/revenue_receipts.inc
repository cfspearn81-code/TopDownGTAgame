/* Latest in-game checkout and seizure, kept with this career. These seven
 * previously unused Rev11 garage words never alter the 304-byte base.
 * The two source-of-truth flags below are not derived from HUD animation. */
enum { REV_MAJOR_PRICE=240, REV_CASE_FINE=180, REV_ANNEX=11,
       RR_PURCHASE_META=31, RR_PURCHASE_CLEAN=35, RR_PURCHASE_DIRTY=46,
       RR_SEIZURE_META=49, RR_SEIZURE_CLEAN=50, RR_SEIZURE_DIRTY=51,
       RR_PENDING_SEIZED=52 };
enum { RR_MAGIC=0x52560000u, RR_TIME_BITS=11, RR_TIME_MASK=2047u, RR_PURCHASE=3,
       RR_PENDING_MAX=1000000+3*480 };
typedef struct {int kind,clean,dirty,pending,minute;} RevenueLog;
static void rr_record(int kind,int clean,int dirty,int pending) {
 U marker=RR_MAGIC|((U)kind<<RR_TIME_BITS)|(U)cl((int)sv.dayClock,0,DAY_SECONDS-1);
 if(kind==RR_PURCHASE){
  nv.garage[RR_PURCHASE_META]=marker;
  nv.garage[RR_PURCHASE_CLEAN]=(U)clean;nv.garage[RR_PURCHASE_DIRTY]=(U)dirty;
 }else{
  nv.garage[RR_SEIZURE_META]=marker;
  nv.garage[RR_SEIZURE_CLEAN]=(U)clean;nv.garage[RR_SEIZURE_DIRTY]=(U)dirty;
  nv.garage[RR_PENDING_SEIZED]=(U)pending;
 }
}
static void rr_checkout(int cost,int clean,int dirty){
 if(cost>=REV_MAJOR_PRICE)rr_record(RR_PURCHASE,clean,dirty,0);
}
static RevenueLog rr_read(int purchase){
 U m=nv.garage[purchase?RR_PURCHASE_META:RR_SEIZURE_META];
 U c=nv.garage[purchase?RR_PURCHASE_CLEAN:RR_SEIZURE_CLEAN];
 U d=nv.garage[purchase?RR_PURCHASE_DIRTY:RR_SEIZURE_DIRTY];
 U pending=purchase?0:nv.garage[RR_PENDING_SEIZED];
 int kind=(m>>RR_TIME_BITS)&3u,minute=m&RR_TIME_MASK;
 if((m&0xffffe000u)!=RR_MAGIC||minute>=DAY_SECONDS||
    c>WALLET_LIMIT||d>WALLET_LIMIT||pending>RR_PENDING_MAX)return (RevenueLog){0};
 if(purchase){
  if(kind!=RR_PURCHASE||c>WALLET_LIMIT-d||c+d<REV_MAJOR_PRICE)return (RevenueLog){0};
 }else if((kind!=REVENUE_THRESHOLD&&kind!=REVENUE_SWEEP)||
          (kind==REVENUE_THRESHOLD&&c)||(kind==REVENUE_SWEEP&&(pending||(!c&&!d))))return (RevenueLog){0};
 return (RevenueLog){kind,(int)c,(int)d,(int)pending,minute};
}
static void rr_clock_text(char label[6],int minute){
 int h=minute/60,m=minute%60;
 label[0]='0'+h/10;label[1]='0'+h%10;label[2]=':';label[3]='0'+m/10;label[4]='0'+m%10;label[5]=0;
}
static char *rr_wallet_delta(char*dest,const char*label,int amount){
 return number(append(append(dest,label),amount?" -$":" $"),amount);
}
