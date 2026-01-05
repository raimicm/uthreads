# compiler
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude

# assembler
AS = $(CC)
ASFLAGS =

# archiver to generate .a files 
AR = ar
ARFLAGS = rcs

# library name
LIB = uthreads

.PHONY: library all examples clean

library: lib/libuthreads.a

all: lib examples

examples: join_example detach_example

lib:
	@mkdir -p lib 

build:
	@mkdir -p build

%: examples/%.c lib/lib$(LIB).a | build
	$(CC) $(CFLAGS) $< -Llib -l$(LIB) -o build/$@

lib/libuthreads.a: build/uthread.o build/context_switch.o build/thread_queue.o | lib build
	$(AR) $(ARFLAGS) $@ $^

build/uthread.o: src/uthread.c include/uthread.h include/thread.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/context_switch.o: src/context_switch.S include/context_switch.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/thread_queue.o: src/thread_queue.c include/thread_queue.h include/thread.h | build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f build/* && rm -f lib/libuthreads.a

