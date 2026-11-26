/* An authored car-space raster, not a second handling shape. Direction,
 * headlights, damage, registration marker and world anchor come from Car. */
enum {MODEL_CAR_X=32,MODEL_CAR_Y=19,MODEL_CAR_UNIT=1024};
typedef struct {
 signed char noseStart,backGlass,roofBack,roofFront,frontGlass,axleRear,axleFront;
 unsigned char tailWidth,noseWidth;
 U glass,roofTrim;
} ModelCar;
static const ModelCar modelGarage[]={
 { 7,-11,-7, 1,5,-10,10,5,6,0x6ab4b6,0x456d71}, /* Mote hatch */
 { 7,-10,-6, 2,6,-10,10,6,5,0x7c9dbf,0x6272a7}, /* Vectra saloon */
 {10, -9,-7, 5,8,-11,11,7,6,0x93aab7,0x9a8367}, /* Pallas touring */
 { 4, -9,-6,-1,3,-10,11,7,3,0x8996ad,0x893f6b}, /* Nightjar wedge */
 {11,-14,-9, 3,7,-10,10,7,7,0x86a7a8,0x79916c}, /* Courier 4 van */
};
static U modelGaragePixels[2][LEN(modelGarage)][MODEL_CAR_Y][MODEL_CAR_X];
static int modelGarageReady;
static int model_car_width(const ModelCar*m,int u){
 if(u<=-15)return m->tailWidth;
 if(u<=-12)return (m->tailWidth+8)/2;
 if(u>=14)return m->noseWidth;
 if(u>m->noseStart)return (m->noseWidth+8)/2;
 return 8;
}
static U model_car_pixel(int model,int u,int v,int wrecked){
 const ModelCar*m=modelGarage+model;
 U paint=specs[model].c,ink=0x101a25;
 int av=v<0?-v:v,half=model_car_width(m,u);
 int wheel=(u>=m->axleRear-2&&u<=m->axleRear+2)||(u>=m->axleFront-2&&u<=m->axleFront+2);
 if(av>half)return wheel&&av<=9?(av==9?ink:0x354454):0;
 if(wrecked)paint=0x414d53;
 U skin=v<0?mix(paint,PAPER,1):mix(paint,0x174052,2);
 if(av==half||u==-16||u==15)return v<0?mix(skin,ink,2):mix(skin,ink,5);
 if(av==half-1&&u>-14&&u<13)return v<0?mix(skin,PAPER,2):mix(skin,ink,2);
 if(u==14&&av<=4)return 0x28404b; /* grille, separate from the live lamps */
 if(u==-15&&av<=4)return mix(paint,ink,4);
 if(model==3&&av<=1&&(u>m->frontGlass||u<m->backGlass))return wrecked?0x565459:0xd1beb0;
 if(model==3&&u>=7&&u<=11&&av>=3&&av<=4)return mix(paint,ink,5);
 if(u>m->frontGlass||u<m->backGlass){
  if(model==2&&av>=4&&av<=5)return mix(skin,PAPER,2);
  if(model==4&&u==11&&av<4)return 0x334751;
  if(u==10&&av<=3&&model!=3)return mix(skin,ink,2);
  return skin;
 }
 U glass=wrecked?0x52606a:m->glass;
 U deep=mix(glass,0x1c3546,5);
 if(u>m->roofFront||u<m->roofBack){
  if(av>=5)return deep;
  if(wrecked&&v==(u%3))return 0x919b93;
  if(u==m->frontGlass&&av<4)return mix(glass,PAPER,wrecked?1:3);
  if(v==1-u%5&&u>m->roofFront)return mix(glass,PAPER,2);
  return u>m->roofFront?glass:mix(glass,0x244355,2);
 }
 if(av>=5)return u==m->roofBack+1||u==m->roofFront-1?0x182d37:deep;
 if(av==4)return v<0?mix(skin,PAPER,2):mix(skin,ink,3);
 U roof=wrecked?0x424850:mix(skin,m->roofTrim,3);
 if(model==4&&u<1){
  if((u+12)%3==0)return mix(roof,ink,3); /* cargo roof ribs */
  return v==-3?mix(roof,PAPER,2):roof;
 }
 if(model==0&&u==m->roofBack&&av<2)return mix(roof,PAPER,2);
 if(model==1&&u<=0&&u>=-4&&av<=2)return v<0?0x506b81:0x314958; /* tinted sunroof */
 if(model==2&&av==3)return mix(roof,PAPER,3); /* twin roof rails */
 if(model==3&&av<=1)return wrecked?0x545b60:0xe1c2ab;
 return roof;
}
static void model_car_bake(void){
 if(modelGarageReady)return;
 for(int d=0;d<2;d++)for(int i=0;i<LEN(modelGarage);i++)
  for(int y=0;y<MODEL_CAR_Y;y++)for(int x=0;x<MODEL_CAR_X;x++)
   modelGaragePixels[d][i][y][x]=model_car_pixel(i,x-16,y-9,d);
 modelGarageReady=1;
}
static int model_car_fixed(float value){
 return (int)(value*MODEL_CAR_UNIT+(value>=0?.5f:-.5f));
}
static void model_car_stock(Car*c,int x,int y){
 float ca=cs(c->a),sa=sn(c->a),ac=absf(ca),as=absf(sa);
 int hx=(int)(17*ac+10*as)+1,hy=(int)(17*as+10*ac)+1;
 if(x+hx+3<clipLeft||x-hx>=clipRight||y+hy+5<clipTop||y-hy-18>=clipBottom)return;
 model_car_bake();
 /* Painted floor shadow stays the same projection on both FX tiers. */
 int ax[4],ay[4];
 static const int vx[]={-15,14,14,-15},vy[]={-7,-7,7,7};
 for(int i=0;i<4;i++){ax[i]=x+3+(int)(vx[i]*ca-vy[i]*sa);ay[i]=y+5+(int)(vx[i]*sa+vy[i]*ca);}
 tri(ax[0],ay[0],ax[1],ay[1],ax[2],ay[2],0x10191f);tri(ax[0],ay[0],ax[2],ay[2],ax[3],ay[3],0x10191f);
 /* Fixed point inverse transform once per visible row. The transparent
  * corners supply the actual silhouettes without redundant fill passes. */
 int l=max(max(0,clipLeft),x-hx),r=min(min(rw,clipRight),x+hx+1);
 int top=max(max(0,clipTop),y-hy),bottom=min(min(rh,clipBottom),y+hy+1);
 int co=model_car_fixed(ca),si=model_car_fixed(sa);
 const U (*body)[MODEL_CAR_X]=modelGaragePixels[c->cond<=0][c->type];
 for(int yy=top;yy<bottom;yy++){
  int u0=(l-x)*co+(yy-y)*si+(16*MODEL_CAR_UNIT+MODEL_CAR_UNIT/2);
  int v0=(yy-y)*co-(l-x)*si+(9*MODEL_CAR_UNIT+MODEL_CAR_UNIT/2);
  U*dst=surface+yy*rw;
  for(int xx=l;xx<r;xx++,u0+=co,v0-=si){
   if((U)u0>=MODEL_CAR_X*MODEL_CAR_UNIT||(U)v0>=MODEL_CAR_Y*MODEL_CAR_UNIT)continue;
   U color=body[v0/MODEL_CAR_UNIT][u0/MODEL_CAR_UNIT];if(color)dst[xx]=color;
  }
 }
 /* Dynamic lamp colours, size, beam emission and ownership marker remain
  * the established gameplay cues. Draw them after the new bodywork. */
 int bulbs=road_lamps(c);
 for(int i=0;i<2;i++){int k=i?5:-5;int x1=x+(int)(13*ca+k*sa),y1=y+(int)(13*sa-k*ca),x2=x-(int)(14*ca+k*sa),y2=y-(int)(14*sa-k*ca);
  disc(x1,y1,2,c->cond?0xffe0a3:0x656764);if(c->cond)dot(x1,y1,0xffebbf);
  disc(x2,y2,bulbs&1?2:1,bulbs&1?0xff5148:c->cond?0xa33e42:0x60383c);
  if(bulbs&2)dot(x-(int)(14*ca+(k>0?3:-3)*sa),y-(int)(14*sa-(k>0?3:-3)*ca),0xe2faff);
 }
 if(c->cond>0&&c->cond<45){disc(x-2,y-3,3,0x303640);if(c->cond<23){disc(x-3-(int)(anim*8)%7,y-8,3,0x4a535b);}}
 if(c->owner==1&&p.car<0){line(x-4,y-18,x,y-14,LIME);line(x,y-14,x+4,y-18,LIME);}
}
