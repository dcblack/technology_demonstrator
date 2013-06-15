#ifndef DEV_HLS_H
#define DEV_HLS_H

#include <ap_int.h>

#define MAX_MATRIX_SIZE 32*32
typedef int          Data_t;
typedef unsigned int Addr_t;

enum Reg_t
{ ARRAY    // Base of matrix array pointers
, SHAPE=16 // Base for shapes { ROWS[31:16], COLS[15:0] }
, R0=32    // General purpose scalar result
, K1       // General purpose scalar constant
, K2       // General purpose scalar constant
, K3       // General purpose scalar constant
, COMMAND=42
, STATUS
};

#define M0_ARRAY ARRAY+0
#define M1_ARRAY ARRAY+1
#define M2_ARRAY ARRAY+2
#define M3_ARRAY ARRAY+3
#define M0_SHAPE SHAPE+0
#define M1_SHAPE SHAPE+1
#define M2_SHAPE SHAPE+2
#define M3_SHAPE SHAPE+3

inline Addr_t Mrows (Addr_t shape) { return shape>>16; }
inline Addr_t Mcols (Addr_t shape) { return shape & 0xFFFF; }
inline Addr_t Mshape(Addr_t rows, Addr_t cols) { return (rows<<16)+(cols&0xFFFF); }
inline Addr_t Msize (Addr_t shape) { return Mrows(shape)*Mcols(shape); }
inline bool   Msquare(Addr_t shape) { return Mrows(shape) == Mcols(shape); }

enum Command_t
{ NOP
, LOAD   // imem[] = xmem[M0_ARRAY..size(M0_shape)]
, STORE  // 
, MCOPY  // M0 = M1;
, MADD   // M0 = M1 + M2;
, MSUB   // M0 = M1 - M2;
, RSUB   // M0 = M2 - M1;
, MMUL   // M0 = M1 x M2;
, KMUL   // M0 = K1 * M1[i];
, MSUM   // R0 = sum(M1[i]);
, MDET0  // R0 = determinant(M0);
, STOP
};

enum Status_t
{ IDLE
, BUSY
, DONE
, HALTED
, UNSUPPORTED_ERROR
, SHAPE_ERROR
, ADDRESS_ERROR
, GENERIC_ERROR
};

#endif /*DEV_HLS_H*/
