// FILE: application.c

////////////////////////////////////////////////////////////////////////////////
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
////////////////////////////////////////////////////////////////////////////////
#if __STDC_VERSION__ < 199901L
#error Requires C99
#endif

#include "driver.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "myrand.h"
#include "creport.h"
#include "ctype.h"

int REGCNT  =  4; /* number of registers  in device*/
int TESTCNT = 20; /* number of tests to perform */

int application_main(int argc, char* argv[])
{
  my_srandom(1);

  int count[REGCNT];
  // Initialize
  for (int t=0; t!=REGCNT; ++t) {
    count[REGCNT] = 0;
  }

  //----------------------------------------------------------------------------
  // Perform TESTCNT tests
  //----------------------------------------------------------------------------
  int data;
  for (int i=0; i!=TESTCNT; ++i) {
    /* Write to all registers */
    for (int t=0; t!=REGCNT; ++t) {
//    if (count[t] != 0) break; /*< not yet done */
//    if (my_random()&1) break; /* 50% chance */
      data = abs(my_random()%5000) + 1000; /*< 1000..5999 */
//    REPORT_DEBUG("Attempting to put...\n");
      if (dev_put(DEV_COUNT1_REG+4*t,(unsigned char*)(&data),sizeof(data))<0) {
        REPORT_ERROR("Unable to set DEV_COUNT%d\n",t+1);
      } else {
        count[t] = data; /*< indicate in progress */
      }
    }//endfor t=0..REGCNT-1
    //dev_wait();
    /* Check status */
    if (dev_get(DEV_STATUS_REG,(unsigned char*)(&data),sizeof(data))<0) {
      REPORT_ERROR("Unable to get DEV_STATUS\n");
    }
    REPORT_INFO("Status = %04x\n",data);
    for (int t=0; t!=REGCNT; ++t) {
      if (dev_get(DEV_COUNT1_REG+4*t,(unsigned char*)(&data),sizeof(data))<0) {
        REPORT_ERROR("Unable to get DEV_COUNT%d\n",t+1);
      } else {
        REPORT_INFO("DEV_COUNT%d = %04x\n",t+1,data);
        count[t] = data; /*< indicate current value */
      }
    }//endfor t=0..REGCNT-1
  }//endfor i=0..19

  REPORT_INFO("Exit application\n");
  return error_status();
}//endmain
