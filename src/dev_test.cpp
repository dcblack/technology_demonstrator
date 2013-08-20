#include "dev_hls.h"
#include "dev_util.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <random>
using namespace std;

#include "command.h"
#include "matrix.h"
#include "memory.h"
#include "registers.h"

unsigned long long errors = 0ULL;

void debug(const string& label) {
  cout << "DEBUGGING: " << label << endl;
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
  static std::default_random_engine         generator;
  static std::uniform_int_distribution<int> distribution(1, 4);
  auto   rand_1_to_4 = std::bind(distribution, generator);
  Registers dev;
  Memory    mem;

  Addr_t base  = 0;

  // Initialize
  dev.fill(0xBADCAB1E);

  // Create/initialize some matrices for testing
  vector<Matrix*> matrices;
  matrices.push_back(new Matrix(3, 4, Matrix::RANDALL)); //0
  matrices.push_back(new Matrix(3, 4, Matrix::RANDALL)); //1
  matrices.push_back(new Matrix(3, 4, Matrix::RANDALL)); //2
  matrices.push_back(new Matrix(3, 4, Matrix::RANDALL)); //3
  matrices.push_back(new Matrix(3, 4, Matrix::RANDALL)); //4
                                                       
  matrices.push_back(new Matrix(5, 5, Matrix::RANDALL)); //6
  matrices.push_back(new Matrix(5, 5, Matrix::RANDALL)); //7
  matrices.push_back(new Matrix(5, 5, Matrix::RANDALL)); //8
                                                       
  matrices.push_back(new Matrix(4, 3, Matrix::RANDALL)); //9
  matrices.push_back(new Matrix(4, 4, Matrix::RANDALL)); //10
  matrices.push_back(new Matrix(4, 1, Matrix::RANDALL)); //11
  matrices.push_back(new Matrix(1, 4, Matrix::RANDALL)); //12
  matrices.push_back(new Matrix(1, 1, Matrix::RANDALL)); //13
  matrices.push_back(new Matrix(3, 3, Matrix::RANDALL)); //14
  matrices.push_back(new Matrix(2, 2, Matrix::RANDALL)); //15

  Addr_t xmem_ptr = 2; // stay away from nullptr

  // Copy matrices into external memory [track base address in mm]
  vector<Addr_t> matrix_address;
  for (auto& matrix : matrices) {
    cout << matrix->dump();
    matrix_address.push_back(xmem_ptr);
    matrix->store(mem, xmem_ptr);
    xmem_ptr += matrix->space();
  }

  cout << "Matrix xmem address mappings:" << "\n";
  for (auto i=0; i!=matrix_address.size(); ++i) {
    cout << "  m" << i << " -> " << matrix_address[i] << "\n";
  }
  cout << endl;

  // Setup commands for hardware
  vector<Command> clist;
  Command c;

  c.set(NOP);
  // Setup internal matrix pointers
  c.set_r(M0+1, 0*MAX_MATRIX_SIZE); //< internal address
  c.set_r(M1+1, 1*MAX_MATRIX_SIZE); //< internal address
  c.set_r(M2+1, 2*MAX_MATRIX_SIZE); //< internal address
  c.set_r(M3+1, 3*MAX_MATRIX_SIZE); //< internal address
  c.set_r(M4+1, 4*MAX_MATRIX_SIZE); //< internal address
  c.set_r(M5+1, 5*MAX_MATRIX_SIZE); //< internal address
  c.set_r(M6+1, 6*MAX_MATRIX_SIZE); //< internal address
  clist.push_back(c);

  c.set(NOP);
  clist.push_back(c);

  c.set(LOAD, M0, R12);
  c.set_r(R12, matrix_address[0]);  //< external address
  clist.push_back(c);

  c.set(LOAD, M1, R12);
  c.set_r(R12, matrix_address[1]);  //< external address
  clist.push_back(c);

  c.set(LOAD, M2, R12);
  c.set_r(R12, matrix_address[2]);  //< external address
  clist.push_back(c);

  c.set(LOAD, M3, R12);
  c.set_r(R12, matrix_address[3]);  //< external address
  clist.push_back(c);

  c.set(MADD, M0, M1, M2);
  clist.push_back(c);

  c.set(STORE, M0, R13);
  c.set_r(R13, matrix_address[3]);  //< external address
  clist.push_back(c);

  c.set(MSUB, M0, M0, M3);
  clist.push_back(c);

  c.set(RSUB, M0, M0, M3);
  clist.push_back(c);

  c.set(MZERO, R11, M0);
  clist.push_back(c);

  c.set(RESET);
  clist.push_back(c);

  c.set(HALT);
  clist.push_back(c);

  dev.reg_AXI_BASE = 0;
  for (auto c : clist) {
    dev.reg_COMMAND = c.command;
    dev.reg_STATUS = c.status;
    for (int i=0; i!=16; ++i) c.get_r(i, dev.reg[i]); // update changed registers
    mem.mirror();

    cout << "\nExecuting command: " << c << endl;
    dev_hls
      ( &(dev.reg[0])
      , &(dev.reg[1])
      , &(dev.reg[2])
      , &(dev.reg[3])
      , &(dev.reg[4])
      , &(dev.reg[5])
      , &(dev.reg[6])
      , &(dev.reg[7])
      , &(dev.reg[8])
      , &(dev.reg[9])
      , &(dev.reg[10])
      , &(dev.reg[11])
      , &(dev.reg[12])
      , &(dev.reg[13])
      , &(dev.reg[14])
      , &(dev.reg[15])
      , &dev.reg_AXI_BASE
      , &dev.reg_COMMAND
      , &dev.reg_STATUS
      , mem.imem
      , mem.xmem
      );

    c.status = Status_t(dev.reg_STATUS);
    cout << "Return status: " << c.result() << endl;
    mem.check();
  }

  cout << "\n";
  if (errors == 0ULL) {
	  cout << "Test PASSED." << endl;
  } else {
	  cout << "Test FAILED with " << errors << " errors!" << endl;
  }
  return (errors == 0ULL) ? 0 : 1;
}
