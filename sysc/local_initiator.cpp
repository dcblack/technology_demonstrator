//BEGIN local_initiator.cpp (systemc)
// -*- C++ -*- vim600:sw=2:tw=80:fdm=marker:fmr=<<<,>>>
///////////////////////////////////////////////////////////////////////////////
// $Info: local initiator implementation $
//
// BRIEF DESCRIPTION:
// Implements a local initiator...
//
// DETAILED DESCRIPTION:
// {:TODO:}

#include "local_initiator.h"
#include "report.h"
#include <iomanip>
#include <string>
#include "mtx_driver.h"
#include "software.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

namespace {
  // Declare string used as message identifier in SC_REPORT_* calls
  static char const* const MSGID = "/Doulos/example/local_initiator";
  // Embed file version information into object to help forensics
  static char const* const RCSID = "(@)$Id: local_initiator.cpp  1.0 09/02/12 10:00 dcblack $";
  //                                        FILENAME  VER DATE     TIME  USERNAME
}


///////////////////////////////////////////////////////////////////////////////
// Constructor <<
Local_initiator_module:: Local_initiator_module
( sc_module_name instance_name
)
: sc_module(instance_name)
, initiator_socket("initiator_socket")
, m_memory_manager ( memory_manager::get_memory_manager() )
, m_memory(nullptr)
, m_free(0)
, m_period(10_ns)
, interrupt_enabled(false)
, interrupt_active(false)
{
  //----------------------------------------------------------------------------
  // Parse command-line arguments
  //----------------------------------------------------------------------------
  for (int i=1; i<sc_argc(); ++i) {
    string arg(sc_argv()[i]);
    if (arg == "-irq") interrupt_enabled = true;
    if (arg == "-h") REPORT_INFO("OPTION: -irq enables interrupt scheduler");
  }//endfor

  //----------------------------------------------------------------------------
  // Report configuration
  //----------------------------------------------------------------------------
  REPORT_INFO("Interrupts are " << (interrupt_enabled?"enabled":"disabled"));

  //----------------------------------------------------------------------------
  // Connectivity
  //----------------------------------------------------------------------------
  interrupt_export.bind(interrupt_signal);

  //----------------------------------------------------------------------------
  // Register TLM backwards path methods -NONE-
  //----------------------------------------------------------------------------
  initiator_socket.register_invalidate_direct_mem_ptr(this, &Local_initiator_module:: invalidate_direct_mem_ptr);

  //----------------------------------------------------------------------------
  // Register processes
  //----------------------------------------------------------------------------
  SC_HAS_PROCESS(Local_initiator_module);
  SC_THREAD(initiator_thread);

  // Establish local time
  m_qk.set_global_quantum(0.5_us);
  m_qk.reset();

  REPORT_INFO("Constructed " << name());
}//endconstructor

///////////////////////////////////////////////////////////////////////////////
// Destructor <<
Local_initiator_module:: ~Local_initiator_module(void)
{
  REPORT_INFO("Destroyed " << name());
}

///////////////////////////////////////////////////////////////////////////////
// Callbacks -- placeholders for now
void Local_initiator_module:: before_end_of_elaboration(void)
{
  REPORT_INFO(__func__ << " " << name());
}

void Local_initiator_module:: end_of_elaboration(void)
{
  REPORT_INFO(__func__ << " " << name());
}

void Local_initiator_module:: start_of_simulation(void) {
  REPORT_INFO(__func__ << " " << name());
}

void Local_initiator_module:: end_of_simulation(void)
{
  REPORT_INFO(__func__ << " " << name());
}

///////////////////////////////////////////////////////////////////////////////
// Processes <<
void Local_initiator_module:: initiator_thread(void)  {
  REPORT_INFO("Started " << __func__ << " " << name());
  Mtx driver(this,0/*device base address*/); //< setup driver to know where we are
  Software* const code(new Software(this));
  int retcode;

  // Launch software & interrupt handler
  sc_process_handle sw = sc_spawn(&retcode,sc_bind(&Software::sw_main,code),"sw_main");
  sc_process_handle ih = sc_spawn(sc_bind(&Software::interrupt_handler,code),"interrupt_handler");
  interrupt_active  = false;
  wait(SC_ZERO_TIME); // allow processes to start
  ih.suspend();
  if (interrupt_enabled) {
    sw.suspend();
    do {
      wait(1_us, interrupt_signal.posedge_event() | scheduler_event);
      ih.suspend();
      sw.suspend();
      if (interrupt_enabled && (!interrupt_active && interrupt_signal.read() == SC_LOGIC_1)) {
        REPORT_INFO(__func__ << "Processing interrupt");
        interrupt_active = true;
        disable_irq();
        ih.resume();
        interrupt_event.notify(SC_ZERO_TIME);
      } else if (interrupt_active) {
        ih.resume();
      } else {
        sw.resume();
        resume_os();
      }//endif
    } while (!sw.terminated());
  } else {
    wait(sw.terminated_event());
  }//endif
  if (retcode == 0) {
    REPORT_INFO("Software terminated with successful return code of zero.");
  } else {
    REPORT_ERROR("Software terminated with non-zero return code " << retcode);
    util::report::adjust_unexpected_errors(-1);
  }//endif
  ih.kill();
  wait(1,SC_SEC);
  REPORT_INFO("Exiting local_initiator");
  sc_stop();
}//end Local_initiator_module:: initiator_thread()

void Local_initiator_module:: invalidate_direct_mem_ptr
( sc_dt::uint64 start_range
, sc_dt::uint64 end_range
) {
  for (unsigned int i = 0; i < dmi_table.size(); i++) {
    sc_dt::uint64  dmi_start_address = dmi_table[i].get_start_address();
    sc_dt::uint64  dmi_end_address   = dmi_table[i].get_end_address();
    if (start_range <= dmi_end_address && end_range >= dmi_start_address)
      // Invalidate entire region
      dmi_table.erase(dmi_table.begin() + i);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Helpers
//------------------------------------------------------------------------------
void Local_initiator_module:: get_mem(size_t n)
{
  if (m_memory == nullptr || m_used.empty()) {
    unsigned char* dmi_pointer;
    sc_dt::uint64  dmi_start_address;
    sc_dt::uint64  dmi_end_address;
    for (unsigned int i = 0; i < dmi_table.size(); i++) {
      dmi_pointer       = dmi_table[i].get_dmi_ptr();
      dmi_start_address = dmi_table[i].get_start_address();
      dmi_end_address   = dmi_table[i].get_end_address();
      if ((dmi_end_address - dmi_start_address > n) && dmi_table[i].is_write_allowed()) {
        m_memory = dmi_pointer;
        m_free   = dmi_end_address - dmi_start_address + 1;
        break;
      }//endif
    }//endfor
    if (m_memory == nullptr) {
      Data_t dummy;
      tlm::tlm_generic_payload* payload_ptr(m_memory_manager->allocate());
      payload_ptr->acquire();
      payload_ptr->set_command( tlm::TLM_WRITE_COMMAND );
      payload_ptr->set_address( 1_GB ); //< MAGIC: We know it memory is allocated there
      payload_ptr->set_data_ptr( reinterpret_cast<unsigned char*>(&dummy) );
      payload_ptr->set_data_length( sizeof(Data_t) );
      payload_ptr->set_streaming_width( sizeof(Data_t) );
      payload_ptr->set_byte_enable_ptr( 0 );
      payload_ptr->set_dmi_allowed( false );
      payload_ptr->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
      tlm::tlm_dmi dmi_data;
      if (initiator_socket->get_direct_mem_ptr( *payload_ptr, dmi_data )) {
        dmi_table.push_back(dmi_data);
        dmi_start_address = dmi_data.get_start_address();
        dmi_end_address   = dmi_data.get_end_address();
        if ((dmi_end_address - dmi_start_address > n) && dmi_data.is_write_allowed()) {
          m_memory = dmi_pointer;
          m_free   = dmi_end_address - dmi_start_address + 1;
        }//endif
      }//endif
      payload_ptr->release();
    }//endif
    assert(m_memory != nullptr);
  }//endif
}//end Local_initiator_module::get_mem(...)

Byte_ptr Local_initiator_module:: malloc(size_t n)
{
  // Acquire pointer to memory
  get_mem(n);

  assert(m_free > n); //< otherwise out of memory

  // Search for free space
  size_t avail = 0;
  // Is there space near zero?
  if (m_used.begin()->first > n) {
    m_used[avail] = n;
    m_free -= n;
    return &m_memory[avail];
  }//endif
  for (auto inuse : m_used) {
    auto avail = inuse.first + inuse.second;
    if (m_used.count(avail)) continue;
    auto next = m_used.upper_bound(inuse.first);
    if (next == m_used.end()) {
      m_used[avail] = n;
      m_free -= n;
      return &m_memory[avail];
    } else {
      if (next->first - avail > n) {
        m_used[avail] = n;
        m_free -= n;
        return &m_memory[avail];
      } else {
        continue;
      }//endif
    }//endif
  }//endfor
  return nullptr;
}//end Local_initiator_module::malloc(...)

//------------------------------------------------------------------------------
void  Local_initiator_module:: free(Byte_ptr ptr)
{
  size_t addr = m_memory - ptr;
  assert(m_used.count(addr) == 1); //< didn't allocate
  m_free += m_used[addr];
  m_used.erase(addr);
}

//------------------------------------------------------------------------------
void Local_initiator_module:: mem_write(size_t waddr, int* data_ptr, size_t words)
{
  bool dmi = false;
  unsigned char* dmi_pointer;
  sc_dt::uint64  dmi_start_address;
  sc_dt::uint64  dmi_end_address;
  sc_time        dmi_latency;
  size_t addr = waddr*sizeof(Data_t); // adjust for byte addresses
  size_t n    = words*sizeof(Data_t);
  for (unsigned int i = 0; i < dmi_table.size(); i++) {
    dmi_pointer       = dmi_table[i].get_dmi_ptr();
    dmi_start_address = dmi_table[i].get_start_address();
    dmi_end_address   = dmi_table[i].get_end_address();
    if (addr >= dmi_start_address && addr <= dmi_end_address && dmi_table[i].is_write_allowed()) {
      dmi = true;
      dmi_latency = dmi_table[i].get_write_latency();
      break;
    }//endif
  }//endfor

  if (dmi) {
    ::memcpy(dmi_pointer + addr - dmi_start_address, data_ptr, n);
    m_qk.inc( dmi_latency );
  } else {
    // Regular transaction
    tlm::tlm_generic_payload* payload_ptr(m_memory_manager->allocate());
    payload_ptr->acquire();
    payload_ptr->set_command( tlm::TLM_WRITE_COMMAND );
    payload_ptr->set_address( addr );
    payload_ptr->set_data_ptr( reinterpret_cast<unsigned char*>(data_ptr) );
    payload_ptr->set_data_length( n );
    payload_ptr->set_streaming_width( n );
    payload_ptr->set_byte_enable_ptr( 0 );
    payload_ptr->set_dmi_allowed( false );
    payload_ptr->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

    sc_time delay = m_qk.get_local_time();

    initiator_socket->b_transport( *payload_ptr, delay );

    m_qk.set_and_sync( delay );

    if ( payload_ptr->is_response_error() )
      SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

    if (payload_ptr->is_dmi_allowed()) {
      // Reuse transaction object to request DMI
      payload_ptr->set_address( addr );
      tlm::tlm_dmi dmi_data;
      if (initiator_socket->get_direct_mem_ptr( *payload_ptr, dmi_data ))
        dmi_table.push_back(dmi_data);
    }//endif
    payload_ptr->release();
  }//endif
}

//------------------------------------------------------------------------------
void Local_initiator_module:: mem_read (size_t waddr, int* data_ptr, size_t words)
{
  bool dmi = false;
  unsigned char* dmi_pointer;
  sc_dt::uint64  dmi_start_address;
  sc_dt::uint64  dmi_end_address;
  sc_time        dmi_latency;
  size_t addr = waddr*sizeof(Data_t); // adjust for byte addresses
  size_t n    = words*sizeof(Data_t);
  for (unsigned int i = 0; i < dmi_table.size(); i++) {
    dmi_pointer       = dmi_table[i].get_dmi_ptr();
    dmi_start_address = dmi_table[i].get_start_address();
    dmi_end_address   = dmi_table[i].get_end_address();
    if (addr >= dmi_start_address && addr <= dmi_end_address && dmi_table[i].is_read_allowed()) {
      dmi = true;
      dmi_latency = dmi_table[i].get_read_latency();
      break;
    }//endif
  }//endfor

  if (dmi) {
    ::memcpy(data_ptr, dmi_pointer + addr - dmi_start_address, n);
    m_qk.inc( dmi_latency );
  } else {
    // Regular transaction
    tlm::tlm_generic_payload* payload_ptr(m_memory_manager->allocate());
    payload_ptr->acquire();
    payload_ptr->set_command( tlm::TLM_READ_COMMAND );
    payload_ptr->set_address( addr );
    payload_ptr->set_data_ptr( reinterpret_cast<unsigned char*>(data_ptr) );
    payload_ptr->set_data_length( n );
    payload_ptr->set_streaming_width( n );
    payload_ptr->set_byte_enable_ptr( 0 );
    payload_ptr->set_dmi_allowed( false );
    payload_ptr->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

    sc_time delay = m_qk.get_local_time();
    initiator_socket->b_transport( *payload_ptr, delay );

    m_qk.set_and_sync( delay );

    if ( payload_ptr->is_response_error() )
      SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

    if (payload_ptr->is_dmi_allowed()) {
      // Reuse transaction object to request DMI
      payload_ptr->set_address( addr );
      tlm::tlm_dmi dmi_data;
      if (initiator_socket->get_direct_mem_ptr( *payload_ptr, dmi_data ))
        dmi_table.push_back(dmi_data);
    }//endif

    payload_ptr->release();
  }
}

//------------------------------------------------------------------------------
// NOTE: addr & n represent 32-bit locations (ie. 4 bytes per)
void Local_initiator_module:: mem_write(size_t addr, int& data)
{
  mem_write(addr,&data,1);
}

//------------------------------------------------------------------------------
// NOTE: addr & n represent 32-bit locations (ie. 4 bytes per)
void Local_initiator_module:: mem_read (size_t addr, int& data)
{
  mem_read(addr,&data,1);
}

//------------------------------------------------------------------------------
// NOTE: addr & n represent 32-bit locations (ie. 4 bytes per)
int  Local_initiator_module:: mem_read (size_t addr)
{
  int result;
  mem_read(addr, result);
  return result;
}

//------------------------------------------------------------------------------
// NOTE: addr & n represent 32-bit locations (ie. 4 bytes per)
void Local_initiator_module:: dev_write(size_t waddr, int* data_ptr, size_t words)
{
  size_t addr = waddr*sizeof(Data_t); // adjust for byte addresses
  size_t n    = words*sizeof(Data_t);
  // Regular transaction
  tlm::tlm_generic_payload* payload_ptr(m_memory_manager->allocate());
  payload_ptr->acquire();
  payload_ptr->set_command( tlm::TLM_WRITE_COMMAND );
  payload_ptr->set_address( addr );
  payload_ptr->set_data_ptr( reinterpret_cast<unsigned char*>(data_ptr) );
  payload_ptr->set_data_length( n );
  payload_ptr->set_streaming_width( n );
  payload_ptr->set_byte_enable_ptr( 0 );
  payload_ptr->set_dmi_allowed( false );
  payload_ptr->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

  sc_time delay = m_qk.get_local_time();

  initiator_socket->b_transport( *payload_ptr, delay );

  m_qk.set_and_sync( delay );

  if ( payload_ptr->is_response_error() )
    SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

  payload_ptr->release();
}

//------------------------------------------------------------------------------
// NOTE: addr & n represent 32-bit locations (ie. 4 bytes per)
void Local_initiator_module:: dev_read (size_t waddr, int* data_ptr, size_t words)
{
  size_t addr = waddr*sizeof(Data_t); // adjust for byte addresses
  size_t n    = words*sizeof(Data_t);
  // Regular transaction
  tlm::tlm_generic_payload* payload_ptr(m_memory_manager->allocate());
  payload_ptr->acquire();
  payload_ptr->set_command( tlm::TLM_READ_COMMAND );
  payload_ptr->set_address( addr );
  payload_ptr->set_data_ptr( reinterpret_cast<unsigned char*>(data_ptr) );
  payload_ptr->set_data_length( n );
  payload_ptr->set_streaming_width( n );
  payload_ptr->set_byte_enable_ptr( 0 );
  payload_ptr->set_dmi_allowed( false );
  payload_ptr->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

  sc_time delay = m_qk.get_local_time();
  initiator_socket->b_transport( *payload_ptr, delay );

  m_qk.set_and_sync( delay );

  if ( payload_ptr->is_response_error() )
    SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

  payload_ptr->release();
}

//------------------------------------------------------------------------------
// NOTE: addr & n represent 32-bit locations (ie. 4 bytes per)
void Local_initiator_module:: dev_write(size_t addr, int& data)
{
  dev_write(addr,&data,1);
}

//------------------------------------------------------------------------------
// NOTE: addr & n represent 32-bit locations (ie. 4 bytes per)
void Local_initiator_module:: dev_read (size_t addr, int& data)
{
  dev_read(addr,&data,1);
}

//------------------------------------------------------------------------------
// NOTE: addr & n represent 32-bit locations (ie. 4 bytes per)
int  Local_initiator_module:: dev_read (size_t addr)
{
  int result;
  dev_read(addr, result);
  return result;
}

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
