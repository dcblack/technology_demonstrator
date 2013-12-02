#!/usr/bin/make -f
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
#
# MAKE TARGETS:
#   clean - removes objects, executables and other compilation features
#   distclean - removes logs & backups in addition to clean
#   exe (default) - creates executable
#   gdb - runs debugger
#   link - creates executable
#   objs - compiles all objects
#   run - executes program
#   flashem-ut - runs unit test of flashem.c

UNIT := software
SRCS :=\
  random.c      \
  creport.c     \
  tlmx_packet.c \
  driver.c      \
  ledsw.c       \
  flashem.c     \
  application.c \
  software.c

# The following conditional allows an upper level to include us to
# obtain a list of source files
ifndef INCLUDED

# Choose a target from:  LINUX|OSX|ZEDBOARD
TARGET_ARCH := ZEDBOARD

.PHONY: default clean distclean objs exe gdb link run flashem-ut

default: clean exe

include ../etc/Makefile.common

#----------------------------------------------------------------------
# Other optional macros you may wish to set. It is preferrable that
# these are the only additional flags you set.
#
#BOOST_LIBS:=
#OTHER_CFLAGS:=
#OTHER_LDFLAGS:=
#OTHER_DEFS:=
#OTHER_INCDIRS:=
#OTHER_LIBDIRS:=
#OTHER_LIBS:=
###############################################################################
override OBJS := $(patsubst %.c,  %.o,${SRCS})
LNX = ${UNIT}.${HOST_ARCH}
ifdef CXX11
override CXX11:=-DCXX11 -std=c++11 $(if $(subst Linux,,$(shell uname -s)),-stdlib=libc++)
endif

DEFS    += -D_${TARGET_ARCH} ${OTHER_DEFS}
INCDIRS := ${OTHER_INCDIRS}
CFLAGS  := ${OTHER_CFLAGS}
LFLAGS  := ${OTHER_LDFLAGS} ${OTHER_LIBDIRS} ${OTHER_LIBS}
ifeq "${TARGET_ARCH}" "ZEDBOARD"
  $(info INFO: $(BOLDRED)Targeting ZEDBOARD$(NONE))
  PRE     := arm-xilinx-linux-gnueabi
  CC      := ${PRE}-gcc
  CXX     := ${PRE}-g++
  INCDIRS := ${XILINX_EDK}/gnu/arm/lin/arm-xilinx-linux-gnueabi/libc/usr/include
  LIBDIRS := ${XILINX_EDK}/gnu/arm/lin/arm-xilinx-linux-gnueabi/libc/usr/lib
  LIBS    := pthread c
  DEFS    += -D__USE_GNU -D_POSIX_SOURCE

# For a static image uncomment the following, but be sure to have ALL
# libraries referenced.
# CFLAGS  := -static
# LFLAGS  := -static

# Uncomment following to suppress printf withing zed code
# DEFS    += -DSILENT

  EXE     := software.zed
else
  $(info INFO: $(BOLDRED)Targeting LINUX HOST$(NONE))
  PRE  :=
  CC   := gcc
  CXX  := g++
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
$(EXE): $(OBJS)
	@-rm -f $@; $(call REPORT_TITLE,Linking $@)
	${CXX} -o $@ $(OBJS) $(LFLAGS)
	@$(RULER)

# compile
%.o: %.cpp
	@-rm -f $*.o; $(call REPORT_TITLE,Compiling $?)
	${CXX} $(CFLAGS) -g ${PTHREAD} ${CXX11} -c -o $*.o $*.c

%.o: %.c
	@-rm -f $*.o; $(call REPORT_TITLE,Compiling $?)
	${CC} $(CFLAGS) -g ${PTHREAD} -std=c99 -c -o $*.o $*.c

clean:
	@$(call REPORT_TITLE,Cleaning)
	-rm -f *.o *.x

distclean: clean
	@$(call REPORT_TITLE,Cleaning distribution)
	-rm -f *.log *~ *.bak

objs: clean $(OBJS)

link: $(EXE)

exe: $(EXE)

gdb: $(LNX)
	@$(call REPORT_TITLE,Enter debugger)
	gdb ./$(LNX)

run: $(LNX)
	@$(call REPORT_TITLE,Run program)
	./$(LNX) localhost 4000

# unit-test
flashem-ut:
	$(MAKE) \
          OTHER_DEFS=-DTEST_FLASHEM\
          SRCS="creport.c ledsw.c flashem.c"\
          TARGET_ARCH=LINUX\
          UNIT=flashem\
          LNX=flashem.${HOST_ARCH}\
          run

flashem.${HOST_ARCH}: creport.o ledsw.o flashem.o

endif
#TAF!
