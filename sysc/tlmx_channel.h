#ifndef TLMX_CHANNEL_H
#define TLMX_CHANNEL_H

#include "use_cxx11.h"
#include "async_thread_if.h"
#include "async_sysc_if.h"
#include <list>
#include <mutex>

// Implements a channel to interface from an OS thread to SystemC. To clarify
// notation of who does what, we use push/pull from the thread side and
// put/get from the systemc side. Notice the interfaces separate the two
// sides (SystemC vs asynchronous thread).
struct tlmx_channel
: sc_core::sc_prim_channel
, virtual async_sysc_if
, virtual async_thread_if
{
  //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Constructors
  //............................................................................
  tlmx_channel(const char* instance_name);
  ~tlmx_channel(void);

  //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // OS thread/process calls
  //............................................................................
  // NOTE: Push can block waiting for mutex, but should be brief
  void             push         (tlmx_packet_ptr  tlmx_payload_ptr) override;
  void             wait_for_get (void) const override;
  void             wait_for_put (void) const override;
  bool             can_pull     (void) const override;
  bool             nb_pull      (tlmx_packet_ptr& tlmx_payload_ptr) override;

  //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // SystemC calls
  //............................................................................
  bool             can_get      (void) const override;
  bool             nb_get       (tlmx_packet_ptr& tlmx_payload_ptr) override;
  void             get          (tlmx_packet_ptr& tlmx_payload_ptr) override;
  tlmx_packet_ptr& get          (void) override;
  // NOTE: Put never blocks because unbounded queue
  void             nb_put       (tlmx_packet_ptr tlmx_payload_ptr) override;
  const sc_core::sc_event& sysc_pushed_event(void) const override;
  const sc_core::sc_event& sysc_pulled_event(void) const override;
  const sc_core::sc_event& default_event(void) const override { return sysc_pushed_event(); }
  void update(void) override;

  //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Internal
  //............................................................................
private:
  std::list<tlmx_packet_ptr>    m_queue_to_sysc;   //< data from thread
  bool                          m_thread_did_push; //< indicates push to above
  std::list<tlmx_packet_ptr>    m_queue_fm_sysc;   //< data from systemc
  bool                          m_sysc_did_put;    //< indicates push to above
  bool                          m_thread_did_pull; //< indicates get from above
  sc_core::sc_event             m_sysc_pushed_event;  //< indicates thread put
  sc_core::sc_event             m_sysc_pulled_event;  //< indicates thread put
  mutable std::mutex            m_mutex_to_sysc;   //< locks shared structures
  mutable std::mutex            m_mutex_fm_sysc;   //< locks shared structures
  mutable std::mutex            m_mutex_wait_get;  //< wait for this
  mutable std::mutex            m_mutex_wait_put;  //< wait for this
};

#endif /*TLMX_CHANNEL_H*/
