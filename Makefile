all: PC

CFLAGS=-I/usr/include/nptl -D_REENTRANT
LDFLAGS=-L/usr/lib/nptl -pthread

pc.o: pc.h

clean:
	rm -f PC
