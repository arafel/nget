CXXFLAGS=-Wall -g -fexceptions -O

#comment these two lines if you don't want readline
#CXXFLAGS+=-DWANT_READLINE
#LIBS+=-lreadline -lhistory -lcurses
#comment this if you are using readline >= 4.0
#CXXFLAGS+=-DOLD_READLINE

LDLIBS=-lz

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/man/man1

all: nget

nget: nget.o prot_nntp.o sockstuff.o cache.o file.o misc.o
	g++ $(CFLAGS) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

filetest: filetest.o file.o
	g++ $(CFLAGS) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

dep depend:
	g++ -MM *.cc > .depend

install: all
	install -s -o root -g bin -m 0755 nget $(BINDIR)
	install -o root -g bin -m 0644 nget.1 $(MANDIR)

dist:
	cd ..;tar -czhf nget/distro/nget-`egrep "nget v[0-9.]+ -" nget/nget.cc | sed "s/.*v\([0-9.]\+\).*/\1/"`.tar.gz nget/README nget/Changelog nget/COPYING nget/*.cc nget/*.h nget/Makefile nget/nget.1
	
clean:
	rm nget *.o *.*~
		 

ifeq (.depend,$(wildcard .depend))
include .depend
endif

