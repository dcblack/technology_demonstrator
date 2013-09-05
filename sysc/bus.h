#ifndef BUS_H
#define BUS_H

#include <systemc>
#include <tlm>
#include <tlm_utils/multi_passthrough_initiator_socket.h>
#include <tlm_utils/multi_passthrough_target_socket.h>
#include "sc_literals.h"
#include <map>

struct Bus_module : sc_core::sc_module
{
  tlm_utils::multi_passthrough_target_socket<Bus_module, 32>    target_socket;
  tlm_utils::multi_passthrough_initiator_socket<Bus_module, 32> initiator_socket;

  SC_CTOR(Bus_module);
  void end_of_elaboration();

  // Forward interface
  void b_transport( int id, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay );
  tlm::tlm_sync_enum nb_transport_fw( int id, tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, sc_core::sc_time& delay );
  bool get_direct_mem_ptr( int id, tlm::tlm_generic_payload& trans,
                                           tlm::tlm_dmi& dmi_data);
  unsigned int transport_dbg( int id, tlm::tlm_generic_payload& trans );

  // Backward interface
  tlm::tlm_sync_enum nb_transport_bw( int id, tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, sc_core::sc_time& delay );
  void invalidate_direct_mem_ptr( int id, sc_dt::uint64 start_range,
                                                  sc_dt::uint64 end_range );
  unsigned int decode_address( sc_dt::uint64 address, sc_dt::uint64& masked_address );
  sc_dt::uint64 reconstruct_address ( sc_dt::uint64 address, unsigned int target );

private:
  std::map <tlm::tlm_generic_payload*, unsigned int> m_id_map;
};

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
//
#endif /*BUS_H*/
