/* THE AFTERIMAGE | Base Village entrance and original Shadow Stage alias.
   Renderer is native software pixel art. No streams/assets or system clocks.
   Persistent fields live with their career; static fields are room/overlay cache only. */
static int night_phone_signature(int design,int consume);
enum { NC_ID=NIGHT_CLUB_LEGACY, NC_SHOW=2700, NC_PRICE=480, NC_BANKMAX=9999, NC_DANCE_FEE=25, NC_DANCE_DURATION=8*60 };
enum { NC_OWN,NC_COVER,NC_ACT,NC_STAFF,NC_DRINK,NC_DECOR,NC_REP,NC_CLEAN,
       NC_SERVICE,NC_TILL,NC_CLOCK,NC_SHOWS,NC_DOOR,NC_DISPUTE,NC_VIP,
       NC_SERVED,NC_ENCORE,NC_TRACK,NC_EXCHANGE,NC_LAST,NC_VISIT,
       NC_TABLES,NC_SIGNATURE0,NC_SIGNATURE1,NC_SIGNATURE2,NC_SHOWCASE,
       NC_LAST_VIP,NC_LAST_DOOR,NC_LAST_DRINKS,NC_LAST_COST,NC_LAST_AUX,NC_PREVENT,NC_FIELD_COUNT };
_Static_assert(NC_FIELD_COUNT<=32,"club save bounds");
enum { NC_FLOOR,NC_SUITE,NC_OFFICE };
enum { NC_NONE,NC_DESK,NC_DECK,NC_TABLE,NC_STOCK,NC_BAR,NC_CUE,NC_GUARD,NC_DANCE_CONFIRM,NC_DANCE_PLAY };
static const U ncGold=0xf4cd86,ncRose=0xf691b4,ncIce=0x8ce7df,ncDim=0xaba7bc;
static const char*const ncTracks[3]={"Velvet Relay","Copper Current","Last Transmission"};
static const char*const ncDrinks[3]={"Rose Comet","Mint Static","Amber Halo"};
static const char*const ncActs[3]={"House trio","Prism Quartet","The Nightbirds"};
static const int ncCoverPrice[3]={0,3,6};
static const int ncSignaturePay[3]={18,25,32};
static const char*const ncPrices[3]={"Open door  /  $0","Standard  /  $3","Premium  /  $6"};
static const char*const ncBooths[3]={"Velvet","Mirage","Aurora"};
static const U ncBoothInk[3]={0xeab1bc,0xdab9ef,0xace6db};
static const U ncBoothUph[3]={0x8c5263,0x694b83,0x416e73};
static const char*const ncSignatures[3]={"Halo Moss","Prism Fern","Echo Bloom"};
static const char*const ncIncidents[4]={"Queue is calm","Overbooked seats","Crossed the rope","Back hall blocked"};
static const char*const ncRemedies[3]={"Check the seating list","Ask guard to reset rope","Clear the back hallway"};
static int ncRoom,ncPanel,ncChoice,ncFreeTrack,ncBooth,ncVisitServed,ncDanceTicks,ncCache=-1;
static float ncX,ncY,ncAngle;
static U ncBackdrop[W*H];
/* All external save words are bounded when read. A zeroed block is an unsold deed. */
static int nc_get(int field){
 const int caps[NC_FIELD_COUNT]={1,2,2,2,12,2,100,100,100,NC_BANKMAX,NC_SHOW-1,999999,
                                 120,3,8,1,1,2,30,600,999999,7,8,8,8,7,100,600,100,12,20,1};
 U v=nv.club[field];return v>(U)caps[field]?caps[field]:(int)v;
}
static int nc_owned(void){return nv.club[NC_OWN]==1u;}
static int nc_re(int n,int d,int cap){return cl(n+d,0,cap);}
static void nc_heat_score(int d){nv.club[NC_REP]=nc_re(nc_get(NC_REP),d,100);}
static int nc_music(void){return nc_owned()?nc_get(NC_TRACK):cl(ncFreeTrack,0,2);}
static int nc_headline(void){int hr=(int)sv.dayClock/60;return hr>=20||hr<5;}
static int night_club_track(void){return scene==NIGHT_CLUB_SCENE?nc_music()+1:0;}
/* Display population, not NPC objects: no collision, pathing or trade logic. */
static int night_club_queue(void){
 if(!nc_headline())return nc_owned()&&nc_get(NC_REP)>=65?2:0;
 if(!nc_owned())return 4;
 return cl(3+nc_get(NC_REP)/18+nc_get(NC_CLEAN)/35+nc_get(NC_SERVICE)/50
           -nc_get(NC_COVER)*2+nc_get(NC_DISPUTE)-nc_get(NC_STAFF),1,8);
}
static void nc_receipt(const char *label,int n,const char *unit){
 char b[120];char*q=append(b,label);q=number(q,n);append(q,unit);tell(b);
}
static void nc_save_note(const char*msg){tell(msg);request_save();}
static int nc_pay(int value){
 if(sv.charges){tell("Business accounts on hold. Resolve the case at Revenue.");return 0;}
 if(sv.cash<value){nc_receipt("Requires $",value," in clean cash.");return 0;}
 return spend(value);
}
/* Slots 0..20 are the old Rev11 club payload. NC_VIP is still the cabinet's
   aggregate quantity; its unstamped portion (from old saves) remains sellable.
   New typed stock never aliases an old slot or exceeds the shared capacity. */
static int night_club_signature_demand(int booth){
 return nc_owned()&&(U)booth<3?(nc_get(NC_SHOWS)+booth)%3:-1;
}
static int nc_typed(int design){
 int available=nc_get(NC_VIP);
 for(int i=0;i<design;i++)available-=min(available,nc_get(NC_SIGNATURE0+i));
 return min(available,nc_get(NC_SIGNATURE0+design));
}
static int nc_legacy_packs(void){return nc_get(NC_VIP)-nc_typed(0)-nc_typed(1)-nc_typed(2);}
static int nc_remove_pack(int design,int legacy_ok){
 int n=nc_typed(design);
 if(n)nv.club[NC_SIGNATURE0+design]=n-1;
 else if(!legacy_ok||!nc_legacy_packs())return 0;
 nv.club[NC_VIP]=nc_get(NC_VIP)-1;return 1;
}
static int nc_guest_order(int booth){return (nc_get(NC_SHOWS)+2*booth)%3;}
static void night_club_business_tick(void){
 if(!nc_owned())return;
 int t=nc_get(NC_CLOCK)+1;
 if(t<NC_SHOW){nv.club[NC_CLOCK]=(U)t;return;}
 nv.club[NC_CLOCK]=0;
 int n=nc_get(NC_SHOWS),cover=nc_get(NC_COVER),act=nc_get(NC_ACT),staff=nc_get(NC_STAFF),
     rep=nc_get(NC_REP),clean=nc_get(NC_CLEAN),service=nc_get(NC_SERVICE),deco=nc_get(NC_DECOR),
     drinks=nc_get(NC_DRINK),incident=nc_get(NC_DISPUTE),match=nc_music()==n%3;
 int attended=cl(12+rep/8+(clean-50)/12+(service-50)/10+act*3+deco*2+(nc_headline()?5:0)
                 +(match?4:-1)-cover*(rep<65?6:2)-incident*3,3,75);
 int served=min(drinks,1+attended/17),vipEarned=0,slots=served;
 /* Passively serve matching prints to this set's three bookings. Visit a
    booth yourself to make the same sale immediately with a personal tip. */
 for(int booth=0;booth<3&&slots;booth++){
  if(nc_get(NC_SHOWCASE)&(1<<booth))continue;
  int design=night_club_signature_demand(booth);
  int typed=nc_typed(design)>0;
  if(nc_remove_pack(design,booth==1)){
   vipEarned+=typed?ncSignaturePay[design]:18;slots--;
  }
 }
 int door=attended*ncCoverPrice[cover];
 int drink=served*6,cost=staff*4,aux=8+nc_get(NC_ENCORE)*6;
 int gross=cl(door+drink+vipEarned+aux-cost,1,600);
 nv.club[NC_TILL]=min(NC_BANKMAX,nc_get(NC_TILL)+gross);
 nv.club[NC_LAST]=gross;nv.club[NC_DOOR]=attended;nv.club[NC_DRINK]=drinks-served;
 nv.club[NC_LAST_DOOR]=door;nv.club[NC_LAST_VIP]=vipEarned;nv.club[NC_LAST_DRINKS]=drink;
 nv.club[NC_LAST_COST]=cost;nv.club[NC_LAST_AUX]=aux;
 nv.club[NC_CLEAN]=nc_re(clean,-(6+attended/6)+staff*2,100);
 nv.club[NC_SERVICE]=nc_re(service,(served?2+staff:-6)-(incident?3:0),100);
 nc_heat_score((served?2:-2)+(clean>=65?2:clean<45?-3:0)+(service>=75?2:service<40?-2:0)
               +(match?2:-1)+(nc_get(NC_TABLES)||nc_get(NC_SERVED)?1:0)+(nc_get(NC_ENCORE)?1:0)
               +(vipEarned?1:0)-(incident?5:0)-(cover==2&&rep<50?2:0));
 /* Two guards always prevent incidents; a manager can also intervene. */
 nv.club[NC_DISPUTE]=staff<2&&!nc_get(NC_PREVENT)&&
    ((n+1)%(3+staff*2)==0||clean<18)?1+(n/2)%3:0;
 nv.club[NC_SHOWS]=n==999999?0:(U)n+1;
 nv.club[NC_SERVED]=0;nv.club[NC_ENCORE]=0;nv.club[NC_TABLES]=0;nv.club[NC_SHOWCASE]=0;nv.club[NC_PREVENT]=0;
 if(nc_get(NC_EXCHANGE)&&!sv.charges&&nc_get(NC_TILL)<=NC_BANKMAX-nc_get(NC_EXCHANGE)){
  nv.club[NC_TILL]=nc_get(NC_TILL)+nc_get(NC_EXCHANGE);nv.club[NC_EXCHANGE]=0;
 }
 if(scene==NIGHT_CLUB_SCENE&&p.noticeT<=0)nc_receipt("Set closed. Box office earned $",gross,".");
 request_save();
}
static void nc_enter_room(int room,int from){
 ncRoom=room;ncPanel=NC_NONE;ncChoice=0;walking=0;ncAngle=-1.570796f;
 ncX=room==NC_FLOOR?(from==NC_SUITE?784:from==NC_OFFICE?178:480):480;
 ncY=room==NC_FLOOR?(from==NC_SUITE||from==NC_OFFICE?361:423):420;
}
static int night_club_open(int building){
 if((building!=NC_ID&&building!=ncFrontId)||(U)building>=(U)nb||!ordinaryDoorY[building])return 0;
 walkin_active=building;ncFreeTrack=0;ncVisitServed=0;
 ncBooth=0;ncDanceTicks=0;ncCache=-1;
 scene=NIGHT_CLUB_SCENE;menu=-1;mapOn=0;p.noticeT=0;nc_enter_room(NC_FLOOR,-1);
 return 1;
}
static int nc_zone(float x,float y,int l,int t,int r,int b){return x>=l&&x<=r&&y>=t&&y<=b;}
static int nc_solid(float x,float y){
 if(!nc_zone(x,y,83,172,877,433))return 1;
 if(ncRoom==NC_FLOOR){
  if(nc_zone(x,y,290,120,671,228))return 1;
  if(nc_zone(x,y,68,145,234,263)||nc_zone(x,y,733,145,892,260))return 1;
  if(nc_zone(x,y,143,305,240,345)||nc_zone(x,y,721,305,817,345))return 1;
  if(nc_zone(x,y,82,302,124,374)||nc_zone(x,y,836,302,880,374))return 1;
 }else if(ncRoom==NC_SUITE){
  if(nc_zone(x,y,77,146,294,247)||nc_zone(x,y,365,144,600,249)||nc_zone(x,y,655,147,884,248))return 1;
  if(nc_zone(x,y,80,304,210,372)||nc_zone(x,y,743,307,884,363))return 1;
 }else{
  if(nc_zone(x,y,84,146,265,211)||nc_zone(x,y,294,143,648,248)||nc_zone(x,y,687,147,875,215))return 1;
  if(nc_zone(x,y,97,302,240,358)||nc_zone(x,y,730,303,867,360))return 1;
 }
 return 0;
}
enum { NCA_NONE,NCA_EXIT,NCA_OFFICE,NCA_SUITE,NCA_RETURN,NCA_DJ,NCA_BAR,NCA_STAGE,NCA_SECURITY,NCA_MANAGE,NCA_VIP_VELVET,NCA_CABINET,NCA_CLEAN,NCA_VIP_MIRAGE,NCA_VIP_AURORA };
#define NCA_VIP NCA_VIP_MIRAGE /* old fixture had only the middle booth */
static int nc_target(void){
 if(nc_zone(ncX,ncY,421,394,540,435))return ncRoom==NC_FLOOR?NCA_EXIT:NCA_RETURN;
 if(ncRoom==NC_FLOOR){
  if(nc_zone(ncX,ncY,86,294,184,379))return NCA_OFFICE;
  if(nc_zone(ncX,ncY,779,294,877,379))return NCA_SUITE;
  if(nc_zone(ncX,ncY,83,171,268,291))return NCA_DJ;
  if(nc_zone(ncX,ncY,702,171,877,291))return NCA_BAR;
  if(nc_zone(ncX,ncY,326,213,634,271))return NCA_STAGE;
  if(nc_zone(ncX,ncY,541,345,638,418))return NCA_SECURITY;
 }else if(ncRoom==NC_SUITE){
  if(nc_zone(ncX,ncY,105,218,298,287))return NCA_VIP_VELVET;
  if(nc_zone(ncX,ncY,368,219,600,287))return NCA_VIP_MIRAGE;
  if(nc_zone(ncX,ncY,653,221,854,287))return NCA_VIP_AURORA;
  if(nc_zone(ncX,ncY,699,276,877,385))return NCA_CABINET;
  if(nc_zone(ncX,ncY,83,278,252,387))return NCA_CLEAN;
 }else if(nc_zone(ncX,ncY,262,154,682,289))return NCA_MANAGE;
 return NCA_NONE;
}
static const char*nc_hint(int target){
 static const char*const h[]={"Walk to a lit station","Exit to the city","Enter the box office","Enter the velvet lounge","Return to the main floor","Choose a record","Visit the bar","Cue an encore","Talk to door staff","Open the management ledger","Attend Velvet booth","Open VIP stock cabinet","Refresh the lounge","Attend Mirage booth","Attend Aurora booth"};
 return h[cl(target,0,LEN(h)-1)];
}
static void nc_showpanel(int panel){ncPanel=panel;ncChoice=0;}
static int nc_items(void){
 switch(ncPanel){case NC_DESK:return nc_owned()?9:2;case NC_DECK:return 4;case NC_TABLE:return nc_owned()?6:5;case NC_DANCE_CONFIRM:return 2;
 case NC_BAR:return 3;case NC_STOCK:return 5;case NC_GUARD:return nc_owned()&&nc_get(NC_DISPUTE)?4:2;case NC_CUE:return 2;}return 0;
}
static void nc_use_target(int target){
 switch(target){
 case NCA_EXIT:{
   int door=walkin_active;scene=1;walkin_active=-1;walking=0;menu=-1;
   p.x=ordinaryDoorX[door];p.y=ordinaryDoorY[door];p.a=1.570796f;by=placeNear();near=carNear();p.noticeT=0;break;}
 case NCA_OFFICE:nc_enter_room(NC_OFFICE,NC_FLOOR);break;
 case NCA_SUITE:nc_enter_room(NC_SUITE,NC_FLOOR);break;
 case NCA_RETURN:nc_enter_room(NC_FLOOR,ncRoom);break;
 case NCA_DJ:nc_showpanel(NC_DECK);break;case NCA_BAR:nc_showpanel(NC_BAR);break;
 case NCA_STAGE:nc_showpanel(NC_CUE);break;case NCA_SECURITY:nc_showpanel(NC_GUARD);break;
 case NCA_MANAGE:nc_showpanel(NC_DESK);break;
 case NCA_VIP_VELVET:case NCA_VIP_MIRAGE:case NCA_VIP_AURORA:
  ncBooth=target==NCA_VIP_VELVET?0:target==NCA_VIP_MIRAGE?1:2;nc_showpanel(NC_TABLE);break;
 case NCA_CABINET:nc_showpanel(NC_STOCK);break;
 case NCA_CLEAN:
  if(!nc_owned()){tell("Housekeeping: the owner can refresh these velvet booths.");break;}
  if(nc_get(NC_CLEAN)>=100){tell("Already spotless. Check again after the next set.");break;}
  nv.club[NC_CLEAN]=100;nc_save_note("Tables reset, glassware polished. Cleanliness restored to 100.");break;
 }
}
static void nc_ledger(int row){
 if(!nc_owned()){
  if(row||!nc_pay(NC_PRICE))return;
  for(int i=0;i<32;i++)nv.club[i]=0;
  nv.club[NC_OWN]=1;nv.club[NC_REP]=45;nv.club[NC_CLEAN]=80;nv.club[NC_SERVICE]=60;
  nv.club[NC_COVER]=1;nv.club[NC_DRINK]=4;nv.club[NC_TRACK]=(U)ncFreeTrack;
  ncCache=-1;nc_save_note("Deed signed. Four drinks included; your first set starts now.");return;
 }
 int x;
 switch(row){
 case 0:
  x=nc_get(NC_TILL);if(!x){tell("Till empty. Sets close every 45 seconds of city time.");break;}
  x=credit_wallet(1,x);nv.club[NC_TILL]=nc_get(NC_TILL)-x;nc_receipt("Collected $",x," in clean box-office proceeds.");request_save();break;
 case 1:nv.club[NC_COVER]=(nc_get(NC_COVER)+1)%3;nc_save_note("Door price changed. Higher prices can cut turnout.");break;
 case 2:x=nc_get(NC_ACT);
  if(x==2){tell("The Nightbirds already hold your headline residency.");break;}
  if(!nc_pay(x?95:55))break;
  nv.club[NC_ACT]=x+1;nc_save_note("Residency booked. The new ensemble is on stage.");break;
 case 3:x=nc_get(NC_STAFF);if(x>=2){tell("Both door shifts are staffed.");break;}
  if(!nc_pay(x?90:60))break;
  nv.club[NC_STAFF]=x+1;nv.club[NC_DISPUTE]=0;nc_heat_score(2);nc_save_note("Door staff hired. The queue is calm.");break;
 case 4:x=nc_get(NC_DRINK);if(x>=12){tell("Drink fridge full: twelve house servings.");break;}
  if(!nc_pay(min(4,12-x)*3))break;
  nv.club[NC_DRINK]=min(12,x+4);nc_save_note("House drinks delivered to the VIP fridge.");break;
 case 5:x=nc_get(NC_DECOR);if(x>=2){tell("Aurora canopy fitted. All interior upgrades installed.");break;}
  if(!nc_pay(x?170:95))break;
  nv.club[NC_DECOR]=x+1;nc_heat_score(4);ncCache=-1;nc_save_note(x?"Aurora canopy installed above the floor.":"Prism mirrors installed. Inspect the floor and the lounge.");break;
 case 6:
  if(sv.charges){tell("Exchange suspended while the Revenue case is open.");break;}
  if(nc_get(NC_EXCHANGE)){tell("One token exchange per set. The next closing clears it.");break;}
  if(sv.dirty<40){tell("Bring 40 marked game credits to exchange for a $30 till token.");break;}
  if(nc_get(NC_TILL)>NC_BANKMAX-530){tell("Collect the till before reserving a token.");break;}
  sv.dirty-=40;nv.club[NC_EXCHANGE]=30;nc_save_note("Arcade token queued. $30 reaches the till next set, case permitting.");break;
 case 7:
  if(nc_get(NC_CLEAN)==100&&nc_get(NC_SERVICE)==100){tell("Both housekeeping and service are at 100.");break;}
  if(!nc_pay(16))break;
  nv.club[NC_CLEAN]=100;nv.club[NC_SERVICE]=100;nc_save_note("House crew reset the room. Service and cleanliness restored.");break;
 }
}
static float nc_beat(void){return sn(anim*(nc_music()==0?4.8f:nc_music()==1?5.6f:4.1f));}
static void nc_activate(int row){
 int count=nc_items();if(row<0||row>=count)return;
 if(ncPanel==NC_DANCE_CONFIRM){
  if(!row){nc_showpanel(NC_TABLE);return;}
  if(sv.cash<NC_DANCE_FEE){tell("The private cabaret costs $25 in clean cash.");return;}
  if(!spend(NC_DANCE_FEE))return;
  request_save();
  if(saveFlash<0){sv.cash+=NC_DANCE_FEE;sv.hash=savehash();tell("Save unavailable. The $25 booking was not charged.");return;}
  p.noticeT=0;ncDanceTicks=0;nc_showpanel(NC_DANCE_PLAY);return;
 }
 if(row==count-1){ncPanel=NC_NONE;return;}
 switch(ncPanel){
 case NC_DESK:nc_ledger(row);break;
 case NC_DECK:
  if(nc_owned())nv.club[NC_TRACK]=row;else ncFreeTrack=row;
  nc_receipt("Record loaded on deck ",row+1,".");if(nc_owned())request_save();break;
 case NC_BAR:
  if(!row){
   if(!spend(5))break;p.hp=cl(p.hp+6,0,100);nc_save_note("House soda served. Recovered up to 6 health.");
  }else if(nc_owned())nc_ledger(4);
  else tell("House deliveries are for the deed holder.");
  break;
 case NC_STOCK:
  if(!nc_owned()){tell("Only the deed holder may stock this cabinet.");break;}
  if(!row){nc_ledger(4);break;}
  if(nc_get(NC_VIP)>=8){tell("Cabinet full: eight Top Shelf packs.");break;}
  if(!night_phone_signature(row-1,1)){tell("No matching Top Shelf strain in your phone satchel.");break;}
  nv.club[NC_SIGNATURE0+row-1]=nc_typed(row-1)+1;
  nv.club[NC_VIP]=nc_get(NC_VIP)+1;
  nc_save_note("Top Shelf stocked. Check which booth has reserved this strain.");break;
 case NC_TABLE:{
  if(row==(nc_owned()?4:3)){nc_showpanel(NC_DANCE_CONFIRM);break;}
  int want=nc_guest_order(ncBooth),bit=1<<ncBooth;
  int tables=nc_get(NC_TABLES);
  /* Old saves stored one already-attended booth, which was Mirage. */
  if(nc_get(NC_SERVED)&&!tables)tables=2;
  if(row==3){
   if(!(tables&bit)){tell("Serve the drink first, then bring the reserved strain.");break;}
   if(nc_get(NC_SHOWCASE)&bit){tell("This booth already received its Top Shelf order.");break;}
   int design=night_club_signature_demand(ncBooth);
   int typed=nc_typed(design)>0;
   if(!nc_remove_pack(design,1)){tell("That strain is out of stock. Restock an exact Top Shelf pack.");break;}
   nv.club[NC_SHOWCASE]=nc_get(NC_SHOWCASE)|bit;
   int value=typed?ncSignaturePay[design]:18;nc_heat_score(1);
   nv.club[NC_TILL]=min(NC_BANKMAX,nc_get(NC_TILL)+value);
   nc_receipt("Top Shelf order served. +$",value," to the till.");request_save();break;
  }
  if((nc_owned()?tables:ncVisitServed)&bit){tell("That booth is all set. A fresh audience arrives after closing.");break;}
  if(row!=want){tell("The host points to the order card. Match the requested drink.");break;}
  const int tips[3]={8,12,16},services[3]={8,14,17};
  if(nc_owned()&&!nc_get(NC_DRINK)){tell("The fridge is empty. Restock at the cabinet.");break;}
  if(nc_owned()){
   nv.club[NC_DRINK]=nc_get(NC_DRINK)-1;nv.club[NC_SERVED]=1;nv.club[NC_TABLES]=tables|bit;
   nv.club[NC_VISIT]=nc_re(nc_get(NC_VISIT),1,999999);nv.club[NC_SERVICE]=nc_re(nc_get(NC_SERVICE),services[ncBooth],100);
   nc_heat_score(2+ncBooth);nv.club[NC_TILL]=min(NC_BANKMAX,nc_get(NC_TILL)+tips[ncBooth]);
   nc_receipt("Order matched. Service and reputation rose. Tip: $",tips[ncBooth],".");request_save();
  }else{ncVisitServed|=bit;tell("Order matched. The host tips their hat: you know this room.");}
  break;}
 case NC_CUE:
  if(!nc_owned()){tell("The ensemble reprises the chorus. Buy the venue to book a residency.");ncPanel=0;break;}
  if(nc_get(NC_ENCORE)){tell("Encore played. Wait for the next set.");break;}
  if(nc_beat()>.28f){nv.club[NC_ENCORE]=1;nc_heat_score(2);nc_save_note("On the downbeat! Encore booked. +2 reputation and a $6 set bonus.");}
  else tell("Just missed. Cue when the beat bar reaches the gold line.");break;
 case NC_GUARD:{
  if(!nc_owned()){tell("The house steward waves. All performances are on the board.");break;}
  int state=nc_get(NC_DISPUTE);
  if(!state){
   if(!nc_get(NC_PREVENT)){
    nv.club[NC_PREVENT]=1;nc_save_note("Staff briefed. No new disruption will start after this set.");
   }else tell("The line is calm and the crew is ready.");
   break;
  }
  if(row!=state-1){tell("That does not solve this report. Check the steward's note.");break;}
  nv.club[NC_DISPUTE]=0;nv.club[NC_PREVENT]=1;nc_heat_score(2);ncPanel=0;
  nc_save_note("Handled. The line settles, reputation rises and the next set is protected.");break;}

 }
}
/* Native scenic shop. Cached architecture, material grain & paint layers; moving
   light and actors never enter the cache. All colors sRGB, deterministic pixel art. */
static void nc_oval(int x,int y,int rx,int ry,U c){
 if(rx<=0||ry<=0)return;
 for(int j=-ry;j<=ry;j++){int w=(int)(rx*sq(1.f-(float)(j*j)/(ry*ry)));rect(x-w,y+j,w*2+1,1,c);}
}
static void nc_rhomb(int x,int y,int w,int h,U c){
 line(x-w,y,x,y-h,c);line(x,y-h,x+w,y,c);line(x+w,y,x,y+h,c);line(x,y+h,x-w,y,c);
}
static void nc_sun(int x,int y,int r,U c){
 for(int i=0;i<15;i++){float a=-3.14159f+i*.2244f;line(x+(int)(cs(a)*r*.35f),y+(int)(sn(a)*r*.35f),x+(int)(cs(a)*r),y+(int)(sn(a)*r),c);}
 line(x-r,y+1,x+r,y+1,c);nc_rhomb(x,y,r/3,r/3,c);
}
static void nc_label(int x,int y,const char*s,U c){
 int w=textw(s,0);rect(x-w/2-6,y+1,w+12,18,0x201a2c);center(x,y,s,c,0);
}
static void nc_speaker(int x,int y){
 box(x,y,23,45,0x252539,0x5d4960);for(int k=0;k<2;k++){
  int yy=y+12+k*20;disc(x+11,yy,9,0x141622);disc(x+11,yy,7,0x3d344c);disc(x+11,yy,4,0x292435);disc(x+10,yy-1,2,0x655166);}
 for(int i=3;i<22;i+=4)dot(x+i,y+2,0x918395);
}
static void nc_sofa(int x,int y,int w,U col){
 rect(x+4,y+8,w,32,0x171521);box(x,y,w,32,mix(col,0x1f1725,3),mix(col,ncGold,2));
 for(int i=3;i<w-6;i+=26){int k=min(24,w-6-i);box(x+i,y+3,k,13,col,mix(col,ncRose,2));rect(x+i+2,y+19,k-2,9,mix(col,ncRose,1));}
 rect(x+3,y+31,w-6,3,0x9f805d);rect(x+7,y+34,3,4,0x9c7b55);rect(x+w-10,y+34,3,4,0x9c7b55);
}
static void nc_bottle(int x,int y,U c){
 rect(x-2,y-6,4,6,mix(c,ncIce,1));rect(x-4,y,8,10,c);rect(x-3,y+4,6,3,ncGold);rect(x-3,y+1,1,3,0xffe3cd);
}
static void nc_console(int x,int y){
 box(x,y,145,44,0x252336,ncGold);rect(x+1,y+2,143,3,0x746071);
 for(int k=0;k<2;k++){int cx=x+28+k*88;
  nc_oval(cx,y+21,20,12,0x0e131e);nc_oval(cx,y+21,15,9,0x343547);
  nc_oval(cx,y+21,9,6,0x202333);nc_oval(cx,y+21,4,3,k?ncRose:ncIce);
  line(cx+11,y+10,cx+8,y+20,ncGold);dot(cx+8,y+20,PAPER);}
 for(int i=0;i<7;i++){int xx=x+54+i*5;rect(xx,y+11,2,19,0x11151c);rect(xx-1,y+14+i%4*3,4,3,i%2?ncIce:ncRose);}
}
static void nc_torch(int x,int y,U accent){
 for(int k=3;k>0;k--)nc_oval(x,y-k*2,3+k*4,5+k*3,mix(0x282239,accent,4-k));
 box(x-3,y-6,6,13,0x473748,ncGold);rect(x-1,y-4,2,8,0xffe5a9);
 line(x-6,y+12,x,y+17,0x9b7e65);line(x,y+17,x+6,y+12,0x9b7e65);
}
static void nc_portal(int x,int y,const char*s,U c){
 box(x-19,y-28,38,60,0x161823,0x715365);box(x-15,y-25,30,54,0x312736,c);
 for(int i=0;i<3;i++)line(x-12+i*9,y-22,x-12+i*9,y+23,0x4a3947);
 rect(x+6,y+2,4,2,ncGold);nc_label(x,y+28,s,c);
}
static void nc_mirror(int x,int y,int w,int h){
 box(x,y,w,h,0x405363,0xc59c83);box(x+3,y+3,w-6,h-6,0x52475e,0x7c687b);
 for(int i=6;i<w-12;i+=15){int len=min(h-8,w-4-i);line(x+i,y+h-4,x+i+len,y+h-4-len,0x736e84);}
 nc_rhomb(x+w/2,y+h/2,8,11,0xdebac0);
}
static void nc_static_floor(int room,int level){
 U top=room==NC_FLOOR?0x2c2034:room==NC_SUITE?0x39232f:0x23323a;
 U bottom=room==NC_FLOOR?0x110f20:room==NC_SUITE?0x1e1725:0x171d29;
 for(int y=0;y<H;y++)rect(0,y,W,1,mix(top,bottom,cl((y-20)/52,0,8)));
 /* A stepped proscenium frames a cutaway rather than a second floating menu. */
 for(int i=0;i<6;i++){int k=i*4;box(50+k,86+k,860-k*2,365-k,0x201d2b,i%2?0x665064:0x2e2738);}
 box(76,113,808,326,top,0x9a7a76);
 for(int y=115;y<438;y++){
  int ratio=(y-115)*64/323;U base=mix(top,bottom,ratio/8);
  for(int x=77;x<883;x++){U grain=wi_hash((U)(x*937+y*5881));U v=base+((grain>>30)&1u)*0x020202;
   if(y>174&&room!=NC_FLOOR&&((x/32+y/16)&1))v+=0x020202;
   surface[y*rw+x]=v;}
 }
 /* Recessed, curved brass sight line with parallel marble inlays. */
 for(int k=0;k<3;k++){int y=395+k*8;
  line(81,y-11,289,y-11,0x53404c);line(289,y-11,332,y+1,0x53404c);
  line(332,y+1,632,y+1,0x53404c);line(632,y+1,675,y-11,0x53404c);line(675,y-11,878,y-11,0x53404c);
 }
 for(int x=88;x<877;x+=22)rect(x,121,2,47,x%3?0x554254:0x785a62);
 for(int x=81;x<883;x+=4)dot(x,169,0xa18174);
 if(room==NC_FLOOR){
  /* Pair of stair-stepped wings and an authentic-looking art deco band shell. */
  box(284,112,392,100,0x291a2a,0xa48178);
  for(int i=0;i<13;i++){
   int l=288+i*14,r=672-i*14,y=132+i*3;
   line(l,y,l+20,115,0x795667);line(r-20,115,r,y,0x795667);
   rect(l,y,2,74-i*4,0x433347);rect(r-2,y,2,74-i*4,0x433347);}
  for(int k=0;k<4;k++)nc_rhomb(480,150,80-k*13,32-k*5,k&1?0xb18583:0x65475d);
  nc_sun(480,157,26,ncGold);
  /* Velvet legs, polished rails and transom bulbs frame the cabaret. */
  for(int side=0;side<2;side++)for(int j=0;j<7;j++){
   int xx=side?650-j*4:301+j*4,h=54-j*5;
   rect(xx,121,4,h,j&1?0x714257:0x512d45);rect(xx,120+h,4,2,0xb98c82);}
  rect(319,120,322,2,0x9a7775);
  for(int i=0;i<15;i++)disc(324+i*22,124,1,ncGold);
  for(int i=0;i<2;i++){int xx=404+i*152;rect(xx-1,129,3,62,0x79687a);rect(xx,130,1,61,0xd5b3a4);}
  rect(302,180,356,36,0x523746);rect(293,192,374,22,0x624351);
  for(int y=195;y<=218;y+=4)rect(291+(y-195)/2,y,378-(y-195),2,(y&4)?0x987279:0x553b4c);
  rect(299,219,362,3,0xd99b8c);rect(306,222,348,4,0x533c4b);
  for(int i=0;i<13;i++){int x=314+i*28;rect(x,220,4,2,ncGold);}
  nc_speaker(268,158);nc_speaker(668,158);
  nc_console(86,208);nc_label(158,171,"01  /  SELECTOR",ncIce);
  for(int i=0;i<2;i++)box(735,160+i*20,134,4,0x795364,0x967576);
  for(int i=0;i<9;i++)nc_bottle(744+i*14,155-(i%2)*4,i%3==0?0x769a90:i%3==1?0xad737f:0xb99c77);
  for(int i=0;i<6;i++)nc_bottle(750+i*20,177-(i%2)*3,i%2?0x9a7c9e:0xa77d71);
  box(732,209,143,41,0x614455,0xb88f83);
  for(int k=0;k<13;k++)rect(739+k*11,218,2,25,0x9b7381);
  rect(730,207,148,6,0xbc9483);nc_label(803,123,"02  /  BAR",ncRose);
  /* Glass tiles laid on the diagonal, with an offset border instead of a grid of noise. */
  box(275,255,409,117,0x27253d,0x815771);box(279,259,401,109,0x211f32,0xb28b81);
  for(int y=262;y<366;y++)for(int x=282;x<678;x++){
   int u=(x-280+2*(y-260))/46,v=(1040+x-2*(y-260))/46;
   int edge=((x-280+2*(y-260))%46<2)||((1040+x-2*(y-260))%46<2);
   surface[y*rw+x]=edge?0x46374e:((u+v)&1)?0x332a42:0x29253a;
  }
  nc_portal(104,331,"OFFICE",ncGold);nc_portal(856,331,"VIP",ncRose);
  nc_sofa(145,309,93,0x534059);nc_sofa(724,309,91,0x634255);
  for(int side=0;side<2;side++){
   int xx=side?766:193;nc_oval(xx,361,21,5,0x151521);rect(xx-2,348,4,11,0x8b7574);
   nc_oval(xx,348,22,6,0x9e7f78);nc_oval(xx,347,20,4,0x5a4350);
   nc_bottle(xx-4,338,side?0x798b7d:0x9c728c);
   int fx=side?693:266;for(int i=0;i<3;i++){nc_rhomb(fx+(i-1)*12,353-i%2*2,4,7,0x896876);}
  }
  nc_torch(253,138,ncRose);nc_torch(707,138,ncIce);
  if(level>0){nc_mirror(87,131,132,32);nc_mirror(731,127,141,26);}
  if(level>=2){for(int i=0;i<7;i++){int x=310+i*56;nc_rhomb(x,116,12,8,ncIce);line(x,124,x,134,ncGold);disc(x,136,2,ncRose);}}
 }else if(room==NC_SUITE){
  /* Private bays: separate finish, silhouette and lamp per booth. */
  for(int i=0;i<3;i++){
   int x=91+i*266,cx=x+119;U ink=ncBoothInk[i],cloth=ncBoothUph[i],base=mix(cloth,0x211b2a,5);
   box(x,125,239,76,base,mix(ink,cloth,4));
   if(i==0){
    for(int j=0;j<19;j++){int xx=x+8+j*12,dy=(int)(sn(j*.1745f)*13);
     rect(xx,128,4,47+dy,mix(cloth,base,2+j%3));line(xx,174+dy,xx+3,176+dy,0xb27d83);}
    nc_sun(cx,164,25,ink);nc_rhomb(cx,160,67,25,0x9b6f7f);
   }else if(i==1){
    for(int j=0;j<5;j++){int xx=x+17+j*43;box(xx,131,32,46,0x4c4167,0x93778d);
     for(int k=0;k<3;k++)line(xx+2+k*8,174,xx+25+k*2,145,0x63547a);}
    nc_rhomb(cx,153,49,16,ink);if(level)nc_mirror(x+66,135,108,41);
   }else{
    for(int j=0;j<21;j++){int xx=x+12+j*10,yy=153+(int)(sn(j*.324f)*14);
     line(xx,140,xx,yy+20,mix(cloth,ink,j%3+1));disc(xx,yy,2,mix(cloth,ink,j%3+2));}
    nc_oval(cx,168,68,15,0x2d414b);nc_rhomb(cx,159,52,17,ink);
   }
   nc_sofa(x+10,189,216,cloth);
   if(i==0){nc_oval(cx,233,51,10,0x171722);nc_oval(cx,226,50,12,0x997f78);nc_oval(cx,224,46,9,0x634552);}
   if(i==1){box(cx-51,214,103,20,0x4a405b,0x887386);nc_rhomb(cx,224,36,7,0x786885);}
   if(i==2){tri(cx,211,cx-53,231,cx+53,231,0x5a7c7e);line(cx-54,232,cx+54,232,0x8aa8a3);nc_rhomb(cx,224,33,8,0x96aca0);}
   nc_bottle(cx-7,214,0x688987);nc_torch(x+12,158,ink);nc_torch(x+225,158,ink);
   char label[28];char*q=number(label,1+i);q=append(q," / ");append(q,ncBooths[i]);nc_label(cx,247,label,ink);
  }
  box(85,305,125,50,0x47313f,0xb58b77);for(int i=0;i<4;i++)box(90+i*30,310,25,39,0x624451,0x957170);
  rect(84,300,128,7,0xb99a87);for(int j=0;j<4;j++){rect(92+j*3,295-j*2,40-j*4,3,ncDim);}
  nc_label(151,356,"LINEN CART",ncGold);
  box(747,306,128,50,0x333944,0xb79b88);for(int j=0;j<6;j++)nc_bottle(761+18*j,323,j%2?0x9e6679:0x688e87);
  rect(746,347,131,5,0x9d7b6a);nc_label(811,355,"STOCK",ncIce);
  box(288,300,385,62,0x432c40,0x8b666c);box(294,306,373,50,0x3b2939,0x735464);
  for(int i=0;i<13;i++)nc_rhomb(315+i*27,331,12,15,i%2?0x765665:0x5d4252);
  if(level>=2){for(int i=0;i<6;i++){int xx=210+i*106;line(xx,115,xx,129,ncGold);nc_rhomb(xx,131,6,6,ncIce);}}
 }else{
  box(94,128,139,64,0x3b3c40,0x998b77);
  for(int j=0;j<3;j++){rect(98,148+j*17,131,2,0x827666);
   for(int i=0;i<11;i++){int xx=104+i*11;rect(xx,134+j*17+(i%3),5+i%3,14-i%3,i%2?0x5b6769:0x7f6268);}}
  box(707,127,152,68,0x253a40,0x998675);for(int i=0;i<6;i++){int j=9+i*5;rect(719+i*21,185-j,11,j,mix(ncIce,ncGold,i));}
  nc_label(480,119,"OPERATIONS   /   PRIVATE",ncGold);
  for(int k=0;k<5;k++){int x=329+k*72;line(x,143,x+14,127,0x877164);line(x+14,127,x+29,143,0x877164);}
  box(305,185,350,55,0x59444b,0xd1a986);
  for(int i=0;i<13;i++)rect(310+i*27,206,2,26,0x81606b);
  rect(301,184,358,13,0xae8e79);rect(305,186,350,2,0xc8ab94);
  box(444,155,69,30,0x172c30,0xc0a28c);for(int i=0;i<4;i++)rect(450,161+5*i,42-i*5,1,i%2?ncIce:0x587a77);
  rect(475,184,7,4,0x51565a);box(369,177,37,12,0xd6c1a0,0x846b62);line(373,182,396,182,0x7c6863);
  nc_bottle(587,173,0x56797a);nc_label(480,240,"04  /  LEDGER",ncIce);
  nc_sofa(101,309,138,0x4b5963);nc_sofa(728,309,135,0x4b5963);
  for(int i=0;i<5;i++)nc_rhomb(362+i*59,335,20,11,0x475058);
  nc_torch(267,144,ncIce);nc_torch(681,144,ncGold);
 }
 nc_label(480,422,room==NC_FLOOR?"STREET  /  EXIT":"MAIN  /  FLOOR",ncGold);
 for(int i=0;i<7;i++){rect(424+i*17,440,8,2,i&1?ncGold:0x8c676b);}
}
static void nc_beam(int ax,int ay,int bx,int by,int w,U color){
 if(fastGfx||by<=ay)return;
 for(int y=ay;y<=by;y++){int d=y-ay,len=by-ay,k=2+d*w/len,cx=ax+(bx-ax)*d/len;
  for(int x=cx-k;x<=cx+k;x++){
   U*p0=pixels+y*W+x;int weight=absf(x-cx)<k*.58f&&d<len*3/4?2:1;
   *p0=mix(*p0,color,weight);}
 }
 nc_oval(bx,by,w,3,mix(0x312837,color,3));
}
static void nc_dancer(int x,int y,int i,int act){
 U coats[]={0xb75c89,0x589a9e,0xa584bf,0xc28858,0x7589b8};
 U shadow=0x261c2a,c=coats[i%5],light=mix(c,PAPER,3),skin= i&1?0xae7e69:0xe0a38b;
 float t=anim*(3.2f+nc_music()*.4f)+i*1.8f;
 int sway=(int)(sn(t)*3),lift=(int)(cs(t)*4),lean=(int)(sn(t*.51f)*3);
 int hip=x+sway,foot=y,should=hip+lean,head=should+lean/2;
 nc_oval(x,y+2,13,3,0x382534);
 /* Fully clothed stage costume: long trousers, tailored jacket, gold lapels and cuffs. */
 for(int q=-1;q<=1;q++){
  line(hip-2+q,y-11,x-5-lift+q,y,0x352c41);line(hip+2+q,y-11,x+6+lift+q,y-1,0x45384b);
  line(should-4+q,y-23,should-10+q,y-17-lift,light);line(should-10+q,y-17-lift,should-13+q,y-25-lift,c);
  line(should+4+q,y-23,should+10+q,y-24+lift,light);line(should+10+q,y-24+lift,should+14+q,y-29+lift,c);
 }
 rect(x-8-lift,foot,7,2,ncGold);rect(x+5+lift,foot-1,7,2,ncGold);
 tri(should-6,y-24,should+6,y-24,hip-4,y-8,c);tri(should+6,y-24,hip+5,y-9,hip-4,y-8,c);
 line(should-3,y-24,hip+1,y-16,ncGold);line(should+3,y-24,hip+1,y-16,ncGold);
 rect(hip-4,y-12,9,2,shadow);dot(hip,y-12,ncGold);
 disc(should-13,y-25-lift,2,skin);disc(should+14,y-29+lift,2,skin);
 rect(head-1,y-26,3,4,skin);disc(head,y-29,4,skin);rect(head-4,y-33,8,3,shadow);
 if(i%2){rect(head-6,y-32,12,2,light);rect(head-3,y-35,7,4,c);}
 else{disc(head-3,y-33,3,shadow);dot(head-5,y-35,ncGold);}
 if(act==2)line(should-4,y-21,hip-8,y-10,light);
}
static void nc_player(void){
 int x=(int)ncX,y=(int)ncY;
 nc_oval(x,y+2,8,3,0x1b1924);person(x,y,0x56baae,walking?(int)(sn(footcycle)*4):0,ncAngle,4,1);
 /* Small open bracket below feet: identify the player even on a busy floor. */
 line(x-7,y+5,x-3,y+7,ncIce);line(x-3,y+7,x+3,y+7,ncIce);line(x+3,y+7,x+7,y+5,ncIce);
}
static void nc_dynamics(void){
 int track=nc_music(),act=nc_get(NC_ACT),level=nc_get(NC_DECOR);
 if(ncRoom==NC_FLOOR){
  U left=track==0?0xe79dad:track==1?0xefd388:0xbcacec,
    right=track==0?0x75c3c6:track==1?0x84d2a6:0x8ac6e7;
  nc_beam(312,126,391+(int)(sn(anim*.71f)*41),226,40,left);
  nc_beam(645,126,563+(int)(sn(anim*.83f+1.4f)*43),225,37,right);
  if(level==2)nc_beam(481,117,481+(int)(sn(anim*.48f)*83),245,48,0xcfacdf);
  for(int j=0;j<2;j++){int xx=j?645:312;rect(xx-8,119,16,7,0x302a3c);rect(xx-5,126,10,2,j?right:left);}
  /* Downstage after reflected light to keep silhouettes + upper bodies readable. */
  int n=3+act;for(int i=0;i<n;i++){int xx=480+(i*2-n+1)*28;nc_dancer(xx,201-(i&1)*5,i,act);}
  for(int i=0;i<6;i++){int xx=141+i*6,h=2+(int)((sn(anim*(3+i*.2f)+i)+1)*4);rect(xx,207-h,3,h,i<4?ncIce:ncGold);}
  int guests=nc_owned()?cl(8+nc_get(NC_DOOR)/4,8,16):12;
  float beat=nc_beat();
  if(!fastGfx)for(int i=0;i<12;i++){int xx=313+i%6*61,yy=281+(i/6)*52;
   U color=mix(0x342e43,(i&1)?right:left,(int)((sn(anim*2.1f+i*1.18f)+1)*1.49f)+1);
   nc_rhomb(xx,yy,16,7,color);}
  /* Ordered by depth; density index grows the audience in little dance circles
     rather than filling rigid rows from front to back. */
  const short crowd[16][3]={{391,264,5},{554,269,0},{620,275,14},{374,283,1},
    {423,286,9},{529,294,7},{585,299,2},{324,306,11},{470,311,12},{608,320,8},
    {344,327,3},{446,335,6},{513,339,13},{359,348,10},{567,351,4},{645,357,15}};
  int plotted=0;
  for(int i=0;i<16;i++){
   int idx=crowd[i][2];if(idx>=guests)continue;
   int yy=crowd[i][1]+(int)(sn(anim*1.9f+idx)*2),xx=crowd[i][0]+(int)(sn(anim*.83f+idx*4.2f)*7);
   if(!plotted&&ncY<=yy){nc_player();plotted=1;}
   person(xx,yy,idx%3==0?0xb77f9d:idx%3==1?0x6a999e:0x938ba9,
          (int)(sn(anim*3+idx*1.7f)*4),1.5f+sn(anim*.5f+idx)*1.7f,0,idx);
  }
  if(!plotted)nc_player();
  person(159,201,0x82718a,0,1.4f,3,1);person(795,203,0x61958e,0,1.25f,0,2);
  for(int i=0;i<1+nc_get(NC_STAFF);i++){int xx=584+i*19;person(xx,378+i%2*5,0x4a5468,0,2.4f-i*.4f,1,3+i);}
  if(nc_get(NC_DISPUTE)){
   nc_label(599,339,"QUEUE  /  !",ncRose);
   for(int i=0;i<nc_get(NC_DISPUTE)+1;i++){
    int yy=393+i%2*9,xx=615+i*15+(int)(sn(anim*3.7f+i)*4);
    person(xx,yy,0x986e82,(int)(sn(anim*4.2f+i)*2),2.2f,0,i);
   }
  }
  else nc_label(603,382,"DOOR STAFF",0xd4bf9a);
  for(int i=0;i<6;i++){int xx=323+i*62;rect(xx,221,7,2,beat>0?left:ncGold);}
  if(nc_owned()&&nc_get(NC_CLEAN)<48)for(int i=0;i<5;i++)rect(290+i*76,373-(i%2)*11,4,2,0xa09895);
 }else if(ncRoom==NC_SUITE){
  for(int i=0;i<3;i++){
   int cx=210+i*266;U ink=ncBoothInk[i];
   for(int j=0;j<2;j++){
    int xx=cx+(j?49:-49)+(int)(sn(anim*.7f+i+j)*2);
    person(xx,210+(int)(cs(anim*1.6f+j)*1.7f),j?mix(ink,0x4d3e55,4):mix(ink,0x37545e,5),
       (int)(sn(anim*1.7f+i+j)*2),(j?2.3f:.7f)+sn(anim*.8f+j+i)*.25f,0,j+i);
   }
   for(int j=0;j<2;j++){
    int xx=cx+24+(j?-48:0),y=223;
    nc_oval(xx,y+3,4,2,ncGold);rect(xx,y-1,1,5,ncGold);
    nc_oval(xx,y-4,4,3,nc_get(NC_TABLES)&(1<<i)?ink:0x988b8b);
   }
   if(nc_get(NC_SHOWCASE)&(1<<i)){nc_rhomb(cx,210,6,6,ink);nc_oval(cx,212,5,3,mix(ink,ncGold,4));}
   if(!fastGfx){
    int k=1+(int)((sn(anim*1.4f+i)+1)*1.4f);
    line(cx-68,182,cx+68,182,mix(0x5c4659,ink,k));disc(cx-68,182,2,ink);disc(cx+68,182,2,ink);
   }
  }
  for(int i=0,k=0;i<4;i++)for(int j=0;j<(i<3?nc_typed(i):nc_legacy_packs());j++,k++){int xx=752+k*15;
   box(xx,340,11,6,i<3?mix(ncBoothInk[i],0x35424b,4):0x5a7d7b,ncGold);}

  nc_player();
 }else{
  person(540,181,0x708889,0,1.9f,0,4);nc_player();
  int val=nc_get(NC_TILL),fill=min(4,val/32);for(int i=0;i<fill;i++)box(405+i*5,181-i*2,15,4,0x6e947f,ncGold);
 }
}
static void nc_st_line(char*b,const char*start,int val,const char*end){char*q=append(b,start);q=number(q,val);append(q,end);}
static void nc_draw_status(void){
 int x=42;U accent=ncRoom==NC_FLOOR?ncRose:ncRoom==NC_SUITE?ncGold:ncIce;
 rect(0,0,W,78,0x141420);rect(0,76,W,1,0x6b515e);
 textat(x,6,"PORT VANTA   /   STRIP CLUB & CABARET",ncGold,0);
 textat(x,24,"The Afterimage",0xffe3c8,1);
 int tx=669;char cash[64],clock[6];
 day_clock_label(clock);nc_st_line(cash,"CASH  $",sv.cash,"");
 textat(tx,10,ncRoom==NC_FLOOR?"01   /   PERFORMANCE HALL":ncRoom==NC_SUITE?"02   /   VELVET LOUNGE":"03   /   BOX OFFICE",accent,0);
 textat(tx,37,cash,PAPER,0);textat(877,37,clock,ncDim,0);
 /* A narrow marquee, separate from the architecture. */
 char marquee[95];char*q=append(marquee,nc_headline()?"HEADLINE SET":"MATINEE");q=append(q,"   /   ");q=append(q,ncActs[nc_get(NC_ACT)]);q=append(q,"    *    ON DECK: ");append(q,ncTracks[nc_music()]);
 int mw=textw(marquee,0);rect(470-mw/2,82,mw+20,22,0x201b2a);center(480,82,marquee,0xe1c1b5,0);
 int t=nc_target();rect(0,455,W,85,0x161622);rect(0,455,W,1,0x715764);
 box(35,466,30,24,0x342a3b,t?accent:0x66596b);center(50,467,"E",t?PAPER:ncDim,0);
 textat(76,467,nc_hint(t),PAPER,0);
 char meter[115];if(nc_owned()){q=append(meter,"REP ");q=number(q,nc_get(NC_REP));q=append(q,"   /   TILL $");q=number(q,nc_get(NC_TILL));q=append(q,"   /   SET ");q=number(q,(NC_SHOW-nc_get(NC_CLOCK)+59)/60);append(q,"s");}
 else append(meter,"PUBLIC ENTRY   /   DEED AVAILABLE");
 textat(925-textw(meter,0),468,meter,accent,0);
 if(p.noticeT>0)textat(36,507,notice,notice_color(),0);
 else textat(36,507,"WASD / ARROWS  MOVE      SHIFT  RUN      E  INTERACT      ESC  PAUSE",ncDim,0);
}
static const char*nc_panel_title(void){
 switch(ncPanel){case NC_DESK:return "Box office";case NC_DECK:return "Selector's booth";
 case NC_STOCK:return "Stock cabinet";case NC_TABLE:return ncBooths[ncBooth];case NC_BAR:return "The house bar";
 case NC_GUARD:return "Door staff";case NC_DANCE_CONFIRM:return "Private cabaret";default:return "Stage cue";}
}
static const char*nc_title_row(int r,char out[90]){
 static const char*const mgmt[]={"Collect till","Set door price","Book residency","Hire door staff","Order drinks","Interior fit-out","Exchange arcade token","Reset house crew","Close ledger"};
 if(ncPanel==NC_DANCE_CONFIRM)return r?"Confirm and pay $25":"Back without paying";
 if(r==nc_items()-1)return "Back to the room";
 switch(ncPanel){
 case NC_DESK:
  if(!nc_owned())return "Purchase deed  /  $480";
  switch(r){
  case 0:nc_st_line(out,"Collect till  /  $",nc_get(NC_TILL),"");return out;
  case 1:append(append(out,"Door  /  "),ncPrices[nc_get(NC_COVER)]);return out;
  case 2:return nc_get(NC_ACT)==0?"Book Prism Quartet  /  $55":nc_get(NC_ACT)==1?"Book Nightbirds  /  $95":"Nightbirds  /  booked";
  case 3:return nc_get(NC_STAFF)==0?"Hire first guard  /  $60":nc_get(NC_STAFF)==1?"Hire second guard  /  $90":"Door staff  /  full";
  case 4:if(nc_get(NC_DRINK)>=12)return "Drinks  /  fully stocked";nc_st_line(out,"Restock drinks  /  $",min(4,12-nc_get(NC_DRINK))*3,"");return out;
  case 5:return nc_get(NC_DECOR)==0?"Prism mirrors  /  $95":nc_get(NC_DECOR)==1?"Aurora canopy  /  $170":"Interior  /  complete";
  case 7:return "Reset house crew  /  $16";default:return mgmt[r];}
 case NC_DECK: append(append(out,r==nc_music()?"ON AIR  /  ":"PLAY  /  "),ncTracks[r]);return out;
 case NC_TABLE:return r<3?ncDrinks[r]:r==(nc_owned()?4:3)?"VIP lap dance  /  $25":"Serve the reserved Top Shelf";
 case NC_STOCK:
  if(!r)return "Refill house drinks";
  append(append(out,"Stock Top Shelf  /  "),ncSignatures[r-1]);return out;
 case NC_BAR:return r?"Order stock for booth":"House soda  /  $5";
 case NC_CUE:return "Cue the encore";
 case NC_GUARD:return nc_items()==4?ncRemedies[r]:"Brief the crew";
 }return "";
}
static void nc_stat_row(int x,int y,const char*label,int val,U color){
 textat(x,y,label,ncDim,0);char b[20];number(b,val);textat(x+158-textw(b,0),y,b,color,0);
 box(x,y+20,158,4,0x262532,0x4d4350);rect(x+1,y+21,156*val/100,2,color);
}
static void nc_panel_draw(void){
 const int x=106,y=44,rx=548,ty=148;const U color=ncPanel==NC_DECK?ncIce:ncPanel==NC_TABLE?ncRose:ncGold;
 modal(x,y,748,431,color);rect(x+2,y+5,744,80,0x292432);
 textat(x+25,y+9,"THE AFTERIMAGE   /   HOUSE CONSOLE",color,0);textat(x+25,y+29,nc_panel_title(),PAPER,1);
 textat(x+26,118,"ACTION LIST",ncDim,0);rect(x+25,140,397,1,0x5e4b58);
 int n=nc_items();
 for(int i=0;i<n;i++){
  int yy=ty+i*30;int chosen=i==ncChoice;char b[90],num[6];number(num,i+1);
  box(x+24,yy,400,26,chosen?0x413346:0x231f2d,chosen?color:0x433847);
  textat(x+35,yy+2,num,chosen?color:ncDim,0);textat(x+62,yy+2,nc_title_row(i,b),chosen?PAPER:ncDim,0);
 }
 rect(rx-12,144,1,266,0x524551);
 char row[100];textat(rx,118,"HOUSE NOTES",color,0);
 if(ncPanel==NC_DESK&&nc_owned()){
  nc_stat_row(rx,148,"REPUTATION",nc_get(NC_REP),ncRose);
  nc_stat_row(rx,181,"CLEANLINESS",nc_get(NC_CLEAN),ncIce);
  nc_stat_row(rx,214,"SERVICE",nc_get(NC_SERVICE),ncGold);
  nc_st_line(row,"Last set:  $",nc_get(NC_LAST),"");textat(rx,249,row,PAPER,0);
  nc_st_line(row,"Guests ",nc_get(NC_DOOR),"  |  Tickets $");number(row+sl(row),nc_get(NC_LAST_DOOR));textat(rx,269,row,ncDim,0);
  nc_st_line(row,"Bar $",nc_get(NC_LAST_DRINKS),"  |  VIP $");number(row+sl(row),nc_get(NC_LAST_VIP));textat(rx,289,row,ncDim,0);
  nc_st_line(row,"Stage $",nc_get(NC_LAST_AUX),"  |  Guards -$");number(row+sl(row),nc_get(NC_LAST_COST));textat(rx,309,row,ncDim,0);
  nc_st_line(row,"Drinks ",nc_get(NC_DRINK),"/12   |   Close ");number(row+sl(row),(NC_SHOW-nc_get(NC_CLOCK)+59)/60);append(row+sl(row),"s");textat(rx,329,row,ncGold,0);
  const char*const help[]={"Last set excludes personal tips and pending exchanges. Collect before $9,999.",
  "Cycle prices for next set. Premium loses fewer guests at 65+ rep.",
  "A residency adds performers and attracts a larger crowd at every set.",
  "Prevents queue trouble. Each guard costs $4 per completed set.",
  "A stocked fridge lifts service. Each guest order uses one drink.",
  "New mirrors and a canopy change the room and pull a larger crowd.",
  "40 marked credits = $30 token. Clears on a closing with room in the till and no case.",
  "Restores cleanliness and service. For free cleaning, visit the lounge cart.",
  "Booked acts and takings stay with this career."};
  nav_wrap(rx,352,280,help[cl(ncChoice,0,8)],ncDim,4);
 }else if(ncPanel==NC_DESK){
  nc_sun(rx+76,212,42,ncGold);nav_wrap(rx,234,280,"Acquire the performance hall, velvet lounge and box office with clean cash.",PAPER,4);
  nav_wrap(rx,314,280,"Every 45 seconds of active city time, a set closes and the door takings reach your till.",ncDim,4);
 }else if(ncPanel==NC_DECK){
  for(int i=0;i<15;i++){int h=6+(int)((sn(anim*(3.1f+i*.05f)+i)+1)*22);rect(rx+i*15,216-h,9,h,i<9?ncIce:ncGold);}
  nav_wrap(rx,237,280,"Choose a house record. Different tempos change the light rig, dancers and synth music.",PAPER,4);
  append(append(row,"Audience request: "),ncTracks[nc_get(NC_SHOWS)%3]);nav_wrap(rx,326,280,row,ncGold,4);
 }else if(ncPanel==NC_TABLE){
  int want=nc_guest_order(ncBooth);U cup=want==0?ncRose:want==1?ncIce:ncGold;
  nc_oval(rx+46,222,26,4,0x665363);rect(rx+44,192,4,28,cup);tri(rx+18,171,rx+76,171,rx+46,201,cup);line(rx+23,173,rx+67,173,PAPER);
  textat(rx,239,"GUEST ORDER",ncDim,0);textat(rx,262,ncDrinks[want],cup,0);
  if(nc_owned()){
   int design=night_club_signature_demand(ncBooth),bit=1<<ncBooth;
   textat(rx,295,"TOP SHELF REQUEST",ncDim,0);
   append(append(row,ncSignatures[design])," / A");textat(rx,319,row,ncGold,0);
   nc_st_line(row,"In cabinet ",nc_typed(design),"   |   +$");number(row+sl(row),ncSignaturePay[design]);textat(rx,341,row,ncIce,0);
   textat(rx,365,nc_get(NC_SHOWCASE)&bit?"TOP SHELF SERVED":nc_get(NC_TABLES)&bit?"DRINK MATCHED":"AWAITING SERVICE",ncGold,0);
   textat(rx,390,"Drinks first, then Top Shelf.",ncDim,0);
  }else nav_wrap(rx,302,280,"Match the order card to the house menu. Owned booths turn good service into tips.",PAPER,4);
 }else if(ncPanel==NC_STOCK){
  nc_st_line(row,"Drinks ",nc_get(NC_DRINK),"/12   |   Packs ");number(row+sl(row),nc_get(NC_VIP));append(row+sl(row),"/8");textat(rx,151,row,PAPER,0);
  textat(rx,180,"TOP SHELF      CLUB / SATCHEL",ncDim,0);
  for(int i=0;i<3;i++){
   textat(rx,204+26*i,ncSignatures[i],ncGold,0);
   nc_st_line(row,"",nc_typed(i)," / ");number(row+sl(row),night_phone_signature(i,0));textat(rx+187,204+26*i,row,ncIce,0);
  }
  if(nc_legacy_packs()){nc_st_line(row,"Legacy reserve: ",nc_legacy_packs(),"");textat(rx,288,row,ncDim,0);}
  nav_wrap(rx,318,280,"Stock grade A from Signal. Booths request exact strains, served at closing or with a matched drink.",PAPER,4);
 }else if(ncPanel==NC_BAR){
  nc_st_line(row,"House drinks  /  ",nc_get(NC_DRINK)," of 12");textat(rx,159,row,PAPER,0);
  nav_wrap(rx,222,280,"A house soda restores up to six health. Owners can order up to four booth servings for $3 apiece.",PAPER,4);
 }else if(ncPanel==NC_DANCE_CONFIRM){
  nc_sun(rx+88,209,45,ncBoothInk[ncBooth]);
  nav_wrap(rx,240,280,"Book a short, private costumed cabaret performance at this booth for $25.",PAPER,4);
  nav_wrap(rx,323,280,"Leave anytime with Esc. The fee is not refunded once booked.",ncDim,4);
 }else if(ncPanel==NC_CUE){
  float beat=nc_beat();int w=(int)((beat+1)*102);
  box(rx,190,206,18,0x2a2635,0x776272);rect(rx+1,191,w,16,beat>.28f?ncGold:0x806a85);
  rect(rx+131,184,2,31,PAPER);textat(rx,224,beat>.28f?"CUE NOW":"WAIT FOR GOLD",beat>.28f?ncGold:ncDim,0);
  nav_wrap(rx,264,280,"Cue on the downbeat. One good encore per set boosts the crowd and box-office take.",PAPER,4);
 }else{
  const char*const trouble[]={"Brief the crew once per set to prevent a new incident. Hire a full team at the ledger.",
  "Two groups have tickets for the same booth. Verify the ledger and seat the overflow group.",
  "Guests are crossing the entrance rope. The steward can reset the line safely.",
  "The back hallway is obstructed. Ask the shift staff to clear the passage."};
  int incident=nc_get(NC_DISPUTE);
  textat(rx,159,ncIncidents[incident],incident?ncRose:ncIce,0);
  nav_wrap(rx,212,280,trouble[incident],PAPER,4);
  if(incident)nav_wrap(rx,323,280,"Ignoring this cuts turnout, earnings and reputation at set close.",ncDim,4);
 }
 rect(x+24,432,699,1,0x574651);textat(x+26,440,"ARROWS  SELECT       E / ENTER  CHOOSE       ESC  BACK",ncDim,0);
 if(p.noticeT>0){rect(0,496,W,44,0x151520);textat(36,507,notice,notice_color(),0);}
}
/* An optional costumed cabaret vignette. The player's avatar remains seated
   behind the illuminated dance lane; the performer has a separate silhouette. */
static void nc_dance_performer(int x,int y,float t,U color){
 U edge=0x221a2b,shade=mix(color,edge,4),rim=mix(color,PAPER,3),skin=0xcf9b85;
 int rhythm=(int)(sn(t)*10),lift=(int)(cs(t)*5),head=(int)(sn(t*.58f)*7);
 Pt lhip={x-6,y-32},rhip={x+7,y-32},la={x-12-rhythm,y-3+lift},ra={x+11+rhythm,y-2-lift};
 Pt ls={x-13+head,y-61},rs={x+13+head,y-62},le={ls.x-20,y-53+lift},re={rs.x+20,y-60-lift};
 Pt lh={le.x-11,y-68+rhythm/2},rh={re.x+10,y-79-rhythm/2};
 nc_oval(x,y+1,33,5,0x271e32);
 rod(lhip,la,4,edge);rod(rhip,ra,4,edge);rod(lhip,la,2,shade);rod(rhip,ra,2,shade);
 line(la.x-7,la.y+3,la.x+4,la.y+3,ncGold);line(ra.x-4,ra.y+3,ra.x+8,ra.y+3,ncGold);
 rod(ls,le,4,shade);rod(le,lh,3,color);rod(rs,re,4,color);rod(re,rh,3,shade);
 disc(lh.x,lh.y,3,skin);disc(rh.x,rh.y,3,skin);
 quad(ls,rs,(Pt){x+9,y-28},(Pt){x-9,y-28},color);
 line(ls.x+3,ls.y+1,x+1,y-43,rim);line(rs.x-3,rs.y+1,x+1,y-43,ncGold);
 rect(x-9,y-34,19,3,edge);dot(x,y-33,ncGold);line(x-6,y-29,x+8,y-29,rim);
 disc(x+head,y-70,7,skin);rect(x+head-8,y-77,16,4,edge);
 rect(x+head-11,y-75,22,2,shade);rect(x+head-6,y-83,12,7,color);
 rect(x+head-6,y-79,12,2,ncGold);
 dot(x+head-2,y-71,edge);dot(x+head+3,y-71,edge);
}
static void nc_dance_paint(void){
 U ink=ncBoothInk[ncBooth],cloth=ncBoothUph[ncBooth],frame=0x1e1828;
 float elapsed=ncDanceTicks/60.f,t=elapsed*(nc_music()==0?4.8f:nc_music()==1?5.6f:4.1f);
 modal(76,74,808,391,ink);
 box(91,145,778,251,frame,0x886c83);box(97,152,766,238,frame,0x564356);
 for(int y=153;y<387;y++)rect(99,y,762,1,mix(0x332438,frame,(y-153)*7/234));
 for(int i=0;i<15;i++){
  int x=154+i*45;rect(x,159,3,110,mix(cloth,frame,i%3+2));rect(x+4,159,1,110,mix(cloth,ink,2));
  if(i%3==0)nc_torch(x,190,ink);
 }
 for(int i=0;i<5;i++)nc_rhomb(498,211,128-18*i,50-7*i,mix(0x4a3653,ink,1+i%3));
 nc_sun(498,219,38,ink);
 line(108,283,851,283,0x99778b);line(108,286,851,286,0x554252);
 for(int i=0;i<8;i++)line(177+i*87,287,116+i*103,382,0x4c3a50);
 for(int y=295;y<385;y+=15)line(108,y,851,y,0x48384b);
 nc_oval(471,349,198,28,0x402c42);nc_oval(471,346,188,23,0x6e5168);nc_oval(471,345,184,20,0x513b53);
 for(int i=0;i<10;i++)nc_rhomb(318+i*33,344,11,6,0x806479);
 nc_beam(420,158,416+(int)(sn(t*.21f)*23),335,70,mix(ink,cloth,3));
 nc_beam(570,158,497+(int)(sn(t*.17f)*32),328,58,0xa88c9e);
 /* Side table and viewing chair, separate from the performer's lane. */
 nc_sofa(573,291,91,cloth);nc_oval(699,322,24,6,0x49384b);nc_oval(699,318,22,5,ncGold);
 nc_bottle(699,307,0x6d8f8e);
 int hx=612,hy=293;
 U jacket=charTop[sv.look.part[TOP_TONE]],skin=charSkin[sv.look.part[SKIN]],hair=charHair[sv.look.part[HAIR_TONE]];
 rod((Pt){hx-4,hy+16},(Pt){hx-15,hy+30},4,0x3a374a);
 rod((Pt){hx+5,hy+16},(Pt){hx+1,hy+32},4,0x3a374a);
 box(hx-12,hy-2,23,22,mix(jacket,frame,2),mix(jacket,PAPER,3));
 rod((Pt){hx-11,hy+1},(Pt){hx-16,hy+14},2,jacket);rod((Pt){hx+10,hy+1},(Pt){hx+12,hy+14},2,jacket);
 disc(hx-2,hy-7,8,hair);disc(hx-3,hy-5,6,skin);rect(hx-9,hy-12,13,3,hair);dot(hx-7,hy-5,frame);
 int x=429+(int)(sn(t*.59f)*46),y=337+(int)(sn(t*.37f)*7);
 nc_dance_performer(x,y,t,mix(cloth,ink,5));
 rect(94,84,768,53,0x292334);
 textat(108,88,"THE AFTERIMAGE    /    PRIVATE PERFORMANCE",ink,0);
 char title[72];char*q=append(title,ncBooths[ncBooth]);append(q," booth  /  Lap dance");textat(108,111,title,PAPER,0);
 box(108,416,273,10,0x211e2c,0x66566c);
 rect(110,418,269*cl(NC_DANCE_DURATION-ncDanceTicks,0,NC_DANCE_DURATION)/NC_DANCE_DURATION,6,ink);
 textat(108,433,"PRIVATE CABARET   /   ONE SHORT SET",ncDim,0);
 box(655,407,198,39,0x36293d,ink);center(754,416,"ESC  /  LEAVE",PAPER,0);
}
static void nc_end_dance(void){ncDanceTicks=0;nc_showpanel(NC_TABLE);}
static int nc_showcase_skip(int x,int y){return x>=655&&x<853&&y>=407&&y<446;}
static void night_club_pointer(int sx,int sy,U clicks){
 if(scene!=NIGHT_CLUB_SCENE||menu>=0||!ncPanel||!clicks)return;
 if(ncPanel==NC_DANCE_PLAY){if((clicks&2)||((clicks&1)&&nc_showcase_skip(sx,sy)))nc_end_dance();return;}
 if(clicks&2){if(ncPanel==NC_DANCE_CONFIRM)nc_showpanel(NC_TABLE);else ncPanel=0;return;}
 if((clicks&1)&&sx>=130&&sx<530&&sy>=148){
  int row=(sy-148)/30;if(row<nc_items()&&(sy-148)%30<26){ncChoice=row;nc_activate(row);}
 }
}
static void night_club_tick(U keys,U tap){
 if(walkin_active!=NC_ID&&(ncFrontId<0||walkin_active!=ncFrontId)){scene=1;menu=-1;return;}
 if(tap&HELP){helpBack=NIGHT_CLUB_SCENE;scene=2;return;}if(tap&GFX)fastGfx=!fastGfx;
 if(p.noticeT>0)p.noticeT-=1.f/60;
 if(ncPanel==NC_DANCE_PLAY){
  if((tap&(ESC|USE|ENTER))||++ncDanceTicks>=NC_DANCE_DURATION)nc_end_dance();
  return;
 }
 if(tap&ESC){
  if(menu>=0){menu=-1;return;}
  if(ncPanel==NC_DANCE_CONFIRM){nc_showpanel(NC_TABLE);return;}
  if(ncPanel){ncPanel=0;return;}
  active(PAUSE_MENU);makeMenu();return;
 }
 if(menu>=0){makeMenu();if(!nopt){menu=-1;return;}if(tap&UP)sel=(sel+nopt-1)%nopt;if(tap&DOWN)sel=(sel+1)%nopt;if(tap&(USE|ENTER))doOpt(opt[sel].action);return;}
 p.secs+=1.f/60;job_clock_tick(1.f/60);
 if(ncPanel){
  int n=nc_items();if(!n){ncPanel=0;return;}
  if(tap&UP)ncChoice=(ncChoice+n-1)%n;if(tap&DOWN)ncChoice=(ncChoice+1)%n;
  if(tap&(USE|ENTER))nc_activate(ncChoice);return;
 }
 int dx=!!(keys&RIGHT)-!!(keys&LEFT),dy=!!(keys&DOWN)-!!(keys&UP);
 float v=(keys&BRAKE?WALKIN_RUN_STEP:WALKIN_WALK_STEP)*(dx&&dy?.707107f:1.f);walking=dx||dy;
 if(walking){ncAngle=look(dx,dy);footcycle=wrap(footcycle+.25f*FOOT_PACE);}
 if(!nc_solid(ncX+dx*v,ncY))ncX+=dx*v;if(!nc_solid(ncX,ncY+dy*v))ncY+=dy*v;
 if(tap&(USE|ENTER))nc_use_target(nc_target());
}
static void night_club_paint(void){
 int key=ncRoom*3+nc_get(NC_DECOR);
 if(ncCache!=key){U*old=surface;surface=ncBackdrop;nc_static_floor(ncRoom,nc_get(NC_DECOR));surface=old;ncCache=key;}
 /* The two aligned arrays are disjoint. A simple word copy vectorizes on both targets. */
 for(int i=0;i<W*H;i++)pixels[i]=ncBackdrop[i];
 nc_dynamics();nc_draw_status();
 if(ncPanel==NC_DANCE_PLAY)nc_dance_paint();else if(ncPanel)nc_panel_draw();
 if(menu>=0&&scene!=2)paint_active_menu();
}
