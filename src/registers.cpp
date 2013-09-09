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
{
  reg = new Data_t[REGS];
  reg[BASE    ] = 0 ;
  reg[COMMAND ] = 0 ;
  reg[STATUS  ] = 0 ;
}

//------------------------------------------------------------------------------
Registers::~Registers(void) //< Destructor
{
  delete [] reg;
}

//------------------------------------------------------------------------------
void Registers::fill(Data_t value) {
  for (unsigned i=0; i!=GPREGS; ++i) reg[i] = value;
  reg[BASE   ] = value;
  reg[COMMAND]  = value;
  reg[STATUS ]  = value;
}

//------------------------------------------------------------------------------
void Registers::randomize(void) {
  // Unclear what to do since constraints are complex
  Command c;
  c.randomize();
  reg[COMMAND] = c.command;
}

//-----------------------------------------------------------------------------/-
string Registers::dump(void) {
  ostringstream sout;
  sout << "reg[BASE   ] =0x" << reg[BASE]    << "\n";
  sout << "reg[COMMAND] =0x" << reg[COMMAND] << "\n";
  sout << "reg[STATUS ] =0x" << reg[STATUS]  << "\n";
  for (unsigned i=0; i!=GPREGS; ++i) {
    sout << "reg["  << reg_name[i] << "] = ";
    sout << setw(8) << "0x" << hex << uppercase << setfill('0') << reg[i];
    sout << "\n";
  }
  return sout.str();
}
