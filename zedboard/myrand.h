#ifndef MYRAND_H
#define MYRAND_H

void  my_srandom(unsigned int x);
char* my_initstate(unsigned int seed, char *arg_state, unsigned int n);
char* my_setstate(const char *arg_state);
long  my_random(void);

#endif /*MYRAND_H*/
