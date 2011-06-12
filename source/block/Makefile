PROGRAM = main

# One of "ram" or "flash"
MODE = flash

# One of "hd" or "md" or "cl"
DENSITY = hd

# Object files
OBJS = routing.o


include ../Makefile.inc

CFLAGS += -DMBLED


CFLAGS += -I./inc
CFLAGS += -I./inc/utils

VPATH += ./src
