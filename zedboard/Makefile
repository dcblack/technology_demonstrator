#!/bin/make

#TARGET_ARCH := LINUX|OSX|ZEDBOARD
TARGET_ARCH  := ZEDBOARD

SRCS:=\
  random.c\
  creport.c\
  tlmx_packet.c\
  driver.c\
  software.c

include ../Makefile.common

###############################################################################
HDRS:=$(wildcard $(patsubst %.c,  %.h,${SRCS}))
OBJS:=$(patsubst %.c,  %.o,${SRCS})
LNX=software.${HOST_ARCH}

DEFS:=-D_${TARGET_ARCH}
INCDIRS:=
CFLAGS:=
LFLAGS:=
ifeq "${TARGET_ARCH}" "ZEDBOARD"
  $(info INFO: $(BOLDRED)Targeting ZEDBOARD$(NONE))
  PTHREAD:=-lpthread
  PRE:=arm-xilinx-eabi
  CPP:=${PRE}-gcc
  INCDIRS:=${XILINX_EDK}/gnu/arm/lin/arm-xilinx-linux-gnueabi/libc/usr/include
  LIBDIRS:=${XILINX_EDK}/gnu/arm/lin/arm-xilinx-linux-gnueabi/libc/usr/lib
  LIBS:=pthread c
  CFLAGS:=-static
  LFLAGS:=-static
  DEFS+=-D__USE_GNU -D_POSIX_SOURCE -DSILENT
  EXE:=software.zed
else
  $(info INFO: $(BOLDRED)Targeting LINUX HOST$(NONE))
  PRE:=
  CPP:=gcc
  LIBS:=pthread c
  PTHREAD:=-pthread
  DEFS+=-D_POSIX_SOURCE -D__USE_POSIX199506
  EXE:=${LNX}
endif

CFLAGS+= $(DEFS)
CFLAGS+= $(addprefix -I,$(wildcard ${INCDIRS}))
LFLAGS+= $(addprefix -L,$(wildcard ${LIBDIRS}))
LFLAGS+= $(addprefix -l,${LIBS})

# Following compiles entire libraries if specified
ifdef ILIBS
LFLAGS+= $(wildcard $(foreach d,${LIBDIRS},$(patsubst $d/%,lib%.a,${ILIBS})))
endif

# link
$(EXE): $(OBJS) $(HDRS)
	@-rm -f $@; $(call REPORT_TITLE,Linking $@)
	${CPP} -o $@ $? $(LFLAGS)
	@$(RULER)

# compile
%.o: %.c
	@-rm -f $*.o; $(call REPORT_TITLE,Compiling $?)
	${CPP} $(CFLAGS) -g ${PTHREAD} -std=c99 -c -o $*.o $*.c

clean:
	-rm -f *.o *.x

distclean: clean
	-rm -f *.log *~ *.bak

objs: clean $(OBJS) $(HDRS)

link: $(EXE)

exe: clean $(EXE)

gdb: $(LNX)
	gdb ./$(LNX)

run: $(LNX)
	./$(LNX) localhost 4000

#TAF!
