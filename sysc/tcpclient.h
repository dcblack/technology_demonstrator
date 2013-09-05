#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <map>
#include <string>

struct tcpclient
{
  // Constructors/destructors
  tcpclient(bool quiet = false);
  tcpclient(const std::string& hostname, uint32_t port);
  ~tcpclient(void);
  // No copies
  tcpclient(const tcpclient& rhs) = delete;
  tcpclient& operator=(const tcpclient& rhs) = delete;
  // Standard methods
  void connect(const std::string& hostname="", uint32_t port=0);
  void disconnect(void);
  unsigned int send(uint32_t size=0, const std::string& buffer=nullptr);
  unsigned int recv(unsigned int max_size=0, char* buffer=0);
  unsigned int send(const std::string& message);
  unsigned int recv(unsigned int max_size, std::string& message);
  char* buffer(void) { return m_buffer; };
  void  setup(char* buffer, uint32_t* size_ptr, uint32_t max_size=0);
  std::string host(void) const { return m_host; }
  int         port(void) const { return m_port; }
private:
  void init();
  enum { INACTIVE, DEAD, CLOSED, OPENING, ACTIVE } m_state{INACTIVE};
  unsigned long      long int m_sends{0ULL};
  unsigned long      long int m_recvs{0ULL};
  std::string        m_host;
  int                m_port;
  uint32_t*          m_size_ptr;
  uint32_t           m_max_size;
  char*              m_buffer;
  int                m_socket;
  struct sockaddr_in m_client;
  char               m_hostip[100]; /*< hold string rep of IP # */
  std::string        client; // for debug
  static std::map<std::string,tcpclient*> clients;
};

#endif /*TCPCLIENT_H*/
