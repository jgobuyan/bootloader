
                      LINUX SECURE BOOTLOADER UTILITY
                      ===============================

REQUIREMENTS
============

This set of port files implements a simplified ModBus master to interface with
the secure bootloader on the embedded microprocessor. The demo file provides an
example of its usage.

INSTALLATION
============

The program can be built by calling 'make'.   This application was tested on
cygwin. However, the latest versions of make will not create proper .d files
(it occasionally creates Windows-type paths) and causes compilation issues.
Version 3.81 of make will work.

TESTING
=======

This utility requires an embedded processor with the matching secure bootloader
for testing. 
