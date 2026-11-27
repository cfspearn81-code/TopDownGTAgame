/* Build a synthetic, expanded Rev11 save and record only keyboard input.
 * No teleport, rider placement or car state edits after loading the career. */
#define main svtest_entry
#include "../../src/verdict_driving_test.c"
#undef main
static int chosen,target;
static const int seeds[]={12492,12448,12350,12316,12416,12308};
static void goWest(int x,int y){
 while(p.x-x>350)drive(p.x-300,y,0);
 drive(x,y,1);
}
int main(int argc,char **argv){if(argc<2)return 1;target=argc>2?atoi(argv[2]):1;if((unsigned)target>=LEN(seeds))return 2;chosen=seeds[target];snprintf(prefix,sizeof prefix,"%s",argv[1]);
 set_session_seed(chosen);init_city();sv.intro=4;sv.legal=3;sv.owned=1;sv.vehicleCond=sv.carCond[0]=100;sv.taxiFares=1;sv.cash=50;sv.dayClock=900;
 sv.x=2080;sv.y=1035;sv.hp=100;p.x=sv.x;p.y=sv.y;save_outer_position();
 Save11 b={0};b.base=sv;b.night=nv;b.base.rev=11;outer_safe_anchor(&b.base);b.night.world[NW_SAVE_X]|=0x80000000u;b.base.hash=night_hash(&b.base,sizeof(Save)-4);b.hash=night_hash(&b,sizeof b-4);
 char fname[512];snprintf(fname,sizeof fname,"%s.dat",prefix);FILE*out=fopen(fname,"wb");assert(out);fwrite(&b,sizeof b,1,out);fclose(out);
 wason=1;profiles_init();nt=0;key(ENTER);assert(scene==1&&p.car<0);key(GFX);board(0);
 /* Start at a legacy/expansion road join and drive into the city.
  * From boarding onward only real controller keys change game state. */
 hold(UP|RIGHT|BRAKE,64);trace("face downtown");goWest(1696,1011);
 trace("park downtown");key(TAXI_QUOTE);
 assert(taxiRadio==TR_QUOTE&&taxiQuote.taxi.pickup==10&&taxiQuote.taxi.dropoff==LEN(loc)+target);
 key(USE);assert(job.kind==JOB_TAXI);int quoted=job.price;
 hold(UP|LEFT|BRAKE,64);trace("turn for rider");
 drive(job.xs[0],job.ys[0],1);hold(0,75);trace("Wrecker pickup");assert(taxi.phase==TAXI_RIDING);
 /* A controller tackles the actual road itinerary without moving entities. */
 static const int r0[][2]={{1806,1024},{1806,760},{1790,712},{1790,530},{2020,512},{2130,494},{2155,483},{2208,425},{2265,394},{2332,421},{2388,471},{2460,491},{2534,450},{2577,416},{2607,394},{2660,395}};
 static const int r2[][2]={{2050,1022},{2190,1017},{2192,822},{2349,816}};
 static const int r3[][2]={{2050,1022},{2177,1027},{2177,1265},{2350,1296},{2671,1296},{2693,1460},{2701,1800}};
 static const int r4[][2]={{1806,1024},{1806,1260},{1806,1522},{1800,1792},{1792,2070},{1792,2370},{1799,2432},{1920,2496},{2038,2496}};
 static const int r5[][2]={{1680,1028},{1610,1012},{1420,1012},{1340,1012},{1250,1030},{1160,1030},{1080,1031},{1033,1020},{1037,1275},{1037,1520},{1031,1792},{1024,2070},{1025,2142},{1112,2240}};
 const int (*route)[2]=0;int n=0;switch(target){case 0:route=r0;n=LEN(r0);break;case 2:route=r2;n=LEN(r2);break;case 3:route=r3;n=LEN(r3);break;case 4:route=r4;n=LEN(r4);break;case 5:route=r5;n=LEN(r5);break;}
 if(target==1){while(job.xs[1]-p.x>350)drive(p.x+300,1011,0);}
 if(target==0||target==4||target==5){hold(UP|RIGHT|BRAKE,64);trace("turn west");}
 for(int i=0;i<n;i++){fprintf(stderr,"waypoint %d/%d %d,%d\n",i,n,route[i][0],route[i][1]);drive(route[i][0],route[i][1],0);}

 drive(job.xs[1],job.ys[1],1);hold(0,75);
 trace("regional drop");assert(job.kind<0&&sv.taxiFares==2);
 key(TAXI_RECEIPT);assert(taxiRadio==TR_RECEIPT&&taxiReceipt.trips==1&&
  taxiReceipt.base==quoted&&taxiReceipt.credit==quoted+taxi_tip_amount(quoted,0,0,0)&&sv.cash==50+taxiReceipt.credit);
 key(ENTER);assert(taxiReceipt.ended);key(ESC);
 key(SAVE);snprintf(fname,sizeof fname,"%s.plan",prefix);out=fopen(fname,"w");assert(out);fprintf(out,"%d 0\n",nt);for(int i=0;i<nt;){int j=i+1;while(j<nt&&tape[j]==tape[i])j++;fprintf(out,"%d %u\n",j-i,tape[i]);i=j;}fclose(out);fprintf(stderr,"FINISH seed=%d %d frames paid %d\n",chosen,nt,sv.cash-50);
}
