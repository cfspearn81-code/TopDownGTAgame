/* Compile beside integrated game.c: gcc -std=c11 -O1 src/night_phone_test.c -o build/night_phone_test. */
#include "game.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned char np_test_disk[16384];
static unsigned np_test_size;
int disk_write(const void*d,unsigned n){assert(n<=sizeof np_test_disk);memcpy(np_test_disk,d,n);np_test_size=n;return 1;}
int disk_read(void*d,unsigned n){if(n!=np_test_size)return 0;memcpy(d,np_test_disk,n);return 1;}
static void np_goto(int i){p.x=loc[i].x;p.y=loc[i].y;p.car=-1;}
static void np_press(int app,int row){if(app!=np_app||row!=np_row)np_confirm=0;np_app=app;np_row=row;night_phone_tick(0,ENTER);}
static void np_ticks(int n){for(int i=0;i<n;i++)night_phone_world_tick(1.f/60.f);}
static void np_new_cards(void){
 NPLine l;night_phone_open();np_app=NP_WIRE;
 np_line(&l,6);assert(strstr(l.label,"Customer DM")&&strstr(l.detail,"Pays $"));
 np_press(NP_WIRE,6);assert(np_app==NP_INBOX&&np_row==1);
 np_app=NP_WIRE;np_line(&l,8);assert(strstr(l.label,"Freight Parkway"));
 int x=0,y=0;night_garage_meet_site(&x,&y);np_press(NP_WIRE,1);assert(navHasManual&&navManual.x==x&&navManual.y==y);
 sv.dayClock=700;np_press(NP_WIRE,7);assert(nav_cart_tracking()&&!navCartOpen&&navCartClock==150);
 sv.dayClock=1200;int vx,vy;assert(night_venue_vendor_pos(&vx,&vy));nav_update();assert(nav_cart_tracking()&&navCartOpen&&navManual.x==vx&&navManual.y==vy);
 np_press(NP_WIRE,10);assert(navManualLoc==12);
 np_press(NP_RADIO,5);assert(!night_phone_take_link()&&np_confirm);
 night_phone_tick(0,UP);assert(!night_phone_take_link()&&!np_confirm);
 np_press(NP_RADIO,5);np_press(NP_RADIO,5);assert(night_phone_take_link()==1&&!night_phone_take_link());night_phone_link_result(0);assert(strstr(np_note,"https://music.youtube.com/"));
 np_press(NP_RADIO,5);np_press(NP_RADIO,5);assert(night_phone_take_link());night_phone_link_result(1);assert(strstr(np_note,"plays there"));
 night_phone_tick(0,ESC);assert(!night_phone_take_link());
}
static void np_arcade_recovery(void){
 int x=(int)p.x,y=(int)p.y,a=night_phone_heat_at(x,y),hold=(int)p.heat;assert(a>45);
 night_phone_rest(2);assert(night_phone_heat_at(x,y)==a-6&&(int)p.heat==hold);
 night_phone_open();np_press(NP_PREF,2);assert(np_get(NP_SIM,3)==1);
 np_press(NP_PREF,3);assert(np_get(NP_SIM,3)==0&&np_get(NP_SIM_COOL,3600)>0&&np_get(NP_ALIAS,3)==1);
 assert(night_phone_heat_at(x,y)==a-21&&(int)p.heat==hold);
 np_press(NP_PREF,2);np_press(NP_PREF,3);assert(np_get(NP_SIM,3)==1&&night_phone_heat_at(x,y)==a-21);
 request_save();z(&nv,sizeof nv);load_save();assert(np_get(NP_SIM,3)==1&&np_get(NP_SIM_COOL,3600)>0);
 night_phone_tick(0,ESC);np_ticks(3600);assert(np_get(NP_SIM_COOL,3600)==0);
}
static void np_roadways(void){
 sv.dayClock=5.5f*60;assert(!np_closed()&&!night_phone_lane_stop(2452,1024,2457,1024,8));
 sv.dayClock=6*60;assert(np_closed()&&night_phone_lane_stop(2452,1024,2457,1024,8));
 assert(night_phone_lane_stop(2476,1024,2471,1024,8));
 assert(!night_phone_lane_stop(2452,1024,2449,1024,8));
 assert(!night_phone_lane_stop(2464,1024,2458,1024,8));
 assert(!night_phone_lane_stop(2452,1110,2475,1110,8));
 sv.dayClock=8*60;assert(!np_closed());
 sv.dayClock=16*60;assert(np_closed());
 sv.dayClock=18*60;assert(!np_closed());
}
#ifdef BASE_ROOM
static void np_live_lane(void){
 scene=1;mapOn=0;menu=-1;sv.dayClock=370;p.x=2431;p.y=1024;p.car=-1;p.dead=0;p.heat=0;
 int pre=night_phone_heat_at(2464,1024);
 night_phone_heat_add(2464,1024,10);assert(night_phone_heat_at(2464,1024)==pre+10&&np_area(2464,1024)==7);
 assert(np_area(1800,1024)==5);
 for(int i=0;i<50;i++)tick(RIGHT);
 assert(p.x<=2457.f&&p.x>=2444.f);
 sv.dayClock=481;
 for(int i=0;i<35;i++)tick(RIGHT);
 assert(p.x>2468.f);
 Car current= {2420,1024,0,0,0,1,100,0};
 street_stage_car(0,current);p.car=0;p.x=2420;p.y=1024;sv.dayClock=370;prev=0;
 for(int i=0;i<80;i++)tick(UP);
 assert(cars[0].x<=2448.f&&cars[0].x>=2436.f&&cars[0].cond==100);
 sv.dayClock=481;
 for(int i=0;i<48;i++)tick(UP);
 assert(cars[0].x>2468.f&&cars[0].cond==100);
}
#endif
static void np_batch_modes(void){
 np_goto(4);night_phone_open();nv.phone[NP_NEWTYPE]=nv.phone[NP_NEWGRADE]=0;
 np_press(NP_LOFT,9);assert(np_profile_mode()==1);np_press(NP_LOFT,3);assert(np_active_left()==28*60&&nv.phone[NP_BATCH_MODE]==1);
 np_press(NP_LOFT,9);assert(np_profile_mode()==2&&nv.phone[NP_BATCH_MODE]==1);
 night_phone_tick(0,ESC);np_ticks(28*60);
 night_phone_open();np_press(NP_LOFT,4);assert(nv.phone[NP_SHELF]==2);
 int h=night_phone_heat_at(loc[4].x,loc[4].y);nv.phone[NP_NEWGRADE]=1;
 nv.phone[NP_MODULES]=7;np_press(NP_LOFT,3);assert(np_active_left()==22*60&&nv.phone[NP_BATCH_MODE]==2&&night_phone_heat_at(loc[4].x,loc[4].y)>h);
 night_phone_tick(0,ESC);np_ticks(22*60);night_phone_open();np_press(NP_LOFT,4);assert(nv.phone[NP_SHELF+1]==4);
 night_phone_tick(0,ESC);assert(night_phone_signature(0,0)==2&&!night_phone_signature(1,1)&&!night_phone_signature(-1,1)&&night_phone_signature(0,0)==2);
}
static void np_highgrade_patrol(void){
 night_phone_open();np_accept(6);night_phone_tick(0,ESC);np_goto(8);
 night_phone_heat_add((int)p.x,(int)p.y,100);int dirty=sv.dirty;
 assert(night_phone_action(USE)&&np_get(NP_PATROL,1)&&np_get(NP_HOLD,600)==360&&p.heat>=24);
 for(int frame=0;frame<100;frame++)dispatch_tick(1.f/60.f);assert(dispatch_engaged()>0);
 np_ticks(360);assert(night_phone_action(USE)&&np_get(NP_RIVAL,1)&&np_get(NP_HOLD,600)==600&&night_phone_signature(0,0)==2);
 np_ticks(600);assert(night_phone_action(USE)&&np_job()<0&&night_phone_signature(0,0)==1&&sv.dirty==dirty+305);
}

static void np_hit(U k){tick(0);tick(k);tick(0);}
static void np_receipts_and_harbor(void){
 /* Ordinary keyboard/pointer path from a named room, not a forced phone scene. */
 scene=1;menu=-1;mapOn=0;job.kind=-1;vdActive=-1;vdPanel=0;p.car=-1;p.dead=0;
 nv.phone[NP_JOB]=0;nv.phone[HJ_PHASE]=nv.phone[HJ_DONE]=nv.phone[HJ_FLOAT]=0;
 /* A full pop-up gift must not swallow the shop door's ordinary E. */
 sv.dayClock=700.f;nv.phone[NP_POPSTAMP]=0;nv.phone[NP_CARD]=1;
 U old_stock=nv.phone[NP_BAG];nv.phone[NP_BAG]=99;
 np_goto(2);np_hit(USE);assert(scene==3&&interior_active_loc==2&&nv.phone[NP_CARD]==1&&nv.phone[NP_BAG]==99);
 nv.phone[NP_CARD]=0;nv.phone[NP_BAG]=old_stock;
 scene=1;np_goto(4);np_hit(USE);assert(scene==3&&interior_active_loc==4);
 float ix=interior_x,iy=interior_y;U key=night_phone_toggle(PHONE)?PHONE:PHONE_ALT;
 np_hit(key);assert(scene==NIGHT_PHONE_SCENE&&np_app==NP_INBOX&&np_return==3);
 np_hit(RIGHT);np_hit(RIGHT);assert(np_app==NP_WIRE&&np_rows()==18&&np_row==0);
 /* Every card is reachable on the original seven-app ring. */
 for(int i=17;i>=0;i--){
  NPLine l;np_hit(UP);assert(np_row==i);
  np_line(&l,i);assert(l.label[0]&&l.detail[0]);
  if(i==NP_WIRE_WOODLINE)assert(strstr(l.label,"Ranger"));
  if(i>=NP_WIRE_HARBOR)assert(strstr(l.label,"Harbor"));
  night_phone_paint();
 }
 np_hit(UP);np_hit(UP);np_hit(UP);assert(np_row==NP_WIRE_HARBOR+HJ_BLUE);
 np_hit(ENTER);assert(hj_phase()==HJ_IDLE&&np_confirm);
 np_hit(MAP);assert(np_app==NP_RECEIPTS&&np_rows()==RRC_ROWS&&!np_confirm);
 np_hit(MAP);assert(np_app==NP_WIRE&&np_row==NP_WIRE_HARBOR+HJ_BLUE&&!np_confirm);
 np_hit(ENTER);assert(hj_phase()==HJ_IDLE&&np_confirm);
 np_hit(ENTER);assert(hj_phase()==HJ_BLUE_CLUE&&!np_confirm);
 /* Both populations of extension fields must coexist, including nonzero receipts. */
 sv.dayClock=704.f;
 nv.garage[WF_FLAGS]=WF_SURVEY_BEGIN|(1u<<WF_PATCH_SHIFT);
 wf_inventory(2,4);wf_bed_write(0,0,1,900);wf_bed_write(1,1,6,2040);
 rr_record(RR_PURCHASE,240,0,0);rr_record(REVENUE_THRESHOLD,0,37,79);
 U f[4];for(int i=0;i<4;i++)f[i]=nv.garage[WF_FLAGS+i];
 U phase=nv.phone[HJ_PHASE],filed=nv.phone[HJ_DONE],token=nv.phone[HJ_FLOAT];
 request_save();assert(np_test_size==sizeof(Save11));
 z(&nv,sizeof nv);load_save();assert(wason);
 assert(nv.phone[HJ_PHASE]==phase&&nv.phone[HJ_DONE]==filed&&nv.phone[HJ_FLOAT]==token&&hj_phase()==HJ_BLUE_CLUE);
 for(int i=0;i<4;i++)assert(nv.garage[WF_FLAGS+i]==f[i]);
 assert(wf_seen(0)&&wf_inv(2)==4&&wf_bed_crop(0,0)==1&&wf_bed_left(1,1)==2040);
 RevenueLog sale=rr_read(1),loss=rr_read(0);
 assert(sale.kind==RR_PURCHASE&&sale.clean==240&&loss.kind==REVENUE_THRESHOLD&&loss.dirty==37&&loss.pending==79);
 NPLine l;np_line(&l,np_row);assert(strstr(l.suffix,"IN PROGRESS"));
 pointer_input(RR_SIGNAL_X+8,RR_SIGNAL_Y+5,1);assert(np_app==NP_RECEIPTS);
 np_line(&l,1);assert(strstr(l.detail,"Dirty -$37")&&strstr(l.detail,"Unclaimed settlements -$79"));
 np_hit(LEFT);assert(np_app==NP_WIRE&&np_row==NP_WIRE_HARBOR+HJ_BLUE&&hj_phase()==HJ_BLUE_CLUE);
 np_hit(ESC);assert(scene==3&&interior_active_loc==4&&interior_x==ix&&interior_y==iy);
}
static void np_import_old_receipt(void){
 /* A genuine pre-merge 1044 layout has seizure fields at garage 49-52,
  * and no data in the new holes. It should import without phantom crops. */
 Save11 old;
 assert(np_test_size==sizeof old);memcpy(&old,np_test_disk,sizeof old);
 old.night.phone[DC_SEIZURE_META]=0;old.night.phone[DC_SEIZURE_CLEAN]=0;
 old.night.garage[DC_SEIZURE_DIRTY]=0;old.night.phone[DC_SEIZURE_PENDING]=0;
 old.night.garage[DC_SEIZURE_LEGACY_META]=RR_MAGIC|(REVENUE_THRESHOLD<<RR_TIME_BITS)|708u;
 old.night.garage[DC_SEIZURE_LEGACY_CLEAN]=0;
 old.night.garage[DC_SEIZURE_LEGACY_DIRTY]=403;
 old.night.garage[DC_SEIZURE_LEGACY_PENDING]=79;
 for(int i=HJ_PHASE;i<=HJ_FLOAT;i++)old.night.phone[i]=0;
 old.hash=night_hash(&old,sizeof old-4);
 assert(disk_write(&old,sizeof old));load_save();assert(wason);
 RevenueLog s=rr_read(0);assert(s.kind==REVENUE_THRESHOLD&&s.dirty==403&&s.pending==79&&s.minute==708);
 for(int k=0;k<4;k++)assert(!nv.garage[WF_FLAGS+k]);
 assert(hj_phase()==HJ_IDLE);request_save();load_save();assert(wason&&rr_read(0).dirty==403);
}
int main(int argc,char **argv){
 init_city();scene=1;menu=-1;mapOn=0;p.car=-1;sv.cash=4000;sv.dirty=0;
 for(int k=0;k<12;k++)night_phone_world_tick(1.f/60.f);for(int i=0;i<64;i++)assert(nv.phone[i]==0);
 night_phone_open();assert(scene==9);NPLine first;np_line(&first,0);assert(strstr(first.suffix,"$105")&&strstr(first.suffix,"CLEAN"));
 np_pin(-1,123,456);np_press(NP_INBOX,0);assert(navHasManual&&navManual.x==123&&navManual.y==456);assert(np_job()==0&&np_job_dest()==2);
 night_phone_tick(0,ESC);assert(scene==1);np_goto(2);assert(night_phone_action(USE));np_goto(0);assert(night_phone_action(USE));assert(np_job()<0&&sv.cash==4105&&navHasManual&&navManual.x==123);
 nv.phone[NP_DONE]=0;night_phone_open();np_press(NP_LOFT,0);assert(!sv.b[0].own);np_press(NP_LOFT,0);assert(sv.b[0].own&&sv.cash==1105);
 np_press(NP_LOFT,3);assert(np_active_batch()==0);night_phone_tick(0,ESC);
#if __has_include("night_world.inc")
 night_phone_world_tick(1.f/60);int snapTimer=np_active_left();request_save();assert(np_test_size==sizeof(Save11));z(&nv,sizeof nv);load_save();assert(wason&&np_active_batch()==0&&np_active_left()==snapTimer);
#endif
 for(int i=0;i<18*60;i++)night_phone_world_tick(1.f/60);night_phone_open();np_press(NP_LOFT,4);assert(np_active_batch()==0&&!nv.phone[NP_SHELF]);night_phone_tick(0,ESC);np_goto(4);
 night_phone_open();np_press(NP_LOFT,4);assert(np_active_batch()<0&&nv.phone[NP_SHELF]==3);np_press(NP_STOCK,0);assert(nv.phone[NP_BAG]==3&&!nv.phone[NP_SHELF]);
 np_press(NP_INBOX,1);assert(np_job()==1&&np_job_dest()==12);night_phone_tick(0,ESC);np_goto(12);
#if __has_include("night_world.inc")
 request_save();z(&nv,sizeof nv);load_save();assert(wason&&np_job()==1&&np_get(NP_BAG,99)==3);
#endif
 int sx=0,sy=0;assert(night_phone_route(&sx,&sy)&&sx==loc[12].x&&sy==loc[12].y);
 night_phone_heat_add(sx,sy,70);assert(night_phone_action(USE)&&np_job()==1&&nv.phone[NP_HOLD]);assert(night_phone_action(USE));for(int i=0;i<600;i++)night_phone_world_tick(1.f/60);
 assert(night_phone_action(USE));assert(np_job()<0&&nv.phone[NP_BAG]==1&&sv.dirty==154);assert(night_phone_heat_at(sx,sy)>70);
 night_phone_open();np_press(NP_LOFT,2);assert(np_sel()==1);np_press(NP_LOFT,3);assert(np_active_batch()<0);np_press(NP_LOFT,2);np_press(NP_LOFT,7);np_press(NP_LOFT,3);assert(np_active_batch()==2);
 night_phone_tick(0,ESC);np_goto(4);for(int i=0;i<34*60;i++)night_phone_world_tick(1.f/60);night_phone_open();np_press(NP_LOFT,4);np_press(NP_STOCK,2);assert(night_phone_premium(0)==3);assert(night_phone_premium(1)&&night_phone_premium(0)==2);
 np_press(NP_RADIO,3);assert(night_phone_radio()==3);np_press(NP_RADIO,4);p.x=350;p.y=350;assert(night_phone_radio()==1);
 np_press(NP_PREF,0);assert(!night_phone_toggle(PHONE)&&night_phone_toggle(PHONE_ALT));sv.dayClock=700;np_press(NP_WIRE,3);night_phone_tick(0,PHONE_ALT);assert(scene==1);np_goto(2);assert(night_phone_action(USE)&&nv.phone[NP_BAG]==2);night_phone_open();np_press(NP_WIRE,3);night_phone_tick(0,ESC);assert(!night_phone_action(USE));
 np_roadways();np_new_cards();np_batch_modes();np_highgrade_patrol();np_arcade_recovery();
 sv.playedTicks+=DAY_SECONDS*60u;sv.dayClock=23*60;night_phone_open();np_press(NP_WIRE,3);night_phone_tick(0,ESC);np_goto(7);assert(night_phone_action(USE)&&np_get(NP_BAG+3,99)==1);
 night_phone_open();for(np_app=0;np_app<NP_NAPP;np_app++)for(np_row=0;np_row<np_rows();np_row++)night_phone_paint();
 np_app=NP_INBOX;np_row=0;night_phone_pointer(100,144+38*5,1);assert(np_app==NP_RADIO);night_phone_pointer(400,170+32,1);night_phone_pointer(400,170+32,1);assert(night_phone_radio()==1);
 if(argc>1){np_app=NP_WIRE;np_row=1;np_note[0]=0;night_phone_paint();FILE*f=fopen(argv[1],"wb");assert(f);fprintf(f,"P6\n%d %d\n255\n",W,H);for(int i=0;i<W*H;i++){int pix=pixels[i];fputc(pix>>16,f);fputc(pix>>8,f);fputc(pix,f);}fclose(f);}
#ifdef BASE_ROOM
 np_live_lane();
#endif
 np_receipts_and_harbor();np_import_old_receipt();
 printf("PASS: merged receipts, 18 accessible cards, active harbor case, planter save, named room return, handset apps, original ownership, timed cabinet, exact grades, satchel, rivals, cash, doors, weather feed, station routing, binding, rotating pop-ups, explicit browser handoff, physical closure and district patrol, all rows painted.\n");return 0;
}
