
CC = gcc
CFLAGS = -Wall -O2

TARGET = $(notdir $(shell pwd))

SRC := $(wildcard *.c)
OBJ = $(SRC:c=o)
HEADER := $(wildcard *.h)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f *.o $(TARGET)