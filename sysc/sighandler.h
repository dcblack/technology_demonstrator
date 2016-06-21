/*

DESCRIPTION
===========

This singleton class provides simple interface to receive the Linux SIGINT signal, which
is received from the terminal when typing control-C.

USAGE
=====

- To activate, simply invoke the static init() method.
- When an interrupt comes in, the number of interrupts is simply incremented.
- To check the current count, read stop_requests()

*/
#ifndef SIGHANDLER_H
#define SIGHANDLER_H

struct Sighandler
{
  static void init(void);
  static unsigned int stop_requests(void);
private:
  Sighandler(void);
  static Sighandler* singleton;
};

#endif /*SIGHANDLER_H*/
