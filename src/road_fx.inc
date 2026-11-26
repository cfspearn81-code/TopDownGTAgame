/* Transient, bounded road feedback. World coordinates stay put as the camera
 * moves. The FX never touch collision, saves, or the game's RNG stream. */
typedef struct {short ax[2],ay[2],bx[2],by[2]; U born;} RoadMark;
enum {ROAD_MARK_COUNT=128,ROAD_MARK_LIFE=300};
static RoadMark roadMarks[ROAD_MARK_COUNT];
static U roadNow;static int roadHead,roadCount,playerBrake;
static void roadFX_reset(void){roadNow=0;roadHead=roadCount=playerBrake=0;}
static void roadFX_tick(void){
 roadNow++;playerBrake=0;
 /* Unsigned ages are valid across wrap. Remove oldest first; inactive
  * streets never need to scan a ring full of stale marks. */
 while(roadCount&&roadNow-roadMarks[(roadHead-roadCount+ROAD_MARK_COUNT)%ROAD_MARK_COUNT].born>=ROAD_MARK_LIFE)roadCount--;
}
static void roadFX_player_car(const Car*c,float oldX,float oldY,float oldA,float before,int pedal,U keys){
 playerBrake=!!(keys&BRAKE)||(pedal>0&&c->s>1.f)||(pedal<0&&c->s<-1.f);
 if(!(keys&BRAKE)||before<48.f||d2(c->x-oldX,c->y-oldY)<.16f)return;
 float oldCo=cs(oldA),oldSi=sn(oldA),co=cs(c->a),si=sn(c->a);
 RoadMark*m=roadMarks+roadHead;m->born=roadNow;
 for(int i=0;i<2;i++){
  int side=i?6:-6;
  m->ax[i]=(short)(oldX-10*oldCo-side*oldSi);m->ay[i]=(short)(oldY-10*oldSi+side*oldCo);
  m->bx[i]=(short)(c->x-10*co-side*si);m->by[i]=(short)(c->y-10*si+side*co);
 }
 roadHead=(roadHead+1)%ROAD_MARK_COUNT;if(roadCount<ROAD_MARK_COUNT)roadCount++;
}
static int road_lamps(const Car*c){
 if(c->cond<=0)return 0;
 int braking=p.car>=0&&c==cars+p.car&&playerBrake;
 return braking|(c->s<-2.f?2:0);
}
static void roadFX_paint(int left,int top){
 if(fastGfx)return;
 for(int n=0;n<roadCount;n++){
  const RoadMark*m=roadMarks+(roadHead-roadCount+n+ROAD_MARK_COUNT)%ROAD_MARK_COUNT;
  int ink=cl((int)(ROAD_MARK_LIFE-(roadNow-m->born))/55+1,1,5);
  for(int i=0;i<2;i++){
   int x=m->ax[i]-left,y=m->ay[i]-top,dx=m->bx[i]-m->ax[i],dy=m->by[i]-m->ay[i];
   int steps=max(absf(dx),absf(dy));if(!steps)steps=1;
   for(int j=0;j<steps;j++){
    int px=x+dx*j/steps,py=y+dy*j/steps;
    if((U)px<(U)W&&(U)py<(U)H){U*pix=pixels+py*W+px;*pix=mix(*pix,0x0b131b,ink);}
   }
  }
 }
}
