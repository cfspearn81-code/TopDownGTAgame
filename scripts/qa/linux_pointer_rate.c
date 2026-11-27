/* Test-only: feed 1,000 SDL pointer-motion batches/second against a fixed
   host clock. Count actual presentations made by the unmodified native ELF. */
#define _GNU_SOURCE
#include "../../src/sdl2_linux.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned clock_calls,presents,queued_at,packets;
static int host(void *pc){Dl_info di;return dladdr(pc,&di)&&di.dli_fname&&strstr(di.dli_fname,"DeadChannel");}
Uint64 SDL_GetPerformanceFrequency(void){
 if(host(__builtin_return_address(0)))return 1000;
 Uint64(*f)(void)=dlsym(RTLD_NEXT,"SDL_GetPerformanceFrequency");return f();
}
Uint64 SDL_GetPerformanceCounter(void){
 if(host(__builtin_return_address(0)))return ++clock_calls;
 Uint64(*f)(void)=dlsym(RTLD_NEXT,"SDL_GetPerformanceCounter");return f();
}
int SDL_PollEvent(SDL_Event *out){
 int (*f)(SDL_Event*)=dlsym(RTLD_NEXT,"SDL_PollEvent");
 if(clock_calls!=queued_at){
  queued_at=clock_calls;
  if(queued_at==2){SDL_Event e={.type=SDL_WINDOWEVENT};e.window.event=SDL_WINDOWEVENT_FOCUS_GAINED;assert(SDL_PushEvent(&e)==1);}
  if(queued_at>=3&&queued_at<2003){
   SDL_Event e={.type=SDL_MOUSEMOTION};e.motion.x=112+queued_at%888;e.motion.y=144;assert(SDL_PushEvent(&e)==1);packets++;
  }
  if(queued_at==2003){SDL_Event e={.type=SDL_QUIT};assert(SDL_PushEvent(&e)==1);}
 }
 return f(out);
}
void SDL_RenderPresent(SDL_Renderer*r){
 if(!presents){
  struct{const char *name;Uint32 flags,n,formats[16];int w,h;} info={0};
  const char *(*video)(void)=dlsym(RTLD_NEXT,"SDL_GetCurrentVideoDriver");
  int (*ri)(SDL_Renderer*,void*)=dlsym(RTLD_NEXT,"SDL_GetRendererInfo");
  if(video&&ri&&ri(r,&info)==0)fprintf(stderr,"SDL video=%s renderer=%s\n",video(),info.name);
 }
 void (*f)(SDL_Renderer*)=dlsym(RTLD_NEXT,"SDL_RenderPresent");f(r);presents++;
}
void SDL_Quit(void){
 void (*f)(void)=dlsym(RTLD_NEXT,"SDL_Quit");f();
 fprintf(stderr,"Native SDL pointer storm: %u packets across 2 simulated seconds; %u full presentations.\n",packets,presents);
 assert(packets==2000&&presents>=120);
 if(!getenv("DC_RATE_OBSERVE"))assert(presents<=126);
}
