EXE = server

OBJS = server.o fileio.o queue.o ../shared/util.o ../shared/wav.o

CC = gcc
CFLAGS = -c -g
LDFLAGS = -lpthread -lasound

all: $(EXE)

$(EXE): shared $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXE) $(OBJS)

server.o: server.c
fileio.o: fileio.c fileio.h
queue.o: queue.c queue.h

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: shared

shared:
	$(MAKE) -C ../shared

clean:
	rm -f *.o $(EXE)
