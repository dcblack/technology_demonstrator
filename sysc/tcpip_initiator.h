#ifndef TCPIP_INITIATOR_H
#define TCPIP_INITIATOR_H
///////////////////////////////////////////////////////////////////////////////
// Interfaces from the TCPIP to initiate transfers.

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

#include "tlmx_channel.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "memory_manager.h"
#include <systemc>
#include <thread>
#include <mutex>

struct tcpip_initiator_module
: sc_core::sc_module
{
public:
  // Ports
  tlm_utils::simple_initiator_socket<tcpip_initiator_module> initiator_socket;
  // Channels
  tlmx_channel             m_async_channel;
  sc_core::sc_buffer<bool> m_keep_alive_signal;
  // Constructor
  tcpip_initiator_module(sc_core::sc_module_name instance_name);
  // Destructor
  virtual ~tcpip_initiator_module(void);
  // SC_MODULE callbacks
  void before_end_of_elaboration(void) override;
  void end_of_elaboration(void) override;
  void start_of_simulation(void) override;
  void end_of_simulation(void) override;
  // SystemC processes
  void initiator_sysc_thread_process(void);
  void keep_alive_process(void);
private:
  // External OS thread
  void async_os_thread(tlmx_channel& channel);
  // Signal handler
  typedef void (*sig_t) (int);
  static void sighandler(int sig);
  // Module attributes/local data
  int          m_tcpip_port;
  std::mutex   m_allow_pthread; //< must be declared before m_lock_permission
  std::unique_ptr<std::lock_guard<std::mutex>> m_lock_permission; //< must be declared before m_pthread
  std::thread  m_pthread;
  memory_manager* m_memory_manager; // Memory manager
  static int   s_stop_requests;
};

#endif
