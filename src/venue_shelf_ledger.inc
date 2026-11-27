/* Rev11 world[23] contains the last THREE private shelf grades: 7 bits each.
 * Crossroads Street Life accidentally used the whole word as Bryn's fee.
 * Keep stock in its original bits, encode deferred fees in disjoint high bits.
 * Keep normal immediate payments byte-compatible; a stock signature that is
 * indistinguishable from an old raw fee receives a stock tag on a new write. */
#ifndef DC_VENUE_SHELF_LEDGER
#define DC_VENUE_SHELF_LEDGER
enum { NCR_OWED_WORD=23 };
#define VENUE_STOCK_MASK 0x001fffffu
#define VENUE_LEDGER_TAG 0x40000000u
#define VENUE_FEE_BITS 0x30000000u
_Static_assert(!(VENUE_STOCK_MASK & (VENUE_LEDGER_TAG|VENUE_FEE_BITS)), "Loft stock and promoter fees overlap");
static int venue_ambiguous(U raw){return raw==145u||raw==165u;}
static U venue_stock(void){
 U raw=nv.world[NCR_OWED_WORD];
 /* 115 cannot be a legal first shelf quantity (each is capped at 99). */
 return raw==115u?0u:raw&VENUE_STOCK_MASK;
}
static int venue_fee_token(void){
 U raw=nv.world[NCR_OWED_WORD];
 if(raw==115u)return 1;
 if((raw&~(VENUE_STOCK_MASK|VENUE_FEE_BITS))!=VENUE_LEDGER_TAG)return 0;
 return (int)((raw&VENUE_FEE_BITS)>>28);
}
static void venue_split_write(U shelf,int fee){
 shelf&=VENUE_STOCK_MASK;
 nv.world[NCR_OWED_WORD]=shelf|(fee||venue_ambiguous(shelf)?VENUE_LEDGER_TAG:0u)|((U)fee<<28);
}
#endif
