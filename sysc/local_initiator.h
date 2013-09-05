#ifndef LOCAL_INITIATOR_H
#define LOCAL_INITIATOR_H

#include <systemc>
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/tlm_quantumkeeper.h"
#include "memory_manager.h"
#include "sc_literals.h"
#include "devtypes.h"
#include "software.h"
#include <map>

struct Local_initiator_module
: sc_core::sc_module
{
public:
  // Ports
  tlm_utils::simple_initiator_socket<Local_initiator_module>     initiator_socket;
  sc_core::sc_export<sc_core::sc_signal_out_if<sc_dt::sc_logic>> interrupt_export;
  sc_core::sc_signal_resolved                                    interrupt_signal;
  // Channels - NONE
  // Constructor
  Local_initiator_module(sc_core::sc_module_name instance_name);
  // Destructor
  virtual ~Local_initiator_module(void);
  // SC_MODULE callbacks
  void before_end_of_elaboration(void) override;
  void end_of_elaboration(void) override;
  void start_of_simulation(void) override;
  void end_of_simulation(void) override;
  void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,sc_dt::uint64 end_range);
  // SystemC processes
  void initiator_thread(void);
  // Helper methods
  void     get_mem (size_t n); //< initializes m_memory
  Byte_ptr malloc  (size_t n);
  void     free    (Byte_ptr ptr);
  void mem_write   (size_t waddr, int* data_ptr, size_t words);
  void mem_read    (size_t waddr, int* data_ptr, size_t words);
  void mem_write   (size_t waddr, int& data);
  void mem_read    (size_t waddr, int& data);
  int  mem_read    (size_t waddr);
  void dev_write   (size_t waddr, int* data_ptr, size_t words);
  void dev_read    (size_t waddr, int* data_ptr, size_t words);
  void dev_write   (size_t waddr, int& data);
  void dev_read    (size_t waddr, int& data);
  int  dev_read    (size_t waddr);
  void cycles      (size_t clocks) { m_qk.set_and_sync(clocks*m_period); }
  void sync        (void) { m_qk.sync(); }
  void clear_irq   (void) {
    //interrupt_signal.write(sc_dt::SC_LOGIC_0); //< let dev do this
  }
  void enable_irq  (void) { interrupt_enabled = true; }
  void disable_irq (void) { interrupt_enabled = false; }
  void wait_irq    (void) { wait(interrupt_event); }
  void wait_os     (void) { wait(scheduler_event); }
  void wait_dev    (void) {
    wait(sc_core::SC_ZERO_TIME);
    if (interrupt_signal.read() != sc_dt::SC_LOGIC_1)
      wait(interrupt_signal.posedge_event());
  }
  void resume_os   (void) {
    interrupt_active = false;
    scheduler_event.notify(sc_core::SC_ZERO_TIME);
  }
private:
  // Module attributes/local data
  memory_manager*              m_memory_manager; // Memory manager
  std::vector<tlm::tlm_dmi>    dmi_table; // Table of valid DMI regions
  tlm_utils::tlm_quantumkeeper m_qk;      // Quantum keeper for temporal decoupling
  Byte_ptr                     m_memory;  // Used for malloc
  size_t                       m_free;    // Numer of free locations
  std::map<size_t,size_t>      m_used;    // Tracks malloc'd used addresses and their size
  sc_core::sc_time             m_period;  // Clock period used to calculate cycles
  bool                         interrupt_enabled;
  sc_core::sc_event            interrupt_event;
  sc_core::sc_event            scheduler_event;
  bool                         interrupt_active;
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
///////////////////////////////////////////////////////////////////////////////

#endif
