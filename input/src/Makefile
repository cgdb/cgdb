CFLAGS=
LDFLAGS=-lcurses -ltgdb

all: input getch

terminfo:terminfo.c
	gcc -c -g -Wall scroller.c -I../..
	gcc -g3 -ggdb -Wall scroller.o terminfo.c -o terminfo $(LDFLAGS) -I ../..

input:input.c input.h
	gcc -g3 -ggdb -Wall input.c -o input $(LDFLAGS) -I ../.. -I ../../tgdb/include -L ../../tgdb/lib

getch:getch.c
	gcc -g3 -ggdb -Wall getch.c -o getch -lcurses

clean:
	rm -f terminfo input getch *.o
