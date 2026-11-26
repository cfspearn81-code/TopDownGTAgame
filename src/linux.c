#define _POSIX_C_SOURCE 200809L
#include "game.h"
#include "linux_save.h"
#include "sdl2_linux.h"
#include "radio_motif.h"
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/* On unsupported older SDL installations only the external link is absent. */
extern int SDL_OpenURL(const char*) __attribute__((weak));

static const char title[]="Dead Channel | Port Vanta Crossroads";
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *surface_tex;
static Uint8 down[SDL_NUM_SCANCODES];
static U buffered;
static int focused,redraw,running=1;
static volatile sig_atomic_t stop_signal;

static const U binding[SDL_NUM_SCANCODES]={
 [SDL_SCANCODE_W]=UP,[SDL_SCANCODE_UP]=UP,[SDL_SCANCODE_S]=DOWN,[SDL_SCANCODE_DOWN]=DOWN,
 [SDL_SCANCODE_A]=LEFT,[SDL_SCANCODE_LEFT]=LEFT,[SDL_SCANCODE_D]=RIGHT,[SDL_SCANCODE_RIGHT]=RIGHT,
 [SDL_SCANCODE_E]=USE,[SDL_SCANCODE_SPACE]=FIRE,[SDL_SCANCODE_X]=PUNCH,
 [SDL_SCANCODE_LSHIFT]=BRAKE,[SDL_SCANCODE_RSHIFT]=BRAKE,[SDL_SCANCODE_ESCAPE]=ESC,
 [SDL_SCANCODE_M]=MAP,[SDL_SCANCODE_F5]=SAVE,[SDL_SCANCODE_F6]=ACTIVITIES,[SDL_SCANCODE_RETURN]=ENTER,[SDL_SCANCODE_KP_ENTER]=ENTER,
 [SDL_SCANCODE_F1]=HELP,[SDL_SCANCODE_F2]=GFX,[SDL_SCANCODE_F3]=SLOT,[SDL_SCANCODE_F4]=NEW_SLOT,
 [SDL_SCANCODE_T]=TAXI_NEXT,[SDL_SCANCODE_DELETE]=DELETE_SLOT,[SDL_SCANCODE_P]=PHONE,[SDL_SCANCODE_F7]=PHONE_ALT,
 [SDL_SCANCODE_C]=CAMERA,[SDL_SCANCODE_L]=LOADOUT,[SDL_SCANCODE_F8]=ATLAS_WORLD,[SDL_SCANCODE_TAB]=WHEEL,[SDL_SCANCODE_V]=USE_PATCH
};
static void stop(int sig){(void)sig;stop_signal=1;}
static void close_try(void){
 if(exit_save_ok()){running=0;return;}
 static const SDL_MessageBoxButtonData buttons[]={
  {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT|SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,0,"Keep playing"},
  {0,1,"Quit without saving"}
 };
 const SDL_MessageBoxData dialog={SDL_MESSAGEBOX_WARNING,window,"Save failed",
  "Your last save is intact, but the latest progress could not be written. Close again to retry.",
  2,buttons,NULL};
 int choice=0;
 if(SDL_ShowMessageBox(&dialog,&choice)==0&&choice==1)running=0;
 else fprintf(stderr,"Dead Channel: save failed; the game is still open.\n");
}

static SDL_Rect viewport(int w,int h){
 SDL_Rect v={0,0,w,h};
 if(w<=0||h<=0)return (SDL_Rect){0,0,0,0};
 if((int64_t)w*H>(int64_t)h*W)v.w=(int)((int64_t)h*W/H);
 else v.h=(int)((int64_t)w*H/W);
 v.x=(w-v.w)/2;v.y=(h-v.h)/2;
 return v;
}
static void pointer(int x,int y,U clicks){
 int ww,wh,dw,dh;SDL_GetWindowSize(window,&ww,&wh);
 if(ww>0&&wh>0&&x>=0&&x<ww&&y>=0&&y<wh&&
    SDL_GetRendererOutputSize(renderer,&dw,&dh)==0){
  SDL_Rect view=viewport(dw,dh);
  int px=(int)((int64_t)x*dw/ww)-view.x,py=(int)((int64_t)y*dh/wh)-view.y;
  if(px>=0&&py>=0&&px<view.w&&py<view.h){
   pointer_input((int)((int64_t)px*W/view.w),(int)((int64_t)py*H/view.h),clicks);return;
  }
 }
 pointer_input(-1,-1,clicks);
}
static void clear_keys(void){memset(down,0,sizeof down);buffered=0;clear_input();}
static U keys(void){
 U k=buffered;buffered=0;
 for(int i=0;i<SDL_NUM_SCANCODES;i++)if(down[i])k|=binding[i];
 return k;
}
static void event(const SDL_Event *e){
 switch(e->type){
 case SDL_QUIT:close_try();break;
 case SDL_WINDOWEVENT:
  switch(e->window.event){
   case SDL_WINDOWEVENT_CLOSE:close_try();break;
   case SDL_WINDOWEVENT_FOCUS_LOST:focused=0;clear_keys();pointer_input(-1,-1,0);redraw=1;break;
   case SDL_WINDOWEVENT_FOCUS_GAINED:focused=1;redraw=1;break;
   case SDL_WINDOWEVENT_LEAVE:pointer_input(-1,-1,0);redraw=1;break;
   case SDL_WINDOWEVENT_SIZE_CHANGED:case SDL_WINDOWEVENT_RESIZED:
   case SDL_WINDOWEVENT_EXPOSED:case SDL_WINDOWEVENT_RESTORED:redraw=1;break;
  }
  break;
 case SDL_KEYDOWN:case SDL_KEYUP:{
   int code=e->key.keysym.scancode;
   if(focused&&(unsigned)code<SDL_NUM_SCANCODES){
    int held=e->type==SDL_KEYDOWN;
    /* Focus loss discarded the previous press. A resumed OS repeat cannot
     * manufacture a fresh press (notably E) after returning to the window. */
    if(held&&e->key.repeat&&!down[code])break;
    if(held&&code==SDL_SCANCODE_BACKSPACE)text_input(8);
    if(held&&!e->key.repeat&&!down[code])buffered|=binding[code]&~WHEEL; /* Tab does not open on a completed tap. */
    down[code]=(Uint8)held;
   }
   break;
 }
 case SDL_TEXTINPUT:
  if(focused){
   /* The shared name editor persists printable ASCII, as on Windows. */
   for(unsigned i=0;i<sizeof e->text.text&&e->text.text[i];i++){
    unsigned char ch=(unsigned char)e->text.text[i];
    if(ch>=32&&ch<=126)text_input(ch);
   }
   redraw=1;
  }
  break;
 case SDL_MOUSEMOTION:
  if(focused){pointer(e->motion.x,e->motion.y,0);redraw=1;}
  break;
 case SDL_MOUSEBUTTONDOWN:
  if(focused){
   U click=e->button.button==SDL_BUTTON_LEFT?1u:e->button.button==SDL_BUTTON_RIGHT?2u:0u;
   if(click){pointer(e->button.x,e->button.y,click);redraw=1;}
  }
  break;
 }
}
/* The video host works even when the audio subsystem or device is absent. */
static SDL_AudioDeviceID radioDev;
static int radioActive,radioOneShot,radioUnavailable;
static void radio_audio(int oneShot,int station){
 int track=oneShot?oneShot:station;
 if(!radioDev&&!track)return;
 if(radioUnavailable)return;
 if(!radioDev){
  SDL_AudioSpec want={.freq=RM_RATE,.format=AUDIO_S16LSB,.channels=1,.samples=512};
  if(SDL_InitSubSystem(SDL_INIT_AUDIO)<0){radioUnavailable=1;return;}
  radioDev=SDL_OpenAudioDevice(NULL,0,&want,NULL,0);
  if(!radioDev){SDL_QuitSubSystem(SDL_INIT_AUDIO);radioUnavailable=1;return;}
 }
 if(oneShot)radioOneShot=1;
 else if(radioOneShot&&SDL_GetQueuedAudioSize(radioDev))return;
 else radioOneShot=0;
 if(!track){if(radioActive)SDL_ClearQueuedAudio(radioDev);radioActive=0;return;}
 if(oneShot||track!=radioActive){SDL_ClearQueuedAudio(radioDev);radioActive=track;}
 else if(SDL_GetQueuedAudioSize(radioDev)>RM_RATE/5)return;
 const RadioWave *w=rm_record(track);if(!w)return;
 if(!SDL_QueueAudio(radioDev,w->pcm,w->dataBytes))SDL_PauseAudioDevice(radioDev,0);
}
static int display(void){
 if(take_phone_link())phone_link_result(SDL_OpenURL&&SDL_OpenURL("https://music.youtube.com/")==0);
 radio_audio(take_radio_record(),night_music_track());
 int w,h;
 if(SDL_GetRendererOutputSize(renderer,&w,&h)!=0)return 0;
 SDL_Rect v=viewport(w,h);
 if(!v.w||!v.h)return 1;
 paint();
 if(SDL_UpdateTexture(surface_tex,NULL,pixels,W*(int)sizeof(U))<0||
    SDL_SetRenderDrawColor(renderer,0,0,0,255)<0||SDL_RenderClear(renderer)<0||
    SDL_RenderCopy(renderer,surface_tex,NULL,&v)<0)return 0;
 SDL_RenderPresent(renderer);return 1;
}
static int report(const char *what,const char *detail){
 char message[512];snprintf(message,sizeof message,"%s: %s",what,detail);
 fprintf(stderr,"%s\n",message);
 if(window)SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,title,message,window);
 return 1;
}
int main(void){
 if(SDL_Init(SDL_INIT_VIDEO)<0)return report("Cannot start SDL2 video",SDL_GetError());
 int status=1,started=0;
 window=SDL_CreateWindow(title,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1152,690,
                         SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI);
 if(!window){report("Cannot open window",SDL_GetError());goto done;}
 renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
 if(!renderer)renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
 if(!renderer){report("Cannot create renderer",SDL_GetError());goto done;}
 surface_tex=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGB888,SDL_TEXTUREACCESS_STREAMING,W,H);
 if(!surface_tex){report("Cannot create framebuffer",SDL_GetError());goto done;}
 if(!linux_disk_init()){report("Cannot open the DeadChannel data directory",strerror(errno));goto done;}
 if(!linux_disk_lock()){
  report("Could not acquire the career lock",errno==EWOULDBLOCK?"Another Afterhours window already has these saves open.":strerror(errno));goto done;
 }

 struct sigaction action={.sa_handler=stop};
 sigemptyset(&action.sa_mask);sigaction(SIGINT,&action,NULL);sigaction(SIGTERM,&action,NULL);
 Uint64 seed=SDL_GetPerformanceCounter();
 set_session_seed((U)(seed^(seed>>32)));
 init_city();started=1;SDL_StartTextInput();
 focused=!!(SDL_GetWindowFlags(window)&SDL_WINDOW_INPUT_FOCUS);
 Uint64 rate=SDL_GetPerformanceFrequency(),last=SDL_GetPerformanceCounter(),time=0;
 if(!rate){report("Cannot start frame clock","SDL2 returned a zero counter frequency");goto done;}
 if(!display()){report("Cannot draw window",SDL_GetError());goto done;}
 status=0;
 while(running&&!stop_signal){
  SDL_Event e;
  while(running&&SDL_PollEvent(&e))event(&e);
  if(!running||stop_signal)break;
  Uint64 now=SDL_GetPerformanceCounter(),elapsed=now>=last?now-last:0;
  last=now;
  if(focused){
   Uint64 cap=rate*3/50;
   time+=(elapsed>cap?cap:elapsed)*60;
   while(time>=rate){tick(keys());time-=rate;redraw=1;}
  }else time=0;
  if(redraw){
   redraw=0;
   if(!display()){status=report("Cannot draw window",SDL_GetError());break;}
  }else SDL_Delay(focused?1:12);
 }
done:
 if(started){if(running&&!exit_save_ok())status=report("Could not save on exit","Previous save kept.");SDL_StopTextInput();}
 if(radioDev)SDL_CloseAudioDevice(radioDev);
 linux_disk_shutdown();
 if(surface_tex)SDL_DestroyTexture(surface_tex);
 if(renderer)SDL_DestroyRenderer(renderer);
 if(window)SDL_DestroyWindow(window);
 SDL_Quit();return status;
}
