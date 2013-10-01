CC = gcc
CFLAGS = -c

OBJS = util.o queue.o

all: $(OBJS)

util.o: util.c util.h
queue.o: queue.c queue.h

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o
