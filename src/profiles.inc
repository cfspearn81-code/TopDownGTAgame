/* Save browser and in-world character creator. Never creates a slot before commit. */
enum{PROFILE_SLOTS=252,PROFILE_PAGE=5,CREATOR_ROWS=CHAR_FIELDS+1};
typedef struct{int id,status,cash,dirty;U ticks,legacy,intro;Look look;char name[20];} Card;
static Card cards[PROFILE_SLOTS];
static int cardCount,cardSelected,profileReturn,profileHoverX=-1,profileHoverY=-1;
static int createField,createWriting,createFacing,createPreset;
static U styleSeed=0x82ef1cabu;
static Look draft;
static char draftName[20],oldDraftName[20];
static const char *profileError;
static Card deleteCard;
static int deleteOpen,deleteArmed,profileSuccess;
static int may_delete(void){return cardCount&&(!profileReturn||cards[cardSelected].id!=sideSlot);}
static void begin_delete(void){
 if(!cardCount)return;
 profileSuccess=0;
 if(!may_delete()){profileError="Current career is protected. Delete it from the launch screen.";return;}
 deleteCard=cards[cardSelected];deleteOpen=1;deleteArmed=0;profileError=0;
}
static void confirm_delete(void);
static int profile_page(void){return cardSelected/PROFILE_PAGE;}
static void profiles_scan(void){
 cardCount=0;int selected=0;
#if defined(_WIN_NATIVE) && !defined(DC_TEST)
 Save held;cp(&held,&sv,sizeof sv);int loaded=wason;
 for(int i=0;i<PROFILE_SLOTS;i++){
  disk_slot(i);if(!disk_present())continue;
  load_save();
  Card *c=cards+cardCount;
  *c=(Card){.id=i,.status=wason,.cash=sv.cash,.dirty=sv.dirty,.ticks=sv.playedTicks,.legacy=sv.legacyClock,.intro=sv.intro,.look=sv.look};
  cp(c->name,sv.name,sizeof c->name);
  if(i==sideSlot)selected=cardCount;
  cardCount++;
 }
 cp(&sv,&held,sizeof sv);wason=loaded;disk_slot(sideSlot);
#else
 if(wason)cards[cardCount++]=(Card){.id=sideSlot,.status=1,.cash=sv.cash,.dirty=sv.dirty,.ticks=sv.playedTicks,.legacy=sv.legacyClock,.intro=sv.intro,.look=sv.look};
#endif
 cardSelected=selected;
}
static void profiles_init(void){
 deleteOpen=deleteArmed=profileSuccess=0;profileReturn=0;profileError=0;profileHoverX=profileHoverY=-1;createField=createWriting=createFacing=0;
 profiles_scan();styleSeed=taxiSeed^0x82ef1cabu;if(!styleSeed)styleSeed=1;
}
static void confirm_delete(void){
 if(!deleteOpen)return;
 deleteOpen=deleteArmed=0;
 if(profileReturn&&deleteCard.id==sideSlot)return;
 int ok=0;
#if defined(_WIN_NATIVE) && !defined(DC_TEST)
 disk_slot(deleteCard.id);ok=disk_delete();disk_slot(sideSlot);
#else
 if(deleteCard.id==sideSlot){wason=0;ok=1;}
#endif
 int old=cardSelected;
 profiles_scan();cardSelected=cl(old,0,max(0,cardCount-1));
 profileSuccess=ok;
 profileError=ok?"Save deleted.":"Could not delete that save. No other save was touched.";
}
static void new_profile(void){
 draft=defaultLook;z(draftName,sizeof draftName);append(draftName,"Vanta Local");
 createField=1;createWriting=0;createFacing=0;createPreset=-1;profileError=0;scene=5;
}
static void profiles_open(int create){
 request_save();
 profileReturn=scene;scene=0;deleteOpen=profileSuccess=0;profileError=saveFlash<0?"The current career could not be saved. Esc to return.":0;
 profiles_scan();if(create&&!profileError)new_profile();
}
static void profiles_activate(void){
 end_job();spawnCars();spawnPlayer();shelterT=0;z(wi_sw,sizeof wi_sw);z(&taxi,sizeof taxi);z(fx,sizeof fx);z(pedHit,sizeof pedHit);
 navSlot=-1;walking=fn=deadAnim=saveFlash=0;punchFlash=crashFlash=0;lastCar=near=by=-1;
 lastTaxiPickup=lastTaxiDrop=lastTaxiRider=-1;menu=-1;mapOn=0;profileReturn=0;profileError=0;
 if(sv.intro==0)open_interior(12);
 else{scene=1;tell(sv.intro<3?"FIRST SHIFT: follow the purple route to Mercury Dispatch.":"Career loaded. Press M to pick a destination.");}
}
static void profiles_choose(void){
 profileSuccess=0;
 if(profileReturn&&saveFlash<0){scene=profileReturn;return;}
 if(!cardCount){new_profile();return;}
 Card *c=cards+cardSelected;
 if(!c->status){profileError="This file could not be loaded. It has not been changed.";return;}
 if(profileReturn&&sideSlot==c->id){scene=profileReturn;return;}
#if defined(_WIN_NATIVE) && !defined(DC_TEST)
 Save held;cp(&held,&sv,sizeof sv);int loaded=wason;
 disk_slot(c->id);load_save();
 if(!wason){cp(&sv,&held,sizeof sv);wason=loaded;disk_slot(sideSlot);profiles_scan();profileError="This file could not be loaded. It has not been changed.";return;}
#endif
 sideSlot=c->id;profiles_activate();
}
static void creator_commit(void){
 int oldSlot=sideSlot;
 Save held;cp(&held,&sv,sizeof sv);
 starter_save();sv.look=draft;
 int len=sl(draftName);
 while(len>0&&draftName[len-1]==' ')len--;
 int start=0;while(start<len&&draftName[start]==' ')start++;
 if(start==len)append(sv.name,"Vanta Local");else cp(sv.name,draftName+start,len-start);
 sv.hash=savehash();
#if defined(_WIN_NATIVE) && !defined(DC_TEST)
 int id=-1;
 for(int i=0;i<PROFILE_SLOTS;i++){
   if(i==1)continue; /* Historical side-career path is load-only unless it already exists. */
   disk_slot(i);if(disk_present())continue;
   if(disk_create(&sv,sizeof sv))id=i;
   break;
 }
 if(id<0){disk_slot(oldSlot);cp(&sv,&held,sizeof sv);profileError="Could not write a new career. No existing save was replaced.";return;}
 sideSlot=id;
#else
 sideSlot=oldSlot+1;
#endif
 wason=1;profiles_activate();
}
static void creator_change(int f,int change){
 if(f==0){cp(oldDraftName,draftName,sizeof draftName);createWriting=1;return;}
 int i=f-1,count=wardrobe[i].count;draft.part[i]=(draft.part[i]+count+change)%count;createPreset=-1;
}
static U style_random(void){styleSeed^=styleSeed<<13;styleSeed^=styleSeed>>17;styleSeed^=styleSeed<<5;return styleSeed;}
static void creator_shuffle(void){for(int i=0;i<CHAR_FIELDS;i++)draft.part[i]=style_random()%(U)wardrobe[i].count;createPreset=-1;}
enum{LOOKBOOK_COUNT=360};
static void creator_preset(int step){
 int i=createPreset<0?(step<0?LOOKBOOK_COUNT-1:0):(createPreset+step+LOOKBOOK_COUNT)%LOOKBOOK_COUNT;
 createPreset=i;
 int haircut=(i/6)%10,top=i%6,tone=i/60;
 draft=(Look){{(i+2*haircut)%3,(haircut+3*tone+top)%8,haircut,(top*3+tone)%8,(tone+haircut+top)%7,top,(tone*2+top*3)%12,
  (haircut+2*top+tone)%5,(2*tone+3*top)%8,(top+tone*3+haircut)%6}};
}
void text_input(U ch){
 if(scene!=5||!createWriting)return;
 int n=sl(draftName);
 if(ch==8&&n)draftName[n-1]=0;
 if(ch>=32&&ch<=126&&n<19){draftName[n]=(char)ch;draftName[n+1]=0;}
}
static void profiles_tick(U tap){
 if(scene==0){
  if(deleteOpen){
   if(tap&ESC){deleteOpen=0;return;}
   if(tap&LEFT)deleteArmed=0;
   if(tap&RIGHT)deleteArmed=1;
   if(tap&(LEFT|RIGHT))return; /* Direction and confirm must be separate presses. */
   if(tap&ENTER){if(deleteArmed)confirm_delete();else deleteOpen=0;}
   return;
  }
  if(tap&DELETE_SLOT){begin_delete();return;}
  if(tap&ESC){if(profileReturn)scene=profileReturn;return;}
  if((tap&NEW_SLOT)&&!(profileReturn&&saveFlash<0)){new_profile();return;}
  if(cardCount){
   if(tap&(DOWN|SLOT))cardSelected=(cardSelected+1)%cardCount;
   if(tap&UP)cardSelected=(cardSelected+cardCount-1)%cardCount;
   if(tap&LEFT)cardSelected=max(0,cardSelected-PROFILE_PAGE);
   if(tap&RIGHT)cardSelected=min(cardCount-1,cardSelected+PROFILE_PAGE);
  }
  if(tap&(ENTER|USE))profiles_choose();
  return;
 }
 if(createWriting){
  if(tap&ESC){cp(draftName,oldDraftName,sizeof draftName);createWriting=0;}
  else if(tap&ENTER)createWriting=0;
  return;
 }
 if(tap&ESC){scene=0;profileError=0;return;}
 if(tap&UP)createField=(createField+CREATOR_ROWS-1)%CREATOR_ROWS;
 if(tap&DOWN)createField=(createField+1)%CREATOR_ROWS;
 if(tap&LEFT)creator_change(createField,-1);
 if(tap&RIGHT)creator_change(createField,1);
 if(tap&FIRE)creator_shuffle();
 if(tap&MAP)createFacing=(createFacing+1)&3;
 if(tap&GFX)creator_preset(1);
 if(tap&USE){if(!createField)creator_change(0,1);else creator_change(createField,1);}
 if(tap&ENTER)creator_commit();
}
static int in_ui(int x,int y,int w,int h){return profileHoverX>=x&&profileHoverY>=y&&profileHoverX<x+w&&profileHoverY<y+h;}
static void profiles_pointer(int sx,int sy,U clicks){
 profileHoverX=sx;profileHoverY=sy;if(!(clicks&1))return;
 if(scene==0){
  if(deleteOpen){
   if(in_ui(276,315,176,38))deleteOpen=0;
   else if(in_ui(508,315,176,38))confirm_delete();
   return;
  }
  if(in_ui(686,348,210,42)){profiles_choose();return;}
  if(in_ui(686,399,210,38)&&!(profileReturn&&saveFlash<0)){new_profile();return;}
  if(cardCount&&in_ui(686,446,210,32)){begin_delete();return;}
  if(profileReturn&&in_ui(686,483,210,27)){scene=profileReturn;return;}
  if(in_ui(64,454,44,30))cardSelected=max(0,cardSelected-PROFILE_PAGE);
  if(in_ui(602,454,44,30)&&cardCount)cardSelected=min(cardCount-1,cardSelected+PROFILE_PAGE);
  if(in_ui(64,142,582,300)){
   if(!cardCount){new_profile();return;}
   int i=profile_page()*PROFILE_PAGE+(sy-142)/60;
   if((sy-142)%60<54){
    if(i<cardCount){cardSelected=i;profileError=saveFlash<0?profileError:0;}
    else if(!(profileReturn&&saveFlash<0))new_profile();
   }
  }
  return;
 }
 if(createWriting&&in_ui(64,138,544,26))return;
 if(createWriting)createWriting=0;
 if(in_ui(689,455,207,42)){creator_commit();return;}
 if(in_ui(64,468,107,33)){scene=0;profileError=0;return;}
 if(in_ui(462,468,146,33)){creator_shuffle();return;}
 if(in_ui(689,358,207,34)){createFacing=(createFacing+1)&3;return;}
 if(in_ui(689,403,207,32)){creator_preset(sx<727?-1:1);return;}
 if(in_ui(64,138,544,CREATOR_ROWS*28)){
  int f=(sy-138)/28;createField=f;
  if(!f||sx>=558)creator_change(f,1);
  else if(sx>=320&&sx<364)creator_change(f,-1);
 }
}
static void profile_bg(void){
 rect(0,0,W,H,0x09131c);
 for(int y=0;y<H;y+=4)line(0,y,W-1,y,0x0d1a24);
 for(int x=0;x<W;x+=40)line(x,0,x,H-1,0x10202b);
 for(int i=0;i<15;i++){
  int x=662+i*15,h=17+(i*67+21)%83;
  rect(x,H-h-8,13,h,0x152633);
  rect(x+2,H-h-6,3,min(h-4,7),0x284048);
 }
 rect(0,0,W,5,TEAL);rect(0,H-3,W,3,0x293e48);
}
static void profile_button(int x,int y,int w,int h,const char*label,U ink,int enabled){
 int over=in_ui(x,y,w,h)&&enabled;
 U c=!enabled?MUTED:over?DARK:ink;
 box(x,y,w,h,over?ink:0x14242e,enabled?ink:0x32434b);
 center(x+w/2,y+(h-20)/2,label,c,0);
}
static void portrait(int x,int y,int w,int h,const Look*l,int scale,int facing){
 static U thumb[54*44];
 U*old=surface;int ow=rw,oh=rh,ol=clipLeft,ot=clipTop,orr=clipRight,ob=clipBottom;
 surface=thumb;rw=clipRight=54;rh=clipBottom=44;clipLeft=clipTop=0;
 rect(0,0,54,44,0x10232c);
 for(int j=3;j<44;j+=5)line(0,j,53,j,0x152b35);
 for(int j=2;j<54;j+=6)line(j,0,j,43,0x152b35);
 disc(27,35,11,0x193a42);line(10,38,43,38,0x315963);
 static const float angles[]={1.570796f,0,-1.570796f,3.141593f};
 previewRig=1;figure(27,35,0,0,angles[facing],0,0,l);previewRig=0;
 surface=old;rw=ow;rh=oh;clipLeft=ol;clipTop=ot;clipRight=orr;clipBottom=ob;
 box(x,y,w,h,0x10232c,0x365362);
 int cw=min(w-2,54*scale),ch=min(h-2,44*scale);
 for(int j=0;j<ch;j++)for(int i=0;i<cw;i++){
  int a=(54-cw/scale)/2+i/scale,b=(44-ch/scale)/2+j/scale;
  dot(x+(w-cw)/2+i,y+(h-ch)/2+j,thumb[b*54+a]);
 }
}
static char *two(char*b,U n){*b++='0'+n/10;*b++='0'+n%10;*b=0;return b;}
static void time_display(char *b,U ticks){
 U secs=ticks/60,h=secs/3600;char*q=h<100?two(b,h):number(b,(int)h);
 *q++=':';q=two(q,secs/60%60);*q++=':';two(q,secs%60);
}
static const char *card_name(const Card*c){return c->name[0]?c->name:c->id==0?"Main Career":c->id==1?"Side Career":"Vanta Local";}
static void slot_label(char *out,int slot){if(!slot)append(out,"FILE 00 / MAIN");else if(slot==1)append(out,"FILE 01 / SIDE");else fmt(out,"FILE ",slot);}
static void profile_list_paint(void){
 textat(64,23,"PORT VANTA  /  DEPARTURES TERMINAL",TEAL,0);
 textat(64,51,"CAREER FILES",LIME,1);
 textat(65,104,"SELECT A SAVE TO LOAD OR START A NEW CAREER.",MUTED,0);
 textat(359,119,"PLAYTIME",MUTED,0);textat(479,119,"WALLET",MUTED,0);
 if(!cardCount){
  box(64,142,582,116,0x14242e,0x42646a);rect(64,142,4,116,TEAL);
  textat(83,163,"YOUR FIRST NIGHT IN VANTA",PAPER,0);
  textat(83,194,"Make a character, then start in Apartment 4.",MUTED,0);
  textat(83,221,"ENTER  /  CREATE YOUR CHARACTER",LIME,0);
 }
 for(int row=0;row<PROFILE_PAGE;row++){
  int idx=profile_page()*PROFILE_PAGE+row;
  if(idx>=cardCount){
   if(!cardCount&&row<2)continue;
   int y=142+row*60,over=in_ui(64,y,582,54)&&!(profileReturn&&saveFlash<0);
   box(64,y,582,54,over?0x192f39:0x0e1d26,over?TEAL:0x2a414c);
   box(76,y+5,52,44,0x0e1d26,0x2a414c);center(102,y+16,"+",over?TEAL:0x637f86,0);
   textat(141,y+15,"EMPTY  /  NEW CAREER",over?TEAL:0x789198,0);
   continue;
  }
  Card*c=cards+idx;int y=142+row*60,sel=idx==cardSelected,over=in_ui(64,y,582,54);
  char buf[36],t[20];
  U primary=sel?LIME:over?TEAL:0x324954;
  box(64,y,582,54,sel?0x20393c:0x12232e,primary);rect(64,y,4,54,primary);
  if(c->status)portrait(76,y+5,52,44,&c->look,1,0);
  else {box(76,y+5,52,44,DARK,0x5e4a54);center(102,y+18,"!",PINK,0);}
  textat(141,y+5,c->status?card_name(c):"Unreadable save",PAPER,0);
  slot_label(buf,c->id);textat(141,y+28,buf,sel?TEAL:MUTED,0);
  if(c->status){
   time_display(t,c->ticks);textat(359,y+5,c->legacy&&!c->ticks?"Untracked":t,PAPER,0);
   if(c->legacy&&c->ticks)textat(359,y+28,"Since update",MUTED,0);
   fmt(buf,"$ ",c->cash);textat(479,y+5,buf,CASH_GREEN,0);
   fmt(buf,"$ ",c->dirty);textat(479,y+28,buf,CASH_RED,0);
  }
 }
 box(666,112,250,406,0x12212b,0x3b5864);
 rect(666,112,250,3,TEAL);
 if(cardCount&&cards[cardSelected].status){
  Card*c=cards+cardSelected;portrait(686,133,210,156,&c->look,4,0);
  center(791,301,c->intro<3?"FIRST SHIFT":"PORT VANTA CAREER",TEAL,0);
  center(791,324,"CLEAN CASH / DIRTY MONEY",MUTED,0);
 }else{
  rect(686,138,210,155,0x10252f);
  for(int r=0;r<10;r++){int h=(r*23+27)%88;rect(690+r*20,275-h,17,h,0x233e4a);rect(694+r*20,279-h,3,7,r&1?TEAL:0x586067);}
  center(791,301,cardCount?"FILE UNAVAILABLE":"NO CAREER ON FILE",cardCount?PINK:MUTED,0);
 }
 profile_button(686,348,210,42,cardCount?"LOAD / RESUME":"CREATE CHARACTER",LIME,!cardCount||cards[cardSelected].status);
 profile_button(686,399,210,38,"F4  NEW CAREER",TEAL,!(profileReturn&&saveFlash<0));
 if(cardCount)profile_button(686,446,210,32,"DEL  DELETE SAVE",PINK,may_delete());
 if(profileReturn)profile_button(686,483,210,27,"ESC  BACK TO GAME",MUTED,1);
 if(cardCount>PROFILE_PAGE){
  profile_button(64,454,44,30,"<",TEAL,profile_page()>0);
  profile_button(602,454,44,30,">",TEAL,profile_page()<(cardCount-1)/PROFILE_PAGE);
  char a[44];char*b=number(append(a,"PAGE "),profile_page()+1);number(append(b," / "),(cardCount+PROFILE_PAGE-1)/PROFILE_PAGE);center(355,458,a,MUTED,0);
 }
 textat(64,502,profileError?profileError:"UP / DOWN  SELECT     ENTER  LOAD     F4  NEW CAREER",profileError?(profileSuccess?TEAL:PINK):MUTED,0);
}
static U creator_color(int f,int v){
 switch(f){case SKIN:return charSkin[v];case HAIR_TONE:return charHair[v];case TOP_TONE:return charTop[v];case LEG_TONE:return charLeg[v];case SHOES:return charShoe[v];default:return 0;}
}
static void profile_creator_paint(void){
 textat(64,23,"PORT VANTA  /  NEW CAREER",TEAL,0);
 textat(64,51,"CREATE YOUR CHARACTER",LIME,1);
 textat(65,104,"MIX AND MATCH YOUR LOOK BEFORE MOVING INTO APARTMENT 4.",MUTED,0);
 for(int f=0;f<CREATOR_ROWS;f++){
  int y=138+f*28,sel=f==createField,over=in_ui(64,y,544,26);
  U c=sel?LIME:over?TEAL:0x304755;
  box(64,y,544,26,sel?0x20393c:0x12232e,c);if(sel)rect(64,y,3,26,c);
  textat(78,y+3,f?wardrobe[f-1].label:"NAME",sel?LIME:MUTED,0);
  if(!f){
   textat(347,y+3,draftName,PAPER,0);
   if(createWriting){int x=347+textw(draftName,0);rect(x+1,y+5,2,16,TEAL);}
   else textat(559,y+3,"EDIT",TEAL,0);
  }else{
   int v=draft.part[f-1];U swatch=creator_color(f-1,v);
   textat(374,y+3,wardrobe[f-1].name[v],PAPER,0);
   if(swatch)box(347,y+6,14,14,swatch,0x69838b);
   profile_button(320,y+2,25,22,"<",TEAL,1);
   profile_button(558,y+2,42,22,">",TEAL,1);
  }
 }
 box(666,115,250,393,0x12212b,0x3b5864);rect(666,115,250,3,TEAL);
 center(791,126,"LIVE PREVIEW",TEAL,0);
 portrait(686,153,210,194,&draft,5,createFacing);
 profile_button(689,358,207,34,"M  TURN CHARACTER",TEAL,1);
 char book[48];if(createPreset<0)append(book,"F2  BROWSE 360 LOOKS");else number(append(number(append(book,"LOOK "),createPreset+1)," / "),LOOKBOOK_COUNT);
 profile_button(689,403,34,32,"<",TEAL,1);profile_button(728,403,129,32,book,TEAL,1);profile_button(862,403,34,32,">",TEAL,1);
 profile_button(689,455,207,42,"ENTER  START CAREER",LIME,1);
 profile_button(64,468,107,33,"ESC  BACK",MUTED,1);
 profile_button(462,468,146,33,"SPACE  SHUFFLE",TEAL,1);
 textat(64,510,profileError?profileError:createWriting?"TYPE A NAME. ENTER ACCEPTS. ESC UNDOES.":"UP / DOWN  CATEGORY     LEFT / RIGHT  CHANGE     E  EDIT OR CHANGE",profileError?PINK:MUTED,0);
}
static void delete_prompt_paint(void){
 modal(247,156,466,216,PINK);
 textat(276,172,"DELETE SAVE?",PINK,1);
 char label[70],slot[24];slot_label(slot,deleteCard.id);
 char *d=append(label,deleteCard.status?card_name(&deleteCard):"Unreadable save");d=append(d,"  /  ");append(d,slot);
 textat(276,225,label,PAPER,0);
 textat(276,256,"This permanently removes the selected save.",MUTED,0);
 textat(276,277,"It cannot be undone.",MUTED,0);
 profile_button(276,315,176,38,"CANCEL",deleteArmed?MUTED:LIME,1);
 profile_button(508,315,176,38,"DELETE SAVE",deleteArmed?PINK:0xc5778d,1);
}
static void profiles_paint(void){
 profile_bg();if(scene==5)profile_creator_paint();else{profile_list_paint();if(deleteOpen)delete_prompt_paint();}
}
