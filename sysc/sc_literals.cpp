// FILE: sc_literals.cpp
//
// DESCRIPTION
//
// Takes advantage of C++ 2011 standard user-defined literals to provide more
// natural syntax for time (e.g. 5.1_ns) and specialized data types such as
// big ints (e.g. 0b_0110_1101_int or 0x) and logic (e.g. 0_XX00_ZZ11_logic).
//
// Special note: Since these operators require a leading numeric character, it
// is bnecessary to have a leading 0 for many the logic type, which is automatically
// removed.

////////////////////////////////////////////////////////////////////////////////
// $License: Apache 2.0 $
//
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

#include "sc_literals.h"
#include <string>
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#if __cplusplus < 201103L
#error Requires C++11
#endif

// The following allows for easy bigint's: 1_2345_6789_ABCD_EF01_2345_6789_int
sc_bigint<SC_MAX_NBITS> operator "" _int(const char* literal_string)
{
  string tempstr(literal_string);
  for (size_t pos=tempstr.find('_'); pos!=string::npos; pos=tempstr.find('_')) tempstr.erase(pos,1);
  return sc_bigint<SC_MAX_NBITS>(tempstr.c_str());
}
auto operator "" _logic(const char* literal_string) -> decltype(sc_lv_base(strlen(literal_string)-1))
{
  string tempstr(literal_string);
  for (size_t pos=tempstr.find('_'); pos!=string::npos; pos=tempstr.find('_')) tempstr.erase(pos,1);
  if (tempstr[0] == '0') tempstr.erase(0,1);
  sc_lv_base result{tempstr.c_str(),int(strlen(literal_string)-1)};
  return result;
}
// The following makes time specification nicer: 15.3_ns or 0.1_us etc...
sc_time operator "" _sec (long double val) { return sc_time(val,SC_SEC); }
sc_time operator "" _ms  (long double val) { return sc_time(val,SC_MS );  }
sc_time operator "" _us  (long double val) { return sc_time(val,SC_US );  }
sc_time operator "" _ns  (long double val) { return sc_time(val,SC_NS );  }
sc_time operator "" _ps  (long double val) { return sc_time(val,SC_PS );  }
sc_time operator "" _fs  (long double val) { return sc_time(val,SC_FS );  }
sc_time operator "" _sec (unsigned long long val) { return sc_time(double(val),SC_SEC); }
sc_time operator "" _ms  (unsigned long long val) { return sc_time(double(val),SC_MS );  }
sc_time operator "" _us  (unsigned long long val) { return sc_time(double(val),SC_US );  }
sc_time operator "" _ns  (unsigned long long val) { return sc_time(double(val),SC_NS );  }
sc_time operator "" _ps  (unsigned long long val) { return sc_time(double(val),SC_PS );  }
sc_time operator "" _fs  (unsigned long long val) { return sc_time(double(val),SC_FS );  }

// The following makes kilo, mega, giga byte specifications easier
sc_dt::uint64 operator "" _KB (unsigned long long val) { return sc_dt::uint64(val*1024); }
sc_dt::uint64 operator "" _MB (unsigned long long val) { return sc_dt::uint64(val*1024*1024); }
sc_dt::uint64 operator "" _GB (unsigned long long val) { return sc_dt::uint64(val*1024*1024*1024); }

//TAF!
