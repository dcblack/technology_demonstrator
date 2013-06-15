#include "dev_hls.h"
#include "dev_util.h"
#include <iostream>
#include <iomanip>
using namespace std;

Addr_t reg[STATUS+1];
Data_t xmem[8*MAX_MATRIX_SIZE+100];

int main(int argc, char* argv[])
{

  Data_t  m0[] = {  0, 0, 0,  0, 0, 0 };
  Data_t  m1[] = {  1, 2, 3,  4, 5, 6 };
  Data_t  m2[] = { 10,20,30, 40,50,60 };
  Data_t  m3[] = { 11,22,33, 44,55,66 };
  Data_t* ma[] = { m0, m1, m2, m3 };

  Addr_t base = 100;
  size_t size = 0;

  // Initialize
  for (int matrix=0; matrix!=4; ++matrix) {
    base += size*sizeof(Data_t);
    reg[ARRAY+matrix] = base;
    reg[SHAPE+matrix] = Mshape(2,3);
    size = Msize(reg[SHAPE+matrix]);
    memcpy(&xmem[base],ma[matrix],size*sizeof(Data_t));
  }

  for (int i=0; i!=4; ++i) dump_array(Reg_t(i));

  Command_t cmnd_list[] = { NOP, LOAD, MCOPY, MADD, MSUB, STOP };

  for (int c=0; cmnd_list[c]!=STOP; ++c) {
    reg[COMMAND] = cmnd_list[c];
    cout << "Executing command: " << command_name[reg[COMMAND]] << endl;
    dev_hls(reg,xmem);
    cout << "Return status: " << status_name[reg[STATUS]] << endl;
    for (int i=0; i!=4; ++i) dump_array(Reg_t(i));
  }

  return 0;
}
