//BEGIN top.cpp (systemc)
// -*- C++ -*- vim600:sw=2:tw=80:fdm=marker:fmr=<<<,>>>
///////////////////////////////////////////////////////////////////////////////
// $Info: Prototypical top module implementation $
//
// Basic top-level module where everything is hooked up.
//
// Use -tcpip or -local to configure at run-time.
//
//  Networked to Zedboard     Local platform
// +-----------------------+ +----------------------------------------------+
// |  top      #           | |   top                                        |
// |           #ETHER      | |                                              |
// |  +--------V--------+  | |   +-----------------+                        |
// |  | tcpip_initiator |  | |   | local_initiator |                        |
// |  +--------V--------+  | |   +---^----v--------+                        |
// |           |           | |    IRQ:    |                                 |
// |           |BUS        | | ......:    |                                 |
// |           |           | | :          |                                 |
// |  +--------V--------+  | | : +--------v------------------------------+  |
// |  | dev             |  | | : | bus                                   |  |
// |  +--------V----v---+  | | : +--------v----^----------------v--------+  |
// |           |    :      | | :          |    |                |           |
// |           |AXI :IRQ   | | :......    |    |                |           |
// |           |    :      | |       :    |    |                |           |
// |  +--------V----v---+  | |   +---^----v----^---+   +--------v--------+  |
// |  | tcpip_target    |  | |   | dev 0..0x0FFFFF |   | mem 0x100000..  |  |
// |  +--------V--------+  | |   +-----------------+   +-----------------+  |
// |           #ETHER      | |                                              |
// +-----------------------+ +----------------------------------------------+

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

#include "top.h"
#include "tcpip_initiator.h"
#include "local_initiator.h"
#include "dev.h"
#include "mem.h"
#include "bus.h"
#include "report.h"
#include "netlist.h"
#include <string>
using namespace sc_core;
using namespace std;

namespace {
  // Declare string used as message identifier in SC_REPORT_* calls
  static char const* const MSGID="/Doulos/example/Top_module";
  // Embed file version information into object to help forensics
  static char const* const RCSID="(@)$Id: top.cpp,v 1.0 2013/02/04 17:54:49 dcblack Exp $";
  //                                      FILENAME VER DATE     TIME  USERNAME
}

///////////////////////////////////////////////////////////////////////////////
// Constructor <<
Top_module::Top_module(sc_module_name instance_name)
: sc_module(instance_name), setup(MSGID)
, tcpip_initiator ()
, local_initiator ()
, dev             ()
, mem             ()
, bus             ()
// TODO: replace with an entire subsystem
{
  bool use_tcpip = false; //< assume local_initiator

  //----------------------------------------------------------------------------
  // Parse command-line arguments
  //----------------------------------------------------------------------------
  for (int i=1; i<sc_argc(); ++i) {
    string arg(sc_argv()[i]);
    if      (arg.find("-debug")  == 0) sc_report_handler::set_verbosity_level(SC_DEBUG);
    else if (arg.find("-quiet")  == 0) sc_report_handler::set_verbosity_level(SC_NONE);
    else if (arg.find("-low")    == 0) sc_report_handler::set_verbosity_level(SC_LOW);
    else if (arg.find("-medium") == 0) sc_report_handler::set_verbosity_level(SC_MEDIUM);
    else if (arg.find("-high")   == 0) sc_report_handler::set_verbosity_level(SC_HIGH);
    else if (arg.find("-full")   == 0) sc_report_handler::set_verbosity_level(SC_FULL);
    else if (arg.find("-tcpip")  == 0) use_tcpip = true;
    else if (arg.find("-local")  == 0) use_tcpip = false;
  }//endfor
  
  //----------------------------------------------------------------------------
  // Report configuration
  //----------------------------------------------------------------------------
  REPORT_INFO("\n"
           << "===================================================================================\n"
           << "CONFIGURATION(" << name() << ")\n"
           << ">   Verbosity is " << sc_report_handler::get_verbosity_level() << "\n"
           << "===================================================================================\n"
           );

  dev              .reset(new Dev_module             ("dev"            )  );
  if (use_tcpip) {
    tcpip_initiator.reset( new tcpip_initiator_module("tcpip_initiator")  );
    bus            .reset( new Bus_module            ("bus"            )  );
#ifdef HAVE_TCPIP_TARGET
    tcpip_target   .reset( new tcpip_target_module   ("tcpip_target"   )  );
#endif
    // Connectivity
    tcpip_initiator->initiator_socket ( bus->target_socket                );
    bus            ->initiator_socket ( dev->target_socket                );
#ifdef HAVE_TCPIP_TARGET
    dev            ->initiator_socket ( tcpip_target->target_socket       );
    dev            ->interrupt_port   ( tcpip_target->interrupt_export    );
#endif
  } else {
    local_initiator.reset( new Local_initiator_module("local_initiator")  );
    mem            .reset( new Mem_module            ("mem", 1024*1024 )  );
    bus            .reset( new Bus_module            ("bus"            )  );
    // Connectivity
    local_initiator->initiator_socket ( bus->target_socket                );
    dev            ->initiator_socket ( bus->target_socket                );
    bus            ->initiator_socket ( dev->target_socket                );
    bus            ->initiator_socket ( mem->target_socket                );
    dev            ->interrupt_port   ( local_initiator->interrupt_export );
  }//endif

  // Register processes
  SC_HAS_PROCESS(Top_module);
  SC_THREAD(top_thread);

  REPORT_INFO("Constructed " << " " << name());
}//endconstructor

///////////////////////////////////////////////////////////////////////////////
// Destructor <<
Top_module::~Top_module(void) { REPORT_INFO("Destroyed " << name()); }

///////////////////////////////////////////////////////////////////////////////
// Callbacks
void Top_module::before_end_of_elaboration(void) { REPORT_INFO(__func__ << " " << name()); }
void Top_module::end_of_elaboration(void) {
  REPORT_INFO(__func__ << " " << name());
  util::netlist();
  // Add sc_trace(...)
  // Configure memory map...
}
void Top_module::start_of_simulation(void) 
{
  REPORT_INFO(__func__ << " " << name());
}

void Top_module::end_of_simulation(void) {
  REPORT_INFO(__func__ << " " << name());
}

///////////////////////////////////////////////////////////////////////////////
// Processes <<
void Top_module::top_thread(void)  {
  REPORT_INFO(__func__ << " " << name());
  // Nothing to do
}

//EOF
