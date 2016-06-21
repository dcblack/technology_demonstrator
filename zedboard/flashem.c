// FILE: flashem.c
//
// DESCRIPTION:
//   Some utilities to flash the LED's of the zedboard in various patterns.
// NOTES:
//   Defining SILENT macro will remove all stdout I/O.
//   Defining TEST_FLASHEM macro will invoke unit-test (incompatible with silent)

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

#include "flashem.h"
#include <stdlib.h>
#include <stdio.h>
#define __USE_POSIX199309
#include <time.h>
#include "creport.h"
#include "ledsw.h"

static const char* MSGID = "/Doulos/flashem";
static int         led_debug = 0;

//------------------------------------------------------------------------------
#ifdef TEST_FLASHEM
void text_display(int value, int bits) //< for debugging
{
  int bit;
  for (int i=abs(bits)-1; i>=0; --i) {
    bit = value & (1<<i);
#ifdef SILENT
#else
    printf("%c",bit?'*':'-');
    fflush(stdout);
#endif
  }/*endfor*/
#ifdef SILENT
#else
  printf("\r");
  fflush(stdout);
#endif
}
#endif

//------------------------------------------------------------------------------
void wait_sec(double sec)
{
  struct timespec rqtp;
  rqtp.tv_sec = sec;
  sec = sec - rqtp.tv_sec;
  rqtp.tv_nsec = 1.0e9*sec;
  while (nanosleep(&rqtp,&rqtp)==-1);
}

//------------------------------------------------------------------------------
double m_led_delay = 0.25; //< pause time after displaying an led
void led_delay(double sec)
{
  m_led_delay = sec;
}

//------------------------------------------------------------------------------
void led_display(int value, int bits)
{
#ifdef TEST_FLASHEM
  if (led_debug) text_display(value,bits);
  else
#else
  write_leds(value);
#endif
  wait_sec(m_led_delay);
}

//------------------------------------------------------------------------------
void led_flash(int *value, int bits)
{
  REPORT_DEBUG("%s(0x%02x,%d):\n",__func__,*value,bits);
  if (!bits) bits = 8;
  int bit;
  for (int rep=0; rep!=2; ++rep) {
    for (int i=0; i!=abs(bits); ++i) {
      bit = 1<<i;
      *value ^= bit;
    }/*endfor*/
    led_display(*value,bits);
  }/*endfor*/
}

//------------------------------------------------------------------------------
void led_fill(int *value, int bits)
{
  REPORT_DEBUG("%s(0x%02x,%d):\n",__func__,*value,bits);
  if (!bits) bits = 8;
  int bit = 0;
  for (int i=abs(bits); i!=0; --i) {
    if (bits > 0) {
      for (int j=0; j!=i; ++j) {
        bit = 1<<j;
        *value = *value ^ bit;
        led_display(*value,bits);
        if (j<i-1) *value = *value ^ bit;
      }/*endfor*/
    } else {
      for (int j=abs(bits)-1; j>=abs(bits)-i; --j) {
        bit = 1<<j;
        *value = *value ^ bit;
        led_display(*value,bits);
        if (j>abs(bits)-i) *value = *value ^ bit;
      }/*endfor*/
    }/*endif*/
  }/*endfor*/
  led_display(*value,bits);
}

//------------------------------------------------------------------------------
void led_empty(int *value, int bits)
{
  REPORT_DEBUG("%s(0x%02x,%d):\n",__func__,*value,bits);
  if (!bits) bits = 8;
  int bit = 0;
  if (bits > 0) {
    for (int i=0; i!=bits; ++i) {
      bit = 1<<i;
      *value = *value ^ bit;
      led_display(*value,bits);
    }/*endfor*/
  } else {
    for (int i=abs(bits)-1; i>=0; --i) {
      bit = 1<<i;
      *value = *value ^ bit;
      led_display(*value,bits);
    }/*endfor*/
  }/*endif*/
}

//------------------------------------------------------------------------------
void led_cylon(int *value, int bits)
{
  NOT_YET_IMPLEMENTED;
}

//------------------------------------------------------------------------------
void led_travel(int *value, int bits)
{
  REPORT_DEBUG("%s(0x%02x,%d):\n",__func__,*value,bits);
  if (!bits) bits = 8;
  int bit;
  if (bits > 0) {
    for (int i=0; i!=abs(bits); ++i) {
      bit = 1<<i;
      *value ^= bit;
      led_display(*value,bits);
      *value ^= bit;
    }/*endfor*/
  } else {
    for (int i=abs(bits)-1; i>=0; --i) {
      bit = 1<<i;
      *value ^= bit;
      led_display(*value,bits);
      *value ^= bit;
    }/*endfor*/
  }/*endif*/
  led_display(*value,bits);
}

//------------------------------------------------------------------------------
void led_climb(int *value, int bits)
{
  REPORT_DEBUG("%s(0x%02x,%d):\n",__func__,*value,bits);
  if (!bits) bits = 8;
  int bit;
  if (bits > 0) {
    for (int i=0; i!=abs(bits); ++i) {
      bit = 1<<i;
      *value ^= bit;
      led_display(*value,bits);
    }/*endfor*/
  } else {
    for (int i=abs(bits)-1; i>=0; --i) {
      bit = 1<<i;
      *value ^= bit;
      led_display(*value,bits);
    }/*endfor*/
  }/*endif*/
}

//------------------------------------------------------------------------------
#ifdef TEST_FLASHEM
#warning Compiling unit-test main for flashem.c
int main(int argc, char* argv[])
{
  verbosity = DEBUG_LEVEL;
  led_debug = 1;
  int bits  = 8;
  int value;
  NEWLINE;
  led_delay(0.5);
  value = 0x00; leds_flash(&value,-bits);  NEWLINE;
  value = 0x00; leds_fill(&value,-bits);   NEWLINE;
  value = 0x00; leds_fill(&value,+bits);   NEWLINE;
                leds_fill(&value,+bits);   NEWLINE;

  value = 0xFF; leds_empty(&value,+bits);  NEWLINE;
  value = 0xFF; leds_empty(&value,-bits);  NEWLINE;
  value = 0x00; leds_empty(&value,-bits);  NEWLINE;

  value = 0x55; leds_travel(&value,+bits); NEWLINE;
  value = 0x00; leds_travel(&value,-bits); NEWLINE;
  value = 0xFF; leds_travel(&value,+bits); NEWLINE;

  value = 0x00; leds_climb(&value,+bits);  NEWLINE;
  value = 0xFF; leds_climb(&value,+bits);  NEWLINE;
  value = 0x00; leds_climb(&value,-bits);  NEWLINE;
}
#endif
