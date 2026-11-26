#define DC_STREET_ADDRESS_PLAQUES 1
/* Each authored address has a trade-specific installation and an architectural plan.
   The plan number selects a cutaway shell; edition selects an installation within
   that trade, not a random color. Keep addresses keyed to the street-building id. */
typedef struct {const char*feature;unsigned char edition,shell;} WiAddress;
static const WiAddress wiAddress[LEN(buildings)]={
 [0]={"MARINE FREQUENCY",0,3},[1]={"HOME COURT",0,1},[2]={"THE MAP ARCHIVE",0,5},[3]={"TERRACOTTA WORKS",0,4},
 [8]={"THE FITTING SALON",0,2},[9]={"THE RED CURTAIN",0,1},[10]={"THE SUNRISE COUNTER",0,6},[11]={"THE MONOLITH",0,7},
 [13]={"THE STAR PALACE",0,8},[14]={"COIN & BUBBLE",0,3},[17]={"THE KELP FOREST",0,5},[18]={"THE OLD BRASS LOBBY",0,6},
 [19]={"MIDNIGHT SESSIONS",1,7},[26]={"LETTER DEAD DROP",0,2},[27]={"THE PATTERN ARCHIVE",1,4},[28]={"FOOTLIGHT REVUE",1,8},
 [29]={"THE PIE WINDOW",1,0},[30]={"BALANCING ACT",1,2},[31]={"EASTBOUND / 01",0,4},[34]={"THE MOON TRANSIT",0,5},
 [35]={"THE BRICK OVEN",0,4},[36]={"THE JELLY DRIFT",1,8},[37]={"THE PALM COURT",1,5},[38]={"STORM WATCH",2,4},
 [39]={"SPARRING NIGHT",1,8},[42]={"THE WOODCUT HOUSE",0,3},[43]={"DUBPLATE CELLAR",0,7},[44]={"TRACK BIKE PIT",0,1},
 [45]={"THE SORTING BELT",1,6},[46]={"THE SILK ROOM",2,5},[47]={"IN THE ROUND",2,5},[48]={"THE OYSTER BAR",2,3},
 [49]={"FOLDED HORIZON",2,4},[52]={"THE FOLDING HALL",1,6},[53]={"THE SOLAR TRANSIT",1,8},[54]={"THE BRAIDED LOAF",1,6},
 [55]={"THE RAY LAGOON",2,7},[56]={"BOARDWALK ROOMS",2,3},[57]={"THE SIGNAL BRIDGE",3,1},[58]={"THE RUNNING LANES",2,4},
 [59]={"THE PAPER GARDEN",1,2},[60]={"THE BLUE GLAZE",1,6},[61]={"THE TWO-COLOR PRESS",1,0},[62]={"THE SLEEVE GALLERY",1,8},
 [63]={"THE TOURING SHED",1,3},[64]={"THE AIRMAIL DEPOT",2,0},[65]={"THE SAILMAKERS",3,8},[66]={"THE SHADOW STAGE",3,3},
 [67]={"THE NIGHT DINER",3,4},[68]={"LUNAR MOBILES",3,5},[69]={"HARBOR LINE / 02",1,1},[70]={"THE SILVER MATINEE",1,0},
 [71]={"THE SOAP BATHS",2,8},[72]={"THE STAR ALMANAC",2,2},[73]={"THE PASTRY WINDOW",2,7},[74]={"THE SHOAL WALK",3,4},
 [75]={"THE TILE FOYER",3,0},[76]={"THE RECORDING CHOIR",4,0},[77]={"THE CIRCUIT",3,3},[78]={"THE NIGHT LIBRARY",2,7},
 [79]={"THE VASE GARDEN",2,5},[80]={"THE TYPE FOUNDRY",2,6},[81]={"THE DANCE PRESSING",2,1}
};
/* Continuous stepped / curved boundaries around the originally authored service
   stations. The street entrance and every existing interaction anchor stay put. */
static int wi_shell_edge(int y,int right){
 const WiAddress*b=wiAddress+walkin_active;int u=max(0,min(316,y-wi_top)),v=316-u;
 int far=(right!=wi_mirror),w=0;
 switch(b->shell){
 case 0:w=far?u<154?75:15:u<70?20:u<235?76:15;break; /* offset shop wings */
 case 1:w=u<48?u+18:u<215?67:max(4,67-(u-215));break; /* bevelled hall */
 case 2:w=far?(u<153?86:0):(u<153?0:74);break; /* dogleg */
 case 3:w=far?max(2,76-max(0,(u-100)/2)):(u<120?4:74);break; /* workshop annex */
 case 4:w=far?(u<107?6:u<202?45:86):(u<107?84:u<202?45:6);break; /* stepped bays */
 case 5:w=8+(int)(77.f*sn(u*(3.14159f/316)));break; /* bowed conservatory */
 case 6:w=far?(u<170?78:12):max(2,72-u/4);break; /* long counter / recess */
 case 7:w=far?max(0,83-u/3):max(0,76-v/3);break; /* clipped gallery */
 default:w=(u<67||u>252)?9:68;break; /* stage box */
 }
 /* Cut back the two front corners only below any furniture. This creates a
    recessed vestibule without narrowing the station-tour's transverse aisle. */
 if(y>406&&wi_role!=4&&wi_role!=15){
  int lip= max(0,(y-406)*(b->shell%3+2)-max(0,(b->edition%3)*8));
  if((b->shell&1)||far)w-=lip;
 }
 return right?wi_right+w:wi_left-w;
}
/* These visitor notes describe what is physically on display at that address. */
static const char*const wiPlaque[LEN(buildings)]={
 [0]="The marine desk still signs off with the three harbor chimes.",[1]="The maple court was salvaged from a neighborhood school.",[2]="The atlas desk holds hand-inked charts from the old ferry company.",[3]="Every earth-red vessel was turned from the same local clay.",
 [8]="The fitting salon kept its first opening-day dress on the rail.",[9]="The house curtain was stitched by the tailors across town.",[10]="The breakfast counter still uses the old sunrise plate design.",[11]="The standing slabs are cut from a single pier foundation.",
 [13]="The star marquee once advertised silent movies and a live organ.",[14]="The first brass token from this laundry is set into the wall.",[17]="The kelp rises and falls in a slow artificial tide.",[18]="Each brass key once hung above a handwritten resident's name.",
 [19]="Midnight Sessions records a local act after the last tram.",[26]="Undeliverable postcards wait behind the letter counter.",[27]="Pattern makers pin one corner of every finished coat pattern here.",[28]="The footlights are arranged for a chorus line, not an orchestra.",
 [29]="The chalkboard changes to match the pies in the window.",[30]="The painted shapes balance around a single narrow pivot.",[31]="Follow the platform stripe to the eastbound departure board.",[34]="The moon transit is tracked against the numbered meridian ring.",
 [35]="The bread oven's brick arch predates the rest of this block.",[36]="Bell-shaped jellies drift across the dark viewing glass.",[37]="The glass palm court was once an outdoor courtyard.",[38]="The weather desk records each approaching front on paper.",
 [39]="Sparring nights start when the corner lights turn on.",[42]="The woodcut marks come from the shop's first hand-carved block.",[43]="The cellar shelves hold one-off pressings from the harbor clubs.",[44]="The painted pit lines follow the shape of the old track.",
 [45]="Stripes on the sorting floor mark the outbound parcel lanes.",[46]="Silk panels from four seasons hang in the showroom.",[47]="The audience sits around the performance, not just in front.",[48]="The oyster counter opens before the fishing boats get home.",
 [49]="The folded landscape changes shape as you walk past.",[52]="The folding hall uses the same blue tile as the original baths.",[53]="The sun dial traces the roofline at the summer solstice.",[54]="The braided loaf is the morning baker's house signature.",
 [55]="The lagoon rays glide past the timber viewing walk.",[56]="The residents kept the boardwalk floor when the lobby was enclosed.",[57]="The signal desk monitors both ends of the harbor bridge.",[58]="Sprinters use the painted lanes for short indoor starts.",
 [59]="The reading room binds its botanical notebooks in green cloth.",[60]="Test bowls are fired in a line from pale blue to cobalt.",[61]="The second press lays a warm color over the first impression.",[62]="Each sleeve on the wall was designed for a local band.",
 [63]="The wheel rack doubles as a route board for the touring club.",[64]="The airmail desk sorts parcels by color before the last collection.",[65]="The sail loft still cuts its largest panels flat on the floor.",[66]="The stage lamps throw silhouettes across the rear curtain.",
 [67]="The late shift keeps the booths lit long after supper.",[68]="The suspended discs turn above a pale gallery floor.",[69]="The harbor branch boards from the platform behind the gate line.",[70]="The silver screen shows a mountain skyline before the feature.",
 [71]="The bubble tiles were made for this wash house, one batch at a time.",[72]="The roof windows frame the same stars printed in the club almanac.",[73]="The pastry counter opens with the first cream-filled shells.",[74]="Silver shoals gather at the bright end of the viewing tank.",
 [75]="The tiled entry was laid when this was still a railway hotel.",[76]="The choir lines up in an arc around the studio microphone.",[77]="Circuit training rotates from the racks to the marked floor stations.",[78]="Readers leave their lamps on until the night bell.",
 [79]="A row of differently shaped vessels records the potters' signatures.",[80]="The old type cabinet sorts letters by size before a press run.",[81]="Weekend dancers follow the rings cut into the listening floor."
};
static int wi_plaque_x(void){return wi_mirror?414:546;}
static int wi_at_plaque(void){return d2(wi_x-wi_plaque_x(),wi_y-414)<=20*20;}
