#ifndef DEV_H
#define DEV_H
#include <cassert>

// DESCRIPTION
//
// This class provides an API to the hardware device.
//
// This is a singleton type of class with a twist. Although, it can only be
// instantiated once, it is important that we ensure its destructor be invoked
// to enable an RAII approach to opening and closing the device. So it must be
// created by a user on the stack, and we don't dare allow its construction via
// the get_dev() method.

#include "dev_hls.h"
#include <map>
#include <string>
#include <cassert>

class Matrix;

struct Dev
{
  Dev(size_t addr); // Constructor
  ~Dev(void); // Destructor
  void open(size_t addr); // initialize device (normally called by constructor)
  void close(void); // close down device
  static Dev* get_dev(void) { assert (singleton_ptr != 0); return singleton_ptr; }
  bool active(void) const { return (dev_addr != 0); }
  bool alloc_mreg(Matrix& m);
  void free_mreg(Matrix& m);
  Reg_t alloc_reg(size_t r=12);
  void free_reg(Reg_t reg);
  bool alloc_mem(Matrix& m);
  void free_mem(Matrix& m);
  CmdState_t do_command
    ( Operation_t cmnd
    , Reg_t       dest=UNUSED
    , Reg_t       src1=UNUSED
    , Reg_t       src2=UNUSED
    );
  int  get_return(void) const;
  int  get_reg(Reg_t reg) const;
  void set_reg(Reg_t reg, int val);
  volatile int* mem_ptr(size_t addr) const;
  int  get_mem(size_t addr) const;
  void set_mem(size_t addr, int val);
private:
  // Attributes
  size_t    dev_addr; //< non-zero indicates device active
  size_t    page_offset;
  size_t    regs_addr; //< dev base register address
  size_t    imem_addr; //< dev internal memory address
  size_t    control_addr; //< dev start
  size_t    return_addr; //< returned result/status
  size_t    regs_used;
  int       mmap_fd;
  volatile int32_t* mmap_ptr;
  volatile int32_t* dev_ptr;
  typedef std::map<size_t,Matrix*> mem_used_t;
  mem_used_t mem_used;
  static Dev*     singleton_ptr;
  static const char* argv0;
  // Methods
  Dev(const Dev& rhs) { assert(0); } // delete copy constructor
  Dev& operator=(const Dev& rhs) { assert(0); } // delete assignment
  bool using_dev(void) { return dev_ptr != 0; }
};

#endif /*DEV_H*/
