/* Generate a disposable starter career at the opening-hour queue. The
   Linux replay supplies every camera/dialogue/movement/save input itself. */
#define main outdoor_roster_checks
#include "outdoor_cast_test.c"
#undef main
int main(int argc,char**argv){
 assert(argc==2);
 init_city();starter_save();sv.intro=4;sv.dayClock=1250;scene=1;menu=-1;mapOn=0;
 p.car=-1;p.a=-1.5707963f;
 NpcRef r;assert(npc_ref(N_ACT_OUT_QUEUE,&r));contact_at(r,1);
 sv.x=p.x;sv.y=p.y;sv.hash=savehash();
 FILE*f=fopen(argv[1],"wb");assert(f&&fwrite(&sv,sizeof sv,1,f)==1&&!fclose(f));
 const char*name=r.name;
 printf("Disposable curb (%.2f, %.2f), beside %s.\n",p.x,p.y,name);
 return 0;
}
