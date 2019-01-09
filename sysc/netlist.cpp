//BEGIN netlist.cpp (systemc)
///////////////////////////////////////////////////////////////////////////////
// $Info: Netlisting utility $

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
  char const * const MSGID = "/Doulos/example/netlist";
  // Embed file version information into object to help forensics
  char const * const RCSID = "(@)$Id: netlist.cpp  1.0 09/02/12 10:00 dcblack $";
  //                                  FILENAME     VER DATE     TIME  USERNAME

////////////////////////////////////////////////////////////////////////////>>/
// Traverse the entire object subhierarchy 
// below a given object 
void scan_hierarchy(sc_object* obj,string indent) { 
  std::vector<sc_object*> child = obj->get_child_objects(); 
  for ( unsigned int i=0; i != child.size(); i++ ) {
    sc_port_base*   iport = dynamic_cast<sc_port_base*>(child[i]);
    if (iport != 0) {
      for (size_t p=0; p!=iport->size(); ++p) {
        sc_channel*      chan = dynamic_cast<sc_channel*>(iport->get_interface(p));
        if (chan != 0) {
          REPORT_INFO( indent << ' ' << child[i]->basename() << ":" << child[i]->kind() << " -> " << chan->name());
        }//endif
        sc_prim_channel* prim = dynamic_cast<sc_prim_channel*>(iport->get_interface(p));
        if (prim != 0) {
          REPORT_INFO( indent << ' ' << child[i]->basename() << ":" << child[i]->kind() << " -> " << prim->name());
        }//endif
        sc_port_base* tiport = dynamic_cast<sc_port_base*>(iport->get_interface(p));
        if (tiport != 0) {
          REPORT_INFO( indent << ' ' << child[i]->basename() << ":" << child[i]->kind() << " -> " << tiport->name());
        }//endif
        sc_export_base* txport = dynamic_cast<sc_export_base*>(iport->get_interface(p));
        if (txport != 0) {
          REPORT_INFO( indent << ' ' << child[i]->basename() << ":" << child[i]->kind() << " -> " << txport->name());
        }//endif
      }//endfor
    } else {
      REPORT_INFO( indent << ' ' << child[i]->basename() << ":" << child[i]->kind() );
      if ( child[i] ) {
        scan_hierarchy(child[i],string("| ")+indent); 
      }//endif
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
        REPORT_INFO( tops[i]->name() << ":" << tops[i]->kind() );
        scan_hierarchy(tops[i],"+-");
      }//endif
    }//endfor
    REPORT_INFO("End netlist");
}//end netlist()

}//endnamespace util

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
