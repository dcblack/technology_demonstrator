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
#include <sys/socket.h>
#include <sys/errno.h>
#include <netdb.h>
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

static const char*        MSGID = "/Doulos/example/driver";
static char               hostip[100]; /*< hold string rep of IP # */
static int                tcpip_port;
static pthread_t          pt_id;
static pthread_t          ct_id;
static int                ct_retval = 0;
static int                rc;
static int                outgoing_socket;
static struct sockaddr_in systemc_server;
static int                interrupt;
static pthread_mutex_t    server_mutex = PTHREAD_MUTEX_INITIALIZER;

void *interrupt_server(void* arg) /*< watches for "interrupt" on TCPIP PORT+1 */
{
  const char* MSGID = "/Doulos/example/interrupt_server";
  int listening_socket, incoming_socket;
  struct sockaddr_in local_server, remote_client;
  int addr_len;
  int option_value;
  const char* acknowledgement = "Ack\n";
  int retval;
  int locked = 1;
  retval = pthread_mutex_lock(&server_mutex);

  REPORT_INFO("Starting %s\n",__func__);

  /* Create socket */
  listening_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (listening_socket == -1) {
    REPORT_ERROR("Could not create socket\n");
    ct_retval = 1;
    pthread_exit(&ct_retval);
  }
  REPORT_INFO("Opened listening socket\n");

  /* Prepare the sockaddr_in structure */
  local_server.sin_family = AF_INET;
  local_server.sin_addr.s_addr = INADDR_ANY;
  local_server.sin_port = htons( tcpip_port+1 );

  option_value = 1;
  if ( setsockopt
       ( listening_socket
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
       ( listening_socket
       , (struct sockaddr *)&local_server 
       , sizeof(local_server)
       ) < 0
  ) {
    REPORT_ERROR("bind failed\n");
    ct_retval = 1;
    pthread_exit(&ct_retval);
  }
  REPORT_INFO("Bind done\n");

  /* Listen */
  listen(listening_socket, 2);

  /* watch interrupt */
  for(;;) {
    /* Accept and incoming connection */
    REPORT_INFO("Waiting for incoming connections...\n");/**/
    if (locked) {
      retval = pthread_mutex_unlock(&server_mutex);
      locked = 0;
    }
    addr_len = sizeof(struct sockaddr_in);
    incoming_socket = accept( listening_socket
                            , (struct sockaddr *)&remote_client
                            , (socklen_t*)&addr_len
                            );
    if (incoming_socket<0) {
      REPORT_ERROR("Accept failed => %s\n",strerror(errno));
      ct_retval = 1;
      pthread_exit(&ct_retval);
    }
    // Signal hardware interrupt
    interrupt = 1;
    pthread_kill(pt_id,SIGUSR1);
    REPORT_INFO("Connection accepted\n");/**/
    int ack_size = strlen(acknowledgement)+1;
    int send_count;
    send_count = write(incoming_socket, acknowledgement, ack_size);
    if (send_count < 0) {
      REPORT_ERROR("Acknowledgement write/send failed => %s\n",strerror(errno));
      ct_retval = 1;
      pthread_exit(&ct_retval);
    }
    assert(send_count == ack_size);
    close(incoming_socket);
  }
  ct_retval = 0;
  pthread_exit(&ct_retval);
  //return
}/*end interrupt_server()*/

void dev_open(char* hostname, int port)
{
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
  tcpip_port = 4000;
  if (port != 0) tcpip_port = port;

  /* Find the host */
  if ((hostentry = gethostbyname( hostname )) == NULL) {
    /* failed */
    herror("gethostbyname");
    exit(1);
  }
  addr_list = (struct in_addr **) hostentry->h_addr_list;
  for (int i=0; addr_list[i] != NULL; ++i) {
    strcpy( hostip, inet_ntoa(*addr_list[i]) );
  }

  /* Open the socket */
  outgoing_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (outgoing_socket == -1) {
    REPORT_ERROR("Could not create socket => %s\n",strerror(errno));
  }
  systemc_server.sin_addr.s_addr = inet_addr(hostip);
  systemc_server.sin_family = AF_INET;
  systemc_server.sin_port = htons( tcpip_port );
  /* Connect to host server */
  int connect_status = 0;
  int tries = 0;
  do {
    ++tries;
    connect_status = connect(outgoing_socket, (struct sockaddr *)&systemc_server, sizeof(systemc_server));
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
  pt_id = pthread_self();
  int rc;
  rc = pthread_create(&ct_id, NULL, interrupt_server, NULL/*no args*/);
  if (rc) {
    REPORT_ERROR("Unable to create pthread => %s\n",strerror(errno));
    exit(1);
  }
  sleep(1); //< allow interrupt server to start
  REPORT_INFO("Finished %s\n",__func__);
  return;
}/*end dev_open(...)*/

void dev_close(void)
{
  pthread_kill(pt_id,SIGINT);
  REPORT_INFO("Closing outgoing socket\n");
  close(outgoing_socket);
  REPORT_INFO("Finished %s\n",__func__);
  return;
}

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
                                    );
  if (debug_level > 1) { print_tlmx(payload_send_ptr,"Request"); }
  bzero(send_message,TLMX_MAX_BUFFER);
  int send_message_size;
  send_message_size = pack_tlmx(send_message,payload_send_ptr);

  /* Send to SystemC server */
//REPORT_DEBUG("Sending transaction request\n");
  int send_count;
  send_count = write(outgoing_socket, send_message, send_message_size);
  if (send_count < 0) {
    REPORT_ERROR("TCPIP write/send failed to send all data\n");
  }
  assert(send_count == send_message_size);
  
//REPORT_DEBUG("Obtaining transaction response\n");
  bzero(server_reply,TLMX_MAX_BUFFER);
  int recv_message_size;
  recv_message_size = send_message_size;
  int recv_count;
  recv_count = read(outgoing_socket, server_reply, recv_message_size);
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
}

int dev_put ( addr_t  address , data_t* data_ptr , dlen_t  data_len )
{
  return dev_transport( TLMX_WRITE, address, data_len, data_ptr );
}

int dev_get ( addr_t  address , data_t* data_ptr , dlen_t  data_len )
{
  return dev_transport( TLMX_READ, address, data_len, data_ptr );
}

int dev_put_debug ( addr_t  address , data_t* data_ptr , dlen_t  data_len )
{
  return dev_transport( TLMX_DEBUG_WRITE, address, data_len, data_ptr );
}

int dev_get_debug ( addr_t  address , data_t* data_ptr , dlen_t  data_len )
{
  return dev_transport( TLMX_DEBUG_READ, address, data_len, data_ptr );
}

void dev_wait(void)
{
  sigset_t sigset;
//REPORT_DEBUG("Waiting for SIGUSR1\n");
  if (0 != sigemptyset(&sigset)) {
    REPORT_ERROR("sigemptyset failed => %s\n",strerror(errno));
  }
  if (0 != sigaddset(&sigset,SIGUSR1)) {
    REPORT_ERROR("sigaddset failed => %s\n",strerror(errno));
  }
  int sig;
  if (0 != sigwait(&sigset,&sig)) {
    REPORT_ERROR("sigwait failed => %s\n",strerror(errno));
  }
  assert(sig == SIGUSR1);
}


debug_t dev_debug(long long int level)
{
  debug_t prev_level;
  prev_level = debug_level;
  if (level >= 0) { debug_level = (debug_t) level; }
  return prev_level;
}

/*
 * TAF!
 */
