
CC = gcc
CFLAGS = -Wall -Wunused -Werror -Wformat-security

SRC := $(wildcard *.c)
OBJ = $(SRC:c=o)
EXE = $(OBJ:%.o=%)

ifeq ($(MAKECMDGOALS),debug)
CFLAGS+=-g
endif

all: $(EXE)

debug: all

$(EXE): %: %.o
	$(CC) $< -o $@

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(EXE) $(OBJ)