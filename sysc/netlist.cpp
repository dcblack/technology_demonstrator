//BEGIN netlist.cpp (systemc)
///////////////////////////////////////////////////////////////////////////////
// $Info: Netlisting utility $

///////////////////////////////////////////////////////////////////////////////
// $License: Apache 2.0 $
//
// This file is licensed under the Apache License, Version 2.0 (the "License").
// You may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "netlist.h"
#include "report.h"
#include <typeinfo>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
using namespace sc_core;
using namespace std;

namespace {
  // Declare string used as message identifier in SC_REPORT_* calls
  static char const * const MSGID = "/Doulos/example/netlist";
  // Embed file version information into object to help forensics
  static char const * const RCSID = "(@)$Id: netlist.cpp  1.0 09/02/12 10:00 dcblack $";
  //                                         FILENAME     VER DATE     TIME  USERNAME

////////////////////////////////////////////////////////////////////////////>>/
// Traverse the entire object subhierarchy 
// below a given object 
void scan_hierarchy(sc_object* obj,string indent) { 
  std::vector<sc_object*> children = obj->get_child_objects(); 
  for ( unsigned int i=0; i != children.size(); i++ ) {
    REPORT_INFO( indent << ' ' << children[i]->basename() << " {kind:" << children[i]->kind() << "}" );
    if ( children[i] ) {
      scan_hierarchy(children[i],string("| ")+indent); 
    }//endif
  }//endfor
}//end scan_hierarchy()

}//endnamespace

namespace util {

void netlist(void) {
    // Traverse the object hierarchy below each top-level object 
    std::vector<sc_object*> tops = sc_get_top_level_objects(); 
    REPORT_INFO("Netlist:");
    for ( unsigned int i=0; i != tops.size(); i++ ) {
      if ( tops[i] ) {
        REPORT_INFO( tops[i]->name() << " {kind:" << tops[i]->kind() << "}" );
        scan_hierarchy(tops[i],"+-");
      }//endif
    }//endfor
    REPORT_INFO("End netlist");
}//end netlist()

}//endnamespace util

//EOF
