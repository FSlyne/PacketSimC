# http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

CC=gcc
CFLAGS=-Wall  -std=c99 -g
LDFLAGS=-lm
DEPS=

all: packetsim 

packetsim: packetsim.c $(DEPS)
	$(CC) $(CFLAGS) packetsim.c  scheduler.c packet.c packetgen.c pbuffer.c queue.c store.c aqm.c dba.c socket.c $(LDFLAGS) -o packetsim

clean:
	rm packetsim
