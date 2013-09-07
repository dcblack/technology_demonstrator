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

const char * const cmd_state_name[] =
{ "IDLE"               /* 00 */
, "START"              /* 01 */
, "BUSY"               /* 02 */
, "DONE"               /* 03 */
, "HALTED"             /* 04 */
, "UNSUPPORTED_ERROR"  /* 05 */
, "ADDRESS_ERROR"      /* 06 */
, "GENERIC_ERROR"      /* 07 */
, "UNKNOWN_ERROR_08"   /* 08 */
, "UNKNOWN_ERROR_09"   /* 09 */
, "UNKNOWN_ERROR_0A"   /* 0A */
, "UNKNOWN_ERROR_0B"   /* 0B */
, "UNKNOWN_ERROR_0C"   /* 0C */
, "UNKNOWN_ERROR_0D"   /* 0D */
, "UNKNOWN_ERROR_0E"   /* 0E */
, "UNKNOWN_ERROR_0F"   /* 0F */
, "R0_SHAPE_ERROR"     /* 10 */
, "R1_SHAPE_ERROR"     /* 11 */
, "R2_SHAPE_ERROR"     /* 12 */
, "R3_SHAPE_ERROR"     /* 13 */
, "R4_SHAPE_ERROR"     /* 14 */
, "R5_SHAPE_ERROR"     /* 15 */
, "R6_SHAPE_ERROR"     /* 16 */
, "R7_SHAPE_ERROR"     /* 17 */
, "R8_SHAPE_ERROR"     /* 18 */
, "R9_SHAPE_ERROR"     /* 19 */
, "R10_SHAPE_ERROR"    /* 1A */
, "R11_SHAPE_ERROR"    /* 1B */
, "R12_SHAPE_ERROR"    /* 1C */
, "R13_SHAPE_ERROR"    /* 1D */
, "R14_SHAPE_ERROR"    /* 1E */
, "R15_SHAPE_ERROR"    /* 1F */
, "R0_REGISTER_ERROR"  /* 20 */
, "R1_REGISTER_ERROR"  /* 21 */
, "R2_REGISTER_ERROR"  /* 22 */
, "R3_REGISTER_ERROR"  /* 23 */
, "R4_REGISTER_ERROR"  /* 24 */
, "R5_REGISTER_ERROR"  /* 25 */
, "R6_REGISTER_ERROR"  /* 26 */
, "R7_REGISTER_ERROR"  /* 27 */
, "R8_REGISTER_ERROR"  /* 28 */
, "R9_REGISTER_ERROR"  /* 29 */
, "R10_REGISTER_ERROR" /* 2A */
, "R11_REGISTER_ERROR" /* 2B */
, "R12_REGISTER_ERROR" /* 2C */
, "R13_REGISTER_ERROR" /* 2D */
, "R14_REGISTER_ERROR" /* 2E */
, "R15_REGISTER_ERROR" /* 2F */
, "SHAPE_MISMATCH"     /* 30 */
, "UNKNOWN_STATUS"     /* 31 */
};

//EOF
