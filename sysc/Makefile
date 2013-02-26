#!/usr/bin/make -f
# -*- make -*- vim:syntax=make:tw=72
# COPYRIGHT (C) 2013 Doulos Inc {{{
# 
# -----------------------------------------------------------------}}}

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
  EDA:=$(firstword $(wildcard $(HOME)/eda /eda))
endif
ifndef HLDW
  $(warning Defining HLDW)
  HLDW:=$(EDA)/hldw/default
endif
# Allow for overrides locally, but normally use $HLDW/etc
RULEDIRS := . $(shell parents -limit -3) $(HLDW)/etc
RULES := $(firstword $(wildcard $(addsuffix /Makefile.rules,$(RULEDIRS))))
$(info Including $(RULES))
include $(RULES)

# COPYRIGHT (C) 2013 Doulos Inc {{{
# 
# -----------------------------------------------------------------}}}
#END $Id$
