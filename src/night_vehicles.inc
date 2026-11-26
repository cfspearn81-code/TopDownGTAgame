/* Projection shared by the street sprites and the live coachwork turntable.
 * U is fore/aft, V is transverse; height is real geometry in the side view.
 * No textures, fonts or downloaded 3D assets. */
typedef struct {float tail,nose,width,trunk,wind,roofTail,roofNose,roofHeight,hood;} NgShape;
static const NgShape ngShape[5]={
 {-14.4f,14.6f,7.8f,-12.5f,7,-10.6f,3.4f,9,4.0f},
 {-16.3f,16.5f,7.8f,-11,8.3f,-6.9f,2.6f,8,3.8f},
 {-17.4f,16.8f,8.0f,-12.5f,8,-8.0f,3.0f,8.7f,4.5f},
 {-16.8f,17.4f,8.2f,-11,7.1f,-6.0f,1.2f,6.5f,3.0f},
 {-16.5f,15.2f,8.0f,-15,8.8f,-13.8f,4.8f,12.0f,4.5f}};
static const U ngPaint[]={0,0x368da9,0x9cc34a,0xfc9654,0xae7ad6,0xe3d8b6,0x354456,0xbeeef0};
static const U ngTags[]={0xc1bba3,0x9fe04c,0xef9969,0x80d3e6};
static const U ngRims[]={0x98aab0,0xf2d799,0xb7dee5,0xf48d80};
typedef struct {int paint,rim,kit,tune,grip,sub,tag;} NgMods;
static NgMods ng_installed(int type);
typedef struct {int x,y;float s,co,si,depth,up;} NgRig;
typedef struct {int x,y;} NgPt;
static NgPt ng_pt(const NgRig*r,float u,float v,float z0){
 return (NgPt){r->x+(int)((u*r->co-v*r->si)*r->s),
 r->y+(int)(((u*r->si+v*r->co)*r->depth-z0*r->up)*r->s)};
}
static void ng_surface(NgPt a,NgPt b,NgPt c,NgPt d,U col){
 tri(a.x,a.y,b.x,b.y,c.x,c.y,col);tri(a.x,a.y,c.x,c.y,d.x,d.y,col);
}
static void ng_l(NgPt a,NgPt b,U c){line(a.x,a.y,b.x,b.y,c);}
/* Horizontal, side and end strips. Color of the visible side responds to the
 * stationary light source instead of remaining a flat body-colored slab. */
static void ng_deck(const NgRig*r,float u,float uu,float w,float ww,float h,float hh,U c){
 ng_surface(ng_pt(r,u,-w,h),ng_pt(r,uu,-ww,hh),ng_pt(r,uu,ww,hh),ng_pt(r,u,w,h),c);
}
static void ng_side(const NgRig*r,float u,float uu,float w,float ww,float h,float hh,float sgn,U c){
 ng_surface(ng_pt(r,u,sgn*w,h),ng_pt(r,uu,sgn*ww,hh),ng_pt(r,uu,sgn*ww,1.3f),ng_pt(r,u,sgn*w,1.3f),c);
}
static void ng_panel(const NgRig*r,float u,float uu,float w,float ww,float h,float hh,float sgn,float base,U c){
 ng_surface(ng_pt(r,u,sgn*w,h),ng_pt(r,uu,sgn*ww,hh),ng_pt(r,uu,sgn*(ww+1),base),ng_pt(r,u,sgn*(w+1),base),c);
}
static void ng_disc3(const NgRig*r,float u,float v,float z0,float rr,int horiz,U c){
 NgPt last={0},mid=ng_pt(r,u,v,z0);
 for(int i=0;i<=12;i++){
  float a=(float)(i%12)*.523599f,b=rr*cs(a),d=rr*sn(a);
  NgPt cur=ng_pt(r,u+b,v+(horiz?d:0),z0+(horiz?0:d));
  if(i)tri(mid.x,mid.y,last.x,last.y,cur.x,cur.y,c);last=cur;
 }
}
static void ng_wheels(const NgRig*r,const NgShape*s,int side,int rim,int low){
 float tire=low?2.65f:2.9f;
 for(int i=0;i<2;i++){
  float u=i?s->nose-5.3f:s->tail+5.0f,v=side*(s->width+.28f),h=tire;
  ng_disc3(r,u,v,h,tire,0,0x10181e);
  ng_disc3(r,u,v,h,tire*.68f,0,ngRims[cl(rim,0,3)]);
  ng_disc3(r,u,v,h,tire*.45f,0,0x323941);
  if(r->s>2)for(int k=0;k<6;k++){
   float a=k*1.047198f;ng_l(ng_pt(r,u+.4f*cs(a),v,h+.4f*sn(a)),ng_pt(r,u+1.7f*cs(a),v,h+1.7f*sn(a)),ngRims[rim]);
  }
 }
}
static void ng_model(int x,int y,float a,float scale,int preview,int type,U color,NgMods m,int cond,int lamps,int subsOpen){
 const NgRig view={x,y,scale,cs(a),sn(a),preview?.52f:.92f,preview?.82f:.34f},*r=&view;
 const NgShape *s=ngShape+cl(type,0,4);float t=s->tail,n=s->nose,w=s->width,front=s->wind,rear=s->trunk;
 int side=r->co>=0?1:-1;float lo=m.grip>=2?.6f:0,h=s->hood-lo,roof=s->roofHeight-lo;
 U shell=cond?(m.paint?ngPaint[cl(m.paint,1,7)]:color):0x475056;
 U edge=mix(shell,0xfcf4df,3),shade=mix(shell,0x17242a,4),glass=0x334c5c,rim=0x779da5;
 /* Soft, rounded contact shadow rather than a floating rectangular slab. */
 NgPt qShadow={0},hub=ng_pt(r,0,.7f,-.65f);
 for(int i=0;i<=18;i++){float q=(i%18)*.349066f;NgPt b=ng_pt(r,(n-t)*.53f*cs(q),w*1.17f*sn(q)+.7f,-.65f);
  if(i)tri(hub.x,hub.y,qShadow.x,qShadow.y,b.x,b.y,0x111d23);qShadow=b;}
 ng_wheels(r,s,-side,m.rim,m.grip>=2);
 /* Four belt sections with model-specific taper; retain the simple,
  * readable navigation footprint of the old 32 x 16 collision model. */
 float uu[5]={t,t+3,(front+rear)*.5f,n-4,n};
 float ww[5]={w-(type==4?.4f:2.f),w,w,w-.3f,w-(type==3?3.3f:1.8f)};
 float hh[5]={h-.85f,h,h+.2f,h,h-(type==3?1.3f:.55f)};
 for(int j=0;j<4;j++){ng_side(r,uu[j],uu[j+1],ww[j],ww[j+1],hh[j],hh[j+1],-side,shade);
  ng_side(r,uu[j],uu[j+1],ww[j],ww[j+1],hh[j],hh[j+1],side,mix(shell,0x16292e,side>0?4:3));
  ng_deck(r,uu[j],uu[j+1],ww[j],ww[j+1],hh[j],hh[j+1],shell);}
 /* Vertical front and rear fascia have thickness and distinct signatures,
  * and never paint through the nose when viewed from the opposite end. */
 int face=r->si>=0?1:-1;float e=face>0?n:t,ew=face>0?ww[4]:ww[0],eh=face>0?hh[4]:hh[0];
 ng_surface(ng_pt(r,e,-ew,eh),ng_pt(r,e,ew,eh),ng_pt(r,e,ew-.5f,1.0f),ng_pt(r,e,-ew+.5f,1.0f),mix(shell,0x16262c,3));
 ng_l(ng_pt(r,e,-ew+.6f,1.15f),ng_pt(r,e,ew-.6f,1.15f),0x526269);
 if(preview){
  ng_surface(ng_pt(r,e,-ew*.39f,eh-.34f),ng_pt(r,e,ew*.39f,eh-.34f),ng_pt(r,e,ew*.33f,1.55f),ng_pt(r,e,-ew*.33f,1.55f),0x25343a);
  if(face>0)ng_l(ng_pt(r,e,-ew*.28f,eh-.95f),ng_pt(r,e,ew*.28f,eh-.95f),0x617078);
  for(int k=-1;k<=1;k+=2){
   U lamp=face>0?(cond?0xffe2a7:0x687375):(cond?0xc04746:0x5e3938);
   ng_surface(ng_pt(r,e,k*(ew-.32f),eh-.06f),ng_pt(r,e,k*(ew-2.7f),eh-.06f),ng_pt(r,e,k*(ew-2.45f),eh-1.0f),ng_pt(r,e,k*(ew-.38f),eh-1.0f),lamp);
  }
 }
 ng_l(ng_pt(r,t+2,side*w,h),ng_pt(r,n-4,side*(w-.3f),h),edge);
 /* Wheels outside sill, visible door trim, mirror and handle. */
 ng_wheels(r,s,side,m.rim,m.grip>=2);
 ng_l(ng_pt(r,t+6,side*(w+.2f),1.9f),ng_pt(r,n-6,side*(w+.2f),1.9f),m.kit?ngRims[m.rim]:shade);
 ng_l(ng_pt(r,-2,side*w,h-.45f),ng_pt(r,.25f,side*w,h-.45f),rim);
 if(m.kit>=1){ng_deck(r,n-1,n+.7f,w-1,w-1,1.15f,1.15f,0x25303a);
  ng_deck(r,t+1,t+2.7f,w+1,w+1,h+1.5f,h+1.5f,0x263947);}
 if(m.kit>=2){ng_deck(r,t+.5f,t+2.8f,w+1.5f,w+1.5f,h+3.f,h+3.f,edge);
  ng_l(ng_pt(r,t+2,side*(w-1),h),ng_pt(r,t+2,side*(w-1),h+3),shade);}
 /* Stamped shoulder and door shut-lines in the hero view. Model-specific
  * hood lines catch a different edge of the stationary studio strip. */
 if(preview){
  for(int k=-1;k<=1;k+=2){
   ng_l(ng_pt(r,front+.6f,k*(w-2.2f),h+.02f),ng_pt(r,n-3,k*(w-3.15f),h-.2f),mix(shell,0xe9ebdf,2));
   ng_l(ng_pt(r,front+.5f,k*(w-2.55f),h+.02f),ng_pt(r,n-3,k*(w-3.5f),h-.2f),mix(shell,0x32424b,1));
  }
  ng_l(ng_pt(r,3.4f,side*w,h-.24f),ng_pt(r,2.9f,side*(w+.05f),1.65f),shade);
  ng_l(ng_pt(r,-5.7f,side*w,h-.24f),ng_pt(r,-5.3f,side*(w+.05f),1.65f),shade);
 }
 /* Greenhouse: separate fascia, cowl, window bands and front/rear glass. */
 float rw=w-2;
 ng_deck(r,rear,s->roofTail,w-1,rw,h+.1f,roof,type==4?shade:glass);
 ng_deck(r,s->roofNose,front,rw,w-1.15f,roof,h+.15f,glass);
 if(preview){
  /* Broad blue reflection cuts across the glazing, coherent at all headings. */
  ng_surface(ng_pt(r,s->roofNose+.15f,-rw+.45f,roof-.1f),ng_pt(r,s->roofNose+.15f,rw*.3f,roof-.1f),
   ng_pt(r,front-.7f,-rw*.15f,h+.65f),ng_pt(r,front-.7f,-rw+.25f,h+.65f),0x506b7a);
  ng_l(ng_pt(r,front-.5f,-rw+.15f,h+.4f),ng_pt(r,front-.5f,rw-.15f,h+.4f),0x607f88);
 }
 ng_panel(r,s->roofTail,s->roofNose,rw,rw,roof,roof,side,h+.65f,0x192d39);
 ng_l(ng_pt(r,s->roofTail+1,side*rw,roof),ng_pt(r,s->roofNose-1,side*rw,roof),rim);
 if(preview&&type!=4)ng_surface(ng_pt(r,s->roofNose-.5f,side*rw,roof-.12f),ng_pt(r,s->roofNose-2.2f,side*rw,roof-.12f),
  ng_pt(r,s->roofNose-4.1f,side*(rw+1),h+.72f),ng_pt(r,s->roofNose-2.35f,side*(rw+1),h+.72f),0x365260);
 if(type==4)ng_panel(r,s->roofTail+.35f,-2,rw,rw,roof-.18f,roof-.18f,side,h+.75f,shell);
 ng_l(ng_pt(r,-2,side*rw,roof),ng_pt(r,-1,side*(rw+1),h+.5f),shade);
 ng_deck(r,s->roofTail,s->roofNose,rw,rw,roof,roof,mix(shell,0xd7ecee,1));
 ng_l(ng_pt(r,s->roofTail,-rw,roof),ng_pt(r,s->roofNose,-rw,roof),edge);
 ng_l(ng_pt(r,s->roofNose+.7f,-rw+.7f,roof-.45f),ng_pt(r,front-1,-rw+3.1f,h+.8f),rim);
 if(preview){ng_l(ng_pt(r,s->roofTail+.8f,-rw+.4f,roof+.01f),ng_pt(r,s->roofNose-.65f,-rw+.4f,roof+.01f),edge);
  ng_l(ng_pt(r,s->roofTail+.1f,rw-.28f,roof+.01f),ng_pt(r,s->roofNose,rw-.28f,roof+.01f),mix(shell,0x354647,2));}
 if(type==4)for(int k=0;k<3;k++)ng_l(ng_pt(r,s->roofTail+2,-3+k*3,roof+.1f),ng_pt(r,s->roofNose-2,-3+k*3,roof+.1f),shade);
 if(type==3||m.paint==7){for(int j=-1;j<=1;j+=2){
   ng_l(ng_pt(r,s->roofTail,j*.85f,roof+.1f),ng_pt(r,s->roofNose,j*.85f,roof+.1f),0xe8dcba);
   ng_l(ng_pt(r,front+1,j*.85f,h+.3f),ng_pt(r,n-2,j*.85f,h-.1f),0xe8dcba);}}
 for(int k=-1;k<=1;k+=2){
  ng_l(ng_pt(r,n-1.1f,k*(w-1),h-.5f),ng_pt(r,n-.8f,k*(w-3.6f),h-.5f),cond?0xffebbf:0x687375);
  U brake=(lamps&1)?0xff5148:cond?0xae4253:0x623540;
  ng_l(ng_pt(r,t+.5f,k*(w-1.3f),h-.55f),ng_pt(r,t+.4f,k*(w-3.5f),h-.55f),brake);
  if(lamps&2){NgPt q=ng_pt(r,t+.5f,k*(w-4.5f),h-.55f);dot(q.x,q.y,0xe2faff);}
  ng_l(ng_pt(r,front-2,k*(w-.55f),h+.4f),ng_pt(r,front-1.5f,k*(w+.7f),h+.55f),edge);
 }
 if(preview){ng_l(ng_pt(r,n,-2,1.8f),ng_pt(r,n,2,1.8f),0xb9b99f);
  ng_l(ng_pt(r,t,-1.6f,2.1f),ng_pt(r,t,1.6f,2.1f),ngTags[m.tag]);}
 if(m.sub&&subsOpen){
  /* Lid parked upward; enclosure embedded in the rear deck. */
  float deck=t+1.2f,level=type==4?roof+.23f:h+.45f;
  ng_deck(r,deck,deck+5.2f,w-1.2f,w-1.2f,level,level,0x202930);
  for(int k=0;k<(m.sub>1?2:1);k++){
   float sideSub=m.sub>1?(k?3.f:-3.f):0;
   ng_disc3(r,deck+2.65f,sideSub,level+.1f,2.2f,1,ngRims[m.rim]);
   ng_disc3(r,deck+2.65f,sideSub,level+.14f,1.7f,1,0x24232b);
   ng_disc3(r,deck+2.65f,sideSub,level+.19f,.65f,1,m.sub>1?0xe48656:0x869fa3);
  }ng_deck(r,deck+5.2f,deck+6.6f,w-1.2f,w-1.8f,level,level+3.2f,shade);
 }
}
