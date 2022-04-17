PROG            = ifcomp unit_tests
CC              = gcc-11
CFLAGS		= -I/usr/local/include #-O3 -Wall -Werror
LIBCMOCKA       = -L/usr/local/lib -lcmocka

all:		$(PROG)

test:           unit_tests
		./unit_tests

clean:
		rm -f $(PROG) *.o

ifcomp:         main.o ifcomp.o
		$(CC) $(LDFLAGS) main.o ifcomp.o -o $@

unit_tests:     unit_tests.o ifcomp.o
		$(CC) $(LDFLAGS) unit_tests.o ifcomp.o $(LIBCMOCKA) -o $@
