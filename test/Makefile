

CC = gcc
INCLUDE_DIRS += -I../xbox/include
LD_LIBRARY_PATH += -L../xbox/lib
LDFLAGS += -lxbox

SRCS=$(wildcard *.c)
EXECUTABLES=$(SRCS:.c=)

all: $(EXECUTABLES)

%: %.c
	$(CC) $(INCLUDE_DIRS) $(LD_LIBRARY_PATH) $^ $(LDFLAGS) -o $@

clean:
	rm $(EXECUTABLES)