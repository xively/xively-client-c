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


		
