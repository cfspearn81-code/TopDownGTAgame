/* Authored job floor art belongs to the same world coordinates in either
 * view. Isolate the native layout in a transparent decal surface, anchor
 * people and text upright, and let pavement depth/houses occlude the rest.
 * No pixels are ever written back to the baked city. */
static int chDecals,chCollect,chLabelN,chActorN;
static U chJobMap[W*H];
static struct {int x,y;U ink;char words[96];}chLabel[48];
static StreetPerson chAct[24];
static int ch_record_label(int x,int y,const char*words,U ink){
 if(!chCollect)return 0;
 if(chLabelN<LEN(chLabel)&&d2(p.x-x,p.y-y)<270.f*270.f){
  int i=chLabelN++;chLabel[i].x=x;chLabel[i].y=y;chLabel[i].ink=ink;
  int j=0;while(words[j]&&j<95){chLabel[i].words[j]=words[j];j++;}chLabel[i].words[j]=0;
 }
 return 1;
}
static int ch_record_actor(int x,int y,U coat,int step,float a,int role,int hue){
 if(!chCollect)return 0;
 if(chActorN<LEN(chAct))chAct[chActorN++]=(StreetPerson){x+cx,y+cy,a,step,role,hue,coat};
 return 1;
}
static void ch_collect_jobs(void){
 chDecals=chLabelN=chActorN=0;
 int crossing=vdActive==VA_B6A||vdActive==VA_B6B||cr.mode==CR_CONVOY;
 if(vdActive<0&&!crossing)return;
 U*old=surface;int aw=rw,ah=rh,l=clipLeft,t=clipTop,r=clipRight,b=clipBottom;
 /* Volatile byte zero() is intended for tiny game-state structs; clearing
  * a 2 MiB CPU drawing target this way every frame costs a full extra ms. */
 typedef U Chunk __attribute__((vector_size(16),aligned(1),may_alias));
 for(int j=0;j<W*H/4;j++)((Chunk*)chJobMap)[j]=(Chunk){0,0,0,0};
 surface=chJobMap;clipLeft=clipTop=0;clipRight=rw=W;clipBottom=rh=H;
 chCollect=chDecals=1;if(crossing)vd_cross_paint();if(vdActive>=0)vd_world();chCollect=0;
 surface=old;rw=aw;rh=ah;clipLeft=l;clipTop=t;clipRight=r;clipBottom=b;
}
static inline U ch_decal_pixel(U source,unsigned x,unsigned y){
 unsigned u=x-(unsigned)cx,v=y-(unsigned)cy;
 if(u<W&&v<H){U col=chJobMap[v*W+u];if(col)return col;}
 return source;
}
/* Reuse the physical handbrake wheel contacts, not a second emitter.
 * The fixed-size queue ages offscreen. No scans/polygons if it is empty. */
static void ch_skid_strips(void){
 for(int n=0;n<roadCount;n++){
  const RoadMark*m=roadMarks+(roadHead-roadCount+n+ROAD_MARK_COUNT)%ROAD_MARK_COUNT;
  if(d2(m->ax[0]-ch.x,m->ay[0]-ch.y)>290.f*290.f)continue;
  int ink=cl((int)(ROAD_MARK_LIFE-(roadNow-m->born))/55+1,1,5);
  for(int i=0;i<2;i++){
   float x=m->ax[i],y=m->ay[i],xx=m->bx[i],yy=m->by[i];
   float dx=xx-x,dy=yy-y,len=sq(d2(dx,dy));if(len<.1f)continue;
   float u=-dy*(.54f/len),v=dx*(.54f/len);
   U floor=(unsigned)(int)x<ROOM&&(unsigned)(int)y<ROOM?chaseFloor[(int)y*ROOM+(int)x]:0x344149;
   ch_quad((ChVec){x-u,y-v,.14f},(ChVec){xx-u,yy-v,.14f},
           (ChVec){xx+u,yy+v,.14f},(ChVec){x+u,y+v,.14f},mix(floor,0x0b131b,ink),1);
  }
 }
}
static void ch_chalk_pixel_actor(void){
 if(roadCount)ch_skid_strips();
 for(int i=0;i<chActorN;i++)ch_character(chAct+i);
 /* The Signal meet customer uses exactly the chosen action spot. */
#if __has_include("night_phone.inc")
 int j=np_job(),at=np_job_dest();
 if(j>=1&&at>=0&&!np_get(NP_HOLD,600)&&d2(p.x-loc[at].x,p.y-loc[at].y)<210.f*210.f){
  static const int dx[]={22,-22,16,-22,0},dy[]={8,9,22,20,20};
  for(int i=0;i<LEN(dx);i++){
   int x=loc[at].x+dx[i],y=loc[at].y+dy[i];if(solid(x,y,7))continue;
   ch_upright(x,y,1.4f,19.f,20.6f,0xff9adb);break;
  }
 }
#endif

}
static void ch_job_labels(void){
 for(int i=0;i<chLabelN;i++){
  ChVec v=ch_local((ChVec){chLabel[i].x,chLabel[i].y,9});if(v.z<12||v.z>275)continue;
  ChPlot a=ch_screen(v);int w=textw(chLabel[i].words,0)+12;
  if(a.x<w/2||a.x>W-w/2||a.y<113||a.y>H-75||a.z+200<chZ[a.y*W+a.x])continue;
  box(a.x-w/2,a.y-21,w,20,DARK,chLabel[i].ink);center(a.x,a.y-22,chLabel[i].words,chLabel[i].ink,0);
 }
}
