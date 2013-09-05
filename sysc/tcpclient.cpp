#include <systemc>
#include "tcpclient.h"
#include "report.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
using namespace std;
using namespace sc_core;

namespace {
  static const char* MSGID = "/Doulos/example/tcpclient";
}

map<string,tcpclient*> tcpclient::clients;

////////////////////////////////////////////////////////////////////////////////
// Default constructor
tcpclient::tcpclient(bool quiet)
: m_host("")
, m_port(0)
, m_size_ptr(nullptr)
, m_max_size(0)
, m_buffer(nullptr)
, m_socket(0)
{
  if (not quiet) REPORT_INFO("Constructed " << __func__);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor with immediate connection
tcpclient::tcpclient(const string& hostname, uint32_t port)
: tcpclient(true)
{
  this->connect(hostname,port);
  REPORT_INFO("Constructed " << __func__);
}

////////////////////////////////////////////////////////////////////////////////
void tcpclient::connect(const string& hostname, uint32_t port)
{
  //****************************************************************************
  // Setup outgoing TCPIP connection
  //****************************************************************************
  struct hostent*  hostentry;
  struct in_addr** addr_list;
  if (m_host.size() == 0) { m_host = "localhost"; }
  {
    ostringstream sout;
    sout << m_host << ":" << m_port << ends;
    client = sout.str();
  }
  if (clients.count(client) != 0) {
    REPORT_ALERT("Client " << client << " already exists");
    m_state = DEAD;
    return;
  }

  // Find the host
  if ((hostentry = gethostbyname( m_host.c_str() )) == nullptr) {
    REPORT_ALERT("Failed to gethostbyname => " << strerror(errno));
    m_state = DEAD;
    return;
  }
  addr_list = (struct in_addr **) hostentry->h_addr_list;
  for (int i=0; addr_list[i] != nullptr; ++i) {
    strcpy( m_hostip, inet_ntoa(*addr_list[i]) );
  }

  // Open the socket
  m_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (m_socket == -1) {
    REPORT_ALERT("Could not create socket => " << strerror(errno));
    m_state = DEAD;
    return;
  }
  m_client.sin_addr.s_addr = inet_addr(m_hostip);
  m_client.sin_family = AF_INET;
  m_client.sin_port = htons( m_port );
  // Connect to host server
  int connect_status = 0;
  int tries = 0;
  do {
    ++tries;
    connect_status = ::connect(m_socket, (struct sockaddr *)&m_client, sizeof(m_client));
    if (connect_status < 0) {
      if (errno == ECONNREFUSED) {
        if (tries%5 == 1) {
          REPORT_INFO("Waiting for connection to " << client << " - start client?");
        }
        sleep(1);
      } else {
        REPORT_ALERT(errno << " " << strerror(errno));
        m_state = DEAD;
        return;
      }
    }
  } while (connect_status < 0 && errno == ECONNREFUSED);
  REPORT_INFO("Connected to " << client);

  // Activate
  m_state = ACTIVE;
  clients[client] = this;

}//end connect

////////////////////////////////////////////////////////////////////////////////
// Destructor
tcpclient::~tcpclient(void)
{
  this->disconnect();
  if (clients.count(client)) {
    clients.erase(client);
  }
  REPORT_INFO("Destroyed " << __func__);
}

////////////////////////////////////////////////////////////////////////////////
void tcpclient::disconnect(void)
{
  if (m_state == ACTIVE) {
    REPORT_INFO("Closed " << client);
    close(m_socket);
    clients.erase(client);
    m_state = CLOSED;
  }
}

////////////////////////////////////////////////////////////////////////////////
void tcpclient::setup(char* buffer, uint32_t* size_ptr, uint32_t max_size)
{
  sc_assert(buffer!=nullptr && size_ptr!=nullptr && *size_ptr!=0U);
  m_buffer   = buffer;
  m_size_ptr = size_ptr;
  m_max_size = max_size;
}

////////////////////////////////////////////////////////////////////////////////
unsigned int tcpclient::send(uint32_t size, const string& buffer)
{
  sc_assert(m_state == ACTIVE);
  unsigned int send_count = 0;
  if (buffer != "" && size != 0) {
    send_count = write(m_socket, buffer.c_str(), size);
    ++m_sends;
  } else if (m_buffer!=nullptr && *m_size_ptr!=0) {
    send_count = write(m_socket, m_buffer, *m_size_ptr);
    ++m_sends;
  } else {
    REPORT_ERROR("Nothing provided to send!?");
  }//endif
  return send_count;
}

////////////////////////////////////////////////////////////////////////////////
unsigned int tcpclient::recv(unsigned int max_size, char* buffer)
{
  if (max_size == 0) max_size = m_max_size;
  sc_assert(m_state == ACTIVE && max_size > 0);
  if (buffer == nullptr) {
#ifndef NDEBUG
    memset(m_buffer,0,max_size); //< aids debug
#endif
    *m_size_ptr = read(m_socket, m_buffer, max_size);
    ++m_recvs;
  } else {
#ifndef NDEBUG
    memset(buffer,0,max_size); //< aids debug
#endif
    *m_size_ptr = read(m_socket, buffer, max_size);
    ++m_recvs;
  }//endif
  return *m_size_ptr;
}

////////////////////////////////////////////////////////////////////////////////
unsigned int tcpclient::send(const string& message)
{
  return send(message.size(),message.c_str());
}

////////////////////////////////////////////////////////////////////////////////
unsigned int tcpclient::recv(unsigned int max_size, string& message)
{
  char* buffer(new char[max_size]);
  unsigned int recvd_count = recv(max_size, buffer);
  message = *buffer;
  return recvd_count;
}

//EOF
