#include "matrix.h"

#include "int.h"
#include <iostream>
#include <iomanip>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
  size_t reps = 1e7;

  //----------------------------------------------------------------------------
  // Process command-line
  for (size_t i=1; i<argc; ++i) {
    string arg(argv[i]);
    if (is_number(arg)) reps = stoi10(arg);
  }
  cout << "Repeating " << commafy(reps) << endl;

  // Create matrices
  Matrix lhs_matrix(3,4);
  Matrix rhs_matrix(4,3);
  Matrix result3_matrix(3,3);
  Matrix result4_matrix(4,4);
  lhs_matrix.randomize();

  // Do a bunch of operations (multiplication, addition, etc...)
  for (size_t rep=reps; rep!=0; --rep)
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