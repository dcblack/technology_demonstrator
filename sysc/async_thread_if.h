#ifndef ASYNC_THREAD_IF_H
#define ASYNC_THREAD_IF_H

#include "tlmx_packet.h"

struct async_thread_if
{
  virtual void push(tlmx_packet_ptr  tlmx_payload_ptr) = 0;
  virtual bool can_pull(void) const = 0;
  virtual bool nb_pull(tlmx_packet_ptr& tlmx_payload_ptr) = 0;
  virtual void wait_for_get (void) const = 0;
  virtual void wait_for_put (void) const = 0;
};

#endif /*ASYNC_THREAD_IF_H*/
