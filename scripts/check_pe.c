/* Linux compatibility smoke harness: map the exact PE image and drive its
 * real entrypoint with stubbed Win32 services. This is not a Windows test. */
#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <setjmp.h>
typedef uint64_t Q;typedef uint32_t D;typedef uint16_t S;
typedef struct{int l,t,r,b;}Rect;
typedef struct {void *w;D msg;Q wp;Q lp;D time;int x,y;D unused;}Msg;
static int frame, saved, volume, delivered, menuFrame, safeWrites;static jmp_buf jb;
static Q ms;static unsigned checksum,previous;static const char *loadCareer,*saveCareer;static int readCareer,opening,pointerScenario,pointerStage,pointerPx,pointerPy,clearedPx,createStage,creates;
static int deleteTest,deleteStage,deleteCalls,atmTest,borderClick;
static unsigned fsz[252],offset;static unsigned char vdisk[252][2048],pending[2048];static unsigned psize;static D status;static unsigned beforePin,afterPin,afterClear;
#define A __attribute__((ms_abi))
typedef int64_t (A *HostProc)(void*,D,Q,int64_t);static HostProc winproc;
static int closeTest,closeTried,closePrompts,closePosted;
static int probeUnmodifiedDefault,probeAltDefault;
static int probeMode,probeFrame=-1,probeWrites,probeCaptured,probeDenied;
static unsigned probeBytes;
static char probeText[12100];
static uint64_t probeClock,probeOldMs;
static int A probe_freq(int64_t*p){assert(probeMode);*p=1000000;return 1;}
static int A probe_counter(int64_t*p){
 assert(probeMode);if(probeOldMs!=ms){probeOldMs=ms;probeClock=0;}
 *p=(int64_t)(1000000+ms*1000+250*probeClock++);return 1;
}
static int is_probe_file(const S*name){
 while(*name&&*name!='\\')name++;
 while(*name){if(*name=='\\'&&name[1]=='f'&&name[2]=='r'){
  const char *leaf="frame-times.csv";name++;
  while(*leaf&&*name==(S)*leaf){name++;leaf++;}
  return !*leaf&&!*name;
 }name++;}
 return 0;
}
static int A close_prompt(void*w,const S*msg,const S*title,D flags){
 assert(closeTest&&closeTried==1&&w==(void*)1&&*msg&&*title&&(flags&0x1ff)==0x134);
 closePrompts++;return closeTest==2?6:7; /* YES: discard; NO (default): keep. */
}
static Q A close_post(D code){assert(!code);closePosted=1;return 0;}
static int64_t A defproc(void*w,D msg,Q wp,int64_t lp){
 if(probeMode&&wp==0x79&&msg>=0x104&&msg<=0x105){if(lp&0x20000000)probeAltDefault++;else probeUnmodifiedDefault++;}
 if(closeTest&&msg==0x10)return winproc(w,2,0,0); /* Close -> Destroy. */
 return 0;
}
static Q A ok(void){return 1;}
static Q A nil(void){return 0;}
/* The optional street tour streams (tick count, key mask) pairs. */
static int tapMode,tapFrame=-1,tapQueue,tapKey,tapUp,tapCount;static unsigned tapPrev;
static int tourFrames,tourHold; static Q tourSeed=17;
static unsigned tourMask,tourSaveHash;
static void next_tour_keys(void){
 if(!tourHold)assert(scanf("%d %u",&tourHold,&tourMask)==2&&tourHold>0);
 tourHold--;
}
static Q A key(int k){
 if(tourFrames){
  if(tapMode&&(k==13||k=='E'||k==0x74))return 0; /* Released before the next poll. */
  static const struct {int key;unsigned bit;} bindings[]={
   {'W',1},{'S',2},{'A',4},{'D',8},{'E',16},{0x20,32},{0x10,64},
   {0x1b,128},{'M',256},{0x74,512},{13,1024},{0x70,2048},{0x71,4096},
   {0x72,8192},{0x73,16384},{'X',32768},{'T',65536},{'Q',1u<<26},{'R',1u<<27},{0x2e,131072},{0x75,262144},{'P',524288},{0x76,1048576},{'C',2097152},{'L',4194304},{0x77,8388608},{0x09,16777216},{'V',33554432}
  };
  for(unsigned i=0;i<sizeof bindings/sizeof *bindings;i++)
   if(bindings[i].key==k)return (tourMask&bindings[i].bit)?0x8000:0;
  return 0;
 }
 if(atmTest)return ((k==13&&(frame==2||frame==7||frame==14))||
  (k=='E'&&frame==4)||(k==0x28&&frame==10)||(k==0x25&&frame==12)||
  (k==0x1b&&frame==16)||(k==0x74&&frame==20))?0x8000:0;
 if(deleteTest)return ((k==0x28&&frame==1)||(k==0x2e&&frame==3)||(k==13&&(frame==6||frame==20))||(k==0x74&&frame==62))?0x8000:0;
 if(pointerScenario)return ((k==13&&frame==1)||(k=='M'&&(frame==5||frame==50))||(k==0x74&&frame==66))?0x8000:0;
 if(loadCareer&&opening!=2){
  return ((k==13&&frame==2)||(k=='D'&&frame>15&&frame<57)||(k==0x74&&frame==62))?0x8000:0;
 }
 if((k==13&&(frame==1||frame==6||frame==11))||(k==0x73&&opening==2&&frame==1)||
    (k=='S'&&frame>=14&&frame<=63)||(k=='E'&&frame==65)||(k==0x74&&frame==70))return 0x8000;
 return 0;
}
static void A die(D code){assert((closeTest?closePosted:frame==(tourFrames?tourFrames:75))&&code==0);longjmp(jb,1);}
static Q A ticks(void){
 if(tourFrames){
  /* Seed 17, then one 60 Hz simulation tick per host loop. */
  if(ms++<2)return tourSeed;
  return tourSeed+((ms-2)*1000+59)/60;
 }
 ms+=17;return ms;
}
/* Model actual GDI operations of frames drawn by the exact Windows entry.
 * DC_PRESENT=observe exposes old full-client blanks without failing;
 * DC_PRESENT=verify asserts all black fills are confined to the bars. */
static int presentMode,presentOverlap,presentFills,presentCalls,presentWheels,presentArea;
static int present_client(int *h);
static int present_size_changed(void){return frame==19||frame==35||frame==51;}
static int present_damage(void){return frame==37||frame==62;}
static int present_client(int *h){
 if(frame<19){*h=790;return 1300;}
 if(frame<35){*h=815;return 1969;}
 if(frame<51){*h=1080;return 1920;}
 *h=791;return 1301;
}
static int present_peek(Msg*m){
 static int sent=-1;
 if(!presentMode||sent==frame||frame>=tourFrames)return 0;
 int move=(frame>=6&&frame<=28)||(frame>=52&&frame<=59),msg=0;
 if(present_size_changed())msg=5;
 else if(present_damage())msg=15;
 else if(frame==48)msg=0x02a3; /* Mouse left the window. */
 else if(frame==41)msg=0x0008; /* Focus-loss message, with foreground still available. */
 else if(move)msg=0x0200;
 if(!msg)return 0;
 sent=frame;memset(m,0,sizeof *m);m->w=(void*)1;m->msg=msg;
 if(move){int h,w=present_client(&h);unsigned x=w/2+(frame%6)*7,y=frame%5?h/2:h/4;
  m->lp=x+(y<<16);}
 return 1;
}
static void present_view(int *x,int *y,int *w,int *h,int *fullw,int *fullh){
 *fullw=present_client(fullh);*w=*fullw;*h=*fullh;
 if(*w*540>*h*960)*w=*h*960/540;else *h=*w*540/960;
 *x=(*fullw-*w)/2;*y=(*fullh-*h)/2;
}
static int A patblt(void*dc,int x,int y,int w,int h,D rop){
 if(!presentMode)return 1;
 int vx,vy,vw,vh,fw,fh;present_view(&vx,&vy,&vw,&vh,&fw,&fh);
 assert(rop==0x42&&w>0&&h>0&&x>=0&&y>=0&&x+w<=fw&&y+h<=fh);
 ++presentFills;presentArea+=w*h;
 if(x<vx+vw&&x+w>vx&&y<vy+vh&&y+h>vy){
  ++presentOverlap;
  printf("GDI BLACKNESS crossed the viewport at frame %d: (%d,%d %dx%d)\n",frame,x,y,w,h);
 }
 return 1;
}
static void present_frame(int x,int y,int w,int h,const D*b){
 int vx,vy,vw,vh,fw,fh;present_view(&vx,&vy,&vw,&vh,&fw,&fh);
 assert(x==vx&&y==vy&&w==vw&&h==vh);
 int refresh=frame==0||present_size_changed()||present_damage();
 /* Bars aren't re-painted on simple cursor activity, even during the wheel. */
 if(presentMode==2)assert(!presentOverlap&&presentArea==(refresh?fw*fh-vw*vh:0));
 presentArea=0;presentCalls++;
 if(b[67*960+480]==0x63d7cf||b[467*960+480]==0x63d7cf)presentWheels++;
}
static int A peek(Msg*m,void*w,D a,D b,D c){
 if(closeTest){
  if(closePosted){memset(m,0,sizeof *m);m->msg=0x12;return 1;}
  if((closeTried==0&&frame==18)||(closeTried==1&&frame==26)){
   closeTried++;memset(m,0,sizeof *m);m->w=(void*)1;m->msg=0x10;return 1;
  }
 }
 if(tapMode&&frame<tourFrames){
  if(tapFrame!=frame){
   tapFrame=frame;tapQueue=(tourMask&~tapPrev)&(1024|16|512);tapPrev=tourMask;
  }
  if(tapUp||tapQueue){
   int up=tapUp;
   if(up)tapUp=0;else{
    unsigned bit=(unsigned)tapQueue&-(unsigned)tapQueue;tapQueue&=~bit;
    tapKey=bit==1024?13:bit==16?'E':0x74;tapUp=1;tapCount++;
   }
   memset(m,0,sizeof *m);m->w=(void*)1;m->msg=up?0x0101:0x0100;m->wp=tapKey;m->lp=up?0xc0000001u:1;return 1;
  }
 }
 if(probeMode&&probeFrame!=frame){
  int k=frame==1||frame==9||frame==24||frame==25||frame==26||frame==27||frame==33?0x79:frame==6||frame==23?0x78:0;
  if(k){
   if(frame==23||frame==27)assert(!probeWrites);
   /* Bare F10 uses the system-key path on a real Windows desktop. */
   probeFrame=frame;memset(m,0,sizeof *m);m->w=(void*)1;m->msg=k==0x79?(frame==25?0x105:0x104):0x100;m->wp=k;
   m->lp=frame==24?0x20000001:frame==25?0xc0000001:frame==26?0x40000001:1;return 1;
  }
 }
 if(present_peek(m))return 1;
 if(tourFrames){
  if(frame<tourFrames)return 0;
  memset(m,0,sizeof *m);m->msg=0x12;return 1;
 }
 if(deleteTest&&((frame==9&&deleteStage==0)||(frame==13&&deleteStage==1))){
   memset(m,0,sizeof *m);m->w=(void*)1;m->msg=0x0201;
   m->lp=deleteStage++==0?790+(460<<16):590+(332<<16);return 1;
 }
 if(pointerScenario==1&&frame==26&&!borderClick){
   memset(m,0,sizeof *m);m->w=(void*)1;m->msg=0x0204;
   m->lp=500+(2<<16);borderClick=1;return 1;
 }
 if(pointerScenario&&((frame==12&&pointerStage==0)||(frame==30&&pointerStage==1))){
   memset(m,0,sizeof *m);m->w=(void*)1;m->msg=pointerStage++==0?0x0201:0x0204;
   int ww=pointerScenario==2?1920:1300,hh=pointerScenario==2?1080:790,vy=(hh-ww*540/960)/2;
   const int sx=610,sy=334;int x=(sx*ww+959)/960,y=vy+(sy*(ww*540/960)+539)/540;
   m->lp=(unsigned)x+((unsigned)y<<16);return 1;
 }
 if(!pointerScenario&&(!loadCareer||opening==2)&&((frame==3&&createStage==0)||(frame==5&&createStage<5)||(frame==8&&createStage==5))){
   memset(m,0,sizeof *m);m->w=(void*)1;
   if(createStage==0){m->msg=0x0201;m->lp=566+(151<<16);}
   else if(createStage<=4){m->msg=0x0102;m->wp="QA: "[createStage-1];}
   else{m->msg=0x0201;m->lp=578+(175<<16);}
   createStage++;return 1;
 }
 if(frame<75)return 0;memset(m,0,sizeof *m);m->msg=0x12;return 1;
}
static int A rect(void*w,Rect*r){if(presentMode){int h,w=present_client(&h);*r=(Rect){0,0,w,h};return 1;}*r=pointerScenario==2?(Rect){0,0,1920,1080}:pointerScenario?(Rect){0,0,1300,790}:(Rect){0,0,960,540};return 1;}
static Q A regcls(void*k){memcpy(&winproc,(char*)k+8,sizeof winproc);return 1;}
static int64_t A dispatchMsg(Msg*m){assert(winproc);return winproc(m->w,m->msg,m->wp,m->lp);}
static int pathslot(const S*p){
 assert(p&&*p=='.');
 while(*p&&*p!='c'&&*p!='o'&&*p!='f')p++;
 if(*p=='c')return 0;if(*p=='o')return 1;
 assert(*p=='f');return 1+100*(p[6]-'0')+10*(p[7]-'0')+p[8]-'0';
}
/* Isolated stage must be carried unchanged through close and commit. */
static S stageName[1024];
/* A backup is not a selectable save slot; its bytes remain independent. */
static S backupName[8][1024];static D backupSize[8];static unsigned char backupBytes[8][2048];
static int backup_slot(const S*p,int create){
 const S*leaf=p;for(const S*q=p;*q;q++)if(*q=='\\')leaf=q+1;
 if(leaf[0]!='p'||leaf[1]!='r'||leaf[2]!='e'||leaf[3]!='-')return -1;
 for(int i=0;i<8;i++){
  int j=0;while(leaf[j]&&backupName[i][j]==leaf[j])j++;
  if(!leaf[j]&&!backupName[i][j])return i;
  if(!backupName[i][0]){if(create){do{backupName[i][j]=leaf[j];}while(leaf[j++]);}return i;}
 }
 assert(!"backup stub capacity");return -1;
}
static Q A openfile(const S*p,D access,D share,void*sec,D mode,D attr,void*t){
 if(mode==1){
  if(closeTest&&closeTried==1){status=5;return UINT64_MAX;}
  if(probeMode&&frame==27){probeDenied++;status=5;return UINT64_MAX;}
  assert(access==0x40000000u&&!share&&!stageName[0]);int n=0;
  do{assert(n<1024);stageName[n]=p[n];}while(p[n++]);
  psize=0;return 400;
 }
 int b=backup_slot(p,0);if(b>=0){offset=0;if(backupSize[b])return 501+b;status=2;return UINT64_MAX;}
 int i=pathslot(p);assert(i>=0&&i<252);
 assert(mode==3&&share==5);offset=0;
 if(fsz[i])return i+1;
 status=2;return UINT64_MAX;
}
static int A removefile(const S*p){assert(deleteTest&&frame==13&&pathslot(p)==2&&fsz[0]&&fsz[2]&&!deleteCalls);fsz[2]=0;deleteCalls++;return 1;}
static Q A errorcode(void){return status;}
static int A readfile(void*h,void*p,D n,D*g,void*over){
 unsigned i=(unsigned)((uintptr_t)h-1);
 if(i>=500&&i<508){D size=backupSize[i-500];assert(offset<=size);*g=n<size-offset?n:size-offset;memcpy(p,backupBytes[i-500]+offset,*g);offset+=*g;return 1;}
 assert(i<252&&offset<=fsz[i]);
 *g=n<fsz[i]-offset?n:fsz[i]-offset;memcpy(p,vdisk[i]+offset,*g);offset+=*g;readCareer=1;return 1;
}
static int A writefile(void*h,const void*p,D n,D*g,void*over){
 if(probeMode&&n>5&&!memcmp(p,"frame",5)){
  assert((uintptr_t)h==400&&n<sizeof probeText&&frame==33);
  memcpy(probeText,p,n);probeBytes=*g=n;return 1;
 }
 assert((uintptr_t)h==400&&n>=128&&n<sizeof pending&&*(const D*)p==0x44433131);
 memcpy(pending,p,n);psize=*g=n;saved++;return 1;
}
static int A replace(const S*a,const S*b,D f){
 if(probeMode&&is_probe_file(b)){
  assert(a!=b&&f==9&&probeBytes&&stageName[0]&&!psize);
  int i=0;do{assert(stageName[i]==a[i]);}while(a[i++]);stageName[0]=0;
  const char*out=getenv("DC_FRAME_EXPORT");assert(out);FILE*fp=fopen(out,"wb");
  assert(fp&&fwrite(probeText,1,probeBytes,fp)==probeBytes&&!fclose(fp));probeWrites++;
  return 1;
 }
 assert(a!=b&&(f&8)&&psize&&stageName[0]);
 int k=0;do{assert(stageName[k]==a[k]);}while(a[k++]);stageName[0]=0;
 int bk=backup_slot(b,1);if(bk>=0){assert(!(f&1)&&!backupSize[bk]);backupSize[bk]=psize;memcpy(backupBytes[bk],pending,psize);safeWrites++;
  const char*out=getenv("DC_TRIP_BACKUP");if(out){FILE*fp=fopen(out,"wb");assert(fp&&fwrite(pending,1,psize,fp)==psize&&!fclose(fp));}return 1;}
 int i=pathslot(b);if(!(f&1)){assert(!fsz[i]);if(opening==2&&loadCareer)assert(i==2);creates++;}
 memcpy(vdisk[i],pending,psize);fsz[i]=psize;
 if(saveCareer){FILE*fp=fopen(saveCareer,"wb");assert(fp&&fwrite(pending,1,psize,fp)==psize);fclose(fp);}
 safeWrites++;return 1;
}
static int A draw(void*dc,int x,int y,int w,int h,int sx,int sy,int sw,int sh,const D *b,const void*fmt,D palette,D op){
 assert(sw==960&&sh==540&&op==0xcc0020);
 if(probeMode){int marker=b[21*960+480]==0x39545e&&b[480]==0x39545e;
  if(frame==0)assert(!marker);
  if(frame==13){assert(marker);probeCaptured++;}
 }
 if(presentMode)present_frame(x,y,w,h,b);
 else if(pointerScenario==2)assert(x==0&&y==0&&w==1920&&h==1080);
 else if(pointerScenario)assert(x==0&&y==29&&w==1300&&h==731);
 else assert(w==960&&h==540);
 frame++;
 if(atmTest&&frame==6)assert(b[96*960+193]==0x8cece0u); /* ATM mint title strip */
 if(tourFrames){if(frame<tourFrames)next_tour_keys();return sh;}
 checksum=2166136261u;for(int i=0;i<960*540;i+=23)checksum=(checksum^b[i])*16777619u;
 if(previous!=checksum)volume++;previous=checksum;
 if(pointerScenario){
   const int pinx=610,piny=334;unsigned t=b[piny*960+pinx];int purple=0;
   for(int yy=70;yy<450;yy++)for(int xx=270;xx<685;xx++)if(b[yy*960+xx]==0xc68aff||b[yy*960+xx]==0xe4c8f8)purple++;
   if(frame==11){beforePin=checksum;pointerPx=purple;}
   if(frame==25){afterPin=checksum;pointerPy=purple;}
   if(pointerScenario==1&&frame==29)assert(borderClick&&purple>pointerPx);
   if(frame==45){afterClear=checksum;clearedPx=purple;}
 }
 if(frame==7)menuFrame=checksum;if(frame==43)delivered=(checksum!=(unsigned)menuFrame);
 return sh;
}
/* Optional audio API. Capture bytes only in the audio-specific regression. */
static void *A load_radio(const S *name,void *h,D flags){
 const char *dll="winmm.dll";for(int i=0;i<=9;i++)assert(name[i]==(S)dll[i]);
 assert(!h&&flags==0x800u);return getenv("DC_RADIO_UNAVAILABLE")?NULL:(void*)7;
}
static int A play_radio(const unsigned char *wave,void *module,D flags){
 if(!wave){assert(!module&&!flags);return 1;} /* PlaySoundW stop */
 assert(!module&&(flags==7||flags==15)&&!memcmp(wave,"RIFF",4)&&!memcmp(wave+8,"WAVEfmt ",8)&&!memcmp(wave+36,"data",4));
 D size;memcpy(&size,wave+40,4);assert(size==112000||(flags==7&&size==40000));
 const char *path=getenv("DC_RADIO_CAPTURE");if(path){
  FILE *f=fopen(path,"ab");assert(f&&fwrite(wave+44,1,size,f)==size&&!fclose(f));}
 return 1;
}
static void *A radio_proc(void *h,const char *name){assert(h==(void*)7&&!strcmp(name,"PlaySoundW"));return play_radio;}
typedef struct{char*n;void*p;}Bind;
static const Bind bind[]={
 {"QueryPerformanceCounter",probe_counter},{"QueryPerformanceFrequency",probe_freq},{"LoadLibraryExW",load_radio},{"GetProcAddress",radio_proc},{"ExitProcess",die},{"GetTickCount64",ticks},{"GetEnvironmentVariableW",nil},{"GetAsyncKeyState",key},{"PeekMessageW",peek},
 {"GetClientRect",rect},{"CreateFileW",openfile},{"WriteFile",writefile},{"MoveFileExW",replace},{"DeleteFileW",removefile},{"StretchDIBits",draw},
 {"GetCurrentProcessId",ok},{"GetLastError",errorcode},{"PatBlt",patblt},{"TrackMouseEvent",ok},{"GetModuleHandleW",ok},{"Sleep",nil},{"CreateDirectoryW",ok},{"ReadFile",readfile},{"CloseHandle",ok},{"RegisterClassExW",regcls},
 {"MessageBoxW",close_prompt},{"CreateWindowExW",ok},{"DefWindowProcW",defproc},{"ShowWindow",ok},{"TranslateMessage",ok},{"DispatchMessageW",dispatchMsg},
 {"GetDC",ok},{"ReleaseDC",ok},{"LoadCursorW",ok},{"PostQuitMessage",close_post},{"SetProcessDPIAware",ok},{"GetForegroundWindow",ok}
};
static D d(const unsigned char*a,size_t p){D x;memcpy(&x,a+p,4);return x;}static S s(const unsigned char*a,size_t p){S x;memcpy(&x,a+p,2);return x;}static Q q(const unsigned char*a,size_t p){Q x;memcpy(&x,a+p,8);return x;}
int main(int argc,char**argv){assert(argc>=2&&argc<=4);probeMode=getenv("DC_FRAME_EXPORT")!=NULL;const char *mode=getenv("DC_OPENING");opening=mode?atoi(mode):0;mode=getenv("DC_CLOSE_TEST");closeTest=mode?atoi(mode):0;mode=getenv("DC_POINTER");pointerScenario=mode?atoi(mode):0;deleteTest=getenv("DC_DELETE")!=0;atmTest=getenv("DC_ATM_TEST")!=0;if(argc==3&&!strcmp(argv[2],"--tour")){
 tapMode=getenv("DC_TAP_REPLAY")!=NULL;const char *p=getenv("DC_PRESENT");if(p)presentMode=!strcmp(p,"verify")?2:1;
 loadCareer=getenv("DC_TRIP_LOAD");saveCareer=getenv("DC_TRIP_SAVE");
 if(getenv("DC_TRIP_SEED"))tourSeed=strtoul(getenv("DC_TRIP_SEED"),0,0);
 assert(scanf("%d %x",&tourFrames,&tourSaveHash)==2&&tourFrames>0);
 next_tour_keys();
 }else if(argc>=3&&strcmp(argv[2],"-"))loadCareer=argv[2];if(argc==4)saveCareer=argv[3];
 if(loadCareer){int i=opening==1?1:0;FILE*fixture=fopen(loadCareer,"rb");assert(fixture);fsz[i]=fread(vdisk[i],1,sizeof vdisk[i],fixture);assert(fsz[i]>0&&fsz[i]<sizeof vdisk[i]);fclose(fixture);}
 if(deleteTest){assert(fsz[0]);fsz[2]=fsz[0];memcpy(vdisk[2],vdisk[0],fsz[2]);}
 FILE*f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);unsigned char*file=malloc(n);assert(file&&fread(file,1,n,f)==(size_t)n);fclose(f);
 D pe=d(file,60),opt=pe+24;assert(d(file,pe)==0x4550&&s(file,pe+4)==0x8664&&s(file,opt)==0x20b);
 Q base=q(file,opt+24);D size=d(file,opt+56);unsigned char*img=mmap((void*)base,size,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED_NOREPLACE,-1,0);assert(img==(void*)base);
 D o=opt+s(file,pe+20);for(int i=0;i<s(file,pe+6);i++,o+=40){D vs=d(file,o+8),rva=d(file,o+12),raw=d(file,o+16),off=d(file,o+20);assert(rva+vs<=size&&off+raw<=(Q)n);if(raw)memcpy(img+rva,file+off,raw);}
 D imp=d(file,opt+120);assert(imp);for(D t=imp;d(img,t);t+=20){D lookup=d(img,t),iat=d(img,t+16);for(int i=0;q(img,lookup+i*8);i++){const char*name=(const char*)img+q(img,lookup+i*8)+2;void*fn=0;for(int j=0;j<(int)(sizeof bind/sizeof *bind);j++)if(!strcmp(name,bind[j].n))fn=bind[j].p;if(!fn)fprintf(stderr,"Unmapped import: %s\n",name);assert(fn);memcpy(img+iat+i*8,&fn,8);}}
 if(!setjmp(jb)){typedef void (A *WinEntry)(void);WinEntry start=(WinEntry)(img+d(file,opt+16));start();assert(!"PE entrypoint returned unexpectedly");}
 if(probeMode){assert(probeWrites==1&&probeCaptured==1&&probeDenied==1&&!probeUnmodifiedDefault&&probeAltDefault==1);printf("Windows diagnostic probe: no auto-write, retry exported %u bytes.\n",probeBytes);}
 if(closeTest){
  printf("Windows simulated close: attempts=%d prompts=%d committed=%d.\n",closeTried,closePrompts,safeWrites);
  assert(closePrompts==1&&closePosted&&((closeTest==1&&closeTried==2&&safeWrites==1)||(closeTest==2&&closeTried==1&&!safeWrites)));
  return 0;
 }
 assert(saved>0&&safeWrites==saved&&(!loadCareer||readCareer));
 if(tourFrames){
  if(tapMode){printf("Buffered Windows keys: %d press/release pairs between ticks.\n",tapCount);assert(tapCount>0);}
  if(presentMode){
   printf("GDI presentation: %d blits, %d black fills, %d overlaps, %d wheel frames.\n",presentCalls,presentFills,presentOverlap,presentWheels);
   if(presentMode==2)assert(presentCalls==tourFrames&&presentFills>=4&&presentWheels>=6);
  }
  assert(fsz[0]>=4);
  D actual=d(vdisk[0],fsz[0]-4);
  printf("Street tour: %d frames, %d atomic writes, final save %08x (expected %08x).\n",frame,safeWrites,actual,tourSaveHash);
  if(loadCareer){assert(!creates&&saveCareer&&(!tourSaveHash||actual==tourSaveHash)&&!tourHold);}
  else assert(creates==1&&actual==tourSaveHash&&!tourHold);
  return 0;
 }
 if(atmTest){
  assert(!creates&&safeWrites==3&&fsz[0]==304&&d(vdisk[0],4)==10);
  assert(d(vdisk[0],8)==250&&d(vdisk[0],136)==900&&d(vdisk[0],288)==100&&d(vdisk[0],292)==0&&d(vdisk[0],296)==0);
  puts("ATM PE entry point: E opened, deposited $100, withdrew $50, exited, saved.");
  return 0;
 }
 if(deleteTest){assert(deleteStage==2&&deleteCalls==1&&fsz[0]&&!fsz[2]&&!creates);puts("Native entrypoint: cancel, confirm, exact slot deleted, other slot still loads and saves.");return 0;}
 if(!loadCareer||opening==2){assert(creates==1&&createStage==6&&saved>=2);
   int slot=opening==2&&loadCareer?2:0;
   assert(fsz[slot]==304&&d(vdisk[slot],4)==10&&!memcmp(vdisk[slot]+268,"Vanta LocalQA:",14));
 }else assert(!creates);
 if(pointerScenario){assert(pointerStage==2&&beforePin!=afterPin&&afterPin!=afterClear&&pointerPy>pointerPx&&clearedPx==pointerPx);printf("Pointer clicks map correctly at %s.\n",pointerScenario==2?"1920x1080":"1300x790 letterboxed; border click ignored");}
 else assert(volume>5&&delivered);printf("Exact Windows x64 EXE: 75 frames via native entry, menu input, walking, frame output, and atomic save call verified against Win32 stubs.\n");
}
