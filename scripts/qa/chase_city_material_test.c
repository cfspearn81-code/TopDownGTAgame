/* Render tests for all original city addresses and their physically fixed
 * door signs; ch_buildings is only entered by perspective rendering.
 * Optional argv[1] is a disposable capture directory. */
#define DC_TEST
#include "../../src/game.c"
#include <assert.h>
#include <stdio.h>
static unsigned cf_hash(const unsigned char *p,int n){unsigned h=2166136261u;for(int i=0;i<n;i++)h=(h^p[i])*16777619u;return h;}
static void city_shot(const char*d,const char*n){if(!d)return;char f[512];snprintf(f,sizeof f,"%s/%s.ppm",d,n);FILE*out=fopen(f,"wb");assert(out);fprintf(out,"P6\n%d %d\n255\n",W,H);
 for(int j=0;j<W*H;j++){U v=pixels[j];fputc(v>>16,out);fputc(v>>8,out);fputc(v,out);}fclose(out);}
int main(int argc,char**argv){
 const char *out=argc>1?argv[1]:0;init_city();scene=1;menu=-1;mapOn=0;sv.intro=4;sv.dayClock=650.f;
 int active=0,styles=0;unsigned set[LEN(buildings)]={0},seed=rng,original=savehash();
 for(int i=0;i<nb;i++){
  if(buildings[i].w<=0||buildings[i].h<=0)continue;
  CfAddress ad=cf_address(i);styles|=1<<ad.style;
  CfSheet *front=cf_bake(i,2,&ad);assert(front->ready&&ad.name&&ad.name[0]);
  int u=cf_door_u(i,&ad);assert(front->t[55*128+u+2]==CF_PAPER);
  for(int side=0;side<4;side++){
   const unsigned char*t=cf_bake(i,side,&ad)->t;
   for(int j=0;j<CF_PITCH;j++)assert(t[j]<CF_N);
  }
  set[active++]=cf_hash(front->t,CF_LO);
 }
 int unique=0;for(int i=0;i<active;i++){int c=1;for(int k=0;k<i;k++)if(set[k]==set[i])c=0;unique+=c;}
 assert(styles==127&&active>=65&&unique>50);assert(seed==rng&&savehash()==original);
 /* Eye level views of a named venue and an ATM, real perspective z buffer, no HUD. */
 int ids[]={28,ncFrontId,zone_idx[0],zone_idx[1]};const char*names[]={"copper","afterimage","mercury","vault"};
 for(int i=0;i<4;i++){
  const Building*b=buildings+ids[i];ch.x=b->x+b->w*.4f;ch.y=b->y+b->h+124;ch.h=18;ch.si=-1;ch.co=0;chSin=.035f;chCos=.999387f;chDark=i%2?.80f:.08f;chFog=.08f;
  ch_ground();ch_buildings();city_shot(out,names[i]);
 }
 ch.x=bankAtms[0].x+6;ch.y=bankAtms[0].y+62;ch.h=17;ch.si=-1;ch.co=0;chSin=.035f;chCos=.999387f;chDark=.25f;ch_ground();ch_buildings();city_shot(out,"atm");
 assert(seed==rng&&savehash()==original);
 printf("PASS: %d addresses, %d distinct front sheets, 7 frontage kits, 4 elevations + mipmaps per address; door alignment, palette bounds and zero simulation writes.\n",active,unique);
}
