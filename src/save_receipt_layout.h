#ifndef DC_SAVE_RECEIPT_LAYOUT_H
#define DC_SAVE_RECEIPT_LAYOUT_H

/* The two supported 1,044-byte Rev11 predecessors shared four garage
 * words. The original receipt tag unambiguously distinguishes a receipt
 * from legal Woodline flags (which use only the low 20 bits). */
enum {
 DC_SAVE11_FAMILY_AT=968, DC_SAVE11_OLD_SEIZURE_AT=628,
 DC_SEIZURE_LEGACY_META=49, DC_SEIZURE_LEGACY_CLEAN=50,
 DC_SEIZURE_LEGACY_DIRTY=51, DC_SEIZURE_LEGACY_PENDING=52,
 DC_SEIZURE_META=2, DC_SEIZURE_CLEAN=3, DC_SEIZURE_DIRTY=62, DC_SEIZURE_PENDING=63,
 RR_MAGIC=0x52560000u, RR_TIME_BITS=11, RR_TIME_MASK=2047u
};
static inline int dc_legacy_receipt_marker(unsigned m){
 unsigned kind=(m>>RR_TIME_BITS)&3u;
 return (m&0xffffe000u)==RR_MAGIC&&(kind==1||kind==2)&&(m&RR_TIME_MASK)<1440u;
}
#endif
