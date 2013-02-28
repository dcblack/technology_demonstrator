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
OTHER_TAR:=Makefile* */Makefile*

override DFLT:=tar

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

#EOF
