#define DC_TEST
#include "game.c"  /* Build with -Isrc; the historical tour supplies only its 1,566-tick opening. */
#include <assert.h>
static U actions[10000];static int total;
static void pressrec(U k){assert(total<LEN(actions));actions[total++]=k;tick(k);}
static void drive(float x,float y,int halt){
 int bottom=total;
 for(int i=0;i<1800;i++){
  assert(p.car>=0&&!p.dead);
  Car*c=cars+p.car;
  float dx=x-c->x,dy=y-c->y,dist=sq(d2(dx,dy));
  if(dist<(halt?14.f:11.f)&&(!halt||absf(c->s)<3.5f)){fprintf(stderr,"arrived at (%g,%g) within %g, HP %d condition %d speed %g time %g over %d frames\n",x,y,dist,p.hp,c->cond,c->s,job.time,total-bottom);return;}
  float angle=wrap(look(dx,dy)-c->a),speed=absf(c->s);U keys=0;
  if(angle>.017f)keys|=RIGHT;else if(angle<-.017f)keys|=LEFT;
  if(halt&&dist<speed*.16f+7)keys=BRAKE;
  else if(absf(angle)>.165f)keys|=BRAKE|UP;
  else if(dist>61||speed<43)keys|=UP;
  else keys|=BRAKE;
  /* Coast between throttle inputs with a rider, so a later handbrake
   * does not start at a needless tip-reducing speed. */
  if(taxi_occupied()&&speed>=92.f&&!(keys&BRAKE))keys&=~(U)UP;
  pressrec(keys);
  if(i&&i%120==0)fprintf(stderr,"go (%g,%g) frame %d xy %.1f %.1f angle %.2f speed %.1f keys %d\n",x,y,total,p.x,p.y,p.a,c->s,keys);
  if(i>119&&i%120==0){Car probe=*c;probe.x+=20;for(int v=0;v<nc;v++)if(v!=p.car&&d2(c->x-cars[v].x,c->y-cars[v].y)<90*90)fprintf(stderr,"  nearby %d xy %.1f %.1f a %.2f s %.1f cond %d\n",v,cars[v].x,cars[v].y,cars[v].a,cars[v].s,cars[v].cond);fprintf(stderr,"  projected wall depth %g\n",street_wall_depth(&probe));}

 }
 assert(0);
}
int main(void){int frames,n;unsigned expected,k;
 assert(scanf("%d %x",&frames,&expected)==2);set_session_seed(17);init_city();
 while(total<1566&&scanf("%d %u",&n,&k)==2){for(int i=0;i<n&&total<1566;i++)pressrec(k);}
 assert(p.car<0&&job.kind==JOB_TAXI&&sv.legal==1&&total==1566);
 for(int i=0;i<140&&p.car<0;i++){
   StreetUse u=street_use_target(placeNear(),carNear());
   if(u.kind==STREET_USE_CAR&&u.id==jobRide){pressrec(ENTER);assert(p.car<0);pressrec(USE);}
   else pressrec(RIGHT);
 }
 assert(p.car==jobRide&&job.kind==JOB_TAXI);
 drive(460,502,1);drive(514,506,1);drive(514,755,1);drive(340,755,1);
 for(int i=0;i<150;i++)pressrec(BRAKE);
 assert(taxi_occupied());
 drive(510,773,1);drive(750,782,0);drive(1030,782,0);drive(1279,775,1);drive(1279,510,1);drive(1382,499,1);
 for(int i=0;i<150;i++)pressrec(BRAKE);
 assert(job.kind<0&&sv.legal==2&&sv.taxiFares==1&&p.car>=0);
 pressrec(USE);assert(p.car<0);pressrec(0);pressrec(SAVE);pressrec(0);
 fprintf(stderr,"SAVE %08x clean %d legal %d x=%g y=%g HP=%d\n",sv.hash,sv.cash,sv.legal,sv.x,sv.y,sv.hp);
 printf("%d %08x\n",total,sv.hash);
 for(int i=0;i<total;){int n=1;while(i+n<total&&actions[i+n]==actions[i])n++;printf("%d %u\n",n,actions[i]);i+=n;}
}
