#include "dev_util.h"
using namespace std;

const char * const reg_name[] =
{ "R0/M0shape"
, "R1/M0array"
, "R2/M1shape"
, "R3/M1array"
, "R4/M2shape"
, "R5/M2array"
, "R6/M3shape"
, "R7/M3array"
, "R8/M4shape"
, "R9/M4array"
, "R10/M5shape"
, "R11/M5array"
, "R12/M6shape"
, "R13/M6array"
, "R14/SR"
, "R15/PC"
, "AXI_BASE"
, "COMMAND"
, "STATUS"
};

const char * const operation_name[] =
{ "NOP  "
, "LOAD "
, "STORE"
, "MCOPY"
, "MADD "
, "MSUB "
, "RSUB "
, "MMUL "
, "KMUL "
, "MSUM "
, "MDET0"
, "EQUAL"
, "MZERO"
, "TRANS"
, "FILL "
, "IDENT"
, "RMOVE"
, "RSETX"
, "RSETH"
, "RSETL"
, "RESET"
, "EXEC "
, "HALT "
};

string cmd_state_name(Data_t status)
{
  string msg;
  if (status == 0) msg += "IDLE ";
  if (status & START            ) msg += "START ";
  if (status & BUSY             ) msg += "BUSY ";
  if (status & DONE             ) msg += "DONE ";
  if (status & HALTED           ) msg += "HALTED ";
  if (status & UNSUPPORTED_ERROR) msg += "UNSUPPORTED_ERROR ";
  if (status & ADDRESS_ERROR    ) msg += "ADDRESS_ERROR ";
  if (status & GENERIC_ERROR    ) msg += "GENERIC_ERROR ";
  if (status & SHAPE_ERROR      ) msg += "SHAPE_ERROR ";
  if (status & REGISTER_ERROR   ) msg += "REGISTER_ERROR ";
  if (status & SHAPE_MISMATCH   ) msg += "SHAPE_MISMATCH ";
  if (status & UNKNOWN_STATUS   ) msg += "UNKNOWN_STATUS ";
  return msg;
}

//EOF
