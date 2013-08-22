#ifndef DEV_HLS_H
#define DEV_HLS_H

#define MAX_MATRIX_SIZE (32/*rows*/ * 32/*cols*/)
#define MAX_MATRIX_SPACE (MAX_MATRIX_SIZE+1)
#define DW 4 /*bytes per datum*/
#define AW 2 /*log2(DW)*/
#define H_SHFT 16
#define L_MASK 0xFFFF
typedef int          Data_t; //< 32-bits
typedef unsigned int Addr_t; //< 32-bits
typedef Data_t       Axi_t;  //< needed for SystemC

// Matrix points to a shape followed by the array itself
// Shape is contained as two 16 bit fields in a word
// Number of rows stored in the upper 16 bits
// Number of columns stored in the lower 16 bits
// Array organized in row major order and contains 32
// bit values thusly { D[0,0], D[0,1] ... }
//
// The following reaonably displays the matrix for small matrices:
#ifndef SYNTHESIS
#include <iostream>
#include <iomanip>
static void display_matrix(Data_t* M) {
  std::size_t cols = ((*M)>>H_SHFT) & L_MASK;
  std::size_t rows =  (*M)      & L_MASK;
  for (int r = 0; r != rows; ++r) {
    std::cout << std::setw(3) << r << ": ";
    for (int c = 0; c != cols; ++c) {
      std::cout << std::setw(4) << M[ (r*cols) + c + 1 ] << " ";
    }
    std::cout << "\n";
  }
  std::cout << std::endl;
}
#endif

#define XMATRICES 64
#define XMEM_SIZE (MAX_MATRIX_SPACE*XMATRICES)
#define IMATRICES 16
#define IMEM_SIZE (IMATRICES*MAX_MATRIX_SIZE)
#define IMEM_LAST (IMEM_SIZE-MAX_MATRIX_SIZE)
#define DEV_REGS  16

enum Reg_t     // Byte addresses for registers (32 bits each)
{ R0           // Aka M0_SHAPE
, R1           // Aka M0_ARRAY
, R2           // Aka M1_SHAPE
, R3           // Aka M1_ARRAY
, R4           // Aka M2_SHAPE
, R5           // Aka M2_ARRAY
, R6           // Aka M3_SHAPE
, R7           // Aka M3_ARRAY
, R8           // Aka M4_SHAPE
, R9           // Aka M4_ARRAY
, R10          // Aka M5_SHAPE
, R11          // Aka M5_ARRAY
, R12          // Aka M6_SHAPE
, R13          // Aka M6_ARRAY
, R14          // Aka SR
, R15          // Aka PC
, BASE=16      // AXI base address
, COMMAND      // Opcode, Dest, Src1, Src2
, STATUS       // Status register
};

inline Addr_t Mrows (Addr_t shape) { return (shape>>H_SHFT) & L_MASK; }
inline Addr_t Mcols (Addr_t shape) { return  shape          & L_MASK; }
inline Addr_t Mshape(Addr_t rows, Addr_t cols) { return (rows<<H_SHFT)+(cols&L_MASK); }
inline Addr_t Msize (Addr_t shape) { return Mrows(shape)*Mcols(shape); }
inline bool   Msquare(Addr_t shape) { return Mrows(shape) == Mcols(shape); }
inline bool   Mvalid(Addr_t ptr) { return ptr%MAX_MATRIX_SIZE==0 && ptr<=IMEM_LAST; }

enum Operation_t // Operations {:TODO:_not_all_implemented:}
{ NOP    // no operation
, LOAD   // M(R(dest)) = xmem(base+R(src1))
, STORE  // xmem(base+R(src1)) = M(R(dest)
, MCOPY  // M(dest) = M(src1);
, MADD   // M(dest) = M(src1) + M(src2); // matrix add
, MSUB   // M(dest) = M(src1) - M(src2); // matrix subtract
, RSUB   // M(dest) = M(src2) - M(src1); // matrix reverse subtract
, MMUL   // M(dest) = M(src1) x M(src2); // matrix multiply
, KMUL   // M(dest) = R(src1) * M(src2)[i]; // constant multiply
, MSUM   // R(dest) = sum(M(src1)[i]);   // sum of elements
, MDET0  // R(dest) = determinant(M0);
, EQUAL  // R(dest) = M(dest) == M(src1); // compare
, MZERO  // R(dest) = sum(M(src)[i]==0); // count zeroes
, TRANS  // M(dest) = transpose(M(src1));
, FILL   // M(dest) = R(src1); // fill matrix
, IDENT  // M(dest) = indentity(R2);
, RCOPY  // R(dest) = R(src1); // register copy
, RSETH  // R(dest)[31:16] = (src1<<8)|src2; // register set hi
, RSETL  // R(dest)[15:00] = (src1<<8)|src2; // register set lo
, RESET  // clear all registers
, HALT   // stop processing
};

enum Status_t
{ IDLE   // waiting
, START  // begin executing
, BUSY   // active
, DONE   // completed command
, HALTED // result of STOP command
, UNSUPPORTED_ERROR
, SHAPE_ERROR
, REGISTER_ERROR
, ADDRESS_ERROR
, GENERIC_ERROR
};

// Helpful aliases
#define M0 2*0
#define M1 2*1
#define M2 2*2
#define M3 2*3
#define M4 2*4
#define M5 2*5
#define M6 2*6

#define reg_M0       reg0
#define reg_M0_SHAPE reg0
#define reg_M0_ARRAY reg1

#define reg_M1       reg2
#define reg_M1_SHAPE reg2
#define reg_M1_ARRAY reg3

#define reg_M2       reg4
#define reg_M2_SHAPE reg4
#define reg_M2_ARRAY reg5

#define reg_M3       reg6
#define reg_M3_SHAPE reg6
#define reg_M3_ARRAY reg7

#define reg_M4       reg8
#define reg_M4_SHAPE reg8
#define reg_M4_ARRAY reg9

#define reg_M5       reg10
#define reg_M5_SHAPE reg10
#define reg_M5_ARRAY reg11

#define reg_M6       reg12
#define reg_M6_SHAPE reg12
#define reg_M6_ARRAY reg13

#define reg_SR reg14
#define reg_PC reg15

#endif /*DEV_HLS_H*/
