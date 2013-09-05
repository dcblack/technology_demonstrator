#include "sighandler.h"
#include <signal.h>

static unsigned int interrupts{0U};
Sighandler* Sighandler::singleton{nullptr};

static void sighandler(int sig)
{
  if (interrupts+1 != 0) ++interrupts;
}

void Sighandler::init(void) {
  if (singleton != nullptr) return;
  singleton = new Sighandler();
}

unsigned int Sighandler::stop_requests(void)
{ 
  return interrupts;
}

Sighandler::Sighandler(void) 
{
    signal(SIGINT,&sighandler); //< allow for graceful interrupts
}

//EOF
