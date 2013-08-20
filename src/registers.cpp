////////////////////////////////////////////////////////////////////////////////
// Implementation
//------------------------------------------------------------------------------

#include "registers.h"
#include "command.h"
#include "dev_util.h"
#include <iomanip>
#include <sstream>
using namespace std;

//------------------------------------------------------------------------------
Registers::Registers(void) //< Constructor
: reg_AXI_BASE ( 0 )
, reg_COMMAND  ( 0 )
, reg_STATUS   ( 0 )
{
  reg = new Data_t[REGS];
}

//------------------------------------------------------------------------------
Registers::~Registers(void) //< Destructor
{
  delete [] reg;
}

//------------------------------------------------------------------------------
void Registers::fill(Data_t value) {
  for (unsigned i=0; i!=REGS; ++i) reg[i] = value;
  reg_AXI_BASE = value;
  reg_COMMAND  = value;
  reg_STATUS   = value;
}

//------------------------------------------------------------------------------
void Registers::randomize(void) {
  // Unclear what to do since constraints are complex
  Command c;
  c.randomize();
  reg_COMMAND = c.command;
}

//-----------------------------------------------------------------------------/-
string Registers::dump(void) {
  ostringstream sout;
  sout << "reg_AXI_BASE=0x" << reg_AXI_BASE << "\n";
  sout << "reg_COMMAND =0x" << reg_COMMAND << "\n";
  sout << "reg_STATUS  =0x" << reg_STATUS  << "\n";
  for (unsigned i=0; i!=REGS; ++i) {
    sout << "reg_"  << reg_name[i] << "] = ";
    sout << setw(8) << "0x" << hex << uppercase << setfill('0') << reg[i];
    sout << "\n";
  }
  return sout.str();
}
