CFLAGS=
LDFLAGS=-lcurses -ltgdb

all: input

terminfo:terminfo.c
	gcc -c -g -Wall scroller.c -I../..
	gcc -g3 -ggdb -Wall scroller.o terminfo.c -o terminfo $(LDFLAGS) -I ../..

input:input.c input.h
	gcc -g3 -ggdb -Wall input.c -o input $(LDFLAGS) -I ../.. -I ../../tgdb/include -L ../../tgdb/lib

clean:
	rm -f terminfo input *.o
