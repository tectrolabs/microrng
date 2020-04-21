CC=gcc
SWRNG_FLAGS=-D_FILE_OFFSET_BITS=64
CFLAGS= -lusb-1.0 -O2 -Wall -Wextra
CPPFLAGS=-lstdc++
CFLAGS_THREAD=-lpthread

PREFIX = $(DESTDIR)/usr/local
BINDIR = $(PREFIX)/bin


MCDIAG = mcdiag

all: $(MCDIAG) 

$(MCDIAG): mcdiag.cpp
	$(CC) mcdiag.cpp MicroRngSPI.cpp -o $(MCDIAG) $(CFLAGS) -lm $(CPPFLAGS)

clean:
	rm -f *.o ; rm $(MCDIAG)

install:
	install $(MCDIAG) $(BINDIR)/$(MCDIAG)

uninstall:
	rm $(BINDIR)/$(MCDIAG)
