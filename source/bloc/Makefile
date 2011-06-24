PROGRAM = main

# Source files
SRCS = $(filter %.c,$(shell git ls-files))

# Object files
OBJS = $(notdir $(SRCS:.c=.o)) tasks.o queue.o port.o list.o heap_3.o croutine.o

include ../Makefile.inc

DIRS = olsr utils watchdog comm AsyncIO
CFLAGS += -D_GNU_SOURCE -DDEBUG -include debug.h
CFLAGS += $(addprefix -I,$(DIRS))
VPATH += $(DIRS)

LDFLAGS += -lpthread

debug.h: dashes.h
dashes.h: dashes.py
	./$< 20 > $@
