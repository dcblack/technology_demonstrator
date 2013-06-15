#ifndef DEV_UTIL_H
#define DEV_UTIL_H

#include "dev_hls.h"

extern Addr_t reg[STATUS+1];
extern Data_t xmem[8*MAX_MATRIX_SIZE+100];

void zero_regs(void);
void dump_regs(void);
void dump_array(Reg_t matrix);
inline Addr_t Mbegin(Reg_t r) { return reg[r]; }
inline Addr_t Mend  (Reg_t r) { return reg[r]+Msize(reg[r+M0_SHAPE]); }

extern const char* const reg_name[];
extern const char * const command_name[];
extern const char * const status_name[];

void dev_hls(Addr_t* reg, Data_t *xmem);

#endif /*DEV_UTIL_H*/
