/* Native Win32 window, DIB rendering, keyboard, and local save. No runtime DLL bundle. */
#include "game.h"
#include "radio_motif.h"
typedef unsigned long long U64; typedef unsigned short U16;
typedef void *Handle; typedef long long S64;
typedef struct{int x,y;} Point;
typedef struct{int l,t,r,b;} Rect;
typedef struct{Handle window;unsigned msg;U64 wp;S64 lp;U time;Point point;unsigned unused;} Msg;
typedef S64 (*Proc)(Handle,U,U64,S64);
typedef struct{U size,style;Proc proc;int cls,win;Handle module,icon,cursor,bg;const U16 *menu,*name;Handle smallicon;} Class;
typedef struct{U size;int w,h;U16 planes,bits;U compression,bytes;int xp,yp;U colors,important;U rgb;} Bitmap;
#define F(t,n,...) extern t (*__imp_##n)(__VA_ARGS__)
F(void,ExitProcess,U);F(Handle,GetModuleHandleW,const U16*);F(void,Sleep,U);F(U64,GetTickCount64,void);
F(U,GetLastError,void);F(U,GetCurrentProcessId,void);F(U,GetEnvironmentVariableW,const U16*,U16*,U);F(int,CreateDirectoryW,const U16*,void*);
F(Handle,CreateFileW,const U16*,U,U,void*,U,U,Handle);F(int,ReadFile,Handle,void*,U,U*,void*);
F(int,WriteFile,Handle,const void*,U,U*,void*);F(int,CloseHandle,Handle);F(int,MoveFileExW,const U16*,const U16*,U);F(int,DeleteFileW,const U16*);
F(U16,RegisterClassExW,Class*);F(Handle,CreateWindowExW,U,const U16*,const U16*,U,int,int,int,int,Handle,Handle,Handle,void*);
F(S64,DefWindowProcW,Handle,U,U64,S64);F(int,ShowWindow,Handle,int);
F(int,PeekMessageW,Msg*,Handle,U,U,U);F(int,TranslateMessage,const Msg*);F(S64,DispatchMessageW,const Msg*);
F(short,GetAsyncKeyState,int);F(int,GetClientRect,Handle,Rect*);F(Handle,GetDC,Handle);F(int,ReleaseDC,Handle,Handle);
F(Handle,LoadCursorW,Handle,const U16*);F(void,PostQuitMessage,int);F(int,SetProcessDPIAware,void);F(Handle,GetForegroundWindow,void);
F(int,TrackMouseEvent,void*);
F(int,PatBlt,Handle,int,int,int,int,U);
F(int,StretchDIBits,Handle,int,int,int,int,int,int,int,int,const void*,const Bitmap*,U,U);
F(Handle,LoadLibraryExW,const U16*,Handle,U);
F(void*,GetProcAddress,Handle,const char*);
static int phone_browse(void){
 typedef Handle (*Open)(Handle,const U16*,const U16*,const U16*,const U16*,int);
 static Open f;static int loaded;
 if(!loaded){
  loaded=1;Handle shell=__imp_LoadLibraryExW(L"shell32.dll",0,0x800);
  if(shell)f=(Open)__imp_GetProcAddress(shell,"ShellExecuteW");
 }
 return f&&(U64)f(0,L"open",L"https://music.youtube.com/",0,0,1)>32;
}
static void radio_audio(int oneShot,int station,U64 now){
 static int active;static U64 transientUntil;
 int track=oneShot?oneShot:station;
 if(!track&&!active)return;
 if(!oneShot&&(transientUntil>now||track==active))return;
 if(oneShot)transientUntil=now+3500;
 active=track;
 typedef int (*PlayFn)(const void*,Handle,U);
 static int resolved;static PlayFn play;
 if(!resolved){
  resolved=1; /* System32 only. Missing multimedia support is not fatal. */
  Handle lib=__imp_LoadLibraryExW(L"winmm.dll",0,0x800);
  if(lib)play=(PlayFn)__imp_GetProcAddress(lib,"PlaySoundW");
 }
 if(play){
  if(!track){play(0,0,0);return;}
  const RadioWave *w=rm_record(track);
  if(w)play(w,0,oneShot?7:15); /* memory | async | nodefault | optional loop */
 }
}
static U16 savePath[1024];static unsigned directoryEnd,stageSequence;
static unsigned add(U16*d,unsigned pos,const U16*s){while(*s)d[pos++]=*s++;d[pos]=0;return pos;}
static void make_fresh_path(int i){
 U16 n[16]=L"\\fresh-000.dat";
 n[7]='0'+i/100;n[8]='0'+i/10%10;n[9]='0'+i%10;
 add(savePath,directoryEnd,n);
}
void disk_slot(int slot){
 if(slot>=2){make_fresh_path(slot-1);return;}
 add(savePath,directoryEnd,slot?L"\\opening.dat":L"\\career.dat");
}
int disk_present(void){
 Handle h=__imp_CreateFileW(savePath,0x80000000u,5,0,3,0x80,0);
 if(h!=(Handle)-1){__imp_CloseHandle(h);return 1;}
 U e=__imp_GetLastError();return e!=2&&e!=3; /* Never reuse unreadable or locked saves. */
}
int disk_delete(void){return __imp_DeleteFileW(savePath);}
static void directories(void){
 U n=__imp_GetEnvironmentVariableW(L"LOCALAPPDATA",savePath,900);
 if(!n||n>=900)n=__imp_GetEnvironmentVariableW(L"USERPROFILE",savePath,900);
 if(!n||n>=900){n=1;savePath[0]='.';}
 n=add(savePath,n,L"\\DeadChannel");__imp_CreateDirectoryW(savePath,0);
 directoryEnd=n;disk_slot(0);
}
int disk_read(void*d,unsigned n){
 Handle h=__imp_CreateFileW(savePath,0x80000000u,5,0,3,0x80,0);if(h==(Handle)-1)return 0;
 U got=0,extra=0;unsigned char byte;
 int r=__imp_ReadFile(h,d,n,&got,0)&&got==n&&__imp_ReadFile(h,&byte,1,&extra,0)&&!extra;
 __imp_CloseHandle(h);return r;
}
/* Read handles share DELETE so another copy scanning the catalog cannot
 * prevent an atomic rename. A committed save is never modified in place. */
static U put_hex8(U16 *p,U at,U n){
 static const char hex[]="0123456789abcdef";
 for(int shift=28;shift>=0;shift-=4)p[at++]=(U16)hex[(n>>shift)&15];
 return at;
}
static int write_slot(const void*d,unsigned n,U flags){
 /* Reserve our own stage. A fixed slot.tmp lets a second process replace
  * the bytes after our CloseHandle and before our MoveFileExW. Never reuse,
  * truncate or erase an abandoned temp, including after PID recycling. */
 /* Keep the basename shorter than even career.dat: a long Windows user
  * profile which fit the old save must still fit the stage. */
 U16 stage[1024];for(U i=0;i<directoryEnd;i++)stage[i]=savePath[i];
 U at=add(stage,directoryEnd,L"\\~"),pid=__imp_GetCurrentProcessId();
 for(int retry=0;retry<64;retry++){
  stage[put_hex8(stage,at,pid+(++stageSequence)*0x9e3779b9u)]=0;
  Handle h=__imp_CreateFileW(stage,0x40000000u,0,0,1,0x80,0); /* CREATE_NEW */
  if(h==(Handle)-1){
   U error=__imp_GetLastError();
   if(error==80||error==183)continue;
   return 0;
  }
  U got=0;int ok=__imp_WriteFile(h,d,n,&got,0)&&got==n;
  if(!__imp_CloseHandle(h))ok=0;
  if(ok&&__imp_MoveFileExW(stage,savePath,flags|8))return 1;
  __imp_DeleteFileW(stage); /* Own stage only; the prior career is intact. */
  return 0;
 }
 return 0;
}
int disk_write(const void*d,unsigned n){return write_slot(d,n,1);}
int disk_create(const void*d,unsigned n){return write_slot(d,n,0);}
typedef struct{int x,y,w,h;} View;
static View framebuffer(Rect r){
 View v={0,0,r.r,r.b};
 if(r.r<=0||r.b<=0){v.w=v.h=0;return v;}
 if((long long)v.w*H>(long long)v.h*W)v.w=(int)((long long)v.h*W/H);
 else v.h=(int)((long long)v.w*H/W);
 v.x=(r.r-v.w)/2;v.y=(r.b-v.h)/2;return v;
}
/* Never erase game pixels to draw the letterbox. GDI can expose separate
 * operations during a redraw; the old full-client BLACKNESS was visible
 * before the next StretchDIBits, particularly under wheel mouse motion. */
static void letterbox_bars(Handle dc,Rect r,View v){
 int right=v.x+v.w,bottom=v.y+v.h;
 if(v.y>0)__imp_PatBlt(dc,0,0,r.r,v.y,0x00000042);
 if(bottom<r.b)__imp_PatBlt(dc,0,bottom,r.r,r.b-bottom,0x00000042);
 if(v.x>0)__imp_PatBlt(dc,0,v.y,v.x,v.h,0x00000042);
 if(right<r.r)__imp_PatBlt(dc,right,v.y,r.r-right,v.h,0x00000042);
}
static void mouse_message(Handle w,S64 packed,U buttons){
 Rect r;if(!__imp_GetClientRect(w,&r))return;
 View v=framebuffer(r);
 const int x=(short)(packed&65535)-v.x,y=(short)((packed>>16)&65535)-v.y;
 if(x>=0&&y>=0&&x<v.w&&y<v.h)pointer_input((int)((long long)x*W/v.w),(int)((long long)y*H/v.h),buttons);
 else pointer_input(-1,-1,buttons);
}
static int redraw=1,barsDirty=1,mouseTracked,focusWasInterrupted;
static S64 wnd(Handle window,U m,U64 w,S64 l){
 if(m==0x0002){request_save();__imp_PostQuitMessage(0);return 0;}
 /* No class background brush: DefWindowProc validates WM_PAINT and
  * this host handles any later WM_ERASEBKGND without clearing the view. */
 if(m==0x000f||m==0x0005)redraw=barsDirty=1;
 if(m==0x0006)redraw=1;
 if(m==0x0008){clear_input();pointer_input(-1,-1,0);focusWasInterrupted=1;redraw=1;}
 if(m==0x02a3){mouseTracked=0;pointer_input(-1,-1,0);return 0;}
 if(m==0x0200||m==0x0201||m==0x0204){
   if(!mouseTracked){struct {U len,flags;Handle window;U hover;} request={24,2,window,0};mouseTracked=__imp_TrackMouseEvent(&request);}
   /* Render pointer changes with the next 60 Hz frame, not once per
      mouse packet. The selected position is still updated on every event. */
   mouse_message(window,l,m==0x0201?1u:m==0x0204?2u:0u);return 0;
 }
 if(m==0x0102){text_input((U)w);return 0;}
 if(m==0x0014)return 1;
 return __imp_DefWindowProcW(window,m,w,l);
}
static int key(int v){return (__imp_GetAsyncKeyState(v)&0x8000)!=0;}
static U input(void){U r=0;
 if(key('W')||key(0x26))r|=UP;if(key('S')||key(0x28))r|=DOWN;if(key('A')||key(0x25))r|=LEFT;if(key('D')||key(0x27))r|=RIGHT;
 if(key('E'))r|=USE;if(key(0x20))r|=FIRE;if(key(0x10))r|=BRAKE;if(key(0x1b))r|=ESC;if(key('M'))r|=MAP;
 if(key(0x2e))r|=DELETE_SLOT;if(key(0x74))r|=SAVE;if(key(0x75))r|=ACTIVITIES;if(key(0x76))r|=PHONE_ALT;if(key('P'))r|=PHONE;if(key('C'))r|=CAMERA;if(key('L'))r|=LOADOUT;if(key(0x77))r|=ATLAS_WORLD;if(key(0x0d))r|=ENTER;if(key(0x70))r|=HELP;if(key(0x71))r|=GFX;if(key(0x72))r|=SLOT;if(key(0x73))r|=NEW_SLOT;if(key('X'))r|=PUNCH;if(key('T'))r|=TAXI_NEXT;if(key(0x09))r|=WHEEL;if(key('V'))r|=USE_PATCH;return r;
}
static Bitmap dib={40,W,-H,1,32,0,0,0,0,0,0,0};
void entry(void){
 __imp_SetProcessDPIAware();Handle instance=__imp_GetModuleHandleW(0);
 static const U16 cls[]=L"DeadChannelNative11";
 Class k={sizeof(Class),0x23,wnd,0,0,instance,0,__imp_LoadCursorW(0,(void*)32512),0,0,cls,0};
 if(!__imp_RegisterClassExW(&k)){__imp_ExitProcess(1);return;}
 Handle win=__imp_CreateWindowExW(0,cls,L"Dead Channel | Port Vanta Crossroads",0x00cf0000,(-2147483647-1),0,1152,690,0,0,instance,0);
 if(!win){__imp_ExitProcess(2);return;}
 directories();set_session_seed((U)__imp_GetTickCount64());init_city();__imp_ShowWindow(win,1);
 U64 t=__imp_GetTickCount64(); unsigned acc=0;int wasFocused=1,lastW=0,lastH=0;U focusBlocked=0;
 for(;;){Msg m;while(__imp_PeekMessageW(&m,0,0,0,1)){if(m.msg==0x0012){__imp_ExitProcess((U)m.wp);return;}__imp_TranslateMessage(&m);__imp_DispatchMessageW(&m);}
  U64 now=__imp_GetTickCount64(), elapsed=now-t;t=now;if(elapsed>60)elapsed=60;acc+=(unsigned)elapsed*60;
  int focus=__imp_GetForegroundWindow()==win,update=0;
  if(!focus&&wasFocused){clear_input();pointer_input(-1,-1,0);acc=0;redraw=1;}
  if(focus&&(!wasFocused||focusWasInterrupted)){focusBlocked=input();focusWasInterrupted=0;}wasFocused=focus;
  while(acc>=1000){if(focus){U down=input();focusBlocked&=down;tick(down&~focusBlocked);}acc-=1000;update=1;}
  if((update&&focus)||redraw){redraw=0;Rect r;if(__imp_GetClientRect(win,&r)&&r.r>0&&r.b>0){
   View v=framebuffer(r);int sw=v.w,sh=v.h;
   if(sw>0&&sh>0){if(take_phone_link())phone_link_result(phone_browse());radio_audio(take_radio_record(),night_music_track(),now);paint();Handle dc=__imp_GetDC(win);
    if(dc){
      if(barsDirty||r.r!=lastW||r.b!=lastH){letterbox_bars(dc,r,v);barsDirty=0;}
      __imp_StretchDIBits(dc,v.x,v.y,sw,sh,0,0,W,H,pixels,&dib,0,0x00cc0020);__imp_ReleaseDC(win,dc);
      lastW=r.r;lastH=r.b;}}
  }}
  __imp_Sleep(focus?2:15);
 }
}
