#DODEBUG=1
ifdef DODEBUG
CXXFLAGS=-Wall -g -fexceptions -O -pg -static
LDLIBS+=/usr/src/donut/zlib-1.1.3/libz.a
#LDLIBS=-lc_g -lm_g
else
CXXFLAGS=-Wall -g -fexceptions -O2
LDLIBS+=-lz
endif

UUDIR=uulib/
UULIB=$(UUDIR)libuu.a
#if you have libuu on your system already, you might want this instead:
#LDLIBS+=-luu

CXXFLAGS+=-I$(UUDIR)


#do consistancy checking and such.. might be a bit slower..
#CXXFLAGS+=-DDEBUG_CACHE

#if you want to store header cache in text instead of binary.
#text cache files will be slightly larger, but you can view them easier.
#however, gzgets() is _really_ slow, so it takes about 10x longer to load
#CXXFLAGS+=-DTEXT_CACHE


PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/man/man1

.PHONY: all dep depend clean install dist distclean

all: nget

nget: nget.o prot_nntp.o sockstuff.o cache.o file.o misc.o myregex.o $(UULIB)
	g++ $(CFLAGS) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

ifdef UULIB
$(UULIB):
	(cd $(UUDIR); ./configure)
	make -C $(UUDIR) libuu.a
endif


filetest: filetest.o file.o
	g++ $(CFLAGS) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

dep depend:
	g++ $(CFLAGS) $(CXXFLAGS) -MM *.cc > .depend

install: all
	install -s -o root -g bin -m 0755 nget $(BINDIR)
	install -o root -g bin -m 0644 nget.1 $(MANDIR)

dist:
	-make -C $(UUDIR) distclean
	cd ..;tar -czhf nget/distro/nget-`egrep "nget v[0-9.]+ -" nget/nget.cc | sed "s/.*v\([0-9.]\+\).*/\1/"`-withuulib.tar.gz nget/README nget/Changelog nget/COPYING nget/*.cc nget/*.h nget/Makefile nget/nget.1 nget/uulib/
	cd ..;tar -czhf nget/distro/nget-`egrep "nget v[0-9.]+ -" nget/nget.cc | sed "s/.*v\([0-9.]\+\).*/\1/"`.tar.gz nget/README nget/Changelog nget/COPYING nget/*.cc nget/*.h nget/Makefile nget/nget.1
	
clean:
	-rm nget *.o

distclean: clean
	-rm *~
		 

ifeq (.depend,$(wildcard .depend))
include .depend
endif

