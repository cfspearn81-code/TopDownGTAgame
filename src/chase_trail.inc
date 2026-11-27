/* In-world field objects use the same states, offsets and interaction points
 * as the field journal. No replay / wildlife AI updates happen in paint. */
static void wl_specimen(float x,float y,int t,float size,float base){
 WlRing f=wl_ring(7+t*13);
 wl_box(x-.5f,y-.5f,x+.5f,y+.5f,base,base+size*.75f,0xc3b8a2,0xdbcdae);
 wl_crown(&f,x,y,base+size*.65f,size*(t==3?.35f:.56f),1.f,size*(t==2?.62f:.33f),t!=2,wf_tint[t]);
}
static void wl_cache(int i,float x,float y){
 int claimed=wf_has(wf_chest_bits[i]);
 U trim=claimed?0x70958b:wf_chest_ready(i)?0xe9b369:0x94a69e;
 wl_box(x-9,y-5,x+9,y+5,0,6.4f,0x635241,claimed?0x343836:0x98774f);
 if(claimed)wl_box(x-9,y+4.5f,x+9,y+5.7f,5.8f,13,0x6f644e,trim);
 for(int n=-1;n<=1;n+=2)
  wl_box(x+n*5.6f-.58f,y-5.2f,x+n*5.6f+.58f,y+5.2f,0,6.55f,trim,trim);
 wl_box(x-1.25f,y-5.3f,x+1.25f,y-5.13f,3.45f,5.7f,trim,trim);
}
/* Surface stroke in a noticeboard's X/Z plane, still occluded by actors. */
static void wl_sheet_ink(float x,float y,float z,float xx,float zz,float width,U col){
 float dx=xx-x,dz=zz-z,len=sq(dx*dx+dz*dz);
 if(len<.1f)return;
 float u=dz*width/len,v=-dx*width/len;
 ch_quad((ChVec){x-u,y,z-v},(ChVec){x+u,y,z+v},(ChVec){xx+u,y,zz+v},(ChVec){xx-u,y,zz-v},col,1);
}
static void wl_ranger(void){
 const OuterPoint q=woodline_sites[WOOD_RANGER];float x=q.x,y=q.y;
 if(wl_visible(x,y,13,45,420)<-9900)return;
 /* Hardwood map case and twin angled roof faces match the timber cabin. */
 for(int side=-1;side<=1;side+=2)wl_box(x+side*15-1.3f,y-7.6f,x+side*15+1.3f,y-5,0,22,0x745f48,0x9c8762);
 wl_box(x-20,y-7.9f,x+20,y-5,7,20.4f,0x3c564b,0x938966);
 /* Survey map and posted notices read above the till from either approach. */
 wl_box(x-18,y-8.14f,x+2,y-4.76f,9.0f,18.3f,0xaabb96,0xcfbd95);
 for(int i=0;i<2;i++)wl_box(x+5+i*7,y-8.1f,x+10+i*7,y-4.8f,10+i,17.8f-i,0xcfbe94,0xb7a781);
 for(int s=0;s<2;s++){
  float v=s?y-8.23f:y-4.67f;
  wl_sheet_ink(x-16,v,10.2f,x-12,14.3f,.42f,0x7d9685);
  wl_sheet_ink(x-12,v,14.3f,x-6,14.8f,.42f,0x7d9685);
  wl_sheet_ink(x-6,v,14.8f,x-1,17.5f,.42f,0x7d9685);
  wl_sheet_ink(x-15,v,16.4f,x-11,13.5f,.23f,0x967852);
  wl_sheet_ink(x-11,v,13.5f,x-9,10.8f,.23f,0x967852);
  for(int n=0;n<2;n++)for(int row=0;row<3;row++)
   wl_sheet_ink(x+5.8f+n*7,v,16-row*1.25f,x+(row==2?8:9.4f)+n*7,16-row*1.25f,.16f,row?0x9d8e72:0x6e7761);
 }
 ch_quad((ChVec){x-22,y-11,20.3f},(ChVec){x+22,y-11,20.3f},(ChVec){x+22,y-6,23},(ChVec){x-22,y-6,23},0x5f7057,1);
 ch_quad((ChVec){x-22,y-6,23},(ChVec){x+22,y-6,23},(ChVec){x+22,y-1,20.3f},(ChVec){x-22,y-1,20.3f},0x526456,1);
}
/* Two-sided route sign shapes read from the street as well as the trail.
 * The app uses the exact same site to actually pin a map route. */
static void wl_stamp_face(float x,float y,float z,int cleared){
 U trail=cleared?0xadc798:0xe9c489;
 /* A diamond is a route blaze, not a direction unavailable on this face. */
 wl_sheet_ink(x-3.4f,y,z,x,z+1.4f,.30f,trail);
 wl_sheet_ink(x,y,z+1.4f,x+3.4f,z,.30f,trail);
 wl_sheet_ink(x+3.4f,y,z,x,z-1.4f,.30f,trail);
 wl_sheet_ink(x,y,z-1.4f,x-3.4f,z,.30f,trail);
}
static void wl_field(void){
 if(ch.x>1510||ch.y<1550)return;
 wl_ranger();
 for(int i=0;i<LEN(wf_patches);i++){
  OuterPoint q=wf_point(wf_patches+i);if(wl_visible(q.x,q.y,5,16,260)<-9900)continue;
  if(wf_gathered()&(1u<<i)){
   wl_box(q.x-2,q.y-1,q.x+2,q.y+1,0,.8f,0x768a70,0xa2a987);continue;
  }
  int t=wf_patches[i].type;wl_specimen(q.x,q.y,t,7,0);wl_specimen(q.x-6,q.y+3,t,4,0);wl_specimen(q.x+5,q.y+4,t,5.4f,0);
 }
 for(int i=0;i<LEN(wf_stamps);i++){
  OuterPoint q=woodline_sites[wf_stamps[i]];float x=q.x,y=q.y;
  if(wl_visible(x,y,8,14,380)<-9900)continue;
  U c=(wf_stamps_done()&(1u<<i))?0x87b68b:0xe3b177;
  wl_box(x-1,y-1,x+1,y+1,0,13.5f,0x806950,0x9a835e);
  wl_box(x-5.4f,y-1.4f,x+5.4f,y+1.4f,10.8f,16.15f,c,mix(c,PAPER,2));
  wl_box(x-4.7f,y-1.52f,x+4.7f,y+1.52f,11.4f,15.35f,0x415d50,0x597060);
  wl_stamp_face(x,y-1.65f,13.28f,(wf_stamps_done()&(1u<<i))!=0);
  wl_stamp_face(x,y+1.65f,13.28f,(wf_stamps_done()&(1u<<i))!=0);
 }
 for(int i=0;i<LEN(wf_chests);i++){
  OuterPoint q=wf_point(wf_chests+i);if(wl_visible(q.x,q.y,7,18,400)>-9900)wl_cache(i,q.x,q.y);
 }
}
static void wl_animal(const WildActor*w,int i,int alert){
 if(!w->active||wl_visible(w->x,w->y,13,36,475)<-9900)return;
 const WildSpec*s=ww_spec+i;ChRig rig={w->x,w->y,cs(w->a),sn(w->a)};
 float gait=w->glide>8?sn(w->phase):0,h=i==WW_BEAR?14:i==WW_WOLF?9.4f:8.1f;
 float wide=i==WW_BEAR?6.1f:i==WW_WOLF?3.5f:3.1f,body=i==WW_BEAR?10.5f:8.5f;
 if(i==WW_BOBCAT&&w->state==WW_WARN)h-=2.f;
 for(int y=-1;y<=1;y+=2)for(int x=-1;x<=1;x+=2){
  float dx=x*y*gait*(i==WW_BEAR?2.4f:3.5f);
  ChVec hip={x*body*.72f,y*wide*.68f,h-.8f},ankle={hip.x+dx,hip.y+y*.4f,.9f};
  float r=i==WW_BEAR?1.65f:1.f;
  ch_link(rig,ankle,hip,r,r*1.28f,r*1.23f,r*1.36f,s->shade);
 }
 /* Tapered shoulders, haunches, foreface, raised ears and species tail.
  * The alert's body and physical strike direction share ww's heading. */
 ch_link(rig,(ChVec){-1,0,h-3},(ChVec){-1,0,h+3.5f},wide*.82f,wide,body*.85f,body,s->top);
 ch_link(rig,(ChVec){1,0,h+3},(ChVec){2,0,h+(i==WW_BEAR?8.3f:5.7f)},wide,wide*.71f,body*.9f,body*.62f,s->top);
 float raise=i==WW_BEAR&&w->state==WW_WARN?(1+sn(w->clock*2.6f))*2:0;
 float neck=h+4+raise;
 ch_link(rig,(ChVec){body-.6f,0,neck-1},(ChVec){body+2.8f,0,neck+4},wide*.72f,wide*.70f,3.4f,3.6f,s->top);
 ch_link(rig,(ChVec){body+5,0,neck-.4f},(ChVec){body+6,0,neck+2.2f},wide*.47f,wide*.42f,2.6f,2.9f,s->light);
 for(int k=-1;k<=1;k+=2){
  ChVec a=ch_joint(rig,body+1,k*wide*.42f,neck+3),b=ch_joint(rig,body+3.8f,k*wide*.60f,neck+3);
  wl_tri(a,b,ch_joint(rig,body+.7f,k*wide*.74f,neck+(i==WW_BEAR?6.4f:8)),s->shade);
 }
 if(i!=WW_BEAR)ch_link(rig,(ChVec){-body*(i==WW_WOLF?1.95f:1.35f),gait*1.4f,h-2},(ChVec){-body+1,0,h+1},1.3f,1.8f,1.3f,2.1f,s->shade);
 if(alert&&(w->state==WW_WARN||w->state==WW_STRIKE)){
  float maxp=s->reach*.78f;
  ch_quad(ch_joint(rig,7,-.8f,.33f),ch_joint(rig,maxp,-.95f,.33f),ch_joint(rig,maxp,.95f,.33f),ch_joint(rig,7,.8f,.33f),0xdd995d,1);
  wl_tri(ch_joint(rig,maxp-1,-2.3f,.37f),ch_joint(rig,maxp+3.5f,0,.37f),ch_joint(rig,maxp-1,2.3f,.37f),0xffc472);
 }
}
static void wl_life(void){
 if(ch.x>1480||ch.y<1520)return;
 if(ww_inc.state!=WW_QUIET){
  WildActor w={.active=1,.x=ww_inc.x,.y=ww_inc.y,.a=ww_inc.a,.phase=ww_inc.phase,.glide=64};
  wl_animal(&w,ww_inc.species,0);
 }
 for(int i=0;i<WW_SPECIES;i++)wl_animal(ww_wild+i,i,i==ww_chosen);
}
/* The journal/planter are modal UIs, not another walkable level. Compose the
 * intact controls on a contextual still-life; never use a fake walking state
 * or tick an actor to draw the menu. Save/restore all camera values we set. */
static int wl_journal_backdrop(void){
 if(!ch_pref()||fastGfx)return 0;
 float old[]={ch.x,ch.y,ch.h,ch.co,ch.si,chSin,chCos,chDark,chFog};
 ch.co=0;ch.si=1;ch.h=18;chSin=.10f;chCos=.995f;chDark=nw_dark();chFog=nwFog;
 if(wf_mode==WF_RANGER){
  ch.x=550;ch.y=2162;ch.si=-1;ch.h=33;chSin=.39f;chCos=.921f;ch_ground();ch_buildings();wl_wood();wl_ranger();
 }else{
  ch.x=0;ch.y=-39;ch.h=13;chDark=.19f;chFog=0;
  for(int y=0;y<H;y++){U color=nw_blend(0x172b31,0x304742,70*y/H);
   for(int x=0;x<W;x++){pixels[y*W+x]=color;chZ[y*W+x]=0;}}
  wl_box(-130,31,130,32,-8,92,0x45554a,0x45554a);
  for(int x=-110;x<110;x+=11)wl_box(x,30.6f,x+.8f,31,0,40,0x52695a,0x6a7f69);
  wl_box(-115,-15,115,27,0,5,0x735d45,0x987d56);
  /* Clockwork grow lights, cable drops and the back wall's shelf joints. */
  wl_box(-110,17,110,19,29,30.4f,0x576e62,0x879987);
  for(int k=-1;k<=1;k+=2){
   float x=53*k;
   wl_box(x-.35f,10,x+.35f,10.7f,19.8f,30,0x617765,0x8a9280);
   wl_box(x-8.0f,7.7f,x+8.0f,13.4f,18.7f,20.2f,0x627868,0x81917a);
   wl_box(x-7,8,x+7,13.1f,18.3f,18.75f,0xd8cf9e,0xd8cf9e);
  }
  for(int i=0;i<2;i++){
   float x=i?-53:53;int crop=wf_bed_crop(wf_shop,i),left=wf_bed_left(wf_shop,i);
   wl_box(x-11,-9,x+11,21,5,8.5f,0x766a50,0x483e36);
   if(crop>=0){
    if(crop<4)for(int m=-1;m<=1;m++)wl_specimen(x+3*m,7+4*m,crop,
      (left?4.f:8.f)+m*.7f,8.5f);
    else{
     WlRing f=wl_ring((unsigned)crop);for(int j=0;j<3;j++)wl_crown(&f,x+j*3-4,3*j,8.5f,3+j,1,left?5:10,0,0x94ae70);
    }
   }
   wl_box(x-12,20,x+12,21,5,9,0x746952,0xaa9d76);
  }
 }
 ch.x=old[0];ch.y=old[1];ch.h=old[2];ch.co=old[3];ch.si=old[4];chSin=old[5];chCos=old[6];chDark=old[7];chFog=old[8];
 return 1;
}
