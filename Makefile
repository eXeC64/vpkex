.PHONY: clean

CFLAGS ?= -O2 -Wall -Werror -Wpedantic

TARGET = vpkex
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:%.c=%.o)

$(TARGET): $(OBJECTS)

clean:
	$(RM) $(TARGET) $(OBJECTS)
