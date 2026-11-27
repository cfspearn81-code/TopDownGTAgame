/* Test only: deliver the public SDL2 device-lost notification and reject
 * uploads into its invalidated texture. Driver and renderer are real SDL
 * software; this is fault injection, not a physical GPU reset. */
#define _GNU_SOURCE
#include "../../src/sdl2_linux.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
static unsigned frames,allocated,stale,discarded;
static int invalid;
static void emit(SDL_Event e){assert(SDL_PushEvent(&e)==1);}
SDL_Texture *SDL_CreateTexture(SDL_Renderer*r,Uint32 fmt,int ac,int w,int h){
 SDL_Texture *(*real)(SDL_Renderer*,Uint32,int,int,int)=dlsym(RTLD_NEXT,"SDL_CreateTexture");
 SDL_Texture*t=real(r,fmt,ac,w,h);assert(t);allocated++;invalid=0;return t;
}
void SDL_DestroyTexture(SDL_Texture*t){
 void(*real)(SDL_Texture*)=dlsym(RTLD_NEXT,"SDL_DestroyTexture");real(t);discarded++;
}
int SDL_UpdateTexture(SDL_Texture*t,const SDL_Rect*r,const void*p,int pitch){
 int (*real)(SDL_Texture*,const SDL_Rect*,const void*,int)=dlsym(RTLD_NEXT,"SDL_UpdateTexture");
 if(invalid){stale++;return -1;}return real(t,r,p,pitch);
}
void SDL_RenderPresent(SDL_Renderer*r){
 void (*real)(SDL_Renderer*)=dlsym(RTLD_NEXT,"SDL_RenderPresent");real(r);
 if(frames==0){SDL_Event e={.type=SDL_WINDOWEVENT};e.window.event=SDL_WINDOWEVENT_FOCUS_GAINED;emit(e);}
 if(frames==2||frames==6){
  invalid=1;emit((SDL_Event){.type=0x2001});emit((SDL_Event){.type=0x2001});
 }
 if(frames==4)emit((SDL_Event){.type=0x2000});
 if(frames==10){
  SDL_Rect center={576,345,1,1};uint32_t pix=0;
  assert(!SDL_RenderReadPixels(r,&center,SDL_PIXELFORMAT_ARGB8888,&pix,4)&&(pix&0xffffffu));
  emit((SDL_Event){.type=SDL_QUIT});
 }
 frames++;
}
void SDL_Quit(void){
 void(*real)(void)=dlsym(RTLD_NEXT,"SDL_Quit");real();
 fprintf(stderr,"reset: frames=%u stale-uploads=%u allocations=%u releases=%u\n",frames,stale,allocated,discarded);
 assert(frames==11&&!stale&&allocated==3&&discarded==allocated);
}
