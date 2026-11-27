/* Actual Afterimage and Shoal Walk room actor roster. Costume, feet, height,
 * routine and shop-side interaction anchor are shared by both cameras.
 * Crowd seats, recognizable staff and optional cast members have stable IDs. */
_Static_assert(NAR_END<=N_ACT_ROOM_END,"room actors must not overlap outdoor NPCs");
typedef struct {float x,y,z,tx,ty,t;const NcCast*costume;const char*name;int pose,face,mul,kind;} RoomCast;
static const short narCrowd[][3]={{391,264,5},{554,269,0},{620,275,14},{374,283,1},
 {423,286,9},{529,294,7},{585,299,2},{324,306,11},{470,311,12},{608,320,8},
 {344,327,3},{446,335,6},{513,339,13},{359,348,10},{567,351,4},{645,357,15}};
static const char*const narGuests[]={"Tess Mercer","Ari Reed","Mina Ashe","Jade Venn","Wren Holt","Sami Vale",
 "Rae Bell","Kite Allen","Finn Wade","Sid Roman","Mara Pike","Reese West","Lois Nash","Vik Hollis","Kit Quinn","Bex Rowan"};
static int npc_club_blueprint(int id,RoomCast*out){
 float t=nc_motion();
 RoomCast q={.mul=100,.kind=6,.face=1,.pose=CA_WAITING};int k;
 if(ncRoom==NC_FLOOR){
  if(id>=NAR_STAGE&&id<NAR_DJ){
   k=id-NAR_STAGE;int count=3+nc_get(NC_ACT);if(k>=count)return 0;
   q.t=t*(3.35f+nc_music()*.45f)+k*1.73f;q.x=480+(k*2-count+1)*30+sn(q.t*.62f)*5.f;q.y=209-(k&1)*5;
   q.z=20;q.tx=q.x;q.ty=244;q.costume=ncStageCast+k;q.name=ncStageNames[k];q.pose=CA_SWAY;q.mul=134;q.kind=7;
  }else if(id==NAR_DJ||id==NAR_BAR){
   k=id-NAR_DJ;q.x=k?800:159;q.y=216;q.z=22;q.tx=q.x;q.ty=k?278:280;
   q.costume=ncHouseCast+k;q.name=k?"Nell / Afterimage bar":"Drew / Afterimage DJ";q.t=t*(k?2.9f:3.5f);
   q.pose=CA_WORKING;q.face=k?-1:1;q.mul=117;q.kind=7;
  }else if(id>=NAR_DOOR&&id<NAR_GUEST){
   k=id-NAR_DOOR;if(k>=1+nc_get(NC_STAFF))return 0;
   static const char*const name[]={"Ira Venn / Door staff","Zane / Floor security","Arden / Floor security"};
   q.x=568+k*25;q.y=377+k*3;q.costume=ncHouseCast+(k?k+3:2);q.name=name[k];q.t=t*1.7f+k;q.face=k==2?-1:1;q.mul=108;q.kind=7;
  }else if(id>=NAR_GUEST&&id<NAR_REGULAR){
   k=id-NAR_GUEST;if(k>=(nc_owned()?cl(8+nc_get(NC_DOOR)/4,8,16):12))return 0;
   int j=0;while(j<LEN(narCrowd)&&narCrowd[j][2]!=k)j++;if(j==LEN(narCrowd))return 0;
   q.x=narCrowd[j][0]+sn(t*.83f+k*4.2f)*7;q.y=narCrowd[j][1]+sn(t*1.9f+k)*2;
   q.costume=ncGuestCast+k;q.name=narGuests[k];q.t=t*(2.6f+k*.07f)+k*1.7f;
   q.pose=k%3==0?CA_APPLAUSE:k%3==1?CA_SWAY:CA_WAITING;q.face=k%4>=2?-1:1;q.mul=93+k%3*4;
  }else if(id>=NAR_REGULAR&&id<NAR_DISPUTE){
   k=id-NAR_REGULAR;if(k==NCR_VOCALIST&&ncr_active())return 0;const NcRegular*r=ncr_cast+k;
   q.x=r->x;q.y=r->y;q.costume=ncRegularWardrobe+k;q.name=r->name;q.t=t*(1.35f+k*.17f)+k;
   q.face=k==NCR_PHOTOGRAPHER?-1:1;q.mul=110;q.kind=7;
  }else if(id>=NAR_DISPUTE&&id<NAR_SUITE){
   k=id-NAR_DISPUTE;int d=nc_get(NC_DISPUTE);if(!d||k>=1+d)return 0;
   static const char*const name[]={"Len Alder","Kit Fen","Luz Page","Aline Coast"};
   q.x=610+k*18+sn(t*3.7f+k)*3;q.y=396+k*3;q.costume=ncGuestCast+12+k;
   q.name=name[k];q.t=t*2.6f+k;q.pose=CA_APPLAUSE;q.face=-1;q.mul=92;
  }else return 0;
 }else if(ncRoom==NC_SUITE){
  k=id-NAR_SUITE;if(k<0||k>=6)return 0;int j=k%2,i=k/2,guest=2+i*3+j;
  static const char*const name[]={"Lina Finn","Niko Glen","Kora Sato","Evan Bell","Sage Tully","Ellis Fern"};
  q.x=210+i*266+(j?49:-49)+sn(t*.7f+i+j)*2;q.y=213;q.z=11;q.tx=q.x;q.ty=262;
  q.costume=ncGuestCast+guest;q.name=name[k];q.pose=CA_SEATED;q.t=t*(1.4f+j*.24f)+i+j;q.face=j?-1:1;q.mul=104;
 }else if(ncRoom==NC_OFFICE&&id==NAR_OFFICE){
  q.x=540;q.y=192;q.z=22;q.tx=q.x;q.ty=266;q.costume=ncHouseCast+3;
  q.name="Ellie / Ledger clerk";q.t=t*1.43f;q.pose=CA_WORKING;q.mul=108;q.kind=7;
 }else return 0;
 if(!q.tx){q.tx=q.x;q.ty=q.y;}*out=q;return 1;
}
static int npc_other_room_key(void){
 if(scene==NIGHT_CLUB_SCENE)return 90+ncRoom;
 if(scene==NIGHT_VENUES_SCENE&&nvs_room==NVR_GLASS&&nvs_mode==4)return 98;
 return 0;
}
static int npc_other_player(float*x,float*y,float**heading){
 if(scene==NIGHT_CLUB_SCENE){*x=ncX;*y=ncY;*heading=&ncAngle;return 1;}
 if(npc_other_room_key()==98){*x=nvs_rail_point();*y=332;*heading=&nvs_aim_angle;return 1;}
 return 0;
}
static int npc_rail_solid(float x,float y){return x<71.f||x>919.f||y<297.f||y>352.f;}
static PulseRoomSolid npc_other_solid(void){
 if(scene==NIGHT_CLUB_SCENE)return nc_solid;
 if(npc_other_room_key()==98)return npc_rail_solid;
 return 0;
}
static void npc_other_death(void){
 if(scene==NIGHT_CLUB_SCENE)nc_use_target(NCA_EXIT);
 else if(npc_other_room_key()==98){nvs_mode=nvs_room=0;scene=1;}
}
static int npc_other_room_ref(int id,NpcRef*r){
 if(scene==NIGHT_CLUB_SCENE){
  RoomCast q;if(!npc_club_blueprint(id,&q))return 0;
  npc_spot(r,id,q.kind,q.name,q.x,q.y,q.face>0?.88f:2.26f,
           q.kind==7?MODEL_STAFF:MODEL_CIVILIAN,id%7,q.costume->cloth);
  r->tx=q.tx;r->ty=q.ty;return 1;
 }
 if(id==NAR_INEZ&&npc_other_room_key()==98)
  return npc_spot(r,id,7,"Inez / Coast Keeper",151,329,3.14159265f,MODEL_STAFF,1,0xbcb98a);
 return 0;
}
static int npc_club_regular_live(int regular){return !npc_replaces(NAR_REGULAR+regular);}
static int npc_club_actor_ref(int id,RoomCast*q,NpcRef*r){
 if(!npc_club_blueprint(id,q)||npc_replaces(id)||!npc_ref(id,r))return 0;
 if(npc_holds(id)||(r->x!=q->x||r->y!=q->y)){
  q->x=r->x;q->y=r->y;q->face=cs(r->a)>=0?1:-1;q->t=0;
  if(!q->z)q->pose=CA_WAITING;
 }
 return 1;
}
static void npc_club_unit(int id){
 RoomCast q;NpcRef r;if(!npc_club_actor_ref(id,&q,&r))return;NcCast skin=*q.costume;skin.cloth=r.coat;
 int x=(int)(q.x+.5f),y=(int)(q.y+.5f);
 nc_cast_actor(x,y,q.mul,&skin,q.pose,q.t,q.face);
 if(id>=NAR_REGULAR&&id<NAR_DISPUTE){
  int i=id-NAR_REGULAR,close=(ncPanel==NC_REGULAR&&ncr_contact==i)||ncr_near()==i;
  if(close)nc_label(x,y-43,r.name,i==NCR_PROMOTER?ncGold:ncIce);
  else disc(x,y-34,2,i==NCR_PROMOTER?ncGold:ncIce);
 }
}
static int npc_club_direct(U keys,U tap,int reserved){
 if(!(tap&USE)||(!(keys&BRAKE)&&reserved))return 0;
 return npc_open_near();
}
static void npc_aqua_actors(void){
 NpcRef r;if(!npc_replaces(NAR_INEZ)&&npc_ref(NAR_INEZ,&r))person((int)r.x,(int)r.y,r.coat,0,r.a,r.model,r.hue);
 npc_private_bodies(0,0);
 player_in_room(nvs_rail_point(),332,0,0,0x51948b,nvs_rail_walk?(int)(nvs_frames%16)/4:0,nvs_aim_angle,1,npc_rail_solid);
}
