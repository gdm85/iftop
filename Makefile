# Makefile:

#CC = gcc

CFLAGS  += -g -Wall -I/usr/local/include -I/usr/include/pcap
LDFLAGS += -g 

LDLIBS += -L/usr/local/lib -lpcap -lpthread -lcurses -lm

SRCS =  iftop.c \
        addr_hash.c \
	hash.c \
	ns_hash.c \
	resolver.c \
	ui.c \
        util.c \
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

iftop.o: /usr/include/pcap/pcap.h /usr/include/sys/types.h
iftop.o: /usr/include/features.h /usr/include/sys/cdefs.h
iftop.o: /usr/include/gnu/stubs.h /usr/include/bits/types.h
iftop.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
iftop.o: /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h
iftop.o: /usr/include/sys/select.h /usr/include/bits/select.h
iftop.o: /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h
iftop.o: /usr/include/sys/time.h /usr/include/bits/time.h
iftop.o: /usr/include/pcap/net/bpf.h /usr/include/stdio.h
iftop.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
iftop.o: /usr/include/libio.h /usr/include/_G_config.h
iftop.o: /usr/include/bits/stdio_lim.h /usr/include/stdlib.h
iftop.o: /usr/include/alloca.h /usr/include/net/ethernet.h
iftop.o: /usr/include/linux/if_ether.h /usr/include/netinet/ip.h
iftop.o: /usr/include/netinet/in.h /usr/include/limits.h
iftop.o: /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h
iftop.o: /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h
iftop.o: /usr/include/stdint.h /usr/include/bits/wordsize.h
iftop.o: /usr/include/bits/socket.h /usr/include/bits/sockaddr.h
iftop.o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
iftop.o: /usr/include/bits/in.h /usr/include/bits/byteswap.h
iftop.o: /usr/include/pthread.h /usr/include/sched.h
iftop.o: /usr/include/bits/sched.h /usr/include/signal.h
iftop.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
iftop.o: /usr/include/curses.h /usr/include/ncurses/unctrl.h
iftop.o: /usr/include/ncurses/curses.h /usr/include/string.h iftop.h
iftop.o: addr_hash.h /usr/include/sys/socket.h /usr/include/arpa/inet.h
iftop.o: hash.h resolver.h ui.h
addr_hash.o: /usr/include/stdio.h /usr/include/features.h
addr_hash.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
addr_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
addr_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
addr_hash.o: /usr/include/bits/types.h /usr/include/libio.h
addr_hash.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
addr_hash.o: /usr/include/stdlib.h /usr/include/sys/types.h
addr_hash.o: /usr/include/time.h /usr/include/endian.h
addr_hash.o: /usr/include/bits/endian.h /usr/include/sys/select.h
addr_hash.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
addr_hash.o: /usr/include/sys/sysmacros.h /usr/include/alloca.h addr_hash.h
addr_hash.o: /usr/include/sys/socket.h /usr/include/bits/socket.h
addr_hash.o: /usr/include/limits.h /usr/include/bits/posix1_lim.h
addr_hash.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
addr_hash.o: /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h
addr_hash.o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
addr_hash.o: /usr/include/netinet/in.h /usr/include/stdint.h
addr_hash.o: /usr/include/bits/wordsize.h /usr/include/bits/in.h
addr_hash.o: /usr/include/bits/byteswap.h /usr/include/arpa/inet.h hash.h
addr_hash.o: iftop.h
hash.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
hash.o: /usr/include/gnu/stubs.h
hash.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
hash.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
hash.o: /usr/include/bits/types.h /usr/include/libio.h
hash.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
hash.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
hash.o: /usr/include/endian.h /usr/include/bits/endian.h
hash.o: /usr/include/sys/select.h /usr/include/bits/select.h
hash.o: /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h
hash.o: /usr/include/alloca.h hash.h iftop.h
ns_hash.o: /usr/include/stdio.h /usr/include/features.h
ns_hash.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
ns_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
ns_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
ns_hash.o: /usr/include/bits/types.h /usr/include/libio.h
ns_hash.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
ns_hash.o: /usr/include/stdlib.h /usr/include/sys/types.h /usr/include/time.h
ns_hash.o: /usr/include/endian.h /usr/include/bits/endian.h
ns_hash.o: /usr/include/sys/select.h /usr/include/bits/select.h
ns_hash.o: /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h
ns_hash.o: /usr/include/alloca.h /usr/include/sys/socket.h
ns_hash.o: /usr/include/bits/socket.h /usr/include/limits.h
ns_hash.o: /usr/include/bits/posix1_lim.h /usr/include/bits/local_lim.h
ns_hash.o: /usr/include/linux/limits.h /usr/include/bits/posix2_lim.h
ns_hash.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
ns_hash.o: /usr/include/asm/sockios.h /usr/include/netinet/in.h
ns_hash.o: /usr/include/stdint.h /usr/include/bits/wordsize.h
ns_hash.o: /usr/include/bits/in.h /usr/include/bits/byteswap.h
ns_hash.o: /usr/include/arpa/inet.h ns_hash.h hash.h iftop.h
resolver.o: /usr/include/netinet/in.h /usr/include/features.h
resolver.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
resolver.o: /usr/include/limits.h /usr/include/bits/posix1_lim.h
resolver.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
resolver.o: /usr/include/bits/posix2_lim.h /usr/include/stdint.h
resolver.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
resolver.o: /usr/include/bits/wordsize.h /usr/include/sys/types.h
resolver.o: /usr/include/bits/types.h /usr/include/time.h
resolver.o: /usr/include/endian.h /usr/include/bits/endian.h
resolver.o: /usr/include/sys/select.h /usr/include/bits/select.h
resolver.o: /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h
resolver.o: /usr/include/bits/socket.h /usr/include/bits/sockaddr.h
resolver.o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
resolver.o: /usr/include/bits/in.h /usr/include/bits/byteswap.h
resolver.o: /usr/include/sys/socket.h /usr/include/arpa/inet.h
resolver.o: /usr/include/pthread.h /usr/include/sched.h
resolver.o: /usr/include/bits/sched.h /usr/include/signal.h
resolver.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
resolver.o: /usr/include/stdio.h
resolver.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
resolver.o: /usr/include/libio.h /usr/include/_G_config.h
resolver.o: /usr/include/bits/stdio_lim.h /usr/include/stdlib.h
resolver.o: /usr/include/alloca.h /usr/include/netdb.h
resolver.o: /usr/include/rpc/netdb.h /usr/include/errno.h
resolver.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
resolver.o: /usr/include/asm/errno.h /usr/include/string.h ns_hash.h hash.h
resolver.o: iftop.h
ui.o: /usr/include/curses.h /usr/include/stdio.h /usr/include/features.h
ui.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
ui.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
ui.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
ui.o: /usr/include/bits/types.h /usr/include/libio.h /usr/include/_G_config.h
ui.o: /usr/include/bits/stdio_lim.h /usr/include/ncurses/unctrl.h
ui.o: /usr/include/ncurses/curses.h /usr/include/string.h /usr/include/math.h
ui.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
ui.o: /usr/include/bits/mathcalls.h
ui.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/float.h
ui.o: /usr/include/pthread.h /usr/include/sched.h /usr/include/time.h
ui.o: /usr/include/bits/sched.h /usr/include/signal.h
ui.o: /usr/include/bits/sigset.h /usr/include/bits/pthreadtypes.h
ui.o: /usr/include/bits/sigthread.h /usr/include/stdlib.h
ui.o: /usr/include/sys/types.h /usr/include/endian.h
ui.o: /usr/include/bits/endian.h /usr/include/sys/select.h
ui.o: /usr/include/bits/select.h /usr/include/sys/sysmacros.h
ui.o: /usr/include/alloca.h /usr/include/sys/time.h /usr/include/bits/time.h
ui.o: addr_hash.h /usr/include/sys/socket.h /usr/include/bits/socket.h
ui.o: /usr/include/limits.h /usr/include/bits/posix1_lim.h
ui.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
ui.o: /usr/include/bits/posix2_lim.h /usr/include/bits/sockaddr.h
ui.o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
ui.o: /usr/include/netinet/in.h /usr/include/stdint.h
ui.o: /usr/include/bits/wordsize.h /usr/include/bits/in.h
ui.o: /usr/include/bits/byteswap.h /usr/include/arpa/inet.h hash.h iftop.h
ui.o: resolver.h sorted_list.h
util.o: /usr/include/sys/types.h /usr/include/features.h
util.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
util.o: /usr/include/bits/types.h
util.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
util.o: /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h
util.o: /usr/include/sys/select.h /usr/include/bits/select.h
util.o: /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h
util.o: /usr/include/errno.h /usr/include/bits/errno.h
util.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
util.o: /usr/include/stdio.h
util.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
util.o: /usr/include/libio.h /usr/include/_G_config.h
util.o: /usr/include/bits/stdio_lim.h /usr/include/stdlib.h
util.o: /usr/include/alloca.h /usr/include/string.h /usr/include/unistd.h
util.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
util.o: /usr/include/getopt.h iftop.h
sorted_list.o: /usr/include/stdlib.h /usr/include/features.h
sorted_list.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
sorted_list.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
sorted_list.o: /usr/include/sys/types.h /usr/include/bits/types.h
sorted_list.o: /usr/include/time.h /usr/include/endian.h
sorted_list.o: /usr/include/bits/endian.h /usr/include/sys/select.h
sorted_list.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
sorted_list.o: /usr/include/sys/sysmacros.h /usr/include/alloca.h
sorted_list.o: /usr/include/stdio.h
sorted_list.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
sorted_list.o: /usr/include/libio.h /usr/include/_G_config.h
sorted_list.o: /usr/include/bits/stdio_lim.h sorted_list.h iftop.h
