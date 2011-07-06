PROGRAM = main

# Source files
SRCS = $(filter %.c,$(shell git ls-files))

# Object files
OBJS = $(notdir $(SRCS:.c=.o)) tasks.o queue.o 		\
	port.o list.o heap_3.o croutine.o

include ../Makefile.inc

DIRS = olsr utils watchdog comm AsyncIO charmap image
CFLAGS += -D_GNU_SOURCE -DDEBUG -DWARNINGS -DERRORS -include debug.h
CFLAGS += $(addprefix -I,$(DIRS))
VPATH += $(DIRS)

LDFLAGS += -lpthread

debug.h: dashes.h
dashes.h: dashes.py
	./$< 20 > $@

olsr/olsr_packet.h: mtu.h
mtu.h: mtu.py
	./$< $$(../scripts/mtu) > $@.tmp
	mv $@.tmp $@

charmap/charmap.c::
	$(MAKE) -C charmap
