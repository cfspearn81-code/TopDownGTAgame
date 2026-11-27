/* Session-only NPC interaction hooks. These IDs label physical actors,
   not the small human sprites inside video/photo previews. */
enum {
 N_ACT_STREET=0,N_ACT_RIDER=64,N_ACT_FRONT=96,N_ACT_REAR=112,
 N_ACT_CHIEF=128,N_ACT_HQ1,N_ACT_HQ2,
 N_ACT_DANCE=136,N_ACT_DJ=139,N_ACT_MIKA=140,N_ACT_PATRON=141,
 N_ACT_GROW=144,N_ACT_CROSS=145,N_ACT_OPEN=159,
 N_ACT_OUTER=160,N_OUTER_SITES=6,N_OUTER_SEATS=4,
 N_ACT_OUTER_END=N_ACT_OUTER+N_OUTER_SITES*N_OUTER_SEATS,
 N_ACT_OUT_QUEUE=N_ACT_OUTER_END,N_ACT_OUT_SIGNAL=N_ACT_OUT_QUEUE+8,
 N_ACT_OUT_CLERK=N_ACT_OUT_SIGNAL+9,N_ACT_OUT_RANGER,N_ACT_OUT_HIKER,N_ACT_OUT_VOCAL=N_ACT_OUT_HIKER+2,
 N_ACT_OUT_MEDIA=N_ACT_OUT_VOCAL+1,N_ACT_OUT_EXTRA_END=N_ACT_OUT_MEDIA+3,
 N_ACT_ROOM_BASE=320,N_ACT_ROOM_END=361,N_ACT_MAX=N_ACT_ROOM_END
};
enum {
 NAR_STAGE=N_ACT_ROOM_BASE,NAR_DJ=NAR_STAGE+5,NAR_BAR,NAR_DOOR,
 NAR_GUEST=NAR_DOOR+3,NAR_REGULAR=NAR_GUEST+16,NAR_DISPUTE=NAR_REGULAR+3,
 NAR_SUITE=NAR_DISPUTE+4,NAR_OFFICE=NAR_SUITE+6,NAR_INEZ,NAR_END
};
static U npcRandom=0xa1387f29u;
static void npc_reset(void);
static void street_extra_reset(void),street_extra_tick(void),street_extra_down_paint(void);
static int street_extra_down(int id);
static U npc_filter_keys(U keys,U *tap);
static int npc_open_near(void),npc_open_context(void);
static int npc_front_option(void);
static int npc_replaces(int id);
static int npc_holds(int id);
static int npc_present_pose(int id,float *x,float *y,float *a,int *step);
static void npc_resume_room_actors(void);
static int npc_attack(U keys,float x,float y,float a);
static void npc_stance(float x,float y,float *a);
static void npc_encounter_tick(float dt);
static int npc_fighter_position(float *x,float *y);
static void npc_body_paint(int dx,int dy);
static void npc_room_trace(int dx,int dy);
static void npc_private_step(U keys,float x,float y,float*a);
static void npc_room_conditions_tick(float dt);
static int npc_private_use(U keys,U tap,int reserved);
static void npc_private_bodies(int dx,int dy);
static void npc_club_unit(int id);
static int npc_club_regular_live(int regular);
static int npc_club_direct(U keys,U tap,int reserved);
static void npc_aqua_actors(void);
static int npc_rail_solid(float x,float y);
static void npc_panel_paint(void);
static const char *npc_prompt(void);

/* Regional crowds share the same collision/reach rules as the city roster,
   but retain their own stable identities and don't consume job/patrol slots. */
typedef struct{int id;float x,y,rank;} OuterHit;
static OuterHit outer_pick(float x,float y,float co,float si,int melee,float limit,float minimum,float mx,float my);
static int outer_injure(int id,int damage,float x,float y);
static int outer_car_hit(float x,float y,float co,float si);
static OuterHit street_extra_pick(float x,float y,float co,float si,int melee,float limit,float minimum,float mx,float my);
static int street_extra_injure(int id,int damage,float x,float y);
static int street_extra_car_hit(float x,float y,float co,float si);
