# http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

CC=gcc
CFLAGS=-Wall  -std=c99 -g
LDFLAGS=-lm
DEPS=

all: packetsim 

packetsim: packetsim.c $(DEPS)
	$(CC) $(CFLAGS) packetsim.c scheduler.c pbuffer.c dba.c aqm.c queue.c packet.c packetgen.c lock.c $(LDFLAGS) -o packetsim

clean:
	rm packetsim
