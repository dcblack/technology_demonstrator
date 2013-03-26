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

UNIT:=async_adaptor
SRCS:=\
  sc_literals.cpp\
  netlist.cpp\
  report.cpp\
  tlmx_packet.cpp\
  tlmx_channel.cpp\
  async_adaptor.cpp\
  dev.cpp\
  top.cpp\
  main.cpp

# The following conditional allows an upper level to include this
# makefile to obtain a list of source files:
ifndef INCLUDED

DFLT:=run

#----------------------------------------------------------------------
# Other optional macros you may wish to set. It is preferrable that
# these are the only additional flags you set.
#
#BOOST_LIBS:=
#OTHER_CFLAGS:=
#OTHER_LDFLAGS:=
#OTHER_INCDIRS:=
#OTHER_LIBDIRS:=
#OTHER_LIBS:=
#USING_SYSTEMC:=1
#USING_SCV:=1
#USING_TLM:=1

#----------------------------------------------------------------------
# *** DO NOT EDIT BELOW THIS LINE ***** DO NOT EDIT BELOW THIS LINE ***
#----------------------------------------------------------------------
# The following locates the closest definition file containing the rules
# that are used to compile designs anywhere here or above to three levels
# or in the $HLDW/etc directory..
ifndef EDA
  # Where EDA related scripts/tools are stored (system defaults)
  EDA:=$(firstword $(wildcard $(HOME)/eda /eda))
endif
ifndef HLDW
  # Locally developed scripts/tools
  $(warning Defining HLDW)
  HLDW:=$(EDA)/hldw/default
endif
# Allow for overrides locally, but normally use $HLDW/etc
RULEDIRS := . .. ../.. ../../.. ../../../.. $(HLDW)/etc
RULES := $(addsuffix /etc/Makefile.rules,$(RULEDIRS))
RULES += $(addsuffix /Makefile.rules,$(RULEDIRS))
RULES := $(firstword $(wildcard $(RULES)))
$(info Including $(RULES))
include $(RULES)

endif

# COPYRIGHT (C) 2013 Doulos Inc {{{
# 
# -----------------------------------------------------------------}}}
#END $Id$
