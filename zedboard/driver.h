#ifndef DRIVER_H
#define DRIVER_H

////////////////////////////////////////////////////////////////////////////////
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include "creport.h"

#define DEV_BASE       0 /*0x8000FF080100*/
#define DEV_STATUS_REG (DEV_BASE + 0*4)
#define DEV_COUNT1_REG (DEV_BASE + 1*4)
#define DEV_COUNT2_REG (DEV_BASE + 2*4)
#define DEV_COUNT3_REG (DEV_BASE + 3*4)
#define DEV_COUNT4_REG (DEV_BASE + 4*4)

typedef uint64_t      addr_t;
typedef unsigned char data_t;
typedef uint16_t      dlen_t;

// Return values: 0=SUCCESS; -1=FAILURE
void    dev_open(char* hostname, int hostport);
void    dev_close(void);
int     dev_put( addr_t  address , data_t* data_ptr , dlen_t  data_len );
int     dev_get( addr_t  address , data_t* data_ptr , dlen_t  data_len );
int     dev_put_debug ( addr_t  address , data_t* data_ptr , dlen_t  data_len );
int     dev_get_debug ( addr_t  address , data_t* data_ptr , dlen_t  data_len );
void    dev_soft_interrupt(const char* irq_message);
void    dev_wait(void); // wait for "interrupt"
debug_t dev_debug(long long int level); // if <0 then read else set level (default 0 => off)

#endif /*DRIVER_H*/
