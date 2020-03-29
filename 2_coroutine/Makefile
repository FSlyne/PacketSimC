# http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

CC=gcc
# https://www.avrfreaks.net/forum/avoiding-unused-set-warning
CFLAGS=-Wall  -std=c99 -g -Wno-unused-but-set-variable
LDFLAGS=-lm
DEPS=

all: packetsim 

packetsim: packetsim.c $(DEPS)
	$(CC) $(CFLAGS) packetsim.c  scheduler.c packet.c packetgen.c pbuffer.c queue.c store.c aqm.c dba.c socket.c $(LDFLAGS) -o packetsim

clean:
	rm packetsim
