#ifndef TOP_H
#define TOP_H
///////////////////////////////////////////////////////////////////////////////
// Top-level module where everything is hooked up.

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

#include <systemc>
#include <memory>
#include "report.h"
#include "zynq.h"
#include "dev.h"

class top_module
: public sc_core::sc_module
{
  util::report setup;/*< used to extend reporting -- ONLY do this in the topmost module */
public:
  // Ports - NONE
  // Structure (e.g. submodules)
  std::unique_ptr<zynq_module> zynq_instance; // ** safe alternative to raw pointers **
  std::unique_ptr<dev_module>  dev_instance;  // ** replace with unique_ptr under C++11 **
  // Constructor
  top_module(sc_core::sc_module_name instance_name);
  // Destructor
  virtual ~top_module(void);
  // Callbacks
  void before_end_of_elaboration(void); //< e.g. modify netlist (rare)
  void end_of_elaboration(void); //< e.g. add sc_trace
  void start_of_simulation(void); //< e.g. channel initializations
  void end_of_simulation(void); //< e.g. cleanup, statistics
  // Processes
  void top_thread(void);
};

#endif
