/* Four independent issues of the Vanta Lantern. No persistent fields or economy.
   All custom devices use ENTER. E continues to operate the original room.
   The player's site and existing fixture geometry are authoritative. */
#define VD_HAS_A4 1
#define VD_HAS_B3A 1
#define VD_HAS_B3B 1
#define VD_HAS_B3C 1
enum{EX_HOME=56,EX_OVEN=54,EX_PAPER=2,EX_KELP=17,EX_PROOF=42,EX_FOOT=28,
     EX_CIRCLE=47,EX_DESK=38,EX_BLOCK0=37,EX_BLOCK1=28,EX_BLOCK2=30};
#define EX_INK 0x202c38u
#define EX_CREAM 0xf5dec0u
#define EX_UMBER 0x845944u
#define EX_LEAF 0x447d70u

static int ex_room(int b){return scene==4&&walkin_active==b;}
static int ex_street(void){return scene==1&&!mapOn;}
static void ex_venue_goal(int b){vd_goal(ordinaryDoorX[b],ordinaryDoorY[b],walkin_name(b));}
static void ex_clear_goal(void){vd_goal(-1,-1,"");}
static int ex_fixture_near(int b,int i){
 if(!ex_room(b)||i>=wi_n||wi_at_door()||wi_at_plaque())return 0;
 InteriorProp *f=wi_f+i;
 float x=wi_x-cl((int)wi_x,f->x,f->x+f->w),y=wi_y-cl((int)wi_y,f->y,f->y+f->h);
 return d2(x,y)<=31.f*31.f;
}
static void ex_tag(int x,int y,const char *s,U color){
 int w=textw(s,0)+12;x=cl(x,w/2+3,W-w/2-3);box(x-w/2,y,w,21,EX_INK,color);center(x,y-1,s,PAPER,0);
}
/* Labels stay attached to physical features, not at arbitrary screen edges. */
static void ex_fix_tag(int i,const char*s,U c){
 InteriorProp *f=wi_f+i;ex_tag(f->x+f->w/2,f->y-23,s,c);
}
static int ex_can_offer(void){return sv.intro>=3&&vdActive<0&&job.kind<0&&!p.dead&&menu<0&&!mapOn;}
static void ex_leaflet(int i,U ink){
 InteriorProp *f=wi_f+i;int x=f->x+f->w/2,y=f->y+f->h/2;
 box(x-12,y-7,25,16,EX_CREAM,EX_INK);rect(x-8,y-4,17,3,ink);
 for(int j=0;j<3;j++)rect(x-8,y+1+j*2,15-j*2,1,EX_UMBER);
}
static void ex_loaf(int x,int y){
 box(x-15,y-6,30,12,0x815334,EX_UMBER);
 for(int j=0;j<4;j++){
  disc(x-10+6*j,y-1+(j&1)*2,5,0xcb914f);
  line(x-12+6*j,y-2,x-8+6*j,y+2,0xfae1a3);
 }
}
static void ex_set_table(int i){
 InteriorProp *f=wi_f+i;int x=f->x+f->w/2,y=f->y+f->h/2;
 box(x-28,y-17,56,34,0xb34e47,0x673f3c);
 for(int j=-25;j<27;j+=8)rect(x+j,y-16,2,31,0xd27f68);
 for(int j=-14;j<16;j+=8)rect(x-27,y+j,54,2,0xde987a);
 for(int j=-1;j<=1;j+=2){
  disc(x+19*j,y+4,7,EX_CREAM);disc(x+19*j,y+4,5,0xd9cfab);disc(x+19*j,y+4,3,0xf9edd5);
  rect(x+19*j-7,y+1,1,8,EX_INK);rect(x+19*j+7,y+1,1,8,EX_INK);
 }
 ex_loaf(x,y-7);
}
static int ex_supper_step,ex_supper_done;
static void ex_take_note(void){
 ex_supper_step=1;ex_clear_goal();
 vd_status("Walk one block west past Mariner Aquarium. Look for a braided loaf on the sidewalk.");
 tell("From Low Tide: take Ada's supper note to the baker with the braided loaf.");
}
static void ex_bake_sign(int b,const char *s){
 int x=sx(ordinaryDoorX[b]),y=sy(ordinaryDoorY[b])+4;
 if(x<0||x>=W||y<0||y>=H)return;
 box(x+9,y-16,12,17,0x354a49,0xc49868);line(x+20,y-15,x+25,y+3,0xae8054);
 for(int i=0;i<3;i++)rect(x+12,y-11+i*3,6-i,1,EX_CREAM);
 if(d2(p.x-ordinaryDoorX[b],p.y-ordinaryDoorY[b])<162.f*162.f)ex_tag(x,y-38,s,GOLD);
}
static int vd_a4(int ev,U keys,U tap){
 (void)keys;
 if(ev==VE_RESET){ex_supper_step=ex_supper_done=0;return 0;}
 if(ev==VE_CANCEL){ex_supper_step=0;return 0;}
 if(ev==VE_START){
  ex_supper_step=0;
  if(ex_fixture_near(EX_HOME,2))ex_take_note();
  else{ex_venue_goal(EX_HOME);vd_status("Read Ada's envelope on the residents' table at Low Tide Rooming House.");}
 }
 if(ev==VE_AMBIENT&&ex_can_offer()&&ex_fixture_near(EX_HOME,2)&&(tap&ENTER)){vd_launch(VA_A4);return 1;}
 if(ev==VE_TICK&&(tap&ENTER)){
  if(ex_supper_step==0&&ex_fixture_near(EX_HOME,2)){ex_take_note();return 1;}
  if(ex_supper_step==1&&ex_fixture_near(EX_OVEN,0)){
   ex_supper_step=2;vd_status("Bring the warm loaf one block east to Ada's table at Low Tide Rooming House.");
   tell("The baker folds Ada's note into his apron. 'You're slicing it. I'll bring the butter.'");return 1;
  }
  if(ex_supper_step==2&&ex_fixture_near(EX_HOME,2)){
   ex_supper_done=1;ex_supper_step=0;vd_end();
   tell("Ada breaks the heel in two. 'We always set one extra.' Your chair stays by the table.");return 1;
  }
 }
 if(ev==VE_WORLD&&ex_street()&&ex_supper_step==1){
  ex_bake_sign(35,"BRICK OVEN / ROUND RYE");ex_bake_sign(73,"PASTRY WINDOW / CREAM SHELLS");
  ex_bake_sign(EX_OVEN,"TODAY: THE BRAIDED LOAF");
 }
 if(ev==VE_DECOR&&ex_room(EX_HOME)){
  int here=ex_fixture_near(EX_HOME,2),running=vdActive==VA_A4;
  if(ex_supper_done||(running&&ex_supper_step==2))ex_set_table(2);else ex_leaflet(2,EX_UMBER);
  if(running&&ex_supper_step==2)ex_fix_tag(2,here?"[ENTER] TAKE YOUR SEAT":"A PLACE SET FOR YOU",GOLD);
  else if(running&&ex_supper_step==1)ex_fix_tag(2,"ADA'S TABLE",GOLD);
  else if(ex_can_offer()||running)ex_fix_tag(2,here?(ex_supper_done?"[ENTER] ANOTHER SUNDAY":"[ENTER] READ ADA'S NOTE"):
         ex_supper_done?"SUNDAY / TABLE FOR TWO":"SUNDAY / ADA'S NOTE",GOLD);
 }
 if(ev==VE_DRAW&&ex_room(EX_OVEN)&&ex_supper_step==1){
  ex_fix_tag(0,ex_fixture_near(EX_OVEN,0)?"[ENTER] DELIVER ADA'S NOTE":"MORNING BAKER / BRAIDED LOAF",GOLD);
 }
 return 0;
}

/* A newspaper still needs an illuminated tank and the right camera position
   and direction. The step/facing tolerance includes diagonals toward the tank. */
static int ex_photo_step,ex_photo_done,ex_photo_result;
static void ex_corners(int x,int y,int w,int h,int len,U col){
 for(int j=0;j<2;j++)for(int i=0;i<2;i++){
  int xx=x+i*w,yy=y+j*h;line(xx,yy,xx+(i?-len:len),yy,col);line(xx,yy,xx,yy+(j?-len:len),col);
 }
}
static int ex_mark_x(void){return wi_f[0].x+wi_f[0].w/2;}
static int ex_mark_y(void){return wi_f[0].y+wi_f[0].h+73;}
static int ex_on_mark(void){return ex_room(EX_KELP)&&absf(wi_x-ex_mark_x())<=22&&absf(wi_y-ex_mark_y())<=16;}
static int ex_facing(void){return sn(wi_angle)<-.61f;}
static void ex_photo_page(int x,int y){
 box(x+3,y+4,252,153,0x152329,EX_INK);box(x,y,252,153,EX_CREAM,EX_UMBER);
 textat(x+10,y+5,"THE VANTA LANTERN",EX_INK,0);textat(x+203,y+5,"P.03",EX_UMBER,0);
 rect(x+10,y+26,232,2,EX_INK);box(x+10,y+33,232,78,0x214b60,EX_INK);
 for(int i=0;i<12;i++){
  int xx=x+18+i*19,ht=20+(i*13)%50;
  for(int t=0;t<ht;t++){int k=(int)(5*sn(t*.12f+i));rect(xx+k,y+107-t,3,2,i%2?0x529d83:0x69b3a6);
   if(t%10==2)line(xx+k,y+106-t,xx+k+(t%20==2?6:-6),y+101-t,0x83c1a0);}
 }
 for(int i=0;i<6;i++){int xx=x+139+i*13,yy=y+51+(i%3)*5;disc(xx,yy,2,EX_CREAM);tri(xx-1,yy,xx-6,yy-2,xx-6,yy+2,EX_CREAM);}
 textat(x+11,y+111,"THE KELP KEEPS TIME",EX_INK,0);textat(x+11,y+131,"Harbourlight night tide",EX_UMBER,0);
}
static void ex_take_camera(void){
 ex_photo_step=1;ex_clear_goal();
 vd_status("Find the Glass Mile aquarium's kelp, not jellies. Set the tide lamp, then shoot from the rail.");
 tell("Picture desk: 'File the night tank. The fronds should rise behind the little fish.'");
}
static int vd_b3a(int ev,U keys,U tap){
 (void)keys;
 if(ev==VE_RESET){ex_photo_step=ex_photo_done=ex_photo_result=0;return 0;}
 if(ev==VE_CANCEL){ex_photo_step=0;return 0;}
 if(ev==VE_START){
  ex_photo_step=ex_photo_result=0;
  if(ex_fixture_near(EX_PAPER,3))ex_take_camera();
  else{ex_venue_goal(EX_PAPER);vd_status("Collect the picture brief from the newspaper rack inside Bluebird Reading Room.");}
 }
 if(ev==VE_AMBIENT){
  if(ex_photo_result>0)ex_photo_result--;
  if(ex_can_offer()&&ex_fixture_near(EX_PAPER,3)&&(tap&ENTER)){vd_launch(VA_B3A);return 1;}
 }
 if(ev==VE_TICK&&(tap&ENTER)){
  if(!ex_photo_step&&ex_fixture_near(EX_PAPER,3)){ex_take_camera();return 1;}
  if(ex_photo_step&&ex_on_mark()){
   if(!wi_sw[EX_KELP])tell("Too dark for the plate. Use E at the large tide tank to switch on its night lamp.");
   else if(!ex_facing())tell("The lens sees the aisle. Turn up toward the big tank, then press Enter.");
   else{ex_photo_step=0;ex_photo_done=1;ex_photo_result=420;vd_end();tell("Extra! 'The Kelp Keeps Time.' Your still is on the rack at Bluebird Reading Room.");}
   return 1;
  }
 }
 if(ev==VE_DECOR&&ex_room(EX_PAPER)){
  ex_leaflet(3,EX_LEAF);
  if(ex_can_offer()||vdActive==VA_B3A)
   ex_fix_tag(3,ex_fixture_near(EX_PAPER,3)?"[ENTER] PICTURE BRIEF":"LANTERN / PICTURE DESK",TEAL);
  if(ex_photo_done&&ex_fixture_near(EX_PAPER,3))ex_photo_page(666,116);
 }
 if(ev==VE_DECOR&&ex_room(EX_KELP)&&ex_photo_result>0)ex_photo_page(666,116);
 if(ev==VE_DRAW&&ex_photo_step&&ex_room(EX_KELP)){
  int x=ex_mark_x(),y=ex_mark_y();U ink=ex_on_mark()?GOLD:PAPER;
  ex_corners(x-24,y-17,48,34,9,ink);line(x,y-24,x,y-34,ink);line(x,y-34,x-4,y-29,ink);line(x,y-34,x+4,y-29,ink);
  ex_fix_tag(0,wi_sw[EX_KELP]?"NIGHT TIDE / LAMP ON":"DAY TIDE / [E] LAMP AT TANK",TEAL);
  ex_tag(x,y+21,!ex_on_mark()?"PRESS RAIL / FACE UP":!wi_sw[EX_KELP]?"LIGHT THE NIGHT TANK":!ex_facing()?"TURN UP TO FRAME TANK":"[ENTER] TAKE STILL",ink);
  if(ex_on_mark()&&ex_facing())ex_corners(wi_f[0].x-6,wi_f[0].y-5,wi_f[0].w+12,wi_f[0].h+10,12,GOLD);
 }
 return 0;
}

/* Compare a linocut against the theatre's address plaque. Either candidate's
   plaque is useful evidence; there is no hidden compulsory tour of both. */
static int ex_bill_step,ex_bill_clues,ex_bill_done,ex_bill_select;
static void ex_bill_card(int corrected){
 int x=675,y=118;
 box(x+3,y+3,247,170,0x12202c,EX_INK);box(x,y,247,170,EX_CREAM,EX_UMBER);
 textat(x+10,y+5,corrected?"SIGNED / NIGHT EDITION":"PROOF 07 / WRONG SLUG",EX_INK,0);
 rect(x+10,y+28,227,77,0x253a46);
 for(int j=0;j<4;j++){tri(x+11+j*12,y+29,x+24+j*9,y+29,x+12,y+70,0x9a5253);
 tri(x+235-j*12,y+29,x+222-j*9,y+29,x+235,y+70,0x9a5253);}
 for(int j=0;j<6;j++){
  int xx=x+42+j*32;tri(xx,y+51,xx-7,y+72,xx+8,y+72,EX_CREAM);
  disc(xx,y+46,3,EX_CREAM);line(xx-4,y+72,xx-6,y+84,EX_CREAM);line(xx+4,y+72,xx+8,y+79,EX_CREAM);
  box(xx-5,y+90,11,7,0xe2b16f,0xeac88e);
 }
 rect(x+10,y+101,227,2,EX_UMBER);textat(x+10,y+105,"Six lamps in one row.",EX_INK,0);
 textat(x+10,y+125,corrected?"COPPER: FOOTLIGHT REVUE":"Slug: IN THE ROUND",corrected?EX_LEAF:EX_UMBER,0);
 if(!corrected)line(x+10,y+136,x+189,y+136,0xa74343);
 textat(x+10,y+146,corrected?"Plaque verified.":"Which stage fits?",EX_INK,0);
}
static void ex_take_proof(void){
 ex_bill_step=1;ex_bill_clues=ex_bill_select=0;ex_clear_goal();
 vd_status("Six footlights in a row. Check a playhouse plaque: Copper (Tide Market) or West Wind (Breakwater).");
 tell("The cut shows a chorus line, but the slug says 'In the Round.' Check a stage door.");
}
static int vd_b3b(int ev,U keys,U tap){
 (void)keys;
 if(ev==VE_RESET){ex_bill_step=ex_bill_clues=ex_bill_done=ex_bill_select=0;return 0;}
 if(ev==VE_CANCEL){ex_bill_step=ex_bill_clues=ex_bill_select=0;return 0;}
 if(ev==VE_START){
  ex_bill_step=ex_bill_clues=ex_bill_select=0;
  if(ex_fixture_near(EX_PROOF,3))ex_take_proof();
  else{ex_venue_goal(EX_PROOF);vd_status("Inspect the flawed theatre proof on Mariner Print Shop's display wall.");}
 }
 if(ev==VE_AMBIENT&&ex_can_offer()&&ex_fixture_near(EX_PROOF,3)&&(tap&ENTER)){vd_launch(VA_B3B);return 1;}
 if(ev==VE_TICK){
  if(ex_bill_select&&vdChoice){
   ex_bill_select=0;
   if(vdChoice==1){ex_bill_done=1;ex_bill_step=0;vd_end();tell("Corrected: Copper / Footlight Revue. The printer pins your bill over the bad proof.");}
   else tell("The proofreader circles the straight row: 'Where's the audience circle?' Try another slug.");
   return 1;
  }
  if(!ex_bill_step&&ex_fixture_near(EX_PROOF,3)&&(tap&ENTER)){ex_take_proof();return 1;}
  if(ex_bill_step&&(ex_room(EX_FOOT)||ex_room(EX_CIRCLE))&&wi_at_plaque()&&(tap&USE)){
   ex_bill_clues|=walkin_active==EX_FOOT?1:2;
   vd_status("Evidence in hand. Return to Mariner Print Shop's display wall and choose the corrected slug.");
   ex_venue_goal(EX_PROOF);
   return 0; /* Let ordinary E show the actual plaque's text. */
  }
  if(ex_bill_step&&ex_fixture_near(EX_PROOF,3)&&(tap&ENTER)){
   if(!ex_bill_clues)tell("The editor needs a source. Read the brass plaque just beside either playhouse exit with E.");
   else{ex_bill_select=1;vd_choose("WHICH SLUG FITS THE CHORUS LINE?","Copper Playhouse / Footlight Revue","West Wind Playhouse / In the Round");}
   return 1;
  }
 }
 if(ev==VE_DECOR&&ex_room(EX_PROOF)){
  int nearwall=ex_fixture_near(EX_PROOF,3);
  ex_leaflet(3,0xaa5960);
  if(ex_can_offer()||vdActive==VA_B3B)ex_fix_tag(3,
   nearwall?(ex_bill_step&&ex_bill_clues?"[ENTER] SET THE SLUG":"[ENTER] READ THE PROOF"):"PROOF 07 / DISPLAY WALL",PINK);
  if(nearwall)ex_bill_card(ex_bill_done&&vdActive!=VA_B3B);
 }
 if(ev==VE_DRAW&&ex_bill_step&&(ex_room(EX_FOOT)||ex_room(EX_CIRCLE)))
  ex_tag(wi_plaque_x(),383,wi_at_plaque()?"[E] READ THE THEATRE PLAQUE":"STAGE DOOR / BRASS PLAQUE",PINK);
 return 0;
}

/* The cabinet radiates a position-dependent signal. Each survey plate occupies
   a different block; students can visit in any order or resample. No GPS owns
   either the survey or final search. The meter includes literal numbers,
   cardinal directions and a visual notch scale; sound is never consulted. */
enum{EX_SOURCE_X=1245,EX_SOURCE_Y=908};
static const int ex_posts[]={EX_BLOCK0,EX_BLOCK1,EX_BLOCK2};
static const char *const ex_post_names[]={"BLUE HOUR ROOMING","COPPER PLAYHOUSE","NINTH ST. GALLERY"};
static int ex_dial_step,ex_dial_mask,ex_dial_done,ex_dial_last=-1;
static int ex_post_x(int i){return ordinaryDoorX[ex_posts[i]];}
static int ex_post_y(int i){return ordinaryDoorY[ex_posts[i]]+13;}
static int ex_bars(int x,int y){
 int r=(int)sq(d2(x-EX_SOURCE_X,y-EX_SOURCE_Y));return cl(100-r/7,0,100);
}
static const char *ex_bearing(int x,int y){
 int dx=EX_SOURCE_X-x,dy=EX_SOURCE_Y-y,ax=(int)absf(dx),ay=(int)absf(dy);
 if(ax<18&&ay<18)return "AT SOURCE";
 if(ax>ay*2)return dx>0?"EAST":"WEST";
 if(ay>ax*2)return dy>0?"SOUTH":"NORTH";
 return dx>0?(dy>0?"SOUTHEAST":"NORTHEAST"):(dy>0?"SOUTHWEST":"NORTHWEST");
}
static int ex_post_near(void){
 if(!ex_street()||p.car>=0)return -1;
 for(int i=0;i<3;i++)if(d2(p.x-ex_post_x(i),p.y-ex_post_y(i))<52*52)return i;
 return -1;
}
static int ex_source_near(void){return ex_street()&&p.car<0&&d2(p.x-EX_SOURCE_X,p.y-EX_SOURCE_Y)<37*37;}
static void ex_deploy_meter(void){
 ex_dial_step=1;ex_dial_last=-1;ex_dial_mask=0;ex_clear_goal();
 vd_status("Log the sidewalk plates: Blue Hour Rooming House, Copper Playhouse and Ninth St. Gallery.");
 tell("Low Tide's night desk: 'Somebody left a weather loop running. Take the field receiver.'");
}
static void ex_dial_panel(void){
 int x=16,y=213,w=285,probe=ex_post_near();U ink=0xb8d4a3;
 box(x,y,w,140,0x16252b,0x9faba0);rect(x+2,y+2,w-4,2,ink);
 textat(x+10,y+5,"LOW TIDE / POCKET RECEIVER",ink,0);
 for(int i=0;i<3;i++){
  int yy=y+29+i*20;char n[22];textat(x+10,yy,ex_post_names[i],MUTED,0);
  if(ex_dial_mask&(1<<i)){char*q=number(n,ex_bars(ex_post_x(i),ex_post_y(i)));append(q," / 100");}
  else append(n,"--");textat(x+w-textw(n,0)-12,yy,n,ink,0);
 }
 rect(x+10,y+93,w-20,1,0x64756b);int strength=ex_bars((int)p.x,(int)p.y);
 for(int j=0;j<10;j++)box(x+11+j*10,y+103,8,11,strength>j*10?ink:0x293b3b,0x557064);
 char t[55];char*q=number(append(t,"LIVE "),strength);append(q," / 100");textat(x+126,y+98,t,ink,0);
 const char *hint;char bearing[36];
 if(ex_dial_mask==7){append(append(bearing,"BEARING: "),ex_bearing((int)p.x,(int)p.y));hint=ex_source_near()?"[ENTER] TAG THE TRANSMITTER":bearing;}
 else if(probe>=0)hint=(ex_dial_mask&(1<<probe))?"[ENTER] RESAMPLE THIS PLATE":"[ENTER] SAMPLE THIS PLATE";
 else if(ex_dial_last>=0){append(append(bearing,"LAST: "),ex_bearing(ex_post_x(ex_dial_last),ex_post_y(ex_dial_last)));hint=bearing;}
 else hint="STAND BY A SURVEY PLATE";
 textat(x+10,y+118,hint,GOLD,0);
}
static void ex_cabinet(void){
 int x=sx(EX_SOURCE_X),y=sy(EX_SOURCE_Y);
 if(x<-20||x>=W+20||y<0||y>=H)return;
 box(x-6,y-15,14,15,0x455752,0xa9b4a6);line(x+5,y-15,x+8,y-24,0xdbe4cc);
 for(int i=0;i<3;i++)rect(x-3,y-11+i*2,7,1,0x9baf9f);
 if(ex_source_near())ex_tag(x,y-48,ex_dial_done&&!ex_dial_step?"RADIO / WEATHER LOOP":"[ENTER] TAG THE TRANSMITTER",GOLD);
}
static int vd_b3c(int ev,U keys,U tap){
 (void)keys;
 if(ev==VE_RESET){ex_dial_step=ex_dial_mask=ex_dial_done=0;ex_dial_last=-1;return 0;}
 if(ev==VE_CANCEL){ex_dial_step=ex_dial_mask=0;ex_dial_last=-1;return 0;}
 if(ev==VE_START){
  ex_dial_step=ex_dial_mask=0;ex_dial_last=-1;
  if(ex_fixture_near(EX_DESK,6))ex_deploy_meter();
  else{ex_venue_goal(EX_DESK);vd_status("Collect a field receiver from the reel shelf at Low Tide Radio.");}
 }
 if(ev==VE_AMBIENT&&ex_can_offer()&&ex_fixture_near(EX_DESK,6)&&(tap&ENTER)){vd_launch(VA_B3C);return 1;}
 if(ev==VE_TICK&&(tap&ENTER)){
  if(!ex_dial_step&&ex_fixture_near(EX_DESK,6)){ex_deploy_meter();return 1;}
  if(ex_dial_step&&ex_dial_mask==7&&ex_source_near()){
   ex_dial_done=1;ex_dial_step=0;vd_end();
   tell("Yesterday's forecast for an empty block. Low Tide logs the cabinet: 'At last, clear.'");return 1;
  }
  if(ex_dial_step){
   int i=ex_post_near();
   if(i>=0){
    ex_dial_last=i;ex_dial_mask|=1<<i;char m[96];char*q=append(m,ex_post_names[i]);q=append(q,": strength ");
    q=number(q,ex_bars(ex_post_x(i),ex_post_y(i)));q=append(q,"/100; source ");q=append(q,ex_bearing(ex_post_x(i),ex_post_y(i)));append(q,".");
    tell(m);if(ex_dial_mask==7)vd_status("Strongest near Ninth St. Gallery. Follow the live bearing and rising number to a service cabinet.");
    return 1;
   }
  }
 }
 if(ev==VE_DECOR&&ex_room(EX_DESK)){
  InteriorProp *f=wi_f+6;int x=f->x+f->w/2,y=f->y+f->h/2;
  box(x-11,y-8,22,14,0x263c42,0xb3bdaf);rect(x-8,y-5,12,5,0xa3b89b);disc(x+6,y+1,2,GOLD);line(x+9,y-8,x+13,y-17,PAPER);
  if(ex_can_offer()||vdActive==VA_B3C)ex_fix_tag(6,ex_fixture_near(EX_DESK,6)?"[ENTER] FIELD RECEIVER":"SIGNAL DESK / RECEIVER",TEAL);
 }
 if(ev==VE_WORLD&&ex_street()&&ex_dial_step){
  for(int i=0;i<3;i++){
   int x=sx(ex_post_x(i)),y=sy(ex_post_y(i));if(x<-30||x>W+30||y<-20||y>H+20)continue;
   box(x-9,y-4,18,8,0x5a5e5b,0xd6c5a4);rect(x-5,y-1,10,2,0xe1d4ac);
   if(d2(p.x-ex_post_x(i),p.y-ex_post_y(i))<138*138)
    ex_tag(x,y-36,(ex_dial_mask&(1<<i))?"LOGGED SURVEY PLATE":"FIELD SURVEY PLATE",GOLD);
  }
  if(ex_dial_mask==7&&d2(p.x-EX_SOURCE_X,p.y-EX_SOURCE_Y)<145*145)ex_cabinet();
 }
 if(ev==VE_DRAW&&ex_street()&&ex_dial_step)ex_dial_panel();
 if(ev==VE_DECOR&&ex_street()&&ex_dial_done&&!ex_dial_step&&d2(p.x-EX_SOURCE_X,p.y-EX_SOURCE_Y)<85*85)ex_cabinet();
 return 0;
}
