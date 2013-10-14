#ifndef MEMORY_H
#define MEMORY_H

// DESCRIPTION
//   Models memory as two arrays, imem & xmem, along with mirrors to aid tracking.
//   current model uses only imem (internal memory). xmem is intended for use with
//   an external AXI based memory via an AXI master bus interface (future).

#include "dev_hls.h"
#include <string>
#ifdef CXX11
#include <random>
#endif

struct Memory
{
#ifdef CXX11
  static std::default_random_engine gen;
#endif
  // Attributes
  size_t  imem_size;
  size_t  xmem_size;
  Data_t* xmem;
  Data_t* xmem_mirror; //< shadow
  Data_t* imem;
  Data_t* imem_mirror; //< shadow

  // Methods
  Memory(int isize=IMEM_SIZE, int xsize=XMEM_SIZE);  //< Constructor
  ~Memory(void); //< Destructor
  Data_t xget (Addr_t addr) const;
  void   xset (Addr_t addr, Data_t data);
  Data_t iget (Addr_t addr) const;
  void   iset (Addr_t addr, Data_t data);
  void   xfill(Data_t value=0xDEADBEEF);
  void   ifill(Data_t value=0xDEADBEEF);
  void   xdump(void);
  void   idump(void);
  void   mirror(void);
  bool   check(Addr_t len=8);
private:
  Memory(const Memory& rhs);
  Memory& operator=(const Memory& rhs);
  std::string dump_line(Data_t* mem, Addr_t addr, Addr_t len=8,int highlight=0);
  int diffs_line(Data_t* lhs, Data_t* rhs, Addr_t addr, Addr_t len=8);
};

#endif /*MEMORY_H*/
