#
# Makefile:
# Makefile for iftop.
#
# $Id$
#

VERSION = 0.10pre1

# C compiler to use.
#CC = gcc

# Give the location of pcap.h here:
CFLAGS += -I/usr/include/pcap 
# CFLAGS += -I/usr/pkg/include
# CFLAGS += -pg -a

# Give the location of libpcap here if it's not in one of the standard
# directories:
# LDFLAGS += -L/usr/local/lib
# LDFLAGS += -pg -a

#
# Uncomment to use libresolv
#
#CFLAGS += -DUSELIBRESOLV 

# This may be needed to use libresolv on Linux.
#LDLIBS += /usr/lib/libresolv.a


# PREFIX specifies the base directory for the installation.
PREFIX = /usr/local
#PREFIX = /software

# BINDIR is where the binary lives. No leading /.
BINDIR = sbin

# MANDIR is where the manual page goes.
MANDIR = man
#MANDIR = share/man     # FHS-ish

# You shouldn't need to change anything below this point.
CFLAGS  += -g -Wall "-DIFTOP_VERSION=\"$(VERSION)\""
LDFLAGS += -g -pthread
LDLIBS += -lpcap -lcurses -lm


SRCS = iftop.c addr_hash.c hash.c ns_hash.c resolver.c ui.c util.c sorted_list.c\
       options.c serv_hash.c threadprof.c
HDRS = addr_hash.h hash.h iftop.h ns_hash.h resolver.h sorted_list.h ui.h options.h sll.h\
       serv_hash.h threadprof.h ether.h ip.h tcp.h
TXTS = README CHANGES INSTALL TODO iftop.8 COPYING
SPECFILE = iftop.spec iftop.spec.in

OBJS = $(SRCS:.c=.o)

iftop: $(OBJS) Makefile
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) 

install: iftop
	install -D iftop   $(PREFIX)/$(BINDIR)/iftop
	install -D iftop.8 $(PREFIX)/$(MANDIR)/man8/iftop.8

uninstall:
	rm -f $(PREFIX)/$(BINDIR)/iftop $(PREFIX)/$(MANDIR)/man8/iftop.8

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *~ *.o core iftop iftop.spec

tarball: depend $(SRCS) $(HDRS) $(TXTS) $(SPECFILE)
	mkdir iftop-$(VERSION)
	set -e ; for i in Makefile depend $(SRCS) $(HDRS) $(TXTS) $(SPECFILE) ; do cp $$i iftop-$(VERSION)/$$i ; done
	tar cvf - iftop-$(VERSION) | gzip --best > iftop-$(VERSION).tar.gz
	rm -rf iftop-$(VERSION)

tags :
	etags *.c *.h

depend: $(SRCS)
	$(CPP) $(CFLAGS)  -MM $(SRCS) > depend

nodepend:
	rm -f depend

iftop.spec: iftop.spec.in Makefile
	sed 's/__VERSION__/$(VERSION)/' < iftop.spec.in > iftop.spec
  
        
include depend
