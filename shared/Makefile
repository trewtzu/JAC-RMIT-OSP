CC = gcc
CFLAGS = -c

OBJS = util.o queue.o wav.o

all: $(OBJS)

util.o: util.c util.h
queue.o: queue.c queue.h
wav.o: wav.c wav.h

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o
