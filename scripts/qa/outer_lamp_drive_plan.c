/* Start with an isolated owned car on Freight Parkway. Enter it with E,
 * toggle C, drive directly through the old pole coordinate, then return
 * overhead and save. Actual game controls, one disposable fixture. */
#define main existing_driving_plan
#include "../../src/verdict_driving_test.c"
#undef main
int main(int argc,char **argv){
 assert(argc==2);snprintf(prefix,sizeof prefix,"%s",argv[1]);set_session_seed(136);
 init_city();sv.intro=4;sv.legal=1;sv.cash=50;sv.dayClock=1250;sv.hp=100;sv.owned=1;sv.selection=0;sv.carCond[0]=100;
 p.x=2158.f;p.y=1007.f;sv.x=p.x;sv.y=p.y;save_outer_position();
 Save11 b={0};b.base=sv;b.night=nv;b.base.rev=11;outer_safe_anchor(&b.base);
 b.night.world[NW_SAVE_X]|=0x80000000u;
 b.base.hash=night_hash(&b.base,sizeof(Save)-4);b.hash=night_hash(&b,sizeof b-4);
 char path[512];snprintf(path,sizeof path,"%s.dat",prefix);FILE*out=fopen(path,"wb");assert(out);
 assert(fwrite(&b,sizeof b,1,out)==1&&!fclose(out));
 wason=1;profiles_init();nt=0;key(ENTER);assert(scene==1&&p.car<0);
 board(0);key(CAMERA);assert(ch_enabled()&&p.y==1007.f&&p.x<2215.f);
 hold(0,6);shot("approach");hold(UP,58);
 assert(p.x>2218.f&&p.y==1007.f&&cars[0].cond==100&&p.car==0);shot("crossed");
 hold(BRAKE,22);key(CAMERA);assert(!ch_enabled());shot("overhead");key(SAVE);
 snprintf(path,sizeof path,"%s.plan",prefix);out=fopen(path,"w");assert(out);fprintf(out,"%d 0\n",nt);
 for(int i=0;i<nt;){int j=i+1;while(j<nt&&tape[j]==tape[i])j++;fprintf(out,"%d %u\n",j-i,tape[i]);i=j;}
 assert(!fclose(out));return 0;
}
