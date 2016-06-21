#ifndef TLMX_PACKET_H
#define TLMX_PACKET_H

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

/*------------------------------------------------------------------------------
 * WARNING
 * =======
 * This header works for both C and C++, but the implementations
 * are in separate .c and .cpp files respectively and should be
 * linked thusly. Be careful to observe the #ifdef's below...
 *------------------------------------------------------------------------------
 */

#include <stdint.h>

typedef enum 
{ TLMX_IGNORE
, TLMX_READ
, TLMX_WRITE
, TLMX_DEBUG_READ
, TLMX_DEBUG_WRITE
, TLMX_MUTEX_LOCK
, TLMX_MUTEX_TRYLOCK //< (data[0] == 0) implies failure
, TLMX_MUTEX_UNLOCK  
, TLMX_INTERRUPT
, TLMX_EXIT /*< must be last - this command is NOT TLM 2.0, but used to close the connection */
} tlmx_command_t;
const char* tlmx_command_to_str(tlmx_command_t s);

typedef enum
{ TLMX_INCOMPLETE_RESPONSE
, TLMX_OK_RESPONSE
, TLMX_ADDRESS_ERROR_RESPONSE
, TLMX_GENERIC_ERROR_RESPONSE /*< must be last */
} tlmx_status_t;
const char* tlmx_status_to_str(tlmx_status_t s);

// REMINDER: If you modify fields in the following, you must also
// carefully modify the _SIZE & _INDEX constants (macros). Also update
// the pack/unpack routines in respective .c and .cpp implementations.
#ifndef __cplusplus
typedef struct {
#else
#include <string>
struct tlmx_packet
{
  tlmx_packet //< Constructor
  ( uint8_t  command 
  , uint64_t address 
  , uint16_t data_len
  , uint8_t* data_ptr
  , uint64_t delay_ns = 0UL
  , uint64_t time_ns = 0UL
  , uint16_t strm_wid = 0
  );
  tlmx_packet(const tlmx_packet& rhs); // Copy constructor
  ~tlmx_packet(void); //< Destructor
  bool operator==(const tlmx_packet& rhs) const;
  tlmx_packet& operator=(const tlmx_packet& rhs);
  int  pack(char* buffer);
  int  unpack(char* buffer);
  std::string str(void) const;
  void print(const char* message) const;
#endif
  int32_t  id;      
  uint8_t  command; 
  uint8_t  status;  
  uint64_t address; 
  uint64_t delay_ns; 
  uint64_t time_ns;  
  uint16_t strm_wid;
  uint16_t data_len;
  uint8_t* data_ptr;
#ifndef __cplusplus
} tlmx_packet;
#else
private:
  static int32_t s_id;
};

#include <memory>
typedef std::shared_ptr<tlmx_packet> tlmx_packet_ptr;
#endif

#define TLMX_PACKET_ID_SIZE       4
#define TLMX_PACKET_COMMAND_SIZE  1
#define TLMX_PACKET_STATUS_SIZE   1
#define TLMX_PACKET_ADDRESS_SIZE  8
#define TLMX_PACKET_DELAY_NS_SIZE 8
#define TLMX_PACKET_TIME_NS_SIZE  8
#define TLMX_PACKET_STRM_WID_SIZE 8
#define TLMX_PACKET_DATA_LEN_SIZE 2

#define TLMX_ID_INDEX       0
#define TLMX_COMMAND_INDEX  (TLMX_ID_INDEX       + TLMX_PACKET_ID_SIZE)      
#define TLMX_STATUS_INDEX   (TLMX_COMMAND_INDEX  + TLMX_PACKET_COMMAND_SIZE) 
#define TLMX_ADDRESS_INDEX  (TLMX_STATUS_INDEX   + TLMX_PACKET_STATUS_SIZE)  
#define TLMX_DELAY_NS_INDEX (TLMX_ADDRESS_INDEX  + TLMX_PACKET_ADDRESS_SIZE)
#define TLMX_TIME_NS_INDEX  (TLMX_DELAY_NS_INDEX + TLMX_PACKET_DELAY_NS_SIZE)
#define TLMX_DATA_LEN_INDEX (TLMX_TIME_NS_INDEX  + TLMX_PACKET_TIME_NS_SIZE)
#define TLMX_STRM_WID_INDEX (TLMX_DATA_LEN_INDEX + TLMX_PACKET_TIME_NS_SIZE)
#define TLMX_DATA_PTR_INDEX (TLMX_STRM_WID_INDEX + TLMX_PACKET_DATA_LEN_SIZE)
#define TLMX_MAX_DATA_LEN   ((1<<(8*TLMX_PACKET_DATA_LEN_SIZE))-1)
#define TLMX_MAX_BUFFER     (TLMX_DATA_PTR_INDEX + TLMX_MAX_DATA_LEN)

#ifndef __cplusplus
tlmx_packet* new_tlmx_packet
( uint8_t  command 
, uint64_t address 
, uint16_t data_len
, uint8_t* data_ptr
, uint64_t delay_ns 
, uint64_t time_ns 
, uint64_t strm_wid 
);
void init_tlmx_packet(tlmx_packet* payload_ptr);
tlmx_packet* clone_tlmx_packet(tlmx_packet* rhs);
void delete_tlmx_packet(tlmx_packet* payload_ptr);
int  compare_tlmx_packet(tlmx_packet* p1, tlmx_packet* p2);
int  pack_tlmx(char buffer[], tlmx_packet* payload);
int  unpack_tlmx(tlmx_packet* payload_ptr, char buffer[]);
void print_tlmx(tlmx_packet* payload_ptr, const char* message);
#endif
int  get_tlmx_data_len(char buffer[]);

#endif /*TLMX_PACKET_H*/
