/* Old tagged receipts never become a forest item or falsely recover funds. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
int main(void){
 starter_save();
 nv.garage[49]=0x1bfffu;
 nv.garage[50]=0x08421u;
 nv.garage[51]=0x0fa20176u;
 nv.garage[52]=0x00500309u;
 const U forest[]={nv.garage[49],nv.garage[50],nv.garage[51],nv.garage[52]};
 rr_import_legacy();
 for(int i=0;i<4;i++)assert(nv.garage[49+i]==forest[i]);
 assert(!rr_read(0).kind);
 nv.garage[49]=RR_MAGIC|(REVENUE_SWEEP<<RR_TIME_BITS)|613u;
 nv.garage[50]=0xffffffffu; /* invalid signed-domain total */
 nv.garage[51]=7;nv.garage[52]=0;
 rr_import_legacy();
 for(int i=49;i<=52;i++)assert(nv.garage[i]==0);
 assert(nv.phone[DC_SEIZURE_META]==RR_MAGIC&&!rr_read(0).kind);
 sv.dayClock=666;
 rr_record(REVENUE_THRESHOLD,0,400,79);
 RevenueLog r=rr_read(0);
 assert(r.kind==REVENUE_THRESHOLD&&r.minute==666&&r.clean==0&&r.dirty==400&&r.pending==79);
 puts("Receipt overlap guard: real forest untouched; malformed prior receipt retained for safe backing up and not shown; subsequent receipt fully valid.");
}
