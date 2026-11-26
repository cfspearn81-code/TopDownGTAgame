#define _POSIX_C_SOURCE 200809L
#define DC_TEST
#include "game.c"
#include <time.h>
static double us(void){struct timespec a;clock_gettime(CLOCK_MONOTONIC,&a);return a.tv_sec*1000000.0+a.tv_nsec/1000.0;}
static int cmp(const void*a,const void*b){double x=*(double*)a,y=*(double*)b;return (x>y)-(x<y);}
static void sample(const char *name,int mode){
 double vals[1000],tot=0;for(int i=0;i<1120;i++){p.x=256+((i*73)%1490);p.y=256+((i*81)%1490);if(mode==1&&i%2==0){p.heat=230;}if(mode==2)menu=3;
 U k=(mode==1?FIRE:0);double n=us();tick(k);paint();double elapsed=us()-n;
 if(i>=120)vals[i-120]=elapsed,tot+=elapsed;
 }
 qsort(vals,1000,sizeof(double),cmp);printf("%-12s  mean %6.3f ms  P95 %6.3f ms  worst %6.3f ms\n",name,tot/1000000,vals[949]/1000,vals[999]/1000);
}
int main(void){double t=us();init_city();printf("First setup: %.2f ms\n",(us()-t)/1000);scene=1;
 sample("Free roam",0);sample("Pursuit",1);p.dead=0;sample("Shop",2);}
