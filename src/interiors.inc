/* Port Vanta's doors. Room coordinates are separate from p.x/p.y, which remain
   valid street/save coordinates even when an NPC opens its existing menu. */
enum { INT_DESK,INT_RACK,INT_BED,INT_SOFA,INT_CRATES,INT_PLANT,INT_TRAY,INT_LAB,
       INT_CAR,INT_GUNCASE,INT_CLINIC,INT_DINER,INT_LOCKER,INT_CONSOLE,INT_REEL,
       INT_FERNS,INT_GROWBED,INT_GLASS,INT_WALL,INT_BARS,INT_SETTLE,INT_SORTER,INT_COOLER,INT_TOOLS,INT_FEATURE,INT_GROW_STOCK };
typedef struct{int x,y,w,h,type;U tint;} InteriorProp;
typedef struct{
 const char *banner,*line,*instruction,*counter,*door;
 U floor,wall,neon;
 int finish;
} InteriorTheme;
static const InteriorTheme int_themes[16]={
 {"MERCURY / NIGHT DESK","TIDE MARKET / 24-HOUR COURIER","Packages, fares and pay slips. Take a shift with Mara.","DISPATCH COUNTER","MRC",0x384644,0x263c49,0x82e5cb,0},
 {"VAULT 39 / LOWER LEVEL","TIDE MARKET / MEMBERS' LOUNGE","Ivo is taking meetings past the low couches.","IVO'S BOOTH","V39",0x393747,0x3b304e,0xf596c5,2},
 {"FERRY MARKET / TILL 02","TIDE MARKET / IMPORTS & PROVISIONS","Find June at the till for supplies and medical patches.","MARKET TILL","FRY",0x514332,0x3c4544,0xf3c97b,1},
 {"NIGHTGLASS / SHOW FLOOR","NIGHTGLASS / MOTORS & SERVICE","Shop the floor, then see Lex at the sales desk.","MOTOR SALES","NGM",0x303e5a,0x25334e,0x8fcdf5,1},
 {"LANTERN / HOUSEPLANTS","LANTERN WARD / FERN HOUSE","Talk to Nell at the register, or use the green door to the grow room.","NELL / REGISTER","LANTERN",0x6b5540,0x3b5748,0xcde7a8,0},
 {"CINDER / PRODUCTION BAY","BREAKWATER / PRIVATE PROPERTY","Kite manages the lab from the service station.","LAB TERMINAL","CND",0x3d4557,0x383b56,0xc0afff,1},
 {"GLASS MILE / RACE CONTROL","GLASS MILE / LICENSED MOTORSPORT","Talk to Paz at race control to register for a sprint.","RACE CONTROL","GMC",0x39434f,0x2d4759,0x8fe9ff,1},
 {"KIOSK 09 / NIGHT WINDOW","EAST FREIGHT / AFTER-HOURS EXCHANGE","Vera works behind the window. Bring stock to the counter.","NIGHT WINDOW","K09",0x4a3740,0x453449,0xef91c9,2},
 {"TIDEWAY / WALK-IN CLINIC","LANTERN WARD / NIGHT SERVICE","Meet Sol at triage for treatment or protective gear.","TRIAGE DESK","MED",0x455758,0x33515b,0xfbdf9e,1},
 {"BREAKWATER / SAFEHOUSE","BREAKWATER / PRIVATE PROPERTY","See Reed for the lease, then use this place to recover.","HOUSE DESK","BKW",0x4d4242,0x3b4849,0xf4bd8a,0},
 {"WRECKER / SERVICE SHED","EAST FREIGHT / RECOVERY CREW","Find Dax at the tool bench to take a recovery job.","WORK ORDERS","TOW",0x3c4544,0x454845,0xe8b97d,1},
 {"REVENUE / TELLER 03","GLASS MILE / MUNICIPAL ANNEX","Holt is at the civil accounts counter.","CIVIL ACCOUNTS","REV",0x424f67,0x394860,0xe8c782,1},
 {0},
 {"APARTMENT 4 / WALK-UP","TIDE MARKET / YOUR APARTMENT","A note from Mara is on the little table.","READ MARA'S NOTE","A04",0x514243,0x514146,0xf5c98e,0},
 {"VANTA ARMORY / SHOWROOM","TIDE MARKET / PULSE EQUIPMENT","See Ada at the lit case for pulse equipment.","ARMORY COUNTER","ARM",0x3a4351,0x354352,0xffac91,1},
 {"VANTA POLICE / HEADQUARTERS","NIGHTGLASS / MUNICIPAL PRECINCT","Dispatch is at the front desk. The chief is in the west office.","FRONT DESK / DISPATCH","VPD / 04",0x475666,0x34475d,0x99baff,1}
};
static int interior_active_loc=-1,interior_nprop,interior_desk_x,interior_desk_y;
static int interior_cache_loc=-2,interior_cache_status,interior_fade;
static const InteriorTheme int_grow={"LANTERN / GROW ROOM","LANTERN WARD / PRODUCTION FLOOR","Start a batch at the terminal and watch the yield develop in the beds.","BATCH TERMINAL","GROW / 01",0x3c4d4c,0x343b53,0xbacff0,1};
static const InteriorTheme int_staff[]={
 {"MERCURY / MAILROOM","MERCURY DISPATCH / STAFF ONLY","Use the settlement terminal to exchange game credits or check pending funds.","MAILROOM SETTLEMENT","RECORDS / MRC",0x495a56,0x294a52,0x83e6cc,1},
 {"FERRY / RECEIVING OFFICE","FERRY MARKET / STAFF ONLY","The till in Receiving handles settlement for this franchise.","RECEIVING SETTLEMENT","RECEIVING / FRY",0x695641,0x474b4e,0xf5ca8d,0},
 {"WRECKER / SERVICE OFFICE","WRECKER YARD / STAFF ONLY","The service office has a settlement counter by the work-order board.","SERVICE SETTLEMENT","WORK ORDERS / TOW",0x4c5158,0x3d4d50,0xe9bf8c,1}
};
static const char*int_staffDoor[]={"MAILROOM","RECEIVING","SERVICE OFFICE"};
static int interior_left,interior_right,interior_subroom;
enum{GROW_RACK_X=665,GROW_RACK_Y=150,
 GROW_PICKUP_TICKS=16,GROW_CARRY_WALK_TICKS=48+80+64,GROW_PUTDOWN_TICKS=32,GROW_RETURN_WALK_TICKS=180};
static const float GROW_PACK_SECONDS=(GROW_PICKUP_TICKS+GROW_CARRY_WALK_TICKS/FOOT_PACE+GROW_PUTDOWN_TICKS)/60.f;
static const float GROW_RETURN_SECONDS=GROW_RETURN_WALK_TICKS/(60.f*FOOT_PACE);
static int grow_seen_batches,grow_seen_stock,grow_stock_before_pack;
static float grow_pack_left;
enum{GROW_GATE_X=W/2,GROW_GATE_Y=153,CHIEF_X=247,CHIEF_Y=242,CUSTODY_X=709,CUSTODY_Y=305};
static float interior_x,interior_y,interior_a,interior_cam_x,interior_cam_y;
static InteriorProp interior_props[32];
static U interior_back[W*H];
static const InteriorTheme *interior_theme(int k){
 if(interior_subroom){
  int li=legal_index_for_kind(k);
  if(li>=0&&li<LEN(int_staff))return int_staff+li;
  if(k==4)return &int_grow;
 }
 return k>=0&&k<LEN(int_themes)&&int_themes[k].banner?int_themes+k:int_themes;
}
static void int_prop(int type,int x,int y,int w,int h,U c){
 if(interior_nprop<LEN(interior_props))interior_props[interior_nprop++]=(InteriorProp){x,y,w,h,type,c};
}
static void int_counter(int type,int x,int y,int w,int h,U c){
 int_prop(type,x,y,w,h,c);interior_desk_x=x+w/2;interior_desk_y=y+h+25;
}
static int interior_kind_loc(int kind){for(int i=0;i<LEN(loc);i++)if(loc[i].kind==kind)return i;return -1;}
/* Surveyed shells, in horizontal slices. The very same stepped footprint is
   used by the floor rasterizer, roof wipe and walking collision. The final slice
   is the street vestibule. Cut-outs are outdoors, never merely darker carpet. */
typedef struct { short edge[4][2]; } VenueShell;
static const VenueShell venue_shell[]={
 {{{160,733},{160,733},{242,793},{337,740}}}, /* dispatch: parcel wing, public waiting apron */
 {{{194,746},{167,774},{167,704},{364,616}}}, /* lounge: recessed stage, sunken dance hall */
 {{{157,785},{157,785},{192,751},{386,591}}}, /* market: fish hall, entrance arcade */
 {{{203,773},{153,789},{153,789},{383,704}}}, /* motors: twin plinth gallery and delivery apron */
 {{{153,807},{153,807},{187,807},{190,790}}}, /* two-level fern conservatory */
 {{{171,786},{171,786},{316,786},{326,587}}}, /* works: control bridge to reactor wing */
 {{{350,787},{156,787},{156,704},{353,588}}}, /* circuit: timing tower, pit lane, tunnel */
 {{{334,652},{334,652},{334,597},{406,548}}}, /* kiosk: service hatch with a narrow entrance */
 {{{157,748},{157,748},{157,803},{377,803}}}, /* clinic: ward wing and arrival hall */
 {{{167,677},{167,784},{279,784},{279,565}}}, /* safehouse: bedroom, galley, den */
 {{{164,756},{164,788},{164,788},{314,731}}}, /* recovery: garage bays, mechanic's well */
 {{{259,703},{176,793},{176,793},{352,613}}}, /* revenue: bilateral records hall */
 {{{146,814},{146,814},{146,814},{146,814}}},
 {{{237,701},{237,701},{237,650},{387,591}}}, /* apartment: bedroom, galley, hall */
 {{{159,760},{159,760},{159,678},{343,677}}}, /* armory: glass-separated range and retail */
 {{{145,815},{145,815},{146,803},{166,787}}}, /* precinct: two custody cells and chief suite */
 {{{157,803},{159,802},{159,799},{387,779}}}, /* mail sorting dock */
 {{{158,796},{162,810},{168,810},{183,794}}}, /* receiving/cold storage */
 {{{159,792},{157,799},{157,799},{175,790}}}, /* service office */
 {{{160,800},{152,808},{152,808},{389,800}}}  /* grow process hall and harvest board */
};
static int venue_shell_id;
static int interior_inspection;
static int venue_band(float y){return y<216?0:y<294?1:y<370?2:3;}
static int venue_limit(float y,int side){return venue_shell[venue_shell_id].edge[venue_band(y)][side];}
typedef struct {int x,y,w,h;const char *label,*read;} Feature;
static const Feature venue_feature[16]={
 {183,158,163,47,"CITY ROUTE BOARD","Route board: shifts start at Mara's desk. The purple GPS line leads to each stop."},
 {211,156,178,53,"DJ / TURNTABLES","The DJ drops the bass. Ivo handles private contracts in the booth across the dance floor."},
 {179,161,166,49,"TODAY'S CATCH","The iced catch came off the dawn ferry. June handles purchases at the till."},
 {240,151,146,54,"ENGINE CUTAWAY","Nightglass showroom: compare the display cars, then visit Lex for sales or repairs."},
 {514,297,98,40,"POTTING BENCH","Nell's potting bench is for the public shop. Follow the green staff door to the grow beds."},
 {610,158,146,89,"REACTOR BANK","The sealed process bank hums. Batch status and upgrades are at Kite's lab terminal."},
 {576,154,171,56,"LAP TIMING","Timing tower: Paz registers drivers for sanctioned sprints at Race Control."},
 {537,155,85,48,"NIGHT HATCH","A metal hatch clicks shut. Vera handles stock trades at the night window."},
 {618,353,136,44,"VITALS STATION","The monitor finds a steady pulse. Ask Sol at triage about treatment."},
 {497,155,140,48,"HOUSE GALLEY","A kettle is still warm. Ask Reed about the safehouse lease and recovery."},
 {577,333,138,48,"RECOVERY WINCH","The yellow hoist is rigged for recovery work. Take an order from Dax."},
 {604,225,156,45,"PUBLIC RECORDS","Civil accounts: Holt can explain a case, collect a penalty or record community service."},
 {0},
 {550,154,123,48,"KITCHENETTE","The counter is bare. Mara left a note at the side table about a first shift."},
 {185,155,143,61,"TARGET LANES","The target carriage is behind safety glass. Ada handles purchases at the lit counter."},
 {582,354,42,36,"BOOKING","The booking terminal logs custody arrivals. Chief Rivas is in the west office."}
};
static int int_feature_near(int k){
 if(k<0||k>=16||interior_subroom||!venue_feature[k].label)return 0;
 const Feature*f=venue_feature+k;
 return d2(interior_x-f->x-f->w/2,interior_y-f->y-f->h-17)<32*32;
}

static void int_layout_staff(int i,U a){
 // Desk at the head of an unblocked central aisle. Furnishings can be walked around.
 int_counter(INT_SETTLE,410,195,140,51,a);
 switch(i){
 case 0:
  int_prop(INT_SORTER,185,198,182,59,0x638ca1);int_prop(INT_SORTER,186,306,175,41,0x759eaa);
  int_prop(INT_RACK,627,153,139,47,0x91acaa);int_prop(INT_LOCKER,179,154,162,26,0x6e8ca0);
  int_prop(INT_CRATES,630,270,142,50,0xb08f6f);int_prop(INT_CRATES,652,350,119,38,0x9a8168);
  break;
 case 1:
  int_prop(INT_COOLER,177,158,191,61,0x96b7af);int_prop(INT_COOLER,604,160,166,52,0xa6afb1);
  int_prop(INT_CRATES,188,274,170,43,0xac9572);int_prop(INT_RACK,195,345,163,46,0x8fa385);
  int_prop(INT_CRATES,632,299,137,58,0xa88d65);int_prop(INT_PLANT,716,377,30,30,0xb3bc86);
  break;
 case 2:
  int_prop(INT_TOOLS,185,159,184,48,0x8a99a5);int_prop(INT_LOCKER,628,151,143,48,0x718894);
  int_prop(INT_CAR,178,247,186,76,0xa8a080);int_prop(INT_TOOLS,634,249,136,56,0x928a78);
  int_prop(INT_CRATES,199,368,165,34,0x92816b);int_prop(INT_TOOLS,612,342,161,44,0x7f929a);
  break;
 }
}
static void int_layout(int k){
 interior_nprop=0;interior_left=k==13?237:146;interior_right=W-interior_left;
 const InteriorTheme*t=interior_theme(k);U a=t->neon;
 int li=legal_index_for_kind(k);venue_shell_id=interior_subroom?(li>=0?16+li:19):k;
 if(interior_subroom&&li>=0){int_layout_staff(li,a);return;}
 if(!interior_subroom&&k>=0&&k<16&&venue_feature[k].label){const Feature*f=venue_feature+k;int_prop(INT_FEATURE,f->x,f->y,f->w,f->h,a);}
 switch(k){
 case 0: /* parcel conveyor feeds the sorting window; dispatchers can reach the staff door on either side */
  int_counter(INT_DESK,399,214,162,42,a);
  int_prop(INT_SORTER,181,244,164,36,0x688c97);int_prop(INT_SOFA,250,312,92,40,0x86866c);
  int_prop(INT_CRATES,580,155,122,43,0xaf886d);int_prop(INT_LOCKER,646,228,60,48,0x718998);
  int_prop(INT_CRATES,593,321,146,49,0xaa8770);break;
 case 1: /* ground-level dance floor below a stepped stage and velvet booth */
  int_counter(INT_DINER,548,222,163,40,a);
  /* Leave a standing pocket on the booth's center line. A tall sprite
     projects north of its feet and must not extend over the couch. */
  int_prop(INT_SOFA,530,160,82,45,0x9e567d);
  int_prop(INT_SOFA,198,308,127,38,0x7e5877);int_prop(INT_REEL,204,232,57,49,0xcc8dad);
  int_prop(INT_WALL,394,152,10,58,0x826382);int_prop(INT_SOFA,556,309,114,39,0x765079);break;
 case 2: /* two aisles flanking the grocer's central walk, staff door behind */
  int_counter(INT_DESK,553,211,151,44,a);int_prop(INT_TRAY,176,237,166,34,0x9d9f6f);
  int_prop(INT_RACK,227,310,130,34,0xb8a273);int_prop(INT_COOLER,574,154,175,41,0x9ebeb2);
  int_prop(INT_CRATES,649,314,74,36,0xb38a70);break;
 case 3:
  int_counter(INT_DESK,594,202,151,42,a);int_prop(INT_CAR,195,243,160,78,0x459cbe);
  int_prop(INT_CAR,521,321,151,65,0xe69a72);int_prop(INT_TOOLS,651,150,89,36,0x92a6b0);
  int_prop(INT_GLASS,399,146,9,75,0x86afc0);break;
 case 4:
  if(!interior_subroom){
   int_counter(INT_DESK,617,228,144,43,0xbda675);
   int_prop(INT_FERNS,175,169,215,67,0x92bb75);int_prop(INT_FERNS,210,270,181,52,0x85aa72);
   int_prop(INT_FERNS,214,357,138,49,0x7bb48a);int_prop(INT_FERNS,615,159,166,46,0xacc271);
   int_prop(INT_FERNS,657,339,108,55,0x83b78b);
   int_prop(INT_PLANT,563,175,33,33,0x9fba77);int_prop(INT_PLANT,408,173,27,27,0x98bc85);
  }else{
   int_counter(INT_CONSOLE,418,186,124,46,0xa9a8d6);
   int_prop(INT_GROWBED,181,183,198,63,0x5d8082);int_prop(INT_GROWBED,581,183,198,63,0x728795);
   int_prop(INT_GROWBED,181,303,198,63,0x5f8585);int_prop(INT_GROWBED,581,303,198,63,0x79859a);
   int_prop(INT_LOCKER,183,150,121,22,0x75889f);int_prop(INT_GROW_STOCK,GROW_RACK_X,GROW_RACK_Y,114,22,0);
  }break;
 case 5:
  int_counter(INT_CONSOLE,354,209,125,42,a);int_prop(INT_LAB,190,157,100,79,0x9faed8);
  int_prop(INT_GLASS,538,145,10,115,a);int_prop(INT_GLASS,548,255,62,8,a);
  int_prop(INT_LAB,593,311,151,46,0xa38fc7);int_prop(INT_LOCKER,191,249,112,32,0x7c9ca4);
  break;
 case 6:
  int_counter(INT_DESK,403,216,154,42,a);int_prop(INT_CAR,184,237,152,67,0xe4b85f);
  int_prop(INT_TOOLS,574,250,173,30,0x7da0b0);int_prop(INT_SOFA,567,323,108,32,0x597b89);
  int_prop(INT_GLASS,555,147,8,53,a);break;
 case 7:
  int_counter(INT_DINER,361,222,147,40,a);int_prop(INT_REEL,362,158,67,45,0xc179a8);
  int_prop(INT_GLASS,517,210,11,61,a);int_prop(INT_CRATES,544,230,73,40,0xb98977);
  int_prop(INT_SOFA,354,309,90,35,0x8f738f);break;
 case 8:
  int_counter(INT_DESK,543,213,150,39,a);int_prop(INT_CLINIC,178,154,128,51,0x8ebabe);
  int_prop(INT_CLINIC,178,280,130,53,0x8ab3ba);int_prop(INT_WALL,161,237,143,9,a);
  int_prop(INT_GLASS,336,149,8,71,a);int_prop(INT_GLASS,336,281,8,66,a);
  int_prop(INT_SOFA,393,338,78,31,0x789b9e);int_prop(INT_LOCKER,582,154,120,30,0x8ca6ac);break;
 case 9:
  int_counter(INT_DESK,578,223,143,40,a);int_prop(INT_BED,185,158,131,74,0x98a0ae);
  int_prop(INT_WALL,353,149,9,106,a);int_prop(INT_WALL,171,251,110,9,a);
  int_prop(INT_SOFA,308,306,129,41,0xac8d69);int_prop(INT_PLANT,716,296,33,33,0x96b77f);
  int_prop(INT_LOCKER,610,310,83,44,0x967f81);break;
 case 10:
  int_counter(INT_CONSOLE,554,220,152,44,a);int_prop(INT_CAR,187,231,172,71,0x969a7c);
  int_prop(INT_TOOLS,184,151,152,50,0x879eaa);int_prop(INT_TOOLS,576,150,144,43,0xa08d71);
  int_prop(INT_CRATES,223,318,109,36,0xa38d6e);break;
 case 11:
  int_counter(INT_DINER,391,202,178,46,a);int_prop(INT_LOCKER,279,155,94,38,0x7a8da4);
  int_prop(INT_LOCKER,585,154,94,34,0x788da5);int_prop(INT_SOFA,193,307,121,36,0x8792a1);
  int_prop(INT_SOFA,649,311,111,33,0x8792a1);int_prop(INT_GLASS,278,221,76,8,a);
  int_prop(INT_GLASS,371,294,8,47,a);int_prop(INT_GLASS,540,316,8,46,a);break;
 case 13:
  int_counter(INT_CONSOLE,545,243,76,33,a);int_prop(INT_BED,259,156,125,76,0xa79abb);
  int_prop(INT_WALL,406,148,8,77,a);int_prop(INT_WALL,248,244,111,8,a);
  int_prop(INT_SOFA,260,311,110,40,0xae8a6b);int_prop(INT_PLANT,610,318,24,24,0x93ae7c);break;
 case 15:
  int_counter(INT_DESK,405,286,170,41,0x839dc7);
  int_prop(INT_DESK,191,180,112,37,0x777f9a);int_prop(INT_LOCKER,169,148,115,22,0x718096);
  int_prop(INT_GLASS,326,145,9,101,0x7896ad);int_prop(INT_GLASS,156,262,119,9,0x7896ad);
  int_prop(INT_DESK,366,166,101,41,0x7b91af);int_prop(INT_DESK,488,166,108,41,0x7d8aaa);
  int_prop(INT_RACK,513,230,84,24,0x8d8498);int_prop(INT_LOCKER,377,239,68,22,0x7892a3);
  int_prop(INT_WALL,624,145,11,125,0x74869c);int_prop(INT_WALL,785,145,10,125,0x74869c);
  int_prop(INT_WALL,706,147,8,122,0x74869c);int_prop(INT_BARS,635,256,150,17,0x8ea5b2);
  int_prop(INT_BED,642,166,54,26,0x79878d);int_prop(INT_BED,723,166,54,26,0x79878d);
  int_prop(INT_SOFA,177,347,133,44,0x737e99);int_prop(INT_SOFA,640,354,137,40,0x747f93);
  int_prop(INT_PLANT,352,379,27,27,0x8faf97);break;
 case 14:
  int_counter(INT_GUNCASE,524,212,169,46,a);
  int_prop(INT_GLASS,356,146,9,104,a);int_prop(INT_GLASS,163,242,132,8,a);
  int_prop(INT_LOCKER,592,150,125,39,0x8598a1);int_prop(INT_GUNCASE,187,302,144,45,0x9caeb7);
  int_prop(INT_CRATES,562,313,76,42,0x839486);break;
 default:int_counter(INT_DESK,393,214,166,43,a);break;
 }
}
/* Preserve mission and accounting notifications when stepping between rooms;
   only an exhibit caption should disappear when its exhibit does. */
static void int_expire_fixture_notice(void){
 if(interior_inspection&&p.noticeT>0){
  const char *a=notice,*b=venue_feature[interior_inspection-1].read;
  while(*a&&*a==*b){a++;b++;}
  if(*a==*b)p.noticeT=0;
 }
 interior_inspection=0;
}
static void open_interior(int n){
 if(n<0||n>=LEN(loc)||p.car>=0)return;
 int_expire_fixture_notice();interior_subroom=0;int_layout(loc[n].kind);interior_active_loc=n;interior_cache_loc=-2;
 p.x=loc[n].x;p.y=loc[n].y;walking=0;interior_x=W/2;
 interior_y=loc[n].kind==13&&sv.intro==0?307:392;interior_a=-1.570796f;
 interior_cam_x=0;interior_cam_y=0;interior_fade=17;menu=-1;mapOn=0;scene=3;
}
static int int_solid(float x,float y){
 if(y<147||y>429)return 1;
 if(x<max(venue_limit(y-4,0),venue_limit(y+4,0))+10||x>min(venue_limit(y-4,1),venue_limit(y+4,1))-10)return 1;
 for(int i=0;i<interior_nprop;i++){const InteriorProp*f=interior_props+i;
  if(x>f->x-8&&x<f->x+f->w+8&&y>f->y-7&&y<f->y+f->h+7)return 1;
 }
 return 0;
}
static int int_at_door(void){return d2(interior_x-(W/2),interior_y-428)<43*43;}
static int int_at_desk(void){return d2(interior_x-interior_desk_x,interior_y-interior_desk_y)<43*43;}
static int int_at_staff_gate(void){return !interior_subroom&&d2(interior_x-GROW_GATE_X,interior_y-GROW_GATE_Y)<32*32;}
static int int_at_grow_gate(void){return int_at_staff_gate();}
static int lantern_terminal(void){return scene==3&&interior_active_loc==4&&interior_subroom;}
static int int_at_chief(void){return d2(interior_x-CHIEF_X,interior_y-CHIEF_Y)<31*31;}
static int int_at_custody(void){return d2(interior_x-CUSTODY_X,interior_y-CUSTODY_Y)<33*33;}
static void int_switch_room(int k,int to){
 int_expire_fixture_notice();interior_subroom=to;int_layout(k);interior_cache_loc=-2;
 if(k==4){
  grow_seen_batches=sv.b[0].batches;grow_seen_stock=grow_stock_before_pack=warehouse_units(0);
  grow_pack_left=0.f;
 }
 interior_x=GROW_GATE_X;interior_y=to?393:169;
 interior_a=to?-1.570796f:1.570796f;walking=0;
 interior_cam_x=0;interior_cam_y=0;interior_fade=12;
}
static void int_leave(void){
 int_expire_fixture_notice();
 shelterT=0;
 int intro=loc[interior_active_loc].kind==13&&sv.intro==0;
 p.a=-1.570796f;scene=1;menu=-1;interior_active_loc=-1;walking=0;
 by=placeNear();near=carNear();
 if(intro){sv.intro=1;request_save();tell("FIRST SHIFT: follow the purple route to Mercury Dispatch.");}
}
static void interior_tick(U keys,U tap){
 if(interior_active_loc<0||interior_active_loc>=LEN(loc)){scene=1;menu=-1;return;}
 const float dt=1.f/60;
 if(interior_fade>0)interior_fade--;
 if(lantern_terminal()&&menu!=PAUSE_MENU){
  if(sv.b[0].batches!=grow_seen_batches){
   grow_seen_batches=sv.b[0].batches;grow_stock_before_pack=grow_seen_stock;grow_pack_left=GROW_PACK_SECONDS+GROW_RETURN_SECONDS;
  }else if(grow_pack_left>0.f)grow_pack_left=grow_pack_left>dt?grow_pack_left-dt:0.f;
  grow_seen_stock=warehouse_units(0);
 }
 if(tap&HELP){helpBack=3;scene=2;return;}
 if(tap&GFX)fastGfx=!fastGfx;
 if(tap&ESC){if(menu>=0){menu=-1;return;}active(PAUSE_MENU);makeMenu();return;}
 if(p.noticeT>0)p.noticeT-=dt;
 if(menu>=0){
  makeMenu();if(!nopt){menu=-1;return;}
  if(tap&UP)sel=(sel+nopt-1)%nopt;if(tap&DOWN)sel=(sel+1)%nopt;
  if(tap&(USE|ENTER))doOpt(opt[sel].action);
  return;
 }
 int x=!!(keys&RIGHT)-!!(keys&LEFT),y=!!(keys&DOWN)-!!(keys&UP);
 float v=(keys&BRAKE?NAMED_RUN_STEP:NAMED_WALK_STEP)*(x&&y?.707107f:1);
 walking=x||y;if(walking){footcycle=wrap(footcycle+.25f*FOOT_PACE);interior_a=look(x,y);}
 if(!int_solid(interior_x+x*v,interior_y))interior_x+=x*v;
 if(!int_solid(interior_x,interior_y+y*v))interior_y+=y*v;
 interior_cam_x+=((interior_x-W/2)*.13f-interior_cam_x)*.11f;
 interior_cam_y+=((interior_y-282)*.08f-interior_cam_y)*.11f;
 p.secs+=dt;job_clock_tick(dt);
 if(vd_frame(keys,tap))tap&=~(USE|ENTER);
 if(tap&USE){
  int k=loc[interior_active_loc].kind,li=legal_index_for_kind(k);
  if(int_at_door()){if(interior_subroom)int_switch_room(k,0);else int_leave();return;}
  if(k==4&&int_at_grow_gate()){
   if(sv.b[0].own)int_switch_room(k,1);
   else tell("Grow room locked. Buy the property at the public counter.");
   return;
  }
  if(li>=0&&int_at_staff_gate()){
   if(!sv.legit[li].owned){tell("Staff door locked. Buy the franchise at the public counter's Business Office.");return;}
   int_switch_room(k,1);return;
  }
  if(k==15&&int_at_chief()){tell("Chief Rivas: The front desk handles dispatch. Offices are open, cells are restricted.");return;}
  if(k==15&&int_at_custody()){tell("Custody wing: Two holding cells. Authorized staff only.");return;}
  if(int_feature_near(k)&&!int_at_desk()){tell(venue_feature[k].read);interior_inspection=k+1;return;}
  if(int_at_desk()){
   if(k==13){tell("Mara: Rent's due. Meet me at Mercury Dispatch for the first shift.");return;}
   if(li>=0&&interior_subroom){active(19+li);makeMenu();return;}
   if(!interior_job_drop(interior_active_loc)){active(k);makeMenu();}
  }
 }
}
static void int_lamp(int x,int y,int rad,U color,int strength){
 int r2=rad*rad;
 for(int dy=max(-rad,-y);dy<min(rad,rh-y);dy++)for(int dx=max(-rad,-x);dx<min(rad,rw-x);dx++){
  int k=(r2-dx*dx-dy*dy)*strength/r2;
  if(k>0){U*q=surface+(y+dy)*rw+x+dx;*q=mix(*q,color,min(7,k));}
 }
}
static U int_hash(int a,int b){U x=(U)a*374761393u+(U)b*668265263u;return(x^(x>>13))*1274126177u;}
static void int_solid_shape(int x,int y,int w,int h,U c){
 rect(x+4,y+8,w+5,h+1,0x192329);
 box(x,y,w,h,mix(c,0x23313c,3),0x18252c);rect(x+1,y+1,w-2,3,mix(c,PAPER,2));
 rect(x+2,y+h-5,w-4,4,mix(c,0x14212c,5));
}
static void int_monitor(int x,int y,int w,U c){
 box(x,y,w,16,0x142631,0x758f94);rect(x+2,y+2,w-4,11,0x253d4c);
 for(int i=0;i<3;i++)rect(x+4,y+3+i*3,max(2,w-11-i*3),1,mix(0x2d535c,c,4));
 rect(x+w/2-2,y+16,5,3,0x17222a);rect(x+w/2-5,y+18,11,2,0x182629);
}
static void int_pulse_tool(int x,int y,int longgun,U c){
 int length=longgun?25:13;
 rect(x-1,y-1,length+9,5,0x1c2732);rect(x,y,length+7,3,c);rect(x+3,y-2,max(6,length-3),1,mix(c,PAPER,3));
 rect(x+4,y+2,5,7,0x263e4c);rect(x+5,y+2,3,5,mix(c,0x1b2d3c,4));rect(x+length+4,y+1,3,1,0xe2ad95);
 if(longgun){rect(x-7,y+1,8,3,mix(c,0x273644,4));line(x+11,y+4,x+16,y+9,0x354b52);}
}
static void int_car(const InteriorProp*f){
 int x=f->x,y=f->y,w=f->w,h=f->h;U c=f->tint;
 box(x-3,y-3,w+6,h+6,0x263444,0x677b8c);
 for(int i=0;i<5;i++){int xx=x+i*8;line(xx,y+h-3,xx+7,y+h-3,0x9ab2ae);line(x+w-i*8,y+2,x+w-i*8-7,y+2,0x9ab2ae);}
 x+=15;y+=13;w-=30;h-=26;
 rect(x+4,y+8,w+3,h+1,0x101923);
 for(int i=0;i<2;i++)for(int j=0;j<2;j++)box(x+12+i*(w-34),y-3+j*(h-6),14,10,0x19212b,0x0c1820);
 rect(x+4,y+2,w-8,h-4,mix(c,DARK,2));
 quad((Pt){x,y+8},(Pt){x+9,y+2},(Pt){x+w-5,y+3},(Pt){x+w,y+10},c);
 quad((Pt){x,y+h-8},(Pt){x+9,y+h-2},(Pt){x+w-5,y+h-3},(Pt){x+w,y+h-10},mix(c,DARK,2));
 rect(x,y+8,w,h-16,c);box(x+w/4,y+8,w/2,h-16,mix(c,0x264254,4),0x28424f);
 quad((Pt){x+w/4,y+10},(Pt){x+w/4+13,y+13},(Pt){x+w/4+13,y+h-13},(Pt){x+w/4,y+h-10},0x9dbec7);
 quad((Pt){x+w*3/4-14,y+13},(Pt){x+w*3/4-1,y+10},(Pt){x+w*3/4-1,y+h-10},(Pt){x+w*3/4-14,y+h-13},0x9dbec7);
 rect(x+w/4+14,y+13,w/2-29,2,mix(c,PAPER,4));rect(x+6,y+10,8,3,0xe38b85);rect(x+6,y+h-13,8,3,0xe38b85);
 rect(x+w-12,y+10,8,4,0xf4e4aa);rect(x+w-12,y+h-14,8,4,0xf4e4aa);
}
/* Overhead ferns: asymmetric feathered fronds rather than solid green blobs. */
static void int_fern(int x,int y,int radius,U a,unsigned seed,int young){
 U deep=mix(a,0x183b37,5),edge=mix(a,0xe9e3ab,2);
 if(radius<=3){disc(x,y,2,a);dot(x+1,y-1,edge);return;}
 for(int j=0;j<(young?4:7);j++){
  float turn=j*.8976f+(seed%13)*.126f;
  int extent=max(3,radius-(int)((seed>>(j+1))&3u)*radius/10);
  float ca=cs(turn),sa=sn(turn);
  int vx=x+(int)(ca*extent),vy=y+(int)(sa*extent*.76f);
  line(x,y,vx,vy,deep);line(x,y,vx,vy-1,mix(a,deep,2));
  for(int f=2;f<=6;f++){
   float t=f/7.f;int tx=x+(int)(ca*extent*t),ty=y+(int)(sa*extent*t*.76f)-1;
   int leaf=max(1,(int)(extent*.24f*(1.f-t)));
   for(int sign=-1;sign<2;sign+=2){
    int ex=tx+(int)(ca*2.f-sa*leaf*sign),ey=ty+(int)((sa*2.f+ca*leaf*sign)*.82f);
    line(tx,ty,ex,ey,(j+f+sign)%3?mix(a,deep,1):edge);
    if(radius>=12)line(tx,ty+1,ex,ey,mix(a,deep,3));
   }
  }
 }
 disc(x,y,1,edge);
}
static void int_potted_fern(int x,int y,int radius,U a,unsigned seed){
 U pot=seed&1?0xb98659:0x91786d;
 disc(x+2,y+3,max(5,radius/2),0x26322e);disc(x,y+2,max(5,radius/2),pot);
 disc(x,y,max(4,radius/2-2),0x413d2e);int_fern(x,y,radius,a,seed,0);
}
/* Site-specific appliances are drawn as silhouettes and moving parts, not another
   variation on the same desk. Their footprints are solid and each is inspectable. */
static void int_draw_feature(const InteriorProp*f){
 int x=f->x,y=f->y,w=f->w,h=f->h; U a=f->tint,ink=0x172733;
 int_solid_shape(x,y,w,h,mix(a,0x273844,5));
 box(x+5,y+5,w-10,h-13,0x344653,mix(a,0x384b55,5));
 int cx=x+w/2,cy=y+h/2;
 switch(venue_shell_id){
 case 0: { /* transit diagram and sorting sectors */
  rect(x+9,y+8,w-18,h-21,0x193743);
  static const int xp[]={12,28,53,77,99,125,143}, yp[]={23,23,13,13,24,24,13};
  for(int j=0;j<2;j++)for(int i=1;i<7;i++){
   int u=x+xp[i-1],v=y+yp[i-1]+j*7,q=x+xp[i],r=y+yp[i]+j*7;
   line(u,v,q,r,j?0xefbd7a:a);line(u,v+1,q,r+1,j?0xefbd7a:a);
   disc(u,v,3,ink);disc(u,v,2,0xe4dfc3);
  }break;}
 case 1: /* DJ wheels, crossfader and level meters */
  for(int j=0;j<2;j++){int u=x+(j?3:1)*w/4;disc(u,cy,18,0xb99fad);disc(u,cy,16,0x1f2232);
   for(int i=0;i<16;i++){float e=i*.392699f;dot(u+cs(e)*14,cy+sn(e)*14,0x8e7a94);}
   disc(u,cy,6,j?0xe4b483:0xdf739f);line(u+10,cy-12,u+5,cy-2,PAPER);}
  for(int i=0;i<4;i++){rect(cx-11+i*6,y+12,2,19,ink);rect(cx-12+i*6,y+16+(i*7)%12,4,3,0xc5bbc5);}
  break;
 case 2: /* silvery fish on a chipped ice bed */
  rect(x+7,y+7,w-14,h-16,0x9dbac0);
  for(int i=0;i<160;i++){U q=int_hash(i,19);rect(x+10+q%(w-23),y+10+(q>>12)%(h-23),3,2,(q&8)?0xd4e6dc:0x88a8b4);}
  for(int i=0;i<5;i++){int u=x+18+i*29,v=cy+(i%2?3:-4);
   tri(u-2,v,u+8,v-5,u+18,v,0x446d7e);tri(u-2,v,u+9,v+4,u+18,v,0x638b98);
   tri(u+16,v,u+23,v-5,u+24,v+4,0x547c8b);dot(u+2,v-1,ink);}
  break;
 case 3: /* exploded inline engine and manifolds */
  for(int i=0;i<5;i++){int u=x+24+i*23;box(u,y+12,16,26,0x647f8c,0x263a48);
   disc(u+8,y+19,6,0xbac5b9);disc(u+8,y+19,3,0x485d72);line(u+8,y+26,u+11,y+35,0xe1ac79);}
  rect(x+13,y+39,w-26,3,0xd59f71);break;
 case 4: /* soil tray, small terra-cotta pots and a scoop */
  box(x+8,y+9,32,20,0x684d3b,0xcba775);
  for(int i=0;i<10;i++)dot(x+12+int_hash(i,3)%22,y+11+int_hash(i,4)%12,0xa57a4e);
  for(int i=0;i<3;i++){disc(x+50+i*13,y+16+(i&1)*6,5,0xd9a07a);disc(x+50+i*13,y+16+(i&1)*6,3,0x75574c);}
  line(x+48,y+31,x+76,y+25,0xe4c890);break;
 case 5: /* shielded fictional reactor cores */
  for(int i=0;i<3;i++){int u=x+29+i*43;
   rect(u-3,y+6,6,h-12,0x8299a9);disc(u,cy,20,0x1b293f);disc(u,cy,17,0x95aebc);
   disc(u,cy,14,0x556a89);disc(u,cy,10,0xaf91d3);disc(u-3,cy-3,5,0xe5c9f4);
   for(int j=0;j<4;j++)rect(u-14+j*8,cy+21,4,3,j==2?0xf4be8c:0x707e95);}
  break;
 case 6: /* track silhouette, finish flags and lap timing live wall */
  box(x+8,y+7,w-16,h-17,0x14323e,0x668593);
  {int tx[]={12,29,58,70,52,30,12},ty[]={22,13,13,24,36,29,22};
   for(int i=1;i<7;i++){line(x+tx[i-1],y+ty[i-1],x+tx[i],y+ty[i],a);
    line(x+tx[i-1],y+ty[i-1]+1,x+tx[i],y+ty[i]+1,a);}}
  textat(x+79,y+7,"LAP 03",PAPER,0);textat(x+79,y+26,"01:42.6",a,0);break;
 case 7:
  for(int i=0;i<7;i++){rect(x+8,y+8+i*4,w-16,2,0x7b7289);rect(x+9,y+10+i*4,w-18,1,ink);}
  rect(cx-8,y+h-13,16,4,0xcf9e7e);break;
 case 8: /* patient monitor and ECG strip */
  box(x+9,y+6,w-18,h-13,0x183d44,0x8cb3ac);
  {int y0=y+21,v=y0;for(int i=2;i<w-24;i++){
   int u=i%26,d=u==14?-9:u==15?7:u==16?-3:u==21?-2:0;line(x+11+i-1,v,x+11+i,y0+d,0x91e4a7);v=y0+d;}}
  break;
 case 9:case 13:
  {int g=x+14;box(g,y+8,42,h-18,0x7d98a1,0xcdd6c8);
   box(g+5,y+11,30,h-25,0x64818a,0x516b73);rect(g+25,y+8,3,8,0xcdcfbf);
   int u=x+w-42;box(u-8,y+7,38,h-17,0x354451,0xb5aa9d);
   for(int i=0;i<4;i++){int p=u+i%2*14,q=y+14+i/2*14;disc(p,q,6,0x8c8076);disc(p,q,4,i==1?0xbe8d6a:0x404c52);}
   rect(g+47,y+10,13,15,0xceaa7b);break;}
 case 10: /* drum, wound cable, lifting frame and suspended hook */
  for(int i=0;i<3;i++){rect(x+12,y+8+i*6,w-24,3,0xf1c372);
   for(int j=13;j<w-18;j+=13)line(x+j,y+7+i*6,x+j+6,y+11+i*6,0x6a695c);}
  box(cx-15,y+10,30,17,0x555c68,ink);for(int j=-11;j<13;j+=4)rect(cx+j,y+11,2,14,0xa0a5a0);
  line(cx,y+26,cx,y+h-14,PAPER);line(cx,y+h-14,cx+5,y+h-11,0xe8cda3);line(cx+5,y+h-11,cx+8,y+h-17,0xe8cda3);break;
 case 11: /* drawers and index cards */
  for(int j=0;j<2;j++)for(int i=0;i<6;i++){int u=x+10+i*23,v=y+8+j*13;
   box(u,v,20,12,0x876f5a,0x453f41);rect(u+5,v+3,10,4,0xe4c9a4);rect(u+8,v+8,5,2,0x343e48);}
  break;
 case 14:
  for(int i=0;i<3;i++){int u=x+27+i*42;line(u,y+6,u,y+h-11,0x829ca5);
   rect(u-15,y+13,30,32,0xc3b8a0);disc(u,y+28,12,0x425466);disc(u,y+28,8,0xd1c6b0);disc(u,y+28,4,0xaf756e);
   dot(u-4,y+25,ink);dot(u+5,y+26,ink);}
  break;
 case 15:
  box(x+7,y+7,w-14,h-16,0x213d50,0x89a3af);for(int j=0;j<4;j++)rect(x+10,y+10+j*3,12-j,1,0x9bc7c0);
  rect(x+w-14,y+10,5,6,0xd4bc91);break;
 }
}

static void int_draw_prop(const InteriorProp*f,int owner){
 int x=f->x,y=f->y,w=f->w,h=f->h;U c=f->tint;
 if(f->type==INT_GROW_STOCK)return; /* Draw live so selling updates the stacked crates. */
 if(f->type==INT_FEATURE){int_draw_feature(f);return;}
 if(f->type==INT_CAR){int_car(f);return;}
 if(f->type==INT_GLASS){
  rect(x+2,y+3,w+3,h+2,0x2a3e4b);box(x,y,w,h,0x7594a9,0x4c6377);
  if(w<h){line(x+2,y+2,x+2,y+h-3,0xb8c6cf);for(int j=y+17;j<y+h-3;j+=19)rect(x+1,j,w-2,2,0x475f74);}
  else{line(x+1,y+2,x+w-2,y+2,0xb8c6cf);for(int j=x+17;j<x+w-3;j+=19)rect(j,y+1,2,h-2,0x475f74);}
  return;
 }
 if(f->type==INT_WALL){box(x,y,w,h,0x617489,0x2c4558);if(w>3)line(x+1,y+1,x+1,y+h-2,0x9caeb9);return;}
 if(f->type==INT_BARS){
  box(x,y,w,h,0x223544,0x617885);
  for(int i=3;i<w-2;i+=5)rect(x+i,y+1,2,h-2,(i%3==0)?0xa7b2c2:0x748a9c);
  for(int i=3;i<w-2;i+=33)rect(x+i,y,4,h,0xb7c5cb);
  rect(x+1,y+h/2,w-2,1,0xa9b9c7);rect(x+w/2-5,y+h-7,9,5,0x35586f);
  rect(x+w/2-2,y+h-6,3,2,0xe8b179);return;
 }
 if(f->type==INT_FERNS){
  int_solid_shape(x,y,w,h,0xaa9271);
  rect(x+5,y+h/2,w-10,2,0x897555);rect(x+6,y+h-7,w-12,2,0xb89e78);
  for(int j=0;j<2;j++)for(int i=15;i<w-10;i+=23){
   unsigned q=int_hash(x+i,y+j);int l=9+(q%6);
   U green=mix(c,(q&1)?0xbdda76:0x37957a,(q>>3)%4+1);
   int_potted_fern(x+i+j*4,y+(j?h*3/4:h/4),l,green,q);
  }
  return;
 }
 if(f->type==INT_SETTLE){
  int_solid_shape(x,y,w,h,c);
  box(x+5,y+6,w-10,h-13,0x344a55,0x627e88);int_monitor(x+w-46,y+9,33,c);
  box(x+12,y+11,33,21,0x647d78,0x243942);rect(x+15,y+14,26,4,0xaeb8a4);
  rect(x+17,y+21,16,3,0x1e3b46);rect(x+32,y+24,6,2,0xaadf98);
  for(int j=0;j<3;j++){int xx=x+53+j*10;box(xx,y+h-19-(j%2)*4,13,8,0x87ab8c,0x354e50);rect(xx+5,y+h-17-(j%2)*4,2,4,0xcfe1b3);}
  rect(x+9,y+h-5,w-18,2,mix(c,DARK,2));return;
 }
 if(f->type==INT_SORTER){
  int_solid_shape(x,y,w,h,c);
  box(x+6,y+6,w-12,h-14,0x394e58,0x7b99a2);
  for(int i=10;i<w-8;i+=7)rect(x+i,y+9,2,h-20,0x6b858e);
  for(int i=17;i<w-22;i+=31){int yy=y+h/2-10+(i%3)*2;
   box(x+i,yy,20,16,0xa28467,0x584b48);rect(x+i+3,yy+3,7,3,0xddd0ab);
   rect(x+i+3,yy+9,14,1,0x665e55);}
  rect(x+5,y+h-6,w-10,2,0x8bb1b4);return;
 }
 if(f->type==INT_COOLER){
  int_solid_shape(x,y,w,h,c);
  box(x+5,y+7,w-10,h-13,0x526b71,0x789597);
  for(int i=8;i<w-22;i+=20){
   box(x+i,y+11,18,h-22,0x3c535d,0x8da8a8);
   int pick=(i%3);for(int j=0;j<3;j++){
    U item=pick?(pick==1?0x8a9c69:0xb68f72):0xbfa471;
    rect(x+i+3,y+17+j*9,12,5,mix(item,c,j));
    rect(x+i+5,y+18+j*9,3,1,0xd7d2c0);
   }
   line(x+i+3,y+12,x+i+3,y+h-13,0xa1bcb9);
  }
  return;
 }
 if(f->type==INT_TOOLS){
  int_solid_shape(x,y,w,h,c);
  box(x+7,y+5,w-14,h-12,0x455961,0x81969d);
  for(int i=9;i<w-9;i+=5)for(int j=9;j<h-8;j+=5)dot(x+i,y+j,0x647c86);
  for(int i=14;i<w-13;i+=15){int yy=y+13+(i%6)*2;
   rect(x+i,yy,3,13,0xabb4ae);rect(x+i-3,yy,9,3,0xbbc5bd);
   rect(x+i+5,yy+5,3,9,(i%2)?0x9c6f55:0xc8a467);
  }return;
 }
 if(f->type==INT_GROWBED){
  int_solid_shape(x,y,w,h,c);
  box(x+6,y+14,w-12,h-20,0x313d47,0x71858a);
  for(int i=0;i<5;i++){
   int u=x+25+i*37;box(u-14,y+20,29,h-30,0x2b363b,0x4c6064);
   disc(u,y+h/2+4,8,0x596878);disc(u,y+h/2+3,6,0x3e3938);
   rect(u-4,y+22,8,2,0x8c98a9);
  }
  rect(x+7,y+6,w-14,3,0x8fa8b2);for(int i=14;i<w-6;i+=11)dot(x+i,y+7,0xd1c2f2);
  return;
 }
 if(f->type==INT_PLANT){
  disc(x+w/2+4,y+h/2+6,w/2,0x1d2730);disc(x+w/2,y+h/2,w/2,0x857064);
  disc(x+w/2,y+h/2,w/2-4,0x314d3b);
  for(int i=0;i<7;i++){float a=(i*2.399963f);int ex=x+w/2+cs(a)*(w/2-2),ey=y+h/2+sn(a)*(h/2-2);
   line(x+w/2,y+h/2,ex,ey,0x96a779);disc(ex,ey,4,i&1?mix(c,0x273b3b,2):mix(c,0x273b3b,4));}
  return;
 }
 int_solid_shape(x,y,w,h,c);
 switch(f->type){
 case INT_DESK:case INT_CONSOLE:case INT_DINER:
  if(f->type==INT_DINER){
   rect(x+3,y+4,w-6,4,0x748a92);for(int i=0;i<4;i++)rect(x+6,y+8+i*4,w-12,1,mix(c,0x22343f,3));
   for(int i=8;i<w-5;i+=15)rect(x+i,y+h-10,7,4,0x293244);
  }
  int_monitor(x+w-49,y+5,34,c);box(x+10,y+9,21,15,0x707e87,0x253440);
  rect(x+12,y+12,12,9,0xc4b99f);for(int i=0;i<3;i++)rect(x+14,y+14+i*2,7,1,0x8a8075);
  if(w>90){disc(x+w/2+2,y+21,5,0x1a2932);disc(x+w/2,y+19,4,0xbd8d70);disc(x+w/2,y+19,2,0x473c3d);}
  rect(x+2,y+h-6,w-4,2,mix(c,0x253540,2));break;
 case INT_RACK:case INT_TRAY:
  rect(x+3,y+h/2,w-6,2,0x647583);
  for(int i=9;i<w-11;i+=13){
   for(int j=0;j<2;j++){
    U cc=(int_hash(i,j)&1)?mix(c,0xe7bf91,2):mix(c,0x395967,2);
    int yy=y+6+j*(h/2),ht=h/2-8;
    if(f->type==INT_TRAY&&owner>=0){
     if(owner){disc(x+i+3,yy+ht/2,5,0x243c3a);disc(x+i+1,yy+ht/2-2,3,cc);disc(x+i+5,yy+ht/2+1,3,mix(cc,0x7bcd88,3));}
     else{rect(x+i-1,yy+2,9,ht-3,0x4b5656);rect(x+i,yy+3,7,1,0x6b7770);}
    }else{box(x+i,yy,8,ht,cc,0x39454d);rect(x+i+2,yy+2,3,1,mix(cc,PAPER,3));}
   }
  }break;
 case INT_BED:case INT_CLINIC:
  box(x+5,y+6,w-10,h-14,0xb3a7a4,0x3b4754);
  rect(x+8,y+11,w/4,h-25,0xe3d6c5);rect(x+9,y+12,w/4-3,h-29,0xc9cdcc);
  rect(x+w/4+11,y+8,w-w/4-18,h-18,mix(c,0xe6dbb8,2));
  for(int i=13;i<h-10;i+=5)rect(x+w/4+11,y+i,w-w/4-18,1,mix(c,0x344355,1));
  if(f->type==INT_CLINIC){rect(x+w*3/4-6,y+h/2-2,15,4,PAPER);rect(x+w*3/4,y+h/2-7,4,14,PAPER);}
  else{rect(x+w-19,y+8,5,h-18,mix(c,0x4c5d72,3));}
  break;
 case INT_SOFA:
  box(x+8,y+14,w-16,h-19,mix(c,0xe7cdac,2),0x453f47);
  rect(x+5,y+5,w-10,7,mix(c,0x3e3a45,3));rect(x+6,y+14,9,h-18,c);rect(x+w-15,y+14,9,h-18,c);
  for(int i=1;i<=2;i++)line(x+i*w/3,y+15,x+i*w/3,y+h-6,mix(c,DARK,2));
  break;
 case INT_CRATES:
  for(int i=6;i<w-19;i+=22){int j=i%3;box(x+i,y+5+j*3,19,h-13-j*3,mix(c,0x6d5547,j),0x3b3639);
   rect(x+i+2,y+9+j*3,15,2,0x938573);line(x+i+3,y+13+j*3,x+i+14,y+h-11,0x5f534d);}
  break;
 case INT_LAB:
  for(int i=11;i<w-14;i+=26){
   box(x+i,y+8,17,h-18,0x253c4b,0x799197);rect(x+i+3,y+12,11,h-25,0x536578);
   if(owner){rect(x+i+3,y+h/2,11,h/2-13,mix(c,0x2b5362,2));for(int j=0;j<3;j++)dot(x+i+5+(j*7)%9,y+h/2+j*4,mix(c,PAPER,3));}
   rect(x+i+2,y+6,13,3,0x98a2ad);rect(x+i+4,y+13,2,h-24,0x8b9da7);
  }break;
 case INT_GUNCASE:
  rect(x+3,y+6,w-6,h-13,0x2b4251);line(x+6,y+h-10,x+w-8,y+8,0x527284);
  for(int i=12;i<w-25;i+=36)int_pulse_tool(x+i,y+13+(i%3),i&4,mix(c,0xd4d6ba,1));
  rect(x+5,y+h-6,w-10,2,mix(c,0x96a79e,2));break;
 case INT_LOCKER:
  for(int i=5;i<w-13;i+=15){box(x+i,y+5,13,h-11,mix(c,0x4d6072,1),0x2d4051);rect(x+i+3,y+10,7,2,0x343f4c);rect(x+i+9,y+h/2,2,5,0x9ba8a5);}
  break;
 case INT_REEL:
  box(x+5,y+5,w-10,h-10,0x34313e,0x786b79);rect(x+9,y+9,w-18,4,c);
  for(int i=0;i<2;i++){disc(x+w*(i+1)/3,y+h/2,7,0x657a80);disc(x+w*(i+1)/3,y+h/2,3,0x333d49);}
  rect(x+9,y+h-11,w-18,2,c);break;
 }
}
static void int_floor_mark(int x,int y,int w,int h,U a){
 box(x,y,w,h,mix(a,0x25313e,5),mix(a,0x142a36,4));
 for(int i=3;i<h-2;i+=4)line(x+2,y+i,x+w-3,y+i,mix(a,0x172633,5));
 line(x+4,y+3,x+w-5,y+3,mix(a,0x94b3b0,2));
}
static void int_facility_floor(int k,U a){
 int li=legal_index_for_kind(k);
 if(li>=0&&!interior_subroom){
  U access=sv.legit[li].owned?0x9bdf8c:0xd58c80;
  box(421,108,118,41,0x253746,0x667c89);
  box(443,113,74,31,0x3e5259,0x83959d);
  rect(446,117,67,5,0x869894);rect(447,126,65,16,0x314650);
  rect(478,125,3,18,0x788d95);rect(468,133,5,3,access);rect(488,133,5,3,access);
  rect(438,145,84,4,0x687a85);rect(448,145,64,2,access);
  int_lamp(480,151,40,access,3);
 }else if(li>=0){
  // Station floor paint and overhead steel organize the displays by use, not generic decor.
  int_floor_mark(389,255,181,162,li==0?0x548185:li==1?0x927e61:0x627b88);
  int_lamp(480,250,102,a,3);
  if(li==0){
    for(int y=163;y<412;y+=13){rect(383,y,4,8,0x87a4a3);rect(573,y+3,3,7,0x87a4a3);}
    rect(170,190,210,4,0x648b8d);rect(185,282,184,2,0xa3b09b);
  }else if(li==1){
    int_lamp(275,197,91,0x7ba6b6,4);int_lamp(686,196,90,0x89a5a9,4);
    for(int x=173;x<796;x+=17){rect(x,237,8,2,0x9e9b7a);rect(x,417,7,2,0x9e9b7a);}
  }else{
    for(int j=0;j<2;j++){int x=j?600:370;for(int y=164;y<414;y+=13){rect(x,y,2,9,0xa69779);rect(x+4,y+2,2,4,0x938670);}}
    box(176,237,192,100,0x37434d,0x7b8582);
    for(int x=181;x<365;x+=10)rect(x,334,5,2,0xc0a079);
  }
 }
 if(k==4){
  if(!interior_subroom){
   int_floor_mark(420,355,121,57,0x8d7664);
   box(416,108,128,42,0x213936,0x6c9271);
   rect(443,113,74,31,0x294536);rect(447,116,65,7,0x7d9a6b);
   rect(448,125,63,17,0x345848);rect(479,125,2,16,0x738d6b);
   rect(470,134,5,2,0xceb985);rect(485,134,5,2,0xceb985);
   rect(438,145,84,3,0xb3d397);
   int_lamp(480,151,40,0x93c58a,3);
   for(int i=0;i<6;i++)rect(454+i*10,159,5,1,0x799577);
   box(422,215,115,24,0x3c4d43,0x788466);
   center(480,215,"FERN HOUSE",0xd3cb97,0);
  }else{
   box(153,103,654,39,0x242b3c,0x56667b);
   for(int j=0;j<2;j++)for(int i=0;i<9;i++){
    int x=178+i*21+j*429;box(x,111,18,18,0x353d55,0x617085);line(x+2,125,x+15,114,0x607986);
   }
   box(370,110,220,22,0x313c4a,0x7d8a9e);center(480,110,"GROW ROOM / 01",a,0);
   for(int j=0;j<2;j++){int y=216+j*120;int_lamp(279,y,105,0xa39ce4,5);int_lamp(681,y,105,0xad9edc,5);}
   for(int j=0;j<2;j++){int x=j?560:397;for(int y=157;y<423;y+=15)rect(x,y,2,4,0x98a4b1);}
  }
 }
 if(k==15){
  // Colored floor plates and traffic stripes make the three work areas navigable.
  int_floor_mark(164,147,164,116,0x697587);
  int_floor_mark(347,146,265,116,0x567888);
  int_floor_mark(635,148,156,111,0x686570);
  int_floor_mark(385,336,210,75,0x748394);
  for(int i=641;i<782;i+=9)rect(i,282,5,2,0xc6a17a);
  for(int j=0;j<7;j++)rect(488,370+j*7,3,4,0x8b9eb4);
 }
}
/* Paint walls and exterior after all floor props. Step faces have a height,
   cap and kickboard, so an L floor never reads as a rectangle of tinted rugs. */
static void int_shell_edges(U wall,U a){
 const VenueShell*sh=venue_shell+venue_shell_id;
 for(int y=95;y<456;y++){
  int l=venue_limit(y,0),r=venue_limit(y,1);
  U outside=mix(0x101d28,0x1b2d38,(y/37)%4);
  rect(0,y,l,1,outside);rect(r,y,W-r,1,outside);
  if(y>=98&&y<444){
   rect(l,y,2,1,0x85949b);rect(l+2,y,5,1,mix(wall,0xacb6b5,3));rect(l+7,y,3,1,0x273f48);
   rect(r-9,y,4,1,mix(wall,0x24383e,1));rect(r-5,y,4,1,0x607983);
  }
 }
 for(int j=1;j<4;j++){
  int y=j==1?216:j==2?294:370;
  for(int e=0;e<2;e++){
   int q=sh->edge[j-1][e],r=sh->edge[j][e];if(q==r)continue;
   int u=min(q,r),w=max(q,r)-u,extends=e?r>q:r<q;
   if(extends){rect(u,y,w,8,wall);rect(u,y,w,2,0x93a3a4);rect(u,y+7,w,2,0x283b44);}
   else{rect(u,y-4,w,4,0x384c59);rect(u,y-4,w,1,0x83969e);}
  }
 }
 int l=sh->edge[0][0],r=sh->edge[0][1];rect(l,98,r-l,3,0x9caeaf);
 l=sh->edge[3][0];r=sh->edge[3][1];rect(l,441,r-l,3,0x273b47);
}

#include "named_sets.inc"

static void int_static(void){
 const int n=interior_active_loc,k=loc[n].kind;
 const int li=legal_index_for_kind(k);
 const int status=li>=0?sv.legit[li].owned:k==4||k==5?sv.b[k-4].own?(1+sv.b[k-4].tier):0:k==9?!!sv.home:0;
 if(interior_cache_loc==n&&interior_cache_status==status)return;
 interior_cache_loc=n;interior_cache_status=status;
 surface=interior_back;rw=W;rh=H;
 const InteriorTheme*t=interior_theme(k);
 U a=t->neon,wall=t->wall;
 for(int y=0;y<H;y++)rect(0,y,W,1,mix(0x14202a,0x192a36,(y/74)%5));
 for(int y=89;y<480;y+=26){
  int zz=24*(y&1);for(int x=5-zz;x<W;x+=46){U c=0x1b303b;line(x,y,x+42,y,c);line(x,y,x,y+8,c);}
 }
 int l=146,r=814,ww=r-l;
 rect(l+11,112,ww+5,342,0x131b25);
 rect(l+7,109,ww+6,340,0x121a23);
 box(l,98,ww,348,0x25333f,0x829798);
 /* Backdrop and furniture art are baked only when this room or its ownership changes. */
 const NsFinish *finish=ns_finish+venue_shell_id;
 for(int y=145;y<433;y++)for(int x=venue_limit(y,0);x<venue_limit(y,1);x++)
  dot(x,y,ns_floor_pixel(x,y,finish));
 ns_wall(venue_shell_id);
 int_facility_floor(k,a);
 ns_ground(venue_shell_id);
 ns_fixed_details(venue_shell_id);
 if(k==15){
  person(CHIEF_X,175,0x8593ad,0,1.570796f,1,1);
  person(418,162,0x75879f,0,1.570796f,1,2);
  person(541,163,0x8697a5,0,1.570796f,1,3);
 }
 for(int i=0;i<interior_nprop;i++){InteriorProp*f=interior_props+i;
  if(k!=13&&!interior_subroom&&f->x+f->w/2==interior_desk_x&&f->y+f->h+25==interior_desk_y){
    person(interior_desk_x,f->y-8,mix(a,0x55738a,3),0,1.570796f,0,k%5);
  }
  if(!ns_setpiece(f,k)&&!ns_furniture(f,k))int_draw_prop(f,k==4||k==5?status:-1);
 }
 /* Ambient signs are mounted on fixtures, not floating aisle text. Interaction
    hotspots, route hints and the staffed service name are retained by interior_paint. */
 if(k==5){box(338,391,139,25,0x243440,0x738696);center(408,392,status?"FACILITY ACTIVE":"LAB AVAILABLE",status?a:MUTED,0);}
 if(k==15){center(CHIEF_X,216,"CHIEF RIVAS",PAPER,0);center(710,222,"CUSTODY",PAPER,0);}
 rect(l+3,432,ww-6,11,0x273844);rect(l+5,432,ww-10,2,0x6c848c);
 rect(W/2-37,426,74,17,0x192e37);rect(W/2-36,433,72,8,0x4c6571);
 for(int i=0;i<8;i++)rect(W/2-30+i*8,435,4,2,mix(a,0x596c76,3));
 rect(W/2-37,423,5,12,0x90a7a5);rect(W/2+32,423,5,12,0x90a7a5);
 rect(W/2-25,440,50,3,a);rect(W/2-22,446,44,3,mix(a,0x222f3a,4));
 int_lamp(W/2,438,50,mix(a,0x537687,4),3);
 int_shell_edges(wall,a);
 surface=pixels;rw=W;rh=H;
}
static void paint_active_menu(void){
 makeMenu();int rear=legal_rear_index(menu),front=legal_menu_index(menu),li=rear>=0?rear:front;
 int which=interior_kind_loc(li>=0?legal_specs[li].kind:menu);const Place*l=which>=0?loc+which:0;
 U a=l?l->tint:TEAL;const char*title=rear>=0?"REAR ACCOUNT DESK":front>=0?"BUSINESS OFFICE":l?l->label:"FIELD MENU";
 int h=208+nopt*30,top=(H-h)/2,left=143,w=W-left*2;
 modal(left,top,w,h,a);
 textat(left+25,top+12,l?(li>=0?l->label:l->npc):"PORT VANTA / PAUSED",a,0);
 int big=textw(title,1)<w-52;textat(left+24,top+35,title,PAPER,big);
 if(l)textat(left+25,top+80,rear>=0?"Deposit and collect fictional game credits at this same desk.":front>=0?"Lease and manage this legal franchise.":l->note,MUTED,0);
 for(int i=0;i<nopt;i++){
  int yy=top+106+i*30;if(i==sel)box(left+23,yy,w-46,27,mix(DARK,a,2),a);
  textat(left+40,yy+2,opt[i].n,opt[i].ink?opt[i].ink:(i==sel?PAPER:MUTED),0);
 }
 int yy=top+108+nopt*30;
 if(nopt)wrapped(left+25,yy,opt[sel].note,a,62);
 if(p.noticeT>0)wrapped(left+25,yy+43,notice,notice_color(),62);else textat(left+25,yy+43,"UP / DOWN    ENTER SELECT    ESC BACK",MUTED,0);
}
#include "grow_room_art.inc"

static void int_grow_live(const GrowBatch*v,int dx,int dy){
 const Business*b=&sv.b[0];
 if(v->count){
  float progress=growth_fraction(0),taper=progress*progress*(3.f-2.f*progress);
  for(int q=0;q<v->count;q++){
   int x=v->pot[q].x-dx,y=v->pot[q].y-dy,radius=3+(int)((15+(q%3)*2)*taper);
   U c=v->product?(q%2?0xb9a9df:0xc7a7cd):(q%2?0x8fbc70:0xa5ce77);
   int_fern(x,y,radius,c,int_hash(q,b->batches+17),progress<.26f);
   if(progress>.77f){dot(x+radius/3,y-1,mix(c,PAPER,3));dot(x-radius/3,y+1,mix(c,PAPER,3));}
  }
 }
 char label[64];const int boardX=575-dx,boardY=389-dy;
 U a=0xbacff0;
 box(boardX,boardY,208,41,0x182c35,0x69858c);
 if(!b->own){textat(boardX+9,boardY+5,"FACILITY AVAILABLE",MUTED,0);return;}
 if(b->timer<=0){
  textat(boardX+9,boardY,"BEDS EMPTY",a,0);
  textat(boardX+9,boardY+18,(sv.stocks[0]>0||sv.stocks[1]>0)?"STOCK IN STORAGE":"START AT TERMINAL",MUTED,0);return;
 }
 int percent=cl((int)(growth_fraction(0)*100.f),0,100);char*at=append(label,b->product?"VIOLET HAZE / ":"NIGHTLEAF / ");
 at=number(at,percent);append(at,"%");
 textat(boardX+9,boardY-1,label,a,0);rect(boardX+9,boardY+25,189,3,0x41526a);
 rect(boardX+9,boardY+25,189*percent/100,3,a);
}
static void int_staff_panel(int li,int dx,int dy,U a){
 const LegalBusiness*b=sv.legit+li;
 const char*status="ACCOUNT DESK ONLINE";U status_color=a;
 if(sv.audit||sv.charges){status="ACCOUNT ON HOLD";status_color=0xff7479;}
 else if(b->pending>0){
  if(b->cooldown<=0.f){status="COLLECTION READY";status_color=0x9ded83;}
  else{status="DEPOSIT PROCESSING";status_color=0xe9c282;}
 }
 int x=376-dx,y=128-dy;
 box(x,y,208,23,0x283a42,0x6f8690);disc(x+12,y+11,2,status_color);
 center(x+111,y+1,status,status_color,0);
}
static void interior_paint(void){
 int n=interior_active_loc;
 if(n<0||n>=LEN(loc)){rect(0,0,W,H,DARK);return;}
 int_static();
 typedef U IV4 __attribute__((vector_size(16),aligned(1),may_alias));
 int dx=(int)interior_cam_x,dy=(int)interior_cam_y;
 int left=max(0,-dx),right=min(W,W-dx),fast=left+(right-left)/4*4;
 for(int y=0;y<H;y++){
  if((U)(y+dy)>=H){rect(0,y,W,1,0x13202a);continue;}
  U*d=pixels+y*W;const U*s=interior_back+(y+dy)*W;int x=0;
  for(;x<left;x++)d[x]=0x13202a;
  for(;x<fast;x+=4)*(IV4*)(d+x)=*(const IV4*)(s+x+dx);
  for(;x<right;x++)d[x]=s[x+dx];
  for(;x<W;x++)d[x]=0x13202a;
 }
 int k=loc[n].kind,li=legal_index_for_kind(k);const InteriorTheme*t=interior_theme(k);U a=t->neon;
 if(!fastGfx){
  for(int i=0;i<32;i++){int x=(i*197+(int)(anim*27))%W,y=84+(i*121+(int)(anim*106))%372;
    if(x<interior_left-dx-2||x>interior_right-dx+5)line(x,y,x-2,y+5,0x29424c);}
  int_lamp(W/2-dx,435-dy,25,a,2+(int)(sn(anim*4.f)+1));
 }
 int x=(int)interior_x-dx,y=(int)interior_y-dy;
 int dock=W/2-dx,doorY=427-dy;
 // Cut-away roof silhouette dissolves, leaving the walls and street coordinates undisturbed.
 if(interior_fade>1&&!fastGfx){
  int a0=min(7,(interior_fade+1)/2),top=104-dy,left0=interior_left+4-dx,r=interior_right-4-dx;
  for(int j=top;j<431-dy;j++)for(int i=max(left0,venue_limit(j+dy,0)-dx);i<min(r,venue_limit(j+dy,1)-dx);i++){
   U c=(i%49==0||j%29==0)?0x4d6875:0x253a4b;
   U*q=pixels+j*W+i;*q=mix(*q,c,a0);
  }
 }
 if(k==4&&interior_subroom){
  GrowBatch batch=grow_batch(sv.b);
  int_grow_live(&batch,dx,dy);grow_storage_art(dx,dy);grow_operator_art(&batch,dx,dy);
 }
 if(li>=0&&interior_subroom)int_staff_panel(li,dx,dy,a);
 mark(dock,doorY,6,int_at_door()?PAPER:a);
 center(dock,447-dy,interior_subroom?(k==4?"BACK TO STORE":"BACK TO PUBLIC COUNTER"):"EXIT TO STREET",a,0);
 if(li>=0&&!interior_subroom){
  int own=sv.legit[li].owned;U gate=own?0x9ded83:0xff9390;
  mark(GROW_GATE_X-dx,GROW_GATE_Y-dy,6,int_at_staff_gate()?PAPER:gate);
  center(GROW_GATE_X-dx,162-dy,own?int_staffDoor[li]:"STAFF DOOR / LOCKED",gate,0);
 }
 if(k==4&&!interior_subroom){
  mark(GROW_GATE_X-dx,GROW_GATE_Y-dy,6,int_at_grow_gate()?PAPER:a);
  center(GROW_GATE_X-dx,162-dy,"GROW ROOM",a,0);
 }
 if(k==15){
  mark(CHIEF_X-dx,CHIEF_Y-dy,5,int_at_chief()?PAPER:a);
  mark(CUSTODY_X-dx,CUSTODY_Y-dy,5,int_at_custody()?PAPER:GOLD);
  center(CUSTODY_X-dx,313-dy,"CUSTODY GATE",GOLD,0);
 }
 int deskX=interior_desk_x-dx,deskY=interior_desk_y-dy;
 mark(deskX,deskY,6,int_at_desk()?PAPER:a);
 /* Use a small legible nameplate near the counter; reserve the rest of the room for the art. */
 {int labelw=textw(t->counter,0);box(deskX-labelw/2-5,deskY+14,labelw+10,21,0x25343a,0x5e6c66);
  center(deskX,deskY+15,t->counter,a,0);}
 if(!interior_subroom&&int_feature_near(k)){
  const Feature *f=venue_feature+k;int cx=f->x+f->w/2-dx,cy=f->y+f->h-dy,lw=textw(f->label,0);
  box(cx-lw/2-4,cy,lw+8,19,0x27383e,0x68797a);center(cx,cy,f->label,a,0);
 }
 person(x,y,0xc6d979,walking?(int)(sn(footcycle)*5):0,interior_a,4,3);
 rect(0,0,W,81,0x111e28);rect(0,78,W,1,0x42616d);
 draw_day_clock(5);textat(DAY_BADGE_LABEL_X,10,t->line,a,0);int big=textw(t->banner,1)<610;textat(26,29,t->banner,PAPER,big);
 char v[84];
 rect(0,483,W,57,0x111e28);rect(0,483,W,1,0x456570);
 const int dropping=job.kind>=0&&job_venue()==n;
 const char *prompt="WASD / WALK   E / INTERACT   ESC / MENU";
 if(int_at_door())prompt=interior_subroom?(k==4?"E / RETURN TO STORE":"E / RETURN TO PUBLIC COUNTER"):"E / EXIT TO STREET";
 else if(k==4&&int_at_grow_gate())prompt=sv.b[0].own?"E / OPEN GROW ROOM":"GROW ROOM / OWNER ACCESS";
 else if(li>=0&&int_at_staff_gate())prompt=sv.legit[li].owned?"E / OPEN STAFF DOOR":"STAFF DOOR / FRANCHISE OWNERS ONLY";
 else if(k==15&&int_at_chief())prompt="E / SPEAK TO CHIEF RIVAS";
 else if(k==15&&int_at_custody())prompt="E / INSPECT CUSTODY GATE";
 else if(int_at_desk())prompt=k==13?"E / READ NOTE":li>=0&&interior_subroom?"E / USE ACCOUNT DESK":dropping&&job.kind==JOB_VAULT&&p.heat>0?"HANDOFF CLOSED / GO OUTSIDE AND LOSE PURSUIT":dropping?(job.kind==JOB_COURIER?"E / SIGN THE MANIFEST":"E / HAND OVER THE CARGO"):k==4&&interior_subroom?"E / OPERATE BATCH TERMINAL":"E / TALK AT THE COUNTER";
 else if(int_feature_near(k))prompt="E / INSPECT FIXTURE";
 if(shelterT>0){fmt(v,"RESTING / SECONDS LEFT: ",(int)(shelterT+0.99f));textat(25,489,v,LIME,0);}
 else if(p.noticeT>0)textat(25,489,notice,notice_color(),0);
 else if(job.kind>=0){
  char*at=append(v,jobNames[job.kind]);if(!job_has_clock()){at=append(at,"  /  STOP ");at=number(at,job.step+1);at=append(at," OF ");number(at,job.total);}else{at=append(at,"  /  TIME LEFT: ");number(at,(int)job.time);}textat(25,489,v,job.kind<4?TEAL:PINK,0);
 }else textat(25,489,t->instruction,PAPER,0);
 textat(25,514,prompt,a,0);
 if(saveFlash)textat(830,512,saveFlash>0?"SAVED":"SAVE ERROR",saveFlash>0?TEAL:PINK,0);
 if(menu>=0&&scene!=2)paint_active_menu();
 draw_wallet(650,7,294);
}
static void interior_facade(int n){
 const Place*l=loc+n;const InteriorTheme*t=interior_theme(l->kind);int x=l->x,y=l->y;
 // These are overhead awnings and inset thresholds, not extra blocking city walls.
 U a=l->tint,c=mix(t->wall,DARK,2);
 light(x,y-12,30,mix(a,DARK,3),3);
 quad((Pt){x-29,y-27},(Pt){x+23,y-27},(Pt){x+32,y-18},(Pt){x-21,y-18},mix(t->wall,0x788499,2));
 box(x-26,y-19,52,13,c,mix(c,a,3));
 rect(x-11,y-23,22,21,0x142731);rect(x-9,y-22,8,17,mix(c,0x84a7ab,3));rect(x+1,y-22,8,17,mix(c,0x84a7ab,3));
 rect(x-20,y-19,7,3,a);rect(x+13,y-19,7,3,a);rect(x-12,y-4,24,3,mix(a,0x445d62,3));
 for(int i=-2;i<=2;i++)rect(x+i*5-1,y+1,3,2,mix(a,0x29363a,5));
 if(l->kind==4||l->kind==5){rect(x-24,y-13,9,3,0x97aa70);rect(x+15,y-13,8,3,0x97aa70);}
 else if(l->kind==8){rect(x-20,y-13,9,2,PAPER);rect(x-17,y-16,2,8,PAPER);}
 else if(l->kind==14){rect(x-23,y-12,9,2,PINK);rect(x+13,y-12,8,2,PINK);}
}
