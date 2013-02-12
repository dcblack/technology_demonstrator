//BEGIN main.cpp (systemc)
// -*- C++ -*- vim600:sw=2:tw=80:fdm=marker:fmr=<<<,>>>

///////////////////////////////////////////////////////////////////////////////
// $Info: Prototypical sc_main $
//
// This is a slightly fancier sc_main than traditionally used that manages
// exceptions and errors more gracefully than the simplistic version used by
// most SystemC examples.

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
#include "top.h"
#include "report.h"
using namespace sc_core;
using namespace std;

namespace {
  // Declare string used as message identifier in SC_REPORT_* calls
  static char const * const MSGID = "/Doulos/example/main";
  // Embed file version information into object to help forensics
  static char const * const RCSID = "(@)$Id: main.cpp  1.0 09/02/12 10:00 dcblack $";
  //                                         FILENAME  VER DATE     TIME  USERNAME
}

///////////////////////////////////////////////////////////////////////////////
int
sc_main
( int argc
, char *argv[]
)
{
  util::pause_on_exit if_windows;
  // Elaborate
  top_module* top_instance; //< sc_module that also inherits from special report class
  try {
    top_instance = new top_module("top_instance");
  } catch (std::exception& e) {
    SC_REPORT_ERROR(MSGID,(string(e.what())+" Please fix elaboration errors and retry.").c_str());
    return 1;
  } catch (...) {
    SC_REPORT_ERROR(MSGID,"Caught exception during elaboration");
    return 1;
  }//endtry

  // Simulate
  try {
    SC_REPORT_INFO(MSGID,"Starting kernal");
    sc_start();
    SC_REPORT_INFO(MSGID,"Exited kernal");
  } catch (std::exception& e) {
    SC_REPORT_WARNING(MSGID,(string("Caught exception ")+e.what()).c_str());
  } catch (...) {
    SC_REPORT_ERROR(MSGID,"Caught exception during simulation.");
  }//endtry
  if (not sc_end_of_simulation_invoked()) {
    SC_REPORT_INFO(MSGID,"ERROR: Simulation stopped without explicit sc_stop()");
    sc_stop();
  }//endif

  delete top_instance;
  return (util::errors()?1:0);
}//end main
///////////////////////////////////////////////////////////////////////////////
//END @() $Id: main.cpp,v 1.0 2012/09/01 05:20:48 dcblack Exp $
