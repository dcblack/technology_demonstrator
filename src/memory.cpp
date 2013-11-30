////////////////////////////////////////////////////////////////////////////////
// Implementation
//------------------------------------------------------------------------------

#include "memory.h"
#include "matrix.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <map>
#ifdef CXX11
#include <random>
#endif
#include <cassert>
using namespace std;

//------------------------------------------------------------------------------
Memory::Memory(int isize, int xsize) //< Constructor
  : xmem_size(xsize)
  , xmem(new Data_t[xmem_size])
  , xmem_mirror(new Data_t[xmem_size])
  , imem_size(isize)
  , imem(new Data_t[imem_size])
  , imem_mirror(new Data_t[imem_size])
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
  : xmem(0)
  , xmem_mirror(0)
  , imem(0)
  , imem_mirror(0)
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
  if (addr >= xmem_size) {
    cerr << "ERROR: Out of bounds access to xmem!" << endl;
  } else {
    result = xmem[addr];
  }
  return result;
}


//------------------------------------------------------------------------------
void Memory::xset(Addr_t addr, Data_t value) {
  if (addr >= xmem_size) {
    cerr << "ERROR: Out of bounds access to xmem!" << endl;
  } else {
    xmem[addr] = value;
  }
}


//------------------------------------------------------------------------------
Data_t Memory::iget(Addr_t addr) const {
  Data_t result = 0xF00DFACE; //< error value
  if (addr >= imem_size) {
    cerr << "ERROR: Out of bounds access to imem!" << endl;
  } else {
    result = imem[addr];
  }
  return result;
}


//------------------------------------------------------------------------------
void Memory::iset(Addr_t addr, Data_t value) {
  if (addr >= imem_size) {
    cerr << "ERROR: Out of bounds access to imem!" << endl;
  } else {
    imem[addr] = value;
  }
}


//------------------------------------------------------------------------------
void Memory::xfill(Data_t value) {
  for (size_t i=0; i!=xmem_size; ++i) {
    xmem[i] = value;
  }
}

//------------------------------------------------------------------------------
void Memory::ifill(Data_t value) {
  for (size_t i=0; i!=imem_size; ++i) {
    imem[i] = value;
  }
}

namespace {
  string getenv(const string& name) //< Test existance of environment variable
  {
    char * value = std::getenv(name.c_str());
    if (value != 0) {
      return string(value); // non-empty
    }//endif
    return "";
  }

  enum {none, normal , bold, red, green, blue, cyan, magenta, yellow, black };
  string color(int highlight)
  {
    string result = "";
    static bool supports_color = false;
    static const string ESX = "\033[";
    static const string SEP = ";";
    static map<string,string> ansi;
    if (ansi.empty()) {
      if ( (getenv("TERM") == "xterm") && (getenv("NOCOLOR") != "1") ) {
        supports_color = true;
      }
      ansi[ "none"    ] = "00";
      ansi[ "bold"    ] = "01";
      ansi[ "feint"   ] = "02";
      ansi[ "italic"  ] = "03";
      ansi[ "under"   ] = "04";
      ansi[ "wink"    ] = "05";
      ansi[ "blink"   ] = "06";
      ansi[ "reverse" ] = "07";
      ansi[ "hide"    ] = "08";
      ansi[ "cross"   ] = "08";
      ansi[ "nobold"  ] = "21";
      ansi[ "nofeint" ] = "22";
      ansi[ "noital"  ] = "23";
      ansi[ "nounder" ] = "24";
      ansi[ "noblink" ] = "25";
      ansi[ "reveal"  ] = "28";
      ansi[ "fg_blk"  ] = "30";
      ansi[ "fg_red"  ] = "31";
      ansi[ "fg_grn"  ] = "32";
      ansi[ "fg_ylw"  ] = "33";
      ansi[ "fg_blu"  ] = "34";
      ansi[ "fg_mag"  ] = "35";
      ansi[ "fg_cyn"  ] = "36";
      ansi[ "fg_wht"  ] = "37";
      ansi[ "bg_blk"  ] = "40";
      ansi[ "bg_red"  ] = "41";
      ansi[ "bg_grn"  ] = "42";
      ansi[ "bg_ylw"  ] = "43";
      ansi[ "bg_blu"  ] = "44";
      ansi[ "bg_mag"  ] = "45";
      ansi[ "bg_cyn"  ] = "46";
      ansi[ "bg_wht"  ] = "47";
    };
    if (supports_color) {
      switch (highlight) {
        case    none: break;
        case  normal: result += ESX + ansi["none"] + SEP + ansi["fg_blk"] + "m"; break;
        case    bold: result += ESX + ansi["bold"]   + "m";              ; break;
        case   green: result += ESX + ansi["fg_grn"] + "m";              ; break;
        case     red: result += ESX + ansi["fg_red"] + "m";              ; break;
        case    blue: result += ESX + ansi["fg_blu"] + "m";              ; break;
        case    cyan: result += ESX + ansi["fg_cyn"] + "m";              ; break;
        case magenta: result += ESX + ansi["fg_mag"] + "m";              ; break;
        case  yellow: result += ESX + ansi["fg_ylw"] + "m";              ; break;
        case   black: result += ESX + ansi["fg_blk"] + "m";              ; break;
      }
    }//endif
    return result;
  }
}//endnamespace anonymous

//------------------------------------------------------------------------------
string Memory::dump_line(string prefix, Data_t* mem, Addr_t addr, Addr_t len, int highlight) {
  ostringstream sout;
  if (highlight) sout << color(highlight);
  sout << prefix << ": " << right << setw(5) << dec << addr << ": ";
  for (Addr_t i=addr; i!=addr+len; ++i) {
    sout << uppercase << hex << setw(8) << setfill('0') << mem[i] << " ";
  }
  if (highlight) sout << color(normal);
  sout << dec << "\n";
  return sout.str();
}

//------------------------------------------------------------------------------
void Memory::mirror(void) {
  for (size_t i=0; i!=xmem_size; ++i) {
    xmem_mirror[i] = xmem[i];
  }
  for (size_t i=0; i!=imem_size; ++i) {
    imem_mirror[i] = imem[i];
  }
}

// Look for differences
int Memory::diffs_line(Data_t* lhs, Data_t* rhs, Addr_t addr, Addr_t len) {
  int diffs = 0;
  for (Addr_t i=addr; i!=addr+len; ++i) {
    if (lhs[i] != rhs[i]) ++diffs;
  }
  return diffs;
}

//------------------------------------------------------------------------------
bool Memory::check(Addr_t len) {
  cout << "Checking memory for differences..." << flush;
  bool first = true;
  int diffs = 0;
  for (size_t addr=0; addr<xmem_size; addr+=8) {
    Addr_t remaining = xmem_size-addr;
    int mydiffs = diffs_line(xmem,xmem_mirror,addr,(remaining>=8)?8:remaining);
    if (mydiffs==0) continue;
    if (first) {
      cout << "\nxmem differences:\n";
      first = false;
    }
    cout << dump_line( "BEFORE", xmem_mirror, addr, len, blue   );
    cout << dump_line( "ACTUAL", xmem,        addr, len, normal );
    diffs += mydiffs;
  }
  first = true;
  for (size_t addr=0; addr<imem_size; addr+=8) {
    Addr_t remaining = xmem_size-addr;
    int mydiffs = diffs_line(imem,imem_mirror,addr,(remaining>=8)?8:remaining);
    if (mydiffs==0) continue;
    if (first) {
      cout << "\nimem differences:\n";
      first = false;
    }
    cout << dump_line( "BEFORE", imem_mirror, addr, len, blue   );
    cout << dump_line( "ACTUAL", imem,        addr, len, normal );
    diffs += mydiffs;
  }
  if (diffs) cout << "\n";
  else       cout << "none\n";
  cout << flush;
  return !diffs;
}

// The end
