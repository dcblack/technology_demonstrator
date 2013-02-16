// FILE: tlmx_packet.c

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

#include "tlmx_packet.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int32_t s_id = 0;

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

/* Constructor */
tlmx_packet* new_tlmx_packet
( uint8_t  command 
, uint64_t address 
, uint16_t data_len
, uint8_t* data_ptr
)
{
  tlmx_packet* payload_ptr;
  payload_ptr = malloc(sizeof *payload_ptr);
  payload_ptr->id       = s_id++;
  payload_ptr->command  = command;
  payload_ptr->address  = address;
  payload_ptr->status   = TLMX_INCOMPLETE_RESPONSE;
  payload_ptr->data_len = data_len;
  /*assert( payload_ptr->data_len == 0 || payload_ptr->data_ptr != NULL ); */
  payload_ptr->data_ptr = data_ptr;
  return payload_ptr;
}

void init_tlmx_packet(tlmx_packet* payload_ptr)
{
  assert(payload_ptr != NULL);
  payload_ptr->command  = TLMX_IGNORE;
  payload_ptr->address  = 0;
  payload_ptr->status   = TLMX_INCOMPLETE_RESPONSE;
  payload_ptr->data_len = 0;
  payload_ptr->data_ptr = NULL;
}

tlmx_packet* clone_tlmx_packet(tlmx_packet* rhs)
{
  tlmx_packet* result;
  assert(rhs != NULL);
  result = malloc(sizeof *rhs);
  result->id       = rhs->id;
  result->command  = rhs->command;
  result->address  = rhs->address;
  result->status   = rhs->status;
  result->data_len = rhs->data_len;
  result->data_ptr = rhs->data_ptr;
  return result;
}

/* Destructor */
void delete_tlmx_packet(tlmx_packet* payload_ptr)
{
  free(payload_ptr);
  payload_ptr = NULL;
}

int compare_tlmx_packet(tlmx_packet* lhs, tlmx_packet* rhs) {
  assert(lhs != NULL && rhs != NULL);
  if ( lhs->id != rhs->id       ) return 0;
  if ( lhs->id != rhs->command  ) return 0;
  if ( lhs->id != rhs->status   ) return 0;
  if ( lhs->id != rhs->address  ) return 0;
  if ( lhs->id != rhs->data_len ) return 0;
  assert( lhs->data_len == 0 || lhs->data_ptr != NULL );
  for (int i=0; i!=lhs->data_len; ++i) {
    if (lhs->data_ptr[i] != rhs->data_ptr[i]) return 0;
  }
  return 1;
}

int pack_tlmx(char buffer[], tlmx_packet* tlmx_payload_ptr)
{
  assert( tlmx_payload_ptr != NULL);
  memcpy( &buffer[ TLMX_ID_INDEX       ], &(tlmx_payload_ptr->id       ), TLMX_PACKET_ID_SIZE       );
  memcpy( &buffer[ TLMX_COMMAND_INDEX  ], &(tlmx_payload_ptr->command  ), TLMX_PACKET_COMMAND_SIZE  );
  memcpy( &buffer[ TLMX_STATUS_INDEX   ], &(tlmx_payload_ptr->status   ), TLMX_PACKET_STATUS_SIZE   );
  memcpy( &buffer[ TLMX_ADDRESS_INDEX  ], &(tlmx_payload_ptr->address  ), TLMX_PACKET_ADDRESS_SIZE  );
  memcpy( &buffer[ TLMX_DATA_LEN_INDEX ], &(tlmx_payload_ptr->data_len ), TLMX_PACKET_DATA_LEN_SIZE );
  assert( tlmx_payload_ptr->data_len == 0 || tlmx_payload_ptr->data_ptr != NULL );
  if (tlmx_payload_ptr->data_len != 0) {
    memcpy( &buffer[TLMX_DATA_PTR_INDEX], tlmx_payload_ptr->data_ptr, tlmx_payload_ptr->data_len  );
  }
  return TLMX_DATA_PTR_INDEX + tlmx_payload_ptr->data_len;
}

int unpack_tlmx(tlmx_packet* tlmx_payload_ptr, char buffer[])
{
  assert( tlmx_payload_ptr != NULL);
  memcpy( &(tlmx_payload_ptr->id       ), &buffer[ TLMX_ID_INDEX       ], TLMX_PACKET_ID_SIZE        );
  memcpy( &(tlmx_payload_ptr->command  ), &buffer[ TLMX_COMMAND_INDEX  ], TLMX_PACKET_COMMAND_SIZE   );
  memcpy( &(tlmx_payload_ptr->status   ), &buffer[ TLMX_STATUS_INDEX   ], TLMX_PACKET_STATUS_SIZE    );
  memcpy( &(tlmx_payload_ptr->address  ), &buffer[ TLMX_ADDRESS_INDEX  ], TLMX_PACKET_ADDRESS_SIZE   );
  memcpy( &(tlmx_payload_ptr->data_len ), &buffer[ TLMX_DATA_LEN_INDEX ], TLMX_PACKET_DATA_LEN_SIZE  );
  assert( tlmx_payload_ptr->data_len == 0 || tlmx_payload_ptr->data_ptr != NULL );
  if (tlmx_payload_ptr->data_len != 0) {
    memcpy(   tlmx_payload_ptr->data_ptr, &buffer[TLMX_DATA_PTR_INDEX], tlmx_payload_ptr->data_len  );
  }
  return TLMX_DATA_PTR_INDEX + tlmx_payload_ptr->data_len;
}

int get_tlmx_data_len(char buffer[])
{
  int data_len;
  assert(buffer != NULL);
  memcpy( &data_len, &buffer[ TLMX_DATA_LEN_INDEX ], TLMX_PACKET_DATA_LEN_SIZE  );
  return data_len;
}

void print_tlmx(tlmx_packet* tlmx_payload_ptr, const char* message)
{
  assert(tlmx_payload_ptr != NULL);
  if (message != 0) printf("%s ",message);
  printf
  ( "tlmx_packet{%d,%s,%s,0x%llx,%d,"
  , tlmx_payload_ptr->id      
  , tlmx_command_to_str(tlmx_payload_ptr->command)
  , tlmx_status_to_str(tlmx_payload_ptr->status)
  , tlmx_payload_ptr->address 
  , tlmx_payload_ptr->data_len
  );
  assert( tlmx_payload_ptr->data_len == 0 || tlmx_payload_ptr->data_ptr != NULL );
  for(int i=0;i!=tlmx_payload_ptr->data_len;++i) {
    if (i < 16 || i == tlmx_payload_ptr->data_len-1) printf("%02x ",tlmx_payload_ptr->data_ptr[i]);
    if (i == 16) printf("... ");
  }
  printf("}\n");
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
    command = random()%(TLMX_EXIT+1);
    address = random();
    if (random()%4<3 && address & 0x80800000) address |= (random() << 32);
    if (i < sizeof(test_len)) data_len = test_len[i];
    else                      data_len = random()%TLMX_MAX_DATA_LEN+1;
    bzero(d1,TLMX_MAX_DATA_LEN);
    for (int i=0; i!= data_len; ++i) {
      d1[i] = random();
    }
    p1 = new_tlmx_packet( command, address, data_len, &d1[0] );
    if (i < sizeof(test_len)) p1->status  = random()%(TLMX_GENERIC_ERROR_RESPONSE+1);
    print_tlmx(p1,"p1");
    bzero(buffer,TLMX_MAX_BUFFER);
    pack_tlmx(buffer,p1);
    memset(d2,0xFFu,TLMX_MAX_DATA_LEN);
    p2 = new_tlmx_packet( TLMX_IGNORE, 0, 0, &d2[0] );
    unpack_tlmx(p2,buffer);
    print_tlmx(p2,"p2");
    if (compare_tlmx_packet(p1,p2)) printf("ERROR: tlmx_packet mismatch!\n");
    delete_tlmx_packet(p1);
    delete_tlmx_packet(p2);
  }
}
#endif
