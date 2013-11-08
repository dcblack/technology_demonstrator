#ifndef DEV_HLS_H
#define DEV_HLS_H

#define MAX_MATRIX_SIZE (32/*rows*/ * 32/*cols*/)
#define MAX_MATRIX_SPACE (MAX_MATRIX_SIZE+1)
#define DW 4 /*bytes per datum*/
#define AW 2 /*log2(DW)*/
#define H_SHFT 16
#define L_MASK 0xFFFF

#ifdef USING_SYSTEMC
typedef unsigned char*      Byte_ptr;
#ifdef BIT_ACCURATE
#warning "INFO: Using bit accurate SystemC"
typedef sc_dt::sc_uint<32>  Addr_t;
typedef sc_dt::sc_int<32>   Data_t;
typedef sc_dt::sc_int<32>   Cmd_t;
class Axibus;
typedef Axibus              AxiM_t;
#else

//warning "INFO: Using fast SystemC"
#include "stdint.h"
typedef uint32_t            Addr_t;
typedef int32_t             Data_t;
typedef uint32_t            Cmd_t;
class Axibus;
typedef Axibus              AxiM_t;
#endif
#else

#ifdef __SYNTHESIS__
#ifdef USE_APINT
#warning "INFO: Using HLS synthesis with ap_int"
#include "ap_int.h"
typedef ap_uint<32>         Addr_t;
typedef ap_int<32>          Data_t;
typedef ap_int<32>          Cmd_t;
typedef Addr_t              AxiM_t;

#else
#warning "INFO: Using HLS synthesis with C++ int"
typedef unsigned int        Addr_t;
typedef int                 Data_t;
typedef int                 Cmd_t;
typedef Data_t              AxiM_t;
#endif

#else /* standard C++ */
#warning "INFO: Using standard C++"
#include "stdint.h"
typedef unsigned int        Addr_t;
typedef int                 Data_t;
typedef int                 Cmd_t;
typedef Data_t              AxiM_t;
#endif
#endif

#define XMATRICES   1
#define XMEM_SIZE   (MAX_MATRIX_SPACE*XMATRICES)
#define IMATRICES   16
#define IMEM_SIZE   (IMATRICES*MAX_MATRIX_SIZE)
#define IMEM_LAST   (IMEM_SIZE-MAX_MATRIX_SIZE)
#define DEV_REGS    16
#define REGISTERS   19
#define ALIGN_CMND  0x3

Data_t dev_hls
( Data_t   reg[REGISTERS]
, Data_t   mem[IMEM_SIZE]
);

// Matrix points to a shape followed by the array itself
// Shape is contained as two 16 bit fields in a word
// Number of rows stored in the upper 16 bits
// Number of columns stored in the lower 16 bits
// Array organized in row major order and contains 32
// bit values thusly { D[0,0], D[0,1] ... }
//
// The following reaonably displays the matrix for small (<=18x18) matrices:
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

enum Reg_t     // Word offsets for registers (32 bits each)
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
inline Addr_t Mspace(Addr_t shape) { return Msize(shape)+1; }
inline Addr_t Mspace(Addr_t rows, Addr_t cols) { return rows*cols + 1; }
inline Addr_t Mindex(Addr_t cols, Addr_t x, Addr_t y) { return x*cols + y + 1; }
inline bool   Msquare(Addr_t shape) { return Mrows(shape) == Mcols(shape); }
inline bool   Mvalid(Addr_t ptr) { return ptr<=IMEM_LAST; }

enum Operation_t // Operations {:TODO:_not_all_implemented:}
{ NOP    //  no operation
, LOAD   //{:M(R(dest)) = xmem(base+R(src1)):}
, STORE  //{:xmem(base+R(src1)) = M(R(dest)):}
, MCOPY  //  M(dest) = M(src1);
, MADD   //  M(dest) = M(src1) + M(src2); // matrix add
, MSUB   //  M(dest) = M(src1) - M(src2); // matrix subtract
, RSUB   //  M(dest) = M(src2) - M(src1); // matrix reverse subtract
, MMUL   //{:M(dest) = M(src1) x M(src2); // matrix multiply:}
, KMUL   //  M(dest) = R(src1) * M(src2)[i]; // constant multiply
, MSUM   //{:R(dest) = sum(M(src1)[i]);   // sum of elements:}
, MDET0  //{:R(dest) = determinant(M0);:}
, EQUAL  //{:R(dest) = M(dest) == M(src1); // compare:}
, MZERO  //  R(dest) = sum(M(src)[i]==0); // count zeroes
, TRANS  //{:M(dest) = transpose(M(src1));:}
, FILL   //  M(dest) = R(src1); // fill matrix
, IDENT  //{:M(dest) = indentity(R2);:}
, RCOPY  //  R(dest) = R(src1); // register copy
, RSETX  //  R(dest)[31:00] = (src1<<8)|src2; // register set with sign-extend
, RSETH  //  R(dest)[31:16] = (src1<<8)|src2; // register set hi
, RSETL  //  R(dest)[15:00] = (src1<<8)|src2; // register set lo
, RESET  //  clear all registers
, EXEC   //  execute sequence starting from M(R(15))
, HALT   //  stop processing
};

enum CmdState_t
{ IDLE               = 0       // waiting
, START              = 1 <<  0 // begin executing
, BUSY               = 1 <<  1 // active
, DONE               = 1 <<  2 // completed command
, HALTED             = 1 <<  3 // result of STOP command
, UNSUPPORTED_ERROR  = 1 <<  4
, ADDRESS_ERROR      = 1 <<  5
, GENERIC_ERROR      = 1 <<  6
, SHAPE_ERROR        = 1 <<  7
, REGISTER_ERROR     = 1 <<  8
, SHAPE_MISMATCH     = 1 <<  9
, UNKNOWN_STATUS     = 1 << 10
};
#define STATE_BITS 0xFFF
#define EXEC_BIT   0x1000
#define INTR_BIT   0x4000
#define AUTOMATIC(status) (((status) & (EXEC_BIT|STATE_BITS)) == (EXEC_BIT|DONE))

// Helpful aliases
#define M0 2*0
#define M1 2*1
#define M2 2*2
#define M3 2*3
#define M4 2*4
#define M5 2*5
#define M6 2*6

#define reg_M0       R0
#define reg_M0_SHAPE R0
#define reg_M0_ARRAY R1

#define reg_M1       R2
#define reg_M1_SHAPE R2
#define reg_M1_ARRAY R3

#define reg_M2       R4
#define reg_M2_SHAPE R4
#define reg_M2_ARRAY R5

#define reg_M3       R6
#define reg_M3_SHAPE R6
#define reg_M3_ARRAY R7

#define reg_M4       R8
#define reg_M4_SHAPE R8
#define reg_M4_ARRAY R9

#define reg_M5       R10
#define reg_M5_SHAPE R10
#define reg_M5_ARRAY R11

#define reg_M6       R12
#define reg_M6_SHAPE R12
#define reg_M6_ARRAY R13

#define reg_SR R14
#define reg_PC R15


//------------------------------------------------------------------------------
// $LICENSE: Apache 2.0 $ <<<
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  L<http://www.apache.org/licenses/LICENSE-2.0>
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//--------------------------------------------------------------------------->>>
// The end!
#endif /*DEV_HLS_H*/
