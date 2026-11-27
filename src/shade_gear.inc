/* SHADE SUPPLY. Separate fictional equipment, shown in the walk-in shop.
 * Only the arcade district-Witness ping and Pulse presentation are hooked;
 * police AI, pursuit Heat and combat behavior are intentionally untouched. */
enum {SG_MASK,SG_HUSH,SG_LAMP,SG_COUNT,SG_COOL_SHIFT=6,SG_COOL_TICKS=3600,SG_MAX_PING=9};
enum {SG_FLAGS=63,SG_COOL_MASK=4095,SG_USED_MASK=(1u<<18)-1};
static const char *const sg_name[]={"Ski mask","Pulse Hush Module","Trail Lamp"};
static const char *const sg_caption[]={"WEARABLE","PULSE COSMETIC","NONWEAPON UTILITY"};
static const int sg_price[]={90,120,38};
static const U sg_ink[]={0xe0bc83,0xa6cff4,0xa6d89d};
static const char *const sg_detail[]={
 "A wearable face cover. It buffers one eligible small opening Witness ping on the district meter, then recharges for 60 active seconds. It never clears or reduces an active pursuit.",
 "An invented Pulse visual-noise filter. Afterglow fades faster; targeting, fire timing, ammunition, charge and police Heat are unchanged. Not a real attachment.",
 "A pocket path light. Shows a small navigation ring in the overhead outdoor view on foot. It does not change movement, detection or any encounter."
};
static int sg_row,sg_arm;
static char sg_note[128];
static float sg_notice_time;
static int sg_bits(void){return (nv.world[OC_GEAR_WORD]>>OC_GEAR_SHIFT)&SG_USED_MASK;}
static void sg_write(U v){
 U mask=(U)SG_USED_MASK<<OC_GEAR_SHIFT;nv.world[OC_GEAR_WORD]=(nv.world[OC_GEAR_WORD]&~mask)|((v&SG_USED_MASK)<<OC_GEAR_SHIFT);
}
static int sg_owns(int item){return !!(sg_bits()&(1u<<(2*item)));}
static int sg_on(int item){int both=3<<(2*item);return (sg_bits()&both)==both;}
static int night_gear_mask(void){return sg_on(SG_MASK);}
static int night_gear_hush(void){return sg_on(SG_HUSH);}
static int night_gear_lamp(void){return sg_on(SG_LAMP);}
static int sg_left(void){return min(SG_COOL_TICKS,(sg_bits()>>SG_COOL_SHIFT)&SG_COOL_MASK);}
static void sg_cool(int t){sg_write((sg_bits()&SG_FLAGS)|((U)cl(t,0,SG_COOL_TICKS)<<SG_COOL_SHIFT));}
static void night_gear_reset(void){sg_row=sg_arm=0;sg_note[0]=0;sg_notice_time=0.f;}
/* Call only on the intended fictional low-level Signal ping, not generic
 * 'heat', shots, reporting, crimes, calls or any high-priority gameplay. */
static int night_gear_witness_delta(int x,int y,int n){
 if(n<1||n>SG_MAX_PING||!night_gear_mask()||sg_left()||p.heat>0.f||dispatch_engaged()>0||
    night_phone_heat_at(x,y)>0)return n;
 sg_cool(SG_COOL_TICKS);sg_notice_time=2.8f;return 0;
}
static float night_gear_trail(float t){return night_gear_hush()?t*.46f:t;}
static void night_gear_world(float dt){
 if(dt<=0||scene==OPEN_CITY_GEAR_SCENE)return;
 int t=sg_left();if(t)sg_cool(t-cl((int)(dt*60.f+.5f),1,60));
 if(sg_notice_time>0)sg_notice_time-=dt;
}
static int sg_clear(void){
 int tx,ty;
 return scene==1&&!p.dead&&p.car<0&&!mapOn&&menu<0&&!vdPanel&&atmOpen<0&&!harbor_boat_active()&&!npc_gate_street_use()&&
  p.heat<=0.f&&!dispatch_engaged()&&job.kind<0&&vdActive<0&&!ncr_active()&&!ng_route_goal(&tx,&ty)&&
  d2(p.x-OC_GEAR_X,p.y-OC_GEAR_Y)<27.f*27.f;
}
static int night_gear_enter(U tap){
 if(!(tap&ENTER)||!sg_clear())return 0;
 scene=OPEN_CITY_GEAR_SCENE;walking=0;sg_row=sg_arm=0;sg_note[0]=0;return 1;
}
static void sg_msg(const char*s){int i=0;while(i<(int)sizeof sg_note-1&&s[i]){sg_note[i]=s[i];i++;}sg_note[i]=0;}
static void sg_checkout(void){
 int n=sg_row;U bits=(U)sg_bits(),equip=2u<<(2*n);
 if(!sg_owns(n)){
  if(!sg_arm){sg_arm=1;sg_msg("Press E or Enter again to pay in clean cash. Any arrow backs out.");return;}
  sg_arm=0;
  if(sv.cash<sg_price[n]){sg_msg("Not enough clean cash. No purchase made.");return;}
  sv.cash-=sg_price[n];sg_write(bits|(3u<<(2*n)));
  request_save();
  sg_msg(saveFlash<0?"Purchased in this session; save failed. F5 retries.":"Paid and fitted. It stays in your gear inventory when switched off.");
  return;
 }
 sg_arm=0;sg_write(bits^equip);
 request_save();
 sg_msg(saveFlash<0?"Equipment changed in this session; save failed. F5 retries.":sg_on(n)?"Fitted.":"Removed. You still own this gear.");
}
static void night_gear_tick(U keys,U tap){
 (void)keys;
 if(tap&ESC){scene=1;sg_arm=0;return;}
 if(tap&(UP|DOWN|LEFT|RIGHT)){sg_row=(sg_row+SG_COUNT+((tap&(UP|LEFT))?-1:1))%SG_COUNT;sg_arm=0;sg_note[0]=0;return;}
 if(tap&(USE|ENTER))sg_checkout();
}
static void night_gear_pin(void){
 nav_slot();navHasManual=1;navManualLoc=NAV_GEAR;navManual=(NavPoint){OC_GEAR_X,OC_GEAR_Y};navGoalKind=0;
}
static void night_gear_sign_world(void){
 int x=sx(OC_GEAR_X),y=sy(OC_GEAR_Y);if(x<-100||x>W+90||y<-40||y>H+66)return;
 /* West side door; the established Cinema primary door stays intact. */
 box(x-8,y-24,16,14,0x152632,0x90b6b4);rect(x+3,y-21,2,5,0x84d5bb);
 rect(x-9,y-9,18,2,0xa99d79);rect(x-6,y-7,11,2,0x696b5c);
 box(x-60,y-50,105,21,0x172832,0x88c9bd);
 center(x-7,y-50,"SHADE SUPPLY",0xaeebd2,0);
 line(x+34,y-29,x+34,y-25,0x86d4bf);line(x+34,y-25,x+8,y-25,0x86d4bf);
}
static void night_gear_sign_chase(void){
 if(d2(p.x-OC_GEAR_X,p.y-OC_GEAR_Y)<465.f*465.f)ch_record_label(OC_GEAR_X,OC_GEAR_Y-10,"SHADE SUPPLY",0xaeebd2);
}
static void night_gear_lamp_world(void){
 if(!night_gear_lamp()||scene!=1||p.car>=0||p.dead||harbor_boat_active())return;
 int x=sx(p.x),y=sy(p.y);
 for(int i=0;i<8;i++){
  float a=(i+.5f)*.785398f;
  int xx=x+(int)(cs(a)*19),yy=y+(int)(sn(a)*19);
  dot(xx,yy,0xb4d8a0);dot(xx+1,yy,0x83b289);
 }
}
static void night_gear_hint(void){
 if(sg_clear()){box(361,483,238,26,0x13242e,0xa5d8ad);center(480,485,"ENTER / SHADE SUPPLY",0xc5e5ae,0);}
 if(scene!=1||menu>=0||p.dead||mapOn||p.car>=0||harbor_boat_active())return;
 if(sg_notice_time>0){
  box(344,423,272,22,0x233634,0x8ccd92);center(480,423,"WITNESS PING BUFFERED",0xcbebb0,0);
 }
 if(night_gear_mask()||night_gear_hush()||night_gear_lamp()){
  char msg[85];char*q=append(msg,night_gear_mask()?"MASK  ":"");
  if(night_gear_mask()){q=append(q,"WITNESS ");if(sg_left())q=append(number(q,(sg_left()+59)/60),"s");else q=append(q,"READY");}
  if(night_gear_hush())q=append(q,night_gear_mask()?"  /  HUSH":"HUSH");
  if(night_gear_lamp())append(q,(night_gear_mask()||night_gear_hush())?"  /  LAMP":"LAMP");
  box(10,452,textw(msg,0)+19,21,0x192e33,0x547a77);textat(19,452,msg,0xb6d8ae,0);
 }
}
static void sg_icon(int which,int x,int y,int active){
 U c=sg_ink[which],d=active?0x487479:0x384e5b;
 if(which==SG_MASK){
  disc(x,y-2,21,0x111b29);disc(x,y-5,16,d);
  box(x-11,y-11,23,6,0x15242c,c);rect(x-6,y-10,3,2,0xdbd4ba);rect(x+4,y-10,3,2,0xdbd4ba);
  for(int p=-7;p<=7;p+=3)dot(x+p,y+4,0x6b858a);
 }else if(which==SG_HUSH){
  box(x-22,y-13,44,23,0x26394b,c);box(x-17,y-9,8,13,d,0x74a0b6);
  for(int z=0;z<7;z++)rect(x-5+z*3,y+2-(z*5)%12,2,2+(z*5)%12,c);
  rect(x-12,y+13,24,2,0x6c8a99);
 }else{
  for(int i=10;i>0;i-=3)disc(x,y-4,10+i,mix(0x263a3a,c,(11-i)/2));
  box(x-6,y-10,13,21,0x344d46,c);rect(x-3,y-14,7,9,0xe7ffd4);
  rect(x-3,y+2,7,3,d);
 }
}
static void night_gear_paint(void){
 const U bg=0x10222e,clerk=0x91c1ac;
 rect(0,0,W,H,0x101925);for(int y=116;y<H;y+=22)line(16,y,W-17,y,0x1d3039);
 box(24,23,912,489,bg,0x668487);rect(26,24,908,2,0x96cdb5);
 textat(47,38,"GLASS MILE / SALTMARSH WEST PORCH",0x9ebcb7,0);
 textat(45,62,"Shade Supply",PAPER,1);
 char money[74];number(append(money,"CLEAN CASH  $"),sv.cash);
 textat(912-textw(money,0),78,money,0xa4d8a5,0);
 box(49,125,215,282,0x213744,0x4c6f73);
 for(int i=0;i<5;i++)rect(57,153+i*38,197,2,0x3b5b65);
 box(77,294,159,64,0x172d37,0x738e8e);rect(82,299,149,4,0x586e69);
 person(151,284,clerk,0,1.57f,1,2);
 textat(67,365,"JULES / COUNTER",clerk,0);
 textat(67,384,"FICTIONAL GEAR",0x8ca4a6,0);
 for(int i=0;i<SG_COUNT;i++){
  int x=280+i*208,selected=i==sg_row;U c=sg_ink[i];char status[40];
  box(x,125,200,202,selected?0x293e48:0x1d323d,selected?c:0x476975);
  if(selected)rect(x+1,126,198,3,c);
  textat(x+13,138,sg_caption[i],c,0);sg_icon(i,x+100,193,sg_on(i));
  textat(x+13,222,sg_name[i],PAPER,0);
  if(sg_owns(i))append(status,sg_on(i)?"FITTED / OWNED":"OFF / OWNED");
  else number(append(status,selected&&sg_arm?"CONFIRM $":"$"),sg_price[i]);
  textat(x+13,251,status,c,0);
  if(i==SG_MASK&&sg_owns(i)){
   if(sg_left())append(number(append(status,"CHARGING "),((sg_left()+59)/60)),"s");else append(status,"WITNESS READY");
   textat(x+13,281,status,0xabc3a9,0);
  }
  if(i==SG_HUSH&&sg_owns(i))textat(x+13,281,sg_on(i)?"TRACE NOISE 46%":"TRACE NOISE 100%",0xa9becc,0);
  if(i==SG_LAMP&&sg_owns(i))textat(x+13,281,sg_on(i)?"PATH RING ON":"PATH RING OFF",0xa9c6aa,0);
 }
 box(280,340,616,98,0x1b3039,0x4b6a6f);
 const char*s=sg_note[0]?sg_note:sg_detail[sg_row];
 for(int i=0;*s&&i<4;i++){char linebuf[130];s=hud_line(s,570,linebuf);textat(293,345+i*18,linebuf,sg_note[0]?sg_ink[sg_row]:PAPER,0);}
 textat(51,467,"ARROWS / BROWSE     E OR ENTER / BUY OR TOGGLE     ESC / STREET     F5 / SAVE",0xa8bbb3,0);
}
