#ifndef LEDSW_H
#define LEDSW_H
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

int  read_sw(int index);
int  read_switches(void);
void write_led(int index, int value);
void write_leds(int value);
#ifdef COMPLETE
void load_oled(void);
void unload_oled(void);
#endif

#endif /*LEDSW_H*/
