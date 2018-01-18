# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

CC?=clang
CFLAGS?=-O2 -Wall -std=c99
INCLUDES?=-Isrc/
SOURCES=$(wildcard *.c src/*.c)
OBJECTS=$(SOURCES:.c=.o)

EXECUTABLE=stack_alloc_C_impl

all: $(EXECUTABLE)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm $(EXECUTABLE) $(OBJECTS)


		
