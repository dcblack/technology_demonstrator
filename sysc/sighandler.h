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
