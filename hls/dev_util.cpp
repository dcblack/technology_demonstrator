#include "dev_util.h"
#include <iostream>
#include <iomanip>
using namespace std;

const char * const reg_name[] =
{ "M0_ARRAY"
, "M1_ARRAY"
, "M2_ARRAY"
, "M3_ARRAY"
, "M4_ARRAY"
, "M5_ARRAY"
, "M6_ARRAY"
, "M7_ARRAY"
, "M8_ARRAY"
, "M9_ARRAY"
, "MA_ARRAY"
, "MB_ARRAY"
, "MC_ARRAY"
, "MD_ARRAY"
, "ME_ARRAY"
, "MF_ARRAY"
, "M0_SHAPE"
, "M1_SHAPE"
, "M2_SHAPE"
, "M3_SHAPE"
, "M4_SHAPE"
, "M5_SHAPE"
, "M6_SHAPE"
, "M7_SHAPE"
, "M8_SHAPE"
, "M9_SHAPE"
, "MA_SHAPE"
, "MB_SHAPE"
, "MC_SHAPE"
, "MD_SHAPE"
, "ME_SHAPE"
, "MF_SHAPE"
, "R0"
, "K1"
, "K2"
, "K3"
, "K4"
, "K5"
, "K6"
, "K7"
, "K8"
, "K9"
, "COMMAND"
, "STATUS"
};

const char * const command_name[] =
{ "NOP"
, "LOAD"
, "STORE"
, "MCOPY"
, "MADD"
, "MSUB"
, "RSUB"
, "MMUL"
, "KMUL"
, "MSUM"
, "MDET0"
};

const char * const status_name[] =
{ "IDLE"
, "BUSY"
, "DONE"
, "HALTED"
, "UNSUPPORTED_ERROR"
, "SHAPE_ERROR"
, "ADDRESS_ERROR"
, "GENERIC_ERROR"
};

void zero_regs(void)
{
  for (int i=STATUS; i>0; --i) {
    reg[i] = 0;
  }
}

void dump_regs(void)
{
  for (int i=STATUS; i>0; --i) {
    cout
      << "reg[" << reg_name[i] << "] = 0x"
      << setw(8) << hex << uppercase << setfill('0') << reg[i]
      << endl;
  }
}

void dump_array(Reg_t matrix)
{
  Addr_t cols = Mcols(reg[SHAPE+matrix]);
  Addr_t rows = Mrows(reg[SHAPE+matrix]);
  Addr_t mptr = reg[ARRAY+matrix];

  // Heading
  cout << "M" << matrix << "/ ";
  for (Addr_t col=0; col!=cols; ++col) {
    if (col != 0) cout << ", ";
    cout << setw(2) << dec << col;
  }
  cout << endl;

  // Separator
  cout << "----";
  for (Addr_t col=0; col!=cols; ++col) {
    if (col != 0) cout << "--";
    cout << "--";
  }
  cout << endl;

  // Out data
  for (Addr_t row=0; row!=rows; ++row) {
    cout << setw(2) << dec << row << ": ";
    // Output a row
    for (Addr_t col=0; col!=cols; ++col) {
      cout << setw(2) << dec << xmem[mptr++] << ", ";
    }
    cout << endl;
  }
  cout << endl;
}

//EOF
