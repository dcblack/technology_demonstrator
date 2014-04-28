#ifndef REPORT_H
#define REPORT_H
// -*- C++ -*- vim600:sw=2:tw=80:fdm=marker:fmr=<<<,>>>


///////////////////////////////////////////////////////////////////////////////
// $License: Apache 2.0 $
//
// This file is licensed under the Apache License, Version 2.0 (the "License").
// You may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

///////////////////////////////////////////////////////////////////////////////
/// DESCRIPTION
///
///   This file pair (report.h and report.cpp) provides several facilities to
///   simplify using SystemC. First some reporting macros that rely on
///   definition of a constant string either:
///
///   - namespace { static char const * const MSGID = "/COMPANY/PROJECT/SUBMODULE"; }
///   - #define MSGID "/COMPANY/PROJECT/SUBMODULE" /* use only in headers */
///
///   The macros are:
///
///   - REPORT_INFO_VERB(message_stream,verbosity); // IEEE1666-2011 only
///   - REPORT_INFO(message_stream);
///   - REPORT_WARNING(message_stream);
///   - REPORT_ERROR(message_stream);
///   - REPORT_FATAL(message_stream);
///
///   The following macros require #include <boost::format.hpp>
///
///   - REPORTF_INFO_VERB(format,elements,verbosity); // IEEE1666-2011 only
///   - REPORTF_INFO(format,elements);
///   - REPORTF_WARNING(format,elements);
///   - REPORTF_ERROR(format,elements);
///   - REPORTF_FATAL(format,elements);
///
///   The following macros allow throwing util::exception's and some common
///   (self-explanatory messages):
///
///   - THROW_ERROR(message_string);
///   - THROW_WARNING(message_string);
///   - THROW_INFO(message_string);
///   - NOT_YET_IMPLEMENTED; // error
///   - UNDER_CONSTRUCTION; // warning
///
///   There is also a report class, which is best installed by inheriting it
///   as the first base class in the top-most module. It provides a report 
///   handler which:
///
///   - adds time to SC_REPORT_INFO
///   - converts info messages to some new categories:
///
///   There is also a summary() method that provides information about runtime
///   performance (elaboration time separated from simulation time) and total
///   number of messages by type (info, warning, error, fatal).
///
///   Finally, there is a pause_on_exit class intended to be instantiated in
///   main to slow-down or prevent windows from disappearing upon exit.
///
///   - pause_on_exit object(bool wait_for_keypress); // always waits if WIN32

///////////////////////////////////////////////////////////////////////////////
#include <systemc>
#include <sstream>
#include <string>

#ifndef OVERRIDE
#if __cplusplus >= 201103L || __cplusplus > 199711L
//#if __has_feature(cxx_override_control) override
#define OVERRIDE override
//#endif
#endif
#ifndef OVERRIDE
#define OVERRIDE
#endif
#endif

///////////////////////////////////////////////////////////////////////////////
// The following macros allow for streaming syntax on sc_report by containing
// the stream inside the macro's arguments.
// LIMITATION: You cannot have functions with commas as arguments unless you
//             add an extra set of parentheses around them due to the way cpp
//             handles arguments.
///////////////////////////////////////////////////////////////////////////////

#define REPORT_INFO(message_stream) \
do {\
  std::ostringstream mout;\
  mout << message_stream;\
  SC_REPORT_INFO(MSGID,mout.str().c_str());\
} while (0)

#define REPORT_INFO_VERB(message_stream,verbosity) \
do {\
  std::ostringstream mout;\
  mout << message_stream;\
  SC_REPORT_INFO_VERB(MSGID,mout.str().c_str(),verbosity);\
} while (0)

#define REPORT_WARNING(message_stream) \
do {\
  std::ostringstream mout;\
  mout << message_stream;\
  SC_REPORT_WARNING(MSGID,mout.str().c_str());\
} while (0)

#define REPORT_ERROR(message_stream) \
try {\
  std::ostringstream mout;\
  mout << message_stream;\
  SC_REPORT_ERROR(MSGID,mout.str().c_str());\
} catch (sc_core::sc_report e) { }

#define REPORT_FATAL(message_stream) \
do {\
  std::ostringstream mout;\
  mout << message_stream;\
  SC_REPORT_FATAL(MSGID,mout.str().c_str());\
} while (0)

// The following are NON-STANDARD extensions supported by the report handler

#define REPORT_DATA(message_stream) \
do {\
  std::ostringstream mout;\
  mout << "DATA: " << message_stream;\
  SC_REPORT_INFO(MSGID,mout.str().c_str());\
} while (0)

// Like REPORT_INFO but adds file & line info
#define REPORT_NOTE(message_stream) \
do {\
  std::ostringstream mout;\
  mout << "NOTE: " << message_stream;\
  SC_REPORT_WARNING(MSGID,mout.str().c_str());\
} while (0)

#define REPORT_DEBUG(message_stream) \
do {\
  std::ostringstream mout;\
  mout << "DEBUG: " << message_stream;\
  SC_REPORT_WARNING(MSGID,mout.str().c_str());\
} while (0)

#define REPORT_ALERT(message_stream) \
do {\
  std::ostringstream mout;\
  mout << "ALERT: " << message_stream;\
  SC_REPORT_WARNING(MSGID,mout.str().c_str());\
} while (0)

///////////////////////////////////////////////////////////////////////////////
// The following macros allow for printf syntax on sc_report; however, due to
// use of boost::format, elements are separated by percent symbols (%) rather
// than commas (,).
//
// Examples:
//
//   REPORTF_INFO("Instance is %s on line // %d", name() % __LINE_);
//   REPORTF_ERROR("Mismatch data %f vs expected %f", actual % expected);
//
// Requires #include <boost/format.hpp> and linking with BOOST. Download
// freely from boost.org
///////////////////////////////////////////////////////////////////////////////
#define REPORTF_INFO_VERB(fmt,elts,verbosity) \
  SC_REPORT_INFO(MSGID,str(boost::format(fmt)%elts).c_str(),verbosity)

#define REPORTF_INFO(fmt,elts) \
  SC_REPORT_INFO(MSGID,str(boost::format(fmt)%elts).c_str())

#define REPORTF_WARNING(fmt,elts) \
  SC_REPORT_WARNING(MSGID,str(boost::format(fmt)%elts).c_str())

#define REPORTF_ERROR(fmt,elts) \
do { try {\
  SC_REPORT_ERROR(MSGID,str(boost::format(fmt)%elts).c_str());\
} catch (sc_core::sc_report e) { } } while(false)

#define REPORTF_FATAL(fmt,elts) \
  SC_REPORT_FATAL(MSGID,str(boost::format(fmt)%elts).c_str())


#define NOTE(arg)\
  REPORT_INFO(MSGID,"NOTE:" #arg " in process " \
      << sc_core::sc_get_current_process_handle().name() \
      << " on line " __LINE__ " of " __FILE__);

namespace util
{
  // Improved output - e.g. adds time to SC_REPORT_INFO
  void report_handler(const sc_core::sc_report& the_report,const sc_core::sc_actions& the_actions);

  int summary(const char * MSGID); //< Summarizes errors and returns exit code. Call once in sc_main as argument to return.

  bool separate_elaboration(void); //< Determines if EDA tool is separating elaboration from active simulation time

  // Returns wall clock time since epoch in milliseconds - reasonable for simulator
  // performance measurements. Works for Linux, OS X and Windows.
  uint64_t GetTimeMs64(void);

  // Convert a double representing seconds to string with appropriate timescale and annotate
  // e.g. 0.00005332001 => "53.32 us" (instead of SystemC's "5332001 ps")
  std::string seconds2str(double delta);

  // The following class initalizes reporting.
  struct report //< *** Instantiate as first data member in the top-most module with MSGID ***
  : sc_core::sc_behavior
  {
    report(char const * const msgid); // Constructor
    ~report(void); // Destructor
    const char * kind(void) const OVERRIDE { return "sc_behavior"; } // because default doesn't
    void start_of_simulation(void) OVERRIDE;
    void end_of_simulation(void) OVERRIDE;
    static void adjust_unexpected_errors(int   amount) { unexpected_error_count   += amount; }
    static void adjust_unexpected_warnings(int amount) { unexpected_warning_count += amount; }
    static void adjust_expected_errors(int     amount) { expected_error_count     += amount; }
    static void adjust_expected_warnings(int   amount) { expected_warning_count   += amount; }
  private:
    static const char * s_msgid;
  protected:
    friend void util::report_handler(const sc_core::sc_report&  the_report, const sc_core::sc_actions& the_actions);
    friend int  util::summary(const char * MSGID);
    static size_t unexpected_warning_count;
    static size_t expected_warning_count;
    static size_t unexpected_error_count;
    static size_t expected_error_count;
  };
  inline unsigned int errors(void) { // use this if you don't use summary
    return sc_core::sc_report_handler::get_count(sc_core::SC_ERROR)+sc_core::sc_report_handler::get_count(sc_core::SC_FATAL);
  }

  // The following class is used to capture information and throw it
  // upward to a catcher to make it simple to trap errors in a
  // single location (at the catch statement), but know the origin.
  struct exception
  : std::exception
  {
    const char * what(void) const throw() { return m_mesg; }
    sc_core::sc_severity m_type;
    const char *         m_mesg;
    const char *         m_file;
    std::size_t          m_line;
    exception
    ( sc_core::sc_severity type
    , const char *         mesg
    , const char *         file
    , std::size_t          line
    )
    : m_type(type)
    , m_mesg(mesg)
    , m_file(file)
    , m_line(line)
    {}
  };
}

// The following issue error messages at the point of invocation,
// and then throw an exception with the information.
#define THROW_ERROR(mesg) do {\
  SC_REPORT_ERROR(MSGID, mesg);\
  throw util::exception(sc_core::SC_ERROR,mesg,__FILE__,__LINE__);\
} while(false)
#define THROW_WARNING(mesg) do {\
  SC_REPORT_WARNING(MSGID, mesg);\
  throw util::exception(sc_core::SC_WARNING,mesg,__FILE__,__LINE__);\
} while(false)
#define THROW_INFO(mesg) do {\
  SC_REPORT_INFO(MSGID, mesg.c_str());\
  throw util::exception(sc_core::SC_INFO,mesg,__FILE__,__LINE__);\
} while(false)

#define NOT_YET_IMPLEMENTED do {\
  std::string nyi_mesg(__func__); nyi_mesg += " NOT YET IMPLEMENTED";\
  THROW_ERROR(nyi_mesg.c_str());\
} while(false)
#define UNDER_CONSTRUCTION do {\
  std::string nyi_mesg(__func__); nyi_mesg += " THIS CODE UNDER CONSTRUCTION -- YMMV";\
  THROW_WARNING(nyi_mesg.c_str());\
} while(false)

// Following keeps window open on exit -- helps when using Visual Studio console apps
#include <limits>
#include <sys/utsname.h>
#include <stdint.h>

namespace util
{
  class pause_on_exit //< instantiate at top of sc_main (OS agnostic)
  {
    public:
      pause_on_exit(bool wait_for_keypress=false); // Constructor
      ~pause_on_exit(void); // Destructor
    private:
      static void sighandler(int sig); // Signal handler
      static void notify(void); // Notification of exit
    private:
      static bool s_wait_for_keypress;
  };

  // Simply useful
  bool exists(char const * name); //< Test existance of environment variable
  bool is_number(const std::string& val); //< Test if string is numeric
  bool geti(std::istream& is, intmax_t& value);

  // Convert string to time. Use "_ns etc for the units".
  sc_core::sc_time get_time(const std::string& val);
}

#endif
