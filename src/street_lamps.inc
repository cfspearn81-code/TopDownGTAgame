/* Career Files (bd5d4e61) lamp palette, rectangular bodies and offsets.
 * The seven-by-seven original crossings share this catalogue with the
 * time/weather compositor and optional perspective camera. */
typedef struct {
 int x,y,radius,weight;
 int poleX,poleY,poleW,poleH,lensX,lensY,lensW,lensH;
 U pool,pole,lens;
} StreetLamp;
static StreetLamp street_lamp_style(int x,int y,U c,int secondary){
 if(secondary)return (StreetLamp){x,y,27,3,0,0,2,10,-2,-4,6,5,0x549e92,0x4b6364,TEAL};
 return (StreetLamp){x,y,46,4,-1,-1,3,13,-4,-4,8,4,c,0x293842,mix(c,PAPER,5)};
}
static int street_lamp_legacy(int gx,int gy,int secondary,StreetLamp *s){
 if(gx<1||gy<1||gx>=BASE_ROOM/256||gy>=BASE_ROOM/256)return 0;
 if(secondary&&(gx+gy)%2)return 0;
 if(secondary)*s=street_lamp_style(gx*256-34,gy*256+38,0,1);
 else{
  int x=gx*256,y=gy*256;
  *s=street_lamp_style(x+31,y-34,y<600?0x845987:x>1300?0x388b8c:0x8d7352,0);
 }
 return 1;
}
static int street_lamp_outer(int i,StreetLamp *s){
 if(i<0||i>=LEN(outerNodes)||(i&1))return 0;
 OuterPoint q=outerNodes[i];if(!outer_is_new(q.x,q.y))return 0;
 /* Use the existing Outer Loop lamp anchors rather than hypothetical city
  * blocks along the highways. Fit the older palette around each district. */
 *s=street_lamp_style(q.x+23,q.y-17,q.y<810?0x845987:q.y<1520?0x8d7352:0x388b8c,0);
 return 1;
}
enum { HARBOR_LAMPS=3 };
static int street_lamp_harbor(int id,StreetLamp*s){
 if((unsigned)id>=HARBOR_LAMPS||!s)return 0;
 const OuterPoint*q=harborDockPoints+id*2;
 *s=street_lamp_style(q->x+(id==0?16:13),q->y-12,id==1?0xa38a5d:0x488f87,0);
 s->radius=27;s->weight=3;return 1;
}
static void street_lamp_bake(const StreetLamp*s){
 light(s->x,s->y,s->radius,s->pool,s->weight);
 rect(s->x+s->poleX,s->y+s->poleY,s->poleW,s->poleH,s->pole);
 rect(s->x+s->lensX,s->y+s->lensY,s->lensW,s->lensH,s->lens);
}
