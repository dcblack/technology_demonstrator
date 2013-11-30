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

# MAKE TARGETS:
#   clean - removes objects, executables and other compilation features
#   distclean - removes logs & backups in addition to clean
#   exe (default) - creates executable
#   gdb - runs debugger
#   link - creates executable
#   objs - compiles all objects
#   run - executes program
#   flashem-ut - runs unit test of flashem.c

UNIT := application
SRCS :=\
  memory.cpp \
  matrix.cpp \
  dev.cpp \
  int.cpp \
  application.cpp
OTHER_DEFS := -DUSING_HARDWARE

# The following conditional allows an upper level to include us to
# obtain a list of source files
ifndef INCLUDED

# Choose a target from:  LINUX|OSX|ZEDBOARD
TARGET_ARCH := ZEDBOARD

.PHONY: default clean distclean objs exe gdb link run flashem-ut

default: clean exe

include ../etc/Makefile.common

###############################################################################
ifndef OBJS
  OBJS:=${SRCS}
  OBJS:=$(patsubst %.cpp,%.o,${OBJS})
  OBJS:=$(patsubst %.cxx,%.o,${OBJS})
  OBJS:=$(patsubst %.cc, %.o,${OBJS})
  OBJS:=$(patsubst %.c,  %.o,${OBJS})
endif
LNX = ${UNIT}.${HOST_ARCH}

DEFS    += -D_${TARGET_ARCH} ${OTHER_DEFS}
INCDIRS :=
CFLAGS  :=
LFLAGS  :=
ifeq "${TARGET_ARCH}" "ZEDBOARD"
  $(info INFO: ${BOLDRED}Targeting ZEDBOARD${NONE})
  PRE     := arm-xilinx-linux-gnueabi
  CC      := ${PRE}-gcc
  CPP     := ${PRE}-g++
  INCDIRS := ${XILINX_EDK}/gnu/arm/lin/arm-xilinx-linux-gnueabi/libc/usr/include
  LIBDIRS := ${XILINX_EDK}/gnu/arm/lin/arm-xilinx-linux-gnueabi/libc/usr/lib
  DEFS    += -D__USE_GNU -D_POSIX_SOURCE

# For a static image uncomment the following, but be sure to have ALL
# libraries referenced.
ifdef USING_STATIC
  CFLAGS  := -static
  LFLAGS  := -static
endif
ifdef USING_CXX11
  CXXFLAGS := -std=c++11
endif

# Uncomment following to suppress printf withing zed code
# DEFS    += -DSILENT

  EXE     := application.zed
else
  $(info INFO: ${BOLDRED}Targeting LINUX HOST${NONE})
  PRE  :=
  CC   := gcc
  CPP  := g++
  DEFS += -D_POSIX_SOURCE -D__USE_POSIX199506
  EXE  := ${LNX}
  PTHREAD := -pthread
endif

CFLAGS += ${DEFS}
CFLAGS += $(addprefix -I,$(wildcard ${INCDIRS}))
LFLAGS += $(addprefix -L,$(wildcard ${LIBDIRS}))
LFLAGS += $(addprefix -l,${LIBS})

# Following compiles entire libraries if specified
ifdef ILIBS
LFLAGS+= $(wildcard $(foreach d,${LIBDIRS},$(patsubst $d/%,lib%.a,${ILIBS})))
endif

# link
${EXE}: ${OBJS}
	@-rm -f $@; $(call REPORT_TITLE,Linking $@)
	${CPP} -o $@ ${OBJS} ${LFLAGS}
	@${RULER}

# compile
%.o: %.cpp
	@-rm -f $*.o; $(call REPORT_TITLE,Compiling $?)
	${CPP} ${CFLAGS} -g -c -o $*.o $*.cpp

%.o: %.c
	@-rm -f $*.o; $(call REPORT_TITLE,Compiling $?)
	${CC} ${CFLAGS} -g -std=c99 -c -o $*.o $*.c

clean:
	@$(call REPORT_TITLE,Cleaning)
	-rm -f *.o *.x

distclean: clean
	@$(call REPORT_TITLE,Cleaning distribution)
	-rm -f *.log *~ *.bak

objs: clean ${OBJS}

link: ${EXE}

exe: ${EXE}

gdb: ${LNX}
	@$(call REPORT_TITLE,Enter debugger)
	gdb ./${LNX}

run: ${LNX}
	@$(call REPORT_TITLE,Run program)
	./${LNX} localhost 4000

endif
#TAF!
