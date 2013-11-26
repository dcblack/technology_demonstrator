#ifndef COMMAND_H
#define COMMAND_H

// DESCRIPTION
//   This class mirrors the hardware design's registers. It is used both to set and send
//   desired commands, and compare the results against expectations.

#include "dev_hls.h"
#ifdef CXX11
#include <random>
#endif

struct Command
{
  static const int REGS = 16;
#ifdef CXX11
  static std::default_random_engine gen;
  static std::uniform_int_distribution<Operation_t> distr;
#endif

  // Attributes
  Data_t     command;
  Data_t     m_reg[REGS];
  Data_t     m_reg_mirror[REGS];
  bool       m_changed[REGS];
  Data_t     status;
  Data_t     expected;
  // Methods
  Command //< Constructor
    ( Operation_t   op   = NOP
    , unsigned char dest = 0xFF
    , unsigned char src1 = 0xFF
    , unsigned char src2 = 0xFF
    , CmdState_t  expect = START
    );
  Command(const Command& rhs); //< Copy constructor
  Command& operator=(const Command& rhs); //< Assignment
  bool operator==(const Command& rhs); //< Compare
  void set_cmd
    ( Operation_t   op   = NOP
    , unsigned char dest = 0xFF
    , unsigned char src1 = 0xFF
    , unsigned char src2 = 0xFF
    , CmdState_t  expect = START
    );
  void get_cmd
    ( Operation_t& op
    , unsigned char& dest
    , unsigned char& src1
    , unsigned char& src2
    ) const;
  Data_t& r(size_t i) { return m_reg[i]; }
  void mirror(void);
  void update(void);
  void diffregs(void);
  void clear_flags(void);
  void reset_reg(size_t i, Data_t  value);
  void set_reg(size_t i, Data_t  value);
  void get_reg(size_t i, Data_t& value, bool always=false);
  std::string regstr(bool all = false) const;
  std::string cmdstr(void) const;
  void randomize(void);
  std::string result(void);
  friend std::ostream& operator<<(std::ostream& os, const Command& rhs);
  //----------------------------------------------------------------------------
private:
  // Helpers
  void set_expected(CmdState_t expect=START);
  // Hidden attributes
  Operation_t m_op;
};//endclass Command

std::ostream& operator<<(std::ostream& os, const Command& rhs);

#endif /*COMMAND_H*/
