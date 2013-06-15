/**********************/
/* Vivado HLS Source */
/********************/
#include "ap_int.h"
#include "dev_hls.h"

#if 0
Data_t saturate(ap_int<64> t)
{
  if (t>0 && (t>>32))  return (~0)^(1<<31);
  if (t<0 && ~(t>>32)) return (1<<31);
  return t;
}
#endif

void dev_hls(Addr_t* reg, Data_t *xmem)
{

  // Define the RTL interfaces
  #pragma HLS interface ap_ctrl_none port=return
  /**pragma HLS interface ap_bus port=xmem**/
  // Define the pcore interface as an AXI4 master
  #pragma HLS resource core=AXI4M variable=xmem

  // Define extra AXI4Lite slave port for controlling purposes 
  /**pragma HLS interface ap_hs port=reg**/
  /**pragma HLS resource core=AXI4LiteS port=reg**/

  reg[STATUS] = BUSY;

  ap_int<32> data1, data2;
  ap_int<64> data0;

  switch(reg[COMMAND]) {
    case NOP:
      {
        reg[STATUS] = DONE;
        break;
      }
    case STOP:
      {
        reg[STATUS] = HALTED;
        break;
      }
    case MCOPY:
      {
        int M0_shape = reg[M0_SHAPE];
        int M1_shape = reg[M1_SHAPE];
        if (M0_shape != M1_shape) {
          reg[STATUS] = SHAPE_ERROR;
        } else {
          memcpy(&xmem[reg[M0_ARRAY]], &xmem[reg[M1_ARRAY]], Msize(M0_shape) * sizeof(Data_t));
          reg[STATUS] = DONE;
        }
        break;
      }
    case MADD:
      {
        int M0_shape = reg[M0_SHAPE];
        int M1_shape = reg[M1_SHAPE];
        int M2_shape = reg[M2_SHAPE];
        if (M0_shape != M1_shape || M0_shape != M2_shape) {
          reg[STATUS] = SHAPE_ERROR;
        } else {
          Addr_t M0_ptr = reg[M0_ARRAY];
          Addr_t M1_ptr = reg[M1_ARRAY];
          Addr_t M2_ptr = reg[M2_ARRAY];
          for (Addr_t i=Msize(M0_shape); i!=0; --i) {
            data1 = xmem[M1_ptr++];
            data2 = xmem[M2_ptr++];
            data0 = data1 + data2;
            //{:TODO:Detect over/underflow:}
            xmem[M0_ptr++] = data0;
          }
          reg[STATUS] = DONE;
        }
        break;
      }
    case MSUB:
      {
        int M0_shape = reg[M0_SHAPE];
        int M1_shape = reg[M1_SHAPE];
        int M2_shape = reg[M2_SHAPE];
        if (M0_shape != M1_shape || M0_shape != M2_shape) {
          reg[STATUS] = SHAPE_ERROR;
        } else {
          Addr_t M0_ptr = reg[M0_ARRAY];
          Addr_t M1_ptr = reg[M1_ARRAY];
          Addr_t M2_ptr = reg[M2_ARRAY];
          for (Addr_t i=Msize(M0_shape); i!=0; --i) {
            data1 = xmem[M1_ptr++];
            data2 = xmem[M2_ptr++];
            data0 = data1 - data2;
            //{:TODO:Detect over/underflow:}
            xmem[M0_ptr++] = data0;
          }
          reg[STATUS] = DONE;
        }
        break;
      }
    case RSUB:
      {
        int M0_shape = reg[M0_SHAPE];
        int M1_shape = reg[M1_SHAPE];
        int M2_shape = reg[M2_SHAPE];
        if (M0_shape != M1_shape || M0_shape != M2_shape) {
          reg[STATUS] = SHAPE_ERROR;
        } else {
          Addr_t M0_ptr = reg[M0_ARRAY];
          Addr_t M1_ptr = reg[M1_ARRAY];
          Addr_t M2_ptr = reg[M2_ARRAY];
          for (Addr_t i=Msize(M0_shape); i!=0; --i) {
            data1 = xmem[M1_ptr++];
            data2 = xmem[M2_ptr++];
            data0 = data2 - data1;
            //{:TODO:Detect over/underflow:}
            xmem[M0_ptr++] = data0;
          }
          reg[STATUS] = DONE;
        }
        break;
      }
    case KMUL:
      {
        int M0_shape = reg[M0_SHAPE];
        int M1_shape = reg[M1_SHAPE];
        int k1 = reg[K1];
        if (M0_shape != M1_shape) {
          reg[STATUS] = SHAPE_ERROR;
        } else {
          Addr_t M0_ptr = reg[M0_ARRAY];
          Addr_t M1_ptr = reg[M1_ARRAY];
          Addr_t M2_ptr = reg[M2_ARRAY];
          for (Addr_t i=Msize(M0_shape); i!=0; --i) {
            data1 = xmem[M1_ptr++];
            data0 = k1*data1;
            //{:TODO:Detect over/underflow:}
            xmem[M0_ptr++] = data0;
          }
          reg[STATUS] = DONE;
        }
        break;
      }
    //{:TODO - additional commands:}
    default: // command not implemented/supported
      {
        reg[STATUS] = UNSUPPORTED_ERROR;
        break;
      }
  }//endswitch

}
