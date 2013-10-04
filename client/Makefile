EXE = client

OBJS = client.o selectionController.o ../shared/util.o ../shared/queue.o ../shared/wav.o

CC = gcc
CFLAGS = -c
LDFLAGS = -lpthread -lasound

all: $(EXE)

$(EXE): shared $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXE) $(OBJS)

client.o: client.c
selectionController.o: selectionController.c selectionController.h

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: shared

shared:
	$(MAKE) -C ../shared

clean:
	rm -f *.o $(EXE)
