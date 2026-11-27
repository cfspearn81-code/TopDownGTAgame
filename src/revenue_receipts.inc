/* Latest checkout and seizure, kept with each career. The original
 * Stability seizure slot collided with Woodline's four forest words.
 * Purchase coordinates are unchanged; seizure uses actual spare words. */
#include "save_receipt_layout.h"
enum { REV_MAJOR_PRICE=240, REV_CASE_FINE=180, REV_ANNEX=11,
       RR_PURCHASE_META=31, RR_PURCHASE_CLEAN=35, RR_PURCHASE_DIRTY=46,
       RR_PURCHASE=3, RR_PENDING_MAX=1000000+3*480 };
typedef struct {int kind,clean,dirty,pending,minute;} RevenueLog;
static void rr_store_seizure(U marker,U clean,U dirty,U pending){
 nv.phone[DC_SEIZURE_META]=marker;
 nv.phone[DC_SEIZURE_CLEAN]=clean;
 nv.garage[DC_SEIZURE_DIRTY]=dirty;
 nv.phone[DC_SEIZURE_PENDING]=pending;
}
static void rr_record(int kind,int clean,int dirty,int pending){
 U marker=RR_MAGIC|((U)kind<<RR_TIME_BITS)|(U)cl((int)sv.dayClock,0,DAY_SECONDS-1);
 if(kind==RR_PURCHASE){
  nv.garage[RR_PURCHASE_META]=marker;
  nv.garage[RR_PURCHASE_CLEAN]=(U)clean;nv.garage[RR_PURCHASE_DIRTY]=(U)dirty;
 }else rr_store_seizure(marker,(U)clean,(U)dirty,(U)pending);
}
static void rr_checkout(int cost,int clean,int dirty){
 if(cost>=REV_MAJOR_PRICE)rr_record(RR_PURCHASE,clean,dirty,0);
}
static RevenueLog rr_decode(int purchase,U m,U c,U d,U pending){
 int kind=(m>>RR_TIME_BITS)&3u,minute=m&RR_TIME_MASK;
 if((m&0xffffe000u)!=RR_MAGIC||minute>=DAY_SECONDS||
    c>WALLET_LIMIT||d>WALLET_LIMIT||pending>RR_PENDING_MAX)return (RevenueLog){0};
 if(purchase){
  if(kind!=RR_PURCHASE||c>WALLET_LIMIT-d||c+d<REV_MAJOR_PRICE)return (RevenueLog){0};
 }else if((kind!=REVENUE_THRESHOLD&&kind!=REVENUE_SWEEP)||
          (kind==REVENUE_THRESHOLD&&c)||(kind==REVENUE_SWEEP&&(pending||(!c&&!d))))return (RevenueLog){0};
 return (RevenueLog){kind,(int)c,(int)d,(int)pending,minute};
}
static RevenueLog rr_read(int purchase){
 if(purchase)return rr_decode(1,nv.garage[RR_PURCHASE_META],nv.garage[RR_PURCHASE_CLEAN],nv.garage[RR_PURCHASE_DIRTY],0);
 return rr_decode(0,nv.phone[DC_SEIZURE_META],nv.phone[DC_SEIZURE_CLEAN],nv.garage[DC_SEIZURE_DIRTY],nv.phone[DC_SEIZURE_PENDING]);
}
static void rr_import_legacy(void){
 U m=nv.garage[DC_SEIZURE_LEGACY_META];
 if(!dc_legacy_receipt_marker(m))return;
 U c=nv.garage[DC_SEIZURE_LEGACY_CLEAN],d=nv.garage[DC_SEIZURE_LEGACY_DIRTY],p=nv.garage[DC_SEIZURE_LEGACY_PENDING];
 if(!rr_read(0).kind){
  if(rr_decode(0,m,c,d,p).kind)rr_store_seizure(m,c,d,p);
  else rr_store_seizure(RR_MAGIC,0,0,0); /* disabled tag; force the safe expanded write */
 }
 /* Even an invalid old receipt must not be interpreted as four items
  * or ready beds. The native disk layer keeps its original whole save. */
 for(int i=DC_SEIZURE_LEGACY_META;i<=DC_SEIZURE_LEGACY_PENDING;i++)nv.garage[i]=0;
}
static void rr_clock_text(char label[6],int minute){
 int h=minute/60,m=minute%60;
 label[0]='0'+h/10;label[1]='0'+h%10;label[2]=':';label[3]='0'+m/10;label[4]='0'+m%10;label[5]=0;
}
static char *rr_wallet_delta(char*dest,const char*label,int amount){
 return number(append(append(dest,label),amount?" -$":" $"),amount);
}
