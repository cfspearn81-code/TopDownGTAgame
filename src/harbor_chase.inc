/* Rigid harbor furniture rises above the same baked footprints in chase.
   Road charts and hull clearance do not depend on which camera is used. */
static void ch_harbor_props(void){
 for(int i=0;i<LEN(harborPieces);i++){
  if(!harborPiecePlaced[i])continue;
  const HarborPiece*p0=harborPieces+i;float l=p0->x,r=l+p0->w,t=p0->y,b=t+p0->h,h=p0->height;
  if(d2((l+r)*.5f-ch.x,(t+b)*.5f-ch.y)>650.f*650.f)continue;
  U panel=p0->body,base=mix(panel,0x273847,3);
  if(ch.y<t)ch_quad((ChVec){l,t,0},(ChVec){r,t,0},(ChVec){r,t,h},(ChVec){l,t,h},panel,1);
  if(ch.x>r)ch_quad((ChVec){r,t,0},(ChVec){r,b,0},(ChVec){r,b,h},(ChVec){r,t,h},base,1);
  if(ch.y>b)ch_quad((ChVec){r,b,0},(ChVec){l,b,0},(ChVec){l,b,h},(ChVec){r,b,h},mix(panel,PAPER,1),1);
  if(ch.x<l)ch_quad((ChVec){l,b,0},(ChVec){l,t,0},(ChVec){l,t,h},(ChVec){l,b,h},base,1);
  ch_quad((ChVec){l,t,h},(ChVec){r,t,h},(ChVec){r,b,h},(ChVec){l,b,h},p0->roof,1);
  if(ch.y>b)for(float x=l+3;x<r-1;x+=5)
   ch_quad((ChVec){x,b+.18f,2},(ChVec){x+.7f,b+.18f,2},(ChVec){x+.7f,b+.18f,h-1},(ChVec){x,b+.18f,h-1},base,1);
 }
 for(int i=0;i<HARBOR_N_DOCK_POINTS;i+=2){
  float x=harborDockPoints[i].x,y=harborDockPoints[i].y;
  if(d2(x-ch.x,y-ch.y)>610.f*610.f)continue;
  for(int d=-1;d<=1;d+=2){
   ch_upright(x+13*d,y-11,1.7f,0,5,0x936f55);
   ch_upright(x+13*d,y-11,1.9f,3,5,0xe3c084);
  }
  ch_quad((ChVec){x-11,y+9,.25f},(ChVec){x+11,y+9,.25f},
          (ChVec){x+11,y+11,.25f},(ChVec){x-11,y+11,.25f},0xe7c29c,1);
 }
}
