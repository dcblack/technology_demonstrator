/**********************/
/* Vivado HLS Source */
/********************/
#include "dev_hls.h"
#include <string.h>

#define SET_REG(r, data) do {     \
switch (r) {                      \
  case  0: *reg_R0  = data; break;\
  case  1: *reg_R1  = data; break;\
  case  2: *reg_R2  = data; break;\
  case  3: *reg_R3  = data; break;\
  case  4: *reg_R4  = data; break;\
  case  5: *reg_R5  = data; break;\
  case  6: *reg_R6  = data; break;\
  case  7: *reg_R7  = data; break;\
  case  8: *reg_R8  = data; break;\
  case  9: *reg_R9  = data; break;\
  case 10: *reg_R10 = data; break;\
  case 11: *reg_R11 = data; break;\
  case 12: *reg_R12 = data; break;\
  case 13: *reg_R13 = data; break;\
  case 14: *reg_R14 = data; break;\
  case 15: *reg_R15 = data; break;\
}} while(0)

#define GET_REG(r, data) do {     \
switch (r) {                      \
  case  0: data = *reg_R0 ; break;\
  case  1: data = *reg_R1 ; break;\
  case  2: data = *reg_R2 ; break;\
  case  3: data = *reg_R3 ; break;\
  case  4: data = *reg_R4 ; break;\
  case  5: data = *reg_R5 ; break;\
  case  6: data = *reg_R6 ; break;\
  case  7: data = *reg_R7 ; break;\
  case  8: data = *reg_R8 ; break;\
  case  9: data = *reg_R9 ; break;\
  case 10: data = *reg_R10; break;\
  case 11: data = *reg_R11; break;\
  case 12: data = *reg_R12; break;\
  case 13: data = *reg_R13; break;\
  case 14: data = *reg_R14; break;\
  case 15: data = *reg_R15; break;\
}} while(0)

#define VALIDATE_REGISTER(r)     \
if ( r<0 || 15<r ) {             \
  *reg_STATUS = REGISTER_ERROR;  \
  break;                         \
} r |= 0 /* to allow ; at end */

#define VALIDATE_MATRIX(r)       \
if ( r<0 || 14<r || r&1) {       \
  *reg_STATUS = REGISTER_ERROR;  \
  break;                         \
} r |= 0 /* to allow ; at end */


// NOTE: _ARRAY's are indexes to internal memory locations
//
void dev_hls
( volatile Data_t* reg_R0          //< AXI slave
, volatile Data_t* reg_R1          //< AXI slave
, volatile Data_t* reg_R2          //< AXI slave
, volatile Data_t* reg_R3          //< AXI slave
, volatile Data_t* reg_R4          //< AXI slave
, volatile Data_t* reg_R5          //< AXI slave
, volatile Data_t* reg_R6          //< AXI slave
, volatile Data_t* reg_R7          //< AXI slave
, volatile Data_t* reg_R8          //< AXI slave
, volatile Data_t* reg_R9          //< AXI slave
, volatile Data_t* reg_R10         //< AXI slave
, volatile Data_t* reg_R11         //< AXI slave
, volatile Data_t* reg_R12         //< AXI slave
, volatile Data_t* reg_R13         //< AXI slave
, volatile Data_t* reg_R14         //< AXI slave
, volatile Data_t* reg_R15         //< AXI slave
, volatile Data_t* reg_AXI_BASE    //< AXI slave offset for master accesses
, volatile Data_t* reg_COMMAND     //< AXI slave
, volatile Data_t* reg_STATUS      //< AXI slave
,          Data_t  imem[IMEM_SIZE] //< Device memory
, volatile Data_t* xmem   //< AXI master
)
{

  FOREVER_LOOP:
  for(;;) {
    Data_t status = *reg_STATUS;
    if (status != START) return;

    *reg_STATUS = BUSY;

    Data_t command = *reg_COMMAND;
    
    unsigned char operation = (command >> 3*8) & 0xFF;
    unsigned char dest      = (command >> 2*8) & 0xFF;
    unsigned char src1      = (command >> 1*8) & 0xFF;
    unsigned char src2      = (command >> 0*8) & 0xFF;

    switch(operation) {

      //--------------------------------------------------------------------------
      // Operation: Clear all to zero
      case RESET:
      {
        *reg_AXI_BASE = 0;
        *reg_R0  = 0;
        *reg_R1  = 0;
        *reg_R2  = 0;
        *reg_R3  = 0;
        *reg_R4  = 0;
        *reg_R5  = 0;
        *reg_R6  = 0;
        *reg_R7  = 0;
        *reg_R8  = 0;
        *reg_R9  = 0;
        *reg_R10 = 0;
        *reg_R11 = 0;
        *reg_R12 = 0;
        *reg_R13 = 0;
        *reg_R14 = 0;
        *reg_R15 = 0;
        *reg_COMMAND = NOP;
        *reg_STATUS = IDLE;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: Do nothing
      case NOP:
      {
        *reg_STATUS = DONE;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: Stop processing (when auto is implemented)
      case HALT:
      {
        *reg_STATUS = HALTED;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: Copy matrix from external *R(src1) to internal M(R(dest))
      case LOAD:
      {
        Addr_t i_ptr, x_ptr; // respectively point to internal & external memories
        Data_t shape;
        Addr_t base;

        base = *reg_AXI_BASE;

        VALIDATE_REGISTER(src1);
        GET_REG(src1,x_ptr);
        x_ptr += base;
        shape = xmem[x_ptr++];
        
        VALIDATE_MATRIX(dest);
        SET_REG(dest,shape);
        GET_REG(dest+1,i_ptr);

        // Make sure it's a valid shape
        if (shape == 0 || Msize(shape) > MAX_MATRIX_SIZE) {
          *reg_STATUS = SHAPE_ERROR;
          break;
        }

        // Make sure it's a valid internal memory location
        if (!Mvalid(i_ptr)) {
          *reg_STATUS = ADDRESS_ERROR;
          break;
        }
        LOAD_LOOP:
        unsigned int size = Msize(shape);
        for (unsigned int i=0; i!=size; ++i) {
          Data_t data;
          data = xmem[x_ptr++];
          imem[i_ptr++] = data;
        }
        *reg_STATUS = DONE;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: Copy matrix to external *R(src1) from internal M(R(dest))
      case STORE:
      {
        Addr_t i_ptr, x_ptr; // respectively point to internal & external memories
        Data_t shape;
        Addr_t base;

        base = *reg_AXI_BASE;

        VALIDATE_MATRIX(dest);
        GET_REG(dest,shape);
        GET_REG(dest+1,i_ptr);

        VALIDATE_REGISTER(src1);
        GET_REG(src1,x_ptr);
        x_ptr += base;
        
        // Make sure it's a valid shape
        if (shape == 0 || Msize(shape) > MAX_MATRIX_SIZE) {
          *reg_STATUS = SHAPE_ERROR;
          break;
        }
        xmem[x_ptr++] = shape;
        // Make sure it's a valid internal memory location
        if (!Mvalid(i_ptr)) {
          *reg_STATUS = ADDRESS_ERROR;
          break;
        }
        STORE_LOOP:
        unsigned int size = Msize(shape);
        for (unsigned int i=0; i!=size; ++i) {
          Data_t data;
          data = imem[i_ptr++];
          xmem[x_ptr++] = data;
        }
        *reg_STATUS = DONE;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: M(R(dest)) = M(R(src1));
      case MCOPY:
      {
        Addr_t dst_ptr, src_ptr;
        Data_t dst_shape, src_shape;

        VALIDATE_MATRIX(dest);
        GET_REG(dest,dst_ptr);
        GET_REG(dest+1,dst_ptr);

        VALIDATE_MATRIX(src1);
        GET_REG(src1,src_shape);
        GET_REG(src1+1,src_ptr);

        // Make sure pointers are within memory
        if (!Mvalid(src_ptr) || !Mvalid(dst_ptr)) {
          *reg_STATUS = ADDRESS_ERROR;
          break;
        }
        if (dst_shape != src_shape) {
          *reg_STATUS = SHAPE_ERROR;
          break;
        }

        COPY_LOOP:
        for (unsigned int i=0; i!= Msize(dst_shape); ++i) {
          Data_t data;
          data = imem[src_ptr++];
          imem[dst_ptr++] = data;
        }
        *reg_STATUS = DONE;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: Simple element by element arithmetic
      case MADD: // M(dest) = M(src1) + M(src2)
      case MSUB: // M(dest) = M(src1) - M(src2);
      case RSUB: // M(dest) = M(src2) - M(src1);
      case KMUL: // M(dest) = R(src1) * M(src1);
      {
        Data_t dest_shape, src1_shape, src2_shape;
        bool not_KMUL = (operation != KMUL);

        VALIDATE_MATRIX(dest);
        if (not_KMUL) {
          VALIDATE_MATRIX(src1);
        } else {
          VALIDATE_REGISTER(src1);
        }
        VALIDATE_MATRIX(src2);

        GET_REG(dest,dest_shape);
        if (not_KMUL) GET_REG(src1,src1_shape);
        GET_REG(src2,src2_shape);

        // Make sure shapes are valid
        if (not_KMUL && dest_shape != src1_shape) {
          *reg_STATUS = SHAPE_ERROR;
          break;
        }
        if (dest_shape != src2_shape) {
          *reg_STATUS = SHAPE_ERROR;
          break;
        }

        Addr_t dest_ptr, src1_ptr, src2_ptr;
        Data_t k;
        GET_REG(dest+1,dest_ptr);
        if (not_KMUL)          GET_REG(src1+1,src1_ptr);
        else                   GET_REG(src1,k);
        GET_REG(src2+1,src2_ptr);

        // Make sure pointers are withing memory
        if (!Mvalid(dest_ptr)
        ||  !(not_KMUL && Mvalid(src1_ptr))
        || !Mvalid(src2_ptr)) {
          *reg_STATUS = ADDRESS_ERROR;
          break;
        }
        Data_t data1, data2;
        long long int data0;
        ARITHMETIC_LOOP:
        for (Addr_t i=Msize(dest_shape); i!=0; --i) {
          if (not_KMUL) data1 = imem[src1_ptr++];
          data2 = imem[src2_ptr++];
          if      (operation == MADD)    data0 = data1 + data2;
          else if (operation == MSUB)    data0 = data1 - data2;
          else if (operation == RSUB)    data0 = data2 - data1;
          else /* (operation == KMUL) */ data0 = k     * data2;
          //{:TODO:Detect over/underflow:}
          imem[dest_ptr++] = data0;
        }
        *reg_STATUS = DONE;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: Count zeroes
      case MZERO: // R(dest) = zeroes(M(src1))
      {
        VALIDATE_REGISTER(dest);

        VALIDATE_MATRIX(src1);
        Addr_t src1_ptr;
        Data_t src1_shape;
        GET_REG(src1,src1_shape);
        GET_REG(src1+1,src1_ptr);
        // Make sure pointer is within memory
        if (!Mvalid(src1_ptr)) {
          *reg_STATUS = ADDRESS_ERROR;
          break;
        }
        Data_t count;
        COUNT_LOOP:
        for (Addr_t i=Msize(src1_shape); i!=0; --i) {
          if (imem[src1_ptr++] == 0) ++count;
        }
        SET_REG(dest,count);
        *reg_STATUS = DONE;
        break;
      }

      //{:TODO - additional commands:}
      //--------------------------------------------------------------------------
      // Operation: Matrix multiply
      case MMUL:  // R(dest) = M(src1) x M(src2)
      {
      }
      //--------------------------------------------------------------------------
      // Operation: Sum of elements
      case MSUM:  // R(dest) = sum(M(src1)[i]);
      {
      }
      //--------------------------------------------------------------------------
      // Operation: Determinant
      case MDET0: // R(dest) = determinant(M0);
      {
      }
      //--------------------------------------------------------------------------
      // Operation: Compare matrices
      case EQUAL: // R(dest) = M(dest) == M(src1);
      {
      }
      //--------------------------------------------------------------------------
      // Operation: Matrix transpose
      case TRANS: // M(dest) = transpose(M(src1));
      {
      }
      //--------------------------------------------------------------------------
      // Operation: Fill matrix with constant
      case FILL:  // M(dest) = R(src1); // fill matrix
      {
      }
      //--------------------------------------------------------------------------
      // Operation: Set matrix to identity
      case IDENT: // M(dest) = indentity(R2);
      {
      }
      //--------------------------------------------------------------------------
      // Operation: Copy register
      case RCOPY: // R(dest) = R(src1); // register copy
      {
        Addr_t val;
        VALIDATE_REGISTER(src1);
        GET_REG(src1,val);
        VALIDATE_REGISTER(dest);
        SET_REG(dest,val);
      }
      //--------------------------------------------------------------------------
      // Operation: Set register high half immediate
      case RSETH: // R(dest)[31:16] = (src1<<8)|src2; // register set hi
      {
        Addr_t val;
        VALIDATE_REGISTER(dest);
        GET_REG(dest,val);
        val &= 0xFFFF;
        val |= ((src1 << 8)|src2)<<16;
        SET_REG(dest,val);
      }
      //--------------------------------------------------------------------------
      // Operation: Set register low half immediate
      case RSETL: // R(dest)[15:00] = (src1<<8)|src2; // register set lo
      {
        Addr_t val;
        VALIDATE_REGISTER(dest);
        GET_REG(dest,val);
        val &= ~0xFFFF;
        val |= ((src1 << 8)|src2);
        SET_REG(dest,val);
      }
      //--------------------------------------------------------------------------
      default: // operation not implemented/supported
        {
          *reg_STATUS = UNSUPPORTED_ERROR;
          break;
        }
    }//endswitch

  }//endforever
  // Never gets here
}
// The end
