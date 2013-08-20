////////////////////////////////////////////////////////////////////////////////

#include "dev_hls.h"
#include <string>
#include <random>
#include <array>
#include <cassert>

struct Registers
{
  Registers(void); //< Constructor
  std::string dump(void);
  void randomize(void);
  void fill(Data_t value=0);
  Data_t reg_AXI_BASE;
  Data_t reg_COMMAND;
  Data_t reg_STATUS;
  Data_t reg[16];
  //----------------------------------------------------------------------------
private:
  Registers(const Registers& rhs) { assert(0); }
  Registers& operator=(const Registers& rhs) {  assert(0); return *this; }
  std::default_random_engine gen;
};//endclass Registers
