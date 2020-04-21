CC=gcc
SWRNG_FLAGS=-D_FILE_OFFSET_BITS=64
CFLAGS= -O2 -Wall -Wextra
CPPFLAGS=-lstdc++
CFLAGS_THREAD=-lpthread

PREFIX = $(DESTDIR)/usr/local
BINDIR = $(PREFIX)/bin


MCDIAG = mcdiag
SAMPLE = sample
MCRNG = mcrng

all: $(MCDIAG) $(SAMPLE) $(MCRNG)

$(MCRNG): mcrng.cpp
	$(CC) mcrng.cpp MicroRngSPI.cpp -o $(MCRNG) $(CFLAGS) -lm $(CPPFLAGS)

$(MCDIAG): mcdiag.cpp
	$(CC) mcdiag.cpp MicroRngSPI.cpp -o $(MCDIAG) $(CFLAGS) -lm $(CPPFLAGS)

$(SAMPLE): sample.cpp
	$(CC) sample.cpp MicroRngSPI.cpp -o $(SAMPLE) $(CFLAGS) -lm $(CPPFLAGS)

clean:
	rm -f *.o ; rm $(MCDIAG) $(SAMPLE) $(MCRNG)

install:
	install $(MCDIAG) $(BINDIR)/$(MCDIAG)
	install $(MCRNG) $(BINDIR)/$(MCRNG)

uninstall:
	rm $(BINDIR)/$(MCDIAG) $(BINDIR)/$(MCRNG)
