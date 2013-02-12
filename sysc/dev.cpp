//BEGIN dev.cpp (systemc)
///////////////////////////////////////////////////////////////////////////////
// $Info: Simple device implementation $

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

#include "dev.h"
#include "report.h"
using namespace sc_core;

namespace {
  // Declare string used as message identifier in SC_REPORT_* calls
  static char const* const MSGID = "/Doulos/example/dev";
  // Embed file version information into object to help forensics
  static char const* const RCSID = "(@)$Id: dev.cpp,v 1.0 2013/02/04 18:13:33 dcblack Exp $";
  //                                        FILENAME VER DATE     TIME  USERNAME
}

///////////////////////////////////////////////////////////////////////////////
// Constructor <<
SC_HAS_PROCESS(dev_module);
dev_module::dev_module
( sc_module_name instance_name
)
: sc_module(instance_name)
, target_socket("target_socket")
, m_register_count(8)
, m_latency(sc_time(10,SC_NS))
{
  // Misc. initialization
  m_byte_width = target_socket.get_bus_width()/8;
  m_register = new int[m_register_count];
  // Register methods
  target_socket.register_b_transport  ( this, &dev_module::b_transport   );
  target_socket.register_transport_dbg( this, &dev_module::transport_dbg );
  // Register processes - NONE
  REPORT_INFO("Constructed " << " " << name());
}//endconstructor

///////////////////////////////////////////////////////////////////////////////
// Destructor <<
dev_module::~dev_module(void)
{
  delete [] m_register;
  REPORT_INFO("Destroyed " << name());
}

///////////////////////////////////////////////////////////////////////////////
// TLM-2 forward methods
void dev_module::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
{
  tlm::tlm_command command = trans.get_command();
  sc_dt::uint64    address = trans.get_address();
  unsigned char*   data_ptr = trans.get_data_ptr();
  unsigned int     data_length = trans.get_data_length();
  unsigned char*   byte_enables = trans.get_byte_enable_ptr();
  unsigned int     streaming_width = trans.get_streaming_width();

  // Obliged to check address range and check for unsupported features,
  //   i.e. byte enables, streaming, and bursts
  // Can ignore DMI hint and extensions
  // Using the SystemC report handler is an acceptable way of signalling an error

  if (address >= sc_dt::uint64(m_register_count) * m_byte_width) {
    trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
    return;
  } else if (address % m_byte_width) {
    // Only allow aligned bit width transfers
    SC_REPORT_WARNING(MSGID,"Misaligned address");
    trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
    return;
  } else if (byte_enables != 0) {
    // No support for byte enables
    trans.set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
    return;
  } else if ((data_length % m_byte_width) != 0 || streaming_width < data_length || data_length == 0
      || (address+data_length)/m_byte_width >= m_register_count) {
    // Only allow word-multiple transfers within memory size
    trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
    return;
  }//endif

  // Obliged to implement read and write commands
  if ( command == tlm::TLM_READ_COMMAND ) {
    memcpy(data_ptr, &m_register[address/m_byte_width], data_length);
  } else if ( command == tlm::TLM_WRITE_COMMAND ) {
    memcpy(&m_register[address/m_byte_width], data_ptr, data_length);
  }//endif

  // Memory access time per bus value
  delay += (m_latency * data_length/m_byte_width);

  // Obliged to set response status to indicate successful completion
  trans.set_response_status( tlm::TLM_OK_RESPONSE );
}//end dev_module::b_transport

unsigned int dev_module::transport_dbg(tlm::tlm_generic_payload& trans)
{
  int              transferred = 0;
  tlm::tlm_command command = trans.get_command();
  sc_dt::uint64    address = trans.get_address();
  unsigned char*   data_ptr = trans.get_data_ptr();
  unsigned int     data_length = trans.get_data_length();
  unsigned char*   byte_enables = trans.get_byte_enable_ptr();
  unsigned int     streaming_width = trans.get_streaming_width();

  // Obliged to check address range and check for unsupported features,
  //   i.e. byte enables, streaming, and bursts
  // Can ignore DMI hint and extensions
  // Using the SystemC report handler is an acceptable way of signalling an error

  if (address >= sc_dt::uint64(m_register_count) * m_byte_width) {
    trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
    return 0;
  } else if (address % m_byte_width) {
    // Only allow aligned bit width transfers
    trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
    return 0;
  }//endif

  // Obliged to implement read and write commands
  if ( command == tlm::TLM_READ_COMMAND ) {
    memcpy(data_ptr, &m_register[address/m_byte_width], data_length);
    transferred = data_length;
  } else if ( command == tlm::TLM_WRITE_COMMAND ) {
    memcpy(&m_register[address/m_byte_width], data_ptr, data_length);
    transferred = data_length;
  }//endif

  // Obliged to set response status to indicate successful completion
  trans.set_response_status( tlm::TLM_OK_RESPONSE );
  return transferred;
}//end dev_module::transport_dbg

//EOF
