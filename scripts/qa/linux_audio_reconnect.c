/* Fault injection on the real SDL ELF: a formerly missing output comes
 * back during a drive, then the currently playing output disconnects. */
#define _GNU_SOURCE
#include "../../src/sdl2_linux.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
static unsigned frame,attempts,first,recovered,lost,closed,queued,queued_after;
static SDL_AudioDeviceID current;
Uint32 SDL_GetTicks(void){
 Dl_info di;
 if(dladdr(__builtin_return_address(0),&di)&&di.dli_fname&&strstr(di.dli_fname,"DeadChannel"))return frame*17;
 Uint32(*real)(void)=dlsym(RTLD_NEXT,"SDL_GetTicks");return real();
}
SDL_AudioDeviceID SDL_OpenAudioDevice(const char*n,int cap,const SDL_AudioSpec*w,SDL_AudioSpec*g,int changes){
 SDL_AudioDeviceID(*real)(const char*,int,const SDL_AudioSpec*,SDL_AudioSpec*,int)=dlsym(RTLD_NEXT,"SDL_OpenAudioDevice");
 assert(!cap);
 if(++attempts==1){first=frame;return 0;}
 if(attempts==2){recovered=frame;assert((recovered-first)*17>=5000&&(recovered-first)*17<=5100);}
 current=real(n,cap,w,g,changes);assert(current);return current;
}
void SDL_CloseAudioDevice(SDL_AudioDeviceID dev){
 void(*real)(SDL_AudioDeviceID)=dlsym(RTLD_NEXT,"SDL_CloseAudioDevice");
 if(dev==current){closed++;current=0;}real(dev);
}
int SDL_QueueAudio(SDL_AudioDeviceID dev,const void*p,Uint32 n){
 int(*real)(SDL_AudioDeviceID,const void*,Uint32)=dlsym(RTLD_NEXT,"SDL_QueueAudio");
 assert(dev&&dev==current&&n);queued++;if(lost)queued_after++;
 return real(dev,p,n);
}
void SDL_RenderPresent(SDL_Renderer*r){
 void(*real)(SDL_Renderer*)=dlsym(RTLD_NEXT,"SDL_RenderPresent");real(r);frame++;
 if(recovered&&!lost&&frame>=recovered+8){
  union {SDL_Event ev;struct{Uint32 type,time,id;Uint8 capture,pad[3];} dev;} event={0};
  event.dev.type=0x1101;event.dev.id=current;assert(SDL_PushEvent(&event.ev)==1);lost=frame;
 }
}
void SDL_Quit(void){
 void(*real)(void)=dlsym(RTLD_NEXT,"SDL_Quit");real();
 fprintf(stderr,"audio reconnect: attempts=%u first=%u recovered=%u lost=%u queued=%u after=%u closes=%u\n",attempts,first,recovered,lost,queued,queued_after,closed);
 assert(attempts==3&&recovered&&lost&&queued_after&&closed==2);
}
