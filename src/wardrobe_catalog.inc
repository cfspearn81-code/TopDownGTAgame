/* Extension data is deliberately independent of the ten-byte legacy Look.
 * A zero extension keeps every original drawing path and profile byte. */
enum{CW_HAT,CW_HAT_TONE,CW_TRIM,CW_ACCESSORY,CW_HAIR,CW_FACE,CW_OUTERWEAR,CW_FOOT_TONE,CW_N};
enum{CW_WORD=1,CW_SHIFT=2,OPEN_CITY_WARDROBE_SCENE=14,CW_PAUSE_OPTION=189};
#define CW_VALUE_MASK 0x01ffffffu
#define CW_SAVE_MASK (CW_VALUE_MASK<<CW_SHIFT)
_Static_assert((CW_SAVE_MASK&3u)==0,"Wardrobe must not overlap residence deed or renovation");
typedef struct{const char*label,*caption;unsigned char shift,width,count;const char*name[12];} WardExtra;
static const WardExtra wardExtra[CW_N]={
 {"HEADWEAR","Distinct fitted shapes. The brim follows your facing.",0,4,10,
  {"None","Signal cap","Fisher knit","Bucket hat","Broad fedora","Newsboy cap","Dock helmet","Storm hood","Bandanna","Sun brim"}},
 {"HAT COLOR","Main material of the chosen cap, brim or hood.",4,4,12,
  {"Black","Pearl","Teal","Oxblood","Indigo","Saffron","Forest","Lavender","Sky blue","Clay","Pink","Steel"}},
 {"ACCENT COLOR","Trim, accessories, and the details of tailored layers.",8,4,12,
  {"Copper","Pearl","Teal","Oxblood","Indigo","Saffron","Forest","Lavender","Sky blue","Clay","Pink","Steel"}},
 {"ACCESSORY","Pick one separate accessory. None leaves the layer open.",12,3,8,
  {"None","Fine chain","Harbor scarf","Ear cuff","Crossbody bag","Enamel pin","Fingerless gloves","Shoulder shawl"}},
 {"HAIR CUT +","Adds a cut without changing your foundation hair or color.",15,2,4,
  {"Use foundation","Long twists","Tapered waves","Twin buns"}},
 {"FACIAL DETAIL +","Independent of foundation eyewear and facial gear.",17,2,4,
  {"None","Small scar","Freckles","Moustache"}},
 {"OUTERWEAR","Choose a tailored layer over your foundation shirt.",19,3,7,
  {"Use foundation","Wool coat","Leather jacket","Utility harness","Rain shell","Striped knit","Shearling"}},
 {"SHOE ACCENT","Highlights the soles and uppers without replacing the shoe.",22,3,8,
  {"Original","Pearl","Teal","Indigo","Saffron","Forest","Clay","Pink"}}
};
static const U wardDyes[]={0x202a3c,0xf5e6cb,0x64e3d0,0xa74658,0x6c78c6,0xf5ba54,0x628968,0xac9ae7,0x76c5ed,0xbe7154,0xea8bb2,0x9eb2bf};
static const U wardAccents[]={0xe7ad72,0xf5e6cb,0x64e3d0,0xb95768,0x8796e5,0xf5ba54,0x82a785,0xc7b8f3,0x8ed6f4,0xda9773,0xffa6c9,0xb8ced9};
static const U wardSole[]={0,0xffe9d3,0x67e9ca,0x8799ed,0xf8c460,0x8bb68c,0xdb8e65,0xff9cbe};
static int ward_part(U bits,int field){
 const WardExtra*f=wardExtra+field;U value=(bits>>f->shift)&((1u<<f->width)-1u);
 return value<f->count?(int)value:0;
}
static U ward_set(U bits,int field,int value){
 const WardExtra*f=wardExtra+field;U mask=((1u<<f->width)-1u)<<f->shift;
 return (bits&~mask)|((U)cl(value,0,f->count-1)<<f->shift);
}
static U ward_valid(U bits){
 U out=0;for(int i=0;i<CW_N;i++)out=ward_set(out,i,ward_part(bits,i));
 return out;
}
static const U wardWrap=0x282b3e;
static int ward_hat(U bits){return ward_part(bits,CW_HAT);}
static U ward_hat_color(U bits){return wardDyes[ward_part(bits,CW_HAT_TONE)];}
static U ward_trim_color(U bits){return wardAccents[ward_part(bits,CW_TRIM)];}
