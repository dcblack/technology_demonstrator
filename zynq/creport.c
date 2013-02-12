// FILE: creport.c

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

#include "creport.h"
#include <stdlib.h>
#include <stdio.h>

debug_t debug_level   = 
#ifdef DEBUG
1
#else
0
#endif
;
count_t warning_count = 0;
count_t error_count   = 0;
count_t fatal_count   = 0;

void report_summary(void)
{
  printf("REPORT SUMMARY\n  %3d warnings\n  %3d errors\n  %3d fatals\n", warning_count, error_count, fatal_count);
  fflush(stdout);
}

int error_status(void)
{
  return (error_count+fatal_count)?1:0;
}

void breakpoint(const char* message)
{
  printf("BREAK: %s\n",message);
  fflush(stdout);
}
