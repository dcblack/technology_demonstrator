// FILE: ledsw.c

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

#include "ledsw.h"
#include <stdio.h>
#include <stdlib.h>

static const char* MSGID = "/Doulos/flashem";

int read_switch(int index)
{
  int   n;
  char  fname[32],str[8];
  FILE* fh;
  n = sprintf(fname,"/sys/class/gpio/gpio%d/value",76-index);
  //assert(n == strlen(fname));
  fh = fopen(fname,"r");
  fgets(str,8,fh);
  fclose(fh);
  return atoi(str);
}

int read_switches(void)
{
  int value = 0;
  for (int index=0; index!=8; ++index) {
    value <<= 1;
    value |= read_switch(index);
  }
  return value;
}

void write_led(int index, int bit)
{
  int   n;
  char  fname[32];
  FILE* fh;
  n = sprintf(fname,"/sys/class/gpio/gpio%d/value",61 + index);
  //assert(n == strlen(fname));
  fh = fopen(fname,"r");
  fprintf(fh,"%d\n",bit);
  fclose(fh);
}

void write_leds(int value)
{
  if ( value >= 0 ) {
    for (int index=0; index!=8; ++index) {
      write_led(index, value & 0x01);
      value >>= 1;
    }/*endfor*/
  }/*endif*/
}

#ifdef COMPLETE
void load_oled(void)
{
  system("insmod /lib/modules/`uname -r`/pmodoled-gpio.ko");
  if ( -c "/dev/zed_oled" ) {
    system("cat /root/logo.bin > /dev/zed_oled");
  }
}

void unload_oled(void)
{
  system("rmmod pmodoled-gpio /lib/modules/`uname -r`/pmodoled-gpio.ko");
}
#endif
