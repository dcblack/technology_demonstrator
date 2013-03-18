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

#ifdef _ZEDBOARD
#define STDOUT /dev/tty
#else
#define STDOUT /dev/tty
#endif

verbosity_t verbosity   = 
#ifdef DEBUG
DEBUG_LEVEL
#else
#ifdef VERBOSE
NOISY_LEVEL
#else
#ifdef QUIET
HIGH_LEVEL
#else
MEDIUM_LEVEL
#endif
#endif
#endif
;
debug_t debug_level   = 0;
count_t warning_count = 0;
count_t error_count   = 0;
count_t fatal_count   = 0;

void report_summary(void)
{
#ifndef SILENT
  fprintf(stdout,"REPORT SUMMARY\n  %3lld warnings\n  %3lld errors\n  %3lld fatals\n", warning_count, error_count, fatal_count);
  fflush(stdout);
#endif
}

int error_status(void)
{
  return (error_count+fatal_count)?1:0;
}

void breakpoint(const char* message)
{
#ifndef SILENT
  fprintf(stdout,"BREAK: %s\n",message);
  fflush(stdout);
#endif
}
