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

typedef unsigned long long int verbosity_t;
typedef unsigned long long int count_t;
extern verbosity_t verbosity;
extern count_t warning_count;
extern count_t error_count;
extern count_t fatal_count;

#define ALWAYS_LEVEL   0
#define NOISY_LEVEL  100
#define LOW_LEVEL    200
#define MEDIUM_LEVEL 300
#define HIGH_LEVEL   400
#define DEBUG_LEVEL  500
#define BREAK_LEVEL  600

void init_term(const char* path);

#ifdef SILENT
#define REPORT_INFO(...)
#define REPORT_INFO_VERB(verbosity_level,...)
#define REPORT_DEBUG(message)
#define REPORT_WARNING(...) ++warning_count
#define REPORT_ERROR(...) ++error_count
#define REPORT_FATAL(...) ++fatal_count
#define BREAK_HERE(message)
#define NOT_YET_IMPLEMENTED
#define UNDER_CONSTRUCTION
#define NEWLINE
#else
#define REPORT_INFO(...)\
  do { fprintf(stdout,"INFO %s@%s:%d: ",MSGID,__FILE__,__LINE__); fflush(stdout); fprintf(stdout,__VA_ARGS__); } while(0)

#define REPORT_INFO_VERB(verbosity_level,...)                     \
  do {                                                            \
    if (verbosity >= verbosity_level) {                           \
      fprintf(stdout,"INFO %s@%s:%d: ",MSGID,__FILE__,__LINE__);  \
      fflush(stdout);                                             \
      fprintf(stdout,__VA_ARGS__);                                \
    }                                                             \
  } while(0)

#define REPORT_DEBUG(...)                                         \
  do {                                                            \
    if (verbosity >= DEBUG_LEVEL) {                               \
      fprintf(stdout,"DEBUG %s@%s:%d: ",MSGID,__FILE__,__LINE__); \
      fprintf(stdout,__VA_ARGS__);                                \
      fflush(stdout);                                             \
    }                                                             \
  } while (0)

#define REPORT_WARNING(...)                                       \
  do {                                                            \
    ++warning_count;                                              \
    fprintf(stdout,"WARNING %s@%s:%d: ",MSGID,__FILE__,__LINE__); \
    fflush(stdout); fprintf(stdout,__VA_ARGS__);                  \
  } while(0)

#define REPORT_ERROR(...)                                         \
  do {                                                            \
    ++error_count;                                                \
    fprintf(stdout,"ERROR %s@%s:%d: ",MSGID,__FILE__,__LINE__);   \
    fflush(stdout);                                               \
    fprintf(stdout,__VA_ARGS__);                                  \
  } while(0)

#define REPORT_FATAL(...)                                         \
  do {                                                            \
    ++fatal_count;                                                \
    fprintf(stdout,"FATAL %s@%s:%d: ",MSGID,__FILE__,__LINE__);   \
    fflush(stdout);                                               \
    fprintf(stdout,__VA_ARGS__);                                  \
  } while(0)

#define BREAK_HERE(message)                                       \
  do {                                                            \
    if (verbosity > BREAK_LEVEL) breakpoint(message);             \
  } while (0)
#endif
#define NOT_YET_IMPLEMENTED REPORT_ERROR("NOT YET IMPLEMENTED")
#define UNDER_CONSTRUCTION  REPORT_WARNING("THIS CODE UNDER CONSTRUCTION -- YMMV")
#define NEWLINE             \
  do {                      \
    fprintf(stdout,"\n");   \
    fflush(stdout);         \
  } while(0)

void report_summary(void);
int error_status(void);
void breakpoint(const char* message);

#endif /*CREPORT_H*/
