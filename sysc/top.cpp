//BEGIN top.cpp (systemc)
// -*- C++ -*- vim600:sw=2:tw=80:fdm=marker:fmr=<<<,>>>
///////////////////////////////////////////////////////////////////////////////
// $Info: Prototypical top module implementation $
//
// Basic top-level module where everything is hooked up.
//
// +--------------------------------+
// |  top_instance                  |
// |                                |
// |  +--------------------------+  |
// |  | tcpip_initiator_instance |  |
// |  +-----------V--------------+  |
// |              V                 |
// |              |                 |
// |              V                 |
// |  +-----------V--------------+  |
// |  | dev_instance             |  |
// |  +--------------------------+  |
// |                                |
// +--------------------------------+

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
#include "dev.h"
#include "report.h"
#include "netlist.h"
using namespace sc_core;

namespace {
  // Declare string used as message identifier in SC_REPORT_* calls
  static char const* const MSGID="/Doulos/example/top_module";
  // Embed file version information into object to help forensics
  static char const* const RCSID="(@)$Id: top.cpp,v 1.0 2013/02/04 17:54:49 dcblack Exp $";
  //                                      FILENAME VER DATE     TIME  USERNAME
}

///////////////////////////////////////////////////////////////////////////////
// Constructor <<
top_module::top_module(sc_module_name instance_name)
: sc_module(instance_name), setup(MSGID)
, tcpip_initiator_instance (new tcpip_initiator_module("tcpip_initiator_instance")) //< initiates transactions from zynq via tcpip sockets
, dev_instance             (new dev_module            ("dev_instance" )           ) //< device being modeled
// TODO: replace with an entire subsystem
{
  // Connectivity
  tcpip_initiator_instance->initiator_socket(dev_instance->target_socket);

  // Register processes
  SC_HAS_PROCESS(top_module);
  SC_THREAD(top_thread);

  REPORT_INFO("Constructed " << " " << name());
}//endconstructor

///////////////////////////////////////////////////////////////////////////////
// Destructor <<
top_module::~top_module(void) { REPORT_INFO("Destroyed " << name()); }

///////////////////////////////////////////////////////////////////////////////
// Callbacks
void top_module::before_end_of_elaboration(void) { REPORT_INFO(__func__ << " " << name()); }
void top_module::end_of_elaboration(void) {
  REPORT_INFO(__func__ << " " << name());
  util::netlist();
  // Add sc_trace(...)
  // Configure memory map...
}
void top_module::start_of_simulation(void) 
{
  REPORT_INFO(__func__ << " " << name());
}

void top_module::end_of_simulation(void) {
  REPORT_INFO(__func__ << " " << name());
}

///////////////////////////////////////////////////////////////////////////////
// Processes <<
void top_module::top_thread(void)  {
  REPORT_INFO(__func__ << " " << name());
  // Nothing to do
}

//EOF
