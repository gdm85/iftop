# Makefile:

#CC = gcc

CFLAGS  += -g -Wall -I/usr/local/include
LDFLAGS += -g 

LDLIBS += -L/usr/local/lib -lpcap -lpthread -lcurses

SRCS =  iftop.c \
        addr_hash.c \
	hash.c \
	ns_hash.c \
	resolver.c \
	ui.c \
	sorted_list.c

OBJS = $(SRCS:.c=.o)

HDRS =  addr_hash.h	

# If you do not have makedepend, you will need to remove references to depend
# and nodepend below.
iftop: depend $(OBJS) Makefile
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS) 

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

clean: nodepend
	rm -f *~ *.o core iftop

tags :
	etags *.c *.h

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)
	touch depend

nodepend:
	makedepend -- --
	rm -f depend
 
# DO NOT DELETE


