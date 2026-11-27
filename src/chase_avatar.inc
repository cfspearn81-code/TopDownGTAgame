/* Local wardrobe/character surfaces for the street chase renderer. No map,
 * NPC, shot or gear state is advanced from paint, even on repeated presents. */
static void av_q(ChRig r,ChVec a,ChVec b,ChVec c,ChVec d,U col){
 ch_quad(ch_joint(r,a.x,a.y,a.z),ch_joint(r,b.x,b.y,b.z),ch_joint(r,c.x,c.y,c.z),ch_joint(r,d.x,d.y,d.z),col,1);
}
static void av_front(ChRig r,float x,float l,float rr,float z,float zz,U col){
 av_q(r,(ChVec){x,l,z},(ChVec){x,rr,z},(ChVec){x,rr,zz},(ChVec){x,l,zz},col);
}
/* Chamfered head and jacket shoulders catch the world light on a real
 * diagonal face. Backface culling only skips the invisible half of each
 * convex ring; silhouettes remain legible at oblique camera angles. */
static void av_round(ChRig r,ChVec a,ChVec b,float aw,float bw,float ad,float bd,U col){
 static const float u[8]={1,1,.56f,-.56f,-1,-1,-.56f,.56f},v[8]={-.52f,.52f,1,1,.52f,-.52f,-1,-1};
 ChVec ring[8],top[8];float dx=ch.x-r.x,dy=ch.y-r.y,eu=dx*r.co+dy*r.si,ev=dy*r.co-dx*r.si;
 for(int i=0;i<8;i++){ring[i]=ch_joint(r,a.x+ad*u[i],a.y+aw*v[i],a.z);top[i]=ch_joint(r,b.x+bd*u[i],b.y+bw*v[i],b.z);}
 for(int i=0;i<8;i++){int k=(i+1)&7;
  if((v[k]-v[i])*eu*aw-(u[k]-u[i])*ev*ad< -3.f)continue;
  U c=i==0||i==7?mix(col,PAPER,1):i==3||i==4?mix(col,0x283549,2):col;
  ch_quad(ring[i],ring[k],top[k],top[i],c,1);}
 for(int side=0;side<2;side++){ChVec q[5];for(int j=0;j<5;j++)q[j]=top[(side*4+j)&7];ch_face(q,5,mix(col,PAPER,1),1);}
}
static int av_visible(float x,float y,float z){
 ChVec m=ch_local((ChVec){x,y,z});return m.z>6&&m.z<690&&absf(m.x)<m.z*1.31f+14;
}
static float av_recoil(float trail){return trail>.085f?.66f:trail>.050f?.28f:0;}
static ChVec av_grip(float strike,float guarded,float step){
 return guarded?(ChVec){1.32f,2.92f,7.64f+step*.038f}:
                 (ChVec){4.42f-strike,1.61f,10.55f+strike*.25f};
}
static ChVec av_muzzle(ChVec wrist,int guarded,float recoil){
 return guarded?(ChVec){wrist.x+.77f,wrist.y+.16f,wrist.z-2.26f}:
                 (ChVec){wrist.x+3.08f,wrist.y,wrist.z+.74f+recoil*.18f};
}
static void av_weapon(ChRig r,ChVec hand,int guarded,float recoil){
 ChVec tip=av_muzzle(hand,guarded,recoil);
 if(guarded){
  ch_link(r,tip,(ChVec){hand.x+.58f,hand.y,hand.z+.32f},.26f,.28f,.32f,.34f,0x475662);
  ch_link(r,(ChVec){hand.x-.43f,hand.y,hand.z-.07f},(ChVec){hand.x+.58f,hand.y,hand.z+.40f},.26f,.27f,.37f,.32f,0x27323d);
 }else{
  ch_link(r,(ChVec){hand.x+.15f,hand.y,hand.z-.70f},(ChVec){hand.x+.52f,hand.y,hand.z+.55f},.24f,.26f,.32f,.34f,0x2b3642);
  ch_link(r,(ChVec){(tip.x+hand.x)*.5f,hand.y,tip.z-.49f},(ChVec){(tip.x+hand.x)*.5f,hand.y,tip.z},.30f,.30f,(tip.x-hand.x)*.5f,(tip.x-hand.x)*.5f,0x667981);
  av_front(r,tip.x+.036f,hand.y-.21f,hand.y+.21f,tip.z-.38f,tip.z-.12f,0x21303a);
 }
}
static void av_person(const StreetPerson*q,const Look*look,int phase,float strike,int armed,int guarded,float gait){
 if(!av_visible(q->x,q->y,8))return;
 ChRig r={q->x,q->y,cs(q->a),sn(q->a)};
 int hue=(q->hue&255)%5,hero=look!=0,role=q->role,uniform=role==MODEL_OFFICER||role==MODEL_DISPATCH;
 const unsigned char*part=hero?look->part:defaultLook.part;
 int style=hero?part[HAIR]:(hue==0?5:hue==1?3:hue==2?1:hue==3?8:9);
 int face=hero?part[DETAIL]:(role==MODEL_STAFF?2:0),top=hero?part[TOP]:uniform?4:role==MODEL_WORKER?5:hue%5;
 static const U npcSkin[]={0xdeaf91,0xa96d54,0x875749,0xe3c2a7,0xbb8271};
 static const U npcHair[]={0x2a2632,0x41303a,0x393332,0x695048,0x392d35};
 U skin=hero?charSkin[part[SKIN]]:npcSkin[hue],hair=hero?charHair[part[HAIR_TONE]]:npcHair[hue],cloth=q->coat;
 U pants=hero?charLeg[part[LEG_TONE]]:uniform?0x334764:role==MODEL_RIVAL?0x56405e:mix(cloth,0x2f3c50,6);
 U shoe=hero?charShoe[part[SHOES]]:0x293747,trim=mix(cloth,0xffe5bc,2),seam=mix(cloth,0x263544,3);
 cloth=headlamps_ground(cloth,(int)r.x,(int)r.y);skin=headlamps_ground(skin,(int)r.x,(int)r.y);
 float step=gait*.48f,bob=absf(step)*.08f,w=hero?1.88f+part[BUILD]*.30f:2.00f+(hue&1)*.22f,head=12.04f+bob;
 float near=d2(r.x-ch.x,r.y-ch.y)<210.f*210.f;
 ch_figure_shadow(r.x,r.y);
 if(!near&&d2(r.x-ch.x,r.y-ch.y)>340.f*340.f){
  for(int s=-1;s<=1;s+=2)ch_link(r,(ChVec){step*s,s*.89f,.4f},(ChVec){0,s*.80f,6.6f},.62f,.70f,.66f,.7f,pants);
  ch_link(r,(ChVec){0,0,6},(ChVec){0,0,11.3f},w*.76f,w,1.14f,1.23f,cloth);
  ch_link(r,(ChVec){0,0,11.45f},(ChVec){0,0,14.1f},.70f,1.13f,.84f,1.06f,skin);
  ch_link(r,(ChVec){0,0,13.4f},(ChVec){0,0,14.85f},1.15f,.96f,1.05f,.86f,uniform?0x3c5377:hair);return;
 }
 for(int side=-1;side<=1;side+=2){
  float stride=step*side,lift=stride>0?stride*.55f:0,lat=side*.96f;
  ChVec hip={-.14f,lat,6.36f+bob},knee={stride*.53f+.16f,lat,3.42f+lift*.40f},heel={stride-.22f,lat,.53f+lift};
  U leg=side==1?pants:mix(pants,0x263449,1);
  ch_link(r,heel,knee,.49f,.62f,.53f,.62f,hero&&part[LEGS]==2?skin:leg);
  ch_link(r,knee,hip,.62f,.78f,.61f,.75f,leg);
  if(near&&hero){
   if(part[LEGS]==1||part[LEGS]==4){ChVec pocket={hip.x+stride*.18f,lat+side*.42f,4.58f+bob};
    ch_link(r,pocket,(ChVec){pocket.x+.07f,pocket.y,5.36f+bob},.39f,.44f,.49f,.55f,mix(leg,0x788388,2));}
   else if(part[LEGS]==3)ch_link(r,heel,(ChVec){heel.x+.10f,lat,heel.z+.59f},.51f,.54f,.54f,.57f,mix(leg,0x202c38,3));
  }
  ch_link(r,(ChVec){stride+.20f,lat,.12f+lift},(ChVec){stride-.04f,lat,.93f+lift},.66f,.55f,.92f,.66f,shoe);
  if(near&&hero&&part[SHOES]!=0)ch_link(r,(ChVec){stride+.22f,lat,.09f+lift},(ChVec){stride+.17f,lat,.29f+lift},.67f,.66f,.95f,.87f,mix(shoe,PAPER,2));
 }
 av_round(r,(ChVec){-.15f,0,6.11f+bob},(ChVec){0,0,10.55f+bob},w*.75f,w,1.04f,1.30f,cloth);
 av_round(r,(ChVec){0,0,10.52f+bob},(ChVec){.03f,0,11.43f+bob},w,w*.72f,1.30f,.92f,cloth);
 if(near){
  av_round(r,(ChVec){-.15f,0,6.15f+bob},(ChVec){-.14f,0,6.49f+bob},w*.77f,w*.78f,1.07f,1.09f,seam);
  /* Yoke and collar wrap the rear shoulder line. This is what the
   * player actually sees in a trailing view; not just front chest badges. */
  av_q(r,(ChVec){-1.23f,-w*.72f,10.56f+bob},(ChVec){-1.40f,0,10.31f+bob},(ChVec){-1.34f,0,9.93f+bob},(ChVec){-1.21f,-w*.70f,10.26f+bob},seam);
  av_q(r,(ChVec){-1.40f,0,10.31f+bob},(ChVec){-1.23f,w*.72f,10.56f+bob},(ChVec){-1.21f,w*.70f,10.26f+bob},(ChVec){-1.34f,0,9.93f+bob},seam);
  av_round(r,(ChVec){-.02f,0,11.25f+bob},(ChVec){.02f,0,11.57f+bob},.89f,.84f,.76f,.73f,top==5?trim:seam);
  if(top==3){ /* Hood drapes over the shoulder blades, not over the face. */
   av_round(r,(ChVec){-1.06f,0,8.96f+bob},(ChVec){-.84f,0,11.41f+bob},.58f,1.38f,.27f,.58f,seam);
  }else if(top==4||role==MODEL_WORKER){av_front(r,-1.29f,-w*.61f,w*.61f,9.81f+bob,10.23f+bob,role==MODEL_WORKER?0xe6c681:trim);}
  if(uniform||role==MODEL_STAFF||top==5)av_front(r,1.31f,-1.12f,-.48f,9.70f+bob,10.30f+bob,uniform?0xe3c878:PAPER);
  if(top==2||top==4||top==5)av_front(r,1.31f,-.10f,.10f,7.3f+bob,10.51f+bob,seam);
 }
 float rec=hero?av_recoil(p.trail):0;ChVec grip=av_grip(rec,guarded,step);
 for(int side=-1;side<=1;side+=2){
  float swing=-step*side;ChVec shoulder={.06f,side*(w-.07f),10.62f+bob},
   elbow={.31f+swing*.48f,side*(w+.47f),8.55f+bob},hand={swing*.88f+.43f,side*(w+.28f),6.81f+bob};
  if(phase==NPC_YIELD){elbow=(ChVec){.18f,side*(w+1.1f),11.9f};hand=(ChVec){.72f,side*(w+.89f),14.0f};}
  if(phase==NPC_FIGHT){elbow.x=1.08f;hand=(ChVec){2.34f,side*1.43f,10.68f+bob};}
  if(armed&&(!guarded||side==1)){
   elbow=(ChVec){guarded?.20f:1.18f,side*(w+.66f),guarded?8.74f:9.28f+bob};
   hand=(ChVec){grip.x+(side<0?.56f:0),grip.y-(side<0?.56f:0),grip.z-(side<0?.17f:0)};
  }
  if(strike>0&&side==(armed?-1:1)){float k=strike/.21f;if(k>1)k=1;elbow=(ChVec){1.81f+1.24f*k,side*(w-.2f),9.81f};hand=(ChVec){3.33f+2.42f*k,side*.81f,10.43f};}
  ch_link(r,elbow,shoulder,.54f,.72f,.56f,.69f,top==1?skin:cloth);
  ch_link(r,hand,elbow,.39f,.53f,.37f,.51f,top<=1||top==5?skin:seam);
  ch_link(r,(ChVec){hand.x,hand.y,hand.z-.32f},(ChVec){hand.x+.11f,hand.y,hand.z+.40f},.40f,.39f,.39f,.38f,skin);
 }
 if(armed)av_weapon(r,grip,guarded,rec);
 ch_link(r,(ChVec){.02f,0,11.31f+bob},(ChVec){.08f,0,head+.32f},.66f,.70f,.62f,.68f,skin);
 av_round(r,(ChVec){.23f,0,head},(ChVec){.08f,0,head+1.91f},.89f,1.32f,.91f,1.12f,skin);
 U crown=uniform?0x435b7a:style==0?mix(skin,hair,2):hair;
 av_round(r,(ChVec){.04f,0,head+1.71f},(ChVec){-.04f,0,head+(style==5?3.36f:style==0?2.37f:2.68f)},style==5?1.48f:1.34f,style==5?1.59f:1.07f,1.15f,style==5?1.45f:.91f,crown);
 if(!near)return;
 if(!uniform&&style!=0&&style!=6){ /* Hairline slopes below the crown at the nape; cheeks stay visible. */
  av_round(r,(ChVec){-.60f,0,head+(style==3?1.05f:.74f)},(ChVec){-.51f,0,head+2.04f},
           style==3?.92f:1.08f,1.26f,.53f,.68f,mix(hair,skin,style==3?2:0));
 }
 if(uniform){av_q(r,(ChVec){.82f,-1.39f,head+1.88f},(ChVec){2.01f,-1.00f,head+1.65f},(ChVec){2.01f,1.00f,head+1.65f},(ChVec){.82f,1.39f,head+1.88f},0x384c68);}
 else if(style>=7){float len=style==7?.66f:style==8?1.75f:2.15f,rad=style==8?.50f:1.21f;
  av_round(r,(ChVec){-1.08f,0,head+.75f-len},(ChVec){-.85f,0,head+1.84f},rad*.76f,rad,.42f,.51f,hair);
 }else if(style==6){av_round(r,(ChVec){-.08f,0,head+2.41f},(ChVec){-.25f,0,head+3.23f},.43f,.32f,.97f,.86f,hair);}
 else if(style==2||style==4){av_round(r,(ChVec){.20f,-.32f,head+2.26f},(ChVec){.16f,-.35f,head+2.95f},.95f,.77f,.88f,.72f,hair);}
 if(face==1||face==2){av_front(r,1.25f,-.99f,.99f,head+.97f,head+1.39f,face==2?0x283444:0x4c565b);}
 else {for(int side=-1;side<=1;side+=2)av_front(r,1.22f,side*.40f,side*.65f,head+1.08f,head+1.28f,0x3b3235);}
 if(face==3||face==4||face==5)av_round(r,(ChVec){.29f,0,head-.06f},(ChVec){.23f,0,head+(face==5?.99f:.55f)},.82f,1.12f,.88f,1.05f,face==5?0x364952:mix(hair,skin,face==3?4:1));
 if(face==6)for(int side=-1;side<=1;side+=2)ch_link(r,(ChVec){.03f,side*1.26f,head+.61f},(ChVec){.03f,side*1.26f,head+.87f},.15f,.15f,.16f,.16f,0xddbd7d);
}
static void ch_character(const StreetPerson*q){av_person(q,0,NPC_IDLE,0,0,0,q->step);}
static void ch_player_character_at(float x,float y,float a){
 const Look*l=look_valid(&sv.look)?&sv.look:&defaultLook;
 StreetPerson hero={x,y,a,0,MODEL_PLAYER,3,p.hit>0&&((int)(anim*20)&1)?PINK:charTop[l->part[TOP_TONE]]};
 int armed=pistol_equipped(),guarded=armed&&pulse_visible_pose().hand!=0;
 av_person(&hero,l,NPC_IDLE,punchFlash,armed,guarded,walking?5.f*sn(footcycle):0);
}
static void ch_player_character(void){if(!p.dead&&p.car<0)ch_player_character_at(p.x,p.y,p.a);}
static void av_downed(float x,float y,float a,U coat){
 if(!av_visible(x,y,1.5f))return;ChRig r={x,y,cs(a),sn(a)};
 for(int s=-1;s<=1;s+=2){
  ch_link(r,(ChVec){3.23f,s*1.11f,.48f},(ChVec){5.53f,s*1.6f,1.08f},.69f,.64f,2.05f,2.0f,0x495364);
  ch_link(r,(ChVec){6.35f,s*1.68f,.22f},(ChVec){6.44f,s*1.68f,1.13f},.69f,.65f,1.09f,.97f,0x283542);
 }
 av_round(r,(ChVec){-.20f,0,.14f},(ChVec){-1.0f,0,2.07f},1.57f,1.9f,3,2.72f,coat);
 av_round(r,(ChVec){-4.95f,.12f,.2f},(ChVec){-4.90f,.22f,2.02f},1.23f,1.13f,1.38f,1.19f,0x9d776a);
}
static void av_mark(float x,float y,float z,U c){
 float u=1.17f*ch.si,v=-1.17f*ch.co;
 ch_quad((ChVec){x,y,z+1.66f},(ChVec){x-u,y-v,z},(ChVec){x,y,z-1.66f},(ChVec){x+u,y+v,z},c,0);
}
/* Transient street FX are moving particles with velocities, not square
 * road signs. Keep their real coordinates/lifetime; only taper the projected
 * streak from its narrow tail to the bright tip. No new RNG calls. */
static void ch_field_sparks(void){
 for(int i=0;i<LEN(fx);i++){const Particle*f=fx+i;if(f->t<=0||!av_visible(f->x,f->y,3.5f))continue;
  float k=f->t<.17f?f->t/.17f:1,span=.012f+.013f*k;
  float width=.10f+.18f*k,x=f->x,y=f->y,z=.78f+f->t*5.0f,dx=ch.si*width,dy=-ch.co*width;
  ChVec a={x-dx,y-dy,z},b={x+dx,y+dy,z+.13f},c={x-f->vx*span,y-f->vy*span,z-.37f*k};
  ChVec q[]={a,b,c};ch_face(q,3,mix(0x4d5152,f->c,cl((int)(k*6)+2,2,8)),1);
 }
}
static void av_flash(ChRig r,ChVec tip,float t){
 float k=t>.085f?1.f:.48f;ChVec a=ch_joint(r,tip.x,tip.y,tip.z-.20f),b=ch_joint(r,tip.x+1.80f*k,tip.y,tip.z-.20f);
 float dx=ch.si*.58f*k,dy=-ch.co*.58f*k;
 ChVec q[]={a,(ChVec){a.x+dx,a.y+dy,a.z+.46f*k},b,(ChVec){a.x-dx,a.y-dy,a.z-.48f*k}};
 ch_face(q,4,0xfff2bd,0);
 ChVec core[]={ch_joint(r,tip.x+.42f*k,tip.y,tip.z-.54f*k),ch_joint(r,tip.x+1.28f*k,tip.y,tip.z+.18f*k),ch_joint(r,tip.x+.20f*k,tip.y,tip.z+.48f*k)};
 ch_face(core,3,0xfff2bd,0);
}
static void ch_street_encounter(void){
 if(scene!=1)return;
 for(int i=0;i<LEN(peds);i++){const Ped*q=peds+i;if(q->hp<=0&&q->life>0&&!npc_replaces(i))av_downed(q->x,q->y,q->a,q->role==MODEL_OFFICER?0x587296:q->role==MODEL_RIVAL?0x9e6283:i&1?0x739691:0xae806f);}
 for(int i=0;i<OUT_WALK_COUNT;i++)if(outBody[i].damage==3&&outBody[i].down>0&&!npc_replaces(i+N_ACT_OUTER))av_downed(outBody[i].x,outBody[i].y,outWalkers[i].a,0x688a88);
 if(npcEnc.phase!=NPC_IDLE&&npcEnc.room==npc_room_key()){
  const NpcRef*r=&npcEnc.actor;int ph=npcEnc.phase;
  if(ph==NPC_DROP||ph==NPC_RECOVER){av_downed(r->x,r->y,r->a,r->coat);
   if(npcEnc.prize){ChRig box={r->x+3.3f,r->y-1.62f,1,0};
    ch_link(box,(ChVec){0,0,.25f},(ChVec){0,0,1.46f},1.55f,1.55f,2.53f,2.53f,0x76a06b);
    ch_link(box,(ChVec){0,0,1.46f},(ChVec){0,0,1.65f},1.52f,1.52f,.48f,.48f,0xd9d295);av_mark(box.x,box.y,7.f,GOLD);}
  }else{
   StreetPerson q={r->x,r->y,r->a,0,r->model,r->hue,npcEnc.flash>0?0xffdfbc:r->coat};
   av_person(&q,0,ph,npcEnc.strike,0,0,ph==NPC_FIGHT?sn(anim*18.f)*4:0);
   av_mark(r->x,r->y,18.9f,ph==NPC_FIGHT?PINK:ph==NPC_YIELD?LIME:GOLD);
  }
 }
 if(p.trail>0){
  /* The 2-D PulsePose contains a fake vertical projection. A 3-D flash
   * starts at the posed barrel; hit/wall end remains the recorded result. */
  ChRig shot={firedX,firedY,cs(firedA),sn(firedA)};float kick=av_recoil(.11f);
  ChVec nozzle=av_muzzle(av_grip(kick,0,0),0,kick);
  ChVec a=ch_joint(shot,nozzle.x,nozzle.y,nozzle.z-.20f),b={trailEndX,trailEndY,9.f};
  float kx=b.x-a.x,ky=b.y-a.y,d=sq(d2(kx,ky));if(d>.5f){
   float u=.17f*ky/d,v=-.17f*kx/d;
   ch_quad((ChVec){a.x-u,a.y-v,a.z},(ChVec){b.x-u,b.y-v,b.z},(ChVec){b.x+u,b.y+v,b.z+.13f},(ChVec){a.x+u,a.y+v,a.z+.13f},trailInk,0);
  }
  if(p.trail>.064f)av_flash(shot,nozzle,p.trail);
 }
}
