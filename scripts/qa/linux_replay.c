/* Test-only SDL interposer. It queues events through the actual runtime and
 * advances the game's host clock by one fixed step per presented frame. */
#define _GNU_SOURCE
#include "../../src/game.h"
#include "../../src/sdl2_linux.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *plan;
static SDL_Window *test_window;
static Uint64 calls;
static unsigned mode,frame,frames,remaining,mask;
static U held;
static unsigned unpinned;
static const struct{int code;U mask;} binding[]={
 {SDL_SCANCODE_W,UP},{SDL_SCANCODE_S,DOWN},{SDL_SCANCODE_A,LEFT},{SDL_SCANCODE_D,RIGHT},
 {SDL_SCANCODE_E,USE},{SDL_SCANCODE_SPACE,FIRE},{SDL_SCANCODE_LSHIFT,BRAKE},
 {SDL_SCANCODE_ESCAPE,ESC},{SDL_SCANCODE_M,MAP},{SDL_SCANCODE_F6,ACTIVITIES},{SDL_SCANCODE_F5,SAVE},{SDL_SCANCODE_RETURN,ENTER},
 {SDL_SCANCODE_F1,HELP},{SDL_SCANCODE_F2,GFX},{SDL_SCANCODE_F3,SLOT},{SDL_SCANCODE_F4,NEW_SLOT},
 {SDL_SCANCODE_TAB,WHEEL},{SDL_SCANCODE_V,USE_PATCH},{SDL_SCANCODE_X,PUNCH},{SDL_SCANCODE_T,TAXI_NEXT},{SDL_SCANCODE_DELETE,DELETE_SLOT}
};
static void emit(SDL_Event e){assert(SDL_PushEvent(&e)==1);}
static int host(void *ret){
 Dl_info info;
 return dladdr(ret,&info)&&info.dli_fname&&strstr(info.dli_fname,"DeadChannel");
}
Uint64 SDL_GetPerformanceFrequency(void){
 if(host(__builtin_return_address(0)))return 60000;
 Uint64(*f)(void)=dlsym(RTLD_NEXT,"SDL_GetPerformanceFrequency");return f();
}
Uint64 SDL_GetPerformanceCounter(void){
 if(host(__builtin_return_address(0))){unsigned n=calls++,seed=getenv("DC_TRIP_SEED")?(unsigned)strtoul(getenv("DC_TRIP_SEED"),0,0):17;return n<2?seed:seed+1000*(n-1);}
 Uint64(*f)(void)=dlsym(RTLD_NEXT,"SDL_GetPerformanceCounter");return f();
}
static void key(int code,int down){
 SDL_Event e={.type=down?SDL_KEYDOWN:SDL_KEYUP};
 e.key.state=down;e.key.keysym.scancode=code;emit(e);
}
static void set_keys(U mask){
 for(unsigned i=0;i<sizeof binding/sizeof *binding;i++)if((held^mask)&binding[i].mask)
  key(binding[i].code,!!(mask&binding[i].mask));
 held=mask;
}
static void focus(int active){
 SDL_Event e={.type=SDL_WINDOWEVENT};
 e.window.event=active?SDL_WINDOWEVENT_FOCUS_GAINED:SDL_WINDOWEVENT_FOCUS_LOST;
 emit(e);
}
static void click(SDL_Renderer*r,int x,int y,int right){
 int w,h;assert(SDL_GetRendererOutputSize(r,&w,&h)==0);
 int vw=w,vh=h;if(w*H>h*W)vw=h*W/H;else vh=w*H/W;
 SDL_Event e={.type=SDL_MOUSEBUTTONDOWN};
 e.button.x=(w-vw)/2+(x*vw+W-1)/W;
 e.button.y=(h-vh)/2+(y*vh+H-1)/H;
 e.button.button=right?SDL_BUTTON_RIGHT:SDL_BUTTON_LEFT;e.button.state=1;emit(e);
}
static void text(const char*s){
 SDL_Event e={.type=SDL_TEXTINPUT};snprintf(e.text.text,sizeof e.text.text,"%s",s);emit(e);
}
static void verify_frame(SDL_Renderer*r){
 int w,h;assert(SDL_GetRendererOutputSize(r,&w,&h)==0);
 assert(w==1152&&h==690);
 SDL_Rect a={w/2,0,1,1},b={w/2,h/2,1,1};
 uint32_t edge=1,center=0;
 assert(SDL_RenderReadPixels(r,&a,SDL_PIXELFORMAT_ARGB8888,&edge,4)==0);
 assert(SDL_RenderReadPixels(r,&b,SDL_PIXELFORMAT_ARGB8888,&center,4)==0);
 assert((edge&0xffffff)==0&&(center&0xffffff)!=0);
}
static unsigned violet(SDL_Renderer*r){
 static uint32_t sample[740*680];
 SDL_Rect crop={280,45,640,690-45};
 assert(SDL_RenderReadPixels(r,&crop,SDL_PIXELFORMAT_ARGB8888,sample,crop.w*4)==0);
 unsigned count=0;
 for(int i=0;i<crop.w*crop.h;i++)count+=(sample[i]&0xffffff)==0xc68aff;
 return count;
}
SDL_Window *SDL_CreateWindow(const char *title,int x,int y,int w,int h,Uint32 flags){
 SDL_Window *(*real)(const char*,int,int,int,int,Uint32)=dlsym(RTLD_NEXT,"SDL_CreateWindow");
 assert(real&&(flags&SDL_WINDOW_RESIZABLE));test_window=real(title,x,y,w,h,flags);return test_window;
}
static uint32_t margin(SDL_Renderer*r,int x,int y){
 int w,h;assert(SDL_GetRendererOutputSize(r,&w,&h)==0);
 int vw=w,vh=h;if(w*H>h*W)vw=h*W/H;else vh=w*H/W;
 SDL_Rect a={(w-vw)/2+(x*vw+W-1)/W,(h-vh)/2+(y*vh+H-1)/H,1,1};
 uint32_t c=0;assert(SDL_RenderReadPixels(r,&a,SDL_PIXELFORMAT_ARGB8888,&c,4)==0);
 return c&0xffffff;
}
static void snapshot_frame(SDL_Renderer*r,const char*name){
 const char *dir=getenv("DC_LINUX_SHOW");
 if(!dir)return;
 int w,h;assert(!SDL_GetRendererOutputSize(r,&w,&h));
 uint32_t *buf=malloc((size_t)w*h*sizeof(*buf));assert(buf);
 assert(!SDL_RenderReadPixels(r,NULL,SDL_PIXELFORMAT_ARGB8888,buf,w*4));
 char target[1024];assert(snprintf(target,sizeof target,"%s/%s.ppm",dir,name)<(int)sizeof target);
 FILE *out=fopen(target,"wb");assert(out);fprintf(out,"P6\n%d %d\n255\n",w,h);
 for(int i=0;i<w*h;i++){fputc(buf[i]>>16,out);fputc(buf[i]>>8,out);fputc(buf[i],out);}
 assert(!fclose(out));free(buf);
}
static unsigned guide(SDL_Renderer*r){
 static uint32_t apt;
 if(frame==0||frame==3)return ENTER;
 if(frame==6){snapshot_frame(r,"apartment");apt=margin(r,20,277);return HELP;}
 if(frame==7){
  snapshot_frame(r,"apartment-guide");
  uint32_t expected=((apt>>1)&0x7f7f7fu)+0x050a0du;
  assert(margin(r,20,277)==expected);
 }
 if(frame==9)return ESC;
 if(frame==10)assert(margin(r,20,277)==apt);
 return 0;
}
static unsigned warm(SDL_Renderer*r){
 /* Use the mouse to load the existing career; M, clicks, Escape and the
  * focus pair exercise input mapping without relying on X11. */
 if(frame==1){SDL_SetWindowSize(test_window,1300,790);click(r,788,369,0);}
 if(frame==5){int w,h;assert(SDL_GetRendererOutputSize(r,&w,&h)==0&&w==1300&&h==790);unpinned=violet(r);}
 if(frame==10){
  assert(violet(r)>unpinned);
  SDL_Event e={.type=SDL_MOUSEBUTTONDOWN};
  e.button.x=500;e.button.y=2;e.button.button=SDL_BUTTON_RIGHT;e.button.state=1;emit(e);
 }
 if(frame==11)assert(violet(r)>unpinned);
 if(frame==13)assert(violet(r)==unpinned);
 if(frame==4||frame==25)return MAP;
 if(frame==9)click(r,570,302,0);
 if(frame==12)click(r,570,302,1);
 if(frame==28)return UP;
 if(frame>28&&frame<48)return UP;
 if(frame==48){
  focus(0);focus(1);held=0;
  SDL_Event repeat={.type=SDL_KEYDOWN};repeat.key.repeat=1;
  repeat.key.keysym.scancode=SDL_SCANCODE_W;emit(repeat);
 }
 if(frame==58)return SAVE;
 return 0;
}
static unsigned create(SDL_Renderer*r){
 if(frame==0||frame==6||frame==9)return ENTER;
 if(frame==3){click(r,377,151,0);text(" Linux");}
 if(frame==4)key(SDL_SCANCODE_BACKSPACE,1);
 if(frame==5)key(SDL_SCANCODE_BACKSPACE,0);
 if(frame>=14&&frame<=65)return DOWN;
 if(frame==68)return USE;
 if(frame==73)return SAVE;
 return 0;
}
void SDL_RenderPresent(SDL_Renderer *renderer){
 void(*real)(SDL_Renderer*)=dlsym(RTLD_NEXT,"SDL_RenderPresent");assert(real);
 if(!frames){
  plan=stdin;
  const char *kind=getenv("DC_LINUX_REPLAY");
  if(kind&&!strcmp(kind,"warm")){mode=1;frames=65;}
  else if(kind&&!strcmp(kind,"create")){mode=2;frames=80;}
  else if(kind&&!strcmp(kind,"resave")){mode=3;frames=24;}
  else if(kind&&!strcmp(kind,"delete")){mode=4;frames=24;}
  else if(kind&&!strcmp(kind,"guide")){mode=5;frames=18;}
  else{
   unsigned expected;
   assert(fscanf(plan,"%u %x",&frames,&expected)==2&&frames>0);
  }
  focus(1);verify_frame(renderer);
 }
 real(renderer);
 if(frame==frames){
  assert(mode||!remaining);
  const char *snapshot=getenv("DC_LINUX_SNAPSHOT");
  if(snapshot){
   const char* data=getenv("XDG_DATA_HOME");char name[1024];
   assert(data&&snprintf(name,sizeof name,"%s/DeadChannel/career.dat",data)<(int)sizeof name);
   FILE* f=fopen(name,"rb");uint8_t bytes[304];assert(f&&fread(bytes,sizeof bytes,1,f)==1);
   assert(fclose(f)==0);f=fopen(snapshot,"wb");assert(f&&fwrite(bytes,sizeof bytes,1,f)==1);
   assert(fclose(f)==0);
  }
  emit((SDL_Event){.type=SDL_QUIT});return;
 }
 if(mode==1)set_keys(warm(renderer));
 else if(mode==2)set_keys(create(renderer));
 else if(mode==3)set_keys(frame==2?ENTER:frame==17?SAVE:0);
 else if(mode==5)set_keys(guide(renderer));
 else if(mode==4)set_keys(frame==2?DELETE_SLOT:frame==5?RIGHT:frame==8?ENTER:0);
 else{
  if(!remaining)assert(fscanf(plan,"%u %u",&remaining,&mask)==2&&remaining);
  remaining--;set_keys(mask);
 }
 frame++;
}

int SDL_QueueAudio(SDL_AudioDeviceID dev,const void *data,Uint32 n){
 int (*real)(SDL_AudioDeviceID,const void *,Uint32)=dlsym(RTLD_NEXT,"SDL_QueueAudio");
 assert(real&&n==112000);int result=real(dev,data,n);const char *path=getenv("DC_RADIO_CAPTURE");
 if(!result&&path){FILE *f=fopen(path,"ab");assert(f&&fwrite(data,1,n,f)==n&&!fclose(f));}
 return result;
}
