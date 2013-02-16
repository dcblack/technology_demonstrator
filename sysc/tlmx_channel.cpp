// FILE: tlmx_channel.cpp

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

#include "tlmx_channel.h"
#include <thread>
#include <mutex>
#include "report.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;
namespace {
  static char const* const MSGID = "/Doulos/example/tlmx_channel";
}

// Constructor
tlmx_channel::tlmx_channel(const char* instance_name)
: m_thread_did_push(false)
, m_thread_did_pull(false)
, m_sysc_did_put(false)
{
  m_mutex_wait_get.lock();
  m_mutex_wait_put.lock();
  REPORT_NOTE("m_mutex_wait_put LOCKED in thread");
}

// Destructor
tlmx_channel::~tlmx_channel(void)
{
}

void tlmx_channel::push(tlmx_packet_ptr tlmx_payload_ptr)
{
  // Lockdown and place in queue
  std::lock_guard<std::mutex> protect(m_mutex_to_sysc);
  m_queue_to_sysc.push_front(tlmx_payload_ptr);
  // Notify SystemC
  m_thread_did_push = true;
  async_request_update();
}

bool tlmx_channel::can_get(void) const
{
  // Lockdown and determine status of queue
  std::lock_guard<std::mutex> protect(m_mutex_to_sysc);
  return not m_queue_to_sysc.empty();
}

void tlmx_channel::get(tlmx_packet_ptr& tlmx_payload_ptr)
{
  while (not nb_get(tlmx_payload_ptr)) wait(m_sysc_get_event);
}

tlmx_packet_ptr& tlmx_channel::get(void)
{
  tlmx_packet_ptr* tlmx_payload_ptr = new tlmx_packet_ptr{0};
  get(*tlmx_payload_ptr);
  return *tlmx_payload_ptr;
}


bool tlmx_channel::nb_get(tlmx_packet_ptr& tlmx_payload_ptr)
{
  // Lockdown and obtain from queue
  std::lock_guard<std::mutex> protect(m_mutex_to_sysc);
  if (m_queue_to_sysc.empty()) return false;
  tlmx_payload_ptr = m_queue_to_sysc.back();
  m_queue_to_sysc.pop_back();
  // Release waiting threads
  m_mutex_wait_get.unlock();
  m_mutex_wait_get.lock();
  return true;
}

void tlmx_channel::wait_for_get(void) const
{
  m_mutex_wait_get.lock();
  m_mutex_wait_get.unlock();
}

void tlmx_channel::nb_put(tlmx_packet_ptr tlmx_payload_ptr)
{
  // Lockdown and push onto queue
  std::lock_guard<std::mutex> protect(m_mutex_fm_sysc);
  m_queue_fm_sysc.push_front(tlmx_payload_ptr);
  tlmx_payload_ptr->print("DEBUG: ");
  // Notify thread
  m_sysc_did_put = true;
  m_mutex_wait_put.unlock();
  m_mutex_wait_put.lock();
}

void tlmx_channel::wait_for_put(void) const
{
  m_mutex_wait_put.lock();
  REPORT_NOTE("m_mutex_wait_put LOCKED in thread");
  m_mutex_wait_put.unlock();
  REPORT_NOTE("m_mutex_wait_put UNlocked in thread");
}

bool tlmx_channel::can_pull(void) const
{
  // Lockdown and determine status of queue
  std::lock_guard<std::mutex> protect(m_mutex_fm_sysc);
  return not m_queue_fm_sysc.empty();
}

bool tlmx_channel::nb_pull(tlmx_packet_ptr& tlmx_payload_ptr)
{
  // Determine if anything available and wait if need be
  // Lockdown and obtain from queue
  std::lock_guard<std::mutex> protect(m_mutex_fm_sysc);
  if (m_queue_fm_sysc.empty()) return false;
  tlmx_payload_ptr = m_queue_fm_sysc.back();
  tlmx_payload_ptr->print("DEBUG: ");
  m_queue_fm_sysc.pop_back();
  // Notify SystemC
  m_thread_did_pull = true;
  m_sysc_did_put = false;
  async_request_update();
  return true;
}

const sc_core::sc_event& tlmx_channel::sysc_put_event(void) const
{
  return m_sysc_put_event;
}

const sc_core::sc_event& tlmx_channel::sysc_get_event(void) const
{
  return m_sysc_get_event;
}

void tlmx_channel::update(void)
{
  { // Handle push
    std::lock_guard<std::mutex> protect(m_mutex_to_sysc);
    if (m_thread_did_push) {
      m_sysc_put_event.notify(SC_ZERO_TIME);
      m_thread_did_push = false;
    }
  }
  { // Handle get
    std::lock_guard<std::mutex> protect(m_mutex_fm_sysc);
    if (m_thread_did_pull) {
      m_sysc_get_event.notify(SC_ZERO_TIME);
      m_thread_did_pull = false;
    }
  }
}
