/* Record normal input from a disposable save by Freight Parkway. The target
 * is a genuine scheduled traffic actor, not injected after slot load. */
#define main driving_fixture_main
#include "../../src/verdict_driving_test.c"
#undef main
#include <assert.h>
int main(int argc,char** argv){
 assert(argc==2);snprintf(prefix,sizeof prefix,"%s",argv[1]);set_session_seed(136);
 init_city();sv.intro=4;sv.legal=1;sv.cash=50;sv.dayClock=600.f;sv.hp=100;
 p.x=2521.f;p.y=1036.f;sv.x=p.x;sv.y=p.y;save_outer_position();
 Save11 b={0};b.base=sv;b.night=nv;b.base.rev=11;outer_safe_anchor(&b.base);
 b.night.world[NW_SAVE_X]|=0x80000000u;b.base.hash=night_hash(&b.base,sizeof(Save)-4);b.hash=night_hash(&b,sizeof b-4);
 char name[512];snprintf(name,sizeof name,"%s.dat",prefix);FILE*out=fopen(name,"wb");assert(out);fwrite(&b,sizeof b,1,out);fclose(out);
 wason=1;profiles_init();nt=0;key(ENTER);assert(scene==1&&p.car<0);
 /* The driver slows down instead of running across the player marker. */
 for(int i=0;i<1200;i++){
  int id=street_near_car();if(id>=STREET_CITY_CAR_LIMIT&&absf(cars[id].s)<3)break;
  k(0);
 }
 int i=street_near_car();assert(i>=STREET_CITY_CAR_LIMIT&&absf(cars[i].s)<3);
 key(USE);assert(p.car==i&&cars[i].reported&&p.heat>25);
 key(CAMERA);assert(ch_enabled());
 hold(UP,48);hold(BRAKE,30);key(USE);assert(p.car<0);
 assert(street_near_car()==i);hold(0,15);key(USE);assert(p.car==i);
 key(USE);assert(p.car<0&&street_foot_clear(p.x,p.y,-1));
 key(SAVE);
 snprintf(name,sizeof name,"%s.plan",prefix);out=fopen(name,"w");assert(out);fprintf(out,"%d 0\n",nt);
 for(int i=0;i<nt;){int j=i+1;while(j<nt&&tape[j]==tape[i])j++;fprintf(out,"%d %u\n",j-i,tape[i]);i=j;}fclose(out);
 snprintf(name,sizeof name,"%s.expected",prefix);out=fopen(name,"w");assert(out);fprintf(out,"%.6f %.6f\n",p.x,p.y);fclose(out);
 printf("Planned %d frames: yield, board E, chase C, move, park, reboard, exit at %.3f,%.3f.\n",nt,p.x,p.y);
 return 0;
}
