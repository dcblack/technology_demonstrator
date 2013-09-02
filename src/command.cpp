////////////////////////////////////////////////////////////////////////////////
// Implementation
//------------------------------------------------------------------------------

#include "command.h"
#include "dev_util.h"
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <cassert>
using namespace std;

extern unsigned long int errors;

#ifdef CXX11
static std::default_random_engine                 Command::gen;
static std::uniform_int_distribution<Operation_t> Command::distr(NOP, HALT);
#endif

//----------------------------------------------------------------------------
Command::Command //< Constructor
( Operation_t op
, unsigned char dest
, unsigned char src1
, unsigned char src2
, CmdState_t    expect
)
: command((op<<3*8)|(dest<<2*8)|(src1<<1*8)|(src2<<0*8))
, status(IDLE)
, m_op(op)
{
  for (size_t i=0; i!=REGS; ++i) {
    m_r[i] = 0xF00DFACE;
    m_c[i] = false;
  }
  if (op != NOP) status = START;
  set_expected(expect);
}

//----------------------------------------------------------------------------
Command::Command(const Command& rhs) //< Copy constructor
: command(rhs.command)
, status (rhs.status)
, expected (rhs.expected)
, m_op(rhs.m_op)
{
  for (size_t i=0; i!=REGS; ++i) {
    m_r[i] = rhs.m_r[i];
    m_c[i] = rhs.m_c[i];
  }
}

//----------------------------------------------------------------------------
Command& Command::operator=(const Command& rhs) { //< Assignment
  command = rhs.command;
  status  = rhs.status;
  expected = rhs.expected;
  m_op = rhs.m_op;
  for (size_t i=0; i!=REGS; ++i) {
    m_r[i] = rhs.m_r[i];
    m_c[i] = rhs.m_c[i];
  }
  return *this;
}

//----------------------------------------------------------------------------
bool Command::operator==(const Command& rhs) { //< Compare
  if ( command != rhs.command 
    && status  != rhs.status ) {
    return false;
  }
  for (size_t i=0; i!=REGS; ++i) {
    if (m_r[i] != rhs.m_r[i]) {
      return false;
    }
  }
  return true;
}

// This is where we determine expected results
void Command::set_expected(CmdState_t expect) {
  if (expect != START)    expected = expect;
  else if (m_op == NOP)   expected = IDLE;
  else if (m_op == RESET) expected = IDLE;
  else if (m_op == HALT)  expected = HALTED;
  else if (m_op == LOAD) {
    expected = DONE;
    //{:TODO-CHECK-FOR-ERRORS:}
  }
  else if (m_op == STORE) {
    expected = DONE;
    //{:TODO-CHECK-FOR-ERRORS:}
  }
  else                    expected = DONE;
}

//----------------------------------------------------------------------------
void Command::set_cmd(Operation_t op, unsigned char dest, unsigned char src1, unsigned char src2, CmdState_t expect) {
  command = ((op&0xFF)<<(3*8))|((dest&0xFF)<<(2*8))|((src1&0xFF)<<(1*8))|((src2&0xFF)<<(0*8));
  m_op = op;
  if (op != NOP) status = START;
  set_expected(expect);
}

//----------------------------------------------------------------------------
void Command::get_cmd
( Operation_t& op
, unsigned char& dest
, unsigned char& src1
, unsigned char& src2
) const {
  op   = Operation_t((command>>(3*8))&0xFF);
  dest = (command>>(2*8))&0xFF;
  src1 = (command>>(1*8))&0xFF;
  src2 = (command>>(0*8))&0xFF;
}

//----------------------------------------------------------------------------
void Command::set_reg(size_t i, Data_t  value) {
  m_r[i]=value;
  m_c[i]=true;    
}

//----------------------------------------------------------------------------
void Command::get_reg(size_t i, Data_t& value, bool always) {
  assert(i<REGS);
  if (always||m_c[i]) {
    value=m_r[i];
    m_c[i]=false;
  } 
}

//----------------------------------------------------------------------------
void Command::clear(void) {
  for (size_t i=0; i!=REGS; ++i) {
    m_c[i] = false;
  }
}

//----------------------------------------------------------------------------
void Command::randomize(void) {
#ifdef CXX11
  Operation_t& op;
  unsigned char& dest;
  unsigned char& src1;
  unsigned char& src2;
  op = distr(gen);
  dest = gen();
  src1 = gen();
  src2 = gen();
  set_cmd(op,dest,src1,src2);
#else
  command = random();
#endif
}

//----------------------------------------------------------------------------
string Command::result(void) {
  string msg = (status <= GENERIC_ERROR)? cmd_state_name[status] : "UNKNOWN_ERROR";
  if (status != expected) {
    msg += " != expected ";
    msg += cmd_state_name[expected];
    msg += " (ERROR)";
    ++errors;
  }
  return msg;
}

//----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const Command& rhs) {
  Operation_t op;
  unsigned char dest, src1, src2;
  rhs.get_cmd(op, dest, src1, src2);
  os << operation_name[op];
  if ((rhs.command&0xFFFFFF) != 0xFFFFFF) os << dec << int(dest);
  if ((rhs.command&0xFFFF  ) != 0xFFFF  ) os << dec << ", " << int(src1);
  if ((rhs.command&0xFF    ) != 0xFF    ) os << dec << ", " << int(src2);
  os << dec << ";";
  for (size_t i=0; i!=rhs.REGS; ++i) {
    if (rhs.m_r[i] != 0xF00DFACE) {
      os << " R" << dec << i << "=0x" << hex << rhs.m_r[i] << ";";
    }
  }
  return os;
}

// The end
