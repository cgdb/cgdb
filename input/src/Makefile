CFLAGS=
LDFLAGS=-lcurses

all: terminfo

terminfo:terminfo.c
	gcc -c -g -Wall scroller.c -I../..
	gcc -g3 -ggdb -Wall scroller.o terminfo.c -o terminfo $(LDFLAGS) -I ../..

clean:
	rm -f terminfo *.o
