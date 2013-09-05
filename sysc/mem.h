#ifndef MEM_H
#define MEM_H

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

#include "tlm_utils/simple_target_socket.h"
#include <systemc>
#include "devtypes.h"
#include "sc_literals.h"

struct Mem_module
: sc_core::sc_module
{
  // Ports
  tlm_utils::simple_target_socket<Mem_module>    target_socket;
  // Constructor
  Mem_module
  ( sc_core::sc_module_name instance_name
  , size_t           depth
  , sc_core::sc_time read_latency  = 10_ns
  , sc_core::sc_time write_latency = 10_ns
  );
  // Destructor
  virtual ~Mem_module(void);
  // SC_MODULE callbacks - NONE
  // TLM-2 forward methods
  void     b_transport        ( tlm::tlm_generic_payload& trans, sc_core::sc_time& delay );
  unsigned transport_dbg      ( tlm::tlm_generic_payload& trans );
  bool     get_direct_mem_ptr ( tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data );
private:
  sc_dt::uint64     m_depth; //< memory locations
  sc_core::sc_time  m_read_latency;
  sc_core::sc_time  m_write_latency;
  Data_t*           m_memory; // register array
  int               m_byte_width; //< byte width of socket
};

#endif
