#ifndef SOFTWARE_H
#define SOFTWARE_H

struct Local_initiator_module;

struct Software
{
  Software(Local_initiator_module* _sys):sys(_sys),fd(0) {}
  int sw_main(void);
  void interrupt_handler(void);
  int exitcode(int retcode);
private:
  Local_initiator_module* sys;
  int fd;
};

#endif /*SOFTWARE_H*/
