#
# Makefile.am:
# Automake file for iftop.
#
# I don't understand this stuff, so I just stole it from the tpop3d
# distribution. That means that, really, Mark Longair should take credit for
# it.
#   -- Chris Lightfoot
#
# $Id$
#

sbin_PROGRAMS = iftop

iftop_SOURCES = addr_hash.c edline.c hash.c iftop.c ns_hash.c \
                 options.c resolver.c screenfilter.c serv_hash.c \
                 sorted_list.c threadprof.c ui.c util.c

noinst_HEADERS = addr_hash.h ether.h ethertype.h extract.h hash.h iftop.h \
                 integers.h ip.h llc.h ns_hash.h options.h resolver.h \
                 screenfilter.h serv_hash.h sll.h sorted_list.h tcp.h \
                 threadprof.h token.h ui.h

man_MANS = iftop.8

SUBDIRS = config

iftop.cat: iftop.8
	(echo -e ".pl 1100i" ; cat iftop.8 ; echo ".pl \n(nlu+10") | groff -Tascii -man > iftop.cat

## These need to be distributed along with configure:

EXTRA_DIST = bootstrap README CHANGES COPYING INSTALL TODO Makefile.OLD \
             $(man_MANS) iftop.cat
             
MAINTERCLEANFILES = Makefile.in aclocal.m4 configure configuration.h.in \
                    stamp-h.in

ACLOCAL = aclocal -I @ac_aux_dir@

