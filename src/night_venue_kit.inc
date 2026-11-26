/* Purely invented pulse-game equipment. No simulation of real weapon parts,
 * operation, ballistics or materials. Slot values packed into former spare
 * world words without changing the Rev11 save format. */
enum { NVH_DEED=1, NVK_CONFIG=2, NVK_RESERVE=3, NVK_BUYS=4, NVF_EXTRA=5 };
enum { NVK_FRAME_BIT=0,NVK_LENS_BIT=2,NVK_CELL_BIT=4,NVK_LOAD_BIT=6,NVK_CASE_BIT=10,
       NVK_CASE_LENS_BIT=12,NVK_CASE_CELL_BIT=14 };
typedef struct{const char *name,*desc;int span;int harm;int cap;float gaps;U ink;int price;} NVKSpec;
static const NVKSpec nvk_frames[]={
 {"Legacy pulse","Infinite baseline charge. Keep the original game loadout.",112,1,0,.33f,GOLD,0},
 {"Ferry Prism","Balanced tool with a steady beam and generous cell capacity.",162,2,9,.32f,0x94f1d7,175},
 {"Arc Seed","A fast, shallow-range arcade pulse; holds more charges.",112,1,12,.15f,0xffa187,210},
 {"Halo Bridge","A slow, long-span beam with a smaller charge reservoir.",230,3,5,.51f,0xbca9fa,310}
};
static const char *const nvk_lens[]={"Stock glass","Iris arc","Tide capacitor"};
static const char *const nvk_lens_desc[]={
 "Default glass. No stat changes.","Adds 34 game units of span; adds a brief recovery beat.","Adds two charge slots; shortens the next pulse beat."};
static const char *const nvk_cell[]={"Chime","Bloom","Echo"};
static const char *const nvk_cell_desc[]={
 "Mint beam. Steady pulse with stock reach.","Rose beam. Adds one Arcade impact and uses one extra charge.","Violet beam. Extends the span at the cost of one Arcade impact."};
static const int nvk_lens_cost[]={0,62,95},nvk_cell_cost[]={16,28,32};
static const U nvk_cell_ink[]={0x93f2d1,0xffa8a0,0xb6aaff};
/* Three frame bits and two lens bits. The starter frame and glass are free. */
static int nvk_cfg(int shift,int mask){return (nv.world[NVK_CONFIG]>>shift)&(U)mask;}
static void nvk_set(int shift,U mask,int n){nv.world[NVK_CONFIG]=(nv.world[NVK_CONFIG]&~(mask<<shift))|(((U)n&mask)<<shift);}
static int nvk_frame(void){return nvk_cfg(NVK_FRAME_BIT,3);}
static int nvk_glass(void){return min(2,nvk_cfg(NVK_LENS_BIT,3));}
static int nvk_kind(void){return min(2,nvk_cfg(NVK_CELL_BIT,3));}
static int nvk_owned(int bit){return bit<=0||(nv.world[NVK_BUYS]&(1u<<bit));}
static int nvk_load(void){return nvk_cfg(NVK_LOAD_BIT,15);}
static int nvk_reserve(int kind){return min(99,(nv.world[NVK_RESERVE]>>(8*kind))&255u);}
static void nvk_put(int kind,int n){int shift=8*kind;nv.world[NVK_RESERVE]=(nv.world[NVK_RESERVE]&~(255u<<shift))|((U)cl(n,0,99)<<shift);}
static void nvk_cycle_frame(int id){
 int cell=nvk_kind();
 nvk_put(cell,nvk_reserve(cell)+nvk_load());nvk_set(NVK_LOAD_BIT,15,0);
 nvk_set(NVK_FRAME_BIT,3,id);
}
static void nvk_cycle_cell(int id){
 int cell=nvk_kind();if(id==cell)return;
 nvk_put(cell,nvk_reserve(cell)+nvk_load());nvk_set(NVK_LOAD_BIT,15,0);
 nvk_set(NVK_CELL_BIT,3,id);
}
static int nvk_cap(void){return nvk_frames[nvk_frame()].cap+(nvk_frame()&&nvk_glass()==2?2:0);}
static void nvk_set_glass(int id){
 int old=nvk_load();nvk_set(NVK_LENS_BIT,3,id);
 if(old>nvk_cap()){nvk_put(nvk_kind(),nvk_reserve(nvk_kind())+old-nvk_cap());nvk_set(NVK_LOAD_BIT,15,nvk_cap());}
}
static void nvk_stats(int *range,int *damage,float *gap,U *ink){
 int frame=nvk_frame(),glass=nvk_glass(),kind=nvk_kind();
 if(!frame)return;
 *range=nvk_frames[frame].span+(glass==1?34:0)+(kind==2?45:0);
 *damage=max(1,nvk_frames[frame].harm+(kind==1?1:kind==2?-1:0));
 *gap=nvk_frames[frame].gaps+(glass==1?.07f:glass==2?-.04f:0.f);
 *ink=nvk_cell_ink[kind];
}
static int night_venue_apply_pulse(int *range,int *damage,float *cooldown,U *ink){
 if(!nvk_frame())return 1;
 int have=nvk_load(),used=nvk_kind()==1?2:1;
 if(have<used){p.shot=.45f;tell("Pulse charges low. Press L to load cells from reserve.");return 0;}
 nvk_set(NVK_LOAD_BIT,15,have-used);nvk_stats(range,damage,cooldown,ink);return 1;
}
static int nvk_fill(void){
 int k=nvk_kind(),old=nvk_load(),next=min(nvk_cap()-old,nvk_reserve(k));
 if(next<=0)return 0;
 nvk_put(k,nvk_reserve(k)-next);nvk_set(NVK_LOAD_BIT,15,old+next);return next;
}
static void night_venue_reload(U tap){
 if(!(tap&LOADOUT)||p.car>=0||sv.gear<0||!nvk_frame())return;
 if(nvk_fill()){p.shot=max(600,(int)(1000*p.shot))/1000.f;tell("Pulse cells loaded.");request_save();}
 else tell(nvk_load()>=nvk_cap()?"Pulse reservoir is full.":"No matching cells in reserve. Visit the Signal Cart.");
}
static void nvk_format(char *s,int frame,int glass,int variant,int loaded){
 int r=frame?nvk_frames[frame].span+(glass==1?34:0)+(variant==2?45:0):sv.gear>0?165:112;
 int d=frame?max(1,nvk_frames[frame].harm+(variant==1?1:variant==2?-1:0)):sv.gear>0?2:1;
 char *q=append(s,nvk_frames[frame].name);q=append(q," / ");q=append(q,nvk_lens[glass]);
 q=append(q," / ");q=append(q,nvk_cell[variant]);q=append(q,"  |  ");
 q=number(q,r);q=append(q," span  ");q=number(q,d);q=append(q," impact");
 if(frame){q=append(q,"  CHARGE ");q=number(q,loaded);q=append(q,"/");number(q,nvk_frames[frame].cap+(glass==2?2:0));}
}
