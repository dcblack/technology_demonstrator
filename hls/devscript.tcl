#!vivado_hls -f
#
# Dev project script
#
# If environment variable ARGV is set then options are
# determined from tokens. Currently supported are NORESET,
# NOSIM, NOSYNTH, NOCOSIM, and NOEXPORT. By default, we
# do everything.

# change to directory containing script
cd [file dirname [info script]]
pwd

# aids & conveniences
proc header args {puts [exec ../bin/header -cpp -cols=80 $args]}
proc quit {} {exit}

# Map each ARGV entry into an array so they can easily be tests as "options"
global env
set argv [split $env(ARGV)]
for {set i 0} {$i<$argc} {incr i} {set OPT([lindex $argv $i]) 1}
puts "Options from environment ARGV: [array names OPT]"
#if {[info exists OPT(STOP)]} {puts "STOPPING" ; illegal}

# Find path to source code
set src "[exec realpath ./src ../src ../../src ../../../src]"

header SETUP
if {[info exists OPT(NORESET)]} {
  open_project mtxdev
} else {
  open_project -reset mtxdev
}
set_top dev_hls

# Design
add_files $src/dev_hls.cpp

# Testbench
add_files -tb $src/command.cpp
add_files -tb $src/dev_test.cpp
add_files -tb $src/dev_util.cpp
add_files -tb $src/matrix.cpp
add_files -tb $src/memory.cpp
add_files -tb $src/registers.cpp

# ************CUSTOMIZE SOLUTIONS BELOW HERE************
# zedboard
set part {xc7z020clg484-1}
set period 10
# names from following array are used foreach solutions
set constraints(solution1) {}
# ************CUSTOMIZE SOLUTIONS ABOVE HERE************

foreach solution [array names constraints] {
  header $solution
  # Basic constraints
  if {[info exists OPT(NORESET)]} {
    open_solution $solution
  } else {
    open_solution -reset $solution
  }
  set_part $part
  create_clock -period $period -name default

  # Directives
  if {$constraints($solution) != ""} {
    eval $constraints($solution)
  }

  if {![info exists OPT(NOSIM)]} {
    header SIMULATE
    csim_design -clean
  }

  if {![info exists OPT(NOSYNTH)]} {
    header SYNTHESIZE
    csynth_design
  }

  if {![info exists OPT(NOCOSIM)]} {
    header COSIM
    cosim_design -trace_level none
  }

  if {![info exists OPT(NOEXPORT)]} {
    header EXPORT
    export_design -format ip_catalog -description "An IP generated by Vivado HLS" -vendor "xilinx.com" -library "hls" -version "1.0"
  }
}

header FINISHED

exit
