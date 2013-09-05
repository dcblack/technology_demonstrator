#include "bus.h"
#include "sc_literals.h"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

Bus_module::Bus_module(sc_module_name instance_name)
: target_socket("target_socket")
, initiator_socket("initiator_socket")
{
  target_socket.register_b_transport              (this, &Bus_module::b_transport);
  target_socket.register_nb_transport_fw          (this, &Bus_module::nb_transport_fw);
  target_socket.register_get_direct_mem_ptr       (this, &Bus_module::get_direct_mem_ptr);
  target_socket.register_transport_dbg            (this, &Bus_module::transport_dbg);
  initiator_socket.register_nb_transport_bw          (this, &Bus_module::nb_transport_bw);
  initiator_socket.register_invalidate_direct_mem_ptr(this, &Bus_module::invalidate_direct_mem_ptr);
}

void Bus_module::end_of_elaboration()
{
  //{:TODO:Report bindings:}
}

// Forward interface

void Bus_module::b_transport( int id, tlm::tlm_generic_payload& trans, sc_time& delay )
{
  sc_dt::uint64 masked_address;
  unsigned int target = decode_address( trans.get_address(), masked_address );
  trans.set_address( masked_address );

  initiator_socket[target]->b_transport( trans, delay );
}


tlm::tlm_sync_enum Bus_module::nb_transport_fw( int id, tlm::tlm_generic_payload& trans,
                                            tlm::tlm_phase& phase, sc_time& delay )
{
  m_id_map[ &trans ] = id;

  sc_dt::uint64 masked_address;
  unsigned int target = decode_address( trans.get_address(), masked_address );
  trans.set_address( masked_address );

  return initiator_socket[target]->nb_transport_fw( trans, phase, delay );
}

bool Bus_module::get_direct_mem_ptr( int id, tlm::tlm_generic_payload& trans,
                                         tlm::tlm_dmi& dmi_data)
{
  sc_dt::uint64 masked_address;
  unsigned int target = decode_address( trans.get_address(), masked_address );
  trans.set_address( masked_address );

  bool status = initiator_socket[target]->get_direct_mem_ptr( trans, dmi_data );

  dmi_data.set_start_address( reconstruct_address(dmi_data.get_start_address(), target) );
  dmi_data.set_end_address(   reconstruct_address(dmi_data.get_end_address(), target) );

  return status;
}

unsigned int Bus_module::transport_dbg( int id, tlm::tlm_generic_payload& trans )
{
  sc_dt::uint64 masked_address;
  unsigned int target = decode_address( trans.get_address(), masked_address );
  trans.set_address( masked_address );

  return initiator_socket[target]->transport_dbg( trans );
}


// Backward interface

tlm::tlm_sync_enum Bus_module::nb_transport_bw( int id, tlm::tlm_generic_payload& trans,
                                            tlm::tlm_phase& phase, sc_time& delay )
{
  return target_socket[ m_id_map[ &trans ] ]->nb_transport_bw( trans, phase, delay );
}

void Bus_module::invalidate_direct_mem_ptr( int id, sc_dt::uint64 start_range,
                                                sc_dt::uint64 end_range )
{
  // Reconstruct address range in system memory map
  sc_dt::uint64 bw_start_range = reconstruct_address(start_range, id);
  sc_dt::uint64 bw_end_range   = reconstruct_address(end_range, id);

  // Propagate call backward to all initiators
  for (unsigned int i = 0; i < target_socket.size(); i++)
    target_socket[i]->invalidate_direct_mem_ptr(bw_start_range, bw_end_range);
}

// Memory is divided into 1 GB chunks
unsigned int Bus_module::decode_address( sc_dt::uint64 address, sc_dt::uint64& masked_address )
{
  masked_address = address & (1_GB-1);
  return (address >> 30);
}

sc_dt::uint64 Bus_module::reconstruct_address ( uint64 address, unsigned int target )
{
  return address | (target << 30);
}


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
