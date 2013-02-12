#!/bin/make

SRCS:=\
  creport.c\
  tlmx_packet.c\
  driver.c\
  software.c

HDRS:=$(wildcard $(patsubst %.c,  %.h,${SRCS}))
OBJS:=$(patsubst %.c,  %.o,${SRCS})
EXE:=software.x

.PHONY: default clean distclean objs exe gdb run

default: run

$(EXE): $(OBJS) $(HDRS)
	@-rm -f $@
	gcc -pthread -o $@ $?

%.o: %.c
	@-rm -f $*.o
	gcc -g -pthread -std=c99 -c -o $*.o $*.c

clean:
	-rm -f *.o *.x

distclean: clean
	-rm -f *.log *~ *.bak

objs: clean $(OBJS) $(HDRS)

exe: clean $(EXE)

gdb: $(EXE)
	gdb ./$(EXE)

run: $(EXE)
	./$(EXE) localhost 4000

#TAF!
