/* A vehicle keeps the legacy integer condition and a remainder of 0..2
 * thirds of a point. One raw crash point now consumes exactly two thirds;
 * 150 raw crash points, rather than 100, exhaust a pristine vehicle.
 * Taxi ride quality and collision impulse still use unscaled severity. */
enum{VEHICLE_WEAR_WORD=61,VEHICLE_CONDITION_MAX=300};
static int vehicle_wear(const Car*c){return c->cond>0?cl(c->wear,0,2):0;}
static int vehicle_hp(const Car*c){return 3*cl(c->cond,0,100)-vehicle_wear(c);}
static void vehicle_set_health(Car*c,int hp){
 hp=cl(hp,0,VEHICLE_CONDITION_MAX);
 c->cond=(hp+2)/3;c->wear=3*c->cond-hp;
}
static void vehicle_impact(Car*c,int raw){
 vehicle_set_health(c,vehicle_hp(c)-2*cl(raw,0,VEHICLE_CONDITION_MAX));
}
static int vehicle_saved_hp(int t){
 int hp=3*cl(sv.carCond[t],0,100);
 /* A prior career has zero bits and needs no migration. Unset, wrecked
  * or invalid 0b11 fields never introduce phantom points. */
 U wear=(nv.garage[VEHICLE_WEAR_WORD]>>(t*2))&3u;
 return hp>0&&wear<3?hp-(int)wear:hp;
}
static void vehicle_store_health(int t,const Car*c){
 unsigned shift=(unsigned)t*2u;
 sv.carCond[t]=c->cond;
 nv.garage[VEHICLE_WEAR_WORD]=(nv.garage[VEHICLE_WEAR_WORD]&~(3u<<shift))|((U)vehicle_wear(c)<<shift);
}
static void vehicle_condition_text(char*label,const Car*c){
 int hp=vehicle_hp(c);
 char*q=number(label,hp/3);
 if(hp%3){*q++='.';*q++=hp%3==1?'3':'7';*q=0;}
}
