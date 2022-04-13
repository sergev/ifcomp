PROG            = ifcomp
CC              = gcc-11
CFLAGS		= #-O3 -Wall -Werror

all:		$(PROG)

clean:
		rm -f $(PROG) *.o

ifcomp:         ifcomp.o
		$(CC) $(LDFLAGS) $< -o $@
