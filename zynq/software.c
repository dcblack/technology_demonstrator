// FILE: software.c

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

#include "driver.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "creport.h"

static const char* MSGID="/Doulos/example/software";

#define REGCNT 4
#define TESTCNT 20

int main(int argc, char* argv[])
{
  long int port = 0;
  char* endptr = 0;

  srandom(1);

  /* Parse command-line */
  if (argc != 3) {
    REPORT_ERROR("Syntax: %s HOSTNAME PORTNUMBER\n",argv[0]);
  }
  port = strtol(argv[2],&endptr,10);
  if (port == 0L) {
    REPORT_ERROR("Unable to parse port number.\n");
  } else if (port < 2000 || port > 65535) {
    REPORT_ERROR("Bad port number (%ld) specified. Port number restricted to between 2000 and 65535.\n",port);
  }
  if (error_count) {
    printf("Please fix above errors and retry.\n");
    return 1;
  }

  dev_open(argv[1],port);
  REPORT_INFO("Device connection opened...\n");
  int count[REGCNT];
  // Initialize
  for (int t=0; t!=REGCNT; ++t) {
    count[REGCNT] = 0;
  }
  int data;
  // Perform TESTCNT tests
  for (int i=0; i!=TESTCNT; ++i) {
    /* Write to all registers */
    for (int t=0; t!=REGCNT; ++t) {
//    if (count[t] != 0) break; /*< not yet done */
//    if (random()&1) break; /* 50% chance */
      data = abs(random()%5000) + 1000; /*< 1000..5999 */
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
  dev_close();
  REPORT_INFO("Exiting\n");
  report_summary();
  return error_status();
}//endmain
