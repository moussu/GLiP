PROGRAM = main

# One of "ram" or "flash"
MODE = flash

# One of "hd" or "md" or "cl"
DENSITY = hd

# Source files
SRCS = $(filter %.c,$(shell git ls-files))

# Object files
OBJS = $(notdir $(SRCS:.c=.o)) tasks.o queue.o port.o list.o heap_3.o croutine.o

include ../Makefile.inc

CFLAGS += -DMBLED

DIRS = fakehardware olsr utils watchdog

CFLAGS += $(addprefix -I,$(DIRS))
VPATH += $(DIRS) $(FREERTOS_DIR)

LDFLAGS += -lpthread

toto:
	@echo $(OBJS)
