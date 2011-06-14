PROGRAM = main

# One of "ram" or "flash"
MODE = flash

# One of "hd" or "md" or "cl"
DENSITY = hd

# Source files
SRCS = $(wildcard *.c)

# Object files
OBJS = $(SRCS:.c=.o)

include ../Makefile.inc

CFLAGS += -DMBLED
