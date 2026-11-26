/* Lantern's crew is scenery driven by the real batch and stock state. These
   small, game-only poses never add collision, run production, or alter saves. */
enum { GROW_OFF, GROW_IDLE, GROW_PLANT, GROW_WATER, GROW_FEED, GROW_HARVEST, GROW_PACK, GROW_RETURN };
enum { GROW_BED_COUNT=4, GROW_POTS_PER_BED=5, GROW_POT_COUNT=GROW_BED_COUNT*GROW_POTS_PER_BED };
typedef struct {int x,y;} GrowPot;
typedef struct {GrowPot pot[GROW_POT_COUNT];int count,product;} GrowBatch;
typedef struct {int x,y,phase,walk,pot;float angle;} GrowPose;
/* Match the fixture layout: fill columns across the four beds before
   opening the next column. This is the only live crop position lookup. */
static GrowBatch grow_batch(const Business*b){
 GrowBatch v={.product=cl(b->product,0,1)};
 int bed=0;
 for(int j=0;j<interior_nprop&&bed<GROW_BED_COUNT;j++)if(interior_props[j].type==INT_GROWBED){
  const InteriorProp*f=interior_props+j;
  for(int i=0;i<GROW_POTS_PER_BED;i++)v.pot[i*GROW_BED_COUNT+bed]=(GrowPot){f->x+25+i*37,f->y+f->h/2+3};
  bed++;
 }
 if(bed==GROW_BED_COUNT&&b->own&&b->timer>0.f)v.count=cl(batch_units(v.product,b->tier),0,GROW_POT_COUNT);
 return v;
}
static int grow_work_pot(const GrowBatch*v,int phase){
 if(v->count<=0)return -1;
 /* Keep working stances on the big center aisle, beside the two upper
    beds. Pick outer occupied pots as more columns open with upgrades. */
 int id=phase==GROW_PLANT||phase==GROW_WATER?0:1;
 if(id>=v->count)id=0;
 if(phase==GROW_WATER||phase==GROW_FEED)while(id+GROW_BED_COUNT<v->count)id+=GROW_BED_COUNT;
 return id;
}
static int grow_work_x(const GrowBatch*v,int id){return v->pot[id].x-5;}
static int grow_case_count(int units){
 if(units<=0)return 0;
 if(units<20)return 1;
 if(units<=50)return 2;
 return 3+(units-51)/25;
}
static GrowPose grow_operator_walk(GrowPose pose,int ax,int ay,int bx,int by,float progress){
 pose.x=ax+(int)((bx-ax)*progress);pose.y=ay+(int)((by-ay)*progress);
 pose.walk=1;pose.pot=-1;pose.angle=look((float)(bx-ax),(float)(by-ay));return pose;
}
static GrowPose grow_operator_pose(const Business*b,const GrowBatch*v,float progress,float packing,float motion){
 GrowPose pose={556,279,GROW_IDLE,0,-1,1.570796f};
 if(!b->own||!b->crew){pose.phase=GROW_OFF;return pose;}
 if(packing>GROW_RETURN_SECONDS){
  /* Carry along the clear aisle east of the console, above the upper bed,
     then put the carton down beside the shelf. Never walk through a bed. */
  const float frame=(GROW_PACK_SECONDS+GROW_RETURN_SECONDS-packing)*60.f;
  const float walk=(frame-GROW_PICKUP_TICKS)*FOOT_PACE;
  int pickup=grow_work_x(v,1),row=v->pot[1].y+42; /* Match the harvest pose at pickup. */
  pose.phase=GROW_PACK;
  if(frame<GROW_PICKUP_TICKS){pose.x=pickup;pose.y=row;pose.angle=-1.570796f;}
  else if(walk<48.f)return grow_operator_walk(pose,pickup,row,557,row,walk/48.f);
  else if(walk<128.f)return grow_operator_walk(pose,557,row,557,170,(walk-48.f)/80.f);
  else if(walk<GROW_CARRY_WALK_TICKS)return grow_operator_walk(pose,557,170,646,170,(walk-128.f)/64.f);
  else{pose.x=646;pose.y=170;pose.angle=0;}
  return pose;
 }
 if(packing>0.f){
  float walk=(GROW_RETURN_SECONDS-packing)*60.f*FOOT_PACE;
  pose.phase=GROW_RETURN;
  if(walk<36.f)return grow_operator_walk(pose,646,170,557,170,walk/36.f);
  if(walk<73.f)return grow_operator_walk(pose,557,170,557,259,(walk-36.f)/37.f);
  if(v->count>0)return grow_operator_walk(pose,557,259,grow_work_x(v,0),259,(walk-73.f)/107.f);
  if(walk<93.f)return grow_operator_walk(pose,557,259,556,279,(walk-73.f)/20.f);
  return pose;
 }
 if(v->count<=0)return pose;
 const float begin[]={0.f,.10f,.45f,.85f},walk[]={.027f,.008f,.030f,.005f};
 int step=progress<begin[1]?0:progress<begin[2]?1:progress<begin[3]?2:3;
 pose.phase=GROW_PLANT+step;
 pose.pot=grow_work_pot(v,pose.phase);int to=grow_work_x(v,pose.pot);
 float when=(progress-begin[step])*FOOT_PACE;
 if(when<walk[step]){
  int from=step?grow_work_x(v,grow_work_pot(v,pose.phase-1)):556;
  if(from!=to)return grow_operator_walk(pose,from,step?259:279,to,259,when/walk[step]);
 }
 pose.angle=-1.570796f;pose.y=v->pot[pose.pot].y+42;
 pose.x=to+(int)(sn(motion*3.f)*2.f);
 return pose;
}
static void grow_carton(int x,int y,int w,int h,U c){
 rect(x+2,y+2,w,h,0x35413d);box(x,y,w,h,c,0x584c3f);
 rect(x+1,y+1,w-2,2,mix(c,0xe6d8a4,3));rect(x+w/2-1,y+1,3,h-2,0xe1c79c);
 rect(x+3,y+4,min(6,w-6),3,0xf0ebce);rect(x+4,y+5,min(4,w-7),1,0x7a8068);
}
static void grow_storage_art(int dx,int dy){
 int units=warehouse_units(0),cases=grow_case_count(units);
 int x=GROW_RACK_X-dx,y=GROW_RACK_Y-dy;
 /* Art stays within the old right-hand locker footprint; overflow still has
    its exact count on the sign, so huge saves cannot blow up the render. */
 rect(x+2,y+5,114,20,0x2a3e45);box(x,y,114,22,0x4c635f,0x81998d);
 for(int j=3;j<21;j+=6)rect(x+3,y+j,108,4,j&1?0x758170:0x687962);
 int staged=sv.b[0].crew>0&&grow_pack_left>GROW_RETURN_SECONDS+.27f?min(cases,grow_case_count(grow_stock_before_pack)):cases;
 for(int i=0;i<min(staged,3);i++)grow_carton(x+8+i*34,y+4,27,14,i&1?0xaa9469:0xb79d72);
 char label[50];char*t=number(label,units);t=append(t," STOCK / ");t=number(t,cases);append(t,cases==1?" BOX":" BOXES");
 int w=textw(label,0);int cy=y-23,cx=x+57;box(cx-w/2-5,cy,w+10,20,0x293c41,0x70867b);
 center(cx,cy,label,0xdcdcb2,0);
}
static void grow_handline(int x,int y,int handx,int handy,U sleeve){
 line(x,y,handx,handy,sleeve);line(x+1,y,handx+1,handy,sleeve);disc(handx,handy,1,0xe1ac84);
}
static void grow_operator_art(const GrowBatch*v,int dx,int dy){
 const Business*b=sv.b;GrowPose a=grow_operator_pose(b,v,growth_fraction(0),grow_pack_left,anim);
 if(a.phase==GROW_OFF||npc_replaces(N_ACT_GROW))return;
 static const char*status[]={"","OPERATOR / ON STANDBY","OPERATOR / PLANTING","OPERATOR / WATERING",
  "OPERATOR / FERTILIZING","OPERATOR / HARVESTING","OPERATOR / PACKING","OPERATOR / RETURNING"};
 const U uniform=0xefad66, sleeve=0xc49263;
 float px=a.x,py=a.y,angle=a.angle;
 int step=a.walk?(int)(sn(anim*(19.f*FOOT_PACE))*5):0;
 int held=npc_present_pose(N_ACT_GROW,&px,&py,&angle,&step);
 int x=(int)px-dx,y=(int)py-dy;
 int hands=(int)(2.f*sn(anim*8.f));
 float drop= grow_pack_left-GROW_RETURN_SECONDS;
 person(x,y,uniform,step,angle,MODEL_WORKER,0);
 if(held)return;
 if(a.phase==GROW_PACK){
  int carryx=x+(int)(cs(a.angle)*8.f)-8,carryy=y+(int)(sn(a.angle)*5.f)-7;
  if(!a.walk&&a.angle==0.f){
   int reach=cl((int)((.54f-drop)*54.f),0,14);
   carryx+=reach;carryy-=reach/3;
  }
  if(grow_pack_left>GROW_RETURN_SECONDS+.27f){
   grow_handline(x+person_offset(-5),y+person_offset(-7),carryx+1,carryy+4,sleeve);
   grow_handline(x+person_offset(5),y+person_offset(-7),carryx+14,carryy+4,sleeve);
   grow_carton(carryx,carryy,16,10,0xd5ae70);
  }
 }else if(!a.walk&&a.pot>=0){
  const GrowPot*p=v->pot+a.pot;
  int cx=p->x-dx,cy=p->y-dy,hx=cx-6,hy=y-23+hands;
  grow_handline(x+person_offset(6),y+person_offset(-6),hx,hy,sleeve);
  if(a.phase==GROW_PLANT){
   box(x-14,y-17,11,8,0xbf976e,0x5e6052);disc(x-10,y-14,2,0x867e59);
   disc(hx,hy-3,2,0xebcc8b);dot(cx,cy+2,0xebcc8b);
   if(!fastGfx)dot(cx-2,cy+3+((int)(anim*6.f)%3),0xd1cc93);
  }else if(a.phase==GROW_WATER){
   /* The spout and its projected droplets stay over this pot, even when
      the next column is locked. The last drop lands on the soil. */
   int tip=hy-7;
   box(hx-3,hy-4,9,7,0x68afb5,0x324d59);
   line(hx+3,hy-3,cx,tip,0x86bec5);line(hx+4,hy-3,cx+1,tip,0x86bec5);
   dot(cx,cy+2,0xa9e4df);
   if(!fastGfx){int n=(int)(anim*14.f)%3;
    for(int j=0;j<3;j++)dot(cx+(j&1),cy+3+(tip-cy-3)*(j*3+n)/9,0xa9e4df);
   }
  }else if(a.phase==GROW_FEED){
   box(hx-4,hy-5,9,9,0xd6c58f,0x687864);rect(hx-2,hy-2,5,2,0x6b9d7d);
   dot(cx,cy+2,0xf5d98b);
   if(!fastGfx)for(int j=0;j<3;j++)dot(cx-2+j*2,cy+3+((j+(int)(anim*8.f))%4),0xf5d98b);
  }else{
   int hx2=cx+6,hy2=y-24-hands;
   grow_handline(x+person_offset(-5),y+person_offset(-7),hx2,hy2,sleeve);ns_oval(hx2,hy2-2,4,3,0x9fb772);
   line(hx,hy-2,cx,cy+2,0xd1d7c3);line(hx,hy-2,cx+2,cy+4,0xd1d7c3);
   ns_oval(cx,cy+2,3,3,0xa8be79);
  }
 }
 box(370-dx,110-dy,220,22,0x263b43,0x817d72);
 center(480-dx,110-dy,status[a.phase],0xf2c992,0);
}
