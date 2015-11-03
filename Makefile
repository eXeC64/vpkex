.PHONY: clean

CFLAGS ?= -O2 -Wall -Werror -Wpedantic

TARGET = vpkex
SOURCES = $(wildcard *.c)
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) $(TARGET) $(OBJECTS)
