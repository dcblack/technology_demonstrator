////////////////////////////////////////////////////////////////////////////////
// Implementation
//------------------------------------------------------------------------------

#include "command.h"
#include "dev_util.h"
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
using namespace std;

extern unsigned long int errors;

#ifdef CXX11
static std::default_random_engine                 Command::gen;
static std::uniform_int_distribution<Operation_t> Command::distr(NOP, HALT);
#endif

//----------------------------------------------------------------------------
Command::Command //< Constructor
( Operation_t oper
, unsigned char dest
, unsigned char src1
, unsigned char src2
, CmdState_t    expect
)
: command((oper<<3*8)|(dest<<2*8)|(src1<<1*8)|(src2<<0*8))
, status(IDLE)
, m_op(oper)
{
  for (size_t i=0; i!=REGS; ++i) {
    m_reg[i] = 0xF00DFACE;
    m_changed[i] = false;
  }
  if (oper != NOP) status = START;
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
    m_reg[i] = rhs.m_reg[i];
    m_changed[i] = rhs.m_changed[i];
  }
}

//----------------------------------------------------------------------------
Command& Command::operator=(const Command& rhs) { //< Assignment
  command = rhs.command;
  status  = rhs.status;
  expected = rhs.expected;
  m_op = rhs.m_op;
  for (size_t i=0; i!=REGS; ++i) {
    m_reg[i] = rhs.m_reg[i];
    m_changed[i] = rhs.m_changed[i];
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
    if (m_reg[i] != rhs.m_reg[i]) {
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
void Command::set_cmd(Operation_t oper, unsigned char dest, unsigned char src1, unsigned char src2, CmdState_t expect) {
  command = ((oper&0xFF)<<(3*8))|((dest&0xFF)<<(2*8))|((src1&0xFF)<<(1*8))|((src2&0xFF)<<(0*8));
  m_op = oper;
  if (oper != NOP) status = START;
  set_expected(expect);
}

//----------------------------------------------------------------------------
void Command::get_cmd
( Operation_t& oper
, unsigned char& dest
, unsigned char& src1
, unsigned char& src2
) const {
  oper = Operation_t((command>>(3*8))&0xFF);
  dest = (command>>(2*8))&0xFF;
  src1 = (command>>(1*8))&0xFF;
  src2 = (command>>(0*8))&0xFF;
}

//----------------------------------------------------------------------------
void Command::mirror(void)
{
  std::memcpy(m_reg_mirror,m_reg,REGS*sizeof(m_reg[0]));
}

//----------------------------------------------------------------------------
void Command::update(void)
{
  std::memcpy(m_reg,m_reg_mirror,REGS*sizeof(m_reg[0]));
}

//----------------------------------------------------------------------------
void Command::diffregs(void)
{
  for (size_t i=0; i!=REGS; ++i) {
    if (m_reg[i]==m_reg_mirror[i]) continue;
    cout
      << " R" << dec << i
      << "=0x" << hex << m_reg[i] 
      << "vs mirror 0x" << hex << m_reg_mirror[i] 
      << ";";
  }
}

//----------------------------------------------------------------------------
void Command::clear_flags(void) {
  for (size_t i=0; i!=REGS; ++i) {
    m_changed[i]=false;
  }
}

//----------------------------------------------------------------------------
void Command::reset_reg(size_t i, Data_t  value) {
  m_reg[i]=value;
}

//----------------------------------------------------------------------------
void Command::set_reg(size_t i, Data_t  value) {
  if (m_reg[i] != value) {
    m_reg[i]=value;
    m_changed[i]=true;    
  }
}

//----------------------------------------------------------------------------
void Command::get_reg(size_t i, Data_t& value, bool always) {
  assert(i<REGS);
  if (always||m_changed[i]) {
    value=m_reg[i];
    m_changed[i]=false;
  } 
}

//----------------------------------------------------------------------------
void Command::randomize(void) {
#ifdef CXX11
  Operation_t& oper;
  unsigned char& dest;
  unsigned char& src1;
  unsigned char& src2;
  oper = distr(gen);
  dest = gen();
  src1 = gen();
  src2 = gen();
  set_cmd(oper,dest,src1,src2);
#else
  command = random();
#endif
}

//----------------------------------------------------------------------------
string Command::result(void) {
  string msg;
  msg = cmd_state_name(status);
  if (status != expected) {
    msg += " != expected ";
    msg += cmd_state_name(expected);
    msg += " (ERROR)";
    ++errors;
  }
  return msg;
}

std::string Command::regstr(bool all) const {
  std::ostringstream os;
  for (size_t i=0; i!=REGS; ++i) {
    if (m_changed[i] || all) {
      os << " R" << dec << i << "=0x" << hex << m_reg[i] << ";";
    }
  }
  return os.str();
}

std::string Command::cmdstr(void) const {
  std::ostringstream os;
  Operation_t oper;
  unsigned char dest, src1, src2;
  get_cmd(oper, dest, src1, src2);
  os << operation_name[oper];
  if ((command&0xFFFFFF) != 0xFFFFFF) os << dec << " "  << int(dest);
  if ((command&0xFFFF  ) != 0xFFFF  ) os << dec << ", " << int(src1);
  if ((command&0xFF    ) != 0xFF    ) os << dec << ", " << int(src2);
  os << dec << ";";
  return os.str();
}

//----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const Command& rhs) {
  os << rhs.cmdstr() << rhs.regstr();
  return os;
}

// The end
