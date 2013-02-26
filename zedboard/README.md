This directory contains C code designed for a Linux host with an ethernet
connection.

HOW TO RUN
==========

To build the software, type:

* make exe

Before running this software, it is neccesary to connect the ethernet to a host
that is running the SystemC server/simulator software called zynq.x.  On the
remote host, type:

* ./zynq.x -port=PORTNUMBER

It should output half a page of info and then pause.

To execute the software in this directory type:

* ./software.x HOSTNAME PORTNUMBER

Where HOSTNAME should designate a host running the SystemC simulator on the
ethernet. You can specify either a DNS name or the IP address (e.g.
198.168.1.100)

Port numbers should match and be any number greater than 2000. Suggest using
4000.
