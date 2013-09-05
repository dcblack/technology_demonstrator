//FILE:axibus.cpp

#include "axibus.h"
#include "tlm_utils/simple_initiator_socket.h"
using namespace std;
using namespace sc_core;

namespace {
  const char * const MSGID = "/Doulos/example/axibus";
}

Axibus::Axibus //< Constructor
( tlm_utils::simple_initiator_socket<Dev_module>& _axi_master_socket
)
: axi_master_socket(_axi_master_socket)
, m_memory_manager ( memory_manager::get_memory_manager() )
{
}

void Axibus::axi_read (Addr_t addr, Data_t data)
{
  axi_read_burst(addr,&data,sizeof(data));
}

void Axibus::axi_write(Addr_t addr, Data_t data)
{
  axi_write_burst(addr,&data,sizeof(data));
}

void Axibus::axi_read_burst (Addr_t addr, Data_t* data, size_t size)
{
  tlm::tlm_generic_payload* tlm2_trans_ptr(m_memory_manager->allocate());
  tlm2_trans_ptr->acquire();
  // Set the eight required attributes
  tlm2_trans_ptr->set_address         ( addr                                   );
  tlm2_trans_ptr->set_data_ptr        ( reinterpret_cast<unsigned char*>(data) );
  tlm2_trans_ptr->set_data_length     ( sizeof(data)*size                      );
  tlm2_trans_ptr->set_streaming_width ( sizeof(data)*size                      );
  tlm2_trans_ptr->set_byte_enable_ptr ( nullptr                                );
  tlm2_trans_ptr->set_dmi_allowed     ( false                                  );
  tlm2_trans_ptr->set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE           );
  tlm2_trans_ptr->set_command         ( tlm::TLM_READ_COMMAND                  );
  sc_time delay(SC_ZERO_TIME);
  axi_master_socket->b_transport(*tlm2_trans_ptr,delay);
  sc_core::wait(delay);
  switch (tlm2_trans_ptr->get_response_status()) {
    case               tlm::TLM_OK_RESPONSE: break;
    case    tlm::TLM_ADDRESS_ERROR_RESPONSE:
    case       tlm::TLM_INCOMPLETE_RESPONSE:
    default                                :
                                             {
                                             }
  }
  if (tlm2_trans_ptr->is_response_error()) {
    SC_REPORT_WARNING(MSGID, "Received error reply from AXI transaction.");
  }//endif
  tlm2_trans_ptr->release();
}

void Axibus::axi_write_burst(Addr_t addr, Data_t* data, size_t size)
{
  tlm::tlm_generic_payload* tlm2_trans_ptr(m_memory_manager->allocate());
  tlm2_trans_ptr->acquire();
  // Set the eight required attributes
  tlm2_trans_ptr->set_address         ( addr                                   );
  tlm2_trans_ptr->set_data_ptr        ( reinterpret_cast<unsigned char*>(data) );
  tlm2_trans_ptr->set_data_length     ( sizeof(data)*size                      );
  tlm2_trans_ptr->set_streaming_width ( sizeof(data)*size                      );
  tlm2_trans_ptr->set_byte_enable_ptr ( nullptr                                );
  tlm2_trans_ptr->set_dmi_allowed     ( false                                  );
  tlm2_trans_ptr->set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE           );
  tlm2_trans_ptr->set_command         ( tlm::TLM_WRITE_COMMAND                 );
  sc_time delay(SC_ZERO_TIME);
  axi_master_socket->b_transport(*tlm2_trans_ptr,delay);
  sc_core::wait(delay);
  switch (tlm2_trans_ptr->get_response_status()) {
    case               tlm::TLM_OK_RESPONSE: break;
    case    tlm::TLM_ADDRESS_ERROR_RESPONSE:
    case       tlm::TLM_INCOMPLETE_RESPONSE:
    default                                :
                                             {
                                             }
  }
  if (tlm2_trans_ptr->is_response_error()) {
    SC_REPORT_WARNING(MSGID, "Received error reply from AXI transaction.");
  }//endif
  tlm2_trans_ptr->release();
}

//------------------------------------------------------------------------------
// $LICENSE: Apache 2.0 $ <<<
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
//--------------------------------------------------------------------------->>>
// The end!
