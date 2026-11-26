/* Starts a disposable career with the stock pistol. Real movement/firing
 * frames then produce the expected image probes for the two actual hosts. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static U keysHeld[1000];static int n;
static void step(U k){assert(n<LEN(keysHeld));keysHeld[n++]=k;tick(k);}
static FILE*probes;
static void marks(const char*name,int dir,Pt a,Pt b,Pt c,Pt d){
 Pt checked[]={a,b,c,d};
 fprintf(probes,"%d %d %d ",n,sx(p.x),sy(p.y));
 for(int i=0;i<LEN(checked);i++){
  Pt q=checked[i];assert((U)q.x<W&&(U)q.y<H);
  fprintf(probes,"%d %d %08x ",q.x,q.y,pixels[q.y*W+q.x]);
 }
 fprintf(probes,"%s-%d\n",name,dir);
}
static void sample(int dir,int fire){
 PulsePose g=pulse_visible_pose();paint();
 PulseSprite s=pulse_project(g,cx,cy);U expected=fire?0xfff2bd:PULSE_MUZZLE_INK;
 assert((U)s.muzzle.x<W&&(U)s.muzzle.y<H&&pixels[s.muzzle.y*W+s.muzzle.x]==expected);
 /* Probe the actual sprite ink, not dynamic headlights behind the
  * weapon. Also keep a nearby slide/trace sample on the same line. */
 Pt extra=model_ptmix(s.slide,s.muzzle,4),railA=extra,railB=extra;
 if(fire){
  int dx=sx(trailEndX)-s.muzzle.x,dy=sy(trailEndY)-s.muzzle.y;
  int reach=max(abs(dx),abs(dy)),found=0;
  for(int i=4;i<=min(15,reach)&&!found;i++){
   int x=s.muzzle.x+dx*i/reach,y=s.muzzle.y+dy*i/reach;
   for(int oy=-1;oy<=1;oy++)for(int ox=-1;ox<=1;ox++){
    int ux=x+ox,uy=y+oy;
    if((U)ux<W&&(U)uy<H&&pixels[uy*W+ux]==GOLD){extra=(Pt){ux,uy};found=1;}
   }
  }
  if(!found)fprintf(stderr,"No visible trace for direction %d, extent %d\n",dir,reach);
  assert(found);railA=railB=extra;
 }else{
  int face=0;
  float a=0,b=0;
  for(int oy=-7;oy<=7;oy++)for(int ox=-7;ox<=7;ox++){
   float axial=-(ox*s.co+oy*s.si),side=oy*s.co-ox*s.si;
   if(axial<3.1f||axial>5.9f)continue;
   Pt q={s.muzzle.x+ox,s.muzzle.y+oy};
   U c=pixels[q.y*W+q.x];
   if(c==PULSE_SLIDE_FACE){extra=q;face=1;}
   if(c!=PULSE_SLIDE_INK&&c!=PULSE_SLIDE_GLINT)continue;
   if(side<a){a=side;railA=q;}
   if(side>b){b=side;railB=q;}
  }
  assert(face&&a<-.85f&&b>.85f&&b-a>=2.8f);
 }
 marks(fire?"flash":"pose",dir,s.muzzle,extra,railA,railB);
}
static void carry_sample(int dir){
 PulsePose g=pulse_visible_pose();paint();
 PulseSprite s=pulse_project(g,cx,cy);
 Pt mid=model_ptmix(s.slide,s.muzzle,4);
 Pt edge=pulse_ink_at(s,mid,0,-1.25f);
 Pt hand={s.grip.x,s.grip.y+1};
 assert(g.hand&&g.mx<p.x-8.f&&g.si>.8f&&g.extent==0);
 assert(s.muzzle.y>s.slide.y&&pixels[s.muzzle.y*W+s.muzzle.x]==PULSE_MUZZLE_INK);
 assert(pixels[mid.y*W+mid.x]==PULSE_SLIDE_FACE);
 marks("carry",dir,s.muzzle,mid,edge,hand);
}
static void cover_route(void){
 assert(scene==1&&!pulse_visible_pose().hand);
 int contact=0;
 for(int i=0;i<70;i++){
  float before=p.x;step(RIGHT);
  if(before==p.x){
   /* A real, unchanged city wall blocks movement here. */
   assert(solid(p.x+FOOT_WALK_SPEED/60.f,p.y,10));
   contact=1;break;
  }
 }
 assert(contact);step(0);carry_sample(0);
 float quiet=p.heat;
 for(int i=0;i<8;i++)step(RIGHT|FIRE);
 assert(p.trail<=0&&p.shot<=0&&p.heat<=quiet);
 carry_sample(1);
 for(int i=0;i<24;i++)step(LEFT);step(0);step(RIGHT);step(0);
 assert(!pulse_visible_pose().hand);
 sample(8,0);
 step(FIRE);
 assert(p.trail>.1f&&trailEndX>trailX&&trailEndX-p.x<75.f);
 sample(8,1);
}
static void north_sample(int phase,int firing){
 PulsePose g=pulse_visible_pose();paint();
 PulseSprite s=pulse_project(g,cx,cy);
 const int x=sx(p.x),y=sy(p.y);Pt head=rig(x,y,cs(p.a),sn(p.a),1,0,14);
 assert(!g.hand&&g.si<-.7f);
 assert(pixels[s.muzzle.y*W+s.muzzle.x]==(firing?0xfff2bd:PULSE_MUZZLE_INK));
 const U skin=charSkin[sv.look.part[SKIN]];
 const Pt crown={head.x-1,head.y-2},cheek={head.x,head.y},rim={head.x-2,head.y+1};
 if(absf(g.co)<.03f){
#ifndef DC_PULSE_BASELINE
  assert(s.muzzle.x>=head.x+7);
  assert(pixels[cheek.y*W+cheek.x]==skin);
#else
  assert(s.muzzle.x==head.x);
  (void)skin;
#endif
 }
 if(firing)assert(s.muzzle.x==sx(trailX)&&s.muzzle.y==sy(trailY));
 marks("north",phase,s.muzzle,crown,cheek,rim);
}
static void north_route(void){
 for(int i=0;i<3;i++)step(UP);
 step(0);north_sample(0,0);
 for(int i=0;i<30;i++){
  step(UP);
  if(i==4)north_sample(1,0);
  if(i==22)north_sample(2,0);
 }
 for(int i=0;i<13;i++)step(UP|BRAKE);
 north_sample(3,0);
 step(UP|BRAKE|FIRE);assert(p.trail>.1f);
 north_sample(4,1);
 for(int i=0;i<28;i++)step(0);
 for(int i=0;i<4;i++)step(LEFT|UP);
 north_sample(5,0);
 for(int i=0;i<4;i++)step(RIGHT|UP|BRAKE);
 north_sample(6,0);
}
int main(int argc,char**argv){
 assert(argc==3||argc==4);set_session_seed(17);init_city();starter_save();
 int north=argc==4&&argv[3][0]=='n',cover=argc==4&&!north;
 sv.intro=4;sv.gear=0;sv.x=north?1000:cover?1036:1042;sv.y=north?1080:cover?1136:1005;
 sv.cash=60;sv.hash=savehash();
 FILE*f=fopen(argv[1],"wb");assert(f&&fwrite(&sv,sizeof sv,1,f)==1&&!fclose(f));
 probes=fopen(argv[2],"w");assert(probes);
 profiles_activate();keysHeld[n++]=0;keysHeld[n++]=ENTER;
 for(int i=0;i<4;i++)step(0);
 if(north)north_route();
 else if(cover)cover_route();
 else {
 const U dirs[]={RIGHT,RIGHT|DOWN,DOWN,LEFT|DOWN,LEFT,LEFT|UP,UP,RIGHT|UP};
 for(int d=0;d<LEN(dirs);d++){
  for(int i=0;i<32;i++)step(0);
  U move=dirs[d]|(d%2?BRAKE:0);for(int i=0;i<3;i++)step(move);
  if(d%2==0)step(0);sample(d,0);
  step((d%2?move:0)|FIRE);assert(p.trail>.1f&&firedPulse.mx==trailX&&firedPulse.my==trailY);
  sample(d,1);
 }
 }
 fclose(probes);step(0);step(SAVE);step(0);step(0);
 printf("%d %08x\n",n,sv.hash);
 for(int i=0;i<n;){int j=i+1;while(j<n&&keysHeld[j]==keysHeld[i])j++;printf("%d %u\n",j-i,keysHeld[i]);i=j;}
}
