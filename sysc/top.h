#ifndef TOP_H
#define TOP_H
///////////////////////////////////////////////////////////////////////////////
// Top-level module where everything is hooked up.

#include <systemc>
#include <memory>
#include "report.h"
#include "tcpip_initiator.h"
#include "local_initiator.h"
class Dev_module;
class Mem_module;
class Bus_module;

class Top_module
: public sc_core::sc_module
{
  util::report setup;/*< used to extend reporting -- ONLY do this in the topmost module */
public:
  // Ports - NONE (this is the top-level)
  // Channels - NONE (everything is TLM 2.0 - i.e. effectively modules ARE channels due to sockets)
  // Structure (e.g. submodules)
  // NOTE:  C++11 unique_ptr<> is *safe alternative to raw pointers* and better than C++03 auto_ptr<>
  std::unique_ptr<tcpip_initiator_module> tcpip_initiator;
  std::unique_ptr<Local_initiator_module> local_initiator;
  std::unique_ptr<Dev_module>             dev;
  std::unique_ptr<Mem_module>             mem;
  std::unique_ptr<Bus_module>             bus;

  // Constructor
  Top_module(sc_core::sc_module_name instance_name);
  // Destructor
  virtual ~Top_module(void);
  // Callbacks
  void before_end_of_elaboration(void) override; //< e.g. modify netlist (rare)
  void end_of_elaboration(void) override; //< e.g. add sc_trace
  void start_of_simulation(void) override; //< e.g. channel initializations
  void end_of_simulation(void) override; //< e.g. cleanup, statistics
  // Processes
  void top_thread(void);
};

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

#endif
