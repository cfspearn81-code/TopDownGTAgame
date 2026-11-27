/* Lantern Quay in the street renderer. Each object consumes the existing helm,
 * traffic, wake ring or Harbor Office objective. No camera owns game state.
 * The hull rocks about its waterline; its pilot, lamps, deck and superstructure
 * travel together. Their visual movement never nudges a mission or collision. */
typedef struct {float x,y,co,si,lift,pitch,roll;} ChShip;
static int ch_sea_visible(float x,float y,float r,float dist){
 float dx=x-ch.x,dy=y-ch.y;
 if(d2(dx,dy)>(dist+r)*(dist+r))return 0;
 ChVec v=ch_local((ChVec){x,y,r*.25f});return v.z>9.f-r&&absf(v.x)<v.z*1.35f+r;
}
static ChVec ch_ship_pt(const ChShip*b,float u,float v,float zc){
 return (ChVec){b->x+b->co*u-b->si*v,b->y+b->si*u+b->co*v,b->lift+zc+b->pitch*u+b->roll*v};
}
/* Tapered, continuous four-sided volume with an optional raked wheelhouse. */
static void ch_ship_box(const ChShip*b,float aft,float fore,float mid,float half,float lo,float hi,float inset,float rake,U side,U roof){
 float a=aft+inset+rake,f=fore-inset+rake,w=half-inset*.45f;
 ChVec p[]={ch_ship_pt(b,aft,mid-half,lo),ch_ship_pt(b,fore,mid-half,lo),ch_ship_pt(b,fore,mid+half,lo),ch_ship_pt(b,aft,mid+half,lo)};
 ChVec t[]={ch_ship_pt(b,a,mid-w,hi),ch_ship_pt(b,f,mid-w,hi),ch_ship_pt(b,f,mid+w,hi),ch_ship_pt(b,a,mid+w,hi)};
 for(int k=0;k<4;k++)ch_quad(p[k],p[(k+1)&3],t[(k+1)&3],t[k],k==2?mix(side,0x122938,2):k==1?mix(side,PAPER,1):side,1);
 ch_face(t,4,roof,1);
}
static void ch_ship_segment(const ChShip*b,float a,float f,float w,float w2,float z0,float z1,U c){
 ch_quad(ch_ship_pt(b,a,-w,z0),ch_ship_pt(b,f,-w2,z1),ch_ship_pt(b,f,w2,z1),ch_ship_pt(b,a,w,z0),c,1);
}
static void ch_ship_pilot(const ChShip*b,int hero){
 U skin=hero?charSkin[sv.look.part[SKIN]]:0xb88162,coat=hero?charTop[sv.look.part[TOP_TONE]]:0xc27f52;
 ch_ship_box(b,-4.5f,-1.1f,0,1.85f,3.6f,8.5f,.26f,.15f,coat,mix(coat,PAPER,1));
 ch_ship_box(b,-3.8f,-1.4f,0,1.28f,8.2f,10.45f,.15f,.2f,skin,skin);
 if(!hero||sv.look.part[HAIR]){
  U cap=hero?charHair[sv.look.part[HAIR_TONE]]:0x334858;
  ch_ship_box(b,-3.8f,-1.2f,0,1.32f,9.9f,10.87f,.16f,0,cap,cap);
 }
 for(int side=-1;side<=1;side+=2)ch_ship_box(b,-2.3f,.75f,side*1.6f,.49f,6.7f,7.65f,.10f,0,coat,skin);
}
/* Public skiff, amber ferry, freight lighter, patrol, blue rendezvous cabin
 * and orange uncrewed trainer. Common five-station chined bow, different rigs. */
enum {CH_BLUE_CABIN=4,CH_ORANGE_TRAINER=5};
static void ch_sea_hull(const HarborHull*h,int skin,int hero){
 if(!ch_sea_visible(h->x,h->y,24,690))return;
 float swell=anim*1.8f+(h->x+h->y)*.023f;
 ChShip b={h->x,h->y,cs(h->a),sn(h->a),.95f+sn(swell)*.35f,sn(swell+.8f)*.021f,cs(swell*.84f)*.047f};
 static const U colors[]={0x54ae9f,0xca8e53,0x9b5a52,0x527996,0x437fa6,0xdc874b};
 U sides=colors[skin],rail=mix(sides,0xf1e8ca,5),deck=skin==HB_CARGO?0xab916d:0xb7ad91;
 float len=skin==HB_CARGO?18.f:skin==CH_ORANGE_TRAINER?12.4f:15.8f;
 float u[]={-len,-len*.60f,len*.43f,len*.81f,len},w[]={5.1f,6.8f,6.6f,3.5f,.18f},lip[]={3.75f,3.5f,3.8f,4.4f,5.05f};
 for(int k=0;k<4;k++){
  if(skin==HB_CARGO){w[k]*=1.22f;lip[k]+=.48f;}
 }
 for(int k=0;k<4;k++)for(int s=-1;s<=1;s+=2){
  float v=w[k]*s,next=w[k+1]*s;
  ChVec a=ch_ship_pt(&b,u[k],v*.69f,.02f),f=ch_ship_pt(&b,u[k+1],next*.69f,.02f);
  ChVec e=ch_ship_pt(&b,u[k+1],next,lip[k+1]),d=ch_ship_pt(&b,u[k],v,lip[k]);
  ChVec m=ch_ship_pt(&b,u[k],v*.90f,lip[k]*.47f),n=ch_ship_pt(&b,u[k+1],next*.90f,lip[k+1]*.47f);
  ch_quad(a,f,n,m,mix(sides,0x163844,s<0?4:6),1);
  ch_quad(m,n,e,d,s<0?sides:mix(sides,0x152c3c,2),1);
  ch_quad(ch_ship_pt(&b,u[k],v*.977f,lip[k]-.49f),ch_ship_pt(&b,u[k+1],next*.977f,lip[k+1]-.49f),e,d,rail,1);
 }
 for(int k=0;k<4;k++)ch_ship_segment(&b,u[k],u[k+1],w[k],w[k+1],lip[k],lip[k+1],deck);
 ch_quad(ch_ship_pt(&b,u[0],-w[0]*.69f,.02f),ch_ship_pt(&b,u[0],w[0]*.69f,.02f),ch_ship_pt(&b,u[0],w[0],lip[0]),ch_ship_pt(&b,u[0],-w[0],lip[0]),sides,1);
 if(skin==HB_SKIFF){
  ch_ship_box(&b,-len-.8f,-len+1.f,0,1.68f,1,5.3f,.22f,0,0x253741,0x567079);
  ch_ship_box(&b,-8,-4,0,4.85f,3.6f,4.75f,.28f,0,0x364a52,0x9b7861);
  ch_ship_box(&b,.5f,4.3f,0,4.5f,3.6f,6.5f,.22f,-.5f,0x375b60,0x536d70);
  ch_ship_box(&b,2.8f,3.95f,0,4.25f,6.3f,9.f,.15f,-1.2f,0x82afb5,0xc2d5ca);
  ch_ship_box(&b,7.4f,11.f,0,1.8f,4.3f,4.9f,.2f,0,0xb79d75,0xe2cfab);
  if(hero||h->s!=0)ch_ship_pilot(&b,hero);
 }else if(skin==HB_CARGO){
  for(int k=0;k<2;k++){
   float u0=-15+k*6.75f;
   ch_ship_box(&b,u0,u0+6,0,5,4.2f,8.7f,.17f,0,0x906b4f,0xb6966c);
   ch_ship_segment(&b,u0+2,u0+2.6f,5.03f,5.03f,8.77f,8.77f,0x644f43);
  }
  ch_ship_box(&b,1,8,0,4.8f,4.f,11.f,.48f,-.2f,0x527473,0xe1d1b3);
  ch_ship_box(&b,1.3f,7.8f,0,4.82f,7.5f,10.2f,.4f,-.2f,0x324e62,0x558693);
 }else if(skin==CH_ORANGE_TRAINER){
  ch_ship_box(&b,-6,4,0,3.6f,3.8f,5.3f,.37f,0,0x3a464e,0xfdac55);
  ch_ship_box(&b,-1.7f,-.3f,0,.45f,5.2f,12.8f,.09f,0,0x685548,0xffda85);
  ch_quad(ch_ship_pt(&b,-1.2f,0,12.5f),ch_ship_pt(&b,-5.8f,0,11.5f),ch_ship_pt(&b,-4.7f,0,9.45f),ch_ship_pt(&b,-1.2f,0,9.85f),0xff8a48,1);
 }else{
  ch_ship_box(&b,-7.1f,6.1f,0,4.8f,3.7f,7.43f,.49f,-.25f,sides,0x91aaa1);
  ch_ship_box(&b,-7,6.2f,0,4.68f,7.25f,11.7f,.98f,-.7f,0x496f83,0x7eacb6);
  for(int s=-1;s<=1;s+=2)for(int k=0;k<2;k++){
   float a=-5.6f+k*6.3f;
   ch_quad(ch_ship_pt(&b,a,s*4.62f,7.8f),ch_ship_pt(&b,a+4.6f,s*4.62f,7.8f),ch_ship_pt(&b,a+3.95f,s*4.35f,10.8f),ch_ship_pt(&b,a+.33f,s*4.35f,10.8f),k?0x8bb6bb:0x567e92,1);
  }
  ch_ship_box(&b,-7.9f,5.7f,0,5.06f,11.55f,12.07f,.15f,-.1f,rail,0xe4dfc4);
  if(skin==HB_PATROL){
   for(int s=-1;s<=1;s+=2){
    U c=((int)(hbClock*4)&1)==(s>0)?0x9ddff2:0x47759b;
    ch_ship_box(&b,-.9f,.9f,s*2.3f,1.23f,12.1f,12.85f,.15f,0,c,c);
   }
  }else if(skin==CH_BLUE_CABIN){
   ch_ship_box(&b,-1.1f,.8f,0,2.2f,12.1f,12.7f,.13f,0,0x579cbf,0xb4daeb);
   ch_ship_box(&b,-5.2f,-4.5f,0,.35f,12,16,.05f,0,0x8ea6af,0xf2e8bc);
  }
 }
 for(int s=-1;s<=1;s+=2){
  U lens=s<0?0xd77e72:0x80cb9e;float v=s*w[2]*.85f;
  ch_ship_box(&b,u[2]-.7f,u[2]+.55f,v,.39f,lip[2],lip[2]+.55f,.10f,0,lens,lens);
 }
}
static void ch_sea_wakes(void){
 if(fastGfx)return;
 for(int i=0;i<HB_WAKE;i++){
  const HarborWake*w=hbWakes+i;
  if(w->time<.035f||!ch_sea_visible(w->x,w->y,28,330))continue;
  float k=1.f-w->time,sp=w->span+k*10.f;
  ChShip b={w->x,w->y,cs(w->a),sn(w->a),.19f,0,0};
  U sea=chDark>.5f?0x2b4251:0x456675,light=chDark>.5f?0x80999a:0xc1d9cb;
  U foam=mix(sea,light,cl((int)(w->time*6),1,6));
  for(int s=-1;s<=1;s+=2){
   ChVec a=ch_ship_pt(&b,-k*6-2,s*sp*.22f,0),b0=ch_ship_pt(&b,-k*6-6,s*sp*.70f,0);
   ChVec c=ch_ship_pt(&b,-k*6-7.2f,s*sp,0),d=ch_ship_pt(&b,-k*6-4.65f,s*sp*.60f,0);
   if(harbor_ocean_cell(c.x,c.y)&&harbor_ocean_cell(a.x,a.y))ch_quad(a,b0,c,d,foam,1);
  }
 }
}
/* Small clusters of long, broken crests, attached to sea cells rather than
 * screen pixels. Patches slide a few units within their cell, not with the
 * camera. The old chart supplies the large-scale shallows/wharf contours. */
static void ch_sea_glints(void){
 if(fastGfx)return;
 int bx=(int)ch.x/45,by=(int)ch.y/38;
 float phase=anim*.9f;U gl=chDark>.5f?0x4b6a72:0x659093;
 for(int y=by-8;y<=by+8;y++)for(int x=bx-8;x<=bx+8;x++){
  if((x^y)&1)continue;
  U seed=outer_noise(x,y);
  float sx=x*45+7+(seed&15),sy=y*38+5+((seed>>8)&15);
  if(!ch_sea_visible(sx,sy,9,280))continue;
  float wave=sn(phase+(int)((seed>>16)&15)*.8f);
  if(wave<-.6f)continue;
  sx+=wave*1.8f;sy+=wave*2.4f;
  float w=2.6f+(seed%4)+wave*1.5f;
  if(!harbor_ocean_cell(sx-w,sy)||!harbor_ocean_cell(sx+w+2,sy+1.2f))continue;
  ch_quad((ChVec){sx-w,sy,.13f},(ChVec){sx+w*.45f,sy+.54f,.13f},(ChVec){sx+w,sy+.22f,.13f},(ChVec){sx+w*.10f,sy+1.1f,.13f},gl,1);
 }
}
static void ch_sea_assignment(void){
 int phase=hj_phase();NavPoint q=hj_office;
 if(ch_sea_visible(q.x,q.y,28,420)){
  ChShip b={q.x,q.y,1,0,0,0,0};
  ch_ship_box(&b,-10,9,0,3.25f,0,6,.22f,0,0x705747,0xbb9e70);
  ch_ship_box(&b,-5,0,0,1.45f,6,7,.2f,0,0x3b535f,0x9eb2ac);
  ch_character(&(StreetPerson){q.x+11,q.y-4,1.5f,0,MODEL_DISPATCH,2,0x718da0});
  for(int s=-1;s<=1;s+=2)ch_upright(q.x+s*12,q.y-10,.72f,0,16,0x53616a);
  ch_quad((ChVec){q.x-13,q.y-10,11},(ChVec){q.x+13,q.y-10,11},(ChVec){q.x+13,q.y-10,17},(ChVec){q.x-13,q.y-10,17},0x467185,1);
 }
 if(phase==HJ_RELAY_TAG||phase==HJ_RELAY_LIFT){
  q=phase==HJ_RELAY_TAG?hj_automaton():hj_alley;
  if(ch_sea_visible(q.x,q.y,16,360)){
   ChShip b={q.x,q.y,1,0,0,0,0};
   if(phase==HJ_RELAY_TAG){
    for(int s=-1;s<=1;s+=2)ch_ship_box(&b,-1.7f,1.4f,s*2.3f,1.05f,0,5,.24f,0,0x576d75,0x728a87);
    ch_ship_box(&b,-2.5f,2.5f,0,4.f,4.4f,12,.65f,0,0x455e6c,0x9aa6a0);
    ch_ship_box(&b,-1.7f,1.6f,0,2.8f,12,15.7f,.21f,0,0x3c5265,0x889da0);
    ch_ship_box(&b,1.f,1.85f,0,2.13f,13.3f,14.2f,.15f,0,hj_gold,hj_gold);
   }else ch_ship_box(&b,-6,6,0,3.8f,0,3.8f,.42f,0,0x455d69,0x7c9599);
  }
 }else if(phase==HJ_BLUE_CLUE||phase==HJ_RELAY_QUAY){
  q=phase==HJ_BLUE_CLUE?hj_cue:hj_quay;
  if(ch_sea_visible(q.x,q.y,21,400)){
   ChShip b={q.x,q.y,1,0,0,0,0};
   ch_ship_box(&b,-1.1f,1.1f,0,.78f,0,11,.1f,0,0x697c7b,0x8fa8a1);
   ch_ship_box(&b,-5.5f,5.5f,0,.93f,9,15,.15f,0,phase==HJ_BLUE_CLUE?0x5989a9:0xbe945c,0xc2d6c9);
   ch_ship_box(&b,-1.8f,1.8f,1,.18f,11.5f,12.7f,0,0,0xbcd9d9,0xbcd9d9);
  }
 }else if(phase==HJ_RELAY_TRUNK&&hj_own(0)&&p.car<0&&!hb.active){
  q=hj_rear();if(ch_sea_visible(q.x,q.y,3,380))ch_upright(q.x,q.y,1.1f,4.f,9.f,hj_gold);
 }
 if(phase==HJ_BLUE_FIND){
  q=hj_water(HJ_BLUE);
  if(hj_near(q,265))ch_sea_hull(&(HarborHull){.x=q.x,.y=q.y,.a=-.87f,.kind=HB_CABIN},CH_BLUE_CABIN,0);
 }else if(phase==HJ_BUMP_TAG){
  q=hj_trainer();ch_sea_hull(&(HarborHull){.x=q.x,.y=q.y,.a=-anim*.36f,.kind=HB_SKIFF},CH_ORANGE_TRAINER,0);
 }else if(phase==HJ_RELAY_BUOY||phase==HJ_BUMP_LOOT){
  q=phase==HJ_RELAY_BUOY?hj_water(HJ_RELAY):hj_drift();
  if(ch_sea_visible(q.x,q.y,14,540)){
   ChShip b={q.x,q.y,.8f,.6f,1.f+sn(hbClock*2)*.4f,0,0};
   if(phase==HJ_RELAY_BUOY){
    ch_ship_box(&b,-4,4,0,4.f,0,2.6f,.8f,0,0xd89555,hj_gold);
    ch_ship_box(&b,-1.6f,1.6f,0,1.6f,2.5f,7.2f,.48f,0,0x486574,0xb68f60);
    ch_ship_box(&b,-1.2f,1.2f,0,1.2f,7.2f,9.2f,.16f,0,hj_gold,0xf5e6bb);
   }else ch_ship_box(&b,-2.7f,2.7f,0,2.55f,.05f,1.3f,.45f,0,0x63888a,hj_gold);
  }
 }
}
static void ch_sea_boats(void){
 if(!hb.active&&(ch.x<1770.f||ch.y<1540.f))return;
 ch_sea_glints();ch_sea_wakes();
 if(hb.active)ch_sea_hull(&hb.hull,HB_SKIFF,1);
 if(ch.x<1770.f||ch.y<1540.f)return; /* cull distant harbor actors, never the helm */
 ch_sea_assignment();
 for(int i=0;i<HB_AMBIENT;i++)if(hbTraffic[i].live)
  ch_sea_hull(&hbTraffic[i].hull,hbTraffic[i].hull.kind,0);
 /* Boardable launch at each genuine berth. Never duplicate the player's
  * launch at its home berth while underway. */
 for(int i=0;i<HB_DOCKS;i++){
  if(hb.active&&i==hb.dock)continue;
  int x,y,wx,wy;if(!hb_dock(i,&x,&y,&wx,&wy)||!ch_sea_visible(wx,wy,20,245))continue;
  if(hb.active&&d2(wx-hb.hull.x,wy-hb.hull.y)<32*32)continue;
  if(harbor_boat_clear(wx,wy,1.5707963f))ch_sea_hull(&(HarborHull){.x=wx,.y=wy,.a=1.5707963f,.kind=HB_SKIFF},HB_SKIFF,0);
 }
}
