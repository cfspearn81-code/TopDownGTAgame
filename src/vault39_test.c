/* Focused Vault 39 regression. Stage only the address and inspect phase data;
 * full physical walks and contract return are replayed by vault39_client.py. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <string.h>
static void vaultkey(U k){tick(k);tick(0);}
int main(void){
 init_city();sv.intro=4;sv.cash=70;scene=1;open_interior(1);
 assert(!v39_staff_visible()&&!v39_staff_near()&&!v39.serves);
 assert(!int_solid(V39_STAFF_X,V39_STAFF_Y)&&!int_solid(V39_PLAYER_X,V39_PLAYER_Y));
 int i=v39.patron;
 int t,x,y;v39_patron_at(i,&t,&x,&y);assert(t==0&&x==v39WaitX[i]&&y==v39WaitY[i]);
 for(int j=0;j<V39_HANDOFF;j++)tick(0);
 v39_patron_at(i,&t,&x,&y);assert(v39.serves==1&&x==v39Px[i]&&y==v39Py[i]);
 int tour=v39.tick;vaultkey(HELP);for(int j=0;j<31;j++)tick(0);assert(v39.tick==tour);
 vaultkey(ESC);vaultkey(ESC);tour=v39.tick;for(int j=0;j<32;j++)tick(0);assert(v39.tick==tour);
 vaultkey(ESC);assert(menu<0);
 for(int j=0;j<V39_CYCLE;j++)tick(0);
 assert(v39.patron!=i&&v39.serves==2);
 interior_x=V39_PLAYER_X;interior_y=V39_PLAYER_Y;vaultkey(USE);
 assert(v39.requested&&menu<0&&!v39.playerServed&&strstr(notice,"Mika")&&sv.cash==70);
 vaultkey(USE);assert(v39.requested&&strstr(notice,"your order"));for(int j=0;j<V39_CYCLE+81;j++)tick(0);
 assert(v39.playerServed&&!v39.playerTick&&!v39.requested&&sv.cash==70);
 vaultkey(USE);assert(!v39.playerServed&&!v39.requested&&strstr(notice,"Enjoy")&&sv.cash==70);
 interior_x=V39_STAFF_X;interior_y=V39_STAFF_Y;assert(v39_staff_near()&&v39_staff_visible());
 tick(0);tick(USE);for(int j=0;j<24;j++)tick(USE);tick(0);
 assert(scene==3&&interior_subroom&&!int_solid(interior_x,interior_y)&&p.noticeT==0);
 interior_x=interior_desk_x;interior_y=interior_desk_y;vaultkey(USE);assert(menu==1&&nopt>=6&&nopt<=7);
 for(int j=0;j<4;j++)assert(opt[j].action==14+j);
 for(int j=0;j<4;j++)vaultkey(DOWN);vaultkey(ENTER);assert(menu==1&&strstr(notice,"Ivo:"));
 vaultkey(ESC);interior_x=W/2;interior_y=428;vaultkey(USE);
 assert(scene==3&&interior_active_loc==1&&!interior_subroom&&!int_solid(interior_x,interior_y));
 assert(interior_x==V39_STAFF_X&&interior_y==198);
 interior_x=W/2;interior_y=428;vaultkey(USE);assert(scene==1&&(int)p.x==loc[1].x&&(int)p.y==loc[1].y);
 vaultkey(USE);assert(!v39.serves&&!v39.playerServed&&!interior_subroom);
 assert(sizeof(Save)==304&&SAVE_REV==10);
 puts("Vault 39: two patron visits, guard on timing, turn taking, second press, free soda, hidden E, held E, contract and dialogue indexes, two exits, local reset.");
 return 0;
}
