/* Verify that the service simulation observes the present bartender. This is
 * a throwaway synthetic character; all venue/encounter actions are normal keys. */
#define main hold_regression_entry
#include "npc_hold_test.c"
#undef main
#include <string.h>
int main(int argc,char**argv){
 int observe=argc>1;init_city();setup();open_interior(1);
 interior_x=V39_PLAYER_X;interior_y=V39_PLAYER_Y;v39.tick=11;v39.patron=0;
 tap(BRAKE|USE);assert(npcTalkOn&&npcTalk.id==N_ACT_MIKA);
 tap(DOWN);set_session_seed(2);tap(ENTER);
 assert(npc_replaces(N_ACT_MIKA));
 int before=v39.tick;tap(USE);int accepted=v39.requested||v39.playerTick;
 /* Same actor is away from the counter during this encounter. */
 wait_ticks(12);assert(npc_replaces(N_ACT_MIKA));
 printf("Absent bartender: request accepted=%d; autonomous service advanced %d ticks; notice=%s\n",accepted,v39.tick-before,notice);
 if(!observe){assert(!accepted&&v39.tick==before&&strstr(notice,"counter"));}
 /* Ensure this doesn't suspend the simulation permanently. */
 npc_forget_encounter();int t=v39.tick;wait_ticks(4);assert(v39.tick==t+4);
 if(accepted){v39.requested=v39.playerTick=0;}
 tap(USE);assert(v39.requested||v39.playerTick);wait_ticks(V39_CYCLE+85);assert(v39.playerServed);
 tap(USE);assert(!v39.playerServed);puts("PASS: resumed bar and collected exactly one waiting order.");
}
