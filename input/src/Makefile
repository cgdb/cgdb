CFLAGS=
LDFLAGS=-lcurses

all: terminfo

terminfo:terminfo.c
	gcc -g3 -ggdb -Wall terminfo.c -o terminfo $(LDFLAGS)

clean:
	rm -f terminfo *.o
