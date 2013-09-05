//BEGIN mem.cpp (systemc)
///////////////////////////////////////////////////////////////////////////////
// $Info: Simple memory implementation $

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

#include "mem.h"
#include "report.h"
#include "sc_literals.h"

using namespace sc_core;

namespace {
  // Declare string used as message identifier in SC_REPORT_* calls
  static char const* const MSGID = "/Doulos/example/mem";
  // Embed file version information into object to help forensics
  static char const* const RCSID = "(@)$Id: mem.cpp,v 1.0 2013/02/04 18:13:33 dcblack Exp $";
  //                                        FILENAME VER DATE     TIME  USERNAME
}

///////////////////////////////////////////////////////////////////////////////
// Constructor <<
Mem_module::Mem_module
( sc_module_name instance_name
, size_t  depth
, sc_time read_latency
, sc_time write_latency
)
: sc_module(instance_name)
, target_socket("target_socket")
, m_depth(depth)
, m_read_latency (read_latency )
, m_write_latency(write_latency)
{
  // Misc. initialization
  m_byte_width = target_socket.get_bus_width()/8;
  m_memory = new int[m_depth];
  // Register methods
  target_socket.register_b_transport        ( this, &Mem_module::b_transport        );
  target_socket.register_transport_dbg      ( this, &Mem_module::transport_dbg      );
  target_socket.register_get_direct_mem_ptr ( this, &Mem_module::get_direct_mem_ptr );
  // Register processes - NONE
  REPORT_INFO("Constructed " << " " << name());
}//endconstructor

///////////////////////////////////////////////////////////////////////////////
// Destructor <<
Mem_module::~Mem_module(void)
{
  delete [] m_memory;
  REPORT_INFO("Destroyed " << name());
}

///////////////////////////////////////////////////////////////////////////////
// TLM-2 forward methods
///////////////////////////////////////////////////////////////////////////////
//
//  #####     ####### ####     #    #     #  ####  #####   ####  ####  ####### 
//  #    #       #    #   #   # #   ##    # #    # #    # #    # #   #    #    
//  #    #       #    #   #  #   #  # #   # #      #    # #    # #   #    #    
//  #####        #    ####  #     # #  #  #  ####  #####  #    # ####     #    
//  #    #       #    # #   ####### #   # #      # #      #    # # #      #    
//  #    #       #    #  #  #     # #    ## #    # #      #    # #  #     #    
//  #####  ##### #    #   # #     # #     #  ####  #       ####  #   #    #    
//
///////////////////////////////////////////////////////////////////////////////
void Mem_module::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
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

  if (address >= sc_dt::uint64(m_depth) * m_byte_width) {
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
      || (address+data_length)/m_byte_width > m_depth) {
    // Only allow word-multiple transfers within memory size
    trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
    return;
  }//endif

  // Obliged to implement read and write commands
  if ( command == tlm::TLM_READ_COMMAND ) {
    memcpy(data_ptr, &m_memory[address/m_byte_width], data_length);
    delay += (m_read_latency * data_length/m_byte_width); // Memory access time per bus value
  } else if ( command == tlm::TLM_WRITE_COMMAND ) {
    memcpy(&m_memory[address/m_byte_width], data_ptr, data_length);
    delay += (m_write_latency * data_length/m_byte_width); // Memory access time per bus value
  }//endif

  // Obliged to set response status to indicate successful completion
  trans.set_response_status( tlm::TLM_OK_RESPONSE );
}//end Mem_module::b_transport

////////////////////////////////////////////////////////////////////////////////
//
// ####### ####    #   #   #  ###  ###   ##  ####  #######    ###   ####   ###  
//    #    #   #  # #  ##  # #   # #  # #  # #   #    #       #  #  #   # #   # 
//    #    #   # #   # # # # #     #  # #  # #   #    #       #   # #   # #     
//    #    ####  #   # # # #  ###  ###  #  # ####     #       #   # ####  # ### 
//    #    # #   ##### # # #     # #    #  # # #      #       #   # #   # #   # 
//    #    #  #  #   # #  ## #   # #    #  # #  #     #       #  #  #   # #   # 
//    #    #   # #   # #   #  ###  #     ##  #   #    # ##### ###   ####   ###  
//
////////////////////////////////////////////////////////////////////////////////
unsigned int Mem_module::transport_dbg(tlm::tlm_generic_payload& trans)
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

  if (address >= sc_dt::uint64(m_depth) * m_byte_width) {
    trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
    return 0;
  } else if (address % m_byte_width) {
    // Only allow aligned bit width transfers
    trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
    return 0;
  }//endif

  // Obliged to implement read and write commands
  if ( command == tlm::TLM_READ_COMMAND ) {
    memcpy(data_ptr, &m_memory[address/m_byte_width], data_length);
    transferred = data_length;
  } else if ( command == tlm::TLM_WRITE_COMMAND ) {
    memcpy(&m_memory[address/m_byte_width], data_ptr, data_length);
    transferred = data_length;
  }//endif

  // Obliged to set response status to indicate successful completion
  trans.set_response_status( tlm::TLM_OK_RESPONSE );
  return transferred;
}//end Mem_module::transport_dbg

bool Mem_module::get_direct_mem_ptr
( tlm::tlm_generic_payload& trans
, tlm::tlm_dmi&             dmi_data
)
{
  dmi_data.set_dmi_ptr( (unsigned char*)m_memory );
  dmi_data.set_start_address( 0 );
  dmi_data.set_end_address( m_depth * sizeof(Data_t) - 1);
  dmi_data.set_read_latency ( m_read_latency );
  dmi_data.set_write_latency( m_write_latency );
  dmi_data.allow_read_write();
  return true;
}

//EOF
