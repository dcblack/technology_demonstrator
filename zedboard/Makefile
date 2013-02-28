#!/bin/make
# -*- make -*- vim:syntax=make:tw=72
################################################################################
# $License: Apache 2.0 $
#
# This file is licensed under the Apache License, Version 2.0 (the "License").
# You may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# This makefile is designed for C code that is cross-compiled (optionally).

SRCS:=\
  random.c\
  creport.c\
  tlmx_packet.c\
  driver.c\
  software.c

# The following conditional allows an upper level to include us to obtain a list of
# source files
ifndef INCLUDED

# Choose a target from:  LINUX|OSX|ZEDBOARD
TARGET_ARCH  := ZEDBOARD

include ../Makefile.common

###############################################################################
HDRS := $(wildcard $(patsubst %.c,  %.h,${SRCS}))
OBJS := $(patsubst %.c,  %.o,${SRCS})
LNX = software.${HOST_ARCH}

DEFS:=-D_${TARGET_ARCH}
INCDIRS:=
CFLAGS:=
LFLAGS:=
ifeq "${TARGET_ARCH}" "ZEDBOARD"
  $(info INFO: $(BOLDRED)Targeting ZEDBOARD$(NONE))
  PRE     := arm-xilinx-linux-gnueabi
  CPP     := ${PRE}-gcc
  INCDIRS := ${XILINX_EDK}/gnu/arm/lin/arm-xilinx-linux-gnueabi/libc/usr/include
  LIBDIRS := ${XILINX_EDK}/gnu/arm/lin/arm-xilinx-linux-gnueabi/libc/usr/lib
  LIBS    := pthread c
  CFLAGS  := -static
  LFLAGS  := -static
  DEFS    += -D__USE_GNU -D_POSIX_SOURCE -DSILENT
  EXE     := software.zed
else
  $(info INFO: $(BOLDRED)Targeting LINUX HOST$(NONE))
  PRE  :=
  CPP  := gcc
  LIBS := pthread c
  DEFS += -D_POSIX_SOURCE -D__USE_POSIX199506
  EXE  := ${LNX}
  PTHREAD := -pthread
endif

CFLAGS += $(DEFS)
CFLAGS += $(addprefix -I,$(wildcard ${INCDIRS}))
LFLAGS += $(addprefix -L,$(wildcard ${LIBDIRS}))
LFLAGS += $(addprefix -l,${LIBS})

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

endif
#TAF!
