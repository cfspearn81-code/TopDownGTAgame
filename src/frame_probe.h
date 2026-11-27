/* Opt-in, host-side frame timing. No wall date, profile or input contents.
 * A trace is held in memory; only a separate user action exports a CSV. */
#ifndef DC_FRAME_PROBE_H
#define DC_FRAME_PROBE_H
#include "game.h"
typedef unsigned long long FPClock;
enum {FP_ROWS=300,FP_SECONDS=5,FP_BYTES=FP_ROWS*40+64};
enum {FP_HIDDEN,FP_CAPTURE,FP_READY,FP_EXPORTED,FP_FAILED,FP_EMPTY,FP_FINISH,FP_CLOCK_ERROR};
typedef struct{U interval,paint,present;} FrameSample;
typedef struct{
 FrameSample rows[FP_ROWS];
 FPClock frequency,start,previous,status_at;
 U count,seen;
 int active,status;
} FrameProbe;
static void fp_status(FrameProbe*p,int status,FPClock now){p->status=status;p->status_at=now;}
static void fp_stop(FrameProbe*p,FPClock now){
 p->active=p->seen=0;fp_status(p,p->count?FP_READY:FP_EMPTY,now);
}
static void fp_toggle(FrameProbe*p,FPClock now,FPClock freq){
 if(p->active){fp_stop(p,now);return;}
 p->frequency=freq&&freq<=1000000000000ull?freq:0;
 if(!p->frequency){fp_status(p,FP_CLOCK_ERROR,now);return;}
 p->start=now;p->active=1;p->count=p->seen=0;fp_status(p,FP_CAPTURE,now);
}
static void fp_finish_if_due(FrameProbe*p,FPClock now){
 if(p->active&&(now<p->start||now-p->start>=p->frequency*FP_SECONDS))fp_stop(p,now);
}
static U fp_us(const FrameProbe*p,FPClock first,FPClock last){
 FPClock d=last>=first?last-first:0;
 if(d/p->frequency>=4294)return 4294000000u;
 return (U)(d/p->frequency*1000000ull+d%p->frequency*1000000ull/p->frequency);
}
static void fp_sample(FrameProbe*p,FPClock begin,FPClock after_paint,FPClock before_present,FPClock end){
 if(!p->active)return;
 if(p->seen&&begin>=p->previous&&p->count<FP_ROWS)
  p->rows[p->count++]=(FrameSample){fp_us(p,p->previous,begin),fp_us(p,begin,after_paint),
                                   fp_us(p,before_present,end)};
 p->seen=1;p->previous=begin;
 if(p->count==FP_ROWS)fp_stop(p,end);
 else fp_finish_if_due(p,end);
}
static char *fp_add(char*d,const char*s){while(*s)*d++=*s++;return d;}
static char *fp_num(char*d,U n){
 char rev[10];U i=0;do{rev[i++]=(char)('0'+n%10);n/=10;}while(n);
 while(i)*d++=rev[--i];return d;
}
static U fp_csv(const FrameProbe*p,char out[FP_BYTES]){
 char*d=fp_add(out,"frame,interval_us,paint_us,present_call_us\r\n");
 for(U i=0;i<p->count;i++){
  FrameSample s=p->rows[i];
  d=fp_num(d,i+1);*d++=',';d=fp_num(d,s.interval);*d++=',';
  d=fp_num(d,s.paint);*d++=',';d=fp_num(d,s.present);*d++='\r';*d++='\n';
 }
 return (U)(d-out);
}
static int fp_export_allowed(FrameProbe*p,FPClock now){
 if(p->active){fp_status(p,FP_FINISH,now);return 0;}
 if(!p->count){fp_status(p,FP_EMPTY,now);return 0;}
 return 1;
}
static void fp_draw(FrameProbe*p,FPClock now){
 if(!p->active&&p->status!=FP_HIDDEN&&p->frequency&&now>=p->status_at&&now-p->status_at>=4*p->frequency)
  p->status=FP_HIDDEN;
 char label[42],*d=label;U color=0x9ae9df;
 switch(p->status){
 case FP_CAPTURE:case FP_FINISH:
  d=fp_add(d,"TRACE ");
  U tenth=p->count?p->rows[p->count-1].interval/100:0;
  if(tenth>9999)d=fp_add(d,">999 MS  ");
  else{d=fp_num(d,tenth/10);*d++='.';d=fp_num(d,tenth%10);d=fp_add(d," MS  ");}
  d=fp_num(d,p->count);
  d=fp_add(d,"/300  ");d=fp_add(d,p->status==FP_FINISH?"F9 THEN F10":"F9 STOP");
  color=0xffd088;break;
 case FP_READY:d=fp_add(d,"TRACE READY  ");d=fp_num(d,p->count);d=fp_add(d," ROWS  F10 EXPORT");break;
 case FP_EXPORTED:d=fp_add(d,"EXPORTED  FRAME-TIMES.CSV");break;
 case FP_FAILED:d=fp_add(d,"TRACE EXPORT FAILED  F10 RETRY");color=0xffa999;break;
 case FP_EMPTY:d=fp_add(d,"NO FRAME TRACE  F9 TO RECORD");break;
 case FP_CLOCK_ERROR:d=fp_add(d,"FRAME CLOCK UNAVAILABLE");color=0xffa999;break;
 default:return;
 }
 *d=0;frame_probe_banner(label,color);
}
#endif
