#include "matrix.h"

#include "dev.h"
#include "int.h"
#include <iostream>
#include <iomanip>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
  size_t repetitions = 1e7;
#ifdef USING_HARDWARE
  size_t mtxdev_addr = 0x0; //< unassigned
#endif

  //----------------------------------------------------------------------------
  // Process command-line
  for (size_t i=1; i<argc; ++i) {
    string arg(argv[i]);
    if (i == 1 && is_u32(arg)) repetitions = stou32(arg);
#ifdef USING_HARDWARE
    if (i == 2 && is_u32(arg)) mtxdev_addr = stou32(arg);
#endif
  }
  cout << "Repeating " << commafy(repetitions) << endl;

#ifdef USING_HARDWARE
  // Setup hardware
  assert(mtxdev_addr != 0);
  Dev mtxdev(mtxdev_addr);
#endif

  // Create matrices
  Matrix lhs_matrix(3,4);
  Matrix rhs_matrix(4,3);
  Matrix result3_matrix(3,3);
  Matrix result4_matrix(4,4);
  lhs_matrix.randomize();

  // Do a bunch of operations (multiplication, addition, etc...)
  for (size_t rep=repetitions; rep!=0; --rep)
  {
    rhs_matrix.randomize();
    result3_matrix = lhs_matrix * rhs_matrix + 10;
    result4_matrix = rhs_matrix * lhs_matrix * 10;
  }

  cout << "Done" << endl;
  return 0;
}//end main()

////////////////////////////////////////////////////////////////////////////////
//EOF
