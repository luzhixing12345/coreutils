
CC = gcc
CFLAGS = -Wall -O2

TARGET = $(notdir $(shell pwd))

SRC := $(wildcard *.c)
OBJ = $(SRC:c=o)
HEADER := $(wildcard *.h)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f *.o $(TARGET)