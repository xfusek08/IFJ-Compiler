
CFLAGS = -std=c99 -Wall -Wextra -Werror
EXECUTABLE = ifjcompile
SOURCES = $(wildcard *.c) $(wildcard */*.c)
OBJS = $(patsubst %.c,%.o,$(SOURCES))

.PHONY: clean

all: $(EXECUTABLE) clean

%.o : %.c
	gcc $(CFLAGS) -c $< -o $@

$(EXECUTABLE): $(OBJS)
	gcc $(CFLAGS) -o $@ $^

clean:
	-rm *.o */*.o

