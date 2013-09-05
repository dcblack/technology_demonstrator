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
#if __STDC_VERSION__ < 199901L
#error Requires C99
#endif

#include "driver.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "creport.h"
#include "ctype.h"

static const char* MSGID="/Doulos/example/software";

extern int appl_main(int argc, char* argv[]);

int startup_main(int argc, char* argv[])
{
  int  port_arr[3] = {4000, 4001, 4100};
  char* begptr  = NULL;
  char* endptr  = NULL;

  //----------------------------------------------------------------------------
  // Parse command-line
  //----------------------------------------------------------------------------
  if (argc != 2 && argc != 3) {
    REPORT_ERROR("Syntax: %s HOSTNAME PORTLIST\n",argv[0]);
  }
  if (argc >= 3) begptr = argv[2];
  for (int i=0; i!= 3; ++i) {
    long int port;
    port = 0;
    if (begptr == NULL || *begptr == '\0') break;
    port = strtol(begptr,&endptr,10);
    if (endptr != NULL && strchr(",:;",*endptr) != NULL) begptr = endptr + 1;
    else                begptr = NULL;
    if (port == 0L) {
      REPORT_ERROR("Unable to parse port number.\n");
    } else if (port < 2000 || port > 65535) {
      REPORT_ERROR("Bad port number (%ld) specified. Port number restricted to between 2000 and 65535.\n",port);
    } else {
      port_arr[i] = port;
    }//endif
  }//endfor
  if (error_count) {
    REPORT_INFO("Please fix above errors and retry.\n");
    return 1;
  }

  dev_open(argv[1],port_arr);
  REPORT_INFO("Device connection opened...\n");
}

int shutdown_main(int argc, char* argv[])
{
  //----------------------------------------------------------------------------
  // Exit
  //----------------------------------------------------------------------------
  dev_close();
  REPORT_INFO("Exiting Pseudo-OS\n");
}//endmain

int main(int argc, char* argv[])
{
  startup_main(argc,argv);
  application_main(argc,argv);
  shutdown_main(argc,argv);
  report_summary();
  return error_status();
}
