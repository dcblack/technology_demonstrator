#!/bin/make -f

default:
	env ARGV="-version=1.2" vivado_hls -f devscript.tcl -l hls.log

test:
	env ARGV="-version=1.2" vivado_hls -f testscript.tcl -l test.log

fail:
	exit 1

pass:
	exit 0
