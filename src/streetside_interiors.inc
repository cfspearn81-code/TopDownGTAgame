/* Street address interiors. These rooms use the same furniture sprite vocabulary
   as signed venues, with their own service stations, architecture and scenery.
   A room owns no street actors, funds or equipment. Its switch survives a visit. */
enum{WI_SCREEN=100,WI_DECK,WI_OVEN,WI_FABRIC,WI_RADIO,WI_GALLERY,WI_POTTERY,
     WI_PROJECTOR,WI_WASH,WI_BICYCLE,WI_AQUA,WI_STAGE,WI_WEIGHT,WI_TIMETABLE,
     WI_PRINT,WI_TELESCOPE,WI_PARCELS,WI_KEYS,WI_KITCHEN,WI_BOOK,WI_SEATS,WI_TABLE};
typedef struct{short t,x,y,w,h;} WiFixture;
typedef struct{
 const char *noun,*section,*action,*on,*off;
 U accent,floor;unsigned char finish;
 WiFixture plan[10]; /* first fixture is the interactive station */
}WiRole;
static const WiRole wiRoles[]={
 {"CINEMA","SCREEN ONE","Start the projector","The feature begins. Light spills across the screen.","The projectionist fades to black.",
  0xffb68b,0x463342,2,{{WI_PROJECTOR,348,300,80,42},{WI_SCREEN,280,150,400,29},{WI_SEATS,215,215,180,40},{WI_SEATS,556,215,180,40},{WI_SEATS,557,298,180,40},{INT_REEL,726,150,53,48}}},
 {"RECORDS","LISTENING BAR","Change the record","Side B: a local trio recorded live off Pier Nine.","Side A: the needle catches an old harbor ballad.",
  0xc59bf8,0x463946,0,{{WI_DECK,575,196,157,57},{INT_RACK,179,154,179,52},{INT_RACK,206,259,121,60},{INT_SOFA,595,341,142,44},{WI_GALLERY,360,151,86,33},{INT_REEL,724,273,53,48}}},
 {"BAKEHOUSE","PROOF & OVEN","Ring the oven bell","The baker opens a hatch. The next tray is ready.","The brass bell calls the next order.",
  0xfbc878,0x584c3f,1,{{WI_OVEN,204,161,211,87},{INT_DINER,174,301,190,45},{WI_TABLE,572,263,75,53},{WI_TABLE,690,316,71,48},{INT_COOLER,577,150,180,62},{INT_TRAY,577,375,112,29}}},
 {"TAILORS","CUTTING FLOOR","Unroll the pattern","The cutting table unrolls a coat pattern.","A roll of blue twill takes its place.",
  0xffb5af,0x51484b,0,{{WI_FABRIC,518,233,199,73},{INT_LOCKER,162,153,159,40},{INT_RACK,168,228,98,78},{INT_SOFA,187,345,136,42},{INT_TOOLS,577,153,148,43},{INT_GLASS,372,148,8,83}}},
 {"RADIO","ON-AIR STUDIO","Switch the feed","Studio Two goes live. The meters bounce on voice.","The transmitter switches back to the music feed.",
  0x8fe9cf,0x354457,2,{{WI_RADIO,356,202,162,52},{INT_GLASS,560,148,9,139},{INT_GLASS,575,295,131,8},{WI_STAGE,635,173,114,67},{INT_SOFA,169,298,134,49},{INT_LOCKER,178,153,119,45},{INT_REEL,679,348,73,48}}},
 {"GALLERY","EXHIBITION HALL","Change the exhibit light","Amber light reveals the second layer of the print.","Cool light returns to the harbor series.",
  0xfbcf9c,0x565550,1,{{WI_GALLERY,360,156,225,70},{WI_GALLERY,180,273,87,77},{WI_GALLERY,631,256,110,55},{INT_SOFA,315,315,99,32},{INT_SOFA,551,358,120,36},{INT_PLANT,723,155,36,36}}},
 {"CERAMICS","WHEEL ROOM","Turn the potter's wheel","The wheel turns. A bowl takes shape on the bat.","The potter stops the wheel to trim the foot.",
  0xe2ac82,0x59483d,0,{{WI_POTTERY,222,238,113,79},{WI_OVEN,180,152,159,55},{INT_TRAY,550,178,195,42},{INT_RACK,624,274,111,68},{WI_TABLE,345,364,79,38},{INT_PLANT,731,365,33,33}}},
 {"WASH HOUSE","SPIN FLOOR","Run the test cycle","The test drum spins behind the glass.","The drum slows; the cycle light goes dark.",
  0x8fd9f4,0x40545a,1,{{WI_WASH,199,163,199,73},{WI_WASH,564,154,171,65},{WI_TABLE,536,298,144,52},{INT_SOFA,189,350,157,44},{INT_LOCKER,168,251,73,64}}},
 {"CYCLE WORKS","REPAIR BAY","Spin the test wheel","The wheel spins freely on the truing stand.","The mechanic brakes the test wheel.",
  0xf8ad75,0x4c4640,1,{{WI_BICYCLE,554,229,184,78},{INT_TOOLS,190,157,178,58},{INT_LOCKER,697,151,75,48},{INT_CRATES,195,299,118,73},{WI_BICYCLE,337,161,98,57},{INT_TOOLS,599,359,124,37}}},
 {"AQUARIUM","TIDAL POOLS","Toggle the tide lamp","Blue nocturnal lighting shows a school near the glass.","The tank returns to daylight.",
  0x86eadf,0x315159,2,{{WI_AQUA,357,159,224,77},{WI_AQUA,178,242,137,78},{WI_AQUA,642,247,120,59},{INT_SOFA,535,351,126,35},{INT_FERNS,178,355,145,43},{INT_LAB,673,151,72,57}}},
 {"PLAYHOUSE","BLACK BOX THEATRE","Cue the stage lights","The house lights fall. The stage goes gold.","The rehearsal resets to its blue wash.",
  0xef99ad,0x483645,2,{{WI_STAGE,292,152,352,79},{WI_SEATS,183,273,191,43},{WI_SEATS,589,273,173,43},{WI_SEATS,173,355,161,38},{WI_SEATS,641,355,116,38},{INT_LOCKER,720,152,55,74}}},
 {"ATHLETIC CLUB","WEIGHT ROOM","Start the interval clock","The training clock switches to the work interval.","Recovery interval. The gym clock turns blue.",
  0xa7daa1,0x45494b,2,{{WI_WEIGHT,230,224,166,71},{WI_WEIGHT,540,171,161,65},{INT_LOCKER,175,151,161,38},{INT_CLINIC,650,303,110,48},{INT_SOFA,209,351,143,41}}},
 {"TRAM HALL","PASSENGER CONCOURSE","Check next departures","Next eastbound train: Pier Nine via Tide Market.","Next westbound train: Lantern Gate via Glass Mile.",
  0xffce74,0x484c51,1,{{WI_TIMETABLE,396,156,170,55},{WI_SEATS,203,242,147,36},{WI_SEATS,603,242,147,36},{WI_SEATS,203,328,147,36},{WI_SEATS,603,328,147,36},{INT_GLASS,170,147,9,61},{INT_LOCKER,667,152,96,45}}},
 {"PRINT SHOP","LETTERPRESS FLOOR","Pull a proof print","The press releases a new two-color harbor poster.","A black-and-white proof slides onto the tray.",
  0xf29a82,0x544743,0,{{WI_PRINT,243,214,175,70},{INT_RACK,172,151,128,39},{INT_SORTER,548,162,175,53},{WI_GALLERY,623,268,123,51},{INT_CRATES,185,337,119,52},{INT_TOOLS,554,363,154,37}}},
 {"SKY CLUB","OBSERVATION DECK","Rotate the telescope","The scope finds the lighthouse across the bay.","The scope swings back to the moon.",
  0xafb7ff,0x383f57,2,{{WI_TELESCOPE,409,174,147,78},{INT_CONSOLE,180,177,125,51},{INT_GLASS,182,263,117,8},{INT_GLASS,637,260,116,8},{INT_SOFA,613,324,134,49},{WI_GALLERY,220,339,96,43}}},
 {"PARCEL HALL","NEIGHBORHOOD POST","Ring for collection","The clerk stamps the outgoing mailbag for the harbor.","The next mailbag is tagged for Lantern Ward.",
  0x9ce5ce,0x44534d,1,{{WI_PARCELS,496,228,206,63},{INT_SORTER,175,160,169,62},{INT_CRATES,189,286,109,72},{INT_LOCKER,605,153,144,49},{INT_GLASS,329,300,8,83},{INT_REEL,701,348,56,48}}},
 {"ROOMING HOUSE","RESIDENTS' LOBBY","Read the message board","Pinned note: Sunday supper. Bring a chair, not money.","Pinned note: spare keys are with the night porter.",
  0xeca98b,0x574848,0,{{WI_KEYS,521,175,170,57},{INT_SOFA,202,291,139,53},{WI_TABLE,357,290,61,48},{INT_BED,176,157,129,57},{INT_WALL,331,147,8,78},{INT_PLANT,700,336,40,40},{INT_RACK,603,289,113,39}}},
 {"DINING ROOM","COUNTER SERVICE","Call the next order","The pickup lamp lights for the window table.","The pickup lamp changes to counter service.",
  0xfbb482,0x514940,1,{{WI_KITCHEN,193,157,218,72},{WI_TABLE,589,172,77,52},{WI_TABLE,677,257,70,50},{WI_TABLE,545,334,72,49},{INT_DINER,174,299,173,51},{INT_COOLER,679,151,89,66}}},
 {"READING ROOM","OPEN STACKS","Turn the atlas page","A foldout map shows the old ferry terminals.","The next page shows the streets before the seawall.",
  0xdcca93,0x514d41,0,{{WI_BOOK,382,251,141,54},{INT_RACK,174,156,185,47},{INT_RACK,561,156,191,47},{INT_RACK,174,257,105,53},{INT_RACK,647,257,103,53},{INT_SOFA,599,352,126,39},{INT_PLANT,217,360,34,34}}}
};
static int walkin_active=-1,wi_role,wi_n,wi_left,wi_right,wi_top,wi_dx,wi_mirror;
static float wi_x,wi_y,wi_angle;
static unsigned char wi_sw[LEN(buildings)];
static char wi_names[LEN(buildings)][48];
static InteriorProp wi_f[12];
static U wi_back[W*H],wi_seed;
static const char *const wi_brand[]={"SALTMARSH","BLUE HOUR","LOW TIDE","HARBOURLIGHT","NINTH ST.","MARINER","BLUEBIRD","COPPER","WEST WIND","TERN","BELL LANE","GREYCAP"};
static U wi_hash(U n){n=(n^(n>>16))*0x7feb352du;n=(n^(n>>15))*0x846ca68bu;return n^(n>>16);}
static int wi_index(int b){return (b*7+4)%LEN(wiRoles);}
static const char*storefront_sign(int i){return wiRoles[wi_index(i)].noun;}
static int tram_hall(int b){return wiRoles[wi_index(b)].plan[0].t==WI_TIMETABLE;}
static int tram_destination(int b){
 for(int offset=1;offset<nb;offset++){
  int stop=(b+offset)%nb;
  if(ordinaryDoorY[stop]&&tram_hall(stop))return stop;
 }
 return -1;
}
static int contract_room(int kind,int x,int y,int exclude){
 int role=kind==JOB_VAULT?WI_PARCELS:WI_RADIO;
 int best=-1;float distance=ROOM*ROOM*2.f;
 for(int i=0;i<nb;i++){
  if(i==exclude||!ordinaryDoorY[i]||wiRoles[wi_index(i)].plan[0].t!=role)continue;
  float d=d2(x-ordinaryDoorX[i],y-ordinaryDoorY[i]);
  if(d<distance){best=i;distance=d;}
 }
 return best;
}
static const char*walkin_name(int b){
 if(b<0||b>=nb)return "STREET DOOR";
 if(!wi_names[b][0]){char*q=append(wi_names[b],wi_brand[(b/LEN(wiRoles)+wi_index(b)*3)%LEN(wi_brand)]);q=append(q," ");append(q,wiRoles[wi_index(b)].noun);}
 return wi_names[b];
}
static void wi_layout(int b){
 wi_role=wi_index(b);const WiRole*r=wiRoles+wi_role;wi_seed=wi_hash((U)b+117u);
 wi_dx=(int)((wi_seed>>4)%31)-15;wi_mirror=wi_seed&1;
 wi_left=123+(int)((wi_seed>>9)%28);wi_right=W-wi_left;wi_top=124+(int)((wi_seed>>16)%13);
 wi_n=0;
 for(int i=0;i<LEN(r->plan)&&r->plan[i].w;i++){
  WiFixture a=r->plan[i];int x=a.x+wi_dx,y=a.y+(int)((wi_seed>>(i%6))%9)-4;
  if(wi_mirror)x=W-x-a.w;
  U t=i?mix(r->accent,(i+(wi_seed>>9))%3==0?0x647e7c:(i+(wi_seed>>9))%3==1?0x69566a:0x93765c,5):r->accent;
  wi_f[wi_n++]=(InteriorProp){x,y,a.w,a.h,a.t,t};
 }
}
#include "storefront_addresses.inc"
static int wi_solid(float x,float y){
 if(y<wi_top+12||y>431)return 1;
 if(x<wi_shell_edge((int)y,0)+12||x>wi_shell_edge((int)y,1)-12)return 1;
 for(int i=0;i<wi_n;i++){InteriorProp*f=wi_f+i;if(x>f->x-8&&x<f->x+f->w+8&&y>f->y-7&&y<f->y+f->h+7)return 1;}
 return 0;
}
static int wi_at_station(void){
 const InteriorProp*f=wi_f;float dx=wi_x< f->x?f->x-wi_x:wi_x>f->x+f->w?wi_x-(f->x+f->w):0;
 float dy=wi_y<f->y?f->y-wi_y:wi_y>f->y+f->h?wi_y-(f->y+f->h):0;return dx*dx+dy*dy<=34*34;
}
static int wi_at_door(void){return d2(wi_x-W/2,wi_y-430)<34*34;}
static void wi_draw(const InteriorProp*f,int enabled){
 if(f->type<WI_SCREEN){int_draw_prop(f,-1);return;}
 int x=f->x,y=f->y,w=f->w,h=f->h,m=x+w/2,n=y+h/2;
 U a=f->tint,dk=0x1d2934,pale=mix(a,PAPER,4),metal=0x929da6,ink=0x121f2a;
 int_solid_shape(x,y,w,h,mix(a,0x3a4450,5));
 switch(f->type){
 case WI_SCREEN:box(x+7,y+4,w-14,h-10,enabled?0xe9cf9e:0x818a94,dk);
  if(enabled){for(int i=0;i<5;i++)tri(m-45+i*22,y+h-7,m-17+i*22,y+8+i%3,m+i*22,y+h-7,0x596e7e);}break;
 case WI_DECK:
  for(int i=0;i<2;i++){int u=x+w*(1+2*i)/4;disc(u,n,h/3+3,ink);disc(u,n,h/3-1,0x434454);disc(u,n,h/3-3,ink);disc(u,n,6,enabled?GOLD:a);dot(u,n,PAPER);line(u+17,n-14,u+6,n,metal);}
  for(int j=0;j<3;j++)rect(m-5+j*5,y+12,2,h-24,metal);break;
 case WI_OVEN:
  for(int i=0;i<2;i++){int l=x+8+i*(w/2-3),ww=w/2-10;box(l,y+9,ww,h-21,0x362e34,metal);for(int t=8;t<ww-6;t+=13){disc(l+t,n,5,enabled?0xc4834f:0x946e54);line(l+t-2,n-2,l+t+2,n-2,GOLD);}rect(l+4,y+h-16,ww-8,2,pale);}break;
 case WI_FABRIC:rect(x+9,y+7,w-18,h-17,0xa7a89c);
  for(int i=0;i<4;i++){rect(x+13,y+11+i*10,w/3,6,i%2?0x998bb4:0x73829e);disc(x+13,n-19+i*10,3,0xd5c8aa);}
  for(int i=0;i<2;i++){int xx=m+i*21;tri(xx,y+12,xx-12,y+h-17,xx+16,y+h-23,enabled?PAPER:0x698499);line(xx,y+13,xx+3,y+h-25,dk);}break;
 case WI_RADIO:box(x+7,y+6,w-14,h-14,0x344354,dk);
  for(int i=0;i<8;i++){int xx=x+14+i*8;rect(xx,y+14,2,h-31,metal);rect(xx-2,y+16+(i*11+(enabled?7:0))%max(1,h-35),6,3,pale);}
  for(int i=0;i<12;i++){int ht=2+(i*7+(enabled?4:0))%15;rect(x+w-48+i*3,n+8-ht,2,ht,i<9?a:GOLD);}break;
 case WI_GALLERY:
  box(x+6,y+5,w-12,h-13,0xcbad79,0x675749);box(x+10,y+9,w-20,h-21,enabled?0xc98a74:0x8397a7,0xf3dfb1);
  disc(m+w/6,n-4,max(2,min(9,h/6)),GOLD);tri(x+12,y+h-14,m-4,y+14,m+w/5,y+h-14,0x395663);tri(m-w/8,y+h-14,m+w/6,n-4,x+w-13,y+h-14,0x587b79);break;
 case WI_POTTERY:disc(m,n,h/2-6,0x384754);disc(m,n,h/2-10,0xb69377);disc(m,n,enabled?16:11,0xca8c74);disc(m,n,enabled?11:7,0x805e58);for(int j=0;j<3;j++)disc(x+9+j*8,y+9,3,a);break;
 case WI_PROJECTOR:for(int i=0;i<2;i++){int u=x+18+i*27;disc(u,y+13,11,metal);disc(u,y+13,8,dk);for(int k=0;k<3;k++)disc(u+(int)(cs(k*2.1f)*4),y+13+(int)(sn(k*2.1f)*4),2,metal);}rect(x+14,n,w-29,12,dk);rect(x+w-20,n-2,11,10,enabled?GOLD:metal);break;
 case WI_WASH:for(int i=0;i<3;i++){int ww=(w-12)/3,u=x+5+i*ww,rr=min(ww/2-5,h/2-10);box(u,y+5,ww-2,h-13,0x8799a2,dk);disc(u+ww/2,n+2,rr,metal);disc(u+ww/2,n+2,rr-3,0x263b50);line(u+ww/2-rr/2,n+4,u+ww/2+rr/2,n-3,enabled?TEAL:0x73919d);rect(u+5,y+10,9,3,pale);}break;
 case WI_BICYCLE:{int rr=min(h/3-3,w/6),u=x+w/4,v=x+3*w/4;for(int j=0;j<2;j++){int c=j?v:u;disc(c,n+3,rr,ink);disc(c,n+3,rr-3,metal);disc(c,n+3,rr-4,dk);line(c-rr+3,n+3,c+rr-3,n+3,metal);line(c,n-rr+6,c,n+rr,metal);}line(u,n+3,m-3,n-12,a);line(m-3,n-12,m+4,n+4,a);line(m+4,n+4,u,n+3,a);line(m+4,n+4,v-8,n-12,a);line(v-8,n-12,v,n+3,pale);line(m-9,n-14,m,n-14,ink);break;}
 case WI_AQUA:box(x+5,y+5,w-10,h-13,enabled?0x21495f:0x427b80,0xa4c9c1);for(int j=0;j<8;j++){int u=x+14+(j*37)%(w-30),v=y+14+(j*19)%(h-32);tri(u-4,v,u+3,v-2,u+3,v+2,j%2?GOLD:TEAL);line(u-5,v-2,u-5,v+2,pale);}for(int j=10;j<w-9;j+=11)line(x+j,y+h-11,x+j+4,y+h-17-(j%7),0x729977);break;
 case WI_STAGE:rect(x+4,y+5,w-8,h-14,enabled?0x916956:0x5c5363);for(int j=0;j<7;j++){int off=j*4;rect(x+5+off,y+5,3,h-15-j*3,0x874555);rect(x+w-8-off,y+5,3,h-15-j*3,0x874555);}disc(m,n+4,5,ink);rect(m,n-13,2,17,metal);line(m-4,n-14,m+6,n-14,pale);break;
 case WI_WEIGHT:for(int j=0;j<2;j++){int v=y+15+j*(h-34);rect(x+13,v,w-26,3,metal);for(int k=0;k<4;k++){int ww=x+19+k*4;rect(ww,v-5-k,3,11+k*2,ink);rect(x+w-19-k*4,v-5-k,3,11+k*2,ink);}}box(m-12,n-5,24,10,enabled?0xdca47f:0x91b5c8,dk);break;
 case WI_TIMETABLE:box(x+6,y+5,w-12,h-14,ink,metal);for(int j=0;j<3;j++){int yy=y+11+j*10;for(int k=0;k<12;k++)rect(x+13+k*7,yy,3+(k+j)%4,3,k<8?GOLD:TEAL);}break;
 case WI_PRINT:rect(x+10,y+9,w/2,h-24,metal);for(int j=0;j<3;j++)box(x+15+j*12,y+13,9,h-32,0x465865,ink);box(m+7,y+12,w/2-21,h-30,enabled?0xd79980:PAPER,dk);tri(m+12,n+11,m+29,n-8,m+43,n+11,0x596b78);break;
 case WI_TELESCOPE:{int s=enabled?-1:1;line(m,n,m-18,n+22,metal);line(m,n,m+18,n+22,metal);line(m,n,m,n+26,pale);for(int j=-3;j<4;j++)line(m-19*s,n+11+j,m+25*s,n-16+j,metal);disc(m+25*s,n-16,6,a);break;}
 case WI_PARCELS:for(int j=0;j<4;j++){int u=x+10+j*(w-18)/4,v=y+12+j%2*5;box(u,v,27,20,0xbc9471,0x735e52);rect(u+12,v+1,4,17,0xe3c79d);rect(u+3,v+4,5,4,PAPER);}break;
 case WI_KEYS:box(x+7,y+5,w-14,h-14,0x96755e,dk);for(int j=0;j<2;j++)for(int k=0;k<6;k++){int u=x+16+k*(w-28)/6,v=y+12+j*14;disc(u,v,3,GOLD);line(u,v+1,u,v+7,pale);rect(u,v+6,4,2,pale);}break;
 case WI_KITCHEN:for(int j=0;j<4;j++){int u=x+19+j*21;disc(u,n,9,ink);disc(u,n,7,enabled?0xd6905a:0x777e7c);disc(u,n,4,dk);}box(m+15,y+9,w/2-26,h-21,metal,dk);rect(m+23,y+15,w/2-43,h-33,0x60747d);break;
 case WI_BOOK:case WI_TABLE:{box(x+5,y+5,w-10,h-13,0x987859,0x624e48);if(f->type==WI_BOOK){for(int j=0;j<2;j++){box(m-32+j*32,n-17,31,30,enabled?0xdbc99f:0xc7d4be,0x685c51);for(int k=0;k<5;k++)rect(m-27+j*32,n-12+k*4,16+k%3*3,1,0x948a75);}}else{disc(m,n,9,PAPER);disc(m,n,6,0xbac5c4);disc(m+15,n-3,4,metal);}break;}
 case WI_SEATS:for(int j=3;j<w-15;j+=23){box(x+j,y+5,20,h-14,0x89576a,dk);rect(x+j+3,y+9,14,5,0xac7480);rect(x+j+2,y+h-14,16,3,0x69485e);}break;
 }
}
#include "storefront_setdress.inc"
static void wi_bake(void){
 const WiFinish*f=wiFinish+wi_role;const WiAddress*ad=wiAddress+walkin_active;
 U*old=surface;surface=wi_back;int b=442;
 for(int y=0;y<H;y++)rect(0,y,W,1,mix(0x111d28,0x25313e,(H-y)*5/H));
 wi_shell(f,ad->edition);
 wi_wall(f,ad->edition);wi_inlay(f,ad->edition);
 const int nx=wi_f[0].x+wi_f[0].w/2,ny=wi_f[0].y+wi_f[0].h;
 int_lamp(nx,ny,78,f->light,2);
 for(int i=0;i<wi_n;i++)wi_service(wi_f+i,ad->edition,i,i==0||wi_f[i].type==WI_SCREEN?wi_sw[walkin_active]:0);
 box(nx-9,ny-4,18,4,DARK,0x62767d);rect(nx-6,ny-3,12,2,wi_sw[walkin_active]?GOLD:TEAL);
 int px=wi_plaque_x();
 box(px-11,408,22,13,0x664e3f,0xe5c68e);rect(px-6,412,12,1,0xe5c68e);rect(px-6,415,8,1,0xd1b383);
 box(W/2-36,b-15,72,16,mix(f->floor,0x2c3942,4),mix(f->floor,PAPER,2));
 for(int j=0;j<4;j++)rect(W/2-31,b-12+3*j,62,1,mix(f->floor,DARK,3));
 rect(W/2-33,b+1,66,4,wiRoles[wi_role].accent);
 center(W/2,b+8,"STREET EXIT",MUTED,0);surface=old;
}
static void walkin_open(int building){
 if(building<0||building>=nb||buildings[building].w<=0||buildings[building].h<=0||zone_is_building(building))return;
 walkin_active=building;wi_layout(building);wi_x=W/2;wi_y=416;wi_angle=-1.570796f;walking=0;scene=4;menu=-1;mapOn=0;
 wi_bake();
}
/* One ticket rule shared by gate, departure board and atlas. */
static const char*tram_block(void){
 if(p.heat>0)return "CLOSED: go outside and lose pursuit.";
 if(job.kind==JOB_TAXI&&taxi.phase>=TAXI_BOARDING&&taxi.phase<=TAXI_EXITING)return "CLOSED: finish your passenger fare.";
 if(job.kind>=0)return "CLOSED: finish or abandon your contract.";
 if(vdActive>=0&&!a1_can_ride())return "CLOSED: finish or close your listing.";
 if(sv.cash<5)return "SHORT FARE: need $5 clean cash on hand.";
 return 0;
}
static void tram_board(void){
 int destination=tram_destination(walkin_active);
 if(destination<0)return;
 const char*blocked=tram_block();
 if(blocked){tell(blocked);return;}
 sv.cash-=5;p.x=ordinaryDoorX[destination];p.y=ordinaryDoorY[destination];
 int parked=sv.owned&&d2(cars[0].x-ordinaryDoorX[walkin_active],cars[0].y-ordinaryDoorY[walkin_active])<230.f*230.f;
 walkin_open(destination);tell(parked?"Arrived. Your car stays at the departure end this session; M shows its marker.":"Arrived. Exit to the street. Your $5 ticket was paid from clean cash.");request_save();
}
static void walkin_tick(U keys,U tap){
 if(walkin_active<0||walkin_active>=nb){scene=1;return;}
 const float dt=1.f/60;
 if(tap&HELP){helpBack=4;scene=2;return;}
 if(tap&GFX)fastGfx=!fastGfx;
 if(p.noticeT>0)p.noticeT-=dt;
 if(tap&ESC){if(menu>=0){menu=-1;return;}active(PAUSE_MENU);makeMenu();return;}
 if(menu>=0){makeMenu();if(!nopt){menu=-1;return;}if(tap&UP)sel=(sel+nopt-1)%nopt;if(tap&DOWN)sel=(sel+1)%nopt;if(tap&(USE|ENTER))doOpt(opt[sel].action);return;}
 int x=!!(keys&RIGHT)-!!(keys&LEFT),y=!!(keys&DOWN)-!!(keys&UP);
 float v=(keys&BRAKE?WALKIN_RUN_STEP:WALKIN_WALK_STEP)*(x&&y?.707107f:1.f);walking=x||y;
 if(walking){wi_angle=look(x,y);footcycle=wrap(footcycle+.25f*FOOT_PACE);}
 if(!wi_solid(wi_x+x*v,wi_y))wi_x+=x*v;
 if(!wi_solid(wi_x,wi_y+y*v))wi_y+=y*v;
 p.secs+=dt;job_clock_tick(dt);
 if(vd_frame(keys,tap))tap&=~(USE|ENTER);
 if((tap&ENTER)&&tram_hall(walkin_active)&&wi_at_station()&&!wi_at_door()){tram_board();return;}
 if(tap&USE){
  if(wi_at_door()){if(storefrontCaption){p.noticeT=0;storefrontCaption=0;}
   int b=walkin_active;walkin_active=-1;scene=1;menu=-1;walking=0;
   p.x=ordinaryDoorX[b];p.y=ordinaryDoorY[b];p.a=1.570796f;by=placeNear();near=carNear();return;}
  if(wi_at_station()&&job_room_action(walkin_active)){job_use_room();return;}
  if(wi_at_station()&&tram_hall(walkin_active)){
   int to=tram_destination(walkin_active);if(to>=0){char msg[95];char*end=append(msg,"Next tram: ");append(end,walkin_name(to));tell(msg);storefrontCaption=1;}return;
  }
  if(wi_at_station()){wi_sw[walkin_active]^=1;const WiRole*r=wiRoles+wi_role;tell(wi_sw[walkin_active]?r->on:r->off);storefrontCaption=1;wi_bake();return;}
  if(wi_at_plaque()){tell(wiPlaque[walkin_active]);storefrontCaption=1;}
 }
}
/* Keep the shop's headline at display size when possible. One-pixel
 * tighter tracking saves Harbourlight Rooming House from the wallet. */
static int wi_title_width(const char*s,int*big,int*tracking){
 int w=textw(s,1),n=max(0,sl(s)-1),limit=(W-291)-22-12;
 *big=1;*tracking=0;
 if(w<=limit)return w;
 if(w-n<=limit){*tracking=1;return w-n;}
 *big=0;return textw(s,0);
}
static void wi_title(const char*s,U color){
 int big,tracking;wi_title_width(s,&big,&tracking);
 if(!tracking){textat(22,31,s,color,big);return;}
 int x=22;for(;*s;s++){char one[]={*s,0};textat(x,31,one,color,big);x+=textw(one,big)-tracking;}
}
static void walkin_paint(void){
 if(walkin_active<0||walkin_active>=nb)return;
 const WiRole*r=wiRoles+wi_role;
 /* Tight copy compiles to vector stores; the heavy scenery bakes once per visit. */
 for(int i=0;i<W*H;i++)pixels[i]=wi_back[i];
 person((int)wi_x,(int)wi_y,0x56baae,walking?(int)(sn(footcycle)*4):0,wi_angle,4,1);
 char ad[72];char*q=number(ad,101+walkin_active*7);q=append(q," / ");q=append(q,venue_address(walkin_active));q=append(q,"   /   ");append(q,r->section);
 draw_day_clock(5);textat(DAY_BADGE_LABEL_X,9,ad,MUTED,0);wi_title(walkin_name(walkin_active),r->accent);draw_wallet(W-291,10,270);
 const InteriorProp*f=wi_f;int sx=f->x+f->w/2,sy=f->y-10;
 const char *task=job_room_action(walkin_active);
 U marker=task?PINK:r->accent;
 if(wi_at_station()){disc(sx,sy,10,0x23343d);center(sx,sy-10,"E",marker,0);}
 else{disc(sx,sy,4,DARK);disc(sx,sy,2,marker);}
 if(tram_hall(walkin_active)){
  int to=tram_destination(walkin_active);const char*blocked=tram_block();char card[147];
  char*end=append(card,walkin_name(walkin_active));end=append(end,"  >  ");append(end,walkin_name(to));
  box(19,132,730,68,0x16242b,0x837b60);textat(30,133,card,GOLD,0);
  textat(30,154,to==69?"LANTERN WARD / West seawall, near West Wind Bakehouse":"EAST FREIGHT / East freight grid, near Wrecker Yard",PAPER,0);
  end=append(card,"$5 CLEAN CASH   /   ");append(end,blocked?blocked:"OPEN: use ENTER at the departures board.");textat(30,175,card,blocked?PINK:TEAL,0);
 }
 rect(16,477,W-32,1,0x475a65);
 const char*hint=wi_at_door()?"Return to street":
     wi_at_station()?(task?task:tram_hall(walkin_active)?(tram_block()?tram_block():"Departures    [ENTER] Ride tram / $5 clean cash"):r->action):task?job_objective():"Walk to the lit station";
 if(!wi_at_door()&&!wi_at_station()&&wi_at_plaque())hint="Read the address plaque";
 textat(23,484,(wi_at_station()||wi_at_door()||wi_at_plaque())?"[E]":"WASD",r->accent,0);textat(81,484,hint,PAPER,0);
 if(p.noticeT>0)textat(23,509,notice,notice_color(),0);else if(saveFlash>0)textat(23,509,"Saved.",CASH_GREEN,0);else if(saveFlash<0)textat(23,509,"Save failed.",CASH_RED,0);
 if(menu>=0&&scene!=2){paint_active_menu();draw_wallet(W-291,10,270);}
}
