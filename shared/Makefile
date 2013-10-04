CC = gcc
CFLAGS = -c -g

OBJS = util.o wav.o

all: $(OBJS)

util.o: util.c util.h
wav.o: wav.c wav.h

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o
