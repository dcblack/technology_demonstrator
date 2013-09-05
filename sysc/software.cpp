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

  Mtx::devbase = dev_addr;

  // Open the device driver
  fd = Mtx::open("/dev/mtx",0);
  if (fd == -1) {
    printf("FATAL: Device open failed\n");
    return 1;
  }
  printf("INFO: Device driver opened\n");

  // Main application begins
  printf("INFO: Running main application\n");

  // A few C++ liberties in the interest of time...
  std::vector<int> cmd(1000,0); //< list of commands
  for (auto& v : cmd) v = Mtx::command(HALT); //< fill with HALT
  mtx_t mtx; //< local image of device registers
  int retval = 0;
  for (auto& v : mtx.reg) v = 0xBADF00d5; //< fill local copy of registers
  retval = Mtx::ioctl(fd,Mtx::READ,&mtx);
  if (retval != 0) {
    printf("FATAL: Device ioctl failed with %d\n", retval);
    if (retcode == 0) retcode = retval;
    return exitcode(retcode);
  }
  printf("INFO: Initial values:\n");
  Mtx::dump(&mtx);
  for (auto& v : mtx.reg) v = 0x2DECADE5; //< fill local copy of registers
  retval = Mtx::ioctl(fd,Mtx::WRITE,&mtx);
  if (retval != 0) {
    printf("FATAL: Device ioctl failed with %d\n", retval);
    if (retcode == 0) retcode = retval;
    return exitcode(retcode);
  }

  int i=0;
  cmd[i++] = Mtx::command(NOP);
  cmd[i++] = Mtx::command(RESET);

  // M0 = fill(0x0000BEAD)
  cmd[i++] = Mtx::command(RSETX, 8,0xBE,0xAD);
  cmd[i++] = Mtx::command(RSETH, 0,0x00,0x03);
  cmd[i++] = Mtx::command(RSETL, 0,0x00,0x04);
  cmd[i++] = Mtx::command(RSETH, 1,0x00,0x00);
  cmd[i++] = Mtx::command(RSETL, 1,0x00,0x00);
  cmd[i++] = Mtx::command(FILL,0,8);

  // M1 = fill(0x1)
  cmd[i++] = Mtx::command(RSETX, 8,0x00,0x01);
  cmd[i++] = Mtx::command(RSETH, 2,0x00,0x03);
  cmd[i++] = Mtx::command(RSETL, 2,0x00,0x04);
  cmd[i++] = Mtx::command(RSETH, 3,0x00,0x00);
  cmd[i++] = Mtx::command(RSETL, 3,0x00,0x00);
  cmd[i++] = Mtx::command(FILL,2,8);

  // M2 = M0 + M1
  cmd[i++] = Mtx::command(RSETH, 4,0x00,0x03);
  cmd[i++] = Mtx::command(RSETL, 5,0x00,0x04);
  cmd[i++] = Mtx::command(MADD,4,0,2);

  // Store M0
  cmd[i++] = Mtx::command(RSETH, 9,mem_addr>>24,mem_addr>>16);
  cmd[i++] = Mtx::command(RSETL, 9,mem_addr>>8,mem_addr);
  cmd[i++] = Mtx::command(STORE, 9,4);

  cmd[i++] = Mtx::command(NOP);
  cmd[i++] = Mtx::command(HALT);
  
  int last=i;

  printf("INFO: malloc(1MB)\n"); fflush(stdout);
  int* xmem = (int*) sys->malloc(1*MB);
  sys->cycles(50); //< simulate passing of time

  printf("INFO: Execute %d commands\n",last); fflush(stdout);
  int command, status;
  Mtx::read(fd,&status,0); //< 0 clears any outstanding interrupt
  for (i=0; i!= last; ++i) {
    command = cmd[i];
    Mtx::disasm(command); fflush(stdout);
    Mtx::write(fd,&command,sizeof(command));
    sys->wait_dev();
    Mtx::read(fd,&status,0); //< 0 clears interrupt
    printf("INFO: Status: %s\n",Mtx::status_cstr(status)); fflush(stdout);
    retval = Mtx::ioctl(fd,Mtx::READ,&mtx);
    if (retval != 0) {
      printf("FATAL: Device ioctl failed with %d\n", retval);
      if (retcode == 0) retcode = retval;
      return exitcode(retcode);
    }
    Mtx::dump(&mtx);
    printf("\n");
  }

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
  int closed = Mtx::close(fd);
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
