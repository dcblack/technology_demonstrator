#include "sc_literals.h"
#include <string>
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#if __cplusplus < 201103L
#error Requires C++11
#endif

// The following allows for easy bigint's: 123_456_789_ABCDEF0123456789_BIGINT
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

//TAF!
