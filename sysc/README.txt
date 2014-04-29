This directory contains SystemC source for two fairly different purposes that
for better or worse are merged. The purposes are as follows:

1. A simulation driven from external code representing the zynq software
   via ethernet. The systemc acts as a TLM 2.0 target. The initiator is a
   TCP/IP receiver. Some ideas of incorporating an reverse direction interrupt
   request are not yet completed.

2. A simulation driven from a local initiator that represents the software
   running natively as a thread.

There are both compile-time and run-time variables that control this collision.
See top.cpp for a better idea of how it fits together. Both simulations were
compiled and driven by a common makefile.
