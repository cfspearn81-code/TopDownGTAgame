/* Build a synthetic, expanded Rev11 save and record only keyboard input.
 * No teleport, rider placement or car state edits after loading the career. */
#define main svtest_entry
#include "../../src/verdict_driving_test.c"
#undef main
enum { chosen=136 };
static void goWest(int x,int y){
 while(p.x-x>350)drive(p.x-300,y,0);
 drive(x,y,1);
}
int main(int argc,char **argv){if(argc<2)return 1;snprintf(prefix,sizeof prefix,"%s",argv[1]);
 set_session_seed(chosen);init_city();sv.intro=4;sv.legal=3;sv.owned=1;sv.vehicleCond=sv.carCond[0]=100;sv.taxiFares=1;sv.cash=50;sv.dayClock=900;
 sv.x=2530;sv.y=1035;sv.hp=100;p.x=sv.x;p.y=sv.y;save_outer_position();
 Save11 b={0};b.base=sv;b.night=nv;b.base.rev=11;outer_safe_anchor(&b.base);b.night.world[NW_SAVE_X]|=0x80000000u;b.base.hash=night_hash(&b.base,sizeof(Save)-4);b.hash=night_hash(&b,sizeof b-4);
 char fname[512];snprintf(fname,sizeof fname,"%s.dat",prefix);FILE*out=fopen(fname,"wb");assert(out);fwrite(&b,sizeof b,1,out);fclose(out);
 wason=1;profiles_init();nt=0;key(ENTER);assert(scene==1&&p.car<0);key(GFX);board(0);
 int radio=argc>2;
 if(radio){
  key(TAXI_RECEIPT);assert(taxiRadio==TR_RECEIPT&&!taxiReceipt.trips);
  key(ESC);assert(!taxiRadio&&job.kind<0);
  key(TAXI_QUOTE);assert(taxiRadio==TR_QUOTE&&job.kind<0);int base=taxiQuote.job.price;
  key(USE);assert(!taxiRadio&&job.kind==JOB_TAXI&&job.price==base);
 }else key(TAXI_NEXT);
 assert(taxi.pickup==LEN(loc)+1&&job.ys[1]==1011);fprintf(stderr,"seed %d: %s to %s. Start %.1f,%.1f a %.2f, stop %d,%d -> %d,%d\n",chosen,taxi_stop_label(taxi.pickup),taxi_stop_label(taxi.dropoff),p.x,p.y,p.a,job.xs[0],job.ys[0],job.xs[1],job.ys[1]);int target=job.xs[1];
 drive(job.xs[0],job.ys[0],1);hold(0,75);assert(taxi.phase==TAXI_RIDING);trace("on board");
 hold(UP|RIGHT|BRAKE,64);trace("turn");
 goWest(target,1011);hold(0,75);assert(job.kind<0&&sv.taxiFares==2);
 if(radio){
  key(TAXI_RECEIPT);assert(taxiRadio==TR_RECEIPT&&taxiReceipt.trips==1&&taxiReceipt.credit==147);
  key(ENTER);assert(taxiReceipt.ended&&sv.cash==197);key(ESC);assert(!taxiRadio);
 }
 key(SAVE);snprintf(fname,sizeof fname,"%s.plan",prefix);out=fopen(fname,"w");assert(out);fprintf(out,"%d 0\n",nt);for(int i=0;i<nt;){int j=i+1;while(j<nt&&tape[j]==tape[i])j++;fprintf(out,"%d %u\n",j-i,tape[i]);i=j;}fclose(out);fprintf(stderr,"FINISH seed=%d %d frames paid %d\n",chosen,nt,sv.cash-50);
}
