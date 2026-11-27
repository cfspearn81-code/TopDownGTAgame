/* Test-only: SDL2 normally queues CLOSE then QUIT for the last window.
 * Inject one transient save failure and choose Keep playing. */
#define _GNU_SOURCE
#include "../../src/sdl2_linux.h"
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
static unsigned frames,blocked,prompts,committed,retry;
static int fail_next;
static void push(SDL_Event e){assert(SDL_PushEvent(&e)==1);}
static void close_pair(void){
 SDL_Event e={.type=SDL_WINDOWEVENT};e.window.event=SDL_WINDOWEVENT_CLOSE;push(e);push((SDL_Event){.type=SDL_QUIT});
}
int renameat(int a,const char *p,int b,const char *q){
 int(*real)(int,const char*,int,const char*)=dlsym(RTLD_NEXT,"renameat");
 if(!strcmp(q,"career.dat")){
  if(fail_next){blocked++;fail_next=0;errno=ENOSPC;return -1;}
  if(blocked)committed++;
 }
 return real(a,p,b,q);
}
int SDL_ShowMessageBox(const SDL_MessageBoxData *d,int *result){
 assert(d->numbuttons==2&&d->buttons[0].buttonid==0&&!strcmp(d->buttons[0].text,"Keep playing"));
 assert(d->buttons[0].flags==(SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT|SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT));
 prompts++;*result=0;return 0;
}
void SDL_RenderPresent(SDL_Renderer*r){
 void(*real)(SDL_Renderer*)=dlsym(RTLD_NEXT,"SDL_RenderPresent");real(r);
 if(!frames){SDL_Event e={.type=SDL_WINDOWEVENT};e.window.event=SDL_WINDOWEVENT_FOCUS_GAINED;push(e);}
 if(frames==1){SDL_Event e={.type=SDL_KEYDOWN};e.key.keysym.scancode=SDL_SCANCODE_RETURN;push(e);e.type=SDL_KEYUP;push(e);}
 if(frames==5){fail_next=1;close_pair();}
 if(frames==9){assert(blocked==1&&prompts==1&&!committed);retry=1;close_pair();}
 frames++;
}
void SDL_Quit(void){
 void(*real)(void)=dlsym(RTLD_NEXT,"SDL_Quit");real();
 fprintf(stderr,"close pair: frames=%u prompts=%u denied=%u later-close=%u commits=%u\n",frames,prompts,blocked,retry,committed);
 assert(retry&&frames==10&&prompts==1&&blocked==1&&committed==1);
}
