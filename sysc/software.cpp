#include "software.h"
#include "mtx_driver.h"
#include "matrix.h"
#include "local_initiator.h"
#include <vector>

////////////////////////////////////////////////////////////////////////////////
int Software::sw_main(void)
{
  const size_t KB = 1024;
  const size_t MB = 1024*KB;
  const size_t GB = 1024*MB;
  const size_t dev_addr = 0;
  const size_t mem_addr = 1*GB;
  int retcode = 0; // Overall status
  int error_count = 0;

  Mtx::devbase = dev_addr;

  // Open the device driver
  fd = mtx_open("/dev/mtx",0);
  if (fd == -1) {
    printf("FATAL: Device open failed\n");
    return 1;
  }
  printf("INFO: Device driver opened\n");

  // Main application begins
  printf("INFO: Running main application\n");

  printf("INFO: malloc(1MB)\n"); fflush(stdout);
  int* xmem = (int*) sys->malloc(1*MB);
  sys->cycles(50); //< simulate passing of time

  // A few C++ liberties in the interest of time...
  std::vector<int> cmd(1000,0); //< list of commands
  for (auto& v : cmd) v = Mtx::command(HALT); //< fill with HALT
  std::vector<int> exp(1000,0); //< list of expected status
  for (auto& v : exp) v = DONE; //< fill with DONE
  mtx_t mtx; //< local image of device registers
  int retval = 0;
  for (auto& v : mtx.reg) v = 0xBADF00d5; //< fill local copy of registers
  retval = mtx_ioctl(fd,Mtx::READ,&mtx);
  if (retval != 0) {
    printf("FATAL: Device ioctl failed with %d\n", retval);
    if (retcode == 0) retcode = retval;
    return exitcode(retcode);
  }
  printf("INFO: Initial values:\n");
  Mtx::dump_registers(&mtx);
  for (auto& v : mtx.reg) v = 0x2DECADE5; //< fill local copy of registers
  retval = mtx_ioctl(fd,Mtx::WRITE,&mtx);
  if (retval != 0) {
    printf("FATAL: Device ioctl failed with %d\n", retval);
    if (retcode == 0) retcode = retval;
    return exitcode(retcode);
  }

  int i=0;
  cmd[i++] = Mtx::command(NOP);
  cmd[i++] = Mtx::command(RESET);
  exp[i-1] = IDLE;

// Simplify test writing
#define DoCmnd1(op)                 cmd[i++] = Mtx::command(op)
#define DoCmnd2(op,dest)            cmd[i++] = Mtx::command(op, dest)
#define DoCmnd3(op,dest,src1)       cmd[i++] = Mtx::command(op, dest, src1)
#define DoCmnd4(op,dest,src1,src2)  cmd[i++] = Mtx::command(op, dest, src1, src2)
#define LoadI16(op,dest,value)      cmd[i++] = Mtx::command(op, dest, ((value>>8)&0xFF), (value&0xFF))
#define LoadI32(dest,value)         LoadI16(RSETH,dest,(value>>16)); LoadI16(RSETL,dest,value);
#define SetRegX(dest,value)         LoadI16(RSETX,dest,value)
#define MatrixI(dest,rows,cols,ptr) LoadI32(dest,Mshape(rows,cols)); LoadI32(dest+1,ptr)

  // M0 = fill(0x0000BEAD)
  Matrix  m0(3,4);
  MatrixI(M0,3,4,0x0000);
  SetRegX(R8,0xBEAD);
  DoCmnd3(FILL,M0,R8);

  // m1 = fill(0x1)
  Matrix  m1(3,4);
  MatrixI(M1,3,4,0x0010);
  SetRegX(R8,1);
  DoCmnd3(FILL,M2,R8);

  // m2 = M0 + M1
  Matrix  m2(3,4);
  MatrixI(M2,3,4,0x0020);
  DoCmnd4(MADD,M3,M0,M1);

  // Store M0
  LoadI32(R9,mem_addr);
  DoCmnd3(STORE, R9,M2);

  DoCmnd1(NOP);
  DoCmnd1(HALT);
  
  int last=i;
  printf("INFO: Preparing to test %d commands\n",last);

  printf("INFO: Execute %d commands\n",last); fflush(stdout);
  int command, status;
  Mtx::read(fd,&status,0); //< 0 clears any outstanding interrupt
  for (i=0; i!= last; ++i) {
    command = cmd[i];
    Mtx::display_command(command); fflush(stdout);
    // Send command
    mtx_write(fd,&command,sizeof(command));
    // Wait for results
    sys->wait_dev();
    mtx_read(fd,&status,0); //< 0 clears interrupt
    // Check results
    if (status == exp[i]) {
      printf("INFO: Status: %s\n",Mtx::status_cstr(status)); fflush(stdout);
    } else {
      printf("ERROR: Status: %s\n",Mtx::status_cstr(status)); fflush(stdout);
      ++error_count;
    }//endif
    // Read registers
    retval = Mtx::ioctl(fd,Mtx::READ,&mtx);
    if (retval != 0) {
      printf("FATAL: Device ioctl failed with %d\n", retval);
      if (retcode == 0) retcode = retval;
      return exitcode(retcode);
    }
    Mtx::dump_registers(&mtx);
    printf("\n");
  }

  if (error_count == 0) {
    printf("INFO: Test PASSED.\n");
  } else {
    printf("INFO: Test FAILED with %d errors\n",error_count);
    if (retcode == 0) retcode = 1;
  }//endif

  // exitcode closes device
  return exitcode(retcode);
}//end Software::sw_main

////////////////////////////////////////////////////////////////////////////////
void Software::interrupt_handler(void)
{
  for(;;) {
    sys->wait_irq();
    printf("INFO: Software interrupt handler invoked\n");
    //{:TODO:?:}
    // Exit
    sys->clear_irq();
    sys->resume_os();
  }//endforever
}//end Software::interrupt_handler

////////////////////////////////////////////////////////////////////////////////
int Software::exitcode(int retcode)
{
  int closed = mtx_close(fd);
  if (closed != 0) {
    printf("FATAL: Device open failed with %d\n", closed);
    if (retcode == 0) retcode = closed;
  }
  printf("INFO: Device driver closed\n");
  return retcode;
}//end Software::exitcode

////////////////////////////////////////////////////////////////////////////////
// $LICENSE: Apache 2.0 $ <<<
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  L<http://www.apache.org/licenses/LICENSE-2.0>
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
/////////////////////////////////////////////////////////////////////////////>>>
// The end!
