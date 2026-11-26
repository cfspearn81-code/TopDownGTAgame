/* Native engine geometric tests. All aim points are fictional game pixels. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <stdio.h>
static void stage(void){
 starter_save();sv.intro=4;sv.gear=0;
 p=(Player){.x=1024,.y=1024,.hp=100,.car=-1};scene=1;menu=-1;mapOn=0;
 bank_ui_reset();vd_reset();clear_input();pulse_guard_reset();end_job();wheelFists=0;punchFlash=0;walking=0;footcycle=0;fn=0;
 for(int i=0;i<LEN(peds);i++)peds[i]=(Ped){.life=800.f};
 for(int i=0;i<LEN(cars);i++)cars[i]=(Car){.x=-100,.y=-100};nc=3;
 z(fx,sizeof fx);
}
static void proof(const char*path){
 if(!path)return;FILE*f=fopen(path,"wb");assert(f&&fprintf(f,"P6\n%d %d\n255\n",W,H)>0);
 for(int i=0;i<W*H;i++){U c=pixels[i];fputc(c>>16,f);fputc(c>>8,f);fputc(c,f);}fclose(f);
}
static int slide_color(U c){return c==PULSE_SLIDE_INK||c==PULSE_SLIDE_FACE||c==PULSE_SLIDE_GLINT;}
static void assert_barrel(PulsePose g,PulseSprite s){
 assert(street_reach_open(p.x,p.y,g.mx,g.my,1,-1));
 assert(s.muzzle.x==(int)g.mx-550&&s.muzzle.y==(int)g.my-762);
 assert(pixels[s.muzzle.y*W+s.muzzle.x]==PULSE_MUZZLE_INK);
 float fd=(s.muzzle.x-s.slide.x)*g.co+(s.muzzle.y-s.slide.y)*g.si;
 assert(fd>8.f&&fd<10.f);
 /* Read the actual colored pixels spanning the central section of the
    slide. Opposite rails and distinct glint stay visible at oblique angles. */
 float lo=0,hi=0;int core=0,glint=0;
 for(int y=-7;y<=7;y++)for(int x=-7;x<=7;x++){
  float back=-(x*g.co+y*g.si),side=y*g.co-x*g.si;
  if(back<3.1f||back>5.9f)continue;
  U c=pixels[(s.muzzle.y+y)*W+s.muzzle.x+x];
  if(!slide_color(c))continue;
  if(side<lo)lo=side;if(side>hi)hi=side;
  core+=c==PULSE_SLIDE_FACE;glint+=c==PULSE_SLIDE_GLINT;
 }
 assert(lo<-.85f&&hi>.85f&&hi-lo>=2.8f&&core&&glint);
}
static void assert_clear_head(float a,int step,float recoil){
 const int x=474,y=262;
 Pt head=rig(x,y,cs(a),sn(a),1,0,14);
 PulsePose g=pulse_aim(p.x,p.y,a,step,recoil);
 PulseSprite held=pulse_project(g,550,762);
 U mask[11*14];
 U hair=charHair[sv.look.part[HAIR_TONE]];
 rect(x-35,y-36,71,50,DARK);
 figure_held(x,y,0xc6d979,step,a,MODEL_PLAYER,3,&sv.look,0);
 for(int dy=-8;dy<=5;dy++)for(int dx=-5;dx<=5;dx++)
  mask[(dy+8)*11+dx+5]=pixels[(head.y+dy)*W+head.x+dx];
 rect(x-35,y-36,71,50,DARK);
 figure_held(x,y,0xc6d979,step,a,MODEL_PLAYER,3,&sv.look,&held);
 int saved=0;
 for(int dy=-8;dy<=5;dy++)for(int dx=-5;dx<=5;dx++){
  U expected=mask[(dy+8)*11+dx+5];
  /* The rectangle also contains sleeves below the head; compare
     head/upper hair, not nearby arms, which are intentionally posed. */
  if(expected==DARK||(dx*dx+dy*dy>16&&dy>-3&&expected!=hair))continue;
  assert(pixels[(head.y+dy)*W+head.x+dx]==expected);
  saved++;
 }
 assert(saved>=35&&pixels[held.muzzle.y*W+held.muzzle.x]==PULSE_MUZZLE_INK);
 if(absf(cs(a))<.03f)assert(held.muzzle.x>=head.x+7);
}
int main(int argc,char**argv){
 init_city();stage();
 for(int r=0;r<2;r++)for(int j=0;j<3;j++)for(int i=0;i<128;i++){
  p.a=wrap(6.283186f*i/128.f);walking=j!=0;footcycle=j==1?1.21f:-1.59f;
  p.trail=r?.11f:0;
  PulsePose g=pulse_pose(r?2.5f:0.f);
  /* Shorter active reach without letting recoil put the muzzle behind
     the shot's forward boundary, even facing south at peak stride. */
  assert(g.extent>.05f&&g.extent<=25.f+.4f*absf((float)player_gait()));
  PulseSprite image=pulse_project(g,550,762);
  rect(0,0,W,H,DARK);
  figure_held(474,262,0xc6d979,player_gait(),p.a,MODEL_PLAYER,3,&sv.look,&image);
  assert_barrel(g,image);
 }
 p.trail=0;
 /* Same head pixels as fists, including bulky hair/hoods and the
    adjacent compass directions, throughout stride and recoil. */
 Look starting=sv.look;
 for(int hair=0;hair<10;hair++)for(int hood=0;hood<2;hood++){
  sv.look.part[HAIR]=hair;sv.look.part[TOP]=hood?3:0;
  for(int d=0;d<3;d++)for(int stride=-4;stride<=4;stride++)for(int recoil=0;recoil<2;recoil++)
   assert_clear_head(-1.570796f+(d-1)*.785398f,stride,recoil?2.5f:0.f);
 }
 sv.look=starting;
 assert(!p.trail);
 stage();p.a=0;paint();
 PulsePose before=pulse_visible_pose();PulseSprite g=pulse_project(before,cx,cy);
 assert(pixels[g.muzzle.y*W+g.muzzle.x]==PULSE_MUZZLE_INK);
 proof(argc>1?argv[1]:0);
 tick(FIRE);
 assert(p.trail>0&&p.shot==.33f&&firedPulse.extent<before.extent);
 assert(trailX==firedPulse.mx&&trailY==firedPulse.my&&trailX>p.x&&trailY<p.y);
 assert(absf(trailEndX-p.x-112.f)<.05f);
 paint();g=pulse_project(pulse_visible_pose(),cx,cy);
 assert(g.muzzle.x==sx(trailX)&&g.muzzle.y==sy(trailY)&&pixels[g.muzzle.y*W+g.muzzle.x]==0xfff2bd);
 proof(argc>2?argv[2]:0);
 float x=trailX,y=trailY;tick(UP);assert(trailX==x&&trailY==y&&pulse_visible_pose().my<y);
 stage();sv.gear=1;p.a=0;shoot();assert(p.shot==.21f&&absf(trailEndX-p.x-165.f)<.05f);
 stage();p.a=0;peds[2]=(Ped){p.x+8,p.y,0,0,3,0};
 shoot();assert(peds[2].hp==2&&firedPulse.extent<8.f&&trailEndX==peds[2].x&&trailEndX>=trailX);
 stage();p.a=-1.570796f;peds[2]=(Ped){p.x,p.y-14,0,0,3,0};
 shoot();assert(peds[2].hp==2&&firedPulse.extent<14.f&&trailX>p.x+7.f&&trailEndY==peds[2].y);
 stage();sv.gear=1;p.a=0;peds[2]=(Ped){p.x+160,p.y,0,0,3,0};
 shoot();assert(peds[2].hp==1&&trailEndX==peds[2].x);
 stage();p.a=0;peds[2]=(Ped){p.x+113,p.y,0,0,3,0};
 shoot();assert(peds[2].hp==3);
 stage();p.a=1.570796f;cars[1]=(Car){1024,1043,0,0,1,0,100};
 peds[2]=(Ped){1024,1088,0,0,3,0};shoot();
 assert(peds[2].hp==3&&p.trail==0&&p.shot==0&&p.heat==0&&pulse_pose(0).hand);
 cars[1].y=1063;shoot();
 assert(peds[2].hp==3&&p.trail>0&&trailEndY<=1055.f&&trailY<=trailEndY);
 assert(street_reach_open(p.x,p.y,trailX,trailY,1,-1));
 for(int d=0;d<4;d++){
  stage();p.a=wrap(d*1.570796f);
  const StreetBounds cover[]={{1036,1008,4,30},{1008,1036,30,4},{1008,999,4,50},{1008,1008,30,4}};
  assert(scenery[36].count<LEN(scenery[36].walls));
  scenery[36].walls[scenery[36].count++]=cover[d];
  pulse_guard_tick(1,p.x,p.y,p.a,0);
  PulsePose low=pulse_pose(0);
  PulseSprite side=pulse_project(low,550,762);
  assert(low.hand&&low.extent==0&&low.si>.8f&&absf(low.mx-p.x)>=8.5f);
  assert(side.muzzle.y>side.slide.y&&pulse_path(p.x,p.y,low.mx,low.my,0));
  rect(0,0,W,H,DARK);
  figure_held(474,262,0xc6d979,0,p.a,MODEL_PLAYER,3,&sv.look,&side);
  assert(pixels[side.muzzle.y*W+side.muzzle.x]==PULSE_MUZZLE_INK);
  for(int y=242;y<281;y++)for(int x=455;x<493;x++){
   U c=pixels[y*W+x];
   if(c!=PULSE_MUZZLE_INK&&!slide_color(c))continue;
   const StreetBounds b=cover[d];int wx=x+550,wy=y+762;
   assert(wx<=b.x||wx>=b.x+b.w||wy<=b.y||wy>=b.y+b.h);
   assert(absf(x-474)<=13&&absf(y-262)<=10);
  }
  shoot();assert(!p.trail&&!p.shot&&!p.heat);
  scenery[36].count--;
  for(int i=0;i<PULSE_CLEAR_TICKS-1;i++){
   pulse_guard_tick(1,p.x,p.y,p.a,0);
   assert(pulse_pose(0).hand);
  }
  pulse_guard_tick(1,p.x,p.y,p.a,0);
  assert(!pulse_pose(0).hand);shoot();assert(p.trail>0&&p.heat>0);
 }
 stage();p.a=0;
 /* A wall beyond immediate contact still terminates the normal trace,
    and a target behind it never takes damage. */
 assert(scenery[36].count<LEN(scenery[36].walls));
 scenery[36].walls[scenery[36].count++]=(StreetBounds){1060,1015,4,24};
 peds[2]=(Ped){1080,1024,0,0,3,0};shoot();assert(p.trail>0&&peds[2].hp==3&&trailEndX<1060);
 scenery[36].count--;
 stage();p.a=0;wheelFists=1;paint();g=pulse_project(pulse_pose(0),cx,cy);
 assert(pixels[g.muzzle.y*W+g.muzzle.x]!=PULSE_MUZZLE_INK);
 tick(FIRE);assert(p.trail<=0&&punchFlash>0);
 stage();tick(PUNCH);assert(punchFlash>0&&p.trail<=0&&pistol_equipped());
 stage();cars[1]=(Car){1024,1024,0,0,1,1,100};p.car=1;
 tick(FIRE);assert(p.trail<=0);paint();g=pulse_project(pulse_pose(0),cx,cy);
 assert(pixels[g.muzzle.y*W+g.muzzle.x]!=PULSE_MUZZLE_INK);
 stage();open_interior(1);interior_fade=0;interior_a=0;tick(FIRE);assert(p.trail<=0&&p.shot==0.f);paint();
 PulsePose room=pulse_room_pose(interior_x,interior_y,interior_a,player_gait(),int_solid);
 g=pulse_project(room,(int)interior_cam_x,(int)interior_cam_y);
 assert(pixels[g.muzzle.y*W+g.muzzle.x]==PULSE_MUZZLE_INK);
 interior_a=1.570796f;
 for(int i=0;i<52;i++)tick(DOWN|FIRE);
 assert(interior_y>426&&p.shot==0&&p.trail<=0);
 room=pulse_room_pose(interior_x,interior_y,interior_a,player_gait(),int_solid);paint();
 g=pulse_project(room,(int)interior_cam_x,(int)interior_cam_y);
 assert(room.hand&&room.si>.8f&&pixels[g.muzzle.y*W+g.muzzle.x]==PULSE_MUZZLE_INK);
 for(int i=0;i<22;i++)tick(UP);
 assert(!pulse_room_pose(interior_x,interior_y,interior_a,player_gait(),int_solid).hand);
 int fixture=0;
 for(int i=0;i<interior_nprop;i++){
  const InteriorProp f=interior_props[i];
  if(f.type!=INT_DINER)continue;
  float rx=f.x+f.w/2.f,ry=f.y+f.h+10.f;
  if(!int_solid(rx,ry)&&int_solid(rx,ry-5.f)){
   assert(pulse_room_pose(rx,ry,-1.570796f,0,int_solid).hand);
   fixture=1;break;
  }
 }
 assert(fixture);
 puts("768 compass/gait/recoil renders and 1080 unchanged head/hair masks with clear muzzle; 112/165 bounds, point blank, four wall directions, near/far cover, steady raise, fists, indoor carry, and driving verified.");
}
