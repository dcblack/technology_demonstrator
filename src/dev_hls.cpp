/**********************/
/* Vivado HLS Source */
/********************/
#include "dev_hls.h"
#include <string.h>

// Isolate implementation from code
#define SET_REG(r, data) reg[r] = data;
#define GET_REG(r, data) data = reg[r];

#define VALIDATE_REGISTER(r)             \
if ( 15<r ) {                            \
  retcode |= REGISTER_ERROR; \
  break;                                 \
} else ;

#define VALIDATE_MATRIX(r)               \
if ( 14<r || r&1) {                      \
  retcode |= REGISTER_ERROR; \
  break;                                 \
} else ;


Data_t dev_hls
( Data_t  reg[REGISTERS]
, Data_t  mem[IMEM_SIZE]
)
{

// Port mappings protcols
#pragma HLS interface ap_memory  port=reg
#pragma HLS interface ap_ctrl_hs port=return register
#pragma HLS interface ap_memory  port=mem

#pragma HLS resource core=RAM_1P    metadata="-bus_bundle devreg" variable=reg
#pragma HLS resource core=AXI4LiteS metadata="-bus_bundle devreg" variable=return
#pragma HLS resource core=RAM_1P    metadata="-bus_bundle devreg" variable=mem

  Data_t status;
  Data_t retcode;

  status = reg[STATUS];

  unsigned char cmd_state = status & STATE_BITS;
  unsigned char run = (status & EXEC_BIT) != 0;

  status &= ~STATE_BITS; // Clear state bits
  retcode = status & ~STATE_BITS;

  if (cmd_state & START || (run == 1 && cmd_state == DONE)) {

    Cmd_t command;

    if (run == 1) {
      Addr_t pc   = reg[R15];
      Addr_t base = reg[BASE];
      pc &= ~ALIGN_CMND;
      pc += sizeof(Cmd_t);
      command = mem[pc];
      reg[R15] = pc;
    } else {
      command = reg[COMMAND];
    }

    unsigned char operation = (command >> 3*8) & 0xFF;
    unsigned char dest      = (command >> 2*8) & 0xFF;
    unsigned char src1      = (command >> 1*8) & 0xFF;
    unsigned char src2      = (command >> 0*8) & 0xFF;

    switch(operation) {

      //--------------------------------------------------------------------------
      // Operation: Clear all to zero
      case RESET:
      {
        for (int i=0; i!=REGISTERS; ++i) reg[i] = 0;
        retcode |= IDLE;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: Do nothing
      case NOP:
      {
        retcode |= DONE;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: Execute sequence starting at M(R15)
      case EXEC:
      {
        retcode |= EXEC_BIT | DONE;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: Stop processing (when auto is implemented)
      case HALT:
      {
        retcode &= ~EXEC_BIT;
        retcode |= HALTED;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: Copy matrix from external *R(src1) to internal M(R(dest))
      case LOAD:
      {
        //{:TODO:Implement AXI4 Master interface to external memory:}
        retcode |= UNSUPPORTED_ERROR;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: Copy matrix to external *R(src1) from internal M(R(dest))
      case STORE:
      {
        //{:TODO:Implement AXI4 Master interface to external memory:}
        retcode |= UNSUPPORTED_ERROR;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: M(R(dest)) = M(R(src1));
      case MCOPY:
      {
        Addr_t dst_ptr, src_ptr;
        Data_t dst_shape, src_shape;

        VALIDATE_MATRIX(dest)
        GET_REG(dest,dst_shape)
        GET_REG(dest+1,dst_ptr)

        VALIDATE_MATRIX(src1)
        GET_REG(src1,src_shape)
        GET_REG(src1+1,src_ptr)

        // Make sure pointers are within memory
        if (!Mvalid(src_ptr) || !Mvalid(dst_ptr)) {
          retcode |= ADDRESS_ERROR;
          break;
        }
        if (dst_shape != src_shape) {
          retcode |= SHAPE_MISMATCH;
          break;
        }

        COPY_LOOP:
        for (unsigned int i=0; i!= Msize(dst_shape); ++i) {
          Data_t data;
          data = mem[src_ptr++];
          mem[dst_ptr++] = data;
        }
        retcode |= DONE;
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

        VALIDATE_MATRIX(dest)
        if (not_KMUL) {
          VALIDATE_MATRIX(src1)
        } else {
          VALIDATE_REGISTER(src1)
        }
        VALIDATE_MATRIX(src2)

        GET_REG(dest,dest_shape)
        if (not_KMUL) GET_REG(src1,src1_shape)
        GET_REG(src2,src2_shape)

        // Make sure shapes are valid
        if (not_KMUL && dest_shape != src1_shape) {
          retcode |= SHAPE_MISMATCH;
          break;
        }
        if (dest_shape != src2_shape) {
          retcode |= SHAPE_MISMATCH;
          break;
        }

        Addr_t dest_ptr, src1_ptr, src2_ptr;
        Data_t k;
        GET_REG(dest+1,dest_ptr)
        if (not_KMUL)          GET_REG(src1+1,src1_ptr)
        else                   GET_REG(src1,k)
        GET_REG(src2+1,src2_ptr)

        // Make sure pointers are within memory
        if (!Mvalid(dest_ptr)
        ||  (not_KMUL && !Mvalid(src1_ptr))
        ||  !Mvalid(src2_ptr)
        ) {
          retcode |= ADDRESS_ERROR;
          break;
        }
        Data_t data1, data2;
        long long int data0;
        ARITHMETIC_LOOP:
        for (Addr_t i=Msize(dest_shape); i!=0; --i) {
          if (not_KMUL) data1 = mem[src1_ptr++];
          data2 = mem[src2_ptr++];
          if      (operation == MADD)    data0 = data1 + data2;
          else if (operation == MSUB)    data0 = data1 - data2;
          else if (operation == RSUB)    data0 = data2 - data1;
          else /* (operation == KMUL) */ data0 = k     * data2;
          //{:TODO:Detect over/underflow:}
          mem[dest_ptr++] = data0;
        }
        retcode |= DONE;
        break;
      }

      //--------------------------------------------------------------------------
      // Operation: Count zeroes
      case MZERO: // R(dest) = zeroes(M(src1))
      {
        VALIDATE_REGISTER(dest)

        VALIDATE_MATRIX(src1)
        Addr_t src1_ptr;
        Data_t src1_shape;
        GET_REG(src1,src1_shape)
        GET_REG(src1+1,src1_ptr)
        // Make sure pointer is within memory
        if (!Mvalid(src1_ptr)) {
          retcode |= ADDRESS_ERROR;
          break;
        }
        Data_t count = 0;
        COUNT_LOOP:
        for (Addr_t i=Msize(src1_shape); i!=0; --i) {
          if (mem[src1_ptr++] == 0) ++count;
        }
        SET_REG(dest,count)
        retcode |= DONE;
        break;
      }

      //{:TODO - additional commands:}
      //--------------------------------------------------------------------------
      // Operation: Matrix multiply
      case MMUL:  // R(dest) = M(src1) x M(src2)
      {
        Addr_t dest_ptr, src1_ptr, src2_ptr;
        Data_t dest_shape, src1_shape, src2_shape;
        unsigned int dest_rows, dest_cols, src1_rows, src1_cols, src2_rows, src2_cols;

        VALIDATE_MATRIX(dest)
        GET_REG(dest,dest_shape)
        GET_REG(dest+1,dest_ptr)
        dest_rows = Mrows(dest_shape);
        dest_cols = Mcols(dest_shape);

        VALIDATE_MATRIX(src1)
        GET_REG(src1,src1_shape)
        GET_REG(src1+1,src1_ptr)
        src1_rows = Mrows(src1_shape);
        src1_cols = Mcols(src1_shape);

        VALIDATE_MATRIX(src2)
        GET_REG(src2,src2_shape)
        GET_REG(src2+1,src2_ptr)
        src2_rows = Mrows(src2_shape);
        src2_cols = Mcols(src2_shape);

        if (!Mvalid(dest_ptr)
         || !Mvalid(src1_ptr)
         || !Mvalid(src2_ptr)
         ) {
          retcode |= ADDRESS_ERROR;
          break;
        }
        if (dest_rows != src1_rows
         || dest_cols != src2_cols
         || src1_cols != src2_rows
           ) {
          retcode |= SHAPE_MISMATCH;
          break;
        }

        Data_t dest_data, src1_data, src2_data;
        for (unsigned int r=0; r!= src1_rows; ++r) {
          for (unsigned int c=0; c!= src2_cols; ++c) {
            dest_data = 0;
            for (unsigned int i=0; i!= src1_rows; ++i) {
              src1_data = mem[src1_ptr+Mindex(src1_cols,r,i)];
              src2_data = mem[src2_ptr+Mindex(src2_cols,i,c)];
              dest_data += src1_data * src2_data;
            }
            mem[dest_ptr+Mindex(src2_cols,r,c)] = dest_data;
          }
        }

        retcode |= DONE;
        break;
      }
      //--------------------------------------------------------------------------
      //{:Operation: Sum of elements:}
      case MSUM:  // R(dest) = sum(M(src1)[i]);
      {
        Addr_t src1_ptr;
        Data_t src1_shape;

        VALIDATE_REGISTER(dest);

        VALIDATE_MATRIX(src1)
        GET_REG(src1,src1_shape)
        GET_REG(src1+1,src1_ptr)

        if (!Mvalid(src1_ptr)) {
          retcode |= ADDRESS_ERROR;
          break;
        }

        Data_t sum;
        sum = 0;
        for (Addr_t i=Msize(src1_shape); i!=0; --i) {
          sum += mem[src1_ptr++];
        }

        retcode |= DONE;
        break;
      }
      //--------------------------------------------------------------------------
      //{:peration: Determinant:}
      case MDET0: // R(dest) = determinant(M0);
      {
        retcode |= UNSUPPORTED_ERROR;
        break;
      }
      //--------------------------------------------------------------------------
      //{:Operation: Compare matrices:}
      case EQUAL: // R(dest) = M(src1) == M(src2);
      {
        Addr_t src1_ptr, src2_ptr;
        Data_t src1_shape, src2_shape;

        VALIDATE_REGISTER(dest);

        VALIDATE_MATRIX(src1)
        GET_REG(src1,src1_shape)
        GET_REG(src1+1,src1_ptr)

        VALIDATE_MATRIX(src2)
        GET_REG(src2,src2_shape)
        GET_REG(src2+1,src2_ptr)

        if (!Mvalid(src1_ptr)
         || !Mvalid(src2_ptr)
         ) {
          retcode |= ADDRESS_ERROR;
          break;
        }

        Data_t equal;
        equal = 1;
        if (src1_shape == src2_shape) {
          for (Addr_t i=Msize(src1_shape); i!=0; --i) {
            if (mem[src1_ptr++] == mem[src2_ptr++]) continue;
            equal = 0;
            break;
          }
        } else {
          equal = 0;
        }
        SET_REG(dest,equal)
        retcode |= DONE;
        break;
      }
      //--------------------------------------------------------------------------
      //{:Operation: Matrix transpose:}
      case TRANS: // M(dest) = transpose(M(src1));
      {
        retcode |= UNSUPPORTED_ERROR;
        break;
      }
      //--------------------------------------------------------------------------
      // Operation: Fill matrix with constant
      case FILL:  // M(dest) = R(src1); // fill matrix
      {
        VALIDATE_MATRIX(dest)
        VALIDATE_REGISTER(src1)
        Data_t fill;
        GET_REG(src1,fill)
        Addr_t dest_ptr;
        Data_t dest_shape;
        GET_REG(dest,dest_shape)
        GET_REG(dest+1,dest_ptr)
        FILL_LOOP:
        for (Addr_t i=Msize(dest_shape); i!=0; --i) {
          mem[dest_ptr++] = fill;
        }
        retcode |= DONE;
        break;
      }
      //--------------------------------------------------------------------------
      //{:Operation: Set matrix to identity:}
      case IDENT: // M(dest) = indentity(R2);
      {
        retcode |= UNSUPPORTED_ERROR;
        break;
      }
      //--------------------------------------------------------------------------
      // Operation: Copy register
      case RCOPY: // R(dest) = R(src1); // register copy
      {
        Addr_t val;
        VALIDATE_REGISTER(src1)
        GET_REG(src1,val)
        VALIDATE_REGISTER(dest)
        SET_REG(dest,val)
        retcode |= DONE;
        break;
      }
      //--------------------------------------------------------------------------
      // Operation: Set register low and sign extend
      case RSETX: // R(dest)[31:0] = (src1<<8)|src2; // register set hi
      {
        Addr_t val;
        VALIDATE_REGISTER(dest)
        GET_REG(dest,val)
        val = ((src1 << 8)|src2);
        val |= (val&0x8000) ? 0xFFFF0000 : 0; // sign extend
        SET_REG(dest,val)
        retcode |= DONE;
        break;
      }
      //--------------------------------------------------------------------------
      // Operation: Set register high half immediate
      case RSETH: // R(dest)[31:16] = (src1<<8)|src2; // register set hi
      {
        Addr_t val;
        VALIDATE_REGISTER(dest)
        GET_REG(dest,val)
        val &= 0xFFFF;
        val |= ((src1 << 8)|src2)<<16;
        SET_REG(dest,val)
        retcode |= DONE;
        break;
      }
      //--------------------------------------------------------------------------
      // Operation: Set register low half immediate
      case RSETL: // R(dest)[15:00] = (src1<<8)|src2; // register set lo
      {
        Addr_t val;
        VALIDATE_REGISTER(dest)
        GET_REG(dest,val)
        val &= ~0xFFFF;
        val |= ((src1 << 8)|src2);
        SET_REG(dest,val)
        retcode |= DONE;
        break;
      }
      //--------------------------------------------------------------------------
      default: // operation not implemented/supported
        {
          retcode |= UNSUPPORTED_ERROR;
          break;
        }
    }//endswitch

  }
  return retcode;
}
// The end
