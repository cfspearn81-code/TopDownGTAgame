#ifndef DC_SDL2_LINUX_H
#define DC_SDL2_LINUX_H
/* Prefer SDL's SDK headers. The fallback only declares the SDL 2 public
 * x86-64 C ABI used here, for builds on runtime-only installations. */
#ifdef DC_USE_SYSTEM_SDL
#include <SDL.h>
#else
#include <stdint.h>
typedef uint8_t Uint8;
typedef uint32_t Uint32;
typedef uint64_t Uint64;
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
typedef struct {int x,y,w,h;} SDL_Rect;
typedef struct {int scancode,sym;uint16_t mod;Uint32 unused;} SDL_Keysym;
typedef struct {Uint32 type,timestamp,windowID;Uint8 state,repeat,padding2,padding3;SDL_Keysym keysym;} SDL_KeyboardEvent;
typedef struct {Uint32 type,timestamp,windowID;Uint8 event,pad1,pad2,pad3;int data1,data2;} SDL_WindowEvent;
typedef struct {Uint32 type,timestamp,windowID,which,state;int x,y,xrel,yrel;} SDL_MouseMotionEvent;
typedef struct {Uint32 type,timestamp,windowID,which;Uint8 button,state,clicks,pad1;int x,y;} SDL_MouseButtonEvent;
typedef struct {Uint32 type,timestamp,windowID;char text[32];} SDL_TextInputEvent;
typedef union SDL_Event {Uint32 type;SDL_KeyboardEvent key;SDL_WindowEvent window;SDL_MouseMotionEvent motion;
 SDL_MouseButtonEvent button;SDL_TextInputEvent text;Uint64 alignment;Uint8 padding[56];} SDL_Event;
_Static_assert(sizeof(SDL_Event)==56&&sizeof(SDL_KeyboardEvent)==32,"SDL2 x86-64 event ABI");

enum {
 SDL_INIT_VIDEO=0x20,SDL_WINDOWPOS_CENTERED=0x2fff0000, SDL_WINDOW_RESIZABLE=0x20,
 SDL_WINDOW_INPUT_FOCUS=0x200,SDL_WINDOW_ALLOW_HIGHDPI=0x2000,
 SDL_RENDERER_SOFTWARE=1,SDL_RENDERER_ACCELERATED=2,
 SDL_PIXELFORMAT_RGB888=0x16161804,SDL_PIXELFORMAT_ARGB8888=0x16362004,SDL_TEXTUREACCESS_STREAMING=1,
 SDL_MESSAGEBOX_ERROR=0x10, SDL_QUIT=0x100, SDL_WINDOWEVENT=0x200,SDL_KEYDOWN=0x300,
 SDL_KEYUP=0x301,SDL_TEXTINPUT=0x303,SDL_MOUSEMOTION=0x400,SDL_MOUSEBUTTONDOWN=0x401,
 SDL_WINDOWEVENT_EXPOSED=3,SDL_WINDOWEVENT_RESIZED=5,SDL_WINDOWEVENT_SIZE_CHANGED=6,
 SDL_WINDOWEVENT_RESTORED=9,SDL_WINDOWEVENT_LEAVE=11,SDL_WINDOWEVENT_FOCUS_GAINED=12,
 SDL_WINDOWEVENT_FOCUS_LOST=13,SDL_WINDOWEVENT_CLOSE=14,SDL_BUTTON_LEFT=1,SDL_BUTTON_RIGHT=3,
 SDL_SCANCODE_A=4,SDL_SCANCODE_D=7,SDL_SCANCODE_E=8,SDL_SCANCODE_M=16,SDL_SCANCODE_S=22,
 SDL_SCANCODE_T=23,SDL_SCANCODE_V=25,SDL_SCANCODE_W=26,SDL_SCANCODE_X=27,SDL_SCANCODE_RETURN=40,
 SDL_SCANCODE_ESCAPE=41,SDL_SCANCODE_BACKSPACE=42,SDL_SCANCODE_TAB=43,SDL_SCANCODE_SPACE=44,
 SDL_SCANCODE_F1=58,SDL_SCANCODE_F2=59,SDL_SCANCODE_F3=60,SDL_SCANCODE_F4=61,SDL_SCANCODE_F5=62,SDL_SCANCODE_F6=63,
 SDL_SCANCODE_DELETE=76,SDL_SCANCODE_RIGHT=79,SDL_SCANCODE_LEFT=80,SDL_SCANCODE_DOWN=81,
 SDL_SCANCODE_UP=82,SDL_SCANCODE_KP_ENTER=88,SDL_SCANCODE_LSHIFT=225,SDL_SCANCODE_RSHIFT=229,
 SDL_NUM_SCANCODES=512
};
enum {SDL_INIT_AUDIO=0x10,AUDIO_S16LSB=0x8010};
typedef Uint32 SDL_AudioDeviceID;
typedef struct {int freq;uint16_t format;Uint8 channels,silence;uint16_t samples,padding;
 Uint32 size;void (*callback)(void*,Uint8*,int);void *userdata;} SDL_AudioSpec;
_Static_assert(sizeof(SDL_AudioSpec)==32,"SDL2 x86-64 audio ABI");
int SDL_InitSubSystem(Uint32 flags);
void SDL_QuitSubSystem(Uint32 flags);
SDL_AudioDeviceID SDL_OpenAudioDevice(const char*,int,const SDL_AudioSpec*,SDL_AudioSpec*,int);
int SDL_QueueAudio(SDL_AudioDeviceID,const void*,Uint32);
void SDL_ClearQueuedAudio(SDL_AudioDeviceID);
void SDL_PauseAudioDevice(SDL_AudioDeviceID,int);
void SDL_CloseAudioDevice(SDL_AudioDeviceID);
int SDL_Init(Uint32 flags);
void SDL_Quit(void);
const char *SDL_GetError(void);
SDL_Window *SDL_CreateWindow(const char *title,int x,int y,int w,int h,Uint32 flags);
SDL_Renderer *SDL_CreateRenderer(SDL_Window *window,int index,Uint32 flags);
SDL_Texture *SDL_CreateTexture(SDL_Renderer *renderer,Uint32 format,int access,int w,int h);
void SDL_DestroyWindow(SDL_Window *window);
void SDL_DestroyRenderer(SDL_Renderer *renderer);
void SDL_DestroyTexture(SDL_Texture *texture);
Uint32 SDL_GetWindowFlags(SDL_Window *window);
void SDL_GetWindowSize(SDL_Window *window,int *w,int *h);
void SDL_SetWindowSize(SDL_Window *window,int w,int h);
int SDL_GetRendererOutputSize(SDL_Renderer *renderer,int *w,int *h);
int SDL_SetRenderDrawColor(SDL_Renderer *renderer,Uint8 r,Uint8 g,Uint8 b,Uint8 a);
int SDL_RenderClear(SDL_Renderer *renderer);
int SDL_UpdateTexture(SDL_Texture *texture,const SDL_Rect *rect,const void *pixels,int pitch);
int SDL_RenderCopy(SDL_Renderer *renderer,SDL_Texture *texture,const SDL_Rect *src,const SDL_Rect *dst);
int SDL_RenderReadPixels(SDL_Renderer *renderer,const SDL_Rect *rect,Uint32 format,void *pixels,int pitch);
void SDL_RenderPresent(SDL_Renderer *renderer);
int SDL_PollEvent(SDL_Event *event);
int SDL_PushEvent(SDL_Event *event);
void SDL_StartTextInput(void);
void SDL_StopTextInput(void);
Uint64 SDL_GetPerformanceCounter(void);
Uint64 SDL_GetPerformanceFrequency(void);
void SDL_Delay(Uint32 ms);
int SDL_ShowSimpleMessageBox(Uint32 flags,const char *title,const char *message,SDL_Window *window);
#endif
#endif
