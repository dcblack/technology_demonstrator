//BEGIN report.cpp (systemc)
// -*- C++ -*- vim600:sw=2:tw=80:fdm=marker:fmr=<<<,>>>
///////////////////////////////////////////////////////////////////////////////
// $Info: Helpful reporting utilities - see report.h for documentation $

///////////////////////////////////////////////////////////////////////////////

#include "report.h"
#include <time.h>
#include <libgen.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <set>
#include <sys/utsname.h>
#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <ctime>
#include <signal.h>
#endif

using namespace sc_core;
using namespace std;

namespace
{
  // Declare string used as message identifier in SC_REPORT_* calls
  static char const * const MSGID = "/Doulos/example/report";\
  // Embed file version information into object to help forensics
  static char const * const RCSID = "$Id: report.cpp 1.1 2012/08/30 11:45:11 dcblack Exp $";\
  //                                      FILENAME   VER DATE       TIME     USERNAME
}

namespace util
{
  struct counter {
    char const *       name;
    unsigned long int count;
  };

  // Data
  static string const       HRULE(70,'~'); //< horizontal ruler
  // Variables to track simulation time (0 means "not yet set")
  uint64_t                  g_elaboration_start_ms = 0;
  uint64_t                  g_simulation_start_ms  = 0;
  uint64_t                  g_simulation_finish_ms = 0;
  // For error situation..
  static string             s_badlog("");
  static char const *       severity_names[] = 
  { "INFO"
  , "WARNING"
  , "ERROR"
  , "FATAL"
  , "UNKNOWN"
  };
  // Following translate INFO & WARNING messages to different text
  static char const * info_names[] = 
  { "DATA"
  , "WARNING"
  , "ERROR"
  , "LOGONLY"
  , "DEBUG"
  , "OPTION"
  , "HELP"
  , "NOTE"
  , "ALERT"
  , "ENTER"
  , "LEAVE"
  , 0
  };
  static counter nonwarnings[] = 
  { { "DEBUG", 0UL }
  , { "NOTE",  0UL }
  , { "ALERT", 0UL }
  , { 0,       0UL }
  };
  static vector<int> changed(4,0);

  //////////////////////////////////////////////////////////////////////////////
  // Functions
  //----------------------------------------------------------------------------
  // Return true if environment variable exists
  //----------------------------------------------------------------------------
  bool exists(char const * name) //< Test existance of environment variable
  {
    char * value = getenv(name);
    if (value != 0) {
      return true; // non-empty
    }//endif
    if (setenv(name,"x",0) == -1) {;
      return true; // already exists
    } else {
      unsetenv(name);
      return false;
    }
  }//end exists()

  //----------------------------------------------------------------------------
  // Return true if string contains only decimal digits -- useful when
  // parsing sc_argv()[i].
  //----------------------------------------------------------------------------
  bool is_number(const string& val)
  {
    if (val.size() == 0) return false;
    size_t pos;
    if (val.find("0x") == 0) {
      return (val.find_first_not_of("0123456789ABCDEF",2) == string::npos);
    } else if ((pos = val.find(".")) != string::npos) {
      return (val.find_first_not_of("0123456789",0,pos-1) == string::npos) and (val.find_first_not_of("0123456789",pos+1) == string::npos);
    } else {
      return (val.find_first_not_of("0123456789") == string::npos);
    }//endif
  }//end is_number()

  sc_time get_time(const string& val)
  {
    double       tv;
    sc_time_unit tu;
    size_t pos;
    if      ((pos = val.find("_ms"))  != string::npos and is_number(val.substr(0,pos))) tu = SC_MS;
    else if ((pos = val.find("_us"))  != string::npos and is_number(val.substr(0,pos))) tu = SC_US;
    else if ((pos = val.find("_ns"))  != string::npos and is_number(val.substr(0,pos))) tu = SC_NS;
    else if ((pos = val.find("_ps"))  != string::npos and is_number(val.substr(0,pos))) tu = SC_PS;
    else if ((pos = val.find("_fs"))  != string::npos and is_number(val.substr(0,pos))) tu = SC_FS;
    else if ((pos = val.find("_sec")) != string::npos and is_number(val.substr(0,pos))) tu = SC_SEC;
    else return sc_max_time();
    istringstream is(val.substr(0,pos));
    is >> tv;
    return sc_time(tv,tu);
  }

  //----------------------------------------------------------------------------
  // Read a number from input stream returning true if successful. Number may
  // be formated as decimal, hex or octal.
  //----------------------------------------------------------------------------
  bool geti(istream& is, intmax_t& value) 
  {
    bool ok(true);
    while (isblank(is.peek())) {
      is.get();
    }//endwhile
    char c(is.peek());
    if ('0' == c) { // hex or octal
      is.get(); // toss 0
      c = is.peek();
      if ('x' == c or 'X' == c) { // hex
        is.get(); // toss X
        is >> hex >> value;
      } else { // octal
        is.unget(); // restore 0
        is >> oct >> value;
      }//endif
    } else if (isdigit(c)) { // decimal
      is >> dec >> value;
    } else { // unknown
      ok = false;
    }//endif
    return ok;
  }//end geti

  //----------------------------------------------------------------------------
  // Returns the amount of milliseconds elapsed since the UNIX epoch. Works on
  // Linux, Windows and MacOSX.
  //----------------------------------------------------------------------------
  uint64_t GetTimeMs64(void)
  {
#ifdef WIN32
    /* Windows */
    FILETIME ft;
    LARGE_INTEGER li;
    
    /* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
     * to a LARGE_INTEGER structure. */
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    
    uint64_t ret = li.QuadPart;
    ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
    ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */
    
    return ret;
#else
    /* Linux and MacOSX */
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    
    uint64_t ret = tv.tv_usec;
    /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
    ret /= 1000;
    
    /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
    ret += (tv.tv_sec * 1000);
    
    return ret;
#endif
  }//end GetTimeMs64

  //----------------------------------------------------------------------------
  inline char const * severity_name(unsigned int severity) {
    return severity_names[(severity<4)?severity:3];
  }

  //----------------------------------------------------------------------------
  void interrupt_here( char const * id, sc_severity severity ) {
    // you can set a breakpoint at some of the lines below, either to
    // interrupt with any severity, or to interrupt with a specific severity

    switch( severity ) {
      case SC_INFO: 
        static char const * info_id;
        info_id = id;
        break;
      case SC_WARNING: 
        static char const * warning_id;
        warning_id = id;
        break;
      case SC_ERROR: 
        static char const * error_id;
        error_id = id;
        break;
      default:
      case SC_FATAL: 
        static char const * fatal_id;
        fatal_id = id;
        break;
    }//endswitch
  }//end interrupt_here()


  //----------------------------------------------------------------------------
  //  FUNCTION : stop_here
  //
  //  Debugging aid for warning, error, and fatal reports.
  //  This function *cannot* be inlined.
  //----------------------------------------------------------------------------
  void stop_here( char const * id, sc_severity severity ) {
    // you can set a breakpoint at some of the lines below, either to
    // stop with any severity, or to stop with a specific severity

    switch( severity ) {
      case SC_INFO: 
        static char const * info_id;
        info_id = id;
        break;
      case SC_WARNING: 
        static char const * warning_id;
        warning_id = id;
        break;
      case SC_ERROR: 
        static char const * error_id;
        error_id = id;
        break;
      default:
      case SC_FATAL: 
        static char const * fatal_id;
        fatal_id = id;
        break;
    }//endswitch
  }//end stop_here()

  //----------------------------------------------------------------------------
  static ofstream * clog(0);
  static struct auto_close_log {
    ~auto_close_log() {
      delete clog;
      sc_report_handler::set_log_file_name(0);
      clog = 0;
    }//end destructor
  } auto_close;

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  bool separate_elaboration(void)
  {
#ifdef NC_SYSTEMC
    return ncsc_in_elaborator();
#else
    if (util::exists("ELABORATE_ONLY")) {
      string val = string(getenv("ELABORATE_ONLY"));
      return is_number(val) && strtol(val.c_str(),0,0) != 0;
    } else {
    return false;
    }//endif
#endif
  }//end separate_elaboration()

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  static const string report_compose_message
  ( const sc_report&  the_report
  , const string the_msg
  )
  {
    static ostringstream msg_stream;
    msg_stream.str("");
    // Only output a time if simulation is currently active; otherwise, 
    // indicate we are outside active simulation
    if (sc_end_of_simulation_invoked()) {
      msg_stream << "! s: "; // simulation complete
    } else if (sc_start_of_simulation_invoked()) {
      msg_stream << the_report.get_time() << ": ";
    } else {
      msg_stream << "- s: "; // simulation not started
    }//endif
    string prefix(msg_stream.str().length(),' ');
    msg_stream <<  severity_name(the_report.get_severity()) << ": ";

    if ( the_report.get_id() >= 0 ) { // backward compatibility with 2.0+
      msg_stream << "IWEF"[the_report.get_severity()] << the_report.get_id() << " ";
    }//endif
    msg_stream << the_report.get_msg_type();
    if( the_msg != "" ) {
      msg_stream << ": " << the_msg;
    }//endif
    if( the_report.get_severity() > SC_INFO or the_msg.find("DEBUG:") != string::npos) {
      msg_stream << endl << prefix << "In file: ";
      msg_stream << the_report.get_file_name() << ":" << the_report.get_line_number();

#if (defined(SYSTEMC_VERS) && SYSTEMC_VERS >= 2002000) || (defined(SYSTEMC_VERSION) && SYSTEMC_VERSION >= 2002000)
      if( sc_is_running() )
#else
      // Backwards compatible
      sc_simcontext * simc = sc_get_curr_simcontext();
      if( simc && simc->is_running() )
#endif
      {
        char const * proc_name = the_report.get_process_name();
        if( proc_name ) {
          msg_stream << ", process: " << proc_name;
        }//endif
      }//endif
    }//endif

    return msg_stream.str().c_str();
  }//end report_compose_message()

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void report_handler
  ( const sc_report&  the_report
  , const sc_actions& the_actions
  )
  {

    string the_msg = report_compose_message(the_report,the_report.get_msg());

    //--------------------------------------------------------------------------
    // Adjust for extended message severity names
    // e.g. SC_REPORT_INFO(MSGID,"DEBUG: blah") => "...INFO: MSGID: DEBUG: blah" => "...DEBUG: MSGID: blah"
    //--------------------------------------------------------------------------
    static const string guard(": ");
    if (the_report.get_severity() == SC_INFO) {
      string info_str(severity_names[SC_INFO]);
      info_str += guard;
      size_t info_pos(the_msg.find(info_str));
      for (int i=0; info_names[i]!=0; ++i) {
        size_t pos;
        string extd(info_names[i]);
        extd = guard + extd + guard;
        pos = the_msg.find(extd);
        if (pos == string::npos or pos < info_pos) continue;
        the_msg.replace(pos,extd.size(),guard);
        the_msg.replace(info_pos,strlen(severity_names[SC_INFO]),info_names[i],extd.size()-2*guard.size());
        break;
      }//endfor
      // Fix so-called runt MSGID
      {
        static const string runt_MSGID(": -: ");
        size_t pos(the_msg.find(runt_MSGID));
        if (pos != string::npos) { // Remove runt MSGID
          the_msg.replace(pos,runt_MSGID.size(),":");
        }//endif
      }
    } else if (the_report.get_severity() == SC_WARNING) {
      string warn_str(severity_names[SC_WARNING]);
      warn_str += guard;
      size_t warn_pos(the_msg.find(warn_str));
      for (int i=0; nonwarnings[i].name!=0; ++i) {
        size_t pos;
        string extd(nonwarnings[i].name);
        extd = guard + extd + guard;
        pos = the_msg.find(extd);
        if (pos == string::npos or pos < warn_pos) continue;
        the_msg.replace(pos,extd.size(),guard);
        the_msg.replace(warn_pos,strlen(severity_names[SC_WARNING]),nonwarnings[i].name,extd.size()-2*guard.size());
        nonwarnings[i].count++;
        break;
      }//endfor
    }//endif

    sc_actions new_actions = the_actions;
    bool logonly(the_msg.find("LOGONLY:") != string::npos);

    //----------------------------------------------------------------------------
    // Display messages
    // > from /eda/osci/src/systemc-2.1.v1/src/sysc/utils/sc_report_handler.cpp
    //----------------------------------------------------------------------------
    if ( new_actions & SC_DISPLAY && ! logonly) {
      cout << the_msg << endl;
    }//endif
    if ( (new_actions & SC_LOG) && sc_report_handler::get_log_file_name() ) {
      if (! clog) {
        clog = new ofstream(sc_report_handler::get_log_file_name()); // ios::trunc
        // What if opening logfile failed?
        if (!*clog) {
          cerr << endl << endl 
            << "**********************************************************************" << endl
            << "**********************************************************************" << endl
            << "ERROR: Unable to open simulation logfile for writing!? Attempting /tmp"
            << endl;
          s_badlog = string("/tmp/");
          char * logfile = const_cast<char *>(sc_report_handler::get_log_file_name());
          s_badlog += basename(logfile);
          clog = new ofstream(s_badlog.c_str()); // ios::trunc
          if (!*clog) {
            cerr << "FATAL: Failed opening '" << s_badlog << "'. Aborting..." << endl
              << "**********************************************************************" << endl
              << "**********************************************************************" << endl
              ;
            abort();
          } else {
            cerr << "INFO: Succeeded opening '" << s_badlog << "'. Proceeding..." << endl;
          }//endif
        }//endif
      }//endif
      *clog << the_msg << endl;
    }//endif
    if ( new_actions & SC_STOP ) {
      util::stop_here(the_report.get_msg_type(), the_report.get_severity());
      sc_stop();
    }//endif
    if ( new_actions & SC_INTERRUPT ) {
      util::interrupt_here(the_report.get_msg_type(), the_report.get_severity());
    }//endif
    if ( new_actions & SC_ABORT ) {
      util::summary(util::report::s_msgid);
      delete clog;
      abort();
    }//endif
    if ( new_actions & SC_THROW ) {
      throw the_report; 
    }//endif
  }//end report_handler()

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  size_t report::unexpected_warning_count(0);
  size_t report::expected_warning_count(0);
  size_t report::unexpected_error_count(0);
  size_t report::expected_error_count(0);
  report::report(char const * const msgid) // Constructor
  : sc_module(sc_module_name("report"))
  {
    static int once_only(0); //< Just in case this gets instantiated more than once
    if (once_only++) return;
    s_msgid = msgid;
    g_elaboration_start_ms = GetTimeMs64();
    sc_report_handler::set_actions( SC_FATAL,   SC_DISPLAY|SC_LOG|SC_ABORT );
    sc_report_handler::set_actions( SC_ERROR,   SC_DISPLAY|SC_LOG|SC_INTERRUPT|SC_THROW );
    sc_report_handler::set_actions( SC_WARNING, SC_DISPLAY|SC_LOG|SC_INTERRUPT );
    sc_report_handler::set_actions( SC_INFO,    SC_DISPLAY|SC_LOG );
    sc_report_handler::set_verbosity_level(SC_MEDIUM);
    sc_report_handler::set_handler(&report_handler);
  }

  char const* report::s_msgid = "";

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  report::~report(void) // Destructor
  {
    summary(s_msgid);
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void report::start_of_simulation(void)
  {
    g_simulation_start_ms = GetTimeMs64();
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void report::end_of_simulation(void)
  {
    g_simulation_finish_ms = GetTimeMs64();
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  string seconds2str(double delta) {
    ostringstream fraction;
    fraction << setprecision(2) << fixed;
    if (delta == 0.0) {
      return string("0 s");
    } else if (delta >= 1.0) {
      fraction << delta << " secs";
      return fraction.str();
    } else if (delta >= 1e-3) {
      fraction << delta * 1e3 << " ms";
      return fraction.str();
    } else if (delta >= 1e-6) {
      fraction << delta * 1e6 << " us";
      return fraction.str();
    } else if (delta >= 1e-9) {
      fraction << delta * 1e9 << " ns";
      return fraction.str();
    } else if (delta >= 1e-12) {
      fraction << delta * 1e12 << " ps";
      return fraction.str();
    } else {
      fraction << delta * 1e15 << " fs";
      return fraction.str();
    }//endif
  }//end seconds2str()


  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  int summary(char const * msgtyp)
  {
    static bool executed_once = false;
    if (executed_once) {
      SC_REPORT_WARNING(msgtyp,"Attempt to call report summary more than once!?");
    }//endif
    executed_once = true;

    //--------------------------------------------------------------------------
    // Calculate run time
    //--------------------------------------------------------------------------
    double elaboration_duration = double(g_simulation_start_ms  - g_elaboration_start_ms)/1000.0;
    double simulation_duration  = double(g_simulation_finish_ms - g_simulation_start_ms)/1000.0;
    //--------------------------------------------------------------------------
    // Summarize messaging & return with pass/fail status
    //--------------------------------------------------------------------------
    unsigned long int debug_count(0UL);
    for (int i=0;nonwarnings[i].name!=0;++i) debug_count += nonwarnings[i].count;
    unsigned long int info_count     = sc_report_handler::get_count(SC_INFO);
    unsigned long int warning_count  = sc_report_handler::get_count(SC_WARNING) - debug_count;
    unsigned long int error_count    = sc_report_handler::get_count(SC_ERROR);
    unsigned long int fatal_count    = sc_report_handler::get_count(SC_FATAL);
    error_count   += report::unexpected_error_count   - report::expected_error_count;
    warning_count += report::unexpected_warning_count - report::expected_warning_count;
    cout
      << "\n" << HRULE << "\n"
      << "SUMMARY: " << MSGID << "\n"
      << "  " << setw(4) << info_count    << " informational messages" << "\n"
      ;
    for (int i=0; nonwarnings[i].name!=0; ++i) {
      if (nonwarnings[i].count == 0) continue;
      cout << "  " << setw(4) << nonwarnings[i].count << " " << nonwarnings[i].name << " messages" << "\n";
    }//endfor
    cout
      << "  " << setw(4) << warning_count << " warning messages" << "\n"
      << "  " << setw(4) << error_count   << " errors" << "\n"
      << "  " << setw(4) << fatal_count   << " fatalities" << "\n"
      << HRULE << endl
      ;
    bool success = ((fatal_count + error_count) == 0);
    time_t now = time(NULL);
    if (! separate_elaboration()) {
      cout
        << "COMMAND LINE\n"
        << "   % ";
      for (int i=0; i<sc_argc(); ++i) {
        cout << " " << sc_argv()[i];
      }
      cout << endl;
      cout
        << "EXECUTION SUMMARY\n"
        << "  Finish time: " << ctime(&now) 
        << "  Elaboration: " << seconds2str(elaboration_duration) << " cpu" << "\n"
        << "  Simulation : " << seconds2str(simulation_duration)  << " cpu" << "\n"
        << HRULE
        << endl;
    }//endif
    {
      utsname info;
      if (uname(&info) == 0) {
      cout
        << "MACHINE INFORMATION\n"
        << "  " << info.version << "\n"
        << endl
        ;
      }//endif
    }
    if (success) {
      cout << "PASSED - No errors detected." << endl;
    } else {
      cout << "FAILED" << endl;
    }//endif
    return (success?0:1);
  }//end summary()

  //----------------------------------------------------------------------------
  // Following assumes existence of a shell script/program 'header'
  // that outputs large text to get the attention of anyone observing
  // the log output
  //----------------------------------------------------------------------------
  void header(string mesg)
  {
    string command("header -hbar \"");
    command += mesg + "\"";
    system(command.c_str());
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  pause_on_exit::pause_on_exit(bool wait_for_keypress) // Constructor
  {
    s_wait_for_keypress |= wait_for_keypress;
    signal(SIGINT, &sighandler); //< register with UNIX
    signal(SIGABRT, &sighandler); //< register with UNIX
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  pause_on_exit::~pause_on_exit(void) { // Destructor
    notify();
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void pause_on_exit::sighandler(int sig) { // Signal handler
    printf("POSIX Signal %d caught...\n", sig);
    pause_on_exit::notify();
    exit(1);
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  void pause_on_exit::notify(void) { // Notification of exit
    static bool notified(false);
    if (notified) return;
    notified = true;
    if (s_wait_for_keypress) {
      printf("Press ENTER to exit...");
      char c;
      do {
        c=getc(stdin);
      } while (c != EOF && c != '\n');
    }//endif
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
# ifdef _WIN32
    bool pause_on_exit::s_wait_for_keypress(true);
# else
    bool pause_on_exit::s_wait_for_keypress(false);
# endif

}//endnamespace util

//------------------------------------------------------------------------------
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  L<http://www.apache.org/licenses/LICENSE-2.0>
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//------------------------------------------------------------------------------
// The end!
