#ifndef FLASHEM_H
#define FLASHEM_H
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

void wait_sec(double sec); //< sleeps specified # of seconds - restarts across signals
void led_delay(double sec); //< sets delays for led_display
void led_display(int value, int bits); //< blocking display of value[bits-1:0] bits with delay
void leds_flash(int *value, int bits);  //< simply flash on-off
void leds_fill(int *value, int bits);   //< fills from left (bits<0) or rigth
void leds_empty(int *value, int bits);  //< removes from left (bits<0) or right
void leds_cylon(int *value, int bits);  //< moves left-to-right-to-left NOT_YET_IMPLEMENTED
void leds_travel(int *value, int bits); //< travels left-to-right or right-to-left
void leds_climb(int *value, int bits);  //< builds up from left (bits<0) or right

#endif /*FLASHEM_H*/
