#define main existing_test
#include "../../src/verdict_driving_test.c"
#undef main
int main(int argc,char**argv){
 assert(argc==2);snprintf(prefix,sizeof prefix,"%s",argv[1]);set_session_seed(136);
 init(278,548);key(GFX);trace("spawn");board(0);key(CAMERA);trace("entered-car");
 drive(442,550,1);trace("aligned");shot("clear");
 int hit=0;
 for(int i=0;i<140;i++){
  Car*c=cars;float a=wrap(look(478-c->x,550-c->y)-c->a);k(UP|(a>.028f?RIGHT:a<-.028f?LEFT:0));
  if(lampBumpRide==0){hit=c->cond;trace("lamp hit");shot("impact");break;}
  if(i%25==24)trace("approaching");
 }
 assert(hit&&hit<100);
 hold(UP,120);trace("held");assert(cars[0].cond==hit);shot("held");
 hold(DOWN,55);trace("away");assert(lampBumpRide<0);shot("away");key(CAMERA);key(SAVE);done();return 0;
}
