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
, "RSETH"
, "RSETL"
, "RESET"
, "HALT "
};

const char * const status_name[] =
{ "IDLE"
, "START"
, "BUSY"
, "DONE"
, "HALTED"
, "UNSUPPORTED_ERROR"
, "SHAPE_ERROR"
, "REGISTER_ERROR"
, "ADDRESS_ERROR"
, "GENERIC_ERROR"
};

//EOF
