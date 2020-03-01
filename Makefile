# http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

CC=gcc
CFLAGS=-Wall  -std=c99 -g
LDFLAGS=
DEPS=

all: packetsim 

packetsim: packetsim.c $(DEPS)
	$(CC) $(CFLAGS) packetsim.c cbuffer.c -o packetsim $(LD_FLAGS)

clean:
	rm packetsim
