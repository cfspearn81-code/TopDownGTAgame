/* All live people share the same conversation and single-entry robbery
 * transaction. Careers carry the existing wallet; encounters, greeted
 * lines, temporary positions and defeated-body art never enter the save. */
#include "npc_greetings.inc"
enum {NPC_IDLE,NPC_DRAW,NPC_FIGHT,NPC_YIELD,NPC_DROP,NPC_RECOVER};
enum {NPC_NONE=-1,NPC_NEAR=38,NPC_KEEP_TALK=64,NPC_LOOT_TICKS=1080};
typedef struct{
 int id,kind,model,hue;
 float x,y,a,tx,ty;
 U coat;
 const char*name;
} NpcRef;
static int outer_person_ref(int id,NpcRef*r,int*step);
typedef struct{
 NpcRef actor;
 int phase,room,ticks,prize,hp,surrender;
 float bornX,bornY,cool,flash,strike;
} NpcEncounter;
static unsigned char npcLetters[N_ACT_MAX],npcSpent[N_ACT_MAX];
static NpcRef npcTalk;
static int npcTalkOn,npcTalkRoom,npcChoice;
typedef struct {float x,y,a;int room;} NpcRejoin;
static NpcRejoin npcRejoin[N_ACT_GROW-N_ACT_DANCE+1];
static NpcEncounter npcEnc;
static int npcTraceTicks;
static PulsePose npcTracePose;
static float npcTraceEndX,npcTraceEndY,npcTracePlayerX,npcTracePlayerY,npcTraceFacing;
static U npcSuppressed;
static const char *npcStreetNames[]={
 "Avery Rune","Hazel Sato","Milo Shaw","June Birch","Elliot Venn","Faye Hart","Kieran Park","Tess Onyx",
 "Ellis Vale","Sofia Lin","Remy Finch","Kai West","Orla Reed","Jules Moss","Nico Frost","Inez Ward",
 "Mina Holt","Kit Rowan","Leah Quill","Max Wren","Rae Alder","Cam Rivers","Pia Rook","Drew Shore",
 "Nell Frost","Ira Bloom","Oli Day","Sid Lake","Lena Brook","Reese Cross","Perry Loom","Ivo Reed",
 "Rue Stone","Ari Dean","Hollis Vega","Zane Tide","Mara Elm","Sol Bryce","Briar Gray","Casey Wynn",
 "Sable","Kirby","Rook","Beck","Officer Torres","Officer Chen","Officer Reed","Officer Reyes",
 "Officer Mara","Officer Lane","Officer Vale","Officer Sato"
};
_Static_assert(LEN(npcStreetNames)==LEN(peds),"live street identities must map one-to-one");
static int npc_room_key(void){
 if(scene==1)return 1;
 if(scene==3&&interior_active_loc>=0)return 32+interior_active_loc*2+interior_subroom;
 return 0;
}
static void npc_player_point(float*x,float*y){
 if(scene==3){*x=interior_x;*y=interior_y;}else{*x=p.x;*y=p.y;}
}
static int npc_clear(float x,float y,float tx,float ty){
 return pulse_path(x,y,tx,ty,scene==3?int_solid:0);
}
static int npc_spot(NpcRef*r,int id,int kind,const char*name,float x,float y,float a,int model,int hue,U coat){
 *r=(NpcRef){id,kind,model,hue,x,y,a,x,y,coat,name};return 1;
}
static int npc_raw_ref(int id,NpcRef*r){
 if(scene==1){
  if(id>=N_ACT_OUTER&&id<N_ACT_MAX)return outer_person_ref(id,r,0);
  if(id>=N_ACT_STREET&&id<N_ACT_STREET+LEN(peds)){
   int i=id-N_ACT_STREET;Ped*q=peds+i;if(q->hp<=0)return 0;
   U coat=q->role==1?0x668df0:q->role==2?PINK:(i%2)?0x92b7b4:0xca8c71;
   return npc_spot(r,id,q->role==1?1:q->role==2?2:0,npcStreetNames[i],q->x,q->y,q->a,q->role,i,coat);
  }
  if(id>=N_ACT_RIDER&&id<N_ACT_RIDER+LEN(riders)&&id==N_ACT_RIDER+taxi.rider&&
     ((job.kind==JOB_TAXI&&taxi.phase!=TAXI_RIDING)||(taxi.phase==TAXI_WALKING&&taxi.linger>0.f))){
   return npc_spot(r,id,3,riders[taxi.rider].name,taxi.x,taxi.y,taxi.a,MODEL_CIVILIAN,riders[taxi.rider].face,riders[taxi.rider].outfit);
  }
  #if __has_include("verdict_street.inc")
  if(id==N_ACT_CROSS&&cr.locked&&cr.mode==CR_KEEPER&&(vdActive==VA_B6A||vdActive==VA_B6B))
   return npc_spot(r,id,9,"Remy / Crossing crew",CR_X+28,cr.workerY,cr.worker==2?-1.57f:1.57f,MODEL_CIVILIAN,2,0xc0dc8d);
  #endif
  return 0;
 }
 if(scene!=3||interior_active_loc<0||interior_active_loc>=LEN(loc))return 0;
 int n=interior_active_loc,k=loc[n].kind;
 if(k==15&&(id==N_ACT_CHIEF||id==N_ACT_HQ1||id==N_ACT_HQ2)){
  if(id==N_ACT_CHIEF){
   npc_spot(r,id,1,"Chief Rivas",CHIEF_X,175,1.570796f,MODEL_OFFICER,1,0x8593ad);r->ty=242;return 1;
  }
  int second=id==N_ACT_HQ2;return npc_spot(r,id,1,second?"Officer Ochoa":"Officer Elan",second?541:418,second?163:162,
      1.570796f,MODEL_OFFICER,second?3:2,second?0x8697a5:0x75879f);
 }
 if(k!=13&&(k==1?interior_subroom:!interior_subroom)&&id==(interior_subroom?N_ACT_REAR:N_ACT_FRONT)+n){
  for(int i=0;i<interior_nprop;i++){
   const InteriorProp*f=interior_props+i;
   if(f->x+f->w/2!=interior_desk_x||f->y+f->h+25!=interior_desk_y)continue;
   npc_spot(r,id,k==15||k==11?1:k==1||k==0?5:4,loc[n].npc,interior_desk_x,f->y-8,1.570796f,
            MODEL_STAFF,k%5,mix(interior_theme(k)->neon,0x55738a,3));
   r->ty=interior_desk_y;return 1;
  }
 }
 if(k==1&&!interior_subroom){
  if(id>=N_ACT_DANCE&&id<N_ACT_DANCE+3){
   int j=id-N_ACT_DANCE;static const char*name[]={"Syl Voss","Darcy Kite","Neve Rio"};
   const int x[]={383,450,405},y[]={263,287,324};const U coat[]={0xa987ba,0xc7a578,0x729db7};
   float a=anim*(1.6f+.35f*j)+j*2.2f;
   return npc_spot(r,id,6,name[j],x[j]+(int)(sn(a)*7),y[j]+(int)(cs(a)*4),a,MODEL_CIVILIAN,2+j,coat[j]);
  }
  if(id==N_ACT_DJ)return npc_spot(r,id,7,"DJ Marlow",301,150,1.570796f,MODEL_STAFF,3,0xc6a8d7);
  if(id==N_ACT_MIKA){
   int i=v39.patron,t=v39.tick,m=v39.playerTick;
   int phase=m?v39_motion(m,1,25):t<V39_SIP?v39_motion(t,0,58):100-v39_motion(t,V39_SIP,V39_CYCLE);
   int target=m?V39_PLAYER_X:v39Px[i],x=(V39_PLAYER_X*(100-phase)+target*phase)/100;
   npc_spot(r,id,7,"Mika",x,V39_BAR_TOP-12,1.570796f,MODEL_STAFF,4,0x8bbeb4);
   r->tx=V39_PLAYER_X;r->ty=V39_PLAYER_Y;return 1;
  }
  if(id==N_ACT_PATRON||id==N_ACT_PATRON+1){
   int i=id-N_ACT_PATRON,pt,x,y;v39_patron_at(i,&pt,&x,&y);
   return npc_spot(r,id,6,i?"Val Marino":"Sasha Fern",x,y,pt>=V39_SIP+12?1.570796f:-1.570796f,
          MODEL_CIVILIAN,i+1,i?0xabb7e6:0xca937f);
  }
 }
 if(k==4&&interior_subroom&&id==N_ACT_GROW){
  GrowBatch b=grow_batch(sv.b);GrowPose a=grow_operator_pose(sv.b,&b,growth_fraction(0),grow_pack_left,anim);
  if(a.phase!=GROW_OFF)return npc_spot(r,id,8,"Renna / Grow operator",a.x,a.y,a.angle,MODEL_WORKER,0,0xefad66);
 }
 return 0;
}
static int npc_holds(int id){
 return npcTalkOn&&id==npcTalk.id&&npcTalkRoom==npc_room_key();
}
static NpcRejoin *npc_rejoin_for(int id){
 return id>=N_ACT_DANCE&&id<=N_ACT_GROW?&npcRejoin[id-N_ACT_DANCE]:0;
}
/* Called by both the picker and the live actor painter, so the person
   selected, displayed and handed into the draw is the same pose. */
static int npc_present_pose(int id,float*x,float*y,float*a,int*step){
 if(npc_holds(id)){
  float px,py;npc_player_point(&px,&py);
  *x=npcTalk.x;*y=npcTalk.y;
  *a=d2(px-*x,py-*y)>.05f?look(px-*x,py-*y):npcTalk.a;
  *step=0;return 1;
 }
 NpcRejoin*q=npc_rejoin_for(id);
 if(q&&q->room&&q->room==npc_room_key()){
  *x=q->x;*y=q->y;*a=q->a;
  *step=(int)(sn(anim*12.f)*3.f);return 2;
 }
 return 0;
}
static int npc_ref(int id,NpcRef*r){
 if(!npc_raw_ref(id,r))return 0;
 int direct=r->tx==r->x&&r->ty==r->y,step=0;
 npc_present_pose(id,&r->x,&r->y,&r->a,&step);
 if(direct){r->tx=r->x;r->ty=r->y;}
 return 1;
}
/* The greenhouse's authored carry route turns through these open aisles.
   Vault actors normally rejoin in a single open segment. */
static int npc_rejoin_goal(int id,float x,float y,float tx,float ty,float *gx,float *gy){
 if(npc_clear(x,y,tx,ty)){*gx=tx;*gy=ty;return 1;}
 if(id!=N_ACT_GROW)return 0;
 static const int chain[][2]={{557,259},{557,170},{646,170},{556,279}};
 float best=1e9f;int found=0;
 for(int i=0;i<LEN(chain);i++)if(npc_clear(x,y,chain[i][0],chain[i][1])){
  for(int j=0;j<LEN(chain);j++)if(npc_clear(chain[j][0],chain[j][1],tx,ty)){
   for(int k=-1;k<LEN(chain);k++){
    float ix=chain[i][0],iy=chain[i][1],jx=chain[j][0],jy=chain[j][1];
    float kx=k<0?ix:chain[k][0],ky=k<0?iy:chain[k][1];
    if(!npc_clear(ix,iy,kx,ky)||!npc_clear(kx,ky,jx,jy))continue;
    float score=sq(d2(x-ix,y-iy))+sq(d2(ix-kx,iy-ky))+sq(d2(kx-jx,ky-jy))+sq(d2(jx-tx,jy-ty));
    if(score>=best)continue;
    if(d2(x-ix,y-iy)>.04f){*gx=ix;*gy=iy;}
    else if(d2(x-kx,y-ky)>.04f){*gx=kx;*gy=ky;}
    else {*gx=jx;*gy=jy;}
    best=score;found=1;
   }
  }
 }
 return found;
}
static void npc_resume_room_actors(void){
 int room=npc_room_key();
 for(int id=N_ACT_DANCE;id<=N_ACT_GROW;id++){
  NpcRejoin*q=npc_rejoin_for(id);
  if(!q->room)continue;
  NpcRef r;if(q->room!=room||!npc_raw_ref(id,&r)){q->room=0;continue;}
  if(npc_holds(id)||npc_replaces(id))continue;
  float dx=r.x-q->x,dy=r.y-q->y;
  if(d2(dx,dy)<=1.75f*1.75f&&npc_clear(q->x,q->y,r.x,r.y)){q->room=0;continue;}
  float tx,ty;
  if(!npc_rejoin_goal(id,q->x,q->y,r.x,r.y,&tx,&ty)){
   /* If a procedural station crossed a closed footprint, keep the pose
      here until that station is clear instead of crossing furniture. */
   continue;
  }
  dx=tx-q->x;dy=ty-q->y;float d=sq(d2(dx,dy));
  if(d>.01f)q->a=look(dx,dy);
  if(d<=1.75f){q->x=tx;q->y=ty;}
  else {q->x+=dx*1.75f/d;q->y+=dy*1.75f/d;}
 }
}
static void npc_end_talk(int rejoin){
 NpcRejoin*q=npc_rejoin_for(npcTalk.id);
 if(q&&npcTalkOn){
  *q=(NpcRejoin){npcTalk.x,npcTalk.y,npcTalk.a,rejoin&&npcTalkRoom==npc_room_key()?npcTalkRoom:0};
 }
 npcTalkOn=0;
}
static int npc_replaces(int id){return npcEnc.phase!=NPC_IDLE&&npcEnc.actor.id==id&&npcEnc.room==npc_room_key();}
static int npc_near_ref(NpcRef*out,int context){
 if(!npc_room_key()||p.car>=0||p.dead||mapOn||atmOpen>=0||wheelOpen)return 0;
 float x,y;npc_player_point(&x,&y);float best=100000.f;int found=0;
 float a=scene==3?interior_a:p.a;
 for(int id=0;id<N_ACT_MAX;id++){
  if(scene==3&&id<N_ACT_FRONT)continue;
  if(scene==1&&id>=N_ACT_FRONT&&id!=N_ACT_CROSS&&id<N_ACT_OUTER)continue;
  NpcRef r;if(npc_replaces(id)||!npc_ref(id,&r))continue;
  float dx=r.x-x,dy=r.y-y,d=d2(dx,dy);
  int touch=d<NPC_NEAR*NPC_NEAR&&npc_clear(x,y,r.x,r.y);
  if(!touch&&context&&scene==3&&(r.tx!=r.x||r.ty!=r.y)){
   dx=r.tx-x;dy=r.ty-y;d=d2(dx,dy);
   touch=d<46*46&&npc_clear(x,y,r.tx,r.ty);d+=380.f;
  }
  /* Distance is primary; your facing breaks close ties in a crowd. */
  if(dx*cs(a)+dy*sn(a)<0)d+=170;
  if(touch&&d<best){best=d;*out=r;found=1;}
 }
 return found;
}
static int npc_start_talk(NpcRef r){
 if(npcEnc.phase!=NPC_IDLE||npcTalkOn)return 0;
 npcTalkOn=1;npcTalk=r;npcTalkRoom=npc_room_key();npcChoice=1;return 1;
}
static int npc_open_near(void){
 NpcRef r;
 if(scene==3&&interior_active_loc>=0){
  int k=loc[interior_active_loc].kind;
  if(k==15&&int_at_chief()&&npc_ref(N_ACT_CHIEF,&r))return npc_start_talk(r);
  if(int_at_desk()){
   if(k==1&&!interior_subroom&&npc_ref(N_ACT_MIKA,&r))return npc_start_talk(r);
   if(npc_ref((interior_subroom?N_ACT_REAR:N_ACT_FRONT)+interior_active_loc,&r))return npc_start_talk(r);
  }
 }
 return npc_near_ref(&r,scene==3)&&npc_start_talk(r);
}
static int npc_front_ref(NpcRef*r){
 if(scene!=3||interior_active_loc<0||!int_at_desk())return 0;
 int k=loc[interior_active_loc].kind;
 if(menu!=k)return 0;
 return npc_ref((interior_subroom?N_ACT_REAR:N_ACT_FRONT)+interior_active_loc,r);
}
static int npc_front_option(void){NpcRef r;return npcEnc.phase==NPC_IDLE&&npc_front_ref(&r);}
static int npc_open_context(void){NpcRef r;return npc_front_ref(&r)&&npc_start_talk(r);}
static const char *npc_prompt(void){
 static char hint[90];
 NpcRef r;
 if(npcTalkOn||npcEnc.phase!=NPC_IDLE||!npc_near_ref(&r,0))return 0;
 append(append(hint,"E  /  INTERACT WITH "),r.name);return hint;
}
static void npc_forget_encounter(void){
 if(npcEnc.phase!=NPC_IDLE&&npcEnc.room>=32)interior_cache_loc=-2;
 npcEnc.phase=NPC_IDLE;npcTraceTicks=0;
}
static void npc_reset(void){
 npc_forget_encounter();npcTalkOn=0;npcSuppressed=0;z(npcRejoin,sizeof npcRejoin);
 z(npcLetters,sizeof npcLetters);z(npcSpent,sizeof npcSpent);
}
/* Independent full-period 32-bit stream; rejecting the short tail avoids
 * bias in the uniform 141-value integer credit interval. */
static U npc_roll_bound(U limit){
 U edge=(0u-limit)%limit,v;
 do{
  npcRandom=npcRandom*1664525u+1013904223u;v=npcRandom;
  v^=v>>16;v*=0x7feb352du;v^=v>>15;v*=0x846ca68bu;v^=v>>16;
 }while(v<edge);
 return v%limit;
}
static void npc_hello(NpcRef r){
 int id=r.id;
 const char*greeting=npc_greet_line(r.kind,(unsigned)id,npcLetters[id]);
 npcLetters[id]=(unsigned char)((npcLetters[id]+1)%NPC_VOICE_CYCLE);
 char line[112];append(append(append(line,r.name),": "),greeting);tell(line);
}
static int npc_stage(NpcRef *r,float x,float y,float *angle){
 PulseRoomSolid collision=scene==3?int_solid:0;
 float heading=look(r->x-x,r->y-y);
 /* The existing compact pose supports close actors; only true sprite
    overlap lacks a useful facing. Preserve wall and counter guards. */
 if(d2(r->x-x,r->y-y)>3*3&&npc_clear(x,y,r->x,r->y)&&
    !pulse_cover(x,y,pulse_aim(x,y,heading,0,0),1.f,collision)){
  *angle=heading;return 1;
 }
 /* A staffed counter is a conversation anchor, not a line through the
    furniture. Only start if there is room to step onto the public side. */
 if(scene!=3||(r->tx==r->x&&r->ty==r->y))return 0;
 static const int turn[]={0,1,-1,2,-2,3,-3,4};
 for(int i=0;i<LEN(turn);i++){
  float a=wrap(heading+.785398f*turn[i]),tx=x+32.f*cs(a),ty=y+32.f*sn(a);
  if(!npc_clear(x,y,tx,ty)||pulse_cover(x,y,pulse_aim(x,y,a,0,0),1.f,collision))continue;
  r->x=tx;r->y=ty;*angle=a;return 1;
 }
 return 0;
}
static int npc_begin(NpcRef r){
 if(sv.gear<0){tell("You need an owned pistol to attempt a robbery.");return 0;}
 if(npcSpent[r.id]){tell("They are keeping their distance after the last encounter.");return 0;}
 if(npcEnc.phase!=NPC_IDLE)return 0;
 float x,y,a;npc_player_point(&x,&y);
 if(!npc_stage(&r,x,y,&a)){tell("No clear line. Step into open space before drawing.");return 0;}
 npcSpent[r.id]=1;
 npcEnc=(NpcEncounter){.actor=r,.phase=NPC_DRAW,.room=npc_room_key(),.ticks=23,
   .prize=15+(int)npc_roll_bound(141),.hp=3,.bornX=x,.bornY=y,.cool=.4f};
 npcEnc.surrender=npc_roll_bound(100)<35;
 npcEnc.actor.a=look(x-r.x,y-r.y);
 wheelFists=0;
 if(scene==3)interior_cache_loc=-2;
 if(scene==1)p.a=a;else interior_a=a;
 /* End only this rider's active booking. Other contacts remain available
    and a greeting or failed draw must not void a legitimate fare. */
 if(scene==1&&job.kind==JOB_TAXI&&r.id==N_ACT_RIDER+taxi.rider){
  end_job();taxi.linger=0;tell("The rider cancels the fare as you draw. No fare will be paid.");
 }else tell("You draw your pistol. Nothing fires automatically.");
 return 1;
}
static int npc_live_talk(NpcRef*r){
 if(npcTalkRoom!=npc_room_key()||!npc_ref(npcTalk.id,r)||p.car>=0||p.dead||mapOn||atmOpen>=0)return 0;
 float x,y;npc_player_point(&x,&y);
 return (d2(x-r->x,y-r->y)<NPC_KEEP_TALK*NPC_KEEP_TALK&&npc_clear(x,y,r->x,r->y))||
    ((r->tx!=r->x||r->ty!=r->y)&&d2(x-r->tx,y-r->ty)<NPC_KEEP_TALK*NPC_KEEP_TALK&&npc_clear(x,y,r->tx,r->ty));
}
static U npc_filter_keys(U keys,U *tap){
 npcSuppressed&=keys;keys&=~npcSuppressed;*tap&=~npcSuppressed;
 /* Signal, field guide and career catalog are modal views, not physical
    exits. Keep the confrontation, its remaining drop and rejoining actors
    suspended. An actual profile change/respawn explicitly calls npc_reset;
    real building changes and entering a car are still handled below. */
 if(!p.dead&&(scene==2||scene==NIGHT_PHONE_SCENE||((scene==0||scene==5)&&profileReturn)))return keys;
 for(int i=0;i<LEN(npcRejoin);i++)if(npcRejoin[i].room&&npcRejoin[i].room!=npc_room_key())npcRejoin[i].room=0;
 if(npcEnc.phase!=NPC_IDLE&&(npcEnc.room!=npc_room_key()||p.dead||p.car>=0))npc_forget_encounter();
 if(!npcTalkOn){
  if(npcEnc.phase==NPC_DRAW){npcSuppressed|=keys&(FIRE|PUNCH);keys&=~(FIRE|PUNCH);*tap&=~(FIRE|PUNCH);}
  return keys;
 }
 NpcRef r;
 if(!npc_live_talk(&r)){npc_end_talk(!p.dead);*tap=0;return 0;}
 U t=*tap;*tap=0;
 if(t&ESC){npc_end_talk(1);npcSuppressed|=keys&(USE|FIRE|PUNCH);return 0;}
 if(t&UP)npcChoice=(npcChoice+2)%3;
 if(t&DOWN)npcChoice=(npcChoice+1)%3;
 if(t&(ENTER|USE)){
  npcSuppressed|=keys&(USE|FIRE|PUNCH);
  int drew=npcChoice==2&&npc_begin(r);
  npc_end_talk(!drew);
  if(npcChoice==0)npc_hello(r);
 }
 return 0;
}
static void npc_stance(float x,float y,float*a){
 if(npcEnc.phase==NPC_DRAW&&npcEnc.room==npc_room_key())
  *a=look(npcEnc.actor.x-x,npcEnc.actor.y-y);
}
static int npc_fighter_position(float*x,float*y){
 if(scene!=1||npcEnc.room!=1||(npcEnc.phase!=NPC_FIGHT&&npcEnc.phase!=NPC_DRAW))return 0;
 *x=npcEnc.actor.x;*y=npcEnc.actor.y;return 1;
}
static PulsePose npc_local_aim(float x,float y,float a,int step,float recoil,PulseRoomSolid room){
 PulsePose g=pulse_with_cover(x,y,a,step,recoil,scene,room);
 if(g.hand||(npcEnc.phase!=NPC_FIGHT&&npcEnc.phase!=NPC_DRAW)||npcEnc.room!=npc_room_key())return g;
 float dx=npcEnc.actor.x-x,dy=npcEnc.actor.y-y,d=dx*g.co+dy*g.si;
 if(d>0&&d<g.extent&&absf(dy*g.co-dx*g.si)<13.f+d*.12f){
  float k=(d>2.f?d-2.f:d*.35f)/g.extent;
  float side=(1.f-k)*pulse_shoulder(g.co,g.si);
  float tx=x+(g.mx-x)*k-g.si*side,ty=y+(g.my-y)*k+g.co*side;
  if(npc_clear(x,y,tx,ty)){g.mx=tx;g.my=ty;g.extent*=k;}
 }
 return g;
}
static PulsePose npc_room_pulse(float x,float y,float a,int step,PulseRoomSolid room){
 if(npcTraceTicks>=6&&x==npcTracePlayerX&&y==npcTracePlayerY&&a==npcTraceFacing)return npcTracePose;
 if(npcEnc.phase!=NPC_FIGHT&&npcEnc.phase!=NPC_DRAW)return pulse_room_pose(x,y,a,step,room);
 return npc_local_aim(x,y,a,step,npcTraceTicks>4?1.f:0.f,room);
}
static int npc_attack(U keys,float x,float y,float a){
 if(!(keys&(FIRE|PUNCH))||npcEnc.room!=npc_room_key()||npcEnc.phase==NPC_IDLE)return 0;
 if(npcEnc.phase==NPC_DRAW||npcEnc.phase==NPC_YIELD)return 1;
 if(npcEnc.phase!=NPC_FIGHT)return 0;
 if(p.shot>0)return 1;
 NpcRef *r=&npcEnc.actor;float dx=r->x-x,dy=r->y-y,co=cs(a),si=sn(a),f=dx*co+dy*si,l=absf(dy*co-dx*si);
 int melee=(keys&PUNCH)||!pistol_equipped();
 if(melee){
  if(f<3||l>13||d2(dx,dy)>=27*27||!npc_clear(x,y,r->x,r->y))return scene==3;
  p.shot=.34f;punchFlash=.21f;npcEnc.hp--;
 }else{
  int range=sv.gear>0?165:112,damage=sv.gear>0?2:1;
  float cooldown=sv.gear>0?.21f:.33f;U ink=GOLD;
  nvk_stats(&range,&damage,&cooldown,&ink);
  PulsePose g=scene==1?pulse_pose(2.5f):npc_local_aim(x,y,a,player_gait(),2.5f,int_solid);
  if(g.hand)return 1;
  if(f<=0||f<g.extent-.01f||f>=range||l>=13.f+f*.12f||
     !npc_clear(x,y,r->x,r->y)||!npc_clear(g.mx,g.my,r->x,r->y))return scene==3;
  if(!night_venue_apply_pulse(&range,&damage,&cooldown,&ink))return 1;
  p.shot=cooldown;npcEnc.hp-=damage;
  if(scene==1){
   p.trail=.11f;firedPulse=g;firedX=p.x;firedY=p.y;firedA=p.a;
   trailInk=ink;trailX=g.mx;trailY=g.my;trailEndX=r->x;trailEndY=r->y;particle(trailX,trailY,5,ink);
  }else{
   npcTraceTicks=7;npcTracePose=g;npcTraceEndX=r->x;npcTraceEndY=r->y;
   npcTracePlayerX=x;npcTracePlayerY=y;npcTraceFacing=a;
  }
 }
 heat(melee?26.f:24.f);npcEnc.flash=.18f;
 if(npcEnc.hp<=0){
  npcEnc.phase=NPC_DROP;npcEnc.ticks=NPC_LOOT_TICKS;heat(30);
  npcSuppressed|=keys&(FIRE|PUNCH);
  /* Outdoor street actors enter the ordinary down/respawn path. Fixed
     services and scripted passengers recover without destroying jobs. */
  if(scene==1&&r->id>=N_ACT_STREET&&r->id<N_ACT_STREET+LEN(peds)){
   Ped *q=peds+r->id-N_ACT_STREET;q->x=r->x;q->y=r->y;q->hp=0;q->life=12;q->s=0;
  }
  if(scene==1&&r->id>=N_ACT_OUTER&&r->id<N_ACT_MAX)outer_injure(r->id,3,r->x,r->y);
  tell("They dropped their cash. Walk over it to collect it.");
 }
 return 1;
}
static void npc_encounter_tick(float dt){
 if(npcEnc.phase==NPC_IDLE)return;
 if(p.dead||p.car>=0||npcEnc.room!=npc_room_key()){npc_forget_encounter();return;}
 if(scene==3){p.shot-=dt;p.hit-=dt;punchFlash-=dt;if(npcTraceTicks>0)npcTraceTicks--;}
 npcEnc.flash-=dt;npcEnc.strike-=dt;
 float x,y;npc_player_point(&x,&y);
 NpcRef*r=&npcEnc.actor;float dx=x-r->x,dy=y-r->y,d=d2(dx,dy);
 if(npcEnc.phase==NPC_DRAW){
  if(d>82*82||!npc_clear(x,y,r->x,r->y)){tell("You broke off the encounter.");npc_forget_encounter();return;}
  if(--npcEnc.ticks>0)return;
  heat(24.f);
  if(npcEnc.surrender){
   npcEnc.phase=NPC_YIELD;npcEnc.ticks=43;
   int prize=npcEnc.prize;npcEnc.prize=0;
   gain(0,prize,"They hand over their cash.");request_save();
  }else{
   npcEnc.phase=NPC_FIGHT;npcEnc.cool=.42f;
   tell("They refuse and fight back. Fight or get away.");
  }
  return;
 }
 if(npcEnc.phase==NPC_YIELD||npcEnc.phase==NPC_RECOVER){
  if(--npcEnc.ticks<=0)npc_forget_encounter();return;
 }
 if(npcEnc.phase==NPC_DROP){
  if(--npcEnc.ticks<=0){npc_forget_encounter();return;}
  if(npcEnc.ticks<NPC_LOOT_TICKS-16&&npcEnc.prize&&d<19*19&&npc_clear(x,y,r->x,r->y)){
   int prize=npcEnc.prize;npcEnc.prize=0;npcEnc.phase=NPC_RECOVER;npcEnc.ticks=90;
   gain(0,prize,"Dropped cash collected.");request_save();
  }
  return;
 }
 if(d>150*150||d2(x-npcEnc.bornX,y-npcEnc.bornY)>196*196){
  tell("You got away. No cash taken.");npc_forget_encounter();return;
 }
 if(d>1)r->a=look(dx,dy);
 if(d>20*20){
  float v=76.f*dt/sq(d),ox=r->x,oy=r->y;
  if(scene==1){
   Ped temp={.x=ox,.y=oy,.a=r->a,.hp=1};
   street_actor_move(&temp,dx*v,dy*v,7);
   r->x=temp.x;r->y=temp.y;
  }else{
   if(!int_solid(ox+dx*v,oy))r->x+=dx*v;
   if(!int_solid(r->x,oy+dy*v))r->y+=dy*v;
  }
 }
 npcEnc.cool-=dt;
 if(d<26*26&&npcEnc.cool<=0&&npc_clear(r->x,r->y,x,y)){
  npcEnc.cool=.85f;npcEnc.strike=.20f;health(8);
  if(scene==1)particle(p.x,p.y,4,PINK);
  else if(p.dead){npc_forget_encounter();int_leave();}
 }
}
static void npc_body_paint(int dx,int dy){
 if(npcEnc.phase==NPC_IDLE||npcEnc.room!=npc_room_key())return;
 const NpcRef*r=&npcEnc.actor;
 int x=(int)r->x-dx,y=(int)r->y-dy;
 if(npcEnc.phase==NPC_DROP||npcEnc.phase==NPC_RECOVER){
  line(x-7,y-3,x+7,y+3,0x815763);disc(x-7,y-4,3,0x85757b);
  if(npcEnc.prize){
   box(x+2,y-8,12,7,0x4a7547,0xc1e380);
   line(x+6,y-7,x+6,y-3,0xe6ed8e);diamond(x+7,y-19,5,GOLD);
  }
 }else{
  int fighting=npcEnc.phase==NPC_FIGHT;
  int step=fighting?(int)(sn(anim*18.f)*4.f):0;
  modelExtraPunch=fighting?npcEnc.strike:0;
  person(x,y,npcEnc.flash>0?0xffdfbc:r->coat,step,r->a,r->model,r->hue);
  modelExtraPunch=0;
  if(fighting){
   diamond(x,y-25,4,PINK);
   if(npcEnc.strike>0){
    int xx=x+(int)(cs(r->a)*11.f),yy=y-7+(int)(sn(r->a)*11.f);
    line(x,y-7,xx,yy,0xffb080);disc(xx,yy,2,0xffd4aa);
   }
  }else if(npcEnc.phase==NPC_YIELD){
   disc(x-7,y-13,2,0xe3b591);disc(x+7,y-13,2,0xe3b591);diamond(x,y-25,4,LIME);
  }else diamond(x,y-25,4,GOLD);
 }
 if(scene==1)cover_behind_buildings((int)r->x,(int)r->y);
}
static void npc_room_trace(int dx,int dy){
 if(npcTraceTicks<=0)return;
 int x=(int)npcTracePose.mx-dx,y=(int)npcTracePose.my-dy;
 line(x,y,(int)npcTraceEndX-dx,(int)npcTraceEndY-dy,GOLD);
 if(npcTraceTicks>=5)dot(x,y,0xfff2bd);
}
static void npc_panel_paint(void){
 if(npcEnc.phase!=NPC_IDLE&&npcEnc.room==npc_room_key()&&!p.dead&&menu<0&&!mapOn){
  const char*s=npcEnc.phase==NPC_DRAW?"DRAWING / NO SHOT FIRED":
    npcEnc.phase==NPC_FIGHT?"REFUSED  /  SPACE OR X TO FIGHT  /  RUN TO LEAVE":
    npcEnc.phase==NPC_DROP?"DROPPED CASH  /  WALK OVER IT":
    npcEnc.phase==NPC_YIELD?"SURRENDERED":0;
  if(s){int w=textw(s,0)+20;box(23,scene==3?85:114,w,24,DARK,GOLD);textat(32,scene==3?87:116,s,GOLD,0);}
 }
 if(!npcTalkOn||npcTalkRoom!=npc_room_key()||p.dead||menu>=0||mapOn)return;
 const int left=195,top=127,w=570;
 modal(left,top,w,277,TEAL);
 textat(left+26,top+13,"PORT VANTA / NEARBY PERSON",TEAL,0);
 textat(left+25,top+35,npcTalk.name,PAPER,textw(npcTalk.name,1)<w-50);
 static const char*labels[]={"SAY HELLO","SAY NOTHING","ROB"};
 for(int i=0;i<3;i++){
  int y=top+87+i*31;
  if(i==npcChoice)box(left+23,y,w-46,28,0x2a4950,TEAL);
  textat(left+38,y+2,labels[i],i==2?PINK:i==npcChoice?PAPER:MUTED,0);
 }
 const char *tip=npcChoice==0?"Hear one of this person's 50 greetings.":
                 npcChoice==1?"Leave without saying anything.":
                 sv.gear<0?"Requires an owned pistol.":npcSpent[npcTalk.id]?"This person won't risk another encounter this session.":"Draw on this person. They might give up cash or fight.";
 textat(left+25,top+189,tip,PAPER,0);
 textat(left+25,top+234,"UP / DOWN     E OR ENTER SELECT     ESC CLOSE",MUTED,0);
}
