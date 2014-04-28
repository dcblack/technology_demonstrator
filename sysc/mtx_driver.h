#ifndef MTX_DRIVER_H
#define MTX_DRIVER_H

#include <cstring>

// Allow it to look more like C
#define mtx_open  Mtx::open
#define mtx_close Mtx::close
#define mtx_read  Mtx::read
#define mtx_write Mtx::write
#define mtx_ioctl Mtx::ioctl

#include "dev_hls.h"

class Local_initiator_module;

struct mtx_t
{
  Data_t reg[REGISTERS];
};

struct Mtx
{
  // IOCTL requests
  enum Request_t : unsigned long { READ, WRITE };
  // Errors
  enum { ERR_OWN, ERR_DEV, ERR_NOREAD, ERR_NOWRITE, ERR_OPEN, ERR_CLOSE, ERR_FD, ERR_UNKNOWN };
  enum { BAD=-1, OK=0 };
  // Special
  static int get_owner(void) { return s_owner; }
  static int get_error(void) { return s_error; }
  // Driver calls
  static int open(const char* nam, int mode);
  static int read(int fd, int* buff, size_t size);
  static int write(int fd, int* buff, size_t size);
  static int ioctl(int fd, Request_t request, mtx_t* ctrl);
  static int close(int fd);
  // Conveniences
  static void dump_registers(mtx_t* ctrl);
  static int command(Operation_t op,int dest=0xFF,int src1=0xFF, int src2=0xFF);
  static void display_command(int cmd);
  static const char* command_cstr(int status);
  static const char* status_cstr(int status);
  // Access to hardware
  static Local_initiator_module* sys;
  static size_t devbase;
private:
  static int error(int error) { s_error = error; return BAD; }
  static int s_owner;
  static int s_error;
  static const int FD = 0x0D00F1E; // doofie magic number
  friend class Local_initiator_module;
  Mtx(Local_initiator_module* _sys, size_t _base=0) { sys = _sys; devbase = _base; }
};

#endif /*DRIVER_H*/
