#ifndef MEMORY_H
#define MEMORY_H

#include "dev_hls.h"
#include <string>
#include <random>

struct Memory
{
  // Attributes
  Data_t* xmem;
  Data_t* xmem_mirror; //< shadow
  Data_t* imem;
  Data_t* imem_mirror; //< shadow

  // Methods
  Memory(void);  //< Constructor
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
  bool   check(void);
private:
  std::default_random_engine gen;
  Memory(const Memory& rhs);
  Memory& operator=(const Memory& rhs);
  void dump8(Data_t* mem, Addr_t addr, Addr_t len=8);
  int diffs8(Data_t* lhs, Data_t* rhs, Addr_t addr, Addr_t len=8);
};

#endif /*MEMORY_H*/
