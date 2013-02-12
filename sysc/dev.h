#ifndef DEV_H
#define DEV_H

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
#include "tlm_utils/simple_target_socket.h"
#include <stdint.h>

struct dev_module
: sc_core::sc_module
{
  // Ports
  tlm_utils::simple_target_socket<dev_module> target_socket;
  // Constructor
  dev_module
  ( sc_core::sc_module_name instance_name
  );
  // Destructor
  virtual ~dev_module(void);
  // SC_MODULE callbacks - NONE
  // TLM-2 forward methods
  void b_transport  ( tlm::tlm_generic_payload& trans, sc_core::sc_time& delay );
  unsigned int  transport_dbg( tlm::tlm_generic_payload& trans );
private:
  sc_dt::uint64    m_register_count; //< number of registers in this device
  int32_t*         m_register; // register array
  int              m_byte_width; //< byte width of socket
  sc_core::sc_time m_latency;
};

#endif
