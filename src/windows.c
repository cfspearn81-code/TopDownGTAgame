/* Native Win32 window, DIB rendering, keyboard, and local save. No runtime DLL bundle. */
#include "game.h"
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
F(U,GetLastError,void);F(U,GetEnvironmentVariableW,const U16*,U16*,U);F(int,CreateDirectoryW,const U16*,void*);
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
static U16 savePath[1024],tmpPath[1024];static unsigned directoryEnd;
static unsigned add(U16*d,unsigned pos,const U16*s){while(*s)d[pos++]=*s++;d[pos]=0;return pos;}
static void make_fresh_path(int i){
 U16 n[16]=L"\\fresh-000.dat",t[16]=L"\\fresh-000.tmp";
 n[7]=t[7]='0'+i/100;n[8]=t[8]='0'+i/10%10;n[9]=t[9]='0'+i%10;
 add(savePath,directoryEnd,n);add(tmpPath,directoryEnd,t);
}
void disk_slot(int slot){
 if(slot>=2){make_fresh_path(slot-1);return;}
 add(savePath,directoryEnd,slot?L"\\opening.dat":L"\\career.dat");
 add(tmpPath,directoryEnd,slot?L"\\opening.tmp":L"\\career.tmp");
}
int disk_present(void){
 Handle h=__imp_CreateFileW(savePath,0x80000000u,1,0,3,0x80,0);
 if(h!=(Handle)-1){__imp_CloseHandle(h);return 1;}
 U e=__imp_GetLastError();return e!=2&&e!=3; /* Never reuse unreadable or locked saves. */
}
int disk_delete(void){return __imp_DeleteFileW(savePath);}
static void directories(void){
 U n=__imp_GetEnvironmentVariableW(L"LOCALAPPDATA",savePath,900);
 if(!n||n>=900)n=__imp_GetEnvironmentVariableW(L"USERPROFILE",savePath,900);
 if(!n||n>=900){n=1;savePath[0]='.';}
 n=add(savePath,n,L"\\DeadChannel");__imp_CreateDirectoryW(savePath,0);
 for(U i=0;i<=n;i++)tmpPath[i]=savePath[i];
 directoryEnd=n;disk_slot(0);
}
int disk_read(void*d,unsigned n){
 Handle h=__imp_CreateFileW(savePath,0x80000000u,1,0,3,0x80,0);if(h==(Handle)-1)return 0;
 U got=0,extra=0;unsigned char byte;
 int r=__imp_ReadFile(h,d,n,&got,0)&&got==n&&__imp_ReadFile(h,&byte,1,&extra,0)&&!extra;
 __imp_CloseHandle(h);return r;
}
static int write_slot(const void*d,unsigned n,U flags){
 Handle h=__imp_CreateFileW(tmpPath,0x40000000u,0,0,2,0x80,0);if(h==(Handle)-1)return 0;
 U got=0;int r=__imp_WriteFile(h,d,n,&got,0);r=__imp_CloseHandle(h)&&r;return r&&got==n&&__imp_MoveFileExW(tmpPath,savePath,flags|8);
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
static void mouse_message(Handle w,S64 packed,U buttons){
 Rect r;if(!__imp_GetClientRect(w,&r))return;
 View v=framebuffer(r);
 const int x=(short)(packed&65535)-v.x,y=(short)((packed>>16)&65535)-v.y;
 if(x>=0&&y>=0&&x<v.w&&y<v.h)pointer_input((int)((long long)x*W/v.w),(int)((long long)y*H/v.h),buttons);
 else pointer_input(-1,-1,buttons);
}
static int redraw,mouseTracked;
static S64 wnd(Handle window,U m,U64 w,S64 l){
 if(m==0x0002){request_save();__imp_PostQuitMessage(0);return 0;}
 if(m==0x000f||m==0x0005||m==0x0006)redraw=1;
 if(m==0x02a3){mouseTracked=0;pointer_input(-1,-1,0);return 0;}
 if(m==0x0200||m==0x0201||m==0x0204){
   if(!mouseTracked){struct {U len,flags;Handle window;U hover;} request={24,2,window,0};mouseTracked=__imp_TrackMouseEvent(&request);}
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
 if(key(0x2e))r|=DELETE_SLOT;if(key(0x74))r|=SAVE;if(key(0x0d))r|=ENTER;if(key(0x70))r|=HELP;if(key(0x71))r|=GFX;if(key(0x72))r|=SLOT;if(key(0x73))r|=NEW_SLOT;if(key('X'))r|=PUNCH;if(key('T'))r|=TAXI_NEXT;return r;
}
static int redraw=1;
static Bitmap dib={40,W,-H,1,32,0,0,0,0,0,0,0};
void entry(void){
 __imp_SetProcessDPIAware();Handle instance=__imp_GetModuleHandleW(0);
 static const U16 cls[]=L"DeadChannelNative11";
 Class k={sizeof(Class),0x23,wnd,0,0,instance,0,__imp_LoadCursorW(0,(void*)32512),0,0,cls,0};
 if(!__imp_RegisterClassExW(&k)){__imp_ExitProcess(1);return;}
 Handle win=__imp_CreateWindowExW(0,cls,L"Dead Channel | Street Handling",0x00cf0000,(-2147483647-1),0,1152,690,0,0,instance,0);
 if(!win){__imp_ExitProcess(2);return;}
 directories();set_session_seed((U)__imp_GetTickCount64());init_city();__imp_ShowWindow(win,1);
 U64 t=__imp_GetTickCount64(); unsigned acc=0;int wasFocused=1,lastW=0,lastH=0;
 for(;;){Msg m;while(__imp_PeekMessageW(&m,0,0,0,1)){if(m.msg==0x0012){__imp_ExitProcess((U)m.wp);return;}__imp_TranslateMessage(&m);__imp_DispatchMessageW(&m);}
  U64 now=__imp_GetTickCount64(), elapsed=now-t;t=now;if(elapsed>60)elapsed=60;acc+=(unsigned)elapsed*60;
  int focus=__imp_GetForegroundWindow()==win,update=0;
  if(!focus&&wasFocused){clear_input();acc=0;}wasFocused=focus;
  while(acc>=1000){if(focus)tick(input());acc-=1000;update=1;}
  if((update&&focus)||redraw){int repaint=redraw;redraw=0;Rect r;if(__imp_GetClientRect(win,&r)&&r.r>0&&r.b>0){
   View v=framebuffer(r);int sw=v.w,sh=v.h;
   if(sw>0&&sh>0){paint();Handle dc=__imp_GetDC(win);
    if(dc){if((sw!=r.r||sh!=r.b)&&(repaint||r.r!=lastW||r.b!=lastH))__imp_PatBlt(dc,0,0,r.r,r.b,0x00000042);
      __imp_StretchDIBits(dc,v.x,v.y,sw,sh,0,0,W,H,pixels,&dib,0,0x00cc0020);__imp_ReleaseDC(win,dc);
      lastW=r.r;lastH=r.b;}}
  }}
  __imp_Sleep(focus?2:15);
 }
}
