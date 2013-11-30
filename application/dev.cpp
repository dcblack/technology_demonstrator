////////////////////////////////////////////////////////////////////////////////
// Implementation
//------------------------------------------------------------------------------
// This implementation uses the crude memory mapping via /dev/mem.
// A more advanced interface would utilize UIO or perhaps even a real device
// driver.

#define NOT_YET_IMPLEMENTED assert("Not yet implemented")

#include "dev.h"
#include "matrix.h"
#include <cstdlib>
// unistd.h -- for sysconf() call, which is used to query information about the runtime system
#include <unistd.h>
// sys/mman.h for mmap() function declaration
#include <sys/mman.h>
// fcntl.h -- for file open flags and modes
#include <fcntl.h>
// xdev_hls_hw.h -- register offsets
#include "xdev_hls_hw.h"
#include <cstdio>
#include <cassert>
using namespace std;

static const size_t reg_width = XDEV_HLS_DEVREG_WIDTH_REG_R/8;
static const size_t mem_width = XDEV_HLS_DEVREG_WIDTH_MEM/8;
Dev*   Dev::singleton_ptr = 0;
const char*  Dev::argv0 = "dev";

//------------------------------------------------------------------------------
Dev::Dev(size_t addr) //< Constructor
: dev_addr     (  0 )
, page_offset  (  0 )
, regs_addr    (  0 )
, imem_addr    (  0 )
, control_addr (  0 )
, return_addr  (  0 )
, regs_used    (  0 )
, mmap_fd      ( -1 )
, mmap_ptr     (  0 )
, dev_ptr      (  0 )
{
  assert(singleton_ptr == 0); //< Allow only one instance
  singleton_ptr = this;
  this->open(addr);
}

void Dev::open(size_t addr)
{
  Matrix::dev = 0;
  if (addr != 0) {
    Matrix::dev = this;
    assert(dev_ptr == 0);
    imem_addr    = XDEV_HLS_DEVREG_ADDR_MEM_BASE;
    regs_addr    = XDEV_HLS_DEVREG_ADDR_REG_R_BASE;
    control_addr = XDEV_HLS_DEVREG_ADDR_AP_CTRL;
    return_addr  = XDEV_HLS_DEVREG_ADDR_AP_RETURN;
    // {:TODO:Map to device:}

    // Make sure we are root prior to attempting open of /dev/mem
    assert(geteuid() == 0);

    // Open memory map
    size_t page_addr;
    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t dev_size  = XDEV_HLS_DEVREG_ADDR_MEM_HIGH + 1;
    size_t need_size = (dev_size / page_size) * page_size;
    if (dev_size % page_size != 0) need_size += page_size; // round up

    mmap_fd=::open("/dev/mem",O_RDWR);
    if ( mmap_fd < 1 ) {
      perror(argv0);
      exit(-1);
    }
    page_addr   = addr & ~(page_size-1);
    page_offset = addr - page_addr;

    mmap_ptr=(int32_t*)mmap(NULL,need_size,PROT_READ|PROT_WRITE,MAP_SHARED,mmap_fd,page_addr);
    if ( mmap_ptr == MAP_FAILED ) {
      perror(argv0);
      exit(-1);
    }
    dev_ptr = mmap_ptr + page_offset;
    // Reset device status
    set_reg(COMMAND, RESET);
  }
  dev_addr    = addr;
}

//------------------------------------------------------------------------------
Dev::~Dev(void) //< Destructor
{
  this->close();
  singleton_ptr = 0;
}

//------------------------------------------------------------------------------
void Dev::close(void)
{
  if (::close(mmap_fd)<1) {
    perror(argv0);
    exit(-1);
  }
  mmap_ptr = 0;
  dev_ptr = 0;
}

//------------------------------------------------------------------------------
// Allocate memory from the device and set the matrix's address pointer if
// allocated. matrix::m_addr == ~0U means "not allocated"
bool Dev::alloc_mem(Matrix& m)
{
  if (!using_dev()) return false;
  size_t needed = (m.space())*sizeof(Data_t);
  const size_t maxsize = XDEV_HLS_DEVREG_DEPTH_MEM; // aka IMEM_SIZE
  if (needed > maxsize) {
    printf("ERROR: Requested memory allocation is too big\n");
    return false;
  }
  // If it already exists, warn and return
  if (m.m_addr != ~0U) {
    return true;
  }
  // Find the first available space
  size_t next_locn = 0;
  mem_used_t::iterator prev = mem_used.end();
  for (mem_used_t::iterator curr=mem_used.begin(); curr!=mem_used.end(); ++curr) {
    if (next_locn < curr->first && (curr->first - next_locn >= needed)) {
      mem_used.insert(make_pair(next_locn,&m));
      m.m_addr = next_locn;
      return true;
    }
    next_locn = curr->first + curr->second->space();
  }
  if (maxsize - next_locn < needed) {
    mem_used.insert(make_pair(next_locn,&m));
    m.m_addr = next_locn;
    return true;
  }
  return false;
}

//------------------------------------------------------------------------------
void Dev::free_mem(Matrix& m)
{
  mem_used_t::iterator used_it = mem_used.find(m.m_addr);
  if (used_it != mem_used.end()) {
    mem_used.erase(used_it);
    m.m_addr = ~0U;
  }
}

//------------------------------------------------------------------------------
// Find an unused register pair suitable for a matrix
bool Dev::alloc_mreg(Matrix& m)
{
  if (!using_dev()) return false;
  if (m.m_reg != UNUSED) return true; //< already allocated
  unsigned int r = 0;
  m.m_reg = UNUSED;
  for (;r < 10; r+=2) {
    if (regs_used & (3<<r)) continue;
    regs_used |= (3<<r);
    m.m_reg = Reg_t(r);
    break;
  }
  return (m.m_reg != UNUSED);
}

//------------------------------------------------------------------------------
void Dev::free_mreg(Matrix& m)
{
  if (m.m_reg != UNUSED) {
    unsigned int r = int(m.m_reg);
    regs_used &= ~(3<<r);
  }
  m.m_reg = UNUSED;
}

//------------------------------------------------------------------------------
Reg_t Dev::alloc_reg(size_t r)
{
  Reg_t result = UNUSED;
  if (using_dev()) {
    for (;r < 14; ++r) {
      if (regs_used & (1<<r)) continue;
      regs_used |= (1<<r);
      result = Reg_t(r);
      break;
    }
  }
  return result;
}

//------------------------------------------------------------------------------
void Dev::free_reg(Reg_t reg)
{
  if (reg != UNUSED) {
    unsigned int r = int(reg);
    regs_used &= ~(1<<r);
  }
}

//------------------------------------------------------------------------------
CmdState_t Dev::do_command(Operation_t opcode, Reg_t dest, Reg_t src1, Reg_t src2)
{
  unsigned cmnd = ((opcode&0xFF)<<24)|((dest&0xFF)<<16)|((src1&0xFF)<<8)|(src2&0xFF);
  set_reg(COMMAND, cmnd);
  CmdState_t status = START;
  set_reg(STATUS,status);
  // Wait for result
  do {
    usleep(1);
    status = CmdState_t(get_reg(STATUS));
  } while( (status & DONE) == 0);
  return status;
}

//------------------------------------------------------------------------------
int Dev::get_return(void) const
{
  return ((int)*(dev_ptr + XDEV_HLS_DEVREG_ADDR_AP_RETURN));
}

//------------------------------------------------------------------------------
int Dev::get_reg(Reg_t reg) const
{
  return ((int)*(dev_ptr + regs_addr + int(reg) * reg_width));
}

//------------------------------------------------------------------------------
void Dev::set_reg(Reg_t reg, int val)
{
  volatile int32_t* locn = (dev_ptr + regs_addr + int(reg) * reg_width);
  *locn = val;
}

//------------------------------------------------------------------------------
volatile int* Dev::mem_ptr(size_t addr) const
{
  assert((addr < XDEV_HLS_DEVREG_DEPTH_MEM) && ((addr & (sizeof(int)-1)) == 0));
  volatile int* locn = (dev_ptr + imem_addr + addr * reg_width);
  return locn;
}

//------------------------------------------------------------------------------
int Dev::get_mem(size_t addr) const
{
  assert((addr < XDEV_HLS_DEVREG_DEPTH_MEM) && ((addr & (sizeof(int)-1)) == 0));
  volatile int* locn = (dev_ptr + imem_addr + addr * reg_width);
  return *locn;
}

//------------------------------------------------------------------------------
void Dev::set_mem(size_t addr, int val)
{
  volatile int32_t* locn = (dev_ptr + regs_addr + addr * reg_width);
  *locn = val;
}

