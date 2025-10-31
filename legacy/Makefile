PROG            = ifcomp
CFLAGS		= -g -O0 -Wall -Werror
LDFLAGS         = -g

all:		$(PROG)

clean:
		rm -f $(PROG) *.o *.input *.output

ifcomp:         main.o ifcomp.o
		$(CC) $(LDFLAGS) main.o ifcomp.o -o $@
