CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99
LFLAGS = 
LIBS = 
SOURCES = hostd.c utility.c queue.c sigtrap.c test.c
OBJECTS = $(subst .c,.o,$(SOURCES))
EXE = process hostd test
.PHONY: clean help

.PHONY: debug
debug: CFLAGS += -O0 -g3
debug: $(EXE)

process : sigtrap.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@ 

hostd : hostd.o utility.o queue.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@ 

test : test.o utility.o queue.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@ 

%.o : %.c
	$(CC) $(CFLAGS) -c $< 

all : $(EXE)

clean:
	rm -f $(OBJECTS) $(EXE) *~

help:
	@echo "Valid targets:"
	@echo "  all:    generates all binary files"
	@echo "  debug:  generates debug binary files"
	@echo "  clean:  removes .o and .exe files"
