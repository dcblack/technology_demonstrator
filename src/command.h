#ifndef COMMAND_H
#define COMMAND_H

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
  Data_t   command;
  Data_t   m_r[REGS];
  bool     m_c[REGS];
  Status_t status;
  Status_t expected;
  // Methods
  Command //< Constructor
    ( Operation_t   op   = NOP
    , unsigned char dest = 0xFF
    , unsigned char src1 = 0xFF
    , unsigned char src2 = 0xFF
    , Status_t    expect = START
    );
  Command(const Command& rhs); //< Copy constructor
  Command& operator=(const Command& rhs); //< Assignment
  bool operator==(const Command& rhs); //< Compare
  void set
    ( Operation_t   op   = NOP
    , unsigned char dest = 0xFF
    , unsigned char src1 = 0xFF
    , unsigned char src2 = 0xFF
    , Status_t expect=START
    );
  void get
    ( Operation_t& op
    , unsigned char& dest
    , unsigned char& src1
    , unsigned char& src2
    ) const;
  Data_t& r(size_t i) { return m_r[i]; }
  void set_r(size_t i, Data_t  value);
  void get_r(size_t i, Data_t& value, bool always=false);
  void clear(void);
  void randomize(void);
  std::string result(void);
  friend std::ostream& operator<<(std::ostream& os, const Command& rhs);
  //----------------------------------------------------------------------------
private:
  // Helpers
  void set_expected(Status_t expect=START);
  // Hidden attributes
  Operation_t m_op;
};//endclass Command

std::ostream& operator<<(std::ostream& os, const Command& rhs);

#endif /*COMMAND_H*/
