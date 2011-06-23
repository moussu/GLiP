# This file will be included in project's Makefile
#
# The Makefile has to define:
#   DENSITY: md or hd or cl
#   MODE:    flash or ram
#   OBJS:    the list of object files
#   PROGRAM: the final program name

MKDIR_P = mkdir -p

CC = ccache gcc
CFLAGS = -std=c99 $(ARCHFLAGS)           \
	 -DDEBUG_BUILD=1 -DUSE_STDIO=1   \
	 $(INCCFLAGS) $(OPTFLAGS)        \
	 -Wall -Werror -fmessage-length=0\
	 -pthread -lrt -Wno-pointer-sign \
	 $(DEBUGFLAGS)
INCCFLAGS = -I. -I$(FREERTOS_INCDIR) -I$(FREERTOS_PLATDIR)  \
	    -I$(FW_INCDIR) -I$(STM32_DIR) -I$(FIRMWARE_DIR) \
	    $(PROGRAM_MODE) -include FreeRTOSConfig.h

ARCHFLAGS = -D__GCC_POSIX__=1
OPTFLAGS = -O2 -g

ifeq ($(MODE),flash)
PROGRAM_MODE=-DPROGRAM_MODE_FLASH
else
PROGRAM_MODE=-DPROGRAM_MODE_RAM
endif

ifeq ($(DENSITY),hd)
INCCFLAGS += -DSTM32F10X_HD
else ifeq ($(DENSITY),md)
INCCFLAGS += -DSTM32F10X_MD
else
INCCFLAGS += -DSTM32F10X_CL
endif

STARTUP_OBJECT = startup_stm32f10x_$(DENSITY).o
LINKER_SCRIPT = stm32f10x_$(MODE)_$(DENSITY).ld

OBJS +=

AS = as
ASFLAGS = $(ARCHFLAGS)

LD = gcc
LDFLAGS = $(ARCHFLAGS) -lrt

OBJCOPY = objcopy
OBJDUMP = objdump

FREERTOS_DIR = ../Source
FREERTOS_INCDIR = $(FREERTOS_DIR)/include
FREERTOS_PLATDIR = $(FREERTOS_DIR)/portable/GCC/Posix
FREERTOS_MEMDIR = $(FREERTOS_DIR)/portable/MemMang

FIRMWARE_DIR = ../firmware

FW_DIR = $(STM32_DIR)/STM32_USB-FS-Device_Lib_V3.1.0/Libraries
FW_CORE_DIR = $(FW_DIR)/CMSIS/Core/CM3
FW_STARTUP_DIR = $(FW_CORE_DIR)/startup/gcc
FW_STDDRIVER_DIR = $(FW_DIR)/STM32F10x_StdPeriph_Driver
FW_STDDRIVER_SRCDIR = $(FW_STDDRIVER_DIR)/src
FW_STDDRIVER_INCDIR = $(FW_STDDRIVER_DIR)/inc
FW_USB_DIR = $(FW_DIR)/Libraries/STM32_USB-FS-Device_Driver
FW_USB_SRCDIR = $(FW_USB_DIR)/src
FW_USB_INCDIR = $(FW_USB_DIR)/inc
INCCFLAGS += -I$(FW_CORE_DIR) -I$(FW_STARTUP_DIR) -I$(FW_STDDRIVER_INCDIR) \
	-I$(FW_USB_INCDIR)
VPATH = $(FW_CORE_DIR) $(FW_STARTUP_DIR) $(FW_STDDRIVER_SRCDIR) \
	$(FW_USB_SRCDIR) $(FREERTOS_DIR) $(FREERTOS_PLATDIR) $(FREERTOS_MEMDIR) \
	$(STM32_DIR) $(FIRMWARE_DIR)

FW_SRCDIR = $(FW_DIR)/src
FW_INCDIR = $(FW_DIR)/inc

STM32_DIR = ../STM32

all:: $(PROGRAM)

clean::
	$(RM) $(PROGRAM) *.o $(PROGRAM).map $(PROGRAM).bin *~ \#*\#
	@$(RM) -r .deps

$(PROGRAM): $(OBJS)
	$(LD) $(LDFLAGS) -o $(PROGRAM) $(OBJS)

info::
	$(OBJDUMP) --section-headers $(PROGRAM)

%.bin: %
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c -MMD -MP $<
	@$(MKDIR_P) .deps
	@mv $(patsubst %.o,%.d,$@) .deps/

%.o: %.raw
	$(OBJCOPY) -B arm -I binary -O elf32-littlearm \
	--rename-section .data=.rodata,alloc,load,readonly,data,contents $< $@

*.o: Makefile ../Makefile.inc

DEPS := $(patsubst %.o,.deps/%.d,$(OBJS))

-include .deps/*.d
