CC = gcc
CFLAGS = -ansi -Wall -pedantic -c
LDFLAGS = -lasound

EXE = sample
OBJECTS = sample.o wav.o util.o

all: $(EXE)

$(EXE): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(EXE) $(OBJECTS)

sample.o: sampleWavUsage.c util.o wav.o
	$(CC) $(CFLAGS) -o $@ $<

wav.o: ../wav.c ../wav.h util.o
	$(CC) $(CFLAGS) -o $@ $<

util.o: ../util.c ../util.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o $(EXE) core
