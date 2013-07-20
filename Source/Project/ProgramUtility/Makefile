#
# FreeModbus Linux PORT - Makefile
#
# Copyright (c) 2006 Christian Walter, © s::can, Vienna 2006.
#
# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------
# platform dependencies
# ---------------------------------------------------------------------------
ifneq ($(strip $(shell gcc -v 2>&1 |grep "cygwin")),)
	CYGWIN_BUILD = YES
endif
CC	    = gcc
CXX	    = g++
OBJCOPY	    = objcopy
INSIGHT     = /opt/insight-x86/bin/insight

TOPDIR=../..
MBDIR=$(TOPDIR)/Libraries/freemodbus-v1.5.0
CCANDIR=$(TOPDIR)/Libraries/ccan_lib
SECDIR=$(TOPDIR)/Libraries/Security
# ---------------------------------------------------------------------------
# project specifics
# ---------------------------------------------------------------------------
CFLAGS	    = -MD -g3 -Wall \
	-Iport \
	-I$(MBDIR)/modbus/rtu \
	-I$(MBDIR)/modbus/ascii \
	-I$(MBDIR)/modbus/include \
	-I$(TOPDIR)/common/inc \
	-I$(SECDIR)/inc \
	-I$(SECDIR)/axTLS/ports/linux \
	-I$(SECDIR)/axTLS/crypto \
	-I$(SECDIR)/axTLS/ssl \
	-I$(CCANDIR)/ccan \
	-I$(CCANDIR)/ccan/crc \
	-I.
LDFLAGS     =
LDLIBS		= -L$(SECDIR)/axTLS/ports/linux -laxtls
LDFLAGS		= 
ifeq ($(CYGWIN_BUILD),YES)
else
LDFLAGS     += -lpthread
CFLAGS      += -pthread
endif

TGT         = prog_util
OTHER_CSRC  = 
OTHER_ASRC  = 
CSRC        = main.c port/portserial.c port/portother.c \
              port/portevent.c port/porttimer.c \
              port/mbmaster.c \
              port/mbrtumaster.c \
              cmd.c \
              crc32.c \
              cmd_getheader.c \
              cmd_prepareflash.c \
              cmd_uploadblock.c \
              cmd_validatesig.c \
              cmd_keys.c \
              util_addheader.c \
              util_upload.c \
              util_encrypt.c \
              util_sign.c \
              $(MBDIR)/modbus/rtu/mbcrc.c \
			  $(MBDIR)/modbus/ascii/mbascii.c \
              $(MBDIR)/modbus/functions/mbfunccoils.c \
              $(MBDIR)/modbus/functions/mbfuncdiag.c \
              $(MBDIR)/modbus/functions/mbfuncholding.c \
              $(MBDIR)/modbus/functions/mbfuncinput.c \
              $(MBDIR)/modbus/functions/mbfuncother.c \
              $(MBDIR)/modbus/functions/mbfuncdisc.c \
              $(MBDIR)/modbus/functions/mbutils.c \
              $(SECDIR)/src/blowfish.c
ASRC        = 
OBJS        = $(CSRC:.c=.o) $(ASRC:.S=.o)
NOLINK_OBJS = $(OTHER_CSRC:.c=.o) $(OTHER_ASRC:.S=.o)
DEPS        = $(OBJS:.o=.d) $(NOLINK_OBJS:.o=.d)
BIN         = $(TGT)

.PHONY: clean all

all: $(BIN)

debug:
	$(INSIGHT) --se=$(TGT)

$(BIN): $(OBJS) $(NOLINK_OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

clean:
	rm -f $(DEPS)
	rm -f $(OBJS) $(NOLINK_OBJS)
	rm -f $(BIN)

# ---------------------------------------------------------------------------
# rules for code generation
# ---------------------------------------------------------------------------
%.o:    %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.o:    %.S
	$(CC) $(ASFLAGS) -o $@ -c $<

# ---------------------------------------------------------------------------
#  # compiler generated dependencies
# ---------------------------------------------------------------------------
-include $(LWOS_DEPS) $(PORT_DEPS) $(APPL_DEPS)

