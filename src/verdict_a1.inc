/* SIDE B BEFORE SIGN-OFF. The rack stands on the concourse, a full aisle
 * away from the fare board. No inventory, car ownership or save-layout change. */
#define VD_HAS_A1 1
enum {A1_HALL=31,A1_ARRIVAL=69,A1_STUDIO=57,A1_PAY=95,A1_SLOT=36*60};
static int a1AudioPending;
int take_radio_record(void){int track=a1AudioPending;a1AudioPending=0;return track;}
static int a1Track,a1Ticks,a1Headline,a1Late,a1BroadcastFrames,a1StreetFrames,a1AwaitStreet;
static const char*const a1Titles[]={"","Seawall Nocturne","Last Tram Home"};
static int a1_rack_y(void){return wi_f[0].y+wi_f[0].h+91;}
static int a1_rack_near(void){return scene==4&&walkin_active==A1_HALL&&d2(wi_x-(wi_f[0].x+wi_f[0].w/2),wi_y-a1_rack_y())<32.f*32.f;}
static int a1_can_ride(void){return vdActive==VA_A1&&a1Track;}
static const char*a1_headline(void){return a1Headline==1?"SEAWALL NOCTURNE lights the late waterfront":"LAST TRAM HOME becomes the night-shift anthem";}
static void a1_pick(void){
 vd_choose("Harbourlight Radio wants one record. $95 clean either way. Tram: $5 clean cash, then walk. Or walk across town free.",
  "Seawall Nocturne / A moon over silver waves. Slow piano and brushed snare.",
  "Last Tram Home / A ticket punched in red. Clacking wheels and a bright brass refrain.");
}
static void a1_sleeve(int x,int y,int size,int track){
 U ink=track==1?0x99e8df:0xffb089,base=track==1?0x1a3545:0x4e2832;
 box(x,y,size,size,base,ink);
 if(track==1){disc(x+size*2/3,y+size/3,max(1,size/7),ink);for(int j=0;j<3;j++)for(int i=0;i<size-8;i+=7)line(x+4+i,y+size*2/3+j*3,x+min(size-4,8+i),y+size*2/3+j*3-2,ink);}
 else{rect(x+size/6,y+size/4,size*2/3,size/2,ink);rect(x+size/4,y+size/3,size/2,max(1,size/8),base);disc(x+size/3,y+size*3/4,max(1,size/9),base);disc(x+size*2/3,y+size*3/4,max(1,size/9),base);}
}
static void a1_draw_choice(void){a1_sleeve(791,236,38,1);a1_sleeve(791,316,38,2);}
static int vd_a1(int ev,U keys,U tap){
 (void)keys;
 if(ev==VE_RESET){a1AudioPending=0;a1Track=a1Ticks=a1Headline=a1Late=a1BroadcastFrames=a1StreetFrames=a1AwaitStreet=0;return 0;}
 if(ev==VE_CANCEL){a1Track=a1Ticks=0;return 0;}
 if(ev==VE_START){
  a1Track=0;a1Ticks=0;vd_goal(ordinaryDoorX[A1_HALL],ordinaryDoorY[A1_HALL],walkin_name(A1_HALL));
  vd_status("Blue Hour Tram Hall: meet the DJ at the record rack beside the timetable.");
  if(a1_rack_near())a1_pick();return 0;
 }
 if(ev==VE_AMBIENT){
  if(scene==4&&walkin_active==A1_STUDIO&&a1BroadcastFrames>0){
   a1BroadcastFrames--;
   if(vdActive<0&&job.kind<0&&wi_at_station()&&(tap&USE)){tell(a1Late?"Encore on air. Your $95 fee is already paid.":"Live set on air. Your $95 fee is already paid.");return 1;}
  }
  if(scene==1&&a1AwaitStreet){a1AwaitStreet=0;a1StreetFrames=60*12;}
  else if(scene==1&&a1StreetFrames>0)a1StreetFrames--;
  if(vdActive<0&&job.kind<0&&a1_rack_near()&&(tap&USE)){
   vd_launch(VA_A1);return 1;
  }return 0;
 }
 if(ev==VE_TICK){
  if(vdChoice&&!a1Track){
   a1Track=vdChoice;a1Ticks=A1_SLOT;
   vd_goal(ordinaryDoorX[A1_STUDIO],ordinaryDoorY[A1_STUDIO],walkin_name(A1_STUDIO));
   vd_status("Harbourlight Radio mixing desk / 36s live slot (job clock). $5 rail, or walk free.");
   tell("Sleeve packed. Harbourlight Radio is marked. Cross town free or ride for $5.");return 1;
  }
  if(!a1Track){if(a1_rack_near()&&(tap&USE)){a1_pick();return 1;}return 0;}
  if(a1Ticks>0)a1Ticks--;
  if(a1Ticks%15==0||a1Ticks==A1_SLOT-1){
   char m[150];char*q=append(m,a1Titles[a1Track]);q=append(q," / Harbourlight Radio mixing desk. ");
   if(a1Ticks){q=number(q,(a1Ticks+59)/60);append(q,"s live slot (job clock). $5 rail / walk free.");}
   else append(q,"Late set open. Walk free or ride for $5.");vd_status(m);
  }
  if(scene==4&&walkin_active==A1_STUDIO&&wi_at_station()&&(tap&USE)){
   a1AudioPending=a1Headline=a1Track;a1Late=!a1Ticks;a1BroadcastFrames=60*14;a1AwaitStreet=1;
   wi_sw[A1_STUDIO]=1;wi_bake();a1Track=a1Ticks=0;vd_end();
   gain(1,A1_PAY,a1Late?"Late set: you're our last voice tonight.":"On air: Harbourlight, this one's yours!");
   request_save();return 1;
  }return 0;
 }
 if(ev==VE_DECOR){
  if(scene==4&&walkin_active==A1_HALL){
   int x=wi_f[0].x+wi_f[0].w/2,y=a1_rack_y();box(x-25,y-17,51,23,0x473938,GOLD);
   a1_sleeve(x-20,y-23,18,1);a1_sleeve(x+2,y-23,18,2);rect(x-24,y+6,4,5,0x6e645e);rect(x+19,y+6,4,5,0x6e645e);
   if(a1_rack_near()&&(vdActive<0||vdActive==VA_A1)&&job.kind<0){
    const char*prompt=sv.intro<3?"DJ / after your first courier shift":a1Track?"Sleeve packed / studio marked":"E  /  Browse two record sleeves";
    int w=textw(prompt,0)+16;box(x-w/2,y+14,w,24,DARK,GOLD);center(x,y+15,prompt,GOLD,0);
   }else if(!a1Track){center(x,y+12,"DJ / RECORDS",GOLD,0);}
  }
  if(scene==4&&walkin_active==A1_STUDIO){
   if(a1Track&&wi_at_station()){rect(20,482,W-40,23,DARK);textat(26,483,"[E]   Play your record on Harbourlight Radio",GOLD,0);}
   if(a1BroadcastFrames){
    if(wi_at_station()&&!a1Track&&job.kind<0){rect(20,482,W-40,23,DARK);textat(26,483,"[E]   Read live broadcast status",GOLD,0);}
    int x=24,y=91;box(x,y,594,75,0x15272c,TEAL);
    textat(x+14,y+5,a1Late?"LIVE / AFTER-HOURS ENCORE":"LIVE / SIGN-OFF SLOT",GOLD,0);
    const char*caption=a1Headline==1?"[A slow piano rolls like water; brushes whisper.]":"[Brass calls over the clack of the last train.]";
    textat(x+14,y+27,a1Titles[a1Headline],PAPER,0);textat(x+14,y+48,caption,MUTED,0);
    for(int i=0;i<11;i++){int v=4+((i*7+a1BroadcastFrames/4)%21);rect(x+484+i*8,y+44-v,5,v,i<8?TEAL:GOLD);}
   }
  }
  if(scene==1&&a1Headline&&(a1StreetFrames||d2(p.x-ordinaryDoorX[A1_STUDIO],p.y-ordinaryDoorY[A1_STUDIO])<90*90)){
   const char*h=a1_headline(),*mast="THE VANTA LATE EDITION  /  HARBOURLIGHT RADIO";int w=max(textw(h,0),textw(mast,0))+28;box(17,160,w,54,0x17262c,0xbaac85);
   textat(30,162,mast,GOLD,0);
   textat(30,185,h,PAPER,0);
  }
 }
 return 0;
}
