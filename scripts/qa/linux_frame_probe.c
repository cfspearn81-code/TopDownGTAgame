/* Exact ELF, real SDL dummy renderer and POSIX file ops; controlled clock
 * only to shorten the regression and bound key sequencing. */
#define _GNU_SOURCE
#include "../../src/sdl2_linux.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
static unsigned frame,clocks;
static int through_focus;
static char csv_path[1024];
static void emit(SDL_Event e){assert(SDL_PushEvent(&e)==1);}
static void key(int k,int repeat){
 SDL_Event e={.type=SDL_KEYDOWN};e.key.keysym.scancode=k;e.key.repeat=repeat;emit(e);
}
static int host(void*ret){
 Dl_info i;return dladdr(ret,&i)&&i.dli_fname&&strstr(i.dli_fname,"DeadChannel");
}
Uint64 SDL_GetPerformanceFrequency(void){
 if(host(__builtin_return_address(0)))return 60000;
 Uint64(*f)(void)=dlsym(RTLD_NEXT,"SDL_GetPerformanceFrequency");return f();
}
Uint64 SDL_GetPerformanceCounter(void){
 if(host(__builtin_return_address(0)))return 17+1000*clocks++;
 Uint64(*f)(void)=dlsym(RTLD_NEXT,"SDL_GetPerformanceCounter");return f();
}
static void focus(int on){
 SDL_Event e={.type=SDL_WINDOWEVENT};e.window.event=on?SDL_WINDOWEVENT_FOCUS_GAINED:SDL_WINDOWEVENT_FOCUS_LOST;emit(e);
}
void SDL_RenderPresent(SDL_Renderer*r){
 void(*real)(SDL_Renderer*)=dlsym(RTLD_NEXT,"SDL_RenderPresent");assert(real);real(r);
 int first=frame==0;
 if(first){
  const char *root=getenv("XDG_DATA_HOME");assert(root);
  int n=snprintf(csv_path,sizeof csv_path,"%s/DeadChannel/frame-times.csv",root);
  assert(n>0&&(unsigned)n<sizeof csv_path);
  through_focus=getenv("DC_PROBE_FOCUS")!=NULL;
  focus(1);
 }
 switch(frame++){
 case 1:key(SDL_SCANCODE_F10,0);break;
 case 4:key(SDL_SCANCODE_F9,0);key(SDL_SCANCODE_F9,1);break;
 case 7:key(SDL_SCANCODE_F10,0);break;
 case 11:
  assert(access(csv_path,F_OK)!=0);
  if(through_focus){focus(0);focus(1);}
  break;
 case 21:
  if(through_focus)key(SDL_SCANCODE_F10,0);
  break;
 case 30:
  if(through_focus)assert(access(csv_path,F_OK)==0);
  else assert(access(csv_path,F_OK)!=0);
  break;
 case 320:
  if(!through_focus){assert(access(csv_path,F_OK)!=0);key(SDL_SCANCODE_F10,0);}
  break;
 case 327:assert(access(csv_path,F_OK)==0);emit((SDL_Event){.type=SDL_QUIT});break;
 }
}
