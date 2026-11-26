/* Native raster/UI regressions. No platform host or player saves required. */
#define DC_TEST
#include "game.c"
#include <assert.h>
#include <string.h>
static int touched(int x0,int y0,int x1,int y1,U base){
 int n=0;for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++)n+=pixels[y*W+x]!=base;return n;
}
static void test_cards(void){
 /* This real line survives leaving Police HQ and used to run under the radar. */
 const char*test="Chief Rivas: The front desk handles dispatch. Offices are open, cells are restricted.";
 char row[130];const char*next=hud_line(test,711,row);
 assert(!strcmp(next,"are restricted.")&&textw(row,0)<=711);
 assert(!*hud_line(next,711,row)&&!strcmp(row,"are restricted."));
 U base=0x010203;rect(0,0,W,H,base);
 tell(test);street_toast();
 assert(touched(20,440,250,460,DARK)>0&&touched(18,417,730,438,base)>0);
 assert(!touched(751,400,W,475,base)); /* nothing under, or on top of, radar */
 char wide[100];for(int i=0;i<99;i++)wide[i]='W';wide[99]=0;
 rect(0,0,W,H,base);objective_card(wide,test);
 assert(!touched(640,0,W,145,base)&&touched(18,85,500,103,base)>0);
 assert(!touched(DAY_BADGE_X,16,DAY_BADGE_X+DAY_BADGE_W,16+DAY_BADGE_H,base));
 for(int col=0;col<2;col++){
  const GuideBinding*g=col?guideCareer:guideStreet;int keyw=col?113:141;
  for(int i=0;i<7;i++){
   assert(7+textw(g[i].key,0)+5<=keyw);
   assert(keyw+textw(g[i].action,0)<=367);
  }
 }
}
static void turns(void){
 init_city();
 navCount=4;
 navNodes[0]=(NavPoint){256,256};navNodes[1]=(NavPoint){768,256};
 navNodes[2]=(NavPoint){768,768};navNodes[3]=(NavPoint){1280,768};
 NavReadout a=nav_readout((NavPoint){400,256});
 assert(a.turn==1&&a.until==368&&a.total==1392);
 NavReadout b=nav_readout((NavPoint){500,256});
 assert(b.turn==1&&b.until==268&&b.total==a.total-100);
 a=nav_readout((NavPoint){768,420});
 assert(a.turn==-1&&a.until==348&&a.total==860);
 a=nav_readout((NavPoint){870,768});assert(!a.turn&&a.total==410);
 /* A tiny final road-to-curb offset is not a turn instruction. */
 navCount=3;navNodes[2]=(NavPoint){768,272};a=nav_readout((NavPoint){400,256});assert(!a.turn);
 navCount=0;a=nav_readout((NavPoint){400,256});assert(!a.turn&&!a.total);
}
static void atlas_sweep(void){
 int renders=0,walkins=0;
 scene=1;sv.intro=4;job.kind=-1;mapOn=1;
 for(int i=0;i<LEN(loc)+nb;i++){
  if(i>=LEN(loc)&&!ordinaryDoorY[i-LEN(loc)])continue;
  navSelected=i;navHover=navKeyboard=1;navManual=nav_door(i);navManualLoc=i;navHasManual=1;
  for(int car=-1;car<=0;car++){p.car=car;paint();renders++;}
  if(i>=LEN(loc)){
   int b=i-LEN(loc),big,tracking;const char*name=walkin_name(b);
   assert(22+wi_title_width(name,&big,&tracking)<=W-291-12);
   assert(big); /* All current storefront headings retain display type. */
   walkin_open(b);paint();walkins++;scene=1;mapOn=1;
  }else{
   open_interior(i);paint();scene=1;mapOn=1;
  }
 }
 assert(renders==156&&walkins==63);
 printf("Rendered 78 interiors and %d on-foot/driving atlas venue states.\n",renders);
}
static void clock_surfaces(void){
 init_city();sv.intro=4;sv.dayClock=1439.f;
 scene=1;char time[6];day_clock_label(time);assert(!strcmp(time,"23:59"));
 assert(textw(time,0)<DAY_BADGE_W-34);
 paint();assert(pixels[16*W+DAY_BADGE_X]==0x415766);
 mapOn=1;paint();assert(pixels[16*W+DAY_BADGE_X]==0x415766);
 open_interior(1);paint();assert(pixels[5*W+DAY_BADGE_X]==0x415766);
 for(int i=0;i<nb;i++)if(ordinaryDoorY[i]){walkin_open(i);break;}
 paint();assert(scene==4&&pixels[5*W+DAY_BADGE_X]==0x415766);
 assert(sv.dayClock==1439.f); /* Painting is not an active tick. */
}
/* Native raster measurements across walking poses. Include all non-shadow
 * body ink and assert the old portrait and facing marker remain pixel-exact. */
static void smaller_world_figures(void){
 const U background=0x070a0d;
 const int n=128*3;
 U arrows=2166136261u,portraits=2166136261u;
 int widths[2]={0},heights[2]={0};
 sv.look=defaultLook;
 for(int i=0;i<128;i++){
  float a=i*6.283186f/128;
  Pt feet=rig(480,260,cs(a),sn(a),0,0,0);
  assert(feet.x==480&&feet.y==260);
  for(int gait=-5;gait<=5;gait+=5){
   for(int style=0;style<2;style++){
    int l=520,r=440,t=300,b=210;
    rect(440,210,80,100,background);
    if(!style)figure(480,260,0xc6d979,gait,a,0,3,&sv.look);
    else person(480,260,0xd68a61,gait,a,1,2);
    for(int y=220;y<295;y++)for(int x=440;x<520;x++){
     U c=pixels[y*W+x];if(c==background||c==0x19232b)continue;
     l=min(x,l);r=max(x,r);t=min(y,t);b=max(y,b);
    }
    assert(t<260&&b>=260); /* Head rises above the feet; soles remain at street level. */
    widths[style]+=r-l+1;heights[style]+=b-t+1;
   }
  }
  rect(436,210,88,100,background);
  person(480,260,0xc6d979,0,a,4,3);
  for(int y=217;y<306;y++)for(int x=436;x<524;x++)
   arrows=(arrows^(pixels[y*W+x]==0xffffff))*16777619u;
 }
 for(int i=0;i<2;i++){
  /* Standard rig previously averaged 15.43 x 24.49 pixels. */
  assert(widths[i]>=13*n&&widths[i]<=14*n&&heights[i]>=21*n&&heights[i]<=22*n);
 }
 for(int i=0;i<4;i++){
  portrait(0,0,180,150,&defaultLook,3,i);
  for(int y=0;y<150;y++)for(int x=0;x<180;x++)
   portraits=(portraits^pixels[y*W+x])*16777619u;
 }
 assert(arrows==0x769095b8u&&portraits==0xffda14e5u);
 puts("Passed: 768 smaller live player/NPC poses, fixed foot anchors, and pixel-identical facing arrows and career portraits.");
}
int main(void){
 test_cards();turns();atlas_sweep();clock_surfaces();smaller_world_figures();
 puts("Passed: full-width notifications and objective text, field-guide columns, bend direction / countdown / arrival, small clock renders and all 78 atlas entries.");
}
