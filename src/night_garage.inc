/* MOTOR WORKS / Switchyard Social
 * nv.garage layout: 0 deed; 1..30 parts by vehicle & category; 32..34
 * best runs in 60 Hz ticks; 36 showcase cooldown; 37 gilded livery;
 * 38 recovered parts; 39 show wins, 40 run wins; 41..45 rival edition flags;
 * 47..48 new courses; 53 rotation epoch; 54 dusk latch; 55..59 arcade tags; 60 Chase View. No transient car indices
 * or fictitious street contracts persist in a career. */
enum{NG_DEED=420,NG_MEET_X=1280,NG_MEET_Y=768,NG_DOCK_X=1024,NG_DOCK_Y=800,NG_ALT_DOCK_X=1792,NG_ALT_DOCK_Y=1024};
enum{NG_MAIN,NG_BUILD,NG_RUNS,NG_MEET,NG_RETURN,NG_CONFIRM,NG_SALVAGE,NG_COLLECTION};
enum{NG_IDLE,NG_STAGE,NG_COUNT,NG_RACE,NG_FETCH,NG_DROP};
static const char*const ngCategory[]={"PAINT","WHEELS","AERO","POWER","HANDLING","AUDIO","STYLE TAG"};
static const char*const ngPart[7][8]={
 {"Factory enamel","Petrol blue","Citron mica","Molten amber","Iris metallic","Oyster pearl","Carbon slate","Signal edition"},
 {"Factory alloy","Champagne mesh","Ice turbines","Ember spokes"},
 {"Factory body","Splitters + lip","Track package"},
 {"Factory tune","Street tune","Circuit tune"},
 {"Factory setup","Street springs","Track coils"},
 {"Factory radio","Single sub rack","Twin bass rack"},
 {"Factory badge","NEON / Lime","AMBER / Sleet","GLASS / Blue"}};
static const int ngParts[]={8,4,3,3,3,3,4},ngCost[]={38,65,95,115,90,95,35};
typedef struct {const char *name,*rival,*ride,*note;int type;NgMods mods;int seconds;NavPoint gate[5];} NgRun;
static const NgRun ngRun[]={
 {"COPPER RELAY","Ada / Breaker 02","Vectra / Champagne mesh","Signal-yellow fastback. Clean lines, no decals.",1,{2,1,1,1,1,1},39,{{1280,768},{1280,512},{768,512},{768,256},{512,256}}},
 {"SEAWALL RUN","Rue / Salt Index","Pallas / Ice turbines","A pearl omnibus built to cruise the waterfront.",2,{5,2,1,1,2,2},46,{{1280,768},{1536,768},{1536,1280},{1024,1280},{1024,1536}}},
 {"GLASS AFTER DARK","Ten / Ghost Static","Nightjar / Signal edition","Twin pinstripes and an exposed bronze sub rack.",3,{7,1,2,2,2,2},49,{{1280,768},{1280,256},{1792,256},{1792,768},{1536,768}}},
 {"BLUFFLIGHT FIVE","Mik / Billboard 07","Courier 4 / Ice turbines","Freight wagon in pearl-blue Signal livery.",4,{7,2,2,1,2,2},54,{{2690,384},{2388,471},{2197,411},{1792,512},{1792,1024}}},
 {"HARBOUR SPUR","Syd / Ferry 12","Mote / Ember spokes","Short wheelbase with a deep amber show build.",0,{3,3,1,2,2,1},52,{{1792,2304},{1792,1792},{1024,1792},{1024,2144},{1592,2464}}}};
static const NavPoint ngSites[]={{1280,768},{2690,384},{1792,2304}};
static const char*const ngSiteName[]={"Switchyard Social","Bluff Lights","Wharf After Dark"};
static int ng_courses(void){return ROOM>2560?LEN(ngRun):3;}
static int ng_best(int n){return n<3?32+n:47+n-3;}
static int ng_site_index(void){
 int a=(int)(nv.garage[53]%3u);
 if(ROOM<=2560)return 0;
 for(int i=0;i<3;i++,a=(a+1)%3)if(!solid(ngSites[a].x,ngSites[a].y,10))return a;
 return 0;
}
static struct {int tab,back,idx,model,part,val,route,bookedCourse,rival,stake,confirm,choice,event,leg,frames,count,ride,spare,paid,stageType,target,pending,meet,chosen,toastT,site,tokens,targetToken,sorting,altDock;float angle,spin,zoom;char toast[140];} ng;
static int ng_read(int i,int hi){U n=nv.garage[i];return n<=(U)hi?(int)n:0;}
static int ng_mod_slot(int type,int k){return k==6?55+cl(type,0,4):1+cl(type,0,4)*6+k;}
static int ng_mod_at(int type,int k){return ng_read(ng_mod_slot(type,k),ngParts[k]-1);}
static NgMods ng_installed(int type){
 return (NgMods){ng_mod_at(type,0),ng_mod_at(type,1),ng_mod_at(type,2),ng_mod_at(type,3),ng_mod_at(type,4),ng_mod_at(type,5),ng_mod_at(type,6)};
}
static int ng_active_mod(const Car*c,int k){return c==cars&&c->owner==1&&(unsigned)c->type<5&&(sv.owned&(1<<c->type))?ng_mod_at(c->type,k):0;}
static int ng_power(const Car*c){return ng_active_mod(c,3);}
static int ng_grip(const Car*c){return ng_active_mod(c,4);}
static int night_garage_audio(void){
 if(scene!=1||p.dead||!ng_active_mod(cars,5)||cars[0].cond<=0)return 0;
 if(p.car==0)return 1;
 return p.car<0&&absf(cars[0].s)<4&&d2(p.x-cars[0].x,p.y-cars[0].y)<(ng_active_mod(cars,5)==2?122*122:94*94)?2:0;
}
static int ng_dark(void){return sv.dayClock>=1140.f||sv.dayClock<300.f;}
static int night_garage_meet_site(int*x,int*y){
 int site=ng_site_index();*x=ngSites[site].x;*y=ngSites[site].y;return ng_dark()?site:-1;
}
static void ng_schedule_tick(void){
 if(ROOM<=2560)return;
 /* Legacy careers with no new feature state retain their original save
  * representation; once Motor Works opens, rotate only on the dusk edge.
  * The midnight continuation and time spent in menus retain the venue. */
 if(!ng_read(0,1)&&!nv.garage[54])return;
 if(ng_dark()){
  if(nv.garage[54]==2)nv.garage[53]=(nv.garage[53]+1)%3;
  nv.garage[54]=1;
 }else nv.garage[54]=2;
}
static void ng_reset(void){z(&ng,sizeof ng);ng.back=1;ng.target=ng.ride=ng.spare=-1;ng.zoom=1;ng.angle=.45f;ng_cam_reset();}
static void ng_msg(const char*m){char *d=ng.toast;for(int i=0;i<132&&*m;i++)*d++=*m++;*d=0;ng.toastT=320;}
static void ng_enter_panel(int tab,int origin){
 ng.tab=tab;ng.back=origin;ng.idx=0;ng.confirm=0;ng.toastT=0;ng.spin=0;
 if(ng.zoom<.7f||ng.zoom>1.32f)ng.zoom=1;
 ng.model=cl(sv.selection,0,4);scene=8;menu=-1;mapOn=0;walking=0;
}
static int night_garage_open(int building){if(building!=-1)return 0;ng_enter_panel(NG_MAIN,scene==3?3:1);return 1;}
static void ng_leave(void){scene=ng.back==3?3:1;menu=-1;ng.toastT=0;}
static void ng_to_street(void){
 if(ng.back==3&&interior_active_loc>=0){scene=3;int_leave();}else{scene=1;menu=-1;}
 ng.toastT=0;
}
static int ng_charge(int price){
 if(!canDirtyPurchase(price)){ng_msg(notice);return 0;}
 finish_purchase();return 1;
}
static int ng_price(int type,int cat,int val){
 if(val==ng_mod_at(type,cat))return 0;
 int price=val==0?0:ngCost[cat]*((cat<2||cat==6)?1:val);
 /* An arcade salvage credit discounts the next paid part. Never cashable. */
 return price&&ng_read(38,20)>0?max(10,price-20):price;
}
static void ng_preview(int type,int cat,int val,NgMods*out){
 *out=ng_installed(type);
 switch(cat){case 0:out->paint=val;break;case 1:out->rim=val;break;case 2:out->kit=val;break;case 3:out->tune=val;break;case 4:out->grip=val;break;case 5:out->sub=val;break;case 6:out->tag=val;break;}
}
static void ng_preview_reset(void){ng.val=ng_mod_at(ng.model,ng.part);}
static int ng_buy_part(void){
 int t=ng.model,k=ng.part,v=ng.val;
 if(!ng_read(0,1)){ng_msg("Buy the Motor Works deed first.");return 0;}
 if(!(sv.owned&(1<<t))){ng_msg("Preview only. Buy this model at the Nightglass desk first.");return 0;}
 if(k==0&&v==7&&!ng_read(37,1)){ng_msg("Signal edition: beat any Switchyard clock to unlock.");return 0;}
 if(v==ng_mod_at(t,k)){ng_msg("This part is already fitted.");return 0;}
 int price=ng_price(t,k,v);
 if(!ng_charge(price))return 0;
 if(price&&ng_read(38,20))nv.garage[38]--;
 nv.garage[ng_mod_slot(t,k)]=v;
 if((k==6||k==0)&&price>0)night_phone_heat_add((int)p.x,(int)p.y,-3);
 request_save();
 ng_msg(k==6?"Arcade style tag fitted. Local reputation cooled slightly.":k==5?"Audio rack fitted. Tune to an original station on your phone.":"Fitted to this model. Your other cars keep their own settings.");return 1;
}
static int ng_route_goal(int*x,int*y){
 if(ng.event==NG_STAGE||ng.event==NG_COUNT||ng.event==NG_RACE){
  NavPoint g=ngRun[ng.bookedCourse].gate[ng.event==NG_RACE?ng.leg:0];*x=g.x;*y=g.y;return 1;
 }
 if(ng.event==NG_FETCH&&ng.target>0&&ng.target<nc){*x=(int)cars[ng.target].x;*y=(int)cars[ng.target].y;return 1;}
 if(ng.event==NG_DROP){*x=ng.altDock?NG_ALT_DOCK_X:NG_DOCK_X;*y=ng.altDock?NG_ALT_DOCK_Y:NG_DOCK_Y;return 1;}return 0;
}
static void ng_clear_event(void){ng.event=NG_IDLE;ng.spare=ng.ride=ng.target=-1;ng.paid=0;ng.frames=0;ng.leg=0;navGoalKind=0;}
static void ng_stop(const char*why){ng_clear_event();tell(why);}
static int ng_free(void){
 if(job.kind>=0||vdActive>=0||ncr_active()){ng_msg("Finish or abandon your current city job before signing up.");return 0;}
 if(ng.event!=NG_IDLE){ng_msg("An event is already on your GPS. Finish it or use Cancel booking.");return 0;}
 return 1;
}
static int ng_spare(void){
 for(int i=0;i<5;i++)if(i!=sv.selection&&(sv.owned&(1<<i)))return i;return -1;
}
static int ng_begin_race(void){
 if(!ng_free())return 0;
 if(!sv.owned){ng_msg("Buy a driveable car at Nightglass Motors before entering.");return 0;}
 int stake=ng.meet?ng.stake:0,spare=stake==2?ng_spare():-1;
 if(stake==2&&spare<0){ng_msg("Title class needs a second owned model. Your driven car is never staked.");return 0;}
 if(stake==1&&sv.cash<75){ng_msg("Credit class needs $75 clean cash. Exhibition is free.");return 0;}
 if(stake==1)sv.cash-=75;
 if(stake==2){sv.owned&=~(1<<spare);}
 /* Once confirmed the course belongs to the booking, not the garage's
  * browsing cursor. Reopening Time Trials must not relocate paid gates. */
 ng.bookedCourse=ng.route;ng.spare=spare;ng.paid=stake;ng.event=NG_STAGE;ng.leg=ng.frames=ng.count=0;ng.ride=-1;
 navigation_follow_job();ng_to_street();request_save();tell("Switchyard booking: drive your owned car to the gold start gate.");return 1;
}
static int ng_begin_recovery(void){
 if(!ng_free())return 0;
 if(!ng_read(0,1)){ng_msg("Recovery contracts come with the Motor Works deed.");return 0;}
 float best=1.e10f;int id=-1;
 for(int i=1;i<nc;i++){Car*c=cars+i;if(c->owner||c->cond<=10||c->x<=32||c->y<=32||streetDrivers[i].live||i==jobRide)continue;
  float dist=d2(c->x-p.x,c->y-p.y)+(sv.owned&(1<<c->type)?250000.f:0);
  if(dist<best){id=i;best=dist;}
 }
 if(id<0){ng_msg("No parked target is available on this shift.");return 0;}
 ng.target=id;ng.stageType=cars[id].type;ng.frames=150*60;ng.event=NG_FETCH;
 navigation_follow_job();ng_to_street();tell("Parts call: collect the marked NPC car, then park at the Motor Works intake.");return 1;
}
static int ng_settle(int keep){
 int id=ng.target,t=ng.stageType;
 if(ng.event!=NG_DROP||id<=0||id>=nc||p.car!=id||cars[id].type!=t||cars[id].cond<=0){ng_msg("The recovery car is no longer available.");return 0;}
 if(keep&&(sv.owned&(1<<t))){ng_msg("This model is already registered. Trade it for parts instead.");return 0;}
 int price=specs[t].price*2/3;
 if(!keep&&sv.dirty>WALLET_LIMIT-42){ng_msg("Marked wallet is full. Exit the intake and make room for the $42 fee.");return 0;}
 if(keep&&(case_blocks_price(price))){ng_msg("Revenue hold: major purchases are frozen.");return 0;}
 if(keep&&sv.cash<price&&sv.dirty<price-sv.cash){ng_msg("Not enough credits for the registration fee.");return 0;}
 if(!street_exit_car()){ng_msg("Driver's door blocked. Move within the ring and stop again.");return 0;}
 if(keep){if(!ng_charge(price)){p.car=id;p.x=cars[id].x;p.y=cars[id].y;return 0;}
  sv.owned|=1<<t;vehicle_store_health(t,cars+id);
  if(sv.owned==(1<<t)){sv.selection=t;sv.vehicleCond=cars[id].cond;cars[0]=cars[id];cars[0].owner=1;cars[0].reported=0;}
 }
 /* Existing NPC slot is consumed, never an owned or story vehicle. */
 street_stage_car(id,(Car){.x=-100,.y=-100});lastCar=-1;
 int added=0;
 if(!keep){int stock=ng_read(38,20);added=min(3,20-stock);nv.garage[38]=(U)(stock+added);credit_wallet(0,42);}
 ng_clear_event();ng_to_street();request_save();
 if(keep)tell("Registered. Your recovered car is stored at Nightglass.");
 else{char out[100];char*q=append(out,"Dock handoff: $42 marked / +");q=number(q,added);
  append(q,added==1?" salvage credit.":" salvage credits.");tell(out);}
 return 1;
}
static void ng_fit_edition(int r,int saveNow){
 const NgRun*run=ngRun+r;int t=run->type;
 NgMods m=run->mods;int vals[]={m.paint,m.rim,m.kit,m.tune,m.grip,m.sub};
 for(int i=0;i<6;i++)nv.garage[1+t*6+i]=(U)vals[i];
 if(saveNow)request_save();
}
static void ng_finish_run(void){
 int ticks=max(1,ng.frames),r=ng.bookedCourse,win=ticks<=ngRun[r].seconds*60,stake=ng.paid,spare=ng.spare;
 int paid=0,best=ng_best(r);
 if(!ng_read(best,36000)||ticks<ng_read(best,36000))nv.garage[best]=(U)ticks;
 if(win){
  nv.garage[37]=1;nv.garage[40]=(U)min(9999,ng_read(40,9999)+1);
  if(stake==2&&spare>=0){
   sv.owned|=1<<spare;int t=ngRun[r].type;
   nv.garage[41+r]=1;
   if(!(sv.owned&(1<<t))){sv.owned|=1<<t;vehicle_store_health(t,&(Car){.cond=100});ng_fit_edition(r,0);}
  }else paid=credit_wallet(1,stake==1?185:42);
 }
 char msg[140];char*q=append(msg,win?"FINISH / WIN / ":"FINISH / OVER PACE / ");
 q=number(q,ticks/60);q=append(q,".");q=number(q,(ticks%60)/6);q=append(q," s. ");
 if(win&&stake!=2&&paid!=(stake==1?185:42)){
  q=append(q,"Wallet limit: $");q=number(q,paid);append(q," credited. Signal edition unlocked.");
 }else append(q,win?(stake==2?"Spare returned. Rival edition unlocked in Motor Works.":stake==1?"$185 purse + Signal edition paint.":"$42 + Signal edition paint unlocked."):
  (stake==2?"Staked spare lost. Driven car retained.":stake==1?"$75 entry lost.":"Record filed. No stake lost."));
 ng_clear_event();request_save();tell(msg);
}
/* Choose once at pickup so HUD/GPS does not flip between two moving targets
 * at an equidistant junction. Both service desks keep accepting the ride. */
static int ng_nearer_intake(int x,int y){
 if(d2(x-NG_ALT_DOCK_X,y-NG_ALT_DOCK_Y)<48*48)return 1;
 if(d2(x-NG_DOCK_X,y-NG_DOCK_Y)<48*48)return 0;
 int a=taxi_dispatch_distance(x,y,NG_DOCK_X,NG_DOCK_Y),b=taxi_dispatch_distance(x,y,NG_ALT_DOCK_X,NG_ALT_DOCK_Y);
 return b>0&&(!a||b<a);
}
static void night_garage_street(U tap){
 if(scene!=1||mapOn||menu>=0||p.dead)return;
 if(ng.event!=NG_IDLE&&(job.kind>=0||vdActive>=0)){ng_stop("Motor Works booking ended: another city job took priority.");return;}
 if(ng.event==NG_IDLE){
  int mx,my,site=night_garage_meet_site(&mx,&my);
  if((tap&ENTER)&&site>=0&&job.kind<0&&vdActive<0&&d2(p.x-mx,p.y-my)<58*58
     &&(p.car<0||absf(cars[p.car].s)<8)){
   ng.meet=1;ng.site=site;ng.rival=ng.stake=0;ng.route=site?site+2:0;
   if(!nv.garage[54]&&ROOM>2560){nv.garage[54]=1;request_save();}ng_enter_panel(NG_MEET,1);
  }return;
 }
 if(ng.event==NG_FETCH||ng.event==NG_DROP){
  if(--ng.frames<=0||ng.target<=0||ng.target>=nc||cars[ng.target].cond<=0||cars[ng.target].x<0){ng_stop("Parts call expired. No fee or title changed hands.");return;}
  if(ng.event==NG_FETCH&&p.car==ng.target){
   ng.altDock=ng_nearer_intake((int)p.x,(int)p.y);ng.event=NG_DROP;navGoalKind=0;
   tell(ng.altDock?"Nearest intake: Wrecker Yard. Park in the ring and press Enter.":"Nearest intake: Nightglass Motor Works. Park in the ring and press Enter.");
  }
  if(ng.event==NG_DROP&&(tap&ENTER)&&p.car==ng.target&&absf(cars[p.car].s)<8&&(d2(p.x-NG_DOCK_X,p.y-NG_DOCK_Y)<48*48||d2(p.x-NG_ALT_DOCK_X,p.y-NG_ALT_DOCK_Y)<48*48))ng_enter_panel(NG_RETURN,1);
  return;
 }
 const NgRun*rr=ngRun+ng.bookedCourse;NavPoint g=rr->gate[ng.event==NG_RACE?ng.leg:0];float dist=d2(p.x-g.x,p.y-g.y);
 if(ng.event==NG_STAGE){
  if(p.car==0&&cars[0].owner==1&&dist<39*39&&absf(cars[0].s)<12){ng.event=NG_COUNT;ng.count=180;ng.ride=0;ng.stageType=cars[0].type;tell("Hold the start ring. Three, two, one...");}return;
 }
 if(p.car!=ng.ride||cars[0].type!=ng.stageType||cars[0].cond<=0){ng_stop("Run ended: the registered car left the course. Entry stakes are forfeit.");return;}
 if(ng.event==NG_COUNT){
  if(dist>47*47){ng.event=NG_STAGE;ng.count=0;tell("False start. Stop inside the gold ring.");return;}
  if(--ng.count<=0){ng.event=NG_RACE;ng.frames=0;ng.leg=1;navGoalKind=0;tell("GO / Follow the gold gates in order. Beat the pace ghost!");}return;
 }
 ng.frames++;if(ng.frames>=(rr->seconds+35)*60){ng_stop("Course closed. No finish recorded; entry stake forfeit.");return;}
 if(dist<43*43){if(ng.leg==4){ng_finish_run();return;}ng.leg++;navGoalKind=0;particle(p.x,p.y,7,GOLD);}
}
static void ng_clock(char out[32],int tt){
 char*q=number(out,tt/60);q=append(q,".");*q++=(char)('0'+(tt%60)/6);*q++='s';*q=0;
}
static void ng_show(void){
 if(p.car!=0||!sv.owned||!ng_read(0,1)){ng_msg("Bring your owned car and a Motor Works build to show.");return;}
 NgMods m=ng_installed(cars[0].type);
 int score=!!m.paint+!!m.rim+m.kit+m.tune+m.grip+m.sub+!!m.tag;
 if(!score){ng_msg("Factory class: install a custom part before showing your car.");return;}
 /* Cooldown per playable 24-minute day, even if the profile clock wraps. */
 U stamp=sv.playedTicks/(DAY_SECONDS*60u)+1;if(nv.garage[36]==stamp){ng_msg("Your build already ran the light line this game day.");return;}
 int takings=25+7*score;
 if(sv.cash>WALLET_LIMIT-takings){ng_msg("Clean wallet is full. Make room for the showcase fee and return tonight.");return;}
 credit_wallet(1,takings);nv.garage[36]=stamp;nv.garage[39]=min(9999,ng_read(39,9999)+1);request_save();
 char out[100];char*q=append(out,"Showcase filed / ");q=number(q,score);q=append(q," style points / +$");number(q,takings);ng_msg(out);
}
static int ng_guest(int slot){return (slot+ng.site*2)%ng_courses();}
static int ng_archive(int slot){return ng_read(41+slot,1);}
static void ng_sort(void){
 if(ng.idx!=ng.targetToken){ng_msg("That card has a different mark. Read the intake slip and try again.");return;}
 /* The last stamp and the handoff are one operation. A blocked door
  * or full wallet must not advance the counter beyond the final stamp. */
 if(ng.tokens>=2){ng_settle(0);return;}
 ng.tokens++;
 ng.targetToken=(ng.stageType+ng.tokens*2)%3;ng_msg("Inventory stamp accepted. Match the next intake slip.");
}
static void ng_ask(int action){ng.choice=ng.tab;ng.tab=NG_CONFIRM;ng.confirm=action;ng.idx=0;ng.toastT=0;}
static void ng_ui_accept(void){
 if(ng.tab==NG_CONFIRM){
  int yes=ng.idx==1,act=ng.confirm;ng.tab=ng.choice;ng.idx=0;ng.confirm=0;if(!yes)return;
  if(act==1){ng_begin_race();return;}
  if(act==3){int t=ngRun[ng.chosen].type;if(ng_archive(ng.chosen)&&(sv.owned&(1<<t))){ng_fit_edition(ng.chosen,1);ng_msg("Rival edition applied to the stored model.");}return;}
  if(act==2){ng_stop("Booking cancelled. Any entry stake is forfeited.");ng_msg("Booking cancelled. Any entry stake is forfeited.");}return;
 }
 if(ng.tab==NG_MAIN){switch(ng.idx){
 case 0:if(!ng_read(0,1)){if(ng_charge(NG_DEED)){nv.garage[0]=1;request_save();ng_msg("Deed signed. Your lift, parts desk and five model slots are open.");}return;}
  if(!(sv.owned&(1<<ng.model))){ng_msg("You don't own this model. Buy it at the Nightglass desk.");return;}
  if(ng.event==NG_COUNT||ng.event==NG_RACE){ng_msg("Finish your run before deploying another car.");return;}
  doOpt(30+ng.model);ng_msg(notice);return;
 case 1:ng.tab=NG_BUILD;ng.part=ng.idx=0;ng_preview_reset();return;
 case 2:ng.tab=NG_RUNS;ng.meet=0;ng.route=ng.idx=0;ng.stake=0;return;
 case 3:ng_begin_recovery();return;
 case 4:{int x,y;night_garage_meet_site(&x,&y);nav_slot();navHasManual=1;navManualLoc=-1;navManual=(NavPoint){x,y};navGoalKind=0;
  ng_msg("Tonight's paddock is pinned. Enter beside the ring, 19:00 to 05:00.");return;}
 case 5:if(ng.event==NG_IDLE)ng_msg("No active booking.");else ng_ask(2);return;
 case 6:ng.tab=NG_COLLECTION;ng.idx=0;ng.chosen=0;return;}}
 if(ng.tab==NG_BUILD){if(ng.idx==7){ng.tab=NG_MAIN;ng.idx=1;return;}ng_buy_part();return;}
 if(ng.tab==NG_RUNS){
  if(ng.idx==0){ng.route=(ng.route+1)%ng_courses();return;}
  if(ng.idx==1&&ng.meet){ng.stake=(ng.stake+1)%3;return;}
  if(ng.idx==(ng.meet?2:1)){if(ng.meet&&ng.stake)ng_ask(1);else ng_begin_race();return;}
  ng.tab=ng.meet?NG_MEET:NG_MAIN;ng.idx=ng.meet?0:2;return;
 }
 if(ng.tab==NG_MEET){
  if(ng.idx==0){ng.rival=(ng.rival+1)%3;return;}
  if(ng.idx==1){ng_show();return;}
  if(ng.idx==2){ng.tab=NG_RUNS;ng.idx=0;ng.meet=1;ng.route=ng_guest(ng.rival);ng.stake=0;return;}
  ng_leave();return;
 }
 if(ng.tab==NG_RETURN){
  if(ng.idx==0){ng.tokens=0;ng.targetToken=ng.stageType%3;ng.idx=0;ng.tab=NG_SALVAGE;}
  else if(ng.idx==1)ng_settle(1);else ng_leave();return;
 }
 if(ng.tab==NG_SALVAGE){if(ng.idx==3){ng.tab=NG_RETURN;ng.idx=0;}else ng_sort();return;}
 if(ng.tab==NG_COLLECTION){
  if(ng.idx==0){ng.chosen=(ng.chosen+1)%ng_courses();return;}
  if(ng.idx==1){
   if(!ng_archive(ng.chosen)){ng_msg("Win the title class on this course to unlock its edition.");return;}
   if(!(sv.owned&(1<<ngRun[ng.chosen].type))){ng_msg("Win or buy the base model to apply this archived edition.");return;}
   ng_ask(3);return;
  }ng.tab=NG_MAIN;ng.idx=6;return;
 }
}
static void night_garage_tick(U keys,U tap){
 if(ng.toastT>0)ng.toastT--;
 if(tap&ESC){if(ng.tab==NG_MAIN||ng.tab==NG_MEET||ng.tab==NG_RETURN)ng_leave();
  else if(ng.tab==NG_CONFIRM){ng.tab=ng.choice;ng.idx=0;ng.confirm=0;}
  else if(ng.tab==NG_SALVAGE){ng.tab=NG_RETURN;ng.idx=0;}
  else{ng.tab=ng.tab==NG_RUNS&&ng.meet?NG_MEET:NG_MAIN;ng.idx=0;}return;}
 /* Shift + left/right controls orbit independently of the option list.
  * Up/down adjusts lens while Shift is held; rotational inertia is bounded. */
 if(keys&BRAKE){int steer=!!(keys&RIGHT)-!!(keys&LEFT),lens=!!(keys&UP)-!!(keys&DOWN);
  ng.spin+=(steer*.045f-ng.spin)*.15f;ng.zoom+=lens*.009f;if(ng.zoom<.72f)ng.zoom=.72f;if(ng.zoom>1.29f)ng.zoom=1.29f;
 }else{ng.spin*=.86f;if(absf(ng.spin)<.0009f)ng.spin=.001f;}
 ng.angle=wrap(ng.angle+ng.spin);
 if(keys&BRAKE)return;
 int rows=ng.tab==NG_MAIN?7:ng.tab==NG_BUILD?8:ng.tab==NG_RUNS?(ng.meet?4:3):ng.tab==NG_MEET?4:ng.tab==NG_RETURN||ng.tab==NG_COLLECTION?3:ng.tab==NG_SALVAGE?4:2;
 int dir=!!(tap&RIGHT)-!!(tap&LEFT),step=!!(tap&DOWN)-!!(tap&UP);
 if(step){ng.idx=(ng.idx+step+rows)%rows;if(ng.tab==NG_BUILD&&ng.idx<7){ng.part=ng.idx;ng_preview_reset();}}
 if(dir){
  if(ng.tab==NG_MAIN&&ng.idx<=1){ng.model=(ng.model+dir+5)%5;}
  if(ng.tab==NG_BUILD&&ng.idx<7){int n=ngParts[ng.part];ng.val=(ng.val+dir+n)%n;}
  if(ng.tab==NG_RUNS&&ng.idx==0){ng.route=(ng.route+dir+ng_courses())%ng_courses();}
  if(ng.tab==NG_RUNS&&ng.meet&&ng.idx==1)ng.stake=(ng.stake+dir+3)%3;
  if(ng.tab==NG_MEET&&ng.idx==0)ng.rival=(ng.rival+dir+3)%3;
  if(ng.tab==NG_COLLECTION&&ng.idx==0)ng.chosen=(ng.chosen+dir+ng_courses())%ng_courses();
 }
 if(tap&(USE|ENTER))ng_ui_accept();
}
static void ng_ellipse(int x,int y,int rx,int ry,U col){
 NgPt last={0};for(int k=0;k<=80;k++){float a=(k%80)*.07853982f;
 NgPt v={x+(int)(rx*cs(a)),y+(int)(ry*sn(a))};if(k)ng_l(last,v,col);last=v;}
}
static void ng_badge(int x,int y,const char*s,U c){int w=textw(s,0)+14;box(x-w/2,y,w,20,0x11202a,c);center(x,y-1,s,c,0);}
static int ng_ghost_car(Car*g){
 if(ng.event!=NG_RACE)return 0;
 /* Non-colliding pace projection, identical timing in both camera modes. */
 const NgRun*rr=ngRun+ng.bookedCourse;
 static const NavPoint bluff[]={
  {2690,384},{2598,380},{2530,437},{2460,481},{2388,471},{2332,421},
  {2263,382},{2197,411},{2144,482},{2048,512},{1792,512},{1792,1024}};
 static const NavPoint spur[]={
  {1792,2304},{1792,1792},{1024,1792},{1024,2048},{1024,2144},
  {1136,2265},{1360,2321},{1520,2368},{1592,2464}};
 const NavPoint*path=ng.bookedCourse==3?bluff:ng.bookedCourse==4?spur:rr->gate;
 int n=ng.bookedCourse==3?LEN(bluff):ng.bookedCourse==4?LEN(spur):LEN(rr->gate);
 float distance=0,lens[LEN(bluff)];for(int i=0;i<n-1;i++){lens[i]=sq(d2(path[i+1].x-path[i].x,path[i+1].y-path[i].y));distance+=lens[i];}
 float q=(float)ng.frames/(rr->seconds*60)*distance;int leg=0;while(leg<n-2&&q>lens[leg])q-=lens[leg++];
 float f=q/lens[leg];if(f>1)f=1;NavPoint a=path[leg],b=path[leg+1];
 int dx=b.x-a.x,dy=b.y-a.y;
 float heading=look(dx,dy);
 /* Blend the direction across the corner, not its timing. An instantaneous
  * yaw made the lane offset jump at a gate. (-sin, cos) is right of travel
  * in map coordinates, matching both directions of the physical traffic. */
 const float ease=16.f,remain=lens[leg]-q;
 if(leg>0&&q<ease){
  NavPoint prev=path[leg-1];float old=look(a.x-prev.x,a.y-prev.y);
  heading=wrap(old+wrap(heading-old)*(.5f+.5f*q/ease));
 }else if(leg<n-2&&remain<ease){
  NavPoint next=path[leg+2];float upcoming=look(next.x-b.x,next.y-b.y);
  heading=wrap(heading+wrap(upcoming-heading)*(.5f-.5f*remain/ease));
 }
 *g=(Car){a.x+dx*f-sn(heading)*7,a.y+dy*f+cs(heading)*7,heading,0,rr->type,2,100};return 1;
}
static void ng_world(void){
 if(scene!=1||menu>=0||mapOn||p.dead)return;
 int gx,gy;
 if(!ng_route_goal(&gx,&gy)){
  int mx,my,site=night_garage_meet_site(&mx,&my);if(site<0||job.kind>=0||vdActive>=0)return;
  int x=sx(mx),y=sy(my);if(x< -80||y< -45||x>W+80||y>H+50)return;
  ng_ellipse(x,y,44,33,0x43867c);ng_ellipse(x,y,39,29,0x42756a);
  for(int k=0;k<8;k++){float a=k*.7853982f;int px=x+(int)(43*cs(a)),py=y+(int)(32*sn(a));disc(px,py,2,k%2?GOLD:TEAL);}
  ng_badge(x,y-53,d2(p.x-mx,p.y-my)<58*58?"PADDOCK / STOP + ENTER":ngSiteName[site],TEAL);return;
 }
 Car ghost;
 if(ng_ghost_car(&ghost)){int x=sx(ghost.x),y=sy(ghost.y);
  if(x>-25&&x<W+25&&y>-20&&y<H+20){
   ng_ellipse(x,y,18,10,0x679f9a);NgRig rig={x,y,1,cs(ghost.a),sn(ghost.a),1,0};
   NgPt q0=ng_pt(&rig,12,0,0),q1=ng_pt(&rig,-9,-5,0),q2=ng_pt(&rig,-4,0,0),q3=ng_pt(&rig,-9,5,0);
   ng_l(q0,q1,TEAL);ng_l(q1,q2,TEAL);ng_l(q2,q3,TEAL);ng_l(q3,q0,TEAL);
  }
 }
 int x=sx(gx),y=sy(gy);
 if(x>-40&&x<W+40&&y>-30&&y<H+30){ng_ellipse(x,y,31,22,GOLD);ng_ellipse(x,y,27,18,0xa5824a);}
}
static void ng_gate_overlay(void){
 int gx,gy;if(scene!=1||menu>=0||mapOn||p.dead||!ng_route_goal(&gx,&gy))return;
 int x=sx(gx),y=sy(gy);
 ng_cam_point(&x,&y);
 char s[100];s[0]=0;
 if(ng.event==NG_RACE){char*q=append(s,ng.leg==4?"FINISH / ":"GATE ");if(ng.leg!=4){q=number(q,ng.leg);q=append(q," / ");}ng_clock(q,ng.frames);}
 else if(ng.event==NG_COUNT){fmt(s,"START IN ",(ng.count+59)/60);}
 else if(ng.event==NG_STAGE)append(s,"START / STOP HERE");
 else{char*q=append(s,ng.event==NG_DROP?"DELIVER / STOP + ENTER / ":"PARTS CALL / ");ng_clock(q,ng.frames);}
 int half=(textw(s,0)+16)/2;
 int xx=cl(x,half+16,W-half-16), yy=cl(y-44,149,H-106);
 if(yy+21>334&&xx+half>745)xx=745-half;
 ng_badge(xx,yy,s,GOLD);
}
static void ng_row(int row,const char*label,const char*value){
 int y=161+row*32,hot=ng.idx==row;
 rect(579,y,348,31,hot?0x283b3c:0x1a282e);if(hot)rect(579,y,3,31,0xe8ba75);
 textat(591,y+5,label,hot?PAPER:0xb4c0be,0);if(value)textat(917-textw(value,0),y+5,value,hot?GOLD:TEAL,0);
}
static void ng_stage(int t,NgMods m,int showSub){
 /* Local 2.5D studio: bay glass, structural truss, perspective tile floor,
  * turntable halo, a rail of alternating index ticks and a softer hub shadow. */
 rect(30,105,527,305,0x15252d);
 for(int i=0;i<8;i++){rect(30,105+i*14,527,14,mix(0x1c333b,0x0f1e27,i));}
 for(int i=0;i<8;i++){int x=35+i*72;line(x,106,x,210,0x263c42);line(x,108,x+57,190,0x2a4146);}
 rect(30,204,527,3,0x35504f);rect(30,207,527,3,0x172a31);
 for(int k=0;k<5;k++)rect(51+105*k,203,35,2,0x81a69d);
 for(int k=-4;k<=5;k++){int end=287+k*81;int top=287+k*29;line(top,212,cl(end,31,556),408,0x253840);}
 int yGrid[]={224,239,259,286,321,371};for(int k=0;k<6;k++)rect(31,yGrid[k],525,1,0x26383e);
 for(int j=-47;j<=47;j++){float f=(float)j/47;int h=(int)(161*sq(1-f*f));rect(288-h,304+j,2*h+1,1,0x1a2a30);}
 ng_ellipse(288,302,163,47,0x60877e);ng_ellipse(288,306,163,47,0x2e4447);
 for(int i=0;i<40;i++){float a=i*.15707963f;int x=288+(int)(155*cs(a)),y=302+(int)(44*sn(a));rect(x,y,i%5?2:4,1,i%5?0x607270:GOLD);}
 ng_model(288,295,ng.angle,7.25f*ng.zoom,1,t,specs[t].c,m,100,0,showSub);
}
static void night_garage_paint(void){
 U bg=0x101b22,amber=0xe8ba75;char tmp[120],buf[80];tmp[0]=buf[0]=0;
 rect(0,0,W,H,bg);rect(0,0,W,6,0xe1ac67);rect(25,23,3,58,amber);
 int meet=ng.tab==NG_MEET||(ng.tab==NG_RUNS&&ng.meet)||(ng.tab==NG_CONFIRM&&ng.meet);
 textat(42,16,meet?"EAST FREIGHT   /   NIGHT PROGRAMME":"NIGHTGLASS MOTORS   /   PORT VANTA",0x93aca7,0);
 textat(40,38,meet?ngSiteName[ng.site]:(ng.tab==NG_RETURN||ng.tab==NG_SALVAGE)?"Motor Works intake":"Motor Works",PAPER,1);
 const char*attr=meet?"MEET HOURS   19:00 / 05:00":"INDEPENDENT COACHWORKS";textat(923-textw(attr,0),26,attr,TEAL,0);
 char*q=append(buf,"WALLET  $");number(q,sv.cash);textat(923-textw(buf,0),53,buf,PAPER,0);
 box(25,99,537,326,0x26373c,0x344a50);box(571,99,364,326,0x152128,0x344248);
 int t=ng.model;NgMods m=ng_installed(t);const char*id="OWNER'S FLOOR";const char*note="Your parked collection lives at Nightglass Motors.";
 if(ng.tab==NG_MEET||ng.tab==NG_RUNS){int r=ng.tab==NG_MEET?ng_guest(ng.rival):ng.route;t=ngRun[r].type;m=ngRun[r].mods;
  id=ng.tab==NG_MEET?"GUEST BUILD":"PACE SETTER";note=ngRun[r].ride;}
 if(ng.tab==NG_BUILD&&ng.idx<7){ng_preview(t,ng.part,ng.val,&m);id="LIVE PART PREVIEW";note="Changes stay in preview until installed.";}
 if(ng.tab==NG_COLLECTION){t=ngRun[ng.chosen].type;m=ngRun[ng.chosen].mods;id="RIVAL EDITION ARCHIVE";note=ngRun[ng.chosen].ride;}
 if(ng.tab==NG_RETURN||ng.tab==NG_SALVAGE){t=ng.stageType;m=(NgMods){0};id="YARD INVENTORY";note="An intake removes only the marked NPC vehicle.";}
 ng_stage(t,m,1);
 textat(45,115,id,TEAL,0);textat(45,138,specs[t].name,PAPER,0);char lens[40];fmt(lens,"LENS  ",(int)(100*ng.zoom+.5f));
 append(lens+sl(lens),"%");textat(539-textw(lens,0),115,lens,0x9aaba9,0);textat(46,386,note,0xb1c0bc,0);
 const char*head=ng.tab==NG_MAIN?"WORK ORDER":ng.tab==NG_BUILD?"FITMENT LAB":ng.tab==NG_RUNS?"RUN SHEET":ng.tab==NG_MEET?"PADDOCK":ng.tab==NG_RETURN?"SETTLEMENT":ng.tab==NG_SALVAGE?"STOCK SORT":ng.tab==NG_COLLECTION?"EDITION ARCHIVE":"CONFIRM ENTRY";
 textat(590,108,head,amber,0);rect(590,140,326,1,0x384749);
 const char*desc="";const char*hint="UP / DOWN  SELECT     LEFT / RIGHT  CHANGE     ENTER  USE     ESC  BACK";
 if(ng.tab==NG_MAIN){
  ng_row(0,ng_read(0,1)?"Deploy stored car":"Buy garage deed",ng_read(0,1)?specs[ng.model].name:"$420");
  ng_row(1,"Build bay",specs[ng.model].name);ng_row(2,"Time trials",ng_courses()==3?"3 routes":"5 routes");ng_row(3,"Parts call","150s");ng_row(4,"Pin night meet","GPS");ng_row(5,"Cancel booking",ng.event?"ACTIVE":"NONE");ng_row(6,"Rival editions","COLLECTION");
  const char*about[]={"Own the lift and five storage slots. Select a model with Left / Right; Enter deploys it after purchase.",
   "Select a model with Left / Right. Preview its parts free, or fit them after buying the deed and the car.",
   "Free solo practice on three city routes, day or night. Beat the marked pace to unlock Signal edition.",
   "Collect the marked NPC car and return it before time expires. Choose parts or a paid registration.",
   "Tonight's paddock opens at 19:00. Stop in the ring and press Enter to inspect builds or race.",
   "Abandon an active event. Credit and title entries are nonrefundable.","Preview limited rival editions and apply a won build to your owned copy."};desc=about[ng.idx];
 }else if(ng.tab==NG_BUILD){
  for(int i=0;i<7;i++)ng_row(i,ngCategory[i],ngPart[i][ng.idx==i?ng.val:ng_mod_at(t,i)]);ng_row(7,"Return to desk",0);
  if(ng.idx<7){int cost=ng_price(t,ng.part,ng.val);char*q=append(tmp,ng.val==ng_mod_at(t,ng.part)?"Fitted / ":"Preview / ");q=append(q,ngPart[ng.part][ng.val]);
   q=append(q,".  ");if(ng.val==ng_mod_at(t,ng.part))append(q,"Use Left / Right to compare parts.");else{q=append(q,"Install for $");q=number(q,cost);append(q,". Enter confirms this part only.");}desc=tmp;
  }else desc="Go back to the main work order.";
 }else if(ng.tab==NG_RUNS){
  ng_row(0,"Course",ngRun[ng.route].name);int i=1;if(ng.meet)ng_row(i++,"Entry class",ng.stake==2?"SPARE TITLE":ng.stake==1?"$75 CREDIT":"EXHIBITION");
  ng_row(i++,"Book run","START GATE");ng_row(i,"Back",0);
  char*q=append(tmp,"Pace: ");q=number(q,ngRun[ng.route].seconds);q=append(q,"s. Best: ");int best=ng_read(ng_best(ng.route),36000);
  if(best){ng_clock(q,best);q+=sl(q);}else q=append(q,"unposted");
  append(q,!ng.meet||ng.stake==0?". Free entry, $42 for beating the clock.":ng.stake==1?". $75 entry; $185 purse for beating the clock.":". Title class risks one SPARE car on the result.");desc=tmp;
 }else if(ng.tab==NG_MEET){
  ng_row(0,"Guest car",ngRun[ng_guest(ng.rival)].rival);ng_row(1,"Show your build","LIGHT LINE");ng_row(2,"Enter rival run","RACE DESK");ng_row(3,"Leave meet",0);
  desc=ng.idx==0?ngRun[ng_guest(ng.rival)].note:ng.idx==1?"Show a custom owned car. Score its paint, wheels, aero, tune, grip and sound once per game day.":
    ng.idx==2?"Choose a course, then free exhibition, credit stake or optional spare-title class.":"Return to the street.";
 }else if(ng.tab==NG_RETURN){
  ng_row(0,"Trade for parts","3 CREDITS");fmt(buf,"$",specs[t].price*2/3);ng_row(1,"Register model",buf);ng_row(2,"Move the car",0);
  desc="Sort the fictional stock cards for $42 and three $20 workshop discounts. Registration stores this car instead.";
 }else if(ng.tab==NG_SALVAGE){
  static const char *tokens[]={"CIRCLE / BLUE","DIAMOND / AMBER","SQUARE / GREEN"};
  for(int i=0;i<3;i++)ng_row(i,tokens[i],"SORT");
  ng_row(3,"Leave sorting desk",0);
  char*q=append(tmp,"INTAKE SLIP / ");q=number(q,ng.tokens+1);q=append(q," OF 3 / Match the ");q=append(q,tokens[ng.targetToken]);append(q," stock card.");desc=tmp;
 }else if(ng.tab==NG_COLLECTION){
  ng_row(0,"Edition",ngRun[ng.chosen].name);ng_row(1,"Fit complete edition",ng_archive(ng.chosen)?"UNLOCKED":"LOCKED");ng_row(2,"Return to desk",0);
  desc=ng_archive(ng.chosen)?"Won in a title-class run. Take it to your car or leave your current parts untouched.":"Win the title class on this course to earn this limited build.";

 }else{
  ng_row(0,"Back / keep everything",0);ng_row(1,"Confirm","ENTER");
  if(ng.confirm==3)desc="Apply the complete archived edition? This replaces the fitted parts on that stored model only.";
  else if(ng.confirm==2)desc="Cancel the active booking? Any committed entry stake is lost.";
  else if(ng.stake==1)desc="Risk $75 clean cash now. Losing, quitting or cancelling forfeits it. Win $185 if you beat the pace.";
  else if(ng_spare()<0)desc="No spare model available. Select a different entry class.";
  else{char*q=append(tmp,"Stake your stored ");q=append(q,specs[ng_spare()].name);append(q," now. Losing, quitting or cancelling costs that title. Your driven car stays yours.");desc=tmp;}
 }
 rect(25,434,910,51,0x1d2c32);rect(25,434,3,51,ng.toastT?TEAL:amber);
 wrapped(40,437,ng.toastT?ng.toast:desc,ng.toastT?TEAL:PAPER,104);
 textat(27,491,hint,0xc6d0c8,0);textat(27,513,"HOLD SHIFT + LEFT / RIGHT  ORBIT 360     SHIFT + UP / DOWN  ZOOM",0x88a49e,0);
}
