////////////////////////////////////////////////////////////////////////////////

#include "dev_hls.h"
#include <string>
#ifdef CXX11
#include <random>
#endif
#include <cassert>

struct Registers
{
  static const int REGS=19;
  static const int GPREGS=16;
  Registers(void); //< Constructor
  ~Registers(void); //< Destructor
  std::string dump(void);
  void randomize(void);
  void fill(Data_t value=0);
  Data_t* reg;
  //----------------------------------------------------------------------------
private:
  Registers(const Registers& rhs) { assert(0); }
  Registers& operator=(const Registers& rhs) {  assert(0); return *this; }
#ifdef CXX11
  std::default_random_engine gen;
#endif
};//endclass Registers
