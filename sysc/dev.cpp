//BEGIN dev.cpp (systemc)
///////////////////////////////////////////////////////////////////////////////
// $Info: Simple device implementation $

#include "dev.h"
#include "axibus.h"
#include "report.h"
#include "sc_literals.h"
#include <iomanip>

using namespace sc_core;
using namespace sc_dt;
using namespace std;

namespace {
  // Declare string used as message identifier in SC_REPORT_* calls
  static char const* const MSGID = "/Doulos/example/dev";
  // Embed file version information into object to help forensics
  static char const* const RCSID = "(@)$Id: dev.cpp,v 1.0 2013/02/04 18:13:33 dcblack Exp $";
  //                                        FILENAME VER DATE     TIME  USERNAME
}

///////////////////////////////////////////////////////////////////////////////
// Constructor <<
Dev_module::Dev_module
( sc_module_name instance_name
)
: sc_module(instance_name)
, target_socket("target_socket")
, initiator_socket("initiator_socket")
, m_register_count(REGISTERS)
, m_latency(10_ns)
, m_axibus(nullptr)
{
  // Misc. initialization
  m_axibus = new Axibus(initiator_socket);
  m_byte_width = target_socket.get_bus_width()/8;
  m_register = new int[m_register_count];
  // Register methods
  target_socket.register_b_transport  ( this, &Dev_module::b_transport   );
  target_socket.register_transport_dbg( this, &Dev_module::transport_dbg );
  // Register processes
  SC_HAS_PROCESS(Dev_module);
  SC_THREAD(execute_thread);
  REPORT_INFO("Constructed " << " " << name());
}//endconstructor

///////////////////////////////////////////////////////////////////////////////
// Destructor <<
Dev_module::~Dev_module(void)
{
  delete [] m_register;
  REPORT_INFO("Destroyed " << name());
}

void Dev_module::interrupt(void)
{
  // Send interrupt
  interrupt_port->write(SC_LOGIC_1); //< cheap, doesn't solve TCPIP
}

void Dev_module::execute_thread(void)
{
  const sc_time clock_period(10,SC_NS);
  for(;;) {
    bool automatic = AUTOMATIC(m_register[STATUS]);
    if ( !automatic ) {
      wait(m_register_write_event);
    } else {
      wait(clock_period);
    }
    if (((m_register[STATUS] & INTR_BIT) == 0) && (interrupt_port->read() == SC_LOGIC_1)) {
      interrupt_port->write(SC_LOGIC_0); //< clear interrupt
    }//endif
    bool watch = (m_register[STATUS] == START);
    Data_t retcode = dev_hls
    ( m_register
    , m_imem
    );
    int cmd_state = m_register[STATUS];
    if (watch || cmd_state >= HALTED) {
      interrupt();
    }
  }//endforever
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
void Dev_module::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
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
    REPORT_INFO("ALERT: Misaligned address: 0x" << hex << address);
    trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
    return;
  } else if (byte_enables != 0) {
    // No support for byte enables
    trans.set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
    return;
  } else if ((data_length % m_byte_width) != 0 || streaming_width < data_length || data_length == 0
      || (address+data_length)/m_byte_width > m_register_count) {
    // Only allow word-multiple transfers within memory size
    trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
    return;
  }//endif

  // Obliged to implement read and write commands
  if ( command == tlm::TLM_READ_COMMAND ) {
    memcpy((void*) data_ptr, (void*) &m_register[address/m_byte_width], data_length);
  } else if ( command == tlm::TLM_WRITE_COMMAND ) {
    memcpy((void*) &m_register[address/m_byte_width], (void*) data_ptr, data_length);
    // Notify execution
    m_register_write_event.notify();
  }//endif

  // Memory access time per bus value
  delay += (m_latency * data_length/m_byte_width);

  // Obliged to set response status to indicate successful completion
  trans.set_response_status( tlm::TLM_OK_RESPONSE );
}//end Dev_module::b_transport

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
unsigned int Dev_module::transport_dbg(tlm::tlm_generic_payload& trans)
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
    memcpy((void*) data_ptr, (void*) &m_register[address/m_byte_width], data_length);
    transferred = data_length;
  } else if ( command == tlm::TLM_WRITE_COMMAND ) {
    memcpy((void*) &m_register[address/m_byte_width], (void*) data_ptr, data_length);
    transferred = data_length;
  }//endif

  // Obliged to set response status to indicate successful completion
  trans.set_response_status( tlm::TLM_OK_RESPONSE );
  return transferred;
}//end Dev_module::transport_dbg

///////////////////////////////////////////////////////////////////////////////
// $License: Apache 2.0 $ <<<
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
////////////////////////////////////////////////////////////////////////////>>>
//EOF
