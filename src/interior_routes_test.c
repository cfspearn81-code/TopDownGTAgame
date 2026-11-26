/* Deterministic native indoor playthrough. The street-side setup selects an
   address; actual USE, cardinal movement and USE again cover the entire visit.
   Compile: cc -std=c11 -O2 src/interior_routes_test.c -o build/room_routes
   Optional argument: existing capture directory, with 82 PPMs and rooms.tsv. */
#define DC_TEST
#include "game.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Goal { EXIT_G, DESK_G, FEATURE_G, GATE_G, CHIEF_G, CELLS_G, STATION_G, PLAQUE_G };
static const char *goals[]={"exit","counter","signature fixture","staff gate","chief","custody","shop station","visitor plaque"};
static const char *room_name;
static int g_kind, doors_seen, rears_seen, walkins_seen, route_count, steps_taken;
static FILE *manifest;
static const char *outdir;

static void fail(const char *why){
 fprintf(stderr,"FAIL [%s] %s (scene %d, rear %d, menu %d, player %.2f/%.2f)\n",room_name?room_name:"city",why,scene,interior_subroom,menu,
 (double)(scene==4?wi_x:interior_x),(double)(scene==4?wi_y:interior_y));exit(1);
}
static void ck(int ok,const char *why){if(!ok)fail(why);}
static void press_key(U key){tick(key);tick(0);}

static int matches(enum Goal g){
 if(scene==4){
#ifdef DC_STREET_ADDRESS_PLAQUES
  if(g==PLAQUE_G)return !wi_at_door()&&!wi_at_station()&&wi_at_plaque();
#endif
  return g==EXIT_G?wi_at_door():wi_at_station()&&!wi_at_door();
 }
 int hasgate=g_kind==4||legal_index_for_kind(g_kind)>=0;
 int atdoor=int_at_door(),atgate=hasgate&&int_at_staff_gate();
 if(g==EXIT_G)return atdoor;
 if(atdoor)return 0;
 if(g==GATE_G)return atgate;
 if(atgate)return 0;
 if(g==CHIEF_G)return int_at_chief();
 if(g==CELLS_G)return !int_at_chief()&&int_at_custody();
 if(g_kind==15&&(int_at_chief()||int_at_custody()))return 0;
 if(g==FEATURE_G)return int_feature_near(g_kind)&&!int_at_desk();
 return g==DESK_G&&int_at_desk();
}
/* A graph vertex is one actual cardinal walking tick from its neighbour.
   Generate float coordinates by repeated addition with the same step as the
   input code. A plan never moves the actor: restore after proximity probes,
   then replay every genuine key tick to reach and validate the destination. */
/* Capacity for cardinal walking steps down to 1.5 pixels per tick. */
enum{NX=W*2/3+2,NY=H*2/3+2,NODES=NX*NY};
static int qnode[NODES];
static signed char came[NODES]; /* -1 unvisited; -2 obstructed; 0 spawn; 1..4 */
static unsigned char rec[NODES];
static float pxs[NX],pys[NY];
static int axis(float *a,int cap,float start,float size,int end,int *origin){
 int i=(int)(start/size);ck(i>=0&&i<cap,"invalid entry coordinate");*origin=i;a[i]=start;
 for(int j=i;j>0;j--)a[j-1]=a[j]-size;
 for(int j=i+1;j<cap;j++){a[j]=a[j-1]+size;if(a[j]>end)return j;}
 fail("test grid exceeded framebuffer");return 0;
}
static void go(enum Goal g){
 int shop=scene==4;float *cx=shop?&wi_x:&interior_x,*cy=shop?&wi_y:&interior_y;
 float sx=*cx,sy=*cy,v=shop?WALKIN_WALK_STEP:NAMED_WALK_STEP;int ix,iy;
 int ww=axis(pxs,NX,sx,v,W,&ix),hh=axis(pys,NY,sy,v,H,&iy),start=iy*ww+ix;
 int delta[]={1,ww,-1,-ww};U key[]={RIGHT,DOWN,LEFT,UP};
 memset(came,-1,(size_t)(ww*hh));came[start]=0;qnode[0]=start;
 int read=0,n=1,found=-1;
 ck(!(shop?wi_solid(sx,sy):int_solid(sx,sy)),"actor spawned inside a solid!");
 while(read<n){
  int id=qnode[read++],x=id%ww,y=id/ww;
  *cx=pxs[x];*cy=pys[y];
  if(matches(g)){found=id;break;}
  for(int d=0;d<4;d++){
   if((d==0&&x+1==ww)||(d==1&&y+1==hh)||(d==2&&x==0)||(d==3&&y==0))continue;
   int nid=id+delta[d];if(came[nid]!=-1)continue;
   float nx=pxs[nid%ww],ny=pys[nid/ww];
   if(shop?wi_solid(nx,ny):int_solid(nx,ny)){came[nid]=-2;continue;}
   came[nid]=(signed char)(d+1);qnode[n++]=nid;
  }
 }
 *cx=sx;*cy=sy;
 if(found<0){fprintf(stderr,"No path to %s; %d reachable nodes examined.\n",goals[g],read);fail("blocked interaction");}
 int count=0;for(int cur=found;cur!=start;){int d=came[cur]-1;rec[count++]=(unsigned char)d;cur-=delta[d];}
 for(int i=count;i>0;i--)tick(key[rec[i-1]]);
 tick(0);steps_taken+=count;route_count++;
 if(!matches(g)){fprintf(stderr,"replay did not arrive at %s after %d key ticks\n",goals[g],count);fail("physics / route mismatch");}
}

static void frame(const char *group,int id,const char *title,const char *subtitle){
 if(!manifest)return;
 char fn[800],leaf[64];snprintf(leaf,sizeof leaf,"%s-%02d.ppm",group,id);
 int n=snprintf(fn,sizeof fn,"%s/%s",outdir,leaf);ck(n>0&&(size_t)n<sizeof fn,"capture path too long");
 /* Let ordinary roof transitions finish. No camera override or art reconstruction. */
 for(int j=0;j<26;j++)tick(0);paint();
 FILE*f=fopen(fn,"wb");ck(f!=NULL,"cannot write native frame");fprintf(f,"P6\n%d %d\n255\n",W,H);
 unsigned char row[3*W];
 for(int y=0;y<H;y++){for(int x=0;x<W;x++){U c=pixels[y*W+x];row[3*x]=(unsigned char)(c>>16);row[3*x+1]=(unsigned char)(c>>8);row[3*x+2]=(unsigned char)c;}
 ck(fwrite(row,1,sizeof row,f)==sizeof row,"frame write failed");}
 ck(!fclose(f),"frame close failed");
 /* Record player-centre collision geometry independent of the artwork and HUD.
    4px sampling, identical coordinates across all addresses, 960x540 raster. */
 n=snprintf(fn,sizeof fn,"%s/%s-%02d-walk.pgm",outdir,group,id);
 ck(n>0&&(size_t)n<sizeof fn,"mask path too long");
 f=fopen(fn,"wb");ck(f!=NULL,"cannot write walkability mask");
 fprintf(f,"P5\n%d %d\n255\n",W/4,H/4);
 for(int y=2;y<H;y+=4){unsigned char linebuf[W/4];for(int x=2;x<W;x+=4)
   linebuf[x/4]=(scene==4?wi_solid((float)x,(float)y):int_solid((float)x,(float)y))?0:255;
   ck(fwrite(linebuf,1,sizeof linebuf,f)==sizeof linebuf,"mask write failed");}
 ck(!fclose(f),"mask close failed");fprintf(manifest,"%s\t%d\t%s\t%s\t%s\n",group,id,leaf,title,subtitle);
}
static void check_counter(int rear){
 go(DESK_G);press_key(USE);
 if(!rear&&g_kind==13){ck(strstr(notice,"Mara")!=NULL&&menu<0,"apartment counter");}
 else{int li=legal_index_for_kind(g_kind);ck(menu==(rear&&li>=0?19+li:g_kind),"counter opened wrong menu");press_key(ESC);ck(menu<0,"cannot dismiss counter");}
}
static void check_ivo_sprite(void){
 const InteriorProp *booth=NULL;
 for(int j=0;j<interior_nprop;j++)if(interior_props[j].type==INT_DINER)booth=interior_props+j;
 ck(booth!=NULL,"Ivo's booth is missing");
 const int x=interior_desk_x,y=booth->y-8;
 ck(!int_solid(x,y),"Ivo's feet are inside furniture");
 int_static();
 /* Compare the unobstructed NPC with the baked scene. The couch used to
    repaint Ivo's head and torso after this same sprite had been drawn. */
 rect(x-12,y-20,25,28,0);
 person(x,y,mix(interior_theme(1)->neon,0x55738a,3),0,1.570796f,0,1);
 int drawn=0;
 for(int v=y-20;v<y+8;v++)for(int u=x-12;u<x+13;u++){
  U ink=pixels[v*W+u];if(!ink)continue;
  drawn++;
  ck(interior_back[v*W+u]==ink,"Ivo's sprite is obscured by booth furniture");
 }
 ck(drawn>64,"Ivo sprite comparison sampled too few pixels");
}
static void visit_named(int i){
 room_name=loc[i].label;g_kind=loc[i].kind;
 p.x=loc[i].x;p.y=loc[i].y;tick(0);press_key(USE);
 ck(scene==3&&interior_active_loc==i&&!interior_subroom,"front door opened wrong room");
 const InteriorTheme*t=interior_theme(g_kind);frame("venue",i,room_name,t->line);
 if(g_kind==1)check_ivo_sprite();
 go(FEATURE_G);press_key(USE);ck(interior_inspection==g_kind+1,"signature fixture did not react");
 check_counter(0);
 if(g_kind==15){
  go(CHIEF_G);press_key(USE);ck(strstr(notice,"Rivas")!=NULL,"chief failed to respond");
  go(CELLS_G);press_key(USE);ck(strstr(notice,"Custody")!=NULL,"cell inspection failed");
 }
 int li=legal_index_for_kind(g_kind);
 if(g_kind==4||li>=0){
  go(GATE_G);press_key(USE);ck(!interior_subroom&&scene==3,"unowned staff door opened");
  if(g_kind==4){sv.b[0].own=sv.b[0].tier=sv.units=1;}else sv.legit[li].owned=1;
  press_key(USE);ck(interior_subroom&&scene==3,"owned staff door stayed closed");
  const InteriorTheme*rear=interior_theme(g_kind);
  if(g_kind==4){
   go(DESK_G);press_key(USE);ck(menu==4&&nopt>0&&opt[sel].action==44,"batch terminal not offering start");
   press_key(ENTER);ck(menu==4&&sv.b[0].timer>0,"batch did not start");
   /* The visual proof shows actual timed growth driven by game ticks, while
      testing the player's reported case of leaving the production menu open. */
   for(int j=0;j<5580;j++)tick(0);
   ck(growth_fraction(0)>.70f&&growth_fraction(0)<.95f,"batch did not grow while menu was open");
   press_key(ESC);go(EXIT_G);
   frame("rear",i,room_name,rear->banner);
  }else{frame("rear",i,room_name,rear->banner);check_counter(1);go(EXIT_G);}
  press_key(USE);ck(scene==3&&!interior_subroom,"rear return door failed");rears_seen++;
 }
 go(EXIT_G);press_key(USE);ck(scene==1,"could not return to street");doors_seen++;
}
static void visit_shop(int b){
 room_name=walkin_name(b);p.x=ordinaryDoorX[b];p.y=ordinaryDoorY[b];tick(0);press_key(USE);
 ck(scene==4&&walkin_active==b,"street door opened wrong address");
 #ifdef DC_STREET_ADDRESS_PLAQUES
 frame("address",b,room_name,wiAddress[b].feature);
#else
 const WiRole*r=wiRoles+wi_role;frame("address",b,room_name,r->section);
#endif
 go(STATION_G);int sw=wi_sw[b];press_key(USE);
 if(tram_hall(b)){ck(strstr(notice,"tram")!=NULL,"tram station did not respond");}
 else{ck(wi_sw[b]!=sw,"station did not respond");press_key(USE);ck(wi_sw[b]==sw,"station did not return to initial state");}
 #ifdef DC_STREET_ADDRESS_PLAQUES
 go(PLAQUE_G);press_key(USE);
 ck(p.noticeT>0&&!strcmp(notice,wiPlaque[b]),"visitor plaque did not display this address's story");
#endif
 go(EXIT_G);press_key(USE);ck(scene==1&&p.x==ordinaryDoorX[b]&&p.y==ordinaryDoorY[b],"street exit location wrong");walkins_seen++;
}
static int shelf_cartons(void){
 const int x=GROW_RACK_X-(int)interior_cam_x,y=GROW_RACK_Y-(int)interior_cam_y;
 int count=0;paint();
 for(int i=0;i<3;i++)count+=pixels[(y+8)*W+x+11+i*34]==0xf0ebce;
 return count;
}
static void open_job_option(int id){
 int i=0;while(i<nopt&&opt[i].action!=id)i++;
 ck(i<nopt,"missing menu option");
 while(sel!=i)press_key(DOWN);
 press_key(ENTER);
}
static GrowPose active_grow_pose(float progress,float packing,float motion){
 GrowBatch v=grow_batch(sv.b);
 return grow_operator_pose(sv.b,&v,progress,packing,motion);
}
static void check_grow_targets(void){
 const float phases[]={.06f,.21f,.66f,.94f};
 const U impact_ink[]={0xebcc8b,0xa9e4df,0xf5d98b,0xa8be79};
 const int yields[2][3]={{5,10,17},{5,10,16}};
 const U cleared=0x010203;
 for(int product=0;product<2;product++)for(int tier=1;tier<=3;tier++){
  sv.b[0].product=product;sv.b[0].tier=tier;
  for(int i=0;i<4;i++){
   start_batch(sv.b,120.f);sv.b[0].timer=120.f*(1.f-phases[i]);tick(0);
   GrowBatch v=grow_batch(sv.b);
   ck(v.count==yields[product][tier-1],"active crop count missed tier or product");
   GrowPose a=active_grow_pose(growth_fraction(0),grow_pack_left,anim);
   ck(a.phase==GROW_PLANT+i&&!a.walk&&!int_solid(a.x,a.y),"work pose wrong or inside a bed");
   ck(a.pot>=0&&a.pot<v.count,"operator picked an empty pot");
   /* On the reported first batch, the first two upper-left pots are active.
      The former animation hit later columns despite their being empty. */
   if(tier==1&&i==1){
    ck(a.pot==4&&v.pot[a.pot].x==243,"tier one watering drifted off the second active pot");
    ck(v.pot[8].x==280&&v.pot[12].x==317&&v.count<8,"tier one watering fixture no longer tests later pots");
   }
   int dx=13,dy=-7;rect(0,0,W,H,cleared);int_grow_live(&v,dx,dy);
   for(int q=0;q<GROW_POT_COUNT;q++){
    int x=v.pot[q].x-dx,y=v.pot[q].y-dy;
    ck((pixels[y*W+x]!=cleared)==(q<v.count),"renderer changed which columns are unlocked");
   }
   int low=fastGfx;
   for(fastGfx=0;fastGfx<2;fastGfx++){
    rect(0,0,W,H,cleared);grow_operator_art(&v,dx,dy);
    for(int q=0;q<GROW_POT_COUNT;q++){
     int x=v.pot[q].x-dx,y=v.pot[q].y-dy+2;
     ck((pixels[y*W+x]==impact_ink[i])==(q==a.pot),"tool effect did not land solely on its active pot");
    }
   }
   fastGfx=low;
  }
  for(int i=0;i<=1000;i++){
   GrowPose a=active_grow_pose(i*.001f,0,0);
   ck(!int_solid(a.x,a.y),"operator movement route crosses furniture");
  }
 }
 sv.b[0].product=0;sv.b[0].tier=1;
}
static void check_grow_operator(void){
 room_name=loc[4].label;g_kind=loc[4].kind;
 /* Seed a legitimate owned loft with enough in-game funds to hire two ranks. */
 sv.b[0]=(Business){.own=1,.tier=1};sv.units=0;sv.cash=600;
 sv.stocks[0]=sv.stocks[1]=0;sv.stocks[2]=59;
 p.x=loc[4].x;p.y=loc[4].y;tick(0);press_key(USE);
 go(GATE_G);press_key(USE);ck(lantern_terminal(),"crew test did not reach grow room");
 go(DESK_G);press_key(USE);ck(menu==4,"crew hire menu unavailable");
 ck(active_grow_pose(0,0,0).phase==GROW_OFF,"operator appeared before hire");
 open_job_option(50);ck(sv.b[0].crew==1,"operator purchase failed");press_key(ESC);
 ck(active_grow_pose(0,0,0).phase==GROW_IDLE,"inactive operator does not stand by");
 for(int i=0;i<30;i++)tick(0);
 ck(shelf_cartons()==0,"Cinder stock appeared in Lantern room");
 sv.stocks[0]=9;sv.stocks[1]=10;ck(shelf_cartons()==1,"19 Lantern units should fill one box");
 sv.stocks[1]=11;ck(shelf_cartons()==2,"20 Lantern units should fill two boxes");
 sv.stocks[0]=39;ck(shelf_cartons()==2,"50 Lantern units should fill two boxes");
 sv.stocks[0]=40;ck(shelf_cartons()==3,"51 Lantern units should fill three boxes");
 ck(grow_case_count(500000000)==3+(500000000-51)/25,"large stock did not produce a bounded display count");
 sv.stocks[0]=sv.stocks[1]=0;tick(0);
 check_grow_targets();
 /* Half the former duration now covers 40% of each segment, without
    changing which plant is targeted or when the underlying batch finishes. */
 GrowBatch v=grow_batch(sv.b);
 int watering=grow_work_x(&v,grow_work_pot(&v,GROW_WATER)),feeding=grow_work_x(&v,grow_work_pot(&v,GROW_FEED));
 GrowPose moving=active_grow_pose(.465f,0,0);
 ck(moving.walk&&absf((float)moving.x-(watering+.4f*(feeding-watering)))<2.f,
    "operator aisle movement did not slow by 20 percent");
 int harvesting=grow_work_x(&v,grow_work_pot(&v,GROW_HARVEST));
 moving=active_grow_pose(.99f,GROW_PACK_SECONDS+GROW_RETURN_SECONDS-(GROW_PICKUP_TICKS+24)/60.f,0);
 ck(moving.phase==GROW_PACK&&moving.walk&&absf((float)moving.x-(harvesting+.4f*(557-harvesting)))<2.f,
    "operator carrying movement did not slow by 20 percent");
 moving=active_grow_pose(.06f,GROW_RETURN_SECONDS-18/60.f,0);
 ck(moving.phase==GROW_RETURN&&moving.walk&&absf((float)moving.x-(646+.4f*(557-646)))<2.f,
    "operator return movement did not slow by 20 percent");
 /* End a real operator-only batch and queue its replacement automatically. */
 sv.units=5;sv.b[0].timer=.01f;int old=sv.b[0].batches;tick(0);
 ck(sv.b[0].batches==old+1&&sv.b[0].timer>100.f&&warehouse_units(0)>0,"operator harvest or auto-queue failed");
 ck(active_grow_pose(growth_fraction(0),grow_pack_left,anim).phase==GROW_PACK,"new batch skipped the packing animation");
 ck(shelf_cartons()==0,"new carton appeared before it was put down");
 int recorded_return=0;
 GrowPose prior=active_grow_pose(growth_fraction(0),grow_pack_left,anim);
 int frames=(int)((GROW_PACK_SECONDS+GROW_RETURN_SECONDS)*60.f)+12;
 for(int i=0;i<frames;i++){
  GrowPose a=active_grow_pose(growth_fraction(0),grow_pack_left,anim);
  ck(!int_solid(a.x,a.y),"route from beds to stock shelf crosses furniture");
  ck(d2((float)(a.x-prior.x),(float)(a.y-prior.y))<=25.f,"operator snapped between route segments");
  prior=a;
  if(a.phase==GROW_RETURN)recorded_return=1;
  tick(0);
 }
 ck(recorded_return&&grow_pack_left==0.f&&shelf_cartons()==1,"packed stock or final return pose missing");
 go(DESK_G);press_key(USE);
 for(int i=0;i<nopt;i++)ck(opt[i].action!=50&&!strstr(opt[i].n,"DISTRIBUTOR"),"removed Lantern upgrade still in menu");
 int funds=sv.cash;
 doOpt(50);ck(sv.b[0].crew==1&&sv.cash==funds,"stale Lantern upgrade still bought");
 press_key(ESC);
 go(EXIT_G);press_key(USE);go(EXIT_G);press_key(USE);
 p.x=loc[7].x;p.y=loc[7].y;tick(0);press_key(USE);
 g_kind=7;go(DESK_G);press_key(USE);open_job_option(60);ck(warehouse_units(0)==0,"stock sale did not clear inventory");
 press_key(ESC);go(EXIT_G);press_key(USE);
 p.x=loc[4].x;p.y=loc[4].y;tick(0);press_key(USE);g_kind=4;
 go(GATE_G);press_key(USE);for(int i=0;i<30;i++)tick(0);
 ck(lantern_terminal()&&shelf_cartons()==0,"shelf did not reflect sale on return");
 puts("Lantern: hires, both products/tier crop targets and raster effects, clear aisle transit, automatic harvest/packing, staged crate counts, blocked distributor and sale/reentry passed.");
}
int main(int argc,char**argv){
 if(argc>2){fprintf(stderr,"Usage: %s [existing-frame-directory]\n",argv[0]);return 2;}
 if(argc==2){outdir=argv[1];char fn[800];int n=snprintf(fn,sizeof fn,"%s/rooms.tsv",outdir);ck(n>0&&(size_t)n<sizeof fn,"capture path too long");manifest=fopen(fn,"w");ck(manifest!=NULL,"cannot create manifest; create capture directory first");}
 init_city();scene=1;sv.intro=4;sv.legal=1;p.car=-1;job.kind=-1;mapOn=0;menu=-1;
 for(int i=0;i<LEN(loc);i++)visit_named(i);
 for(int i=0;i<nb;i++){
  if(buildings[i].w<=0||buildings[i].h<=0)continue;
  if(zone_is_building(i)){ck(zone_idx[zone_id[i]-1]==i,"named building entrance mapping invalid");continue;}
  ck(ordinaryDoorY[i]>0,"physical building lacks a street entrance");
#ifdef DC_STREET_ADDRESS_PLAQUES
  ck(wiPlaque[i]&&*wiPlaque[i]&&wiAddress[i].feature&&*wiAddress[i].feature,"address missing a custom plaque or installation title");
  for(int j=0;j<i;j++)if(ordinaryDoorY[j]){
   ck(strcmp(wiPlaque[i],wiPlaque[j])!=0,"two street addresses have the same plaque text");
   ck(strcmp(wiAddress[i].feature,wiAddress[j].feature)!=0,"two street addresses have the same installation title");
  }
#endif
  visit_shop(i);
 }
 ck(doors_seen==15&&rears_seen==4&&walkins_seen==63,"unexpected room count; inspect world inventory");
 if(manifest)ck(!fclose(manifest),"manifest close failed");
 check_grow_operator();
 printf("Passed %d public venues, %d gated rear rooms, %d street addresses; %d interaction routes, %d actual movement ticks.%s\n",
  doors_seen,rears_seen,walkins_seen,route_count,steps_taken,manifest?" Exported 82 native raster frames.":"");
 return 0;
}
