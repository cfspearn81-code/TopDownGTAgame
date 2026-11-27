/* The survey rectangle ends in open water at the southeast coast. Extend its
 * sea material visually, not the navigable mask. Blend before fog/night tint,
 * with the same world-space ripple frequency on both sides of the boundary. */
static U wl_ocean_edge(U ink,unsigned x,unsigned y){
 if(x>=12000u||y>=12000u||!((x>=ROOM-72&&y>=2010)||(y>=ROOM-72&&x>=920)))return ink;
 int edge=min(ROOM-1-(int)x,ROOM-1-(int)y);
 if(edge>=0){
  int ix=(int)y*ROOM+(int)x;if(outerLand[ix]!=TILE_TIDE||outerLane[ix])return ink;
 }
 /* Feather at substeps rather than replacing in-bounds waves altogether. */
 unsigned band=(y+((x>>4)&3)+((x>>6)&3))%19,cell=(x>>3)+(y>>3)*7u;
 U water=band<2&&cell%11<6?0x304b57:(cell%3?0x1c3546:0x1f394a);
 return edge<0?water:nw_blend(ink,water,cl(255-(edge*255/71),0,255));
}
