/* One session identity for each physical person that used to be a purely
 * decorative sprite or chase figure. The owning job still decides when its
 * contact is on duty and what E does at its original interactive station. */
typedef struct {int damage,ticks,flash;float x,y,a;U coat;} StreetExtraHealth;
static StreetExtraHealth streetExtraHealth[N_ACT_OUT_EXTRA_END];
static int street_extra_id(int id){
 return id==N_ACT_CROSS||(id>=N_ACT_RIDER&&id<N_ACT_RIDER+LEN(riders))||
        (id>=N_ACT_OUT_QUEUE&&id<N_ACT_OUT_EXTRA_END);
}
static void street_extra_reset(void){z(streetExtraHealth,sizeof streetExtraHealth);}
static int street_extra_down(int id){return street_extra_id(id)&&streetExtraHealth[id].ticks>0;}
static void street_extra_tick(void){
 if(scene!=1||menu>=0||mapOn)return;
 for(int id=N_ACT_RIDER;id<N_ACT_OUT_EXTRA_END;id++)if(street_extra_id(id)){
  StreetExtraHealth*d=streetExtraHealth+id;
  if(d->flash>0)d->flash--;
  if(d->ticks>0&&!--d->ticks)*d=(StreetExtraHealth){0};
 }
}
static int street_extra_ref(int id,NpcRef*r,int*step){
 if((scene!=1&&!(scene==2&&helpBack==1))||street_extra_down(id))return 0;
 if(step)*step=0;
 if(id>=N_ACT_OUT_QUEUE&&id<N_ACT_OUT_SIGNAL){
  static const char *const name[]={"Lou Baines","Emi Lune","Dara Flint","Ray Doyle","Peri Quinn","Jo Sable","Ash Mercer","Robin Dove"};
  static const U coats[]={0xb487ae,0x6ca9b2,0xd8af80,0x52627d};
  int i=id-N_ACT_OUT_QUEUE,door=night_club_listing();
  if(i>=night_club_queue()||!ordinaryDoorX[door])return 0;
  float x=ordinaryDoorX[door]-21-(i%4)*16,y=ordinaryDoorY[door]+16+(i/4)*13;
  if(solid(x,y,5))return 0;
  return npc_spot(r,id,6,name[i],x,y,i%3?1.f:-2.f,MODEL_CIVILIAN,i%5,coats[i%4]);
 }
 if(id>=N_ACT_OUT_SIGNAL&&id<N_ACT_OUT_CLERK){
  int j=1+id-N_ACT_OUT_SIGNAL,at=np_job_dest();
  if(j!=np_job()||at<0||np_get(NP_HOLD,600)>0)return 0;
  static const int dx[]={22,-22,16,-22,0},dy[]={8,9,22,20,20};
  for(int i=0;i<LEN(dx);i++){
   int x=loc[at].x+dx[i],y=loc[at].y+dy[i];
   if(!solid(x,y,7))return npc_spot(r,id,4,np_brief[j].name,x,y,look(p.x-x,p.y-y),MODEL_CIVILIAN,j%3,np_color[j%NP_NAPP]);
  }
  return 0;
 }
 if(id==N_ACT_OUT_CLERK)
  return npc_spot(r,id,4,"Ellis / Harbor clerk",hj_office.x+24,hj_office.y-3,1.57f,MODEL_DISPATCH,2,0x718da0);
 if(id==N_ACT_OUT_RANGER){
  OuterPoint q=woodline_sites[WOOD_RANGER];
  return npc_spot(r,id,8,"Rowan / Woodline ranger",q.x+24,q.y+8,1.570796f,MODEL_STAFF,1,0xa5b882);
 }
 if(id>=N_ACT_OUT_HIKER&&id<N_ACT_OUT_VOCAL){
  int i=id-N_ACT_OUT_HIKER;const WildHiker*h=ww_hikers+i;
  if(h->away||h->x<1.f||h->y<1.f)return 0;
  if(step)*step=(int)(sn(p.secs*4.7f+i)*2.8f);
  return npc_spot(r,id,0,i?"Ren / Seawall hiker":"Parker / Seawall hiker",h->x,h->y,
                  sn(p.secs*.56f+i*2.5f+.93f)>0?0:3.14159f,MODEL_CIVILIAN,2+i,i?0x7ca8a0:0xc4a16f);
 }
 if(id==N_ACT_OUT_VOCAL){
  float x,y;if(!ncr_ped(&x,&y))return 0;
  const NcRegular*c=ncr_cast+NCR_VOCALIST;
  if(step)*step=ncr.phase==NCR_WAIT?0:(int)(sn(anim*12.f)*4);
  return npc_spot(r,id,6,c->name,x,y,ncr.a,c->model,c->hue,c->coat);
 }
 if(id>=N_ACT_OUT_MEDIA&&id<N_ACT_OUT_EXTRA_END){
  int i=id-N_ACT_OUT_MEDIA;
  if(vdActive!=VA_B7||vm.newsPhase!=2||vm.van<0||vm.event!=(i==2))return 0;
  static const int dx[]={-10,0,-5},dy[]={1,0,-3},hue[]={1,4,2};
  static const U coat[]={0x925d9d,0xd98569,0xe8b876};
  static const char*name[]={"Alden / Copper duo","Fern / Copper duo","Quill / Yard crew"};
  if(step)*step=(i==0?-1:1)*(int)(sn(vm.phaseT*.14f)*4);
  return npc_spot(r,id,6,name[i],vm_subject_x()+dx[i],vm_subject_y()+dy[i],
      vm.phaseT<240?0.f:3.14159f,MODEL_CIVILIAN,hue[i],coat[i]);
 }
 return 0;
}
static int street_extra_pose(int id,StreetPerson*out){
 NpcRef r;int step;
 if(npc_replaces(id)||!street_extra_ref(id,&r,&step))return 0;
 npc_present_pose(id,&r.x,&r.y,&r.a,&step);
 *out=(StreetPerson){r.x,r.y,r.a,step,r.model,r.hue,streetExtraHealth[id].flash>0?0xffdfbc:r.coat};
 return 1;
}
static int street_extra_fallen(int id,StreetPerson*out){
 if(!street_extra_down(id)||npc_replaces(id))return 0;
 const StreetExtraHealth*d=streetExtraHealth+id;
 *out=(StreetPerson){.x=d->x,.y=d->y,.a=d->a,.coat=d->coat};
 return 1;
}
static void street_extra_down_paint(void){
 StreetPerson v;
 for(int id=N_ACT_RIDER;id<N_ACT_OUT_EXTRA_END;id++)if(street_extra_fallen(id,&v)){
  int x=sx(v.x),y=sy(v.y);if(x<-16||y<-16||x>W+16||y>H+16)continue;
  float u=cs(v.a),w=sn(v.a);
  Pt h=rig(x,y,u,w,-7,0,2),a=rig(x,y,u,w,-3,0,1),b=rig(x,y,u,w,4,0,1);
  rod(a,b,2,v.coat);disc(h.x,h.y,2,0xbe9b88);
  for(int side=-1;side<=1;side+=2)rod(b,rig(x,y,u,w,9,side*2,0),1,0x495364);
 }
}
static OuterHit street_extra_pick(float x,float y,float co,float si,int melee,float limit,float minimum,float mx,float my){
 OuterHit best={-1,0,0,limit};
 if(scene!=1)return best;
 for(int id=N_ACT_RIDER;id<N_ACT_OUT_EXTRA_END;id++){
  if(!street_extra_id(id)||npc_replaces(id)||npc_holds(id))continue;
  NpcRef r;if(!npc_ref(id,&r))continue;
  float dx=r.x-x,dy=r.y-y,f=dx*co+dy*si,l=absf(dy*co-dx*si),d=melee?d2(dx,dy):f;
  if(f<=0||f<minimum||d>=best.rank||(melee?l>13.f:l>=13.f+f*.12f))continue;
  if(!street_reach_open(x,y,r.x,r.y,1,-1)||
     (!melee&&(mx!=x||my!=y)&&!street_reach_open(mx,my,r.x,r.y,1,-1)))continue;
  best=(OuterHit){id,r.x,r.y,d};
 }
 return best;
}
static int street_extra_injure(int id,int damage,float x,float y){
 if(damage<=0||!street_extra_id(id)||street_extra_down(id))return 0;
 StreetExtraHealth*d=streetExtraHealth+id;
 NpcRef r;int found=npc_raw_ref(id,&r);
 if(!found&&!npc_replaces(id))return 0;
 if(!found)r=npcEnc.actor;
 d->damage+=damage;d->flash=13;
 if(d->damage<3)return 0;
 d->ticks=600;d->x=x;d->y=y;d->a=r.a;d->coat=r.coat;
 return 1;
}
static int street_extra_car_hit(float x,float y,float co,float si){
 if(scene!=1)return 0;
 for(int id=N_ACT_RIDER;id<N_ACT_OUT_EXTRA_END;id++){
  NpcRef r;if(!street_extra_id(id)||npc_replaces(id)||npc_holds(id)||!npc_ref(id,&r))continue;
  float dx=r.x-x,dy=r.y-y;
  if(absf(dx*co+dy*si)<15.f&&absf(dy*co-dx*si)<6.5f)return street_extra_injure(id,3,r.x,r.y);
 }
 return 0;
}
