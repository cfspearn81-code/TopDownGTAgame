/* WOODLINE / fictional collectibles and game-only planter timers.
 * Existing 1044-byte Rev11 save: garage words 49-52 were zero migration holes.
 * No world actor IDs, real botanical species or real growing parameters. */
enum { WF_FLAGS=49, WF_INV=50, WF_PLANT_LANTERN=51, WF_PLANT_KIOSK=52 };
enum { WF_SURVEY_BEGIN=1u<<0,WF_SURVEY_PAID=1u<<1,WF_CATALOG_BEGIN=1u<<2,WF_CATALOG_PAID=1u<<3,
 WF_SALVAGE_BEGIN=1u<<4,WF_SALVAGE_PAID=1u<<5,WF_SUPPLY=1u<<6,WF_RUIN=1u<<7,WF_FINAL=1u<<8,
 WF_STAMP_SHIFT=9,WF_PATCH_SHIFT=12 };
enum { WF_RANGER,WF_PLANTER };
enum { WF_OBJ_RANGER,WF_OBJ_STAMP,WF_OBJ_PATCH,WF_OBJ_CHEST };
enum { WF_BENCH_X=555,WF_BENCH_Y=355,WF_INTERACT=32 };
static const char *const wf_fungi[]={"Velvet Lantern","Clockcap","Frostbell","Moonspindle"};
static const U wf_tint[]={0xe7a86f,0xd185bf,0xa8dfec,0xe9e59b};
static const char *const wf_quests[]={"Mark the walking loop","Four lights for the catalog","The missing trail beacon"};
typedef struct{unsigned char site,type; signed char x,y;} WfPatch;
static const WfPatch wf_patches[]={
 {WOOD_FERN,0,-18,-10},{WOOD_LOG,1,-21,-8},{WOOD_LOG,1,18,10},{WOOD_GRANITE,2,16,15},
 {WOOD_MEADOW,3,-17,-18},{WOOD_MEADOW,0,16,13},{WOOD_RUINS,2,16,-14},{WOOD_SALVAGE,3,17,10}
};
static const int wf_stamps[]={WOOD_FERN,WOOD_GRANITE,WOOD_RISE};
static const WfPatch wf_chests[]={{WOOD_SALVAGE,0,0,0},{WOOD_RUINS,0,0,0},{WOOD_RISE,0,17,13}};
static const U wf_chest_bits[]={WF_SUPPLY,WF_RUIN,WF_FINAL};
static const U wf_q_begin[]={WF_SURVEY_BEGIN,WF_CATALOG_BEGIN,WF_SALVAGE_BEGIN};
static const U wf_q_paid[]={WF_SURVEY_PAID,WF_CATALOG_PAID,WF_SALVAGE_PAID};
static const int wf_credit[]={90,120,165};
static const int wf_ticks[]={900,1080,1320,1560,1620,1800,2040};
static const int wf_fees[]={5,6,8,10,13,16,19};
static int wf_mode,wf_back,wf_shop,wf_row,wf_bed;
static char wf_note[168];
static int wf_has(U mask){return (nv.garage[WF_FLAGS]&mask)==mask;}
static int wf_inv(int v){return (nv.garage[WF_INV]>>(v*5))&31u;}
static void wf_inventory(int v,int count){
 int shift=v*5;nv.garage[WF_INV]=(nv.garage[WF_INV]&~(31u<<shift))|((U)cl(count,0,31)<<shift);
}
static int wf_count(U mask){int n=0;while(mask){n+=mask&1u;mask>>=1;}return n;}
static U wf_stamps_done(void){return (nv.garage[WF_FLAGS]>>WF_STAMP_SHIFT)&7u;}
static U wf_gathered(void){return (nv.garage[WF_FLAGS]>>WF_PATCH_SHIFT)&255u;}
static int wf_seen(int v){for(int i=0;i<LEN(wf_patches);i++)if(wf_patches[i].type==v&&(wf_gathered()&(1u<<i)))return 1;return 0;}
static int wf_species(void){int n=0;for(int i=0;i<LEN(wf_fungi);i++)if(wf_seen(i))n++;return n;}
static OuterPoint wf_point(const WfPatch*b){OuterPoint p=woodline_sites[b->site];p.x+=b->x;p.y+=b->y;return p;}
static void wf_msg(const char*s){int i=0;while(i<(int)sizeof wf_note-1&&s[i]){wf_note[i]=s[i];i++;}wf_note[i]=0;}
static void wf_commit(const char*s,int dialog){
 request_save();const char *text=saveFlash<0?"Woodline: save failed. Your old disk copy is intact; retry with F5.":s;
 if(dialog)wf_msg(text);else tell(text);
}
static int wf_cash_room(int v,int dialog){
 if(sv.cash<=WALLET_LIMIT-v)return 1;
 const char*s="Clean wallet full. Spend or bank some cash first. Reward not taken.";
 if(dialog)wf_msg(s);else tell(s);return 0;
}
static int wf_site_pin(OuterPoint q){
 nav_slot();navHasManual=1;navManualLoc=-1;navManual=(NavPoint){q.x,q.y};navGoalKind=0;return 1;
}
static void woodline_pin_ranger(void){wf_site_pin(woodline_sites[WOOD_RANGER]);}
static int woodline_post_status(void){return wf_count((U)(!!wf_has(WF_SURVEY_PAID)|(!!wf_has(WF_CATALOG_PAID)<<1)|(!!wf_has(WF_SALVAGE_PAID)<<2)));}
static int wf_chest_ready(int which){
 return which==0?wf_has(WF_SURVEY_PAID):which==1?wf_has(WF_SALVAGE_BEGIN|WF_SUPPLY):
  wf_has(WF_SURVEY_PAID|WF_CATALOG_PAID|WF_SALVAGE_PAID);
}
static void wf_open_chest(int which){
 if(wf_has(wf_chest_bits[which])){tell("Woodline / This chest has already been claimed.");return;}
 if(!wf_chest_ready(which)){tell(which==0?"Supply chest / Bring the Ranger a complete trail survey.":which==1?
  "Ruins chest / Take the beacon assignment and check the supply chest first.":"Lookout chest / Finish all three Ranger assignments.");return;}
 int reward=which==0?28:which==1?45:85;
 if(!wf_cash_room(reward,0))return;
 if(which==2&&wf_inv(3)>=31){tell("Lookout chest / Your Moonspindle specimen album is full.");return;}
 nv.garage[WF_FLAGS]|=wf_chest_bits[which];sv.cash+=reward;
 if(which==2)wf_inventory(3,wf_inv(3)+1);
 wf_commit(which==0?"Supply chest: $28 and the ruins key.":which==1?"Ruins chest: $45 and the missing trail beacon.":
  "Lookout chest: $85, Moonspindle and a Woodline keeper badge.",0);
}
static void wf_pick(int i){
 U bit=1u<<(WF_PATCH_SHIFT+i);if(wf_has(bit)){tell("Only an empty habitat marker remains.");return;}
 int type=wf_patches[i].type;if(wf_inv(type)>=31){tell("This specimen slot is full.");return;}
 wf_inventory(type,wf_inv(type)+1);nv.garage[WF_FLAGS]|=bit;
 char s[100];append(append(s,wf_fungi[type])," added to your fictional field collection.");wf_commit(s,0);
}
static void wf_stamp(int i){
 U bit=1u<<(WF_STAMP_SHIFT+i);
 if(!wf_has(WF_SURVEY_BEGIN)){tell("Trail marker / Accept the survey from the Woodline Ranger.");return;}
 if(wf_has(bit)){tell("Field journal / This trail stamp is already filed.");return;}
 nv.garage[WF_FLAGS]|=bit;wf_commit("Field journal / Trail stamp recorded.",0);
}
static int wf_prereq(int quest){return quest<2||wf_has(WF_SURVEY_PAID|WF_CATALOG_PAID);}
static int wf_condition(int quest){
 if(!quest)return wf_stamps_done()==7u;
 if(quest==1){for(int i=0;i<LEN(wf_fungi);i++)if(!wf_inv(i))return 0;return wf_species()==4;}
 return wf_has(WF_RUIN);
}
static void wf_ranger_claim(int quest){
 if(wf_has(wf_q_paid[quest])){wf_msg("Assignment already paid. Use M to pin the lookout chest after all three.");return;}
 if(!wf_prereq(quest)){wf_msg("Finish the trail survey and specimen catalog before searching for the beacon.");return;}
 if(!wf_has(wf_q_begin[quest])){
  nv.garage[WF_FLAGS]|=wf_q_begin[quest];
  wf_commit(quest==0?"Survey accepted. Press M to pin the next trail stamp.":quest==1?
    "Catalog accepted. Collect one of each fictional mushroom. Press M for a hint.":"Beacon assignment accepted. Check the supply chest and then explore the ruins.",1);
  return;
 }
 if(!wf_condition(quest)){wf_msg("Not ready yet. Press M to pin the next objective.");return;}
 if(!wf_cash_room(wf_credit[quest],1))return;
 if(quest==1)for(int i=0;i<LEN(wf_fungi);i++)wf_inventory(i,wf_inv(i)-1);
 sv.cash+=wf_credit[quest];nv.garage[WF_FLAGS]|=wf_q_paid[quest];
 wf_commit(!quest?"Survey filed. $90 and access to the supply chest.":quest==1?
    "Catalog filed. $120 for one preserved specimen of each kind.":"Beacon returned. $165 and access to the keeper's lookout chest.",1);
}
static void wf_ranger_track(int q){
 OuterPoint p=woodline_sites[WOOD_RANGER];const char*s="Ranger desk pinned. It sits just south of the Seawall.";
 if(wf_has(wf_q_paid[q])){
  if(woodline_post_status()==3){p=wf_point(wf_chests+2);s="Beacon lookout chest pinned.";}
  else if(q==0&&!wf_has(WF_SUPPLY)){p=wf_point(wf_chests);s="Trail supply chest pinned (contains the ruins key).";}
 }else if(wf_has(wf_q_begin[q])){
  if(!q){for(int i=0;i<3;i++)if(!(wf_stamps_done()&(1u<<i))){p=woodline_sites[wf_stamps[i]];s="Next unstamped trail post pinned.";break;}}
  else if(q==1){
   for(int i=0;i<LEN(wf_patches);i++)if(!wf_inv(wf_patches[i].type)&&!(wf_gathered()&(1u<<i))){p=wf_point(wf_patches+i);s="A missing specimen patch is pinned.";break;}
  }else if(!wf_has(WF_RUIN)){p=wf_point(wf_chests+(wf_has(WF_SUPPLY)?1:0));s="Next beacon-trail cache pinned.";}
 }
 wf_site_pin(p);wf_msg(s);
}
static int wf_shop_id(void){
 if(interior_subroom||interior_active_loc<0||interior_active_loc>=LEN(loc))return -1;
 int k=loc[interior_active_loc].kind;return k==4?0:k==7?1:-1;
}
static int wf_bench_near(void){return wf_shop_id()>=0&&d2(interior_x-WF_BENCH_X,interior_y-WF_BENCH_Y)<39.f*39.f;}
static int woodline_bench_solid(float x,float y){
 return wf_shop_id()>=0&&x>WF_BENCH_X-37&&x<WF_BENCH_X+37&&y>WF_BENCH_Y-54&&y<WF_BENCH_Y-17;
}
/* Each bed: tiny 1-based crop code in bits 0-2; timer ticks in bits 3-14. */
static unsigned wf_bed_state(int shop,int bed){return (nv.garage[WF_PLANT_LANTERN+shop]>>(bed*16))&65535u;}
static int wf_bed_crop(int shop,int bed){return ((int)wf_bed_state(shop,bed)&7)-1;}
static int wf_bed_left(int shop,int bed){return (wf_bed_state(shop,bed)>>3)&4095u;}
static void wf_bed_write(int shop,int bed,int crop,int left){
 int w=WF_PLANT_LANTERN+shop,shift=bed*16;
 U v=(U)(crop+1)|((U)cl(left,0,4095)<<3);
 nv.garage[w]=(nv.garage[w]&~(65535u<<shift))|(v<<shift);
}
static const char*wf_crop(int i){return i<LEN(wf_fungi)?wf_fungi[i]:np_design[i-LEN(wf_fungi)];}
static int wf_crop_unlocked(int i){
 if(i<LEN(wf_fungi))return wf_seen(i);
 return sv.b[0].own&&np_unlocked((i-LEN(wf_fungi))*3);
}
static void wf_planter_act(void){
 int i=wf_bed_crop(wf_shop,wf_bed);
 if(i>=0){
  if(wf_bed_left(wf_shop,wf_bed)){wf_msg("This crop is still growing on the game clock.");return;}
  if(i<LEN(wf_fungi)){if(wf_inv(i)>=31){wf_msg("Field collection full. The ready crop remains in its bed.");return;}wf_inventory(i,wf_inv(i)+1);}
  else{int k=NP_BAG+(i-LEN(wf_fungi))*3,n=np_get(k,99);
   if(n>=99){wf_msg("Your House-grade satchel slot is full. The ready crop remains in its bed.");return;}nv.phone[k]=n+1;}
  wf_bed_write(wf_shop,wf_bed,-1,0);wf_commit("Harvest stored. This bed is now empty.",1);return;
 }
 i=cl(wf_row,0,LEN(wf_ticks)-1);
 if(!wf_crop_unlocked(i)){wf_msg(i<4?"Discover this fictional species in Woodline first.":"This in-game strain needs Lantern ownership and its Glasshouse unlock.");return;}
 if(sv.cash<wf_fees[i]){wf_msg("Not enough clean credits for this fictional seed token.");return;}
 sv.cash-=wf_fees[i];wf_bed_write(wf_shop,wf_bed,i,wf_ticks[i]);
 wf_commit("Planter queued. Leave the menu and spend time in the city.",1);
}
static void woodline_world_tick(float dt){
 if(dt<=0||scene==NIGHT_FOREST_SCENE||!(nv.garage[WF_PLANT_LANTERN]|nv.garage[WF_PLANT_KIOSK]))return;
 int ready=0,k=cl((int)(dt*60.f+.5f),1,60);
 for(int i=0;i<2;i++)for(int j=0;j<2;j++){int crop=wf_bed_crop(i,j),left=wf_bed_left(i,j);
  if(crop>=0&&left){int r=max(0,left-k);wf_bed_write(i,j,crop,r);if(!r)ready=1;}}
 if(ready){tell("Planter crop ready. Visit its original shop to harvest.");request_save();}
}
typedef struct{int kind,id,d;} WfTarget;
static void wf_offer(WfTarget*t,OuterPoint at,int kind,int id){
 int d=(int)d2(p.x-at.x,p.y-at.y);if(d<t->d)*t=(WfTarget){kind,id,d};
}
static WfTarget wf_target(void){
 WfTarget t={-1,0,WF_INTERACT*WF_INTERACT};
 if(scene!=1||p.y<2040||p.x>970||p.car>=0||p.dead||menu>=0||mapOn||vdPanel||atmOpen>=0)return t;
 int x,y;if(job.kind>=0||vdActive>=0||ng_route_goal(&x,&y))return t;
 wf_offer(&t,woodline_sites[WOOD_RANGER],WF_OBJ_RANGER,0);
 for(int i=0;i<LEN(wf_stamps);i++)wf_offer(&t,woodline_sites[wf_stamps[i]],WF_OBJ_STAMP,i);
 for(int i=0;i<LEN(wf_patches);i++)wf_offer(&t,wf_point(wf_patches+i),WF_OBJ_PATCH,i);
 for(int i=0;i<LEN(wf_chests);i++)wf_offer(&t,wf_point(wf_chests+i),WF_OBJ_CHEST,i);
 return t;
}
static int woodline_entry(U tap){
 if(p.dead||menu>=0||mapOn||vdPanel||atmOpen>=0)return 0;
 if(scene==3&&(tap&ENTER)&&wf_bench_near()){
  int x,y;if(job.kind>=0||vdActive>=0||ng_route_goal(&x,&y))return 0;
  wf_back=3;wf_mode=WF_PLANTER;wf_shop=wf_shop_id();wf_row=wf_bed=0;wf_note[0]=0;scene=NIGHT_FOREST_SCENE;walking=0;return 1;
 }
 if(!(tap&USE))return 0;
 WfTarget t=wf_target();if(t.kind<0)return 0;
 if(t.kind==WF_OBJ_PATCH)wf_pick(t.id);
 else if(t.kind==WF_OBJ_STAMP)wf_stamp(t.id);
 else if(t.kind==WF_OBJ_CHEST)wf_open_chest(t.id);
 else{wf_back=1;wf_mode=WF_RANGER;wf_row=wf_bed=0;wf_note[0]=0;scene=NIGHT_FOREST_SCENE;walking=0;}
 return 1;
}
static void woodline_tick(U keys,U tap){
 (void)keys;
 if(tap&ESC){scene=wf_back;return;}
 int n=wf_mode==WF_RANGER?3:LEN(wf_ticks);
 if(tap&(UP|DOWN)){wf_row=(wf_row+n+((tap&UP)?-1:1))%n;wf_note[0]=0;return;}
 if(wf_mode==WF_PLANTER&&(tap&(LEFT|RIGHT))){wf_bed^=1;wf_note[0]=0;return;}
 if(wf_mode==WF_RANGER&&(tap&MAP)){wf_ranger_track(wf_row);return;}
 if(tap&(USE|ENTER)){if(wf_mode==WF_RANGER)wf_ranger_claim(wf_row);else wf_planter_act();}
}
static void wf_mushroom(int x,int y,int t,int size){
 U tint=wf_tint[t];int h=max(1,size/2);
 disc(x+2,y+1,size/2+1,0x172829);rect(x-1,y-size/2,2+size/4,size/2+1,0xcfbba6);
 if(t==0){disc(x,y-size,size/2+1,0x875746);rect(x-h,y-size-2,h*2+1,3,tint);disc(x+1,y-size-2,1,PAPER);}
 else if(t==1){box(x-h,y-size-4,h*2+1,6,tint,0x8d6689);rect(x-h+2,y-size-2,h*2-3,2,0xe9bfde);}
 else if(t==2){line(x-h-1,y-size,x,y-size-5,tint);line(x,y-size-5,x+h+1,y-size,tint);rect(x-h,y-size,h*2+1,2,tint);dot(x,y-size-5,PAPER);}
 else{disc(x,y-size-3,max(2,h-1),tint);disc(x+1,y-size-5,max(1,h-2),0x243c39);dot(x+3,y-size-3,PAPER);}
}
static void wf_stamp_art(int x,int y,int on){
 rect(x-4,y-1,9,3,0x293d3b);rect(x-1,y-14,3,15,0x887758);rect(x-5,y-17,11,7,on?0x9bbe83:0xd5ad73);
 rect(x-2,y-15,4,2,0x354848);dot(x-4,y-15,0xecc488);
}
static void wf_cache_art(int x,int y,int kind){
 int claimed=wf_has(wf_chest_bits[kind]),open=wf_chest_ready(kind);
 U c=claimed?0x5c8078:open?0xefc57b:0x829388;
 rect(x-13,y+3,28,3,0x203438);
 box(x-12,y-8,25,12,0x4b4437,c);
 rect(x-9,y-7,19,4,0x726047);
 rect(x-8,y-7,2,9,c);rect(x+6,y-7,2,9,c);rect(x-2,y-3,5,4,c);
 if(claimed){box(x-11,y-18,23,9,0x344044,c);rect(x-9,y-16,19,3,0x536363);}
}
static void woodline_street_paint(void){
 if(cy+H<2030||cx>1015)return;
 for(int i=0;i<LEN(wf_patches);i++){
  OuterPoint p=wf_point(wf_patches+i);int x=sx(p.x),y=sy(p.y);if(x<-18||x>W+18||y<-18||y>H+18)continue;
  if(wf_gathered()&(1u<<i)){line(x-3,y,x,y-2,0x889c7b);dot(x+1,y-3,0xa29c77);continue;}
  int t=wf_patches[i].type;
  wf_mushroom(x,y,t,8);wf_mushroom(x-8,y+4,t,5);wf_mushroom(x+7,y+4,t,6);
  if(!fastGfx&&((int)(anim*19)+i*6)%55<18)dot(x-2,y-18-((int)(anim*19)+i*6)%12,wf_tint[t]);
 }
 for(int i=0;i<LEN(wf_stamps);i++){OuterPoint p=woodline_sites[wf_stamps[i]];wf_stamp_art(sx(p.x),sy(p.y),(wf_stamps_done()>>i)&1u);}
 for(int i=0;i<LEN(wf_chests);i++){OuterPoint p=wf_point(wf_chests+i);wf_cache_art(sx(p.x),sy(p.y),i);}
 {OuterPoint p=woodline_sites[WOOD_RANGER];int x=sx(p.x),y=sy(p.y);
  if(x>-50&&x<W+50&&y>-30&&y<H+55){
   box(x-24,y-27,48,17,0x273e3d,0xb0ab7b);rect(x-21,y-13,42,4,0x605c49);
   rect(x-18,y-9,3,8,0x8c7651);rect(x+16,y-9,3,8,0x8c7651);
   for(int i=0;i<4;i++){int xx=x-17+i*9;rect(xx,y-24,5,8,i%2?0x98ad94:0xe5c58b);}
   person(x+24,y+8,0xa5b882,0,1.570796f,1,1);
  }}
}
static void woodline_hint(void){
 WfTarget t=wf_target();if(t.kind<0)return;
 const char*s=t.kind==WF_OBJ_RANGER?"E / WOODLINE RANGER":t.kind==WF_OBJ_STAMP?"E / TRAIL SURVEY POST":
  t.kind==WF_OBJ_CHEST?(wf_has(wf_chest_bits[t.id])?"E / CLAIMED FOREST CHEST":"E / INSPECT FOREST CHEST"):
  wf_gathered()&(1u<<t.id)?"E / EMPTY HABITAT":wf_fungi[wf_patches[t.id].type];
 int w=textw(s,0)+54;box((W-w)/2,482,w,27,0x162332,0xa2c98e);
 if(t.kind==WF_OBJ_PATCH&&!(wf_gathered()&(1u<<t.id))){textat((W-w)/2+11,486,"E /",0xa2c98e,0);textat((W-w)/2+37,486,s,PAPER,0);}
 else center(W/2,486,s,0xcce0a5,0);
}
static void woodline_interior_paint(int dx,int dy){
 int shop=wf_shop_id();if(shop<0)return;
 int x=WF_BENCH_X-dx,y=WF_BENCH_Y-dy;
 box(x-33,y-50,67,24,0x354843,0x8f9980);rect(x-29,y-47,59,15,0x28312e);
 for(int b=0;b<2;b++){int q=x-20+b*39,crop=wf_bed_crop(shop,b),left=wf_bed_left(shop,b);
  box(q-8,y-47,17,11,0x50463c,0x86745a);
  if(crop>=0){if(crop<4)wf_mushroom(q,y-38,crop,left?4:7);
   else{int h=left?5:10;line(q,y-38,q,y-38-h,0x9dbc7a);for(int j=1;j<=h;j+=2){line(q,y-j-38,q-4,y-j-40,0x759d6e);line(q,y-j-38,q+4,y-j-39,0xa7bc7a);}}}
  line(q-9,y-22,q+9,y-22,crop<0?0x725745:left?0xdfa76d:0x9ce08e);}
 box(x-58,y-20,117,21,0x2b4138,0x7ca167);center(x,y-20,"PLANTERS",PAPER,0);
 if(wf_bench_near()&&scene==3&&menu<0){box(x-68,y+23,138,20,0x182b27,0x5f986d);center(x,y+22,"ENTER / USE",0xd3eba5,0);}
}
static void wf_trail_wallpaper(void){
 for(int y=0;y<H;y+=6)rect(0,y,W,6,mix(0x122930,0x111d28,y*7/H));
 for(int i=0;i<56;i++){int x=(i*277+19)%W,y=(i*143+11)%H;if((x>143&&x<817)||y<63)continue;
  int r=4+i%10;tri(x,y-r*2,x-r,y+r,x+r,y+r,i%2?0x294845:0x233c3e);rect(x-1,y+r,2,3,0x365752);}
}
static void woodline_paint(void){
 wf_trail_wallpaper();
 int plant=wf_mode==WF_PLANTER;U accent=plant?0xa9d793:0xf1c478;char s[110];const char*msg=wf_note;
 box(147,57,666,428,0x142630,0x567778);rect(148,58,664,3,accent);
 textat(170,73,plant?(wf_shop==0?"LANTERN GROW LOFT / COMMUNITY BEDS":"KIOSK 09 / COMMUNITY BEDS"):"WOODLINE / RANGER DISTRICT",accent,0);
 textat(168,98,plant?"Shop planters":"Ranger field journal",PAPER,1);
 rect(170,150,620,1,0x48605d);
 if(!plant){
  char*q=number(append(s,"Assignments complete: "),woodline_post_status());append(q," / 3");
  textat(170,156,s,0xb5c8ba,0);
  for(int i=0;i<3;i++){int y=187+i*32;U c=i==wf_row?accent:MUTED;
   box(170,y,619,28,i==wf_row?0x304541:0x1d3436,0x3e5350);textat(179,y+4,wf_quests[i],c,0);
   const char*tag=wf_has(wf_q_paid[i])?"COMPLETE":!wf_prereq(i)?"LOCKED":!wf_has(wf_q_begin[i])?"AVAILABLE":wf_condition(i)?"READY TO TURN IN":"ACTIVE";
   textat(780-textw(tag,0),y+4,tag,c,0);}
  for(int i=0;i<4;i++){int x=179+(i%2)*311,y=303+(i/2)*23;
   box(x-9,y-1,304,22,0x182e34,0x36504c);wf_mushroom(x+7,y+14,i,7);
   textat(x+21,y+1,wf_fungi[i],wf_seen(i)?PAPER:0xb0bdb2,0);
   number(s,wf_inv(i));textat(x+284-textw(s,0),y+1,s,wf_tint[i],0);}
  if(!*msg)msg=!wf_row?"Stamp three posts: Fern Hollow, Granite Fork and East Rise. Then hand your survey in here.":
   wf_row==1?"Find one of each fictional mushroom. The ranger trades clean cash for the full set.":
   "Finish the first two assignments. Then take the supply chest's key to the ruins and return the beacon.";
 }else{
  for(int i=0;i<2;i++){int x=170+i*312,crop=wf_bed_crop(wf_shop,i),left=wf_bed_left(wf_shop,i);
   box(x,156,304,36,i==wf_bed?0x294138:0x192e32,i==wf_bed?accent:0x41635a);
   char*q=number(append(s,"BED "),i+1);q=append(q," / ");if(crop<0)append(q,"EMPTY");else if(left)append(number(q,(left+59)/60)," s");else append(q,"HARVEST");
   textat(x+10,155,s,accent,0);if(crop>=0)textat(x+10,172,wf_crop(crop),PAPER,0);}
  for(int i=0;i<LEN(wf_ticks);i++){int y=197+i*24;U c=i==wf_row?accent:MUTED;
   box(170,y,620,23,i==wf_row?0x2c433a:0x1a3032,0x354d47);textat(180,y+1,wf_crop(i),c,0);
   if(wf_crop_unlocked(i)){char*q=number(append(s,i>=4?"HOUSE C  /  $":"FICTIONAL  /  $"),wf_fees[i]);q=number(append(q,"  /  "),wf_ticks[i]/60);append(q," s");}
   else append(s,i<4?"FIND IN WOODLINE":"GLASSHOUSE LOCKED");
   textat(778-textw(s,0),y+1,s,c,0);}
  if(!*msg){int crop=wf_bed_crop(wf_shop,wf_bed);msg=crop<0?"Select a specimen and queue one unit. Timers are entirely fictional game stats.":
   wf_bed_left(wf_shop,wf_bed)?"This bed is busy. Timers advance outside this panel but never while the game is paused.":
   "Bed is ready. Enter harvests one exact specimen or one House C game strain to your inventory.";}
 }
 rect(170,382,620,60,0x1d3537);
 for(int i=0;*msg&&i<3;i++){char ln[130];msg=hud_line(msg,591,ln);textat(180,383+i*18,ln,0xd0e3be,0);}
 textat(170,455,plant?"LEFT/RIGHT BED  UP/DOWN CROP  E/ENTER ACT  ESC BACK":"UP/DOWN ASSIGNMENT  E/ENTER ACT  M PIN GOAL  ESC BACK",accent,0);
}
