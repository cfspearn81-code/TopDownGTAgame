/* TRAP HOUSE / Bell Lane Annex. Persisted in formerly zero Rev11 words 58-62.
 * Cabinet stock preserves the Signal strain and quality. The dispatch desk is
 * a separate fictional neighborhood electronics-refurbishment service. */
enum {NT_DEED=58,NT_SHELF0,NT_SHELF1,NT_CLOCK,NT_LEDGER,NT_RESERVED};
enum {NT_BUY=480,NT_RACK=0,NT_SHUTTER,NT_RELAY,NT_BUILD,NT_STOCK,NT_DISPATCH,NT_VISIT};
enum {NT_STOCK_DOOR=0,NT_DESK_DOOR,NT_RELAY_DOOR,NT_EXIT_DOOR};
static const U nt_amber=0xffc278,nt_mint=0x89e2bc,nt_low=0x617981,nt_panel=0x101e28;
static const char *const nt_upgrade[]={"Steel racks","Storm shutters","Dispatch relay"};
static const int nt_price[3][2]={{90,160},{110,195},{140,220}};
static const char *const nt_station[]={"LOCKED STOCKROOM","BUILD & DEED","DISPATCH DESK","BELL LANE EXIT"};
static const int nt_sz[]={213,480,739,480},nt_sy[]={263,254,257,438};
static float nt_x,nt_y,nt_angle;
static int nt_walking,nt_pane,nt_sel,nt_confirm;
static char nt_msg[120];
static int night_trap_owned(void){return !!(nv.phone[NT_DEED]&1u);}
static int nt_level(int id){return min(2,(nv.phone[NT_DEED]>>(1+2*id))&3u);}
static int nt_capacity(void){return 9*(1+nt_level(NT_RACK));}
static int nt_count(int id){int slot=id<6?NT_SHELF0:NT_SHELF1,shift=(id%6)*5;return (nv.phone[slot]>>shift)&31u;}
static void nt_write(int id,int count){int slot=id<6?NT_SHELF0:NT_SHELF1,shift=(id%6)*5;
 nv.phone[slot]=(nv.phone[slot]&~(31u<<shift))|((U)cl(count,0,31)<<shift);}
static int nt_stock_total(void){int n=0;for(int i=0;i<9;i++)n+=nt_count(i);return n;}
static int nt_clock(void){return (int)min(nv.phone[NT_CLOCK]>7200?7200:(int)nv.phone[NT_CLOCK],7200);}
static int nt_ticket(void){return (int)(nv.phone[NT_LEDGER]&3u);}
static int nt_jobs(void){return min(99999,(int)(nv.phone[NT_LEDGER]>>2));}
static int nt_income(int ticket){return (ticket==2?82:49)+nt_level(NT_RELAY)*14;}
static int nt_fee(int ticket){return ticket==2?30:14;}
static int nt_delay(int ticket){return (ticket==2?44:26)-nt_level(NT_RELAY)*6;}
static int nt_overhead(void){int h=night_phone_heat_at(NT_DOOR_X,NT_DOOR_Y);return max(0,(h>=70?15:h>=40?6:0)-8*nt_level(NT_SHUTTER));}
static void nt_note(const char*s){char*q=nt_msg;int i=0;while(s[i]&&i<(int)sizeof(nt_msg)-1){q[i]=s[i];i++;}q[i]=0;}
static int nt_pay(int value){if(sv.cash<value){nt_note("Not enough clean cash on hand. Community dispatch is separate from the scrip wallet.");return 0;}sv.cash-=value;return 1;}
static void nt_save(const char*s){request_save();nt_note(saveFlash<0?"Save failed. The changes are still in this session; use F5 to retry.":s);}
static void nt_open(int pane){nt_pane=pane;nt_sel=nt_confirm=0;nt_msg[0]=0;}
static int nt_door_ready(void){
 int x,y;
 return scene==1&&p.car<0&&!p.dead&&menu<0&&!mapOn&&!vdPanel&&atmOpen<0&&
  job.kind<0&&vdActive<0&&!ng_route_goal(&x,&y)&&d2(p.x-NT_DOOR_X,p.y-NT_DOOR_Y)<=31.f*31.f;
}
static int night_trap_entry(U tap){
 if(!(tap&ENTER)||!nt_door_ready())return 0;
 scene=NIGHT_TRAP_SCENE;nt_x=480;nt_y=412;nt_angle=-1.57f;nt_walking=0;
 nt_open(night_trap_owned()?-1:NT_VISIT);walking=0;return 1;
}
static int nt_nearest(void){int best=-1;float d=52.f*52.f;for(int i=0;i<4;i++){float q=d2(nt_sz[i]-nt_x,nt_sy[i]-nt_y);if(q<d){d=q;best=i;}}return best;}
static void nt_shift_stock(int id,int out){
 if(!night_trap_owned())return;
 int n=nt_count(id),sat=np_get(NP_BAG+id,99);
 if(out){if(!n){nt_note("That bin is empty.");return;}if(sat>=99){nt_note("No room in the matching satchel slot.");return;}
  nt_write(id,n-1);nv.phone[NP_BAG+id]=sat+1;nt_save("Moved one unit to the satchel. Strain and grade preserved.");return;}
 if(!sat){nt_note("No matching satchel stock.");return;}
 if(nt_stock_total()>=nt_capacity()||n>=31){nt_note("Rack full. Withdraw stock or build more steel racks.");return;}
 nt_write(id,n+1);nv.phone[NP_BAG+id]=sat-1;nt_save("Stored one unit without changing its strain or grade.");
}
static void nt_dispatch(int row){
 int booking=nt_ticket();
 if(row==2){
  if(booking<1||booking>2){nt_note("There is no dispatch booking.");return;}
  if(nt_clock()){nt_note("The electronics bench is still occupied. Come back after the countdown.");return;}
  int reward=nt_income(booking),accepted=min(reward,WALLET_LIMIT-sv.cash);
  if(!accepted){nt_note("Your cash wallet is full. Payout remains at the desk.");return;}
  /* Partial payout is deliberately deferred rather than burning the balance. */
  if(accepted<reward){nt_note("Clear cash wallet space for the full payout.");return;}
  sv.cash+=reward;nv.phone[NT_LEDGER]=(U)(min(99999,nt_jobs()+1)<<2);
  nt_save("Repair commission paid into clean cash.");return;
 }
 if(booking){nt_note("Claim the existing dispatch before booking another.");return;}
 if(row==1&&!nt_level(NT_RELAY)){nt_note("Build the first dispatch relay to unlock rush service.");return;}
 int ticket=row+1,due=nt_fee(ticket)+nt_overhead();
 if(!nt_confirm){nt_confirm=1;nt_note("Confirm the booking, or change rows to cancel.");return;}
 nt_confirm=0;if(!nt_pay(due))return;
 /* The relay level is selected at collection deliberately, so players never
  * lose a newly purchased upgrade's payout boost on an accepted ticket. */
 nv.phone[NT_LEDGER]=(U)(nt_jobs()<<2)|ticket;nv.phone[NT_CLOCK]=(U)(60*nt_delay(ticket));
 nt_save("Dispatch booked. Time advances while you explore, not in the phone or pause menus.");
}
static void nt_panel_use(void){
 if(nt_pane==NT_VISIT){
  if(!nt_confirm){nt_confirm=1;nt_note("Pay $480 clean cash? Enter or E again confirms. Escape cancels.");return;}
  nt_confirm=0;if(!nt_pay(NT_BUY))return;
  nv.phone[NT_DEED]|=1u;nt_open(NT_BUILD);nt_save("Keys issued. Walk to the racks, build bench or dispatch relay.");
 }else if(nt_pane==NT_BUILD){
  int L=nt_level(nt_sel);if(L>=2){nt_note("Fully built.");return;}
  if(!nt_confirm){nt_confirm=1;nt_note("Confirm construction with Enter or E. Changing rows cancels.");return;}
  nt_confirm=0;if(!nt_pay(nt_price[nt_sel][L]))return;
  int shift=1+nt_sel*2;nv.phone[NT_DEED]=(nv.phone[NT_DEED]&~(3u<<shift))|((U)(L+1)<<shift);
  nt_save(nt_sel==NT_RACK?"Steel racks installed. Storage capacity increased.":nt_sel==NT_SHUTTER?"Shutters installed. The dispatch attention surcharge dropped.":"Relay installed. Bookings shorten, payouts rise, and rush work unlocks.");
 }else if(nt_pane==NT_STOCK)nt_shift_stock(nt_sel,0);
 else if(nt_pane==NT_DISPATCH)nt_dispatch(nt_sel);
}
static void night_trap_tick(U keys,U tap){
 if(tap&ESC){if(nt_pane>=0){nt_open(-1);return;}scene=1;return;}
 if(nt_pane>=0){
  int n=nt_pane==NT_STOCK?9:nt_pane==NT_VISIT?1:3;
  if(tap&(UP|DOWN|LEFT|RIGHT)){nt_sel=(nt_sel+n+((tap&(UP|LEFT))?-1:1))%n;nt_confirm=0;nt_msg[0]=0;}
  else if((tap&FIRE)&&nt_pane==NT_STOCK)nt_shift_stock(nt_sel,1);
  else if(tap&(USE|ENTER))nt_panel_use();
  return;
 }
 int dx=!!(keys&RIGHT)-!!(keys&LEFT),dy=!!(keys&DOWN)-!!(keys&UP);float pace=keys&BRAKE?3.15f:2.05f;
 nt_walking=!!(dx||dy);if(nt_walking){nt_angle=look(dx,dy);if(dx&&dy)pace*=.7071f;
  float ax=nt_x+dx*pace,ay=nt_y+dy*pace;if(ax>113&&ax<846)nt_x=ax;if(ay>228&&ay<449)nt_y=ay;}
 if(tap&(USE|ENTER)){int at=nt_nearest();if(at==NT_EXIT_DOOR){scene=1;return;}
  if(!night_trap_owned()){if(at==NT_DESK_DOOR)nt_open(NT_VISIT);else if(at>=0)nt_note("Deed required. Find the center build desk.");return;}
  if(at>=0)nt_open(at==NT_STOCK_DOOR?NT_STOCK:at==NT_DESK_DOOR?NT_BUILD:NT_DISPATCH);}
}
static void night_trap_world_tick(float dt){
 int t=nt_clock();if(dt<=0||!t||!night_trap_owned()||nt_ticket()<1||nt_ticket()>2)return;
 nv.phone[NT_CLOCK]=(U)max(0,t-cl((int)(dt*60+.5f),1,60));
 if(!nv.phone[NT_CLOCK]){tell("Trap House / Electronics dispatch ready to collect.");request_save();}
}
static void night_trap_street_paint(void){
 int x=sx(NT_DOOR_X),y=sy(NT_DOOR_Y);if(x<-58||x>W+58||y<-26||y>H+34)return;
 U sig=night_trap_owned()?nt_mint:nt_amber;
 rect(x-10,y-30,20,16,0x080e17);box(x-8,y-30,15,17,0x1b2930,sig);rect(x+3,y-25,2,3,sig);
 line(x-9,y-11,x+10,y-11,0x6a7779);line(x-9,y-10,x+10,y-10,sig);
 for(int i=0;i<3;i++)rect(x-10-i,y-8+i*2,21+i*2,1,0x65716d);
 box(x-36,y-54,103,22,0x17202b,sig);center(x+15,y-54,"TRAP HOUSE",sig,0);
 line(x+60,y-32,x+60,y-27,sig);line(x+60,y-27,x+19,y-27,sig);line(x+19,y-27,x+24,y-30,sig);
 for(int i=0;i<3;i++){int xx=x-31+i*4;dot(xx,y-48,0x7d7169);}
}
static void night_trap_hint(void){
 if(!nt_door_ready())return;
 box(358,469,244,27,0x121e2a,nt_amber);center(480,472,"ENTER  /  TRAP HOUSE",nt_amber,0);
}
static void nt_label(int x,int y,const char*s,U tint){box(x+1,y+2,190,23,0x101a22,0x232b30);textat(x+9,y+3,s,tint,0);}
static void nt_shelf_art(int x,int y,int w,int k){
 box(x-3,y-3,w+6,42,0x243943,0x151e27);rect(x,y,w,24,0x131d27);
 for(int i=0;i<w;i+=19){int xx=x+i+2,c=8+(i*13+k)%12;U ink=i%3?0x756c54:0x546c78;
  rect(xx,y+24-c,10,c,ink);rect(xx+2,y+27-c,8,1,nt_amber);}
 rect(x,y+25,w,3,0x54727b);rect(x,y+32,w,3,0x334c53);
}
static void nt_show_floor(void){
 rect(0,0,W,H,0x0c1521);rect(61,100,838,372,0x182837);rect(74,125,811,314,0x26363c);
 for(int i=0;i<37;i++){int x=79+(i*113)%798;U ink=i%2?0x505455:0x445759;rect(x,219+(i*47)%214,7+i%4,1,ink);}
 for(int y=223;y<445;y+=18){line(74,y,885,y,0x384853);for(int x=75+(y%36?28:0);x<885;x+=58)line(x,y-17,x,y,0x43575a);}
 rect(73,132,813,83,0x3c5156);rect(83,197,794,27,0x27393d);line(84,195,876,195,0x5b7179);line(85,212,875,212,0x26323f);
 for(int i=0;i<5;i++){int x=123+i*158;rect(x-21,116,42,13,0x162737);box(x-22,130,44,8,0x334c54,0x77868e);rect(x-17,132,34,3,i==1||i==4?0xe6c58d:0x9cd0c8);
  for(int j=0;j<46;j++){int k=8+j*2/3;rect(x-k,138+j,k*2,1,mix(0x3c5156,i==1||i==4?0xa58459:0x659291,3-j/16));}}
 for(int i=0;i<4;i++){int x=115+74*i;box(x,149,53,39,0x10222c,0x658590);line(x,168,x+52,168,0x58777e);line(x+26,150,x+26,188,0x58777e);
  if(nt_level(NT_SHUTTER)){for(int y=151;y<187;y+=6)line(x+2,y,x+51,y,nt_level(NT_SHUTTER)==2?0x8dc2b0:0x8a9b9e);}
  else{rect(x+5,158,5,6,0xdab983);rect(x+35,171,9,9,0x678496);}}
 int st=nt_level(NT_RACK);nt_shelf_art(119,196,173,1);if(st)nt_shelf_art(119,164,173,2);if(st==2)nt_shelf_art(119,131,173,3);
 box(397,182,167,54,0x182430,0x697d84);rect(399,187,163,22,0x445962);box(450,165,80,34,0x252f3d,0xb69969);
 for(int x=458;x<527;x+=13)rect(x,172,7,1,0xe9ba82);rect(460,184,54,5,0x35847c);rect(404,218,70,9,0x2e484d);rect(487,218,70,9,0x2e484d);
 box(659,184,159,45,0x192a31,0x687f82);rect(661,188,155,11,0x426569);box(684,161,65,30,0x142537,0x6d979e);
 for(int i=0;i<15;i++){int v=3+(i*41+((int)anim*11)%5)%12;rect(691+i*3,180-v,1,v,nt_level(NT_RELAY)?nt_mint:0x526b79);}
 if(nt_level(NT_RELAY))for(int i=0;i<nt_level(NT_RELAY)+1;i++){box(757+i*18,155,15,27,0x142735,0x50848a);rect(761+i*18,163,7,2,nt_mint);}
 for(int x=360;x<=600;x+=8){int c=(x/8)&1;rect(x,421,4,3,c?0x986a52:0x5e5552);}
 rect(440,440,81,26,0x131b28);box(445,447,72,17,0x192632,0x5e6f73);center(481,445,"EXIT",nt_amber,0);
 for(int i=0;i<3;i++){int x=nt_sz[i]-95,y=nt_sy[i]-23;int near=d2(nt_x-nt_sz[i],nt_y-nt_sy[i])<54*54;nt_label(x,y,nt_station[i],near?nt_amber:nt_low);}
 disc((int)nt_x,(int)nt_y+4,11,0x20343d);person((int)nt_x,(int)nt_y,0x6b9a90,nt_walking?((int)(anim*15)&3):0,nt_angle,4,0);
}
static void nt_panel_paint(void){
 int rows=nt_pane==NT_VISIT?1:nt_pane==NT_STOCK?9:3;
 const char*title=nt_pane==NT_VISIT?"TRAP HOUSE LEASE":nt_pane==NT_BUILD?"CONSTRUCTION":nt_pane==NT_STOCK?"GRADED STOCKROOM":"ELECTRONICS DISPATCH";
 box(206,102,548,345,nt_panel,0x627a80);rect(207,103,546,3,nt_amber);textat(227,112,title,nt_amber,1);
 char sum[96];const char*sub;
 if(nt_pane==NT_VISIT)sub="Bell Lane Annex / Separate property from Lantern Grow Loft";
 else if(nt_pane==NT_BUILD)sub="Three separate construction projects / clean wallet";
 else if(nt_pane==NT_STOCK){number(append(number(append(sum,"Rack space: "),nt_stock_total())," / "),nt_capacity());sub=sum;}
 else{char*q=number(append(sum,"Completed: "),nt_jobs());q=number(append(q,"  /  local surcharge: $"),nt_overhead());sub=sum;(void)q;}
 textat(229,160,sub,0xb4c4c6,0);line(228,182,733,182,0x43595e);
 int start=nt_pane==NT_STOCK?cl(nt_sel-2,0,4):0;
 for(int r=start;r<rows&&r<start+5;r++){
  int y=187+(r-start)*30,chosen=r==nt_sel;char label[86]="",right[70]="";
  if(nt_pane==NT_VISIT){append(label,"Purchase the Bell Lane Trap House");append(right,nt_confirm?"CONFIRM $480":"$480 CLEAN");}
  else if(nt_pane==NT_BUILD){int t=nt_level(r);append(label,nt_upgrade[r]);char*q=number(append(right,"TIER "),t);
   if(t<2)number(append(q,nt_confirm&&chosen?"  CONFIRM $":"  /  $"),nt_price[r][t]);else append(q,"  /  BUILT");}
  else if(nt_pane==NT_STOCK){np_print(label,r);char*q=number(append(right,"RACK "),nt_count(r));number(append(q,"  /  BAG "),np_get(NP_BAG+r,99));}
  else if(r==2){append(label,"Claim completed payout");if(!nt_ticket())append(right,"IDLE");else if(nt_clock())number(append(right,"SECONDS "),((nt_clock()+59)/60));else number(append(right,"READY $"),nt_income(nt_ticket()));}
  else{int t=r+1;append(label,r==0?"Standard repair":"Rush refurbishment");if(r&&!nt_level(NT_RELAY))append(right,"RELAY REQUIRED");
    else{char*q=number(append(right,nt_confirm&&chosen?"CONFIRM $":"FEE $"),nt_fee(t)+nt_overhead());number(append(q,"  /  PAYS $"),nt_income(t));}}
  box(226,y,507,28,chosen?0x283d49:0x172b35,chosen?0x71858b:0x2b3d47);if(chosen)rect(226,y,3,28,nt_amber);
  textat(236,y+4,label,chosen?PAPER:0xb7c5c9,0);textat(725-textw(right,0),y+4,right,chosen?nt_amber:nt_mint,0);
 }
 const char*detail=nt_msg;
 if(!*detail){
  if(nt_pane==NT_VISIT)detail="Enter twice to buy. It opens the stockroom, upgrades and neighborhood repair commissions.";
  else if(nt_pane==NT_BUILD)detail=nt_sel==NT_RACK?"Adds nine grade-preserving storage spaces per tier.":nt_sel==NT_SHUTTER?"Each tier removes $8 from local-attention service surcharges. It does not change an active pursuit.":"Each tier saves six seconds per dispatch and adds $14 to the payout. Tier one opens rush bookings.";
  else if(nt_pane==NT_STOCK)detail="Enter / E stores one. Space returns one. Your exact strain and quality stay with each unit.";
  else detail="A separate legal electronics service. Book a repair, explore the city, then return to claim clean earnings.";
 }
 wrapped(229,345,detail,nt_msg[0]?nt_amber:0xb5c7ce,61);
 textat(229,414,"ENTER / E  ACT        ARROWS  CHOOSE        ESC  BACK",0x9db0b8,0);
}
static void night_trap_paint(void){
 nt_show_floor();textat(67,42,"TRAP HOUSE",PAPER,1);textat(70,86,"BELL LANE ANNEX  /  TIDE MARKET",nt_amber,0);
 char b[68];number(append(b,"CLEAN CASH  $"),sv.cash);textat(883-textw(b,0),86,b,nt_mint,0);
 if(nt_pane>=0)nt_panel_paint();
 else{
  int near=nt_nearest();box(61,471,838,35,0x101c28,0x475d63);
  textat(75,478,near<0?"WASD WALK   /   E AT A STATION":near==NT_EXIT_DOOR?"E / RETURN TO BELL LANE":nt_station[near],nt_amber,0);
  textat(638,478,"ESC EXIT   /   F5 SAVE",0xb8c5c8,0);
  if(nt_msg[0])center(480,446,nt_msg,nt_mint,0);
 }
}
