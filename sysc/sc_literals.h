#ifndef USER_LITERALS_H
#define USER_LITERALS_H

#include <systemc>

#ifndef SC_MAX_NBITS
#define SC_MAX_NBITS 17*30
#endif

// Make life easier by allowing user defined logic, int and time literals.
// Examples: 15.8_ns, 98DEADD0BE_BIGINT, 1_sec, 0ZZ_logic

#define X sc_lv(string(SC_MAX_NBITS,'X').c_str())
#define Z sc_lv(string(SC_MAX_NBITS,'Z').c_str())
auto operator "" _logic(const char* literal_string) ->  decltype(sc_dt::sc_lv_base(strlen(literal_string)-1));

sc_dt::sc_bigint<SC_MAX_NBITS> operator "" _int(const char* literal_string);
sc_core::sc_time      operator "" _sec(long double val);
sc_core::sc_time      operator "" _ms (long double val);
sc_core::sc_time      operator "" _us (long double val);
sc_core::sc_time      operator "" _ns (long double val);
sc_core::sc_time      operator "" _ps (long double val);
sc_core::sc_time      operator "" _fs (long double val);
sc_core::sc_time      operator "" _sec(unsigned long long val);
sc_core::sc_time      operator "" _ms (unsigned long long val);
sc_core::sc_time      operator "" _us (unsigned long long val);
sc_core::sc_time      operator "" _ns (unsigned long long val);
sc_core::sc_time      operator "" _ps (unsigned long long val);
sc_core::sc_time      operator "" _fs (unsigned long long val);

#endif /*USER_LITERALS_H*/
