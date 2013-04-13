# MODBUS sources
MODBUSDIR = ${TOPDIR}/Libraries/freemodbus-v1.5.0
# List of all port files.
PORTDIR = ${MODBUSDIR}/projects/${CHIP}
PORTSRC  =
PORTSRC += ${PORTDIR}/port/portevent.c
PORTSRC += ${PORTDIR}/port/portserial.c
PORTSRC += ${PORTDIR}/port/porttimer.c
PORTSRC += ${PORTDIR}/port/mbrtu.c
PORTSRC += ${PORTDIR}/port/mbcrc.c
PORTSRC += ${PORTDIR}/port/mb.c
PORTSRC += ${PORTDIR}/demo.c

MBDIR = ${MODBUSDIR}/modbus
MBSRC  =
#MBSRC += ${MBDIR}/rtu/mbrtu.c
#MBSRC += ${MBDIR}/rtu/mbcrc.c
#MBSRC += ${MBDIR}/mb.c
MBSRC += ${MBDIR}/functions/mbfunccoils.c
MBSRC += ${MBDIR}/functions/mbfuncdiag.c
MBSRC += ${MBDIR}/functions/mbfuncdisc.c
MBSRC += ${MBDIR}/functions/mbfuncholding.c
MBSRC += ${MBDIR}/functions/mbfuncinput.c
MBSRC += ${MBDIR}/functions/mbfuncother.c
MBSRC += ${MBDIR}/functions/mbutils.c

INCDIR += $(PORTDIR)/port
INCDIR += $(MBDIR)/ascii
INCDIR += $(MBDIR)/functions
INCDIR += $(MBDIR)/include
INCDIR += $(MBDIR)/rtu
INCDIR += $(MBDIR)/tcp
INCDIR += $(MODBUSDIR)/projects/common/inc

CSRC += $(PORTSRC) $(MBSRC)