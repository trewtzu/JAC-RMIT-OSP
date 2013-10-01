EXE = server

OBJS = server.o fileio.o ../shared/util.o ../shared/queue.o

CC = gcc
CFLAGS = -c
LDFLAGS = -lpthread

all: $(EXE)

$(EXE): shared $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXE) $(OBJS)

server.o: server.c
fileio.o: fileio.c fileio.h

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: shared

shared:
	$(MAKE) -C ../shared

clean:
	rm -f *.o $(EXE)
