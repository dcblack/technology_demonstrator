// FILE: tlmx_packet.cpp

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

#include "tlmx_packet.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int32_t tlmx_packet::s_id = 0;

const char* tlmx_command_to_str(tlmx_command_t s) {
  switch(s) {
    case TLMX_IGNORE      : return "TLMX_IGNORE";
    case TLMX_READ        : return "TLMX_READ";
    case TLMX_WRITE       : return "TLMX_WRITE";
    case TLMX_DEBUG_READ  : return "TLMX_DEBUG_READ";
    case TLMX_DEBUG_WRITE : return "TLMX_DEBUG_WRITE";
    case TLMX_EXIT        : return "TLMX_EXIT";
    default               : return "*UNKNOWN_STATUS_ERROR*";
  }
}

const char* tlmx_status_to_str(tlmx_status_t s) {
  switch(s) {
    case TLMX_INCOMPLETE_RESPONSE    : return "TLMX_INCOMPLETE_RESPONSE";
    case TLMX_OK_RESPONSE            : return "TLMX_OK_RESPONSE";
    case TLMX_ADDRESS_ERROR_RESPONSE : return "TLMX_ADDRESS_ERROR_RESPONSE";
    case TLMX_GENERIC_ERROR_RESPONSE : return "TLMX_GENERIC_ERROR_RESPONSE";
    default                          : return "*UNKNOWN_STATUS_ERROR*";
  }
}

// Constructor
tlmx_packet::tlmx_packet
( uint8_t  command 
, uint64_t address 
, uint16_t data_len
, uint8_t* data_ptr
)
{
  this->id       = s_id++;
  this->command  = command;
  this->address  = address;
  this->status   = TLMX_INCOMPLETE_RESPONSE;
  this->data_len = data_len;
  //assert( payload_ptr->data_len == 0 || payload_ptr->data_ptr != nullptr );
  this->data_ptr = data_ptr;
}

// Destructor
tlmx_packet::~tlmx_packet(void)
{
}

bool tlmx_packet::operator==(const tlmx_packet& rhs) const
{
  if ( id       != rhs.id       ) return false;
  if ( command  != rhs.command  ) return false;
  if ( status   != rhs.status   ) return false;
  if ( address  != rhs.address  ) return false;
  if ( data_len != rhs.data_len ) return false;
  assert( data_len == 0 || data_ptr != nullptr );
  for (int i=0; i!=data_len; ++i) {
    if (data_ptr[i] != rhs.data_ptr[i]) return false;
  }
  return true;
}

tlmx_packet& tlmx_packet::operator=(const tlmx_packet& rhs)
{
}

int tlmx_packet::pack(char* buffer)
{
  memcpy( &buffer[ TLMX_ID_INDEX       ], &id      , TLMX_PACKET_ID_SIZE       );
  memcpy( &buffer[ TLMX_COMMAND_INDEX  ], &command , TLMX_PACKET_COMMAND_SIZE  );
  memcpy( &buffer[ TLMX_STATUS_INDEX   ], &status  , TLMX_PACKET_STATUS_SIZE   );
  memcpy( &buffer[ TLMX_ADDRESS_INDEX  ], &address , TLMX_PACKET_ADDRESS_SIZE  );
  memcpy( &buffer[ TLMX_DATA_LEN_INDEX ], &data_len, TLMX_PACKET_DATA_LEN_SIZE );
  assert( data_len == 0 || data_ptr != nullptr );
  if (data_len != 0) {
    memcpy( &buffer[TLMX_DATA_PTR_INDEX], data_ptr, data_len  );
  }
  return TLMX_DATA_PTR_INDEX + data_len;
}

int tlmx_packet::unpack(char* buffer)
{
  memcpy( &id       , &buffer[ TLMX_ID_INDEX       ], TLMX_PACKET_ID_SIZE        );
  memcpy( &command  , &buffer[ TLMX_COMMAND_INDEX  ], TLMX_PACKET_COMMAND_SIZE   );
  memcpy( &status   , &buffer[ TLMX_STATUS_INDEX   ], TLMX_PACKET_STATUS_SIZE    );
  memcpy( &address  , &buffer[ TLMX_ADDRESS_INDEX  ], TLMX_PACKET_ADDRESS_SIZE   );
  memcpy( &data_len , &buffer[ TLMX_DATA_LEN_INDEX ], TLMX_PACKET_DATA_LEN_SIZE  );
  assert( data_len == 0 || data_ptr != nullptr );
  if (data_len != 0) {
    memcpy(   data_ptr, &buffer[TLMX_DATA_PTR_INDEX], data_len  );
  }
  return TLMX_DATA_PTR_INDEX + data_len;
}

#include <sstream>
#include <iomanip>
std::string tlmx_packet::str(void) const
{
  using namespace std;
  ostringstream os;
  os << "tlmx_packet{"
     << this->id << ","
     << tlmx_command_to_str(tlmx_command_t(this->command)) << ","
     << tlmx_status_to_str(tlmx_status_t(this->status)) << ","
     << "addr=0x" << hex << this->address << ","
     << "dlen="   << dec << this->data_len
     ;
  assert( this->data_len == 0 || this->data_ptr != nullptr );
  if (this->data_len > 0) os << ",data=";
  for(int i=0;i!=this->data_len;++i) {
    if (i < 16) os << "0x" << hex << int(this->data_ptr[i]) << " ";
    else if (i == this->data_len-1) os << "0x" << hex << int(this->data_ptr[i]);
    else if (i == 16) os << "... ";
  }
  os << "}" << ends;
  return os.str();
}

void tlmx_packet::print(const char* message) const
{
  if (message != 0) printf("%s ",message);
  printf
  ( "tlmx_packet{%d,%s,%s,addr=0x%llx,dlen=%d"
  , this->id      
  , tlmx_command_to_str(tlmx_command_t(this->command))
  , tlmx_status_to_str(tlmx_status_t(this->status))
  , this->address 
  , this->data_len
  );
  assert( this->data_len == 0 || this->data_ptr != nullptr );
  if (this->data_len > 0) printf(",data=");
  for(int i=0;i!=this->data_len;++i) {
    if (i < 16) printf("%02x ",(int)this->data_ptr[i]);
    else if (i == this->data_len-1) printf("%02x",(int)this->data_ptr[i]);
    else if (i == 16) printf("... ");
  }
  printf("}\n");
}

int get_tlmx_data_len(char buffer[])
{
  int data_len;
  memcpy( &data_len, &buffer[ TLMX_DATA_LEN_INDEX ], TLMX_PACKET_DATA_LEN_SIZE  );
  return data_len;
}

#ifdef UNIT_TEST
int main(int argc, char* argv[])
{
  tlmx_packet*   p1 = 0;
  uint8_t        d1[TLMX_MAX_DATA_LEN];
  tlmx_packet*   p2 = 0;
  uint8_t        d2[TLMX_MAX_DATA_LEN];
  tlmx_command_t command;
  uint64_t       address;
  uint16_t       data_len;
  char           buffer[TLMX_MAX_BUFFER];
  int            test_len[] = { 1, 2, 8, 16, 0, TLMX_MAX_DATA_LEN, TLMX_MAX_DATA_LEN-1 };
  srandom(1);
  for (int i=0; i!=20; ++i) {
    command = tlmx_command_t(random()%(TLMX_EXIT+1));
    address = random();
    if (random()%4<3 && address & 0x80800000) address |= (random() << 32);
    if (i < sizeof(test_len)) data_len = test_len[i];
    else                      data_len = random()%TLMX_MAX_DATA_LEN+1;
    bzero(d1,TLMX_MAX_DATA_LEN);
    for (int i=0; i!= data_len; ++i) {
      d1[i] = random();
    }
    p1 = new tlmx_packet( command, address, data_len, &d1[0] );
    if (i < sizeof(test_len)) p1->status  = tlmx_status_t(random()%(TLMX_GENERIC_ERROR_RESPONSE+1));
    p1->print("p1");
    bzero(buffer,TLMX_MAX_BUFFER);
    p1->pack(buffer);
    memset(d2,0xFFu,TLMX_MAX_DATA_LEN);
    p2 = new tlmx_packet( TLMX_IGNORE, 0, 0, &d2[0] );
    p2->unpack(buffer);
    p2->print("p2");
    if (not(*p1 == *p2)) fprintf(stderr,"ERROR: tlmx_packet mismatch!\n");
    delete p1;
    delete p2;
  }
}
#endif
