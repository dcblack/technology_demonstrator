// FILE: driver.c

////////////////////////////////////////////////////////////////////////////////
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
////////////////////////////////////////////////////////////////////////////////

#include "driver.h"
#include "tlmx_packet.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include "creport.h"

#if __STDC_VERSION__ < 199901L
#error Requires C99
#endif

typedef enum { INITIATOR_PORT, SIGNAL_PORT, TARGET_PORT } port_t;

static const char*        MSGID = "/Doulos/example/driver";
static char               hostip[100]; /*< hold string rep of IP # */
static int                tcpip_port[3] = {4000, 4001, 4100};
static pthread_t          main_id;   // identifies the main thread
static pthread_t          initiator_id; // identifies the initiator server thread
static int                initiator_exitcode = 0;
static pthread_t          target_id; // identifies the target server thread
static int                target_exitcode = 0;
static int                outgoing_initiator_socket;
static struct sockaddr_in systemc_server;
static int                interrupt_flag  = 0;
static pthread_mutex_t    interrupt_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t     interrupt_cond  = PTHREAD_COND_INITIALIZER;
static int                initiator_ready    = 0;
static pthread_mutex_t    initiator_mutex    = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t     initiator_cond     = PTHREAD_COND_INITIALIZER;
static int                initiator_stop     = 0; //< indicates server should stop
static const char*        acknowledgement = "Ack\n";

////////////////////////////////////////////////////////////////////////////////
//****************************************************************************//
//*                                                                          *//
//*  ### #####   ####          ####  ##### #####  #     # ##### #####        *//
//*   #  #    # #    #        #    # #     #    # #     # #     #    #       *//
//*   #  #    # #    #        #      #     #    # #     # #     #    #       *//
//*   #  #####  #    #         ####  ##### #####  #     # ##### #####        *//
//*   #  #  #   #  # #             # #     #  #    #   #  #     #  #         *//
//*   #  #   #  #   #         #    # #     #   #    # #   #     #   #        *//
//*  ### #    #  ### # ######  ####  ##### #    #    #    ##### #    #       *//
//*                                                                          *//
//****************************************************************************//
////////////////////////////////////////////////////////////////////////////////
void *interrupt_server(void* arg) /*< watches for "interrupt" on TCPIP PORT+1 */
{
  const char* MSGID = "/Doulos/example/interrupt_server";
  int listening_interrupt_socket, incoming_interrupt_socket;
  struct sockaddr_in local_server, remote_client;
  int addr_len;
  int option_value;
  static int server_retval = 0; //< static to aid debug
  server_retval = pthread_mutex_lock(&server_mutex);
  if (server_retval) {
    REPORT_ERROR("Unable to lock server_mutex => %s\n",strerror(errno));
    exit(1);
  }

  REPORT_INFO("Starting %s\n",__func__);

  /* Create socket */
  listening_interrupt_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listening_interrupt_socket == -1) {
    REPORT_ERROR("Could not create socket\n");
    server_exitcode = 1;
    pthread_exit(&server_exitcode);
  }
  REPORT_INFO("Opened listening socket\n");

  /* Prepare the sockaddr_in structure */
  local_server.sin_family = AF_INET;
  local_server.sin_addr.s_addr = INADDR_ANY;
  local_server.sin_port = htons( tcpip_port[INTERRUPT_PORT] );

  option_value = 1;
  if ( setsockopt
       ( listening_interrupt_socket
       , SOL_SOCKET
       , SO_REUSEADDR
       , (void* )&option_value
       , (socklen_t)(sizeof(socklen_t))
       ) < 0
  ) {
    REPORT_ERROR("Unable to set socket option\n");
  }
  REPORT_INFO("Socket options set.\n");

   
  /* Bind */
  if ( bind
       ( listening_interrupt_socket
       , (struct sockaddr *)&local_server 
       , sizeof(local_server)
       ) < 0
  ) {
    REPORT_ERROR("bind failed\n");
    server_exitcode = 1;
    pthread_exit(&server_exitcode);
  }
  REPORT_INFO("Bind done\n");

  /* Listen */
  listen(listening_interrupt_socket, 2);

  // Indicate server is now initialized/ready
  server_ready = 1;
  server_retval = pthread_cond_signal(&server_cond);
  if (server_retval) {
    REPORT_ERROR("Unable to signal server_cond => %s\n",strerror(errno));
    exit(1);
  }
  server_retval = pthread_mutex_unlock(&server_mutex);
  if (server_retval) {
    REPORT_ERROR("Unable to unlock server_mutex => %s\n",strerror(errno));
    exit(1);
  }

  /* watch interrupt */
  for(;;) {
    /* Accept and incoming connection */
    REPORT_INFO("Waiting for incoming connections...\n");/**/
    addr_len = sizeof(struct sockaddr_in);
    incoming_interrupt_socket = accept( listening_interrupt_socket
                            , (struct sockaddr *)&remote_client
                            , (socklen_t*)&addr_len
                            );
    if (incoming_interrupt_socket<0) {
      REPORT_ERROR("Accept failed => %s\n",strerror(errno));
      continue; // Try again
    }
    /* Check to see if shutdown requested */
    if (server_stop) {
      break;
    }
    // Signal hardware interrupt
    server_retval = pthread_mutex_lock(&interrupt_mutex);
    if (server_retval) {
      REPORT_ERROR("Unable to lock interrupt_mutex => %s\n",strerror(errno));
      exit(1);
    }
    interrupt_flag = 1;
    server_retval = pthread_cond_signal(&interrupt_cond);
    if (server_retval) {
      REPORT_ERROR("Unable to signal interrupt_cond => %s\n",strerror(errno));
      exit(1);
    }
    server_retval = pthread_mutex_unlock(&interrupt_mutex);
    if (server_retval) {
      REPORT_ERROR("Unable to unlock interrupt_mutex => %s\n",strerror(errno));
      exit(1);
    }
    REPORT_INFO("Connection accepted\n");/**/
    int ack_size = strlen(acknowledgement)+1;
    int send_count;
    send_count = write(incoming_interrupt_socket, acknowledgement, ack_size);
    if (send_count < 0) {
      REPORT_ERROR("Acknowledgement write/send failed => %s\n",strerror(errno));
      continue; // Try again
    }
    assert(send_count == ack_size);
    close(incoming_interrupt_socket);
  }/*endforever*/
  server_exitcode = 0;
  pthread_exit(&server_exitcode);
  //return
}/*end interrupt_server()*/

////////////////////////////////////////////////////////////////////////////////
//****************************************************************************//
//*                                                                          *//
//*  #######    #    ####   ####  #### #######      ###  ####  #   # ####    *//
//*     #      # #   #   # #    # #       #        #   # #   # #   # #   #   *//
//*     #     #   #  #   # #      #       #        #     #   # #   # #   #   *//
//*     #    #     # ####  #  ### ####    #         ###  ####  #   # ####    *//
//*     #    ####### # #   #    # #       #            # # #   #   # # #     *//
//*     #    #     # #  #  #    # #       #        #   # #  #   # #  #  #    *//
//*     #    #     # #   #  ####  ####    #   ####  ###  #   #   #   #   #   *//
//*                                                                          *//
//****************************************************************************//
////////////////////////////////////////////////////////////////////////////////
void *target_server(void* arg) /*< watches for "target" on TCPIP */
{
  const char* MSGID = "/Doulos/example/target_server";
  int listening_target_socket, incoming_target_socket;
  struct sockaddr_in local_server, remote_client;
  int addr_len;
  int option_value;
  static int server_retval = 0; //< static to aid debug
  server_retval = pthread_mutex_lock(&server_mutex);
  if (server_retval) {
    REPORT_ERROR("Unable to lock server_mutex => %s\n",strerror(errno));
    exit(1);
  }

  REPORT_INFO("Starting %s\n",__func__);

  /* Create socket */
  listening_target_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listening_target_socket == -1) {
    REPORT_ERROR("Could not create socket\n");
    server_exitcode = 1;
    pthread_exit(&server_exitcode);
  }
  REPORT_INFO("Opened listening socket\n");

  /* Prepare the sockaddr_in structure */
  local_server.sin_family = AF_INET;
  local_server.sin_addr.s_addr = INADDR_ANY;
  local_server.sin_port = htons( tcpip_port[TARGET_PORT] );

  option_value = 1;
  if ( setsockopt
       ( listening_target_socket
       , SOL_SOCKET
       , SO_REUSEADDR
       , (void* )&option_value
       , (socklen_t)(sizeof(socklen_t))
       ) < 0
  ) {
    REPORT_ERROR("Unable to set socket option\n");
  }
  REPORT_INFO("Socket options set.\n");

   
  /* Bind */
  if ( bind
       ( listening_target_socket
       , (struct sockaddr *)&local_server 
       , sizeof(local_server)
       ) < 0
  ) {
    REPORT_ERROR("Target bind failed\n");
    server_exitcode = 1;
    pthread_exit(&server_exitcode);
  }
  REPORT_INFO("Target bind done\n");

  /* Listen */
  listen(listening_target_socket, 2);

  // Indicate server is now initialized/ready
  server_ready = 1;
  server_retval = pthread_cond_signal(&server_cond);
  if (server_retval) {
    REPORT_ERROR("Unable to signal server_cond => %s\n",strerror(errno));
    exit(1);
  }
  server_retval = pthread_mutex_unlock(&server_mutex);
  if (server_retval) {
    REPORT_ERROR("Unable to unlock server_mutex => %s\n",strerror(errno));
    exit(1);
  }

  /* watch target */
  for(;;) {
    /* Accept and incoming connection */
    REPORT_INFO("Waiting for incoming connections...\n");/**/
    addr_len = sizeof(struct sockaddr_in);
    incoming_target_socket = accept( listening_target_socket
                            , (struct sockaddr *)&remote_client
                            , (socklen_t*)&addr_len
                            );
    if (incoming_target_socket<0) {
      REPORT_ERROR("Accept failed => %s\n",strerror(errno));
      continue; // Try again
    }
    /* Check to see if shutdown requested */
    if (server_stop) {
      break;
    }
    // Signal hardware target
    server_retval = pthread_mutex_lock(&target_mutex);
    if (server_retval) {
      REPORT_ERROR("Unable to lock target_mutex => %s\n",strerror(errno));
      exit(1);
    }
    target_flag = 1;
    server_retval = pthread_cond_signal(&target_cond);
    if (server_retval) {
      REPORT_ERROR("Unable to signal target_cond => %s\n",strerror(errno));
      exit(1);
    }
    server_retval = pthread_mutex_unlock(&target_mutex);
    if (server_retval) {
      REPORT_ERROR("Unable to unlock target_mutex => %s\n",strerror(errno));
      exit(1);
    }
    REPORT_INFO("Connection accepted\n");/**/
    int ack_size = strlen(acknowledgement)+1;
    int send_count;
    send_count = write(incoming_target_socket, acknowledgement, ack_size);
    if (send_count < 0) {
      REPORT_ERROR("Acknowledgement write/send failed => %s\n",strerror(errno));
      continue; // Try again
    }
    assert(send_count == ack_size);
    close(incoming_target_socket);
  }/*endforever*/
  server_exitcode = 0;
  pthread_exit(&server_exitcode);
  //return
}/*end target_server()*/

////////////////////////////////////////////////////////////////////////////////
//****************************************************************************//
//*                                                                          *//     *
//*  ####   ##### #     #         ####  #####  ##### #     #                 *//     *
//*  #   #  #     #     #        #    # #    # #     ##    #                 *//     *
//*  #    # #     #     #        #    # #    # #     # #   #                 *//     *
//*  #    # ##### #     #        #    # #####  ##### #  #  #                 *//     *
//*  #    # #      #   #         #    # #      #     #   # #                 *//     *
//*  #   #  #       # #          #    # #      #     #    ##                 *//     *
//*  ####   #####    #    ######  ####  #      ##### #     #                 *//     *
//*                                                                          *//     *
//****************************************************************************//
////////////////////////////////////////////////////////////////////////////////
void dev_open(char* hostname, int port_arr[])
{
  static int dev_open_retval = 0; //< static to aid debug
  REPORT_INFO("Starting %s\n", __func__);
  debug_level = 2;
  /*
   *****************************************************************************
   * Setup outgoing TCPIP connection
   *****************************************************************************
   */
  struct hostent*  hostentry;
  struct in_addr** addr_list;
  if (hostname == NULL || hostname[0] == '\0') { hostname = "localhost"; }
  for (int i=0; i!=3; ++i) {
    if (port_arr[i] == 0) break;
    tcpip_port[i] = port_arr[i];
  }

  /* Display configuration */
  REPORT_INFO("INITIATOR_PORT => %d\n", port_arr[INITIATOR_PORT]);
  REPORT_INFO("SIGNAL_PORT    => %d\n", port_arr[SIGNAL_PORT   ]);
  REPORT_INFO("TARGET_PORT    => %d\n", port_arr[TARGET_PORT   ]);

  /* Find the host */
  if ((hostentry = gethostbyname( hostname )) == NULL) {
    /* failed */
    REPORT_ERROR("Failed to gethostbyname => %s\n",strerror(errno));
    exit(1);
  }
  addr_list = (struct in_addr **) hostentry->h_addr_list;
  for (int i=0; addr_list[i] != NULL; ++i) {
    strcpy( hostip, inet_ntoa(*addr_list[i]) );
  }

  /* Open the socket */
  outgoing_initiator_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (outgoing_initiator_socket == -1) {
    REPORT_ERROR("Could not create socket => %s\n",strerror(errno));
  }
  systemc_server.sin_addr.s_addr = inet_addr(hostip);
  systemc_server.sin_family = AF_INET;
  systemc_server.sin_port = htons( tcpip_port[INITIATOR_PORT] );
  /* Connect to host server */
  int connect_status = 0;
  int tries = 0;
  do {
    ++tries;
    connect_status = connect(outgoing_initiator_socket, (struct sockaddr *)&systemc_server, sizeof(systemc_server));
    if (connect_status < 0) {
      if (errno == ECONNREFUSED)
      {
        if (tries%5 == 1) 
        {
          REPORT_INFO("Waiting for connection - start SystemC?\n");/**/
        }
        sleep(1);
      }
      else
      {
        REPORT_ERROR("%d %s\n",errno,strerror(errno));
        exit(1);
      }
    }
  } while (connect_status < 0 && errno == ECONNREFUSED);
  REPORT_INFO("Connected\n");

  /*
   *****************************************************************************
   * Spawn interrupt server
   *****************************************************************************
   */
  main_id = pthread_self();
  dev_open_retval = pthread_create(&server_id, NULL, interrupt_server, NULL/*no args*/);
  if (dev_open_retval) {
    REPORT_ERROR("Unable to create pthread => %s\n",strerror(errno));
    exit(1);
  }
  // allow interrupt server to start
  dev_open_retval = pthread_mutex_lock(&server_mutex);
  if (dev_open_retval) {
    REPORT_ERROR("Unable to lock server_mutex => %s\n",strerror(errno));
    exit(1);
  }
  dev_open_retval = pthread_cond_wait(&server_cond,&server_mutex);
  if (dev_open_retval) {
    REPORT_ERROR("Unable to wait for server_cond => %s\n",strerror(errno));
    exit(1);
  }
  dev_open_retval = pthread_mutex_unlock(&server_mutex);
  if (dev_open_retval) {
    REPORT_ERROR("Unable to unlock server_mutex => %s\n",strerror(errno));
    exit(1);
  }

  REPORT_INFO("Finished %s\n",__func__);
  return;
}/*end dev_open(...)*/

////////////////////////////////////////////////////////////////////////////////
//****************************************************************************//
//*                                                                          *//
//*  ###   ##### #     #      ####   ###  #### #######     ### ####   ###    *//
//*  #  #  #     #     #     #    # #   # #       #         #  #   # #   #   *//
//*  #   # #     #     #     #      #   # #       #         #  #   # #   #   *//
//*  #   # ##### #     #      ####  #   # ####    #         #  ####  #   #   *//
//*  #   # #      #   #           # #   # #       #         #  # #   # # #   *//
//*  #  #  #       # #       #    # #   # #       #         #  #  #  #  #    *//
//*  ###   #####    #  #####  ####   ###  #       #  ##### ### #   #  ## #   *//
//*                                                                          *//
//****************************************************************************//
////////////////////////////////////////////////////////////////////////////////
void dev_soft_interrupt(const char* irq_message) /*< send a software interrupt*/
{
  struct hostent*   hostentry;
  struct in_addr**  addr_list;
  const char* const hostname = "localhost";
  char              ack_reply[TLMX_MAX_BUFFER];
  int               interrupt_socket;
  if (irq_message == NULL) {
    irq_message = "Software interrupt\n";
  }

  /* Find the host */
  if ((hostentry = gethostbyname( hostname )) == NULL) {
    /* failed */
    REPORT_ERROR("Failed to gethostbyname => %s\n",strerror(errno));
    exit(1);
  }
  addr_list = (struct in_addr **) hostentry->h_addr_list;
  for (int i=0; addr_list[i] != NULL; ++i) {
    strcpy( hostip, inet_ntoa(*addr_list[i]) );
  }

  /* Open the socket */
  interrupt_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (interrupt_socket == -1) {
    REPORT_ERROR("Could not create socket => %s\n",strerror(errno));
  }
  systemc_server.sin_addr.s_addr = inet_addr(hostip);
  systemc_server.sin_family = AF_INET;
  systemc_server.sin_port = htons( tcpip_port[INTERRUPT_PORT] );
  /* Connect to host server */
  int connect_status = 0;
  int tries = 0;
  do {
    ++tries;
    connect_status = connect(interrupt_socket, (struct sockaddr *)&systemc_server, sizeof(systemc_server));
    if (connect_status < 0) {
      if (errno == ECONNREFUSED)
      {
        if (tries%5 == 1) 
        {
          REPORT_INFO("Waiting for server connection\n");/**/
        }
        sleep(1);
      }
      else
      {
        REPORT_ERROR("%d %s\n",errno,strerror(errno));
        exit(1);
      }
    }
  } while (connect_status < 0 && errno == ECONNREFUSED);

  int irq_message_size;
  irq_message_size = strlen(irq_message);
  int send_count;
  send_count = write(interrupt_socket, irq_message, irq_message_size);
  // ignore send_count, it's just the fact of sending that matters
  int ack_reply_size;
  ack_reply_size = strlen(ack_reply);
  int recv_count;
  recv_count = read(interrupt_socket, ack_reply, ack_reply_size);
  if (recv_count < 0) {
    REPORT_ERROR("TCPIP read/recv didn't receive enough data\n");
  }
  close(interrupt_socket);
}/*end dev_soft_interrupt(...)*/

////////////////////////////////////////////////////////////////////////////////
//****************************************************************************//
//*                                                                          *//
//*  ####   ##### #     #         ####  #      ####   ####  #####            *//
//*  #   #  #     #     #        #    # #     #    # #    # #                *//
//*  #    # #     #     #        #      #     #    # #      #                *//
//*  #    # ##### #     #        #      #     #    #  ####  #####            *//
//*  #    # #      #   #         #      #     #    #      # #                *//
//*  #   #  #       # #          #    # #     #    # #    # #                *//
//*  ####   #####    #    ######  ####  #####  ####   ####  #####            *//
//*                                                                          *//
//****************************************************************************//
////////////////////////////////////////////////////////////////////////////////
void dev_close(void)
{
  REPORT_INFO("Closing outgoing socket\n");
  close(outgoing_initiator_socket);

  // stop the interrupt server
  server_stop = 1;
  dev_soft_interrupt("Shutdown\n"); //< send soft interrupt for force notice of interrupt

  REPORT_INFO("Finished %s\n",__func__);
  return;
}/*end dev_close()*/

////////////////////////////////////////////////////////////////////////////////
//****************************************************************************//
//*                                                                          *//
//*  ##   ### #   #   ##### ####    #   #    #  ###  ####   ###  ####  ##### *//
//*  # #  #   #   #     #   #   #  # #  ##   # #   # #   # #   # #   #   #   *//
//*  #  # #   #   #     #   #   # #   # # #  # #     #   # #   # #   #   #   *//
//*  #  # ### #   #     #   ####  #   # # #  #  ###  ####  #   # ####    #   *//
//*  #  # #   #   #     #   # #   ##### #  # #     # #     #   # # #     #   *//
//*  # #  #    # #      #   #  #  #   # #   ## #   # #     #   # #  #    #   *//
//*  ##   ###   #  #### #   #   # #   # #    #  ###  #      ###  #   #   #   *//
//*                                                                          *//
//****************************************************************************//
////////////////////////////////////////////////////////////////////////////////
int dev_transport
( tlmx_command_t  command
, addr_t  address
, dlen_t  data_len
, data_t* data_ptr
)
{
  int          status = 0;
  tlmx_packet* payload_send_ptr = NULL;
  char         send_message[TLMX_MAX_BUFFER];
  tlmx_packet* payload_recv_ptr = NULL;
  char         server_reply[TLMX_MAX_BUFFER];

  /* Compose transaction */
  payload_send_ptr = new_tlmx_packet( command
                                    , address
                                    , data_len
                                    , data_ptr
                                    , 0UL
                                    , 0UL
                                    );
  if (debug_level > 1) { print_tlmx(payload_send_ptr,"Request"); }
#ifndef NDEBUG
  memset(send_message,0,TLMX_MAX_BUFFER); //< aids debug
#endif
  int send_message_size;
  send_message_size = pack_tlmx(send_message,payload_send_ptr);

  /* Send to SystemC server */
//REPORT_DEBUG("Sending transaction request\n");
  int send_count;
  send_count = write(outgoing_initiator_socket, send_message, send_message_size);
  if (send_count < 0) {
    REPORT_ERROR("TCPIP write/send failed to send all data\n");
  }
  assert(send_count == send_message_size);
  
//REPORT_DEBUG("Obtaining transaction response\n");
#ifndef NDEBUG
  memset(server_reply,0,TLMX_MAX_BUFFER); //< aids debug
#endif
  int recv_message_size;
  recv_message_size = send_message_size;
  int recv_count;
  recv_count = read(outgoing_initiator_socket, server_reply, recv_message_size);
  if (recv_count < 0) {
    REPORT_ERROR("TCPIP read/recv didn't receive enough data\n");
  }
  assert(recv_count == recv_message_size);
  payload_recv_ptr = clone_tlmx_packet(payload_send_ptr);
  unpack_tlmx(payload_recv_ptr,server_reply);

  if (debug_level > 1) { print_tlmx(payload_recv_ptr,"Response"); }
  if (payload_recv_ptr->status != TLMX_OK_RESPONSE) {
    REPORT_ERROR("%s(addr=%0llx) got %s\n"
           , tlmx_command_to_str(payload_recv_ptr->command)
           , payload_recv_ptr->address
           , tlmx_status_to_str(payload_recv_ptr->status)
           );
    status = -1;
  }
  delete_tlmx_packet(payload_send_ptr);
  delete_tlmx_packet(payload_recv_ptr);
  return status;
}/*end dev_transport(...)*/

////////////////////////////////////////////////////////////////////////////////
int dev_put ( addr_t  address , data_t* data_ptr , dlen_t  data_len )
{
  return dev_transport( TLMX_WRITE, address, data_len, data_ptr );
}/*end dev_put(...)*/

////////////////////////////////////////////////////////////////////////////////
int dev_get ( addr_t  address , data_t* data_ptr , dlen_t  data_len )
{
  return dev_transport( TLMX_READ, address, data_len, data_ptr );
}/*end dev_get(...)*/

////////////////////////////////////////////////////////////////////////////////
int dev_put_debug ( addr_t  address , data_t* data_ptr , dlen_t  data_len )
{
  return dev_transport( TLMX_DEBUG_WRITE, address, data_len, data_ptr );
}/*end dev_put_debug(...)*/

////////////////////////////////////////////////////////////////////////////////
int dev_get_debug ( addr_t  address , data_t* data_ptr , dlen_t  data_len )
{
  return dev_transport( TLMX_DEBUG_READ, address, data_len, data_ptr );
}/*end dev_get_debug(...)*/

////////////////////////////////////////////////////////////////////////////////
//****************************************************************************//
//*                                                                          *//
//*  ####   ##### #     #        #     #    #    ### #######                 *//
//*  #   #  #     #     #        #  #  #   # #    #     #                    *//
//*  #    # #     #     #        #  #  #  #   #   #     #                    *//
//*  #    # ##### #     #        #  #  # #     #  #     #                    *//
//*  #    # #      #   #         #  #  # #######  #     #                    *//
//*  #   #  #       # #          #  #  # #     #  #     #                    *//
//*  ####   #####    #    ######  ## ##  #     # ###    #                    *//
//*                                                                          *//
//****************************************************************************//
////////////////////////////////////////////////////////////////////////////////
void dev_wait(void)
{
  static int dev_wait_retval = 0; //< static to aid debug
//REPORT_DEBUG("Waiting for SIGUSR1\n");
  dev_wait_retval = pthread_mutex_lock(&interrupt_mutex);
  if (dev_wait_retval) {
    REPORT_ERROR("Unable to lock interrupt_mutex => %s\n",strerror(errno));
    exit(1);
  }
  while (!interrupt_flag) {
    dev_wait_retval = pthread_cond_wait(&interrupt_cond,&interrupt_mutex);
    if (dev_wait_retval) {
      REPORT_ERROR("Unable to wait for interrupt_cond => %s\n",strerror(errno));
      exit(1);
    }
  }
  dev_wait_retval = pthread_mutex_unlock(&interrupt_mutex);
  if (dev_wait_retval) {
    REPORT_ERROR("Unable to unlock interrupt_mutex => %s\n",strerror(errno));
    exit(1);
  }
}/*end dev_wait()*/

////////////////////////////////////////////////////////////////////////////////
//****************************************************************************//
//*                                                                          *//
//*  ####   ##### #     #        ####   ##### #####  #    #  ####            *//
//*  #   #  #     #     #        #   #  #     #    # #    # #    #           *//
//*  #    # #     #     #        #    # #     #    # #    # #                *//
//*  #    # ##### #     #        #    # ##### #####  #    # #  ###           *//
//*  #    # #      #   #         #    # #     #    # #    # #    #           *//
//*  #   #  #       # #          #   #  #     #    # #    # #    #           *//
//*  ####   #####    #    ###### ####   ##### #####   ####   ####            *//
//*                                                                          *//
//****************************************************************************//
////////////////////////////////////////////////////////////////////////////////
debug_t dev_debug(long long int level)
{
  debug_t prev_level;
  prev_level = debug_level;
  if (level >= 0) { debug_level = (debug_t) level; }
  return prev_level;
}/*end dev_debug(...)*/

/*
 * TAF!
 */
