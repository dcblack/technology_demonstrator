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
# This makefile is designed for C++ code that is cross-compiled (optionally).
#
# MAKE TARGETS:
#   clean - removes objects, executables and other compilation features
#   distclean - removes logs & backups in addition to clean
#   exe (default) - creates executable
#   gdb - runs debugger
#   link - creates executable
#   objs - compiles all objects
#   run - executes program

UNIT:=profile
SRCS:=$(strip   \
  int.cpp   \
  matrix.cpp    \
  memory.cpp    \
  orig.cpp  \
)

ifdef CXX11
override CXX11:=-DCXX11 -std=c++11 $(if $(subst Linux,,$(shell uname -s)),-stdlib=libc++)
endif
# Tell automation to ignore SystemC content
NOT_SYSTEMC:=1

# The following conditional allows an upper level to include this
# makefile to obtain a list of source files:
ifndef INCLUDED

# Choose a target from:  LINUX|OSX|ZEDBOARD
TARGET_ARCH := linux64

.PHONY: default clean distclean objs exe gdb link run

DFLT:=run

include ../etc/Makefile.common

#----------------------------------------------------------------------
# Other optional macros you may wish to set. It is preferrable that
# these are the only additional flags you set.
#
#BOOST_LIBS:=
#OTHER_CFLAGS:=
#OTHER_LDFLAGS:=
OTHER_DEFS:=-DUSING_MATRIX
OTHER_INCDIRS:=-I/usr/local/include -I/opt/local/include
OTHER_LIBDIRS:=-L/usr/local/lib -L/opt/local/lib
OTHER_LIBS:=-lprofiler
#USING_SYSTEMC:=1
#USING_SCV:=1
#USING_TLM:=1

###############################################################################
ifndef OBJS
  OBJS:=${SRCS}
  OBJS:=$(patsubst %.cpp,%.o,${OBJS})
  OBJS:=$(patsubst %.cxx,%.o,${OBJS})
  OBJS:=$(patsubst %.cc, %.o,${OBJS})
  OBJS:=$(patsubst %.c,  %.o,${OBJS})
endif
LNX = ${UNIT}.${HOST_ARCH}
SCR = ${UNIT}.scr
ifdef CXX11
override CXX11:=-DCXX11 -std=c++11 $(if $(subst Linux,,$(shell uname -s)),-stdlib=libc++)
endif

DEFS    += -D_${TARGET_ARCH} ${OTHER_DEFS}
INCDIRS := ${OTHER_INCDIRS}
CFLAGS  :=
LFLAGS  := ${OTHER_LIBDIRS} ${OTHER_LIBS}
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
	${CXX} $(CFLAGS) -g ${PTHREAD} ${CXX11} -c -o $*.o $*.cpp

%.o: %.c
	@-rm -f $*.o; $(call REPORT_TITLE,Compiling $?)
	${CC} $(CFLAGS) -g ${PTHREAD} -std=c99 -c -o $*.o $*.c

clean:
	@$(call REPORT_TITLE,Cleaning)
	-rm -f *.o ${EXE}

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
	./$(LNX) 

export EXE
export UNIT
export TARGET_ARCH

profile: $(LNX)
	@$(call REPORT_TITLE,Profile program)
	./$(SCR)

endif
#TAF!
