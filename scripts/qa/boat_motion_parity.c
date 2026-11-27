/* Build against either exact All-Area 3D or Open City src with -I src.
 * Print float bits (not rounded coordinates) for the 60 Hz helm and
 * persistent ambient motion on common water. Deliberate shoreline repairs
 * are verified by separate navigability tests, not changed equations. */
#define DC_TEST
#include "game.c"
#include <assert.h>
static U bits(float f){U v;cp(&v,&f,4);return v;}
static void hull_fields(const HarborHull*h){
 printf(" %08x%08x%08x%08x%08x%08x:%d",bits(h->x),bits(h->y),
  bits(h->a),bits(h->s),bits(h->vx),bits(h->vy),h->kind);
}
static void sample(int frame){
 printf("%04d %d.%d.%d.%d.%d.%d",frame,scene,mapOn,hb.active,hb.dock,hb.condition,hb.contact);
 hull_fields(&hb.hull);
 printf(" %08x%08x%08x%08x%08x%08x%08x%08x%08x%08x %08x:%d",
  bits(p.x),bits(p.y),bits(p.a),bits(hb.x0),bits(hb.y0),bits(hb.safeX),bits(hb.safeY),
  bits(hb.contactSpeed),bits(hb.cool),bits(hb.wakeTime),bits(hbClock),p.hp);
 for(int i=0;i<HB_AMBIENT;i++){
  const HarborTraffic*b=hbTraffic+i;printf(" %d.%d.%d:%08x",b->live,b->route,b->leg,bits(b->blocked));
  hull_fields(&b->hull);
 }
 puts("");
}
typedef struct{int frames;U keys;} Turn;
int main(void){
 static const Turn actions[]={
  {1,USE},{72,UP},{28,UP|RIGHT},{25,UP|LEFT},{80,BRAKE},{42,DOWN},
  {25,DOWN|RIGHT},{19,DOWN|LEFT},{55,BRAKE},{1,CAMERA},{20,0},
  {1,MAP},{9,0},{1,MAP},{17,UP},{1,HELP},{10,0},{1,ESC},
  {1,PHONE},{8,0},{1,PHONE},{1,SAVE},{8,0},{50,UP|RIGHT},{1,CAMERA},
  {1,GFX},{49,BRAKE},{1,GFX},{2300,0}};
 init_city();starter_save();sv.intro=4;sv.dayClock=1250;scene=1;menu=-1;mapOn=0;fastGfx=0;
 harbor_boats_reset();bank_ui_reset();npc_reset();clear_input();vd_reset();
 int x,y;assert(harbor_dock_point(0,&x,&y));
 p=(Player){.x=x,.y=y,.car=-1,.hp=100,.a=1.57f};
 int f=0,sawReverse=0,sawPatrol=0,saw3D=0;
 for(int i=0;i<LEN(actions);i++)for(int j=0;j<actions[i].frames;j++){
  tick(actions[i].keys);f++;
  if(f%73==1){
   paint();
   if(hb.active&&scene==1&&!mapOn&&chProjected)saw3D=1;
  }
  assert(hb.active&&p.car<0&&p.hp==100);
  assert(harbor_boat_clear(p.x,p.y,hb.hull.a));
  sawReverse|=hb.hull.s<-.25f;sawPatrol|=hbTraffic[HB_AMBIENT-1].live;
  sample(f);
 }
 assert(sawReverse&&saw3D&&sawPatrol&&sv.x==x&&sv.y==y);
 return 0;
}
