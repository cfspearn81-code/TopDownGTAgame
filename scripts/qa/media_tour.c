#include "route_test_base.inc"
#include <assert.h>
static void board(void){
 int rd=0,n=0,stuck=0;
 for(int k=0;k<400&&p.car<0;k++){
  if(board_test(p.x,p.y)){press_key(USE);continue;}
  if(rd>=n||stuck>8){rd=0;n=street_path_xy((int)cars[vm.van].x,(int)cars[vm.van].y,0);stuck=0;if(!n){record(0);continue;}}
  int tar=rd;for(int j=rd+1;j<n&&j<rd+13;j++){if(!street_reach_open(p.x,p.y,xof(path[j]),yof(path[j]),9,-1))break;tar=j;}
  rd=tar;float dx=xof(path[rd])-p.x,dy=yof(path[rd])-p.y,ox=p.x,oy=p.y;
  if(d2(dx,dy)<1.5f*1.5f){rd++;continue;}
  U m=0;if(absf(dx)>.4f*absf(dy))m|=dx<0?LEFT:RIGHT;if(absf(dy)>.4f*absf(dx))m|=dy<0?UP:DOWN;
  record(m);if(d2(p.x-ox,p.y-oy)<.001f)stuck++;else stuck=0;
 }
 assert(vm.van>0&&p.car==vm.van);
}
static void drive_leg(int tx,int ty,int halt){
 for(int t=0;t<1000;t++){
  if(p.car<0||p.car!=vm.van){fprintf(stderr,"lost %d gap near %g,%g => %d,%d t=%d\n",vm.van,p.x,p.y,tx,ty,t);exit(3);}
  Car*c=cars+p.car;float dx=tx-c->x,dy=ty-c->y,d=sq(d2(dx,dy)),speed=absf(c->s);
  if(d<(halt?12:15)&&(!halt||speed<3))return;
  float a=wrap(look(dx,dy)-c->a);U m=0;
  if(a>.017f)m|=RIGHT;else if(a<-.017f)m|=LEFT;
  if(halt&&d<speed*.18f+8)m=BRAKE;
  else if(absf(a)>.18f)m|=BRAKE|UP;
  else if(speed<100&&(!halt||d>54||speed<34))m|=UP;else if(halt&&d<48)m|=BRAKE;
  if(t%100==1)fprintf(stderr,"goto %d,%d pos %.1f,%.1f a%.1f s%.1f cond%d heat%.1f\n",tx,ty,p.x,p.y,p.a,c->s,c->cond,p.heat);record(m);
 }
 fprintf(stderr,"Stuck %g,%g a %g s %g cond %d => %d,%d\n",p.x,p.y,p.a,cars[p.car].s,cars[p.car].cond,tx,ty);exit(5);
}
static void tapeout(FILE*f){
 printf("%d %08x\n",total,sv.hash);
 for(int i=0;i<total;){int n=1;while(i+n<total&&akeys[i]==akeys[i+n])n++;printf("%d %u\n",n,akeys[i]);i+=n;}
}
static void cap(const char*path,const char*name){
 char dest[2048];snprintf(dest,sizeof dest,"%s/%s.ppm",path,name);FILE*f=fopen(dest,"wb");assert(f);
 paint();fprintf(f,"P6\n960 540\n255\n");for(int i=0;i<W*H;i++){fputc(pixels[i]>>16,f);fputc(pixels[i]>>8,f);fputc(pixels[i],f);}fclose(f);
 fprintf(stderr,"%s=%d\n",name,total);
}
static void listing(int task){
 press_key(ACTIVITIES);ck(vdPanel==1,"F6 board");while(vdSelected!=task)press_key(DOWN);
 press_key(ENTER);ck(vdActive==task,"F6 activity launch");
}
int main(int argc,char**argv){
 assert(argc>=5);int task=atoi(argv[1]),branch=atoi(argv[2]);const char*dir=argv[4];
 int id=task==4?VA_B4:task==7?VA_B7:VA_B8;int venue=task==4?VM_RECORDS:task==7?VM_STUDIO:VM_PRESS;
 set_session_seed(17);init_city();sv.intro=3;sv.legal=1;sv.cash=13;sv.dirty=9;sv.taxiFares=1;sv.hp=100;sv.x=ordinaryDoorX[venue];sv.y=ordinaryDoorY[venue];
 memset(sv.name,0,sizeof sv.name);append(sv.name,"Noor");sv.hash=savehash();FILE*f=fopen(argv[3],"wb");assert(f&&fwrite(&sv,sizeof sv,1,f)==1);fclose(f);
 profiles_activate();akeys[total++]=0;akeys[total++]=ENTER;record(0);record(0);record(0);
 listing(id);press_key(USE);ck(vm_station(venue)||scene==4,"wrong entry");go(STATION_G);press_key(USE);ck(vdPanel==2,"no choice");cap(dir,"choice");
 if(branch)press_key(DOWN);press_key(ENTER);ck(!vdPanel,"choice unresolved");
 if(task==4){
  assert(vm.blockBill==branch);go(EXIT_G);press_key(USE);street_to(vmVenue[branch]);ck(vdActive<0&&sv.cash==98&&sv.legal==2,"booking payout");cap(dir,"marquee");
  for(int i=0;i<100;i++)record(0);assert(sv.cash==98&&vm.blockBill==branch);
 }else if(task==8){
  ck(vm.printPhase==2,"x stage");if(!branch)for(int i=0;i<46;i++)record(0);
  cap(dir,"keyplate");press_key(ENTER);ck(vm.printPhase==3,"y stage");if(!branch)for(int i=0;i<56;i++)record(0);
  cap(dir,"spotplate");press_key(ENTER);ck(vm.impression&&sv.cash==58&&sv.legal==2,"print payout");assert(vm.quality==(branch?0:2));cap(dir,"interior-proof");
  go(EXIT_G);press_key(USE);assert(scene==1);cap(dir,"window");press_key(USE);assert(scene==4);cap(dir,"revisit");
 }else{
  assert(vm.event==branch);go(EXIT_G);press_key(USE);board();fprintf(stderr,"boarded %.1f,%.1f a %.2f cond %d\n",p.x,p.y,p.a,cars[p.car].cond);cap(dir,"van");
  if(argc>5){for(int i=5;i<argc;i++){int x,y,h;assert(sscanf(argv[i],"%d,%d,%d",&x,&y,&h)==3);drive_leg(x,y,h);}}else {drive_leg(1023,1278,1);drive_leg(1024,1025,1);
  if(branch){drive_leg(1448,1025,0);drive_leg(1699,1025,0);drive_leg(1853,1021,1);}else drive_leg(861,1025,1);}
  assert(vm_bay(cars+p.car));press_key(ENTER);assert(vdActive==id);
  cap(dir,"frame");int left=0,right=0,first=vm_subject_x();
  for(int i=0;i<330&&vm.hold<120;i++){
   int d=vm_pan_error();U k=d>2?RIGHT:d< -2?LEFT:0;record(k|BRAKE);left+=!!(k&LEFT);right+=!!(k&RIGHT);
  }
  assert(vm.hold>=120&&(left||right)&&vm_subject_x()!=first);
  cap(dir,"ready");press_key(ENTER);ck(sv.cash==148&&vdActive<0&&vm.newsPage==branch,"live payout");cap(dir,"live");
 }
 press_key(SAVE);assert(sv.dirty==9);tapeout(stdout);fprintf(stderr,"end %d frames %s $%d legal=%d HP=%d\n",total,notice,sv.cash,sv.legal,sv.hp);return 0;
}
