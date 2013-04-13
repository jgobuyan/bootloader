##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O2 -ggdb -fomit-frame-pointer -falign-functions=16
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Enables the use of FPU on Cortex-M4.
# Enable this if you really want to use the STM FWLib.
ifeq ($(USE_FPU),)
  USE_FPU = no
endif

# Enable this if you really want to use the STM FWLib.
ifeq ($(USE_FWLIB),)
  USE_FWLIB = no
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Define project name here
PROJECT = bootloader
CHIP = STM32F37x
CHIP_PREFIX = stm32f37x
PLATFORM = STM32373C_EVAL
# Imported source files and paths
CSRC =

TOPDIR = ../../..
LIBDIR = ${TOPDIR}/Libraries/CMSIS
STDPERIPHDIR = ${TOPDIR}/Libraries/${CHIP}_StdPeriph_Driver
SECDIR = ${TOPDIR}/Libraries/Security
# List of all Library files
LIBSRC =
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_adc.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_can.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_cec.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_comp.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_crc.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_dac.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_dbgmcu.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_dma.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_exti.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_flash.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_gpio.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_i2c.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_iwdg.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_misc.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_pwr.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_rcc.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_rtc.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_sdadc.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_spi.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_syscfg.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_tim.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_usart.c
LIBSRC += ${STDPERIPHDIR}/src/${CHIP_PREFIX}_wwdg.c

# List of all IAP files.
IAPDIR = ${TOPDIR}/Project/SecureBoot
IAPSRC += ${IAPDIR}/src/common.c
IAPSRC += ${IAPDIR}/src/flash_if.c
IAPSRC += ${IAPDIR}/src/${CHIP_PREFIX}_it.c
IAPSRC += ${IAPDIR}/src/system_${CHIP_PREFIX}.c
IAPSRC += ${IAPDIR}/src/platform_${CHIP_PREFIX}.c
IAPSRC += ${IAPDIR}/src/debug.c
IAPSRC += ${IAPDIR}/src/main.c
IAPSRC += ${IAPDIR}/src/mbfuncboot.c
IAPSRC += ${IAPDIR}/src/crc32.c
IAPSRC += ${IAPDIR}/src/encryption.c

IAPSRC += ${SECDIR}/src/blowfish.c

#List of all Utility files
# TODO: Use custom board files instead
UTILDIR = ${TOPDIR}/Utilities
UTILSRC =
include platform_${PLATFORM}.mk
# Define linker script file here
LDSCRIPT= STM32F373VC_FLASH.ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(LIBSRC) $(IAPSRC) $(UTILSRC) build_version.c


# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC =

# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMSRC = startup_stm32f37x.s


INCDIR =
INCDIR += $(LIBDIR)/Include
INCDIR += $(LIBDIR)/Device/ST/${CHIP}/Include
INCDIR += $(IAPDIR)/inc
INCDIR += $(UTILDIR)/STM32_EVAL/${PLATFORM}
INCDIR += $(UTILDIR)/STM32_EVAL/Common
INCDIR += $(STDPERIPHDIR)/inc
INCDIR += $(TOPDIR)/common/inc
INCDIR += $(SECDIR)/inc
#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU  = cortex-m4

#TRGT = arm-elf-
TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
OD   = $(TRGT)objdump
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# ARM-specific options here
AOPT =

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra

#
# Compiler settings
##############################################################################

##############################################################################
# Start of default section
#

# List all default C defines here, like -D_DEBUG=1
DDEFS = -DSTM32F37X -DUSE_STDPERIPH_DRIVER

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS =

#
# End of default section
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS =

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

#
# End of user defines
##############################################################################

ifeq ($(USE_FPU),yes)
  USE_OPT += -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -fsingle-precision-constant
  DDEFS += -DCORTEX_USE_FPU=TRUE
else
  DDEFS += -DCORTEX_USE_FPU=FALSE
endif

include modbus.mk
include rules.mk

build_version.c: FORCE
	@echo "#include \"bootloader.h\"" > build_version.c
	@echo "#include \"fwheader.h\"" > build_version.c
	@echo "fwInfo __fwinfo = { .magic = FW_MAGIC, .version = \"Bootloader `date +%Y-%b-%d`\" };" >> build_version.c

FORCE: