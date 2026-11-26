/* Addition to the world, not to Save. A modal holds all clocks. Activity ticks
 * run on the game clock, after movement and before a native E interaction. */
enum {VA_A1,VA_A2,VA_A3,VA_A4,VA_B1,VA_B2A,VA_B2B,VA_B3A,VA_B3B,VA_B3C,
      VA_B4,VA_B5,VA_B6A,VA_B6B,VA_B7,VA_B8,VA_B9,VA_COUNT};
enum {VE_RESET,VE_START,VE_CANCEL,VE_TICK,VE_WORLD,VE_DRAW,VE_AMBIENT,VE_DECOR};
static const char *const vdNames[VA_COUNT]={"Side B Before Sign-Off","One-Take Corner","One More Stop","Sunday Supper",
 "Cab Rank","Nightglass Valet","Curb Chess","City in Focus","Poster Detective","Dial Hunter","The Block Bill",
 "Switchboard Drop","Crossing Keeper","Freight Crossing","Dead Air Live","Harbor Proof","The Second Shadow"};
static int vdActive=-1,vdChoice,vdPanel,vdSelected,vdCursor,vdGoalX=-1,vdGoalY=-1;
static char vdGoalName[100],vdLine[230],vdTitle[180],vdOptions[2][180];
static void vd_end(void),vd_cancel(void),vd_reset(void),vd_status(const char*),vd_goal(int,int,const char*);
static void vd_choose(const char*,const char*,const char*);
static int vd_launch(int),vd_input(U),vd_frame(U,U),vd_locked(void);
static void vd_draw(void),vd_world(void);
#if __has_include("verdict_taxi.inc")
static const char *taxi_route_name(void);
#endif
