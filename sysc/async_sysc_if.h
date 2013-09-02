#ifndef ASYNC_SYSC_IF_H
#define ASYNC_SYSC_IF_H

#include "tlmx_packet.h"
#include <systemc>

struct async_sysc_if : sc_core::sc_interface
{
  virtual bool nb_get(tlmx_packet_ptr& tlmx_payload_ptr)         = 0;
  virtual bool can_get(void) const                               = 0;
  virtual void get(tlmx_packet_ptr& tlmx_payload_ptr)            = 0;
  virtual tlmx_packet_ptr& get(void)                             = 0;
  virtual void nb_put(tlmx_packet_ptr tlmx_payload_ptr)          = 0;
  virtual const sc_core::sc_event& default_event(void) const     = 0;
  virtual const sc_core::sc_event& sysc_pushed_event(void) const = 0;
  virtual const sc_core::sc_event& sysc_pulled_event(void) const = 0;
};

#endif /*ASYNC_SYSC_IF_H*/
