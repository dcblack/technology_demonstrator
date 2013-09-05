#include "dev_hls.h"
#include "dev_util.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <vector>
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

  // Initialize
  dev.fill(0xBADCAB1E);

  // Create/initialize some matrices for testing
  vector<Matrix*> matrix_list;
  matrix_list.push_back(new Matrix(3, 4, Matrix::RANDALL)); //0
  matrix_list.push_back(new Matrix(3, 4, Matrix::FILL0  )); //1
  matrix_list.push_back(new Matrix(3, 4, Matrix::PLUS1  )); //2
  matrix_list.push_back(new Matrix(3, 4, Matrix::RANDALL)); //3
  matrix_list.push_back(new Matrix(3, 4, Matrix::RANDALL)); //4
                                                       
  matrix_list.push_back(new Matrix(5, 5, Matrix::RANDALL)); //6
  matrix_list.push_back(new Matrix(5, 5, Matrix::RANDALL)); //7
  matrix_list.push_back(new Matrix(5, 5, Matrix::RANDALL)); //8
                                                       
  matrix_list.push_back(new Matrix(4, 3, Matrix::RANDALL)); //9
  matrix_list.push_back(new Matrix(4, 4, Matrix::RANDALL)); //10
  matrix_list.push_back(new Matrix(4, 1, Matrix::RANDALL)); //11
  matrix_list.push_back(new Matrix(1, 4, Matrix::RANDALL)); //12
  matrix_list.push_back(new Matrix(1, 1, Matrix::RANDALL)); //13
  matrix_list.push_back(new Matrix(3, 3, Matrix::RANDALL)); //14
  matrix_list.push_back(new Matrix(2, 2, Matrix::RANDALL)); //15

  Addr_t xmem_ptr = 2; // stay away from nullptr

  // Copy matrices into external memory [track base address in mm]
  vector<Addr_t> matrix_address;
  for (int i=0; i!=matrix_list.size(); ++i) {
    cout << matrix_list[i]->dump();
    matrix_address.push_back(xmem_ptr);
    matrix_list[i]->store(mem, xmem_ptr);
    xmem_ptr += matrix_list[i]->space();
  }

  cout << "Matrix xmem address mappings:" << "\n";
  for (int i=0; i!=matrix_address.size(); ++i) {
    cout
      << "  m" << dec << left << setw(2) << i << " -> "
      << "0x" << hex << right << setfill('0') << setw(4) << matrix_address[i] << " "
      << "(" << dec << matrix_address[i] << ")"
      << "\n";
  }
  cout << endl;

  // Setup commands for hardware
  vector<Command> cmd_list;
  Command cmd;

  cmd.set_cmd(NOP);
  // Setup internal matrix pointers
  cmd.set_reg(M0+1, 0*MAX_MATRIX_SIZE); //< internal address
  cmd.set_reg(M1+1, 1*MAX_MATRIX_SIZE); //< internal address
  cmd.set_reg(M2+1, 2*MAX_MATRIX_SIZE); //< internal address
  cmd.set_reg(M3+1, 3*MAX_MATRIX_SIZE); //< internal address
  cmd.set_reg(M4+1, 4*MAX_MATRIX_SIZE); //< internal address
  cmd.set_reg(M5+1, 5*MAX_MATRIX_SIZE); //< internal address
  cmd.set_reg(M6+1, 6*MAX_MATRIX_SIZE); //< internal address
  cmd_list.push_back(cmd);
  cmd.clear_flags();

  cmd.set_cmd(NOP);
  cmd_list.push_back(cmd);

  cmd.set_cmd(LOAD, M0, R12);
  cmd.set_reg(R12, matrix_address[0]);  //< external address
  cmd_list.push_back(cmd);
  cmd.clear_flags();

  cmd.set_cmd(LOAD, M1, R12);
  cmd.set_reg(R12, matrix_address[1]);  //< external address
  cmd_list.push_back(cmd);
  cmd.clear_flags();

  cmd.set_cmd(LOAD, M2, R12);
  cmd.set_reg(R12, matrix_address[2]);  //< external address
  cmd_list.push_back(cmd);
  cmd.clear_flags();

  cmd.set_cmd(LOAD, M3, R12);
  cmd.set_reg(R12, matrix_address[3]);  //< external address
  cmd_list.push_back(cmd);
  cmd.clear_flags();

  cmd.set_cmd(MADD, M0, M1, M2);
  cmd_list.push_back(cmd);

  cmd.set_cmd(STORE, M0, R13);
  cmd.set_reg(R13, matrix_address[3]);  //< external address
  cmd_list.push_back(cmd);
  cmd.clear_flags();

  cmd.set_cmd(MSUB, M0, M0, M3);
  cmd_list.push_back(cmd);

  cmd.set_cmd(RSUB, M0, M0, M3);
  cmd_list.push_back(cmd);

  cmd.set_cmd(MZERO, R11, M0);
  cmd_list.push_back(cmd);

  cmd.set_cmd(RSETX, R11, 0xAA, 0xAA);
  cmd_list.push_back(cmd);

  cmd.set_cmd(FILL, M0, R11);
  cmd_list.push_back(cmd);

  cmd.set_cmd(MZERO, R11, M0);
  cmd_list.push_back(cmd);

  cmd.set_cmd(RSETX, R11, 0x00, 0x00);
  cmd_list.push_back(cmd);

  cmd.set_cmd(FILL, M0, R11);
  cmd_list.push_back(cmd);

  cmd.set_cmd(MZERO, R11, M0);
  cmd_list.push_back(cmd);

  cmd.set_cmd(RESET);
  cmd_list.push_back(cmd);

  cmd.set_cmd(HALT);
  cmd_list.push_back(cmd);

  dev.reg_AXI_BASE = 0;
  for (int ci=0; ci!=cmd_list.size(); ++ci) {
    cout << "\nExecuting command: " << cmd_list[ci] << endl;
    dev.reg_COMMAND = cmd_list[ci].command;
    dev.reg_STATUS = cmd_list[ci].status;
    for (int i=0; i!=16; ++i) {
      cmd_list[ci].get_reg(i, dev.reg[i]); // update changed registers
      cmd_list[ci].reset_reg(i, dev.reg[i]); // update changed registers
    }
    mem.mirror();

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

    for (int i=0; i!=16; ++i) cmd_list[ci].set_reg(i, dev.reg[i]); // update changed registers
    cout << "Results: " << cmd_list[ci].regstr() << endl;
    cmd_list[ci].status = CmdState_t(dev.reg_STATUS);
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
