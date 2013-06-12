#!/usr/bin/make -f
#
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

# Define sources
INCLUDED:=1
include sysc/Makefile
SYSC_SRCS:=$(addprefix sysc/,${SRCS})
include zedboard/Makefile
ZEDB_SRCS:=$(addprefix zedboard/,${SRCS})

UNIT:=async_request_example
SRCS:=$(sort ${SYSC_SRCS} ${ZEDB_SRCS})

MESSAGE="WARNING: Top-level make NOT supported here - only 'tar'"
override DFLT:=help

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

#EOF
