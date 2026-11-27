/* Synthetic career only; route uses real keyboard ticks with no relocation,
 * zeroed abilities, or forced payout after the profile is loaded. */
#define main svtest_entry
#include "../../src/verdict_driving_test.c"
#undef main
static void room_at(float x,float y){
 assert(scene==NIGHT_CLUB_SCENE);
 for(int i=0;i<300;i++){
  float dx=x-ncX,dy=y-ncY;
  if(absf(dx)<1.7f&&absf(dy)<1.7f){k(0);return;}
  k((absf(dx)>=1.7f?(dx>0?RIGHT:LEFT):0)|(absf(dy)>=1.7f?(dy>0?DOWN:UP):0));
 }
 fprintf(stderr,"Room walk stalled at %.2f,%.2f to %.2f,%.2f\n",ncX,ncY,x,y);assert(0);
}
static void talk_to(int i){
 assert(ncr_near()==i);key(USE);
 assert(ncPanel==NC_REGULAR&&ncr_contact==i&&!strcmp(nc_panel_title(),ncr_cast[i].name));
}
static void drive_via(float x,float y,int halt){
 while(d2(p.x-x,p.y-y)>340.f*340.f){float len=sq(d2(p.x-x,p.y-y));drive(p.x+(x-p.x)*285.f/len,p.y+(y-p.y)*285.f/len,0);}
 drive(x,y,halt);
}
int main(int argc,char **argv){
 assert(argc==2||argc==3);int show=argc==3?atoi(argv[2]):0;assert(show>=0&&show<3);assert(snprintf(prefix,sizeof prefix,"%s",argv[1])<(int)sizeof prefix);
 set_session_seed(17);noisy=!!getenv("VERBOSE");
 if(!show)init(179,676);
 else{
  /* Synthetic legal career at the requested later shift, before all input. */
  init_city();sv.x=179;sv.y=676;sv.intro=4;sv.owned=1;sv.selection=0;sv.carCond[0]=100;sv.legal=2;sv.cash=318;sv.dayClock=480*show+10;sv.hash=savehash();
  char path[500];snprintf(path,sizeof(path),"%s.dat",prefix);FILE*f=fopen(path,"wb");assert(f);fwrite(&sv,sizeof sv,1,f);fclose(f);
  wason=1;profiles_init();nt=0;key(ENTER);assert(scene==1);key(GFX);
 }
 assert(ordinaryDoorX[ncFrontId]==179&&scene==1);
 key(USE);assert(scene==NIGHT_CLUB_SCENE&&walkin_active==ncFrontId);
 room_at(696,423);room_at(696,318);talk_to(NCR_PHOTOGRAPHER);
 const char *line=ncr_small_talk();key(ESC);talk_to(NCR_PHOTOGRAPHER);
 assert(strcmp(line,ncr_small_talk()));key(ESC);
 room_at(275,318);room_at(275,286);talk_to(NCR_VOCALIST);assert(strstr(ncr_small_talk(),"Jules"));key(ESC);
 room_at(273,372);talk_to(NCR_PROMOTER);
 assert(ncChoice==1&&!ncr_active()&&ncr_quote==show);
 key(DOWN);key(ENTER);assert(ncr.phase==NCR_WAIT&&job.kind<0);
 room_at(480,422);key(USE);assert(scene==1&&ncr_active());
 walkXY(253,692);board(0);
 key(MAP);paint();assert(navGoalKind==8&&navGoal.x==179&&navGoal.y==746);
 key(MAP);
 drive(248,722,1);drive_via(179,746,1);hold(0,70);assert(ncr.phase==NCR_DRIVE);
 drive(170,775,1);drive_via(509,778,1);
 if(!show){
  /* The old test cut the north curb and hit a now-solid Tideway lamp.
   * Stay centered on the road until turning into the actual marked stop. */
  drive(522,1024,1);drive_via(862,1024,1);drive_via(ncr.drop.x,ncr.drop.y,1);
 }else{drive(520,1025,1);
  if(show==1){drive_via(1260,1028,1);drive_via(1297,1049,1);drive_via(1376,1049,1);}
  else{drive_via(1026,1028,1);drive_via(1026,1535,1);drive_via(1090,1532,1);}
  drive_via(ncr.drop.x,ncr.drop.y,1);
 }
 hold(0,100);
 assert((ncr.phase==NCR_LEAVE||ncr.phase==NCR_OFF)&&sv.cash==318+ncr_shows[show].fee&&!ncr_owed());
 hold(0,100);assert(sv.cash==318+ncr_shows[show].fee&&ncr.phase==NCR_OFF&&job.kind<0&&!sv.taxiFares);
 done();
 fprintf(stderr,"SHOW RIDE: %s, %d frames, $%d clean paid exactly once; no dispatch fare completed.\n",ncr_shows[show].shortName,nt,ncr_shows[show].fee);
}
