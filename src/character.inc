/* Palette and interchangeable silhouette layers shared by the editor and the street. */
enum{BUILD,SKIN,HAIR,HAIR_TONE,DETAIL,TOP,TOP_TONE,LEGS,LEG_TONE,SHOES,CHAR_FIELDS};
typedef struct{unsigned char part[CHAR_FIELDS];} Look;
typedef struct{const char*label;int count;const char*name[12];} Wardrobe;
static const Wardrobe wardrobe[]={
 {"BUILD",3,{"Lean","Regular","Broad"}},
 {"SKIN TONE",8,{"Porcelain","Peach","Sand","Warm tan","Copper","Umber","Deep brown","Espresso"}},
 {"HAIRSTYLE",10,{"Shaved","Crew cut","Side part","Undercut","Curls","Afro","Mohawk","Bob","Ponytail","Long"}},
 {"HAIR COLOR",8,{"Black","Brown","Chestnut","Copper","Blond","Silver","Violet","Mint"}},
 {"FACE / GEAR",7,{"None","Glasses","Shades","Stubble","Beard","Mask","Ear studs"}},
 {"TOP",6,{"Tee","Tank","Bomber","Hoodie","Track jacket","Work vest"}},
 {"TOP COLOR",12,{"Acid lime","Charcoal","Bone","Petrol","Rust","Indigo","Berry","Ochre","Mint","Scarlet","Denim","Violet"}},
 {"BOTTOMS",5,{"Slim jeans","Cargo pants","Shorts","Joggers","Work pants"}},
 {"BOTTOM COLOR",8,{"Slate","Ink","Stone","Olive","Navy","Wine","Lilac","Sand"}},
 {"SHOES",6,{"Black boots","White sneakers","Red high-tops","Tan boots","Mint trainers","Violet kicks"}}
};
static const U charSkin[]={0xf5d0bd,0xedb799,0xdba78a,0xc38966,0xa66448,0x885340,0x693e31,0x4d302b};
static const U charHair[]={0x293039,0x513733,0x744b3a,0xa75532,0xd9a547,0xbfc7d9,0x9960b8,0x68caa1};
static const U charTop[]={0xc6d979,0x46545a,0xe1d7c0,0x328c88,0xcb6746,0x6675aa,0xc05b88,0xc3a449,0x79d4ae,0xe25861,0x538ab8,0xa581d4};
static const U charLeg[]={0x42546b,0x282d3a,0x8a8991,0x667146,0x37436c,0x6d455f,0x987da4,0xab946c};
static const U charShoe[]={0x223540,0xe6e7da,0xbe485c,0xaf7850,0x5bbfa1,0x906acc};
static const Look defaultLook={{1,2,2,0,0,0,0,0,0,0}};
static int look_valid(const Look*l){for(int i=0;i<CHAR_FIELDS;i++)if(l->part[i]>=wardrobe[i].count)return 0;return 1;}
