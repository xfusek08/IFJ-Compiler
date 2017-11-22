
CFLAGS = -std=c99 -Wall -Wextra -Werror
EXECUTABLE = ifjcompile
SOURCES = $(wildcard *.c) $(wildcard */*.c)
OBJS = $(patsubst %.c,%.o,$(SOURCES))

.PHONY: clean

all: $(EXECUTABLE) clean

#setting debug flags
debug: CFLAGS += -g -DDEBUG #-DST_DEBUG
debug: $(EXECUTABLE) clean

test: debug $(EXECUTABLE)
	cat testcode.ifj | ./ifjcompile

%.o : %.c
	gcc $(CFLAGS) -c $< -o $@

$(EXECUTABLE): $(OBJS)
	gcc $(CFLAGS) -o $@ $^ -lm

clean:
	-rm *.o */*.o
