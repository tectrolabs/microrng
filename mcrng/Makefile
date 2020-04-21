CC=gcc
SWRNG_FLAGS=-D_FILE_OFFSET_BITS=64
CFLAGS= -lusb-1.0 -O2 -Wall -Wextra
CPPFLAGS=-lstdc++
CFLAGS_THREAD=-lpthread

PREFIX = $(DESTDIR)/usr/local
BINDIR = $(PREFIX)/bin


MCDIAG = mcdiag
SAMPLE = sample

all: $(MCDIAG) $(SAMPLE)

$(MCDIAG): mcdiag.cpp
	$(CC) mcdiag.cpp MicroRngSPI.cpp -o $(MCDIAG) $(CFLAGS) -lm $(CPPFLAGS)

$(SAMPLE): sample.cpp
	$(CC) sample.cpp MicroRngSPI.cpp -o $(SAMPLE) $(CFLAGS) -lm $(CPPFLAGS)
clean:
	rm -f *.o ; rm $(MCDIAG) $(SAMPLE)

install:
	install $(MCDIAG) $(BINDIR)/$(MCDIAG)

uninstall:
	rm $(BINDIR)/$(MCDIAG)
