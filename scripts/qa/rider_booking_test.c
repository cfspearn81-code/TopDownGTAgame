/* Staged curbs with real input and fare/encounter updates. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void key(U k){tick(k);tick(0);}
int main(void){
 init_city();starter_save();profiles_activate();sv.intro=4;sv.cash=50;sv.dirty=0;sv.owned=1;sv.taxiFares=3;
 scene=1;menu=-1;mapOn=0;clear_input();end_job();bank_ui_reset();set_session_seed(136);
 p=(Player){.x=768,.y=1038,.hp=100,.car=0};cars[0]=(Car){.x=p.x,.y=p.y,.owner=1,.cond=100};
 cabRequested=1;beginJob(JOB_TAXI);cabRequested=0;assert(job.kind==JOB_TAXI);
 int sx=job.xs[0],sy=job.ys[0],dx=job.xs[1],dy=job.ys[1];
 p.car=-1;p.x=taxi.x-20;p.y=taxi.y;sv.gear=0;int base=job.price,full=base+taxi_tip_amount(base,0,0,0);
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=900};
 for(int i=0;i<nc;i++)if(i){cars[i]=(Car){.x=-100,.y=-100};streetDrivers[i].live=0;}
 /* A different person is not our booked passenger. Use a busy public
  * checkpoint and a separate actor; dropping out of the encounter doesn't
  * touch the pending route or cash. */
 float riderX=p.x,riderY=p.y;p.x=1024;p.y=1024;peds[0]=(Ped){.x=1054,.y=1024,.hp=3};
 key(USE);assert(npcTalkOn&&npcTalk.id==N_ACT_STREET);key(DOWN);key(ENTER);
 assert(npcEnc.phase==NPC_DRAW&&job.kind==JOB_TAXI&&sv.cash==50);
 p.x=riderX;p.y=riderY;key(0);assert(npcEnc.phase==NPC_IDLE&&job.kind==JOB_TAXI);
 for(U i=0;;i++){set_session_seed(i);npc_roll_bound(141);if(npc_roll_bound(100)<35){set_session_seed(i);break;}}
 key(USE);
 assert(npcTalkOn&&npcTalk.id==N_ACT_RIDER+taxi.rider);
 key(UP);key(ENTER);assert(!npcTalkOn&&job.kind==JOB_TAXI);
 /* An empty-handed failed robbery cannot cancel the job. */
 sv.gear=-1;key(USE);key(DOWN);key(ENTER);assert(!npcTalkOn&&!npcSpent[N_ACT_RIDER+taxi.rider]&&job.kind==JOB_TAXI);sv.gear=0;
 key(USE);key(DOWN);key(ENTER);
 assert(npcEnc.phase==NPC_DRAW&&job.kind<0&&strstr(notice,"rider cancels"));int stolen=npcEnc.prize;
 for(int i=0;i<90;i++)tick(0);
 assert(sv.dirty==stolen&&stolen>=15&&stolen<=155&&sv.cash==50&&npcEnc.phase==NPC_IDLE);
 printf("After confirmed hold-up and surrender: stolen $%d, current booking=%d\n",stolen,job.kind);fflush(stdout);
 p.car=0;p.x=cars[0].x=sx;p.y=cars[0].y=sy;
 for(int i=0;i<110;i++)tick(0);
 assert(taxi.phase!=TAXI_RIDING&&sv.taxiFares==3);
 p.x=cars[0].x=dx;p.y=cars[0].y=dy;
 for(int i=0;i<130;i++)tick(0);
 printf("After normal drop-off: $%d additional fare vs unpenalized $%d; clean=%d, completed=%u, shift trips=%d\n",sv.cash-50,full,sv.cash,sv.taxiFares,taxiReceipt.trips);fflush(stdout);
 assert(sv.cash==50&&sv.taxiFares==3&&!taxiReceipt.trips&&job.kind<0);
}
