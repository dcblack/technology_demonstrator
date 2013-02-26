//BEGIN async_adaptor.cpp (systemc)
// -*- C++ -*- vim600:sw=2:tw=80:fdm=marker:fmr=<<<,>>>
///////////////////////////////////////////////////////////////////////////////
// $Info: async_adaptor interface implementation $
//
// Implements a TLM initiator that performs transactions via TCP IP socket
// information.

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

#include "async_adaptor.h"
#include "report.h"
#include <iomanip>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>

using namespace std;
using namespace sc_core;
using namespace sc_dt;

namespace {
  // Declare string used as message identifier in SC_REPORT_* calls
  static char const* const MSGID = "/Doulos/example/async_adaptor";
  // Embed file version information into object to help forensics
  static char const* const RCSID = "(@)$Id: async_adaptor.cpp  1.0 09/02/12 10:00 dcblack $";
  //                                        FILENAME  VER DATE     TIME  USERNAME
}

int async_adaptor_module::s_stop_requests{0};

///////////////////////////////////////////////////////////////////////////////
// Constructor <<
async_adaptor_module::async_adaptor_module(sc_module_name instance_name)
: sc_module(instance_name)
, initiator_socket("initiator_socket")
, m_async_channel("m_async_channel")
, m_keep_alive_signal("m_keep_alive_signal")
, m_tcpip_port(4000)
, m_lock_permission(new std::lock_guard<std::mutex>(m_allow_pthread))
, m_pthread(&async_adaptor_module::async_os_thread,this,std::ref(m_async_channel))
{
  signal(SIGINT,&async_adaptor_module::sighandler); //< allow for graceful interrupts

  // Parse command-line arguments
  for (int i=1; i<sc_argc(); ++i) {
    string arg(sc_argv()[i]);
    if      (arg.find("-debug")  == 0) sc_report_handler::set_verbosity_level(SC_DEBUG);
    else if (arg.find("-quiet")  == 0) sc_report_handler::set_verbosity_level(SC_NONE);
    else if (arg.find("-low")    == 0) sc_report_handler::set_verbosity_level(SC_LOW);
    else if (arg.find("-medium") == 0) sc_report_handler::set_verbosity_level(SC_MEDIUM);
    else if (arg.find("-high")   == 0) sc_report_handler::set_verbosity_level(SC_HIGH);
    else if (arg.find("-full")   == 0) sc_report_handler::set_verbosity_level(SC_FULL);
    else if (arg.find("-port=")  == 0) {
      m_tcpip_port = atoi(arg.substr(6).c_str());
    }//endif
  }//endfor

  // Report configuration
  REPORT_INFO("\n===================================================================================\n"
           << "CONFIGURATION\n"
           << ">   Listening on port " << m_tcpip_port << "\n"
           << ">   Verbosity is " << sc_report_handler::get_verbosity_level() << "\n"
           << "===================================================================================\n"
           );

  // Register TLM backwards fsc_report_handler::snctions
  // -NONE-
  // Register processes
  SC_HAS_PROCESS(async_adaptor_module);
  SC_THREAD(initiator_sysc_thread_process);
  SC_THREAD(keep_alive_process);
  REPORT_INFO("Constructed " << name());
}//endconstructor

///////////////////////////////////////////////////////////////////////////////
// Destructor <<
async_adaptor_module::~async_adaptor_module(void)
{
  REPORT_INFO("Destroyed " << name());
}

///////////////////////////////////////////////////////////////////////////////
// Callbacks
void async_adaptor_module::before_end_of_elaboration(void)
{
  REPORT_INFO(__func__ << " " << name());
}

void async_adaptor_module::end_of_elaboration(void)
{
  REPORT_INFO(__func__ << " " << name());
}

void async_adaptor_module::start_of_simulation(void) {
  REPORT_INFO(__func__ << " " << name());
  m_keep_alive_signal.write(true);
  m_lock_permission.reset(nullptr);
}

void async_adaptor_module::end_of_simulation(void)
{
  REPORT_INFO(__func__ << " " << name());
}

///////////////////////////////////////////////////////////////////////////////
// External threads
void async_adaptor_module::async_os_thread(tlmx_channel& async_channel) {
  REPORT_INFO("Starting " << __func__ << " ...");

  // Open TCP/IP socket to async_adaptor
  struct sockaddr_in local_server;
  int option_value;

  // Create socket
  int listening_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listening_socket == -1) {
    REPORT_FATAL("Could not create socket");
  }

  // Prepare the sockaddr_in structure
  local_server.sin_family = AF_INET;
  local_server.sin_addr.s_addr = INADDR_ANY;
  local_server.sin_port = htons( m_tcpip_port );

  option_value = 1;
  if ( setsockopt
       ( listening_socket
       , SOL_SOCKET
       , SO_REUSEADDR
       , (void* )&option_value
       , (socklen_t)(sizeof(socklen_t))
       ) < 0
  ) {
    REPORT_FATAL("Unable to set socket option");
  }
   
  // Bind
  if ( bind
       ( listening_socket
       , (struct sockaddr *)&local_server 
       , sizeof(local_server)
       ) < 0
  ) {
    REPORT_FATAL("Bind failed");
    exit(1);
  }
  REPORT_NOTE("Bind done");

  // Listen
  REPORT_INFO("Queueing incoming connections...");
  listen(listening_socket, 1 /*request at a time*/);

  { // wait for systemc to release
    std::lock_guard<std::mutex> request_permission(m_allow_pthread);
  }

  // Holding place for data
  uint8_t data_ptr[TLMX_MAX_DATA_LEN];
  int     incoming_socket{0};

  // Watch for traffic
  REPORT_INFO("Waiting for incoming connections...");
  // Accept an incoming connection
  int addr_len = sizeof(struct sockaddr_in);
  struct sockaddr_in remote_client;
  incoming_socket = accept( listening_socket
                          , (struct sockaddr *)&remote_client
                          , (socklen_t*)&addr_len
                          );
  if (incoming_socket<0) {
    REPORT_FATAL("Accept failed: " << strerror(errno));
  }
  REPORT_NOTE("Connection accepted...");

  for(;;) {

    // Get data
    char receive_buffer[TLMX_MAX_BUFFER];
    bzero(receive_buffer,TLMX_MAX_BUFFER);
    int recv_count = read(incoming_socket, receive_buffer, TLMX_MAX_BUFFER);
    if(recv_count < 0) {
      REPORT_ERROR("TCPIP read/recv failed" << strerror(errno));
    }
    if (recv_count < TLMX_DATA_PTR_INDEX) {
      REPORT_FATAL("Incomplete packet received");
    }
    REPORT_NOTE("Received data...");

    // Unpack data
    bzero(data_ptr,TLMX_MAX_DATA_LEN); //< clear to aid debugging
    tlmx_packet_ptr tlmx_trans_ptr(new tlmx_packet( TLMX_IGNORE, 0, 0, data_ptr ));
    int unpacked_size = tlmx_trans_ptr->unpack(receive_buffer);
    REPORT_NOTE("Request to SystemC " << tlmx_trans_ptr->str());

    // Exit if commanded
    if (tlmx_trans_ptr->command == TLMX_EXIT) {
      REPORT_NOTE("Exiting due to TLMX_EXIT...");
      break;
    }

    // Send
    REPORT_NOTE("Pushing to async_channel...");
    async_channel.push(tlmx_trans_ptr);
    // Wait for channel to pass payload to initiator_sysc_thread_process & return results
    // from TLM 2.0 transport.
    REPORT_NOTE("Waiting for async_channel ...");
    async_channel.wait_for_put();
    // ASSERTION: Should be ready
    REPORT_NOTE("Pulling from async_channel ...");
    if (not async_channel.nb_pull(tlmx_trans_ptr)) {
      REPORT_FATAL("Missing response");
    }
    REPORT_NOTE("Response from SystemC " << tlmx_trans_ptr->str());
    // Check for errors and adjust
    if (tlmx_trans_ptr->status != TLMX_OK_RESPONSE) {
      REPORT_ERROR(tlmx_status_to_str(tlmx_status_t(tlmx_trans_ptr->status)));
    }
    // Pack data
    char transmit_buffer[TLMX_MAX_BUFFER];
    bzero(transmit_buffer,TLMX_MAX_BUFFER);
    int packed_size = tlmx_trans_ptr->pack(transmit_buffer);
    sc_assert(packed_size == unpacked_size);
    // Send response to TCP/IP
    REPORT_NOTE("Sending response ...");
    int send_count = write(incoming_socket, transmit_buffer, packed_size);
    if(send_count < 0) {
      REPORT_ERROR("TCPIP write/send failed" << strerror(errno));
    }
    sc_assert(send_count == packed_size);
  }//endforever

  REPORT_INFO("Closing down...");

  // Close TCP/IP socket to async_adaptor
  close(incoming_socket);

}//end async_adaptor_module::async_os_thread()

///////////////////////////////////////////////////////////////////////////////
// Processes <<
void async_adaptor_module::initiator_sysc_thread_process(void)  {
  REPORT_INFO("Started " << __func__ << " " << name());

  // Holding place for data
  uint8_t data_ptr[TLMX_MAX_DATA_LEN];
  tlm::tlm_generic_payload tlm2_trans;
  sc_time delay(SC_ZERO_TIME);

  for(;;) {
    // Wait for data to arrive from remote
    m_keep_alive_signal.write(true);
    wait(m_async_channel.sysc_put_event());
    REPORT_NOTE("Received sysc_put_event");
    m_keep_alive_signal.write(false);

    // Setup place to receive incoming payload
    bzero(data_ptr,TLMX_MAX_DATA_LEN); //< clear to aid debugging
    tlmx_packet_ptr tlmx_trans_ptr(new tlmx_packet( TLMX_IGNORE, 0, 0, data_ptr ));

    // Lockdown and obtain from incoming queue
    if (not m_async_channel.nb_get(tlmx_trans_ptr)) {
      REPORT_ERROR("Missing response");
    }

    // Setup TLM 2.0 generic payload
    tlm2_trans.set_address         ( tlmx_trans_ptr->address         );
    tlm2_trans.set_data_ptr        ( tlmx_trans_ptr->data_ptr        );
    tlm2_trans.set_data_length     ( tlmx_trans_ptr->data_len        );
    tlm2_trans.set_streaming_width ( tlmx_trans_ptr->data_len        );
    tlm2_trans.set_byte_enable_ptr ( nullptr                      );
    tlm2_trans.set_dmi_allowed     ( false                        );
    tlm2_trans.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
    switch(tlmx_trans_ptr->command) {
      case      TLMX_IGNORE: tlm2_trans.set_command(tlm::TLM_IGNORE_COMMAND);        break;
      case       TLMX_WRITE: tlm2_trans.set_command(tlm::TLM_READ_COMMAND);          break;
      case TLMX_DEBUG_WRITE: tlm2_trans.set_command(tlm::TLM_WRITE_COMMAND);         break;
      case        TLMX_READ: tlm2_trans.set_command(tlm::TLM_READ_COMMAND);          break;
      case  TLMX_DEBUG_READ: tlm2_trans.set_command(tlm::TLM_READ_COMMAND);          break;
      default              : REPORT_WARNING("Unknown TLMX command - ignored");  break;
    }//endswitch
    
    // Initiate appropriate transport
    int transferred(0);
    switch(tlmx_trans_ptr->command) {
      case TLMX_DEBUG_READ:
      case TLMX_DEBUG_WRITE:
        {
        transferred = initiator_socket->transport_dbg(tlm2_trans);
        if (transferred != tlmx_trans_ptr->data_len) REPORT_WARNING("transport_dbg returned " << transferred);
        // TODO: add this to tlmx_packet information
        break;
        }
      default :
        {
        delay = SC_ZERO_TIME;
        initiator_socket->b_transport(tlm2_trans,delay);
        wait(delay);
        break;
        }
    }//endswitch

    // Update tlmx_packet
    switch (tlm2_trans.get_response_status()) {
      case               tlm::TLM_OK_RESPONSE: tlmx_trans_ptr->status = TLMX_OK_RESPONSE;            break;
      case    tlm::TLM_ADDRESS_ERROR_RESPONSE: tlmx_trans_ptr->status = TLMX_ADDRESS_ERROR_RESPONSE; break;
      case       tlm::TLM_INCOMPLETE_RESPONSE: tlmx_trans_ptr->status = TLMX_INCOMPLETE_RESPONSE;    break;
      default                                : tlmx_trans_ptr->status = TLMX_GENERIC_ERROR_RESPONSE; break;
    }//endswitch

    // Lockdown and place in outgoing queue
    m_async_channel.nb_put(tlmx_trans_ptr);

  }//endforever
  wait(1,SC_SEC);
  REPORT_INFO("Exiting async_adaptor interface");
  sc_stop();
}//end async_adaptor_module::initiator_sysc_thread_process()

void async_adaptor_module::keep_alive_process(void)  {
  REPORT_INFO("Started " << __func__ << " " << name());
  for(;;) {
    if ( s_stop_requests > 0 ) break;
    if (!m_keep_alive_signal.read()) {
      wait(m_keep_alive_signal.default_event());
    }
    wait(SC_ZERO_TIME);
  }//endforever
  wait(1,SC_SEC);
  REPORT_INFO("Exiting due to stop request.");
  sc_stop();
}//end async_adaptor_module::keep_alive_process()

void async_adaptor_module::sighandler(int sig)
{
  ++s_stop_requests;
}

///////////////////////////////////////////////////////////////////////////////
// Helper methods -NONE-

//EOF
