#ifndef CREPORT_H
#define CREPORT_H

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

typedef unsigned long long int debug_t;
typedef unsigned long long int count_t;
extern debug_t debug_level;
extern count_t warning_count;
extern count_t error_count;
extern count_t fatal_count;

#define REPORT_INFO\
  printf("INFO %s@%s:%d: ",MSGID,__FILE__,__LINE__); fflush(stdout); printf

#define REPORT_DEBUG(message)\
  do {\
    if (debug_level > 1) {\
      printf("DEBUG %s@%s:%d: ",MSGID,__FILE__,__LINE__);\
      printf(message);\
      fflush(stdout);\
    }\
  } while (0)

#define REPORT_WARNING\
  ++warning_count; printf("WARNING %s@%s:%d: ",MSGID,__FILE__,__LINE__); fflush(stdout); printf

#define REPORT_ERROR\
  ++error_count; printf("ERROR %s@%s:%d: ",MSGID,__FILE__,__LINE__); fflush(stdout); printf

#define REPORT_FATAL\
  ++fatal_count; printf("FATAL %s@%s:%d: ",MSGID,__FILE__,__LINE__); fflush(stdout); printf

#define BREAK_HERE(message)\
  do { if (debug_level > 0) breakpoint(message); } while (0)

void report_summary(void);
int error_status(void);
void breakpoint(const char* message);

#endif /*CREPORT_H*/
