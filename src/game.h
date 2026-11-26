#ifndef DC_H
#define DC_H
typedef unsigned int U;
enum{W=960,H=540};
enum{UP=1,DOWN=2,LEFT=4,RIGHT=8,USE=16,FIRE=32,BRAKE=64,ESC=128,MAP=256,SAVE=512,ENTER=1024,HELP=2048,GFX=4096,SLOT=8192,NEW_SLOT=16384,PUNCH=32768,TAXI_NEXT=65536,DELETE_SLOT=131072,ACTIVITIES=262144};
extern U pixels[W*H];
void init_city(void);
void tick(U keys);
void clear_input(void);
void set_session_seed(U seed);
void pointer_input(int sx,int sy,U clicks);
void text_input(U ch);
void paint(void);
int take_radio_record(void);
int get_scene(void);
void request_save(void);
void load_save(void);
#endif
