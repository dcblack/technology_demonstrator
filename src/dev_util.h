#ifndef DEV_UTIL_H
#define DEV_UTIL_H

#include "dev_hls.h"

extern const char* const reg_name[];
extern const char* const operation_name[];
extern const char* const status_name[];

void dev_hls
( volatile Data_t* reg_R0
, volatile Data_t* reg_R1
, volatile Data_t* reg_R2
, volatile Data_t* reg_R3
, volatile Data_t* reg_R4
, volatile Data_t* reg_R5
, volatile Data_t* reg_R6
, volatile Data_t* reg_R7
, volatile Data_t* reg_R8
, volatile Data_t* reg_R9
, volatile Data_t* reg_R10
, volatile Data_t* reg_R11
, volatile Data_t* reg_R12
, volatile Data_t* reg_R13
, volatile Data_t* reg_R14
, volatile Data_t* reg_R15
, volatile Data_t* reg_AXI_BASE
, volatile Data_t* reg_COMMAND
, volatile Data_t* reg_STATUS
, Data_t  imem[IMEM_SIZE]
, volatile Data_t* axibus
);

#endif /*DEV_UTIL_H*/
