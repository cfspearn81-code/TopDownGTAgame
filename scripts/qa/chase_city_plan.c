/* Real input tape for both release hosts: owned ride, C on, street turn,
 * brake, C back off and save. All files are disposable. */
#define main existing_driving_plan
#include "../../src/verdict_driving_test.c"
#undef main
int main(int argc,char**argv){
 assert(argc==2);snprintf(prefix,sizeof prefix,"%s",argv[1]);
 set_session_seed(136);init(285,544);key(GFX);board(0);key(CAMERA);assert(ch_enabled());
 hold(UP,75);shot("avenue");hold(UP|RIGHT,18);hold(UP,26);hold(BRAKE,65);shot("crossing");
 hold(0,12);key(CAMERA);assert(!ch_enabled());key(SAVE);done();
 return 0;
}
