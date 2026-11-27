/* The night desk: world art, lettering and news are all local to this career
 * session. Only ordinary earnings and the career's lawful-work count are saved. */
#define VD_HAS_B4 1
#define VD_HAS_B7 1
#define VD_HAS_B8 1

enum { VM_RECORDS=43,VM_THEATRE=28,VM_DINER=48,VM_STUDIO=38,VM_PRESS=61 };
typedef struct { const char *title; int art; } VmStory;
enum { VM_AMPLIFIER,VM_TABLE,VM_VAN,VM_HOOK,VM_ENVELOPE,VM_CAB,VM_FLAG,VM_TYPE,VM_SKY };
typedef struct {
 int blockPhase,booking,blockBill,newsPhase,event,newsPage,van,returnVan,pan,hold,phaseT,wasInside;
 int printOffset1,printOffset2;
 int lastAirTitle,lastAirTicks;
 int prevKind,prevLegal,prevCrime,historyN,impression,printPhase,plateTick,dx,dy,quality,folio;
 VmStory history[2],edition[2],working,proof;
} VmMedia;
static VmMedia vm;
static const int vmVenue[]={VM_THEATRE,VM_DINER};
static const VmStory vmBill[]={{"VELVET STATIC",VM_AMPLIFIER},{"THE LONG TABLE",VM_TABLE}};
static const VmStory vmLive[]={{"ENCORE AT COPPER",VM_AMPLIFIER},{"LAST HOOK HOME",VM_HOOK}};
static const VmStory vmArchive[]={{"FERRY AT FIRST LIGHT",VM_SKY},{"THE NIGHT EDITION",VM_TYPE}};
static const VmStory vmJobs[]={
 {"SIGNED AT MERCURY",VM_ENVELOPE},{"METER AFTER DARK",VM_CAB},{"HOOK & HAUL",VM_HOOK},{"THE GLASS MILE",VM_FLAG},
 {"THE VAULT RUN",VM_ENVELOPE},{"HOT PROPERTY",VM_CAB},{"SIGNAL RAID",VM_AMPLIFIER},{"EAST FREIGHT",VM_TYPE}
};
static const U vmInk[]={0xdb5577,0x3da594,0x507aaa,0xd38338,0x6a8ea0,0xb48d36,0xa16ba4,0x5b9588,0x777daa};
static void vm_history(VmStory s){
 if(vm.historyN&&s.title==vm.history[0].title)return;
 if(vm.historyN)vm.history[1]=vm.history[0];
 vm.history[0]=s;vm.historyN=min(2,vm.historyN+1);
}
static void vm_goal_room(int i){vd_goal(ordinaryDoorX[i],ordinaryDoorY[i],walkin_name(i));}
static int vm_station(int i){return scene==4&&walkin_active==i&&wi_at_station();}
static int vm_free(void){return vdActive<0&&job.kind<0&&sv.intro>=3;}
static void vm_hint(const char *label){rect(19,482,W-38,24,DARK);textat(26,484,label,GOLD,0);}
static void vm_receipt(VmStory s,int amount,const char*label){
 char msg[98];int accepted=credit_wallet(1,amount);if(sv.legal<1000000000)sv.legal++;
 char*q=append(msg,label);q=append(q,s.title);q=append(q,"  /  $");number(q,accepted);tell(msg);noticeInk=CASH_GREEN;
 request_save();vd_end();
}
static void vmbracket(int x,int y,int w,int h,int n,U c){
 for(int i=0;i<2;i++)for(int j=0;j<2;j++){
  int u=x+i*(w-1),v=y+j*(h-1);
  line(u,v,u+(i?-n:n),v,c);line(u,v,u,v+(j?-n:n),c);
 }
}
/* All the screen-print plates are drawn twice. The offset of the orange
 * impression is visible even on a badly registered print. Art never relies
 * on the key colour alone. x,y center; units in native pixels. */
static void vm_stamp(int a,int x,int y,int u,U ink){
 #define VX(t) (x+(t)*u)
 #define VY(t) (y+(t)*u)
 #define VS(ax,ay,bx,by) line(VX(ax),VY(ay),VX(bx),VY(by),ink)
 #define VB(ax,ay,w,h) rect(VX(ax),VY(ay),(w)*u,(h)*u,ink)
 #define VO(ax,ay,r) disc(VX(ax),VY(ay),(r)*u,ink)
 switch(a){
 case VM_AMPLIFIER:
  VB(-9,-7,9,13);VO(-5,-2,3);VS(3,-7,6,-10);VO(7,-11,2);VS(3,-7,3,6);VS(-1,8,3,6);VS(3,6,7,8);break;
 case VM_TABLE:
  VB(-10,2,20,2);VB(-8,4,2,5);VB(6,4,2,5);
  for(int i=-1;i<=1;i++){VO(i*6,-5,2);VB(i*6-2,-2,4,4);}break;
 case VM_HOOK:
  VB(-9,4,14,3);VB(-9,0,5,4);VO(-6,8,2);VO(3,8,2);
  VS(-3,3,4,-8);VS(-2,3,5,-8);VS(5,-8,9,-8);VS(9,-8,9,0);VS(9,0,7,2);VS(7,2,5,0);break;
 case VM_ENVELOPE:
  VB(-11,-6,22,1);VB(-11,6,22,1);VB(-11,-6,1,13);VB(10,-6,1,13);VS(-10,-5,0,2);VS(0,2,10,-5);break;
 case VM_CAB:case VM_VAN:
  VB(-11,0,22,5);VB(-7,-4,12,4);VB(-3,-7,5,3);VO(-6,6,2);VO(6,6,2);break;
 case VM_FLAG:
  VB(-7,-10,1,19);for(int j=0;j<3;j++)for(int i=0;i<5;i++)if((i+j)%2==0)VB(-6+i*3,-10+j*3,3,3);break;
 case VM_SKY:
  VO(5,-6,3);VB(-9,2,19,2);VB(-6,0,11,2);VS(-6,5,6,5);VS(-12,8,-2,8);VS(2,9,10,9);break;
 default:
  VB(-8,-9,16,2);VB(-2,-8,4,16);VB(-5,7,10,2);VS(-10,-4,-10,4);VS(10,-4,10,4);break;
 }
 #undef VX
 #undef VY
 #undef VS
 #undef VB
 #undef VO
}
/* The color plate is a different cut: halo, registration dots and a soft
 * echo of the icon. Even perfectly aligned work retains both inks. */
static void vm_colorcut(int a,int x,int y,int u,U c){
 disc(x+5*u,y-4*u,7*u,c);
 for(int i=0;i<3;i++)rect(x+(i*4-10)*u,y+10*u,2*u,u,c);
 vm_stamp(a,x+u,y-u,u,c);
}
/* A miniature street facade, not a new solid actor or a blocking crowd.
 * Audience silhouettes sit in the illuminated foyer behind the glass. */
static void vm_marquee(int id,VmStory bill,int diners){
 int x=sx(ordinaryDoorX[id]),y=sy(ordinaryDoorY[id]);if(x< -130||x>W+130||y<197||y>H-15)return;
 U c=diners?0xce9970:0xde7897;
 box(x-40,y-50,80,24,0x422c37,0xb77e6b);
 for(int i=0;i<(diners?4:6);i++){
  int u=x-28+i*(diners?17:11),v=y-36+(i%2)*2;
  disc(u,v-4,3,c);rect(u-3,v-1,7,7,diners?0x4e4548:0x292b3e);
 }
 if(diners)rect(x-36,y-30,72,2,0xcd976b);
 int w=textw(bill.title,0)+20;
 box(x-w/2,y-69,w,23,0x242333,c);center(x,y-68,bill.title,PAPER,0);
 for(int i=5;i<w-2;i+=9)dot(x-w/2+i,y-47,GOLD);
}
static int vd_b4(int ev,U keys,U tap){
 (void)keys;
 if(ev==VE_RESET){vm.blockBill=vm.booking=-1;vm.blockPhase=0;return 0;}
 if(ev==VE_START){vm.booking=-1;vm.blockPhase=0;vm_goal_room(VM_RECORDS);vd_status("Mariner Records: go to the listening bar and press E to book a night.");return 0;}
 if(ev==VE_AMBIENT&&vm_free()&&vm_station(VM_RECORDS)&&(tap&ENTER)){
  if(vd_launch(VA_B4))return vd_b4(VE_TICK,keys,tap);return 1;
 }
 if(ev==VE_DECOR&&vm_station(VM_RECORDS)){
  if(vm_free())vm_hint("E: Change the record     ENTER: Book a block event");
  else if(vdActive==VA_B4&&!vm.blockPhase)vm_hint("E / ENTER: Choose the block event");
 }
 if(ev==VE_CANCEL){vm.blockPhase=0;vm.booking=-1;return 0;}
 if(ev==VE_DECOR&&scene==1&&vm.blockBill>=0)vm_marquee(vmVenue[vm.blockBill],vmBill[vm.blockBill],vm.blockBill==1);
 if(ev!=VE_TICK)return 0;
 if(!vm.blockPhase&&vm_station(VM_RECORDS)&&(tap&(ENTER|USE))){
  vm.blockPhase=1;vd_choose("BOOK THE BLOCK","Velvet Static  /  Copper Playhouse","The Long Table  /  Mariner Dining Room");return 1;
 }
 if(vm.blockPhase==1&&vdChoice){
  vm.booking=vdChoice-1;vm.blockBill=vm.booking;vm.blockPhase=2;vm_goal_room(vmVenue[vm.booking]);
  vd_status(vm.booking?"Look for The Long Table outside Mariner Dining Room.":"See Velvet Static on Copper Playhouse's marquee.");
  tell("Your bill is up. Visit the venue to collect your booking fee.");return 1;
 }
 if(vm.blockPhase==2&&vm.booking>=0&&scene==1){
  int b=vmVenue[vm.booking];
  if(d2(p.x-ordinaryDoorX[b],p.y-ordinaryDoorY[b])<67*67){
   VmStory s=vmBill[vm.booking];vm_history(s);vm.blockPhase=3;vm_receipt(s,85,"Booked: ");return 0;
  }
 }
 return 0;
}
/* The two camera curbs deliberately have a long seating area, so an ordinary
 * parked car can't invalidate the only shot. BOTH scene paths loop until cut. */
static const int vmBayX[]={858,1860},vmBayY[]={1022,1022};
static int vm_subject_x(void){
 int v=vm.phaseT<240?vm.phaseT:480-vm.phaseT;
 return vmBayX[vm.event]+(v-120)*(vm.event?21:29)/120;
}
static int vm_subject_y(void){return vmBayY[vm.event]-41+(vm.event?((vm.phaseT/60)%2):0);}
static int vm_bay(const Car*c){return absf(c->x-vmBayX[vm.event])<=68&&absf(c->y-vmBayY[vm.event])<=19;}
static int vm_pan_error(void){return vm_subject_x()-(int)cars[vm.van].x-vm.pan;}
static const char*vm_frame_caption(void){
 const Car*c=cars+vm.van;
 if(!vm_bay(c))return "PARK IN THE STRIP";
 if(absf(c->s)>3)return "BRAKE TO STEADY";
 if(vm_pan_error()>12)return "PAN RIGHT  >";
 if(vm_pan_error()< -12)return "<  PAN LEFT";
 return vm.hold<120?"HOLD THE FRAME":"READY / ENTER: GO LIVE";
}
static int vm_stagevan(void){
 int i=nc-1;if(i<=0||i==p.car)return 0;
 Car v={.type=4,.owner=2,.cond=100};
 /* Hand off on a through road, not in the tight courtyard behind Low Tide.
  * Transactional fallback keeps a busy pickup curb from soft-locking a call. */
 static const int curb[][2]={{944,1262},{906,1298},{844,1261},{938,1291}};
 for(int k=0;k<LEN(curb);k++){
  v.x=curb[k][0];v.y=curb[k][1];v.a=0;
  if(!street_car_clear(i,0,&v))continue;
  vm.van=i;street_stage_car(i,v);return 1;
 }
 if(!street_find_car_space(i,&v,944,1280,25,23))return 0;
 vm.van=i;street_stage_car(i,v);return 1;
}
/* While the player is inside, release it as a free licensed ride rather
 * than deleting the body out from under them or teleporting them. It remains
 * the base game's reusable contract slot until the next dispatch. */
static void vm_releasevan(void){
 if(vm.van>0&&vm.van<nc&&p.car!=vm.van)
  street_stage_car(vm.van,(Car){.x=-100,.y=-100});
 else if(vm.van>0&&p.car==vm.van)vm.returnVan=vm.van;
 vm.van=-1;
}
static void vm_props(int e,int x,int y,int t,int humans){
 int facing=t<240?0:1;int stride=(int)(sn(t*.14f)*4);
 if(!e){
  if(humans&&!street_extra_down(N_ACT_OUT_MEDIA)&&!npc_replaces(N_ACT_OUT_MEDIA))
   person(x-10,y+1,0x925d9d,-stride,facing?3.14159f:0.f,0,1);
  if(humans&&!street_extra_down(N_ACT_OUT_MEDIA+1)&&!npc_replaces(N_ACT_OUT_MEDIA+1))
   person(x,y,0xd98569,stride,facing?3.14159f:0.f,0,4);
  box(x+5,y-8,10,7,0x46373d,0xc1a065);rect(x+7,y-10,4,1,PAPER);
 }else{
  if(humans&&!street_extra_down(N_ACT_OUT_MEDIA+2)&&!npc_replaces(N_ACT_OUT_MEDIA+2))
   person(x-5,y-3,0xe8b876,stride,facing?3.14159f:0.f,0,2);
  box(x+4,y-6,16,8,0x57696e,0xe1c295);disc(x+6,y+2,2,DARK);disc(x+17,y+2,2,DARK);
  line(x+8,y-6,x+12,y-16,GOLD);line(x+12,y-16,x+18,y-16,GOLD);line(x+18,y-16,x+18,y-10,PAPER);
 }
}
static void vm_world_news(void){
 if(vm.newsPhase<2||vm.van<0||vm.event<0)return;
 Car*c=cars+vm.van;int vx=sx(c->x),vy=sy(c->y);
 if(vx>=-24&&vx<W+24&&vy>=-25&&vy<H+45){
  box(vx-9,vy-9,18,8,0x1f3449,0xd6c08a);
  line(vx+1,vy-11,vx+5,vy-17,PAPER);disc(vx+5,vy-17,2,GOLD);
  if(p.car!=vm.van)center(vx,vy-41,"STUDIO 7",GOLD,0);
 }
 int x=sx(vmBayX[vm.event]),y=sy(vmBayY[vm.event]);
 if(x>=-100&&x<=W+100&&y>=-20&&y<=H+80){
  vmbracket(x-69,y-19,139,39,9,GOLD);line(x-56,y+19,x+56,y+19,0xb59e65);
  int ax=sx(vm_subject_x()),ay=sy(vm_subject_y());
  vm_props(vm.event,ax,ay,vm.phaseT,0);
  if(p.car==vm.van||d2(p.x-vmBayX[vm.event],p.y-vmBayY[vm.event])<210*210)
    center(x,y+20,"UNIT 7   /   CAMERA BAY",GOLD,0);
 }
}
static void vm_viewfinder(void){
 if(scene!=1||vm.newsPhase!=2||vm.event<0||vm.van<=0||p.car!=vm.van||
    d2(p.x-vmBayX[vm.event],p.y-vmBayY[vm.event])>128*128)return;
 /* Pixel geometry mirrors actual subject & swivel coordinates (not a
  * canned inset). The center box covers the same +/-12 as the shutter. */
 int x=18,y=153,w=259,h=188,mid=x+w/2;
 box(x,y,w,h,0x162a32,0x98c7bd);rect(x+2,y+2,w-4,22,0x25414a);
 textat(x+10,y+2,vm.event?"07 / YARD CAMERA":"07 / COPPER CAMERA",PAPER,0);
 rect(x+7,y+30,w-14,83,vm.event?0x4e5459:0x514652);
 for(int i=0;i<9;i++)rect(x+12+i*28,y+52-(i%4)*3,19,22+(i%4)*3,vm.event?0x555f61:0x65545b);
 rect(x+7,y+87,w-14,26,0x485159);
 int l=clipLeft,r=clipRight,t=clipTop,b=clipBottom;
 clipLeft=x+7;clipRight=x+w-7;clipTop=y+30;clipBottom=y+112;
 int py=y+89+cl((int)(vmBayY[vm.event]-cars[vm.van].y),-15,15);
 /* A live magnified cut from the actual actor pose. Draw to a small
  * keyed surface using the ordinary figure rig, then nearest-neighbour
  * sample at 2x; the scene position remains the real camera offset. */
 static U cell[52*40];U *save=surface;int sw=rw,sh=rh;
 z(cell,sizeof cell);surface=cell;rw=52;rh=40;
 clipLeft=clipTop=0;clipRight=52;clipBottom=40;
 vm_props(vm.event,22,27,vm.phaseT,1);
 surface=save;rw=sw;rh=sh;clipLeft=x+7;clipRight=x+w-7;clipTop=y+30;clipBottom=y+112;
 for(int j=0;j<40;j++)for(int i=0;i<52;i++)if(cell[j*52+i])
  rect(mid+vm_pan_error()*2+2*(i-22),py+2*(j-27),2,2,cell[j*52+i]);
 clipLeft=l;clipRight=r;clipTop=t;clipBottom=b;
 vmbracket(mid-24,y+51,49,48,7,vm.hold?LIME:PAPER);
 rect(mid-3,y+75,7,1,GOLD);rect(mid,y+72,1,7,GOLD);
 textat(x+10,y+115,vm_frame_caption(),vm.hold>=120?LIME:GOLD,0);
 box(x+10,y+140,w-20,9,0x253741,0x61787d);
 rect(x+12,y+142,vm.hold*(w-24)/120,5,TEAL);
 textat(x+10,y+158,"A/D: PAN   SHIFT: BRAKE",PAPER,0);
}
static int vd_b7(int ev,U keys,U tap){
 if(ev==VE_RESET){vm.newsPhase=0;vm.event=vm.newsPage=vm.van=vm.returnVan=-1;return 0;}
 if(ev==VE_AMBIENT&&vm.returnVan>0&&vm.returnVan<nc&&p.car!=vm.returnVan){
  /* An occupied van stays until the player actually steps out. Do not
   * reclaim the reserved slot if another dispatcher has already reused it. */
  if(vdActive<0&&job.kind<0&&cars[vm.returnVan].owner==2&&cars[vm.returnVan].type==4)
   street_stage_car(vm.returnVan,(Car){.x=-100,.y=-100});
  vm.returnVan=-1;
 }
 if(ev==VE_AMBIENT&&vm_free()&&vm_station(VM_STUDIO)&&(tap&ENTER)){
  if(vd_launch(VA_B7))return vd_b7(VE_TICK,keys,tap);return 1;
 }
 if(ev==VE_DECOR&&vm_station(VM_STUDIO)){
  if(vm_free())vm_hint("E: Switch the studio feed     ENTER: Take a live camera call");
  else if(vdActive==VA_B7&&!vm.newsPhase)vm_hint("E / ENTER: Choose tonight's live story");
 }
 if(ev==VE_CANCEL){vm_releasevan();vm.newsPhase=0;return 0;}
 if(ev==VE_START){vm.newsPhase=0;vm.event=-1;vm_goal_room(VM_STUDIO);vd_status("Low Tide Radio: visit the studio console and press E for the two live calls.");return 0;}
 if(ev==VE_WORLD){vm_world_news();return 0;}
 if(ev==VE_DRAW){vm_viewfinder();return 0;}
 if(ev==VE_DECOR&&vm.newsPage>=0&&scene==1){
  int x=sx(ordinaryDoorX[VM_STUDIO]),y=sy(ordinaryDoorY[VM_STUDIO])-57;
  if(x>=-100&&x<=W+100&&y>=112&&y<=H-82){
   VmStory story=vmLive[vm.newsPage];int w=textw(story.title,0)+22;
   box(x-w/2,y-21,w,42,0x192f36,TEAL);center(x,y-21,"LOW TIDE  /  LIVE",TEAL,0);center(x,y-2,story.title,PAPER,0);
  }
 }
 if(ev!=VE_TICK)return 0;
 if(!vm.newsPhase&&vm_station(VM_STUDIO)&&(tap&(USE|ENTER))){
  vm.newsPhase=1;vd_choose("LOW TIDE  /  Pick tonight's live call","Copper Playhouse: a local duo unloads for the encore.","Wrecker Yard: Dax's crew brings in the last hook.");return 1;
 }
 if(vm.newsPhase==1&&vdChoice){
  if(!vm_stagevan()){vm.newsPhase=0;tell("Studio curb blocked. Press E at the console to dispatch again.");return 1;}
  vm.event=vdChoice-1;vm.newsPhase=2;vm.wasInside=0;vm.phaseT=vm.hold=vm.pan=0;
  vd_goal((int)cars[vm.van].x,(int)cars[vm.van].y,"STUDIO VAN - LOW TIDE SOUTH CURB");
  vd_status("Board the licensed Unit 7 van on the road south of Low Tide Radio.");return 1;
 }
 if(vm.newsPhase!=2||vm.van<=0)return 0;
 if(!cars[vm.van].cond){vm_releasevan();vm.newsPhase=0;vm_goal_room(VM_STUDIO);
  vd_status("Van out of service. Return to the Low Tide console to dispatch a replacement.");return 0;}
 if(scene!=1){vm.hold=0;return 0;}
 int first=vm.event?N_ACT_OUT_MEDIA+2:N_ACT_OUT_MEDIA,n=vm.event?1:2;
 for(int i=first;i<first+n;i++)
  if(npc_holds(i)||npc_replaces(i)||street_extra_down(i)){
   vm.hold=0;
   if(p.car==vm.van&&p.noticeT<=0.f)tell("Live subjects are unavailable. Hold for the original feed to resume.");
   return 0;
  }
 vm.phaseT=(vm.phaseT+1)%480;
 if(p.car!=vm.van){
  if(vm.wasInside){vm.wasInside=0;vd_goal((int)cars[vm.van].x,(int)cars[vm.van].y,"UNIT 7 STUDIO VAN");
   vd_status("Return to the studio van. Only its roof camera can go live.");}
  vm.hold=0;return 0;
 }
 if(!vm.wasInside){
  vm.wasInside=1;vd_goal(vmBayX[vm.event],vmBayY[vm.event],vm.event?"WRECKER YARD CAMERA BAY":"COPPER CAMERA BAY");
  vd_status("Park in the striped camera bay. A/D pan the roof camera; hold the subject 2s, then Enter.");
 }
 Car*c=cars+vm.van;
 if(vm_bay(c)&&absf(c->s)<=3){
  vm.pan=cl(vm.pan+!!(keys&RIGHT)-!!(keys&LEFT),-110,110);
  vm.hold=absf((float)vm_pan_error())<=12?min(120,vm.hold+1):0;
 }else vm.hold=0;
 if((tap&ENTER)&&vm.hold>=120){
  vm.newsPage=vm.event;VmStory s=vmLive[vm.event];vm_history(s);vm_releasevan();vm.newsPhase=0;
  vm_receipt(s,135,"LIVE: ");return 1;
 }
 return 0;
}
static int vm_platepos(int t,int second){
 /* Triangle carriers, not a random result. The pass starts off-register;
  * no immediate follow-up Enter can stamp two centered plates. */
 int period=second?234:192,half=period/2,extent=second?40:60;
 int i=(t+(second?half:0))%period;
 return (i<=half?i:period-i)*2*extent/half-extent;
}
static const char*vm_grade(int grade){return grade==2?"IN REGISTER":grade==1?"SOFT EDGE":"SPLIT INK";}
static void vm_proofcard(int x,int y,int w,VmStory art,int dx,int dy,int grade,int num){
 int h=99;U paper=0xe8d9b6,ink=0x2e3843,spot=vmInk[art.art];
 box(x+3,y+3,w,h,0x1d262c,0x1d262c);box(x,y,w,h,paper,0x8d8574);
 rect(x+8,y+5,w-16,2,spot);
 vm_colorcut(art.art,x+w/2+dx/6,y+38+dy/6,1,spot);vm_stamp(art.art,x+w/2,y+38,1,ink);
 char label[30];char*q=number(append(label,"NO. "),num);q=append(q,"   ");append(q,vm_grade(grade));
 center(x+w/2,y+7,label,ink,0);
 center(x+w/2,y+52,art.title,ink,0);
 char sign[38];q=append(sign,"by ");append(q,sv.name[0]?sv.name:"Vanta Local");center(x+w/2,y+73,sign,0x523e40,0);
 for(int i=0;i<2;i++){int u=x+7+i*(w-14);line(u-2,y+44,u+2,y+44,spot);line(u,y+42,u,y+46,spot);}
}
static void vm_pressdraw(void){
 int second=vm.printPhase==3,phase=vm_platepos(vm.plateTick,second),x=122,y=126;
 U paper=0xe6d9b9,ink=0x273a43,spot=vmInk[vm.working.art];
 box(x,y,716,306,0x1b2e37,0xd8b288);rect(x+1,y+1,714,28,0x403744);
 textat(x+15,y+4,"BLUEBIRD   /   HARBOR PROOF",GOLD,0);
 textat(x+17,y+45,second?"02  /  COLOR CARRIAGE":"01  /  KEY CARRIAGE",PAPER,0);
 textat(x+17,y+74,second?"Set the height of the spot ink.":"Set the horizontal keyline.",TEAL,0);
 textat(x+17,y+102,"Press ENTER as the moving",PAPER,0);textat(x+17,y+123,"triangle crosses the notch.",PAPER,0);
 textat(x+17,y+162,"Even a split print goes up.",MUTED,0);
 textat(x+17,y+200,"COMMISSION  /  $45 CLEAN",GOLD,0);
 const int px=x+509,py=y+142;
 box(px-131,py-92,262,176,paper,0x969080);
 for(int j=0;j<2;j++)for(int i=0;i<2;i++){
  int u=px-114+228*i,v=py-77+151*j;
  line(u-4,v,u+4,v,ink);line(u,v-4,u,v+4,ink);
 }
 vm_stamp(vm.working.art,px+1,py+1,3,0xbba78d); /* static registration ghost */
 if(!second)vm_stamp(vm.working.art,px+phase,py,3,ink);
 else{
  vm_colorcut(vm.working.art,px,py+phase,3,spot);
  vm_stamp(vm.working.art,px+vm.printOffset1,py,3,ink);
 }
 const int rail=y+247,m=x+475;
 if(!second){
  rect(m-120,rail,240,3,MUTED);rect(m-6,rail-3,13,9,0x5f7974);rect(m,rail-8,2,19,PAPER);
  int tip=m+2*phase;tri(tip,rail-1,tip-5,rail-11,tip+5,rail-11,GOLD);
 }else{
  int col=x+665,tip=py+phase*2;
  rect(col,py-80,3,161,MUTED);rect(col-3,py-5,9,11,0x5f7974);rect(col-8,py,19,2,PAPER);
  tri(col-1,tip,col-11,tip-5,col-11,tip+5,GOLD);
 }
 center(m,rail+12,second?"HEIGHT REGISTER  /  Y":"SIDE REGISTER  /  X",TEAL,0);
}
static void vm_editions(void){
 /* An old Rev10 career has aggregate fare and first-paycheck facts but no
  * last-job ID. Name a fallback as an archive print, not a fictitious run. */
 int n=0;for(int i=0;i<vm.historyN;i++)vm.edition[n++]=vm.history[i];
 if(n<2&&sv.taxiFares&&(!n||vm.edition[0].title!=vmJobs[JOB_TAXI].title))vm.edition[n++]=vmJobs[JOB_TAXI];
 if(n<2&&sv.legal>0&&sv.intro>=3)vm.edition[n++]=(VmStory){"FIRST PAYCHECK",VM_ENVELOPE};
 if(n<2)vm.edition[n++]=vmArchive[0];if(n<2)vm.edition[n++]=vmArchive[1];
 char a[100],b[100];for(int i=0;i<2;i++){VmStory s=vm.edition[i];char *q=i?b:a;
  q=append(q,s.title);append(q,s.title==vmArchive[0].title||s.title==vmArchive[1].title?"  /  city archive":"  /  career story");}
 vd_choose("HARBOR PROOF  /  Which story belongs in the window?",a,b);
}
static int vd_b8(int ev,U keys,U tap){
 (void)keys;
 if(ev==VE_RESET){
  vm.historyN=vm.impression=vm.folio=vm.printPhase=0;vm.prevKind=-1;vm.lastAirTitle=vm.lastAirTicks=0;vm.prevLegal=sv.legal;vm.prevCrime=sv.crime;return 0;
 }
 if(ev==VE_AMBIENT){
#ifdef VD_HAS_A1
  if(a1Headline>0&&(a1Headline!=vm.lastAirTitle||a1BroadcastFrames>vm.lastAirTicks)){
   vm_history((VmStory){a1Headline==1?"SEAWALL NOCTURNE":"LAST TRAM HOME",a1Headline==1?VM_SKY:VM_AMPLIFIER});vm.lastAirTitle=a1Headline;
  }
  vm.lastAirTicks=a1BroadcastFrames;
#endif
  /* The base job increments one of these counters on a successful finish,
   * including unpaid service. Mere cancellation does not write a story. */
  if(vm.prevKind>=0&&vm.prevKind<LEN(vmJobs)&&
     ((vm.prevKind<JOB_VAULT&&sv.legal>vm.prevLegal)||(vm.prevKind>=JOB_VAULT&&sv.crime>vm.prevCrime)))vm_history(vmJobs[vm.prevKind]);
  vm.prevKind=job.kind;vm.prevLegal=sv.legal;vm.prevCrime=sv.crime;
  if(vm_free()&&vm_station(VM_PRESS)&&(tap&ENTER)){
   if(vd_launch(VA_B8))return vd_b8(VE_TICK,keys,tap);return 1;
  }
  return 0;
 }
 if(ev==VE_START){vm.printPhase=0;vm_goal_room(VM_PRESS);vd_status("Bluebird Print Shop: press E at the letterpress to choose a story.");return 0;}
 if(ev==VE_CANCEL){vm.printPhase=0;return 0;}
 if(ev==VE_DECOR&&vm_station(VM_PRESS)){
  if(vm_free())vm_hint("E: Pull a shop proof     ENTER: Print your career poster");
  else if(vdActive==VA_B8)vm_hint(vm.printPhase>=2?"ENTER: Clamp the current plate on its notch":"E / ENTER: Choose a story for the press");
 }
 if(ev==VE_DECOR&&vm.impression){
  int x=-1000,y=-1000;
  if(scene==1){int dx=ordinaryDoorX[VM_PRESS]-(int)p.x,dy=ordinaryDoorY[VM_PRESS]-(int)p.y;
   int win=sx(ordinaryDoorX[VM_PRESS]),top=sy(ordinaryDoorY[VM_PRESS])-47;
   if(win> -30&&win<W+30&&top>150&&top<H-48){
    box(win-15,top,30,24,0xe2d0ac,0x59505c);
    vm_colorcut(vm.proof.art,win+vm.dx/15,top+13+vm.dy/15,1,vmInk[vm.proof.art]);vm_stamp(vm.proof.art,win,top+13,1,0x354755);
   }
   if(dx*dx+dy*dy<99*99){x=cl(win-101,6,W-207);y=cl(top-100,120,H-149);}
  }else if(scene==4&&walkin_active==VM_PRESS&&vm.printPhase<2){x=695;y=127;}
  if(x>=0){int w=max(238,max(textw(vm.proof.title,0)+21,textw(sv.name,0)+44));vm_proofcard(cl(x,5,W-5-w),y,w,vm.proof,vm.dx,vm.dy,vm.quality,vm.folio);}
 }
 if(ev==VE_DRAW&&vm.printPhase>=2&&vm_station(VM_PRESS))vm_pressdraw();
 if(ev!=VE_TICK)return 0;
 if(!vm.printPhase&&vm_station(VM_PRESS)&&(tap&(ENTER|USE))){vm.printPhase=1;vm_editions();return 1;}
 if(vm.printPhase==1&&vdChoice){
  vm.working=vm.edition[vdChoice-1];vm.printPhase=2;vm.printOffset1=vm.printOffset2=0;vm.plateTick=0;
  vd_status("Enter on the register notch for each moving plate. Both good and rough prints go in the window.");return 1;
 }
 if(vm.printPhase>=2&&vm_station(VM_PRESS)){
  int second=vm.printPhase==3,offset=vm_platepos(vm.plateTick,second);
  if(tap&ENTER){
   if(!second){vm.printOffset1=offset;vm.printPhase=3;vm.plateTick=0;return 1;}
   vm.printOffset2=offset;vm.dx=vm.printOffset1;vm.dy=vm.printOffset2;
   vm.quality=absf((float)vm.dx)<=6&&absf((float)vm.dy)<=5?2:absf((float)vm.dx)<=19&&absf((float)vm.dy)<=14?1:0;
   vm.proof=vm.working;vm.impression=1;vm.printPhase=0;if(vm.folio<9999)vm.folio++;vm_receipt(vm.proof,45,"Window proof: ");return 1;
  }
  vm.plateTick=(vm.plateTick+1)%(second?234:192);
  if(tap&USE)return 1;
 }
 return 0;
}
