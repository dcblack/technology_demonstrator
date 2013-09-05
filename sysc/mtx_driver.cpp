#include "mtx_driver.h"
#include "local_initiator.h"
#include "dev_util.h"
#include <memory>
#include <cstdio>
using namespace std;

int    Mtx::s_owner = 0;
int    Mtx::s_error = 0;
size_t Mtx::devbase = 0;
Local_initiator_module* Mtx::sys = nullptr;

int Mtx::open(const char* nam, int mode)
{
  if (s_owner != 0) return error(ERR_OWN); //< only one owner at a time
  s_owner = 1;
  //{:TODO:Reset?:}
  return (strcmp(nam,"/dev/mtx") == 0) ? FD : error(ERR_DEV);
}

ssize_t Mtx::read(int fd, int* buff, size_t size)
{
  sys->dev_read(devbase+STATUS,buff,1);
  if (size == 0) {
    int status = *buff & ~INTR_BIT;
    sys->dev_write(devbase+STATUS,&status,1);
  }
  return Mtx::OK;
}

ssize_t Mtx::write(int fd, int* buff, size_t size)
{
  sys->dev_write(devbase+COMMAND,buff,1);
  static int start = START;
  sys->dev_write(devbase+STATUS,&start,1);
  return Mtx::OK;
}

int Mtx::ioctl(int fd, Request_t request, mtx_t* ctrl)
{
  int status = error(ERR_UNKNOWN); //< until proven otherwise
  assert(sizeof(ctrl->reg)>=REGISTERS);
  switch (request) {
    case Mtx::READ:
      {
        sys->dev_read(devbase,ctrl->reg,REGISTERS);
        status = Mtx::OK;
        break;
      }
    case Mtx::WRITE:
      {
        sys->dev_write(devbase,ctrl->reg,REGISTERS);
        status = Mtx::OK;
        break;
      }
  }
  return status;
}

int Mtx::close(int fd)
{
  if (s_owner == 0) return error(ERR_CLOSE);
  s_owner = 0;
  return (fd == FD) ? Mtx::OK : error(ERR_FD);
}

void Mtx::dump(mtx_t* ctrl)
{
  for (int i=0; i!=REGISTERS; ++i) {
    printf("  R%02d=0x%08x",i,ctrl->reg[i]);
    if (i%8 == 7) printf("\n");
  }
  printf("\n");
}

int Mtx::command(Operation_t oper,int dest,int src1, int src2)
{
  return ( (oper & 0xFF) << 24 )
        |( (dest & 0xFF) << 16 )
        |( (src1 & 0xFF) <<  8 )
        |( (src2 & 0xFF)       )
        ;
}

void Mtx::disasm(int cmd)
{
  Operation_t oper;
  int dest,src1,src2;
  oper = Operation_t(( cmd >> 24 ) & 0xFF);
  dest =             ( cmd >> 16 ) & 0xFF;
  src1 =             ( cmd >>  8 ) & 0xFF;
  src2 =             ( cmd       ) & 0xFF;
  printf("NOTE: %s 0x%02x 0x%02x 0x%02x\n",operation_name[oper],dest,src1,src2);
}

const char* Mtx::status_cstr(int status)
{
  return cmd_state_name[status];
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
// The end!
