//$File: dev_test.cpp $
//
// Description
//   Tests dev_hls
//
// Comments
//   Needs to be restructured to better reflect a reusable test methodology.

#include "dev_hls.h"
#include "dev_util.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <vector>
#include <map>
#ifdef CXX11
#include <functional>
#include <random>
#endif
using namespace std;

#include "command.h"
#include "matrix.h"
#include "memory.h"
#include "registers.h"

unsigned long long errors = 0ULL;

void debug(const string& label) {
  cout << "DEBUGGING: " << label << endl;
}

#include <cstdio>
static string gen_name(const string& base, size_t n)
{
  char buffer[20];
  int size;
  size = snprintf(buffer,20,"%llu",n);
  return base+buffer;
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
#ifdef CXX11
  static std::default_random_engine         generator;
  static std::uniform_int_distribution<int> distribution(1, 4);
  auto   rand_1_to_4 = std::bind(distribution, generator);
#endif
  Registers dev;
  Memory    mem;

  Addr_t base  = 0;
  Data_t retcode = 0;

  // Initialize
  dev.fill(0xBADCAB1E);

  // Create/initialize some matrices for testing
  vector<Matrix*> matrix_list;
  matrix_list.push_back(new Matrix(3, 4, Matrix::RANDALL)); //mtx0 3x4
  matrix_list.push_back(new Matrix(3, 4, Matrix::FILL0  )); //mtx1 3x4
  matrix_list.push_back(new Matrix(3, 4, Matrix::PLUS1  )); //mtx2 3x4
  matrix_list.push_back(new Matrix(3, 4, Matrix::RANDALL)); //mtx3 3x4
  matrix_list.push_back(new Matrix(3, 4, Matrix::RANDALL)); //mtx4 3x4
  matrix_list.push_back(new Matrix(4, 3, Matrix::RANDALL)); //mtx5 4x3
                                                       
  matrix_list.push_back(new Matrix(5, 5, Matrix::RANDALL)); //mtx6 5x5
  matrix_list.push_back(new Matrix(5, 5, Matrix::RANDALL)); //mtx7 5x5
  matrix_list.push_back(new Matrix(5, 5, Matrix::RANDALL)); //mtx8 5x5
                                                       
  matrix_list.push_back(new Matrix(3, 3, Matrix::RANDALL)); //mtx9  3x3
  matrix_list.push_back(new Matrix(4, 4, Matrix::RANDALL)); //mtx10 4x4
  matrix_list.push_back(new Matrix(4, 1, Matrix::RANDALL)); //mtx11 4x1
  matrix_list.push_back(new Matrix(1, 4, Matrix::RANDALL)); //mtx12 1x4
  matrix_list.push_back(new Matrix(1, 1, Matrix::RANDALL)); //mtx13 1x1
  matrix_list.push_back(new Matrix(3, 3, Matrix::RANDALL)); //mtx14 3x3
  matrix_list.push_back(new Matrix(2, 2, Matrix::RANDALL)); //mtx15 2x2

  // Copy matrices into device memory [track base address in mm]
  Addr_t mem_ptr = 0; // stay away from nullptr
  vector<Addr_t> matrix_address;
  for (int i=0; i!=matrix_list.size(); ++i) {
    matrix_list[i]->print();
    matrix_address.push_back(mem_ptr);
    matrix_list[i]->store(mem, mem_ptr);
    mem_ptr += matrix_list[i]->space();
  }

  cout << "Matrix memory address mappings:" << "\n";
  for (int i=0; i!=matrix_address.size(); ++i) {
    cout
      << "  mtx" << dec << setfill('0') << setw(2) << i << " -> "
      << "0x" << hex << setfill('0') << setw(4) << matrix_address[i] << " "
      << "(" << dec << matrix_address[i] << ")"
      << "\n";
  }
  cout << endl;

  // Setup results map
  map<size_t,Matrix*> expected_matrix;
  map<size_t,Data_t> expected_scalar;
  map<size_t,size_t> expected_reg;
  size_t curr, prev;
  string nm;
  // Setup commands for hardware
  vector<Command> cmd_list;
  Command cmd;

  cmd.set_cmd(NOP);
  cmd_list.push_back(cmd);

  cmd.set_cmd(NOP);
  // Setup internal matrix registers
  cmd.set_reg( M0  , matrix_list[0]->shape() );
  cmd.set_reg( M0+1, matrix_address[0]+1     );
  cmd.set_reg( M1,   matrix_list[1]->shape() );
  cmd.set_reg( M1+1, matrix_address[1]+1     );
  cmd.set_reg( M2,   matrix_list[2]->shape() );
  cmd.set_reg( M2+1, matrix_address[2]+1     );
  cmd.set_reg( M3,   matrix_list[3]->shape() );
  cmd.set_reg( M3+1, matrix_address[3]+1     );
  cmd_list.push_back(cmd);
  cmd.clear_flags();

  curr = cmd_list.size();
  nm = gen_name("e",curr);
  expected_matrix[curr] = new Matrix(matrix_list[0]->rows(), matrix_list[0]->cols(),nm);
  cmd.set_cmd(MADD, M0, M1, M2);
  cmd_list.push_back(cmd);
  expected_reg[curr] = M0;
  *expected_matrix[curr] = *matrix_list[1] + *matrix_list[2];
  expected_matrix[curr]->print("mtx1 + mtx2");
  prev = curr;

  curr = cmd_list.size();
  cmd.set_cmd(RSETX, R13, 0, 3);
  cmd_list.push_back(cmd);
  expected_reg[curr] = R13;
  expected_scalar[curr] = 3;

  curr = cmd_list.size();
  expected_matrix[curr] = new Matrix(matrix_list[2]->rows(), matrix_list[2]->cols(),nm);
  cmd.set_cmd(KMUL, M2, R13, M2);
  cmd_list.push_back(cmd);
  expected_reg[curr] = R13;
  *expected_matrix[curr] = *matrix_list[2] * 3;
  expected_matrix[curr]->print("mtx2 * 3");

  curr = cmd_list.size();
  nm = gen_name("e",curr);
  expected_matrix[curr] = new Matrix(matrix_list[0]->rows(), matrix_list[0]->cols(),nm);
  cmd.set_cmd(MSUB, M0, M0, M2);
  cmd_list.push_back(cmd);
  expected_reg[curr] = M0;
  *expected_matrix[curr] = *expected_matrix[prev] - *matrix_list[3];
  prev = curr;

  curr = cmd_list.size();
  nm = gen_name("e",curr);
  expected_matrix[curr] = new Matrix(matrix_list[0]->rows(), matrix_list[0]->cols(),nm);
  cmd.set_cmd(RSUB, M0, M0, M2);
  cmd_list.push_back(cmd);
  expected_reg[curr] = M0;
  *expected_matrix[curr] = *matrix_list[3] - *expected_matrix[curr];
  prev = curr;

  curr = cmd_list.size();
  nm = gen_name("e",curr);
  cmd.set_cmd(MZERO, R13, M0);
  cmd_list.push_back(cmd);
  expected_reg[curr] = R13;
  expected_scalar[curr] = matrix_list[0]->zeroes();
  prev = curr;

  curr = cmd_list.size();
  cmd.set_cmd(RSETX, R13, 0xAA, 0xAA);
  cmd_list.push_back(cmd);
  expected_reg[curr] = R13;
  expected_scalar[curr] = 0xFFFFAAAA;

  curr = cmd_list.size();
  nm = gen_name("e",curr);
  expected_matrix[curr] = new Matrix(matrix_list[0]->rows(), matrix_list[0]->cols(),nm);
  cmd.set_cmd(MFILL, M0, R13);
  cmd_list.push_back(cmd);
  expected_reg[curr] = M0;
  expected_matrix[curr]->fill(0xFFFFAAAA);

  curr = cmd_list.size();
  nm = gen_name("e",curr);
  cmd.set_cmd(MZERO, R13, M0);
  cmd_list.push_back(cmd);
  expected_reg[curr] = R13;
  expected_scalar[curr] = 0;

  curr = cmd_list.size();
  nm = gen_name("e",curr);
  cmd.set_cmd(RSETX, R13, 0x00, 0x00);
  cmd_list.push_back(cmd);
  expected_reg[curr] = R13;
  expected_scalar[curr] = 0;

  curr = cmd_list.size();
  nm = gen_name("e",curr);
  expected_matrix[curr] = new Matrix(matrix_list[0]->rows(), matrix_list[0]->cols(),nm);
  cmd.set_cmd(MFILL, M0, R13);
  cmd_list.push_back(cmd);
  expected_reg[curr] = M0;
  expected_matrix[curr]->fill(0);

  curr = cmd_list.size();
  nm = gen_name("e",curr);
  cmd.set_cmd(MZERO, R13, M0);
  cmd_list.push_back(cmd);
  expected_reg[curr] = R13;
  expected_scalar[curr] = matrix_list[0]->size();

  curr = cmd_list.size();
  nm = gen_name("e",curr);
  expected_matrix[curr] = new Matrix(matrix_list[9]->rows(), matrix_list[9]->cols(),nm);
  matrix_list[4]->fill_patt(Matrix::RAND100);
  matrix_list[5]->fill_patt(Matrix::PLUS1);
  cmd.set_reg( M3  , matrix_list[9]->shape() ); // dest mtx9 3x3
  cmd.set_reg( M3+1, matrix_address[9]+1     );
  cmd.set_reg( M4,   matrix_list[4]->shape() ); // src1 mtx4 3x4
  cmd.set_reg( M4+1, matrix_address[4]+1     );
  cmd.set_reg( M5,   matrix_list[5]->shape() ); // src2 mtx5 4x3
  cmd.set_reg( M5+1, matrix_address[5]+1     );
  cmd.set_cmd(MMUL, M3, M4, M5);
  cmd_list.push_back(cmd);
  expected_reg[curr] = M3;
  *expected_matrix[curr] = *matrix_list[4] * *matrix_list[5];
  matrix_list[4]->print("mtx4");
  matrix_list[5]->print("mtx5");
  expected_matrix[curr]->print("mtx4 x mtx5");
  prev = curr;

  cmd.set_cmd(RESET);
  cmd_list.push_back(cmd);

  cmd.set_cmd(HALT);
  cmd_list.push_back(cmd);

  dev.reg[BASE] = 0;
  for (int ci=0; ci!=cmd_list.size(); ++ci) {
    cout << "\nExecuting command: " << cmd_list[ci] << endl;
    dev.reg[COMMAND] = cmd_list[ci].command;
    dev.reg[STATUS] = cmd_list[ci].status;
    for (int i=0; i!=16; ++i) {
      cmd_list[ci].get_reg(i, dev.reg[i]); // update changed registers
      cmd_list[ci].reset_reg(i, dev.reg[i]); // update changed registers
    }
    mem.mirror();

    // Execute command
    retcode = dev_hls
      ( dev.reg
      , mem.imem
      );

    for (int i=0; i!=16; ++i) cmd_list[ci].set_reg(i, dev.reg[i]); // update changed registers
    if (expected_matrix.count(ci) == 1) { // check expected matrix
      // TODO: Get matrix referenced by expected_reg[ci] and compare against *expected_matrix[ci]
    }
    if (retcode != UNSUPPORTED_ERROR && expected_scalar.count(ci) == 1) { // check expected scalar
      if (cmd_list[ci].r(expected_reg[ci]) != expected_scalar[ci]) {
        cout << "ERROR: Unexpected scalar value from command " << ci << endl;
        ++errors;
      }//endif
    }//endif
    cout << "Results: " << cmd_list[ci].regstr() << endl;
    cmd_list[ci].status = retcode;
    cout << "Return status: " << cmd_list[ci].result() << endl;
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
