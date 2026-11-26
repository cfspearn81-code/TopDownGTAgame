/* Shared fixed-coordinate radial selector. It owns neither a separate host
 * cursor nor a simulated weapon: links point to the native gear and wallet. */
enum {
 WW_CX=W/2,WW_CY=267,WW_IN=102,WW_OUT=202,WW_DIST=155,
 /* Reference: center (239, 267.5), R210.5; parachute (413, 441).
  * Scale that center-to-center offset by our ring's radius. */
 WW_MED_X=WW_CX+167,WW_MED_Y=WW_CY+167,WW_MED_R=26,
 WW_MED_CARD_X=680,WW_MED_CARD_Y=399,WW_MED_CARD_W=252,WW_MED_CARD_H=70,
 WW_TOP=0,WW_RIGHT_UP,WW_RIGHT,WW_RIGHT_DOWN,WW_BOTTOM,WW_LEFT_DOWN,WW_LEFT,WW_LEFT_UP,
 WW_NO_FEEDBACK=0,WW_FULL_HP,WW_NONE_LEFT,WW_HEALED
};
static int wheel_allowed(void){
 return (scene==1||scene==3||scene==4)&&!p.dead&&!mapOn&&menu<0&&!vdPanel&&atmOpen<0;
}
static int wheel_med_hover(void){
 return d2(wheelX-WW_MED_X,wheelY-WW_MED_Y)<=WW_MED_R*WW_MED_R ||
        (wheelX>=WW_MED_X&&wheelX<WW_MED_CARD_X+WW_MED_CARD_W&&
         wheelY>=WW_MED_CARD_Y&&wheelY<WW_MED_CARD_Y+WW_MED_CARD_H);
}
static int wheel_nearest(void){
 if(sv.gear<0)return 1;
 /* The utility is not a weapon. Deliberately hovering beside its
  * V action cannot switch away from the current weapon preview. */
 if(wheelY<0||wheel_med_hover())return wheelOpen?wheelPreviewFists:wheelFists;
 int dx=wheelX-WW_CX,y=wheelY-WW_CY;
 float high=d2(dx,y+WW_DIST),low=d2(dx,y-WW_DIST);
 return high==low?wheelPreviewFists:low<high;
}
static int wheel_tick(U keys,U tap){
 if(!(keys&WHEEL)){
  if(!wheelOpen)return 0;
  /* Only a normal held→released transition equips. Focus loss uses
   * clear_input(), so it discards a pending hover. Off-frame motion does
   * not create a second choice. Require a fresh combat input on release. */
  if(wheel_allowed())wheelFists=wheel_nearest();
  wheelSuppressed|=keys&(FIRE|PUNCH);
  wheelOpen=wheelFeedback=0;return 1;
 }
 if(!wheel_allowed())return 0;
 if(!wheelOpen){wheelPreviewFists=!pistol_equipped();wheelFeedback=WW_NO_FEEDBACK;}
 wheelPreviewFists=wheel_nearest();wheelOpen=1;
 if(tap&USE_PATCH){
  if(p.hp>=100)wheelFeedback=WW_FULL_HP;
  else if(!sv.medPatches)wheelFeedback=WW_NONE_LEFT;
  else{
   --sv.medPatches;p.hp=min(100,p.hp+50);wheelFeedback=WW_HEALED;
   tell("Med patch used. Restored up to 50 HP.");request_save();
  }
 }
 return 1;
}
static int wheel_sector(int x,int y){
 int ax=x<0?-x:x,ay=y<0?-y:y;
 if(ax*1000<ay*400)return y<0?WW_TOP:WW_BOTTOM;
 if(ay*1000<ax*400)return x<0?WW_LEFT:WW_RIGHT;
 if(ax*1000<ay*429||ay*1000<ax*429)return -1;
 return y<0?(x<0?WW_LEFT_UP:WW_RIGHT_UP):(x<0?WW_LEFT_DOWN:WW_RIGHT_DOWN);
}
static void wheel_icon_fists(int x,int y,U ink){
 U shade=mix(ink,DARK,4);
 for(int j=0;j<4;j++){
  int u=x-15+7*j,v=y-13+(j==0?4:j==3?2:0);box(u,v,9,16,ink,shade);
  rect(u+1,v+1,5,2,mix(ink,PAPER,3));
 }
 box(x-13,y-3,26,15,ink,shade);rect(x-11,y+10,20,5,shade);
 disc(x+10,y+2,5,shade);disc(x+11,y,4,ink);line(x-7,y+4,x+4,y+4,shade);
}
static void wheel_icon_pistol(int x,int y,U ink){
 U deep=mix(ink,DARK,4),lit=mix(ink,PAPER,3);
 box(x-22,y-12,40,12,ink,deep);rect(x+16,y-10,8,5,deep);
 rect(x-21,y-13,5,2,deep);rect(x+11,y-14,4,2,deep);
 rect(x-17,y-10,27,2,lit);for(int j=0;j<3;j++)rect(x-16+j*3,y-7,1,4,deep);
 tri(x-19,y-2,x-5,y-2,x-12,y+14,ink);tri(x-19,y-2,x-23,y+13,x-12,y+14,ink);
 line(x-19,y+9,x-12,y+9,deep);line(x-21,y+12,x-13,y+12,deep);
 box(x-6,y-1,14,10,deep,ink);line(x-1,y,x-3,y+4,ink);
 if(sv.gear>0){rect(x+9,y-8,7,3,TEAL);dot(x+17,y-7,PAPER);}
}
static void wheel_icon_patch(int x,int y){
 const U edge=0xa5c5ad,a=0x536b61;
 box(x-16,y-13,32,25,a,edge);
 box(x-13,y-10,26,19,0x668877,0xaed3bd);
 rect(x-3,y-7,6,13,PAPER);rect(x-7,y-4,14,7,PAPER);
 dot(x-14,y-11,PAPER);dot(x+12,y-11,PAPER);
}
static void wheel_med_utility(void){
 int x=WW_MED_CARD_X,y=WW_MED_CARD_Y;
 disc(WW_MED_X,WW_MED_Y,WW_MED_R,0x85a795);
 disc(WW_MED_X,WW_MED_Y,WW_MED_R-2,0x13242b);
 disc(WW_MED_X,WW_MED_Y,WW_MED_R-5,0x344f50);
 wheel_icon_patch(WW_MED_X,WW_MED_Y);
 box(x,y,WW_MED_CARD_W,WW_MED_CARD_H,0x152631,0x628678);
 char row[56];
 number(append(row,"MED PATCHES  x"),sv.medPatches);textat(x+14,y+3,row,0xa6d8b6,0);
 number(append(number(append(row,"V USE ONE   HP "),p.hp),"/"),100);
 textat(x+14,y+24,row,PAPER,0);
 const char*hint=wheelFeedback==WW_HEALED?"PATCH USED":wheelFeedback==WW_FULL_HP?"FULL HP / NO PATCH USED":wheelFeedback==WW_NONE_LEFT?"NO PATCHES IN STOCK":"HEALS UP TO 50 HP";
 textat(x+14,y+45,hint,wheelFeedback==WW_NONE_LEFT?GOLD:wheelFeedback==WW_HEALED?LIME:MUTED,0);
}
static void wheel_paint(void){
 int selected=wheelPreviewFists?WW_BOTTOM:WW_TOP;
 U picked=0x63d7cf;
 for(int i=0;i<W*H;i++)pixels[i]=((pixels[i]>>1)&0x007f7f7f)+0x05080e;
 for(int y=-WW_OUT;y<=WW_OUT;y++)for(int x=-WW_OUT;x<=WW_OUT;x++){
  int rr=x*x+y*y;if(rr>WW_OUT*WW_OUT)continue;
  int at=(WW_CY+y)*W+WW_CX+x;
  if(rr<WW_IN*WW_IN){pixels[at]=mix(pixels[at],0x15232d,6);continue;}
  int k=wheel_sector(x,y),rim=rr>(WW_OUT-4)*(WW_OUT-4)||rr<(WW_IN+2)*(WW_IN+2);
  if(k<0){pixels[at]=0x233741;continue;}
  U col=k==selected?(rim?picked:0x31696e):k==WW_TOP&&sv.gear>=0?0x45545c:0x34444f;
  if(rim&&k!=selected)col=0x62757e;
  if(!rim)col=mix(pixels[at],col,7);
  pixels[at]=col;
 }
 int top=WW_CY-WW_DIST,bottom=WW_CY+WW_DIST;
 if(sv.gear>=0){wheel_icon_pistol(WW_CX,top-7,selected==WW_TOP?PAPER:0xb8c5ca);center(WW_CX,top+14,"PULSE PISTOL",selected==WW_TOP?PAPER:MUTED,0);}
 else{box(WW_CX-9,top-9,19,13,0x75838e,0x4f5c68);rect(WW_CX-6,top-14,13,5,0x75838e);rect(WW_CX-3,top-12,7,4,0x34444f);center(WW_CX,top+12,"EMPTY",MUTED,0);}
 wheel_icon_fists(WW_CX,bottom-8,selected==WW_BOTTOM?0xf2c5a0:0xcaa88b);
 center(WW_CX,bottom+14,"FISTS",selected==WW_BOTTOM?PAPER:MUTED,0);
 center(WW_CX,WW_CY-61,"LOADOUT",MUTED,0);
 center(WW_CX,WW_CY-37,selected==WW_TOP?"PISTOL":"FISTS",PAPER,1);
 center(WW_CX,WW_CY+10,selected==WW_TOP?(sv.gear>0?"AMPLIFIED":"OWNED WEAPON"):"UNARMED",picked,0);
 center(WW_CX,WW_CY+37,"RELEASE TAB",PAPER,0);
 int x=27,y=WW_CY-64;
 box(x,y,233,127,0x13252c,0x465d67);textat(x+15,y+9,"HOLD TAB / WEAPONS",picked,0);
 textat(x+15,y+37,"Point near a weapon.",PAPER,0);
 textat(x+15,y+60,"Release Tab to equip.",PAPER,0);
 textat(x+15,y+91,"The city is paused.",MUTED,0);
 wheel_med_utility();
 if(wheelX>=0&&wheelY>=0){diamond(wheelX,wheelY,7,DARK);diamond(wheelX,wheelY,5,PAPER);dot(wheelX,wheelY,PAPER);}
}
