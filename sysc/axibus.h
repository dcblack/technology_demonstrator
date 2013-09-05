#ifndef AXIBUS_H
#define AXIBUS_H

#include <systemc>
#include "tlm_utils/simple_initiator_socket.h"
#include "memory_manager.h"
#include <dev.h>

class Dev_module;

struct Axibus
{
  // Methods
  Axibus
  ( tlm_utils::simple_initiator_socket<Dev_module>& axi_master_socket
  );
  void axi_read (Addr_t addr, Data_t data);
  void axi_write(Addr_t addr, Data_t data);
  void axi_read_burst (Addr_t addr, Data_t* data, size_t size);
  void axi_write_burst(Addr_t addr, Data_t* data, size_t size);
private:
  // Attributes
  tlm_utils::simple_initiator_socket<Dev_module>& axi_master_socket;
  memory_manager* m_memory_manager; // Memory manager
};

////////////////////////////////////////////////////////////////////////////////
// $License: Apache 2.0 $ <<<
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
//
/////////////////////////////////////////////////////////////////////////////>>>

#endif
