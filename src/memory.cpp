////////////////////////////////////////////////////////////////////////////////
// Implementation
//------------------------------------------------------------------------------

#include "memory.h"
#include "matrix.h"
#include <iostream>
#include <iomanip>
#include <random>
#include <cassert>
using namespace std;

//------------------------------------------------------------------------------
Memory::Memory(void) //< Constructor
  : xmem(new Data_t[XMEM_SIZE])
  , xmem_mirror(new Data_t[XMEM_SIZE])
  , imem(new Data_t[IMEM_SIZE])
  , imem_mirror(new Data_t[IMEM_SIZE])
{
  xfill();
  ifill();
}

//------------------------------------------------------------------------------
Memory::~Memory(void) { //< Destructor
  delete [] xmem;
  delete [] xmem_mirror;
  delete [] imem;
  delete [] imem_mirror;
}

Memory::Memory(const Memory& rhs)
  : xmem(nullptr)
  , xmem_mirror(nullptr)
  , imem(nullptr)
  , imem_mirror(nullptr)
{
  cerr << "FATAL: Memory copy constructor not allowed!" << endl;
}

Memory& Memory::operator=(const Memory& rhs) {
  cerr << "FATAL: Memory assignment not allowed!" << endl;
  return *this;
}


//------------------------------------------------------------------------------
Data_t Memory::xget(Addr_t addr) const {
  Data_t result = 0xF00DFACE; //< error value
  if (addr >= XMEM_SIZE) {
    cerr << "ERROR: Out of bounds access to xmem!" << endl;
  } else {
    result = xmem[addr];
  }
  return result;
}


//------------------------------------------------------------------------------
void Memory::xset(Addr_t addr, Data_t value) {
  if (addr >= XMEM_SIZE) {
    cerr << "ERROR: Out of bounds access to xmem!" << endl;
  } else {
    xmem[addr] = value;
  }
}


//------------------------------------------------------------------------------
Data_t Memory::iget(Addr_t addr) const {
  Data_t result = 0xF00DFACE; //< error value
  if (addr >= IMEM_SIZE) {
    cerr << "ERROR: Out of bounds access to imem!" << endl;
  } else {
    result = imem[addr];
  }
  return result;
}


//------------------------------------------------------------------------------
void Memory::iset(Addr_t addr, Data_t value) {
  if (addr >= IMEM_SIZE) {
    cerr << "ERROR: Out of bounds access to imem!" << endl;
  } else {
    imem[addr] = value;
  }
}


//------------------------------------------------------------------------------
void Memory::xfill(Data_t value) {
  for (size_t i=0; i!=XMEM_SIZE; ++i) {
    xmem[i] = value;
  }
}

//------------------------------------------------------------------------------
void Memory::ifill(Data_t value) {
  for (size_t i=0; i!=IMEM_SIZE; ++i) {
    imem[i] = value;
  }
}

//------------------------------------------------------------------------------
void Memory::dump8(Data_t* mem, Addr_t addr, Addr_t len) {
  cout << right << setw(5) << dec << addr << ": ";
  for (Addr_t i=addr; i!=addr+len; ++i) {
    cout << uppercase << hex << setw(8) << setfill('0') << mem[i] << " ";
  }
  cout << dec << "\n";
}

//------------------------------------------------------------------------------
void Memory::xdump(void) {
  for (Addr_t addr=0; addr<XMEM_SIZE; addr+=8) {
    Addr_t remaining = XMEM_SIZE-addr;
    dump8(xmem,addr,(remaining>=8)?8:remaining);
  }
}

//------------------------------------------------------------------------------
void Memory::idump(void) {
  for (int addr=0; addr<IMEM_SIZE; addr+=8) {
    Addr_t remaining = IMEM_SIZE-addr;
    dump8(imem,addr,(remaining>=8)?8:remaining);
  }
}

//------------------------------------------------------------------------------
void Memory::mirror(void) {
  for (size_t i=0; i!=XMEM_SIZE; ++i) {
    xmem_mirror[i] = xmem[i];
  }
  for (size_t i=0; i!=IMEM_SIZE; ++i) {
    imem_mirror[i] = imem[i];
  }
}

// Look for differences
int Memory::diffs8(Data_t* lhs, Data_t* rhs, Addr_t addr, Addr_t len) {
  int diffs = 0;
  for (Addr_t i=addr; i!=addr+len; ++i) {
    if (lhs[i] != rhs[i]) ++diffs;
  }
  return diffs;
}

//------------------------------------------------------------------------------
bool Memory::check(void) {
  cout << "Checking memory for differences..." << flush;
  bool first = true;
  int diffs = 0;
  for (size_t addr=0; addr!=XMEM_SIZE; addr+=8) {
    Addr_t remaining = XMEM_SIZE-addr;
    int mydiffs = diffs8(xmem,xmem_mirror,addr,(remaining>=8)?8:remaining);
    if (mydiffs==0) continue;
    if (first) {
      cout << "\nxmem differences:\n";
      first = false;
    }
    dump8(xmem,addr);
    diffs += mydiffs;
  }
  first = true;
  for (size_t addr=0; addr!=IMEM_SIZE; addr+=8) {
    Addr_t remaining = XMEM_SIZE-addr;
    int mydiffs = diffs8(imem,imem_mirror,addr,(remaining>=8)?8:remaining);
    if (mydiffs==0) continue;
    if (first) {
      cout << "\nimem differences:\n";
      first = false;
    }
    dump8(imem,addr);
    diffs += mydiffs;
  }
  if (diffs) cout << "\n";
  else       cout << "identical\n";
  cout << flush;
  return !diffs;
}

// The end
