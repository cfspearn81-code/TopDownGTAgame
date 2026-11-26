/* Historic palette and dimensions were measured from Career Files
 * bd5d4e61ddb9830b125aabc9bfd03e4a04017e6d. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
static void sample_ground(void){
 for(int y=0;y<H;y++)for(int x=0;x<W;x++)pixels[y*W+x]=city[(y+cy)*ROOM+cx+x];
}
static U px(int x,int y){return pixels[(y-cy)*W+x-cx];}
static void lenses(const StreetLamp *s){
 for(int y=s->lensY;y<s->lensY+s->lensH;y++)for(int x=s->lensX;x<s->lensX+s->lensW;x++){
  assert(city[(s->y+y)*ROOM+s->x+x]==s->lens);assert(px(s->x+x,s->y+y)==s->lens);
 }
}
int main(void){
 init_city();int count=0,alt=0,out=0;StreetLamp s,a,t;
 for(int y=0;y<ROOM/256;y++)for(int x=0;x<ROOM/256;x++){
  count+=street_lamp_legacy(x,y,0,&s);alt+=street_lamp_legacy(x,y,1,&s);
 }
 for(int i=0;i<LEN(outerNodes);i++)if(street_lamp_outer(i,&s)){
  ++out;assert(s.lensW==8&&s.lensH==4&&s.poleH==13);
 }
 assert(count==49&&alt==25&&out>15);
 assert(street_lamp_legacy(2,2,0,&s)&&street_lamp_legacy(2,2,1,&a));
 assert(street_lamp_legacy(3,3,0,&t)&&t.pool==0x8d7352);
 assert(street_lamp_legacy(6,3,0,&t)&&t.pool==0x388b8c);
 assert(s.x==543&&s.y==478&&s.radius==46&&s.lens==0xcbb8bb&&s.pole==0x293842);
 assert(a.x==478&&a.y==550&&a.radius==27&&a.lens==0x7bead8);
 assert(street_lamp_style(7,7,0,0).lens==mix(0,PAPER,5));
 cx=210;cy=312;sv.dayClock=0;
 night_world_reset();
 sample_ground();night_world_street_fx();lenses(&s);lenses(&a);
 /* Round emitter overlays used to color a pixel below the old lamp lens. */
 assert(px(s.x,s.y)==nw_blend(s.pole,0x121727,92));
 U dry=px(s.x,s.y+14);
 nwWet=.7f;sample_ground();night_world_street_fx();
 assert(px(s.x,s.y+14)!=dry);
 nwFog=.75f;sample_ground();night_world_street_fx();
 assert(px(s.x,s.y-2)!=s.lens); /* Fog still softens both lamps. */
 fastGfx=1;sample_ground();night_world_street_fx();
 assert(px(s.x,s.y-2)==nw_blend(s.lens,0x121727,92));
 puts("PASS: Career Files lamp counts, exact offsets and rectangles, original lavender/teal/amber lenses, night and Low FX, wet reflection and fog.");
}
