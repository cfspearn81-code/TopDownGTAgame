/* Clock conversion, independent limits, inactive behavior and CSV bounds. */
#include "../../src/frame_probe.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int shown;
void frame_probe_banner(const char*s,U ink){assert(*s&&ink);shown++;}
int main(void){
 FrameProbe p={0};char out[FP_BYTES+1];
 fp_draw(&p,0);assert(!shown&&!p.active&&!p.count);
 assert(!fp_export_allowed(&p,10));fp_toggle(&p,0,1000000);
 for(U i=0;i<=FP_ROWS&&p.active;i++){
  FPClock n=i*4000;fp_sample(&p,n,n+500,n+600,n+850);
 }
 assert(!p.active&&p.status==FP_READY&&p.count==FP_ROWS);
 U n=fp_csv(&p,out);out[n]=0;
 assert(n<FP_BYTES&&strstr(out,"1,4000,500,250\r\n")&&strstr(out,"300,4000,500,250\r\n"));
 assert(fp_export_allowed(&p,1300000));
 fp_draw(&p,p.status_at);assert(shown==1);
 fp_draw(&p,p.status_at+4000000);assert(shown==1&&p.status==FP_HIDDEN&&p.count==FP_ROWS);
 fp_toggle(&p,50,1000);assert(p.active&&!p.count&&!fp_export_allowed(&p,50));
 fp_sample(&p,50,50,50,51);fp_sample(&p,100,102,103,107);
 fp_finish_if_due(&p,5049);assert(p.active);
 fp_finish_if_due(&p,5050);assert(!p.active&&p.count==1&&p.rows[0].interval==50000&&p.rows[0].paint==2000&&p.rows[0].present==4000);
 puts("Frame probe: 300-row cap, five-second cap, hidden defaults, blocked early export, exact CSV timing.");
}
