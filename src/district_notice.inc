/* A short, unintrusive exterior arrival title. Boundary jitter does not
 * complete the dwell timer, and rapid crossings share a six-second budget. */
static const char *districtStable,*districtCandidate;
static float districtDwell,districtCooldown,districtTTL;
static void district_notice_reset(void){
 districtStable=districtCandidate=district((int)p.x,(int)p.y);
 districtDwell=districtCooldown=districtTTL=0;
}
static void district_notice_tick(float dt){
 if(districtCooldown>0.f)districtCooldown-=dt;
 if(districtTTL>0.f)districtTTL-=dt;
 const char *here=district((int)p.x,(int)p.y);
 if(here!=districtCandidate){districtCandidate=here;districtDwell=0.f;}
 else if(districtDwell<1.f)districtDwell+=dt;
 if(here!=districtStable&&districtDwell>=.85f&&districtCooldown<=0.f){
  districtStable=here;districtTTL=2.4f;districtCooldown=6.f;
 }
}
static void district_notice_paint(void){
 if(scene!=1||p.dead||menu>=0||atmOpen>=0||mapOn||vdPanel||wheelOpen||
    districtTTL<=0.f||districtCandidate!=districtStable||
    districtStable!=district((int)p.x,(int)p.y))return;
 const int w=textw(districtStable,0)+21,x=451-w;
 box(x,16,w,24,0x182431,0x4d6974);
 rect(x+1,18,2,20,TEAL);
 textat(x+11,17,districtStable,PAPER,0);
}
