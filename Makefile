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
iftop.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
iftop.o: /usr/include/time.h /usr/include/sys/time.h /usr/include/bits/time.h
iftop.o: /usr/include/sys/select.h /usr/include/bits/select.h
iftop.o: /usr/include/bits/sigset.h /usr/include/pcap/net/bpf.h
iftop.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
iftop.o: /usr/include/wchar.h /usr/include/bits/wchar.h /usr/include/gconv.h
iftop.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
iftop.o: /usr/include/bits/stdio_lim.h /usr/include/stdlib.h
iftop.o: /usr/include/sys/ioctl.h /usr/include/bits/ioctls.h
iftop.o: /usr/include/asm/ioctls.h /usr/include/asm/ioctl.h
iftop.o: /usr/include/bits/ioctl-types.h /usr/include/sys/ttydefaults.h
iftop.o: /usr/include/net/if.h /usr/include/net/ethernet.h
iftop.o: /usr/include/linux/if_ether.h /usr/include/netinet/ip.h
iftop.o: /usr/include/netinet/in.h /usr/include/stdint.h
iftop.o: /usr/include/bits/wordsize.h /usr/include/bits/socket.h
iftop.o: /usr/include/limits.h
iftop.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
iftop.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
iftop.o: /usr/include/asm/sockios.h /usr/include/bits/in.h
iftop.o: /usr/include/endian.h /usr/include/bits/endian.h
iftop.o: /usr/include/bits/byteswap.h /usr/include/pthread.h
iftop.o: /usr/include/sched.h /usr/include/bits/sched.h /usr/include/signal.h
iftop.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/initspin.h
iftop.o: /usr/include/bits/sigthread.h /usr/include/curses.h
iftop.o: /usr/include/ncurses/unctrl.h /usr/include/ncurses/curses.h
iftop.o: /usr/include/string.h /usr/include/unistd.h
iftop.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h iftop.h
iftop.o: addr_hash.h /usr/include/sys/socket.h /usr/include/sys/uio.h
iftop.o: /usr/include/bits/uio.h /usr/include/arpa/inet.h hash.h resolver.h
iftop.o: ui.h
addr_hash.o: /usr/include/stdio.h /usr/include/features.h
addr_hash.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
addr_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
addr_hash.o: /usr/include/bits/types.h /usr/include/libio.h
addr_hash.o: /usr/include/_G_config.h /usr/include/wchar.h
addr_hash.o: /usr/include/bits/wchar.h /usr/include/gconv.h
addr_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
addr_hash.o: /usr/include/bits/stdio_lim.h /usr/include/stdlib.h addr_hash.h
addr_hash.o: /usr/include/sys/socket.h /usr/include/sys/uio.h
addr_hash.o: /usr/include/sys/types.h /usr/include/time.h
addr_hash.o: /usr/include/bits/uio.h /usr/include/bits/socket.h
addr_hash.o: /usr/include/limits.h
addr_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
addr_hash.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
addr_hash.o: /usr/include/asm/sockios.h /usr/include/netinet/in.h
addr_hash.o: /usr/include/stdint.h /usr/include/bits/wordsize.h
addr_hash.o: /usr/include/bits/in.h /usr/include/endian.h
addr_hash.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
addr_hash.o: /usr/include/arpa/inet.h hash.h iftop.h
hash.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
hash.o: /usr/include/gnu/stubs.h
hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
hash.o: /usr/include/bits/types.h /usr/include/libio.h
hash.o: /usr/include/_G_config.h /usr/include/wchar.h
hash.o: /usr/include/bits/wchar.h /usr/include/gconv.h
hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
hash.o: /usr/include/bits/stdio_lim.h /usr/include/stdlib.h hash.h iftop.h
ns_hash.o: /usr/include/stdio.h /usr/include/features.h
ns_hash.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
ns_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
ns_hash.o: /usr/include/bits/types.h /usr/include/libio.h
ns_hash.o: /usr/include/_G_config.h /usr/include/wchar.h
ns_hash.o: /usr/include/bits/wchar.h /usr/include/gconv.h
ns_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
ns_hash.o: /usr/include/bits/stdio_lim.h /usr/include/stdlib.h
ns_hash.o: /usr/include/sys/socket.h /usr/include/sys/uio.h
ns_hash.o: /usr/include/sys/types.h /usr/include/time.h
ns_hash.o: /usr/include/bits/uio.h /usr/include/bits/socket.h
ns_hash.o: /usr/include/limits.h
ns_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
ns_hash.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
ns_hash.o: /usr/include/asm/sockios.h /usr/include/netinet/in.h
ns_hash.o: /usr/include/stdint.h /usr/include/bits/wordsize.h
ns_hash.o: /usr/include/bits/in.h /usr/include/endian.h
ns_hash.o: /usr/include/bits/endian.h /usr/include/bits/byteswap.h
ns_hash.o: /usr/include/arpa/inet.h ns_hash.h hash.h iftop.h
resolver.o: /usr/include/netinet/in.h /usr/include/features.h
resolver.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
resolver.o: /usr/include/stdint.h /usr/include/bits/wchar.h
resolver.o: /usr/include/bits/wordsize.h /usr/include/bits/types.h
resolver.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
resolver.o: /usr/include/bits/socket.h /usr/include/limits.h
resolver.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
resolver.o: /usr/include/sys/types.h /usr/include/time.h
resolver.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
resolver.o: /usr/include/asm/sockios.h /usr/include/bits/in.h
resolver.o: /usr/include/endian.h /usr/include/bits/endian.h
resolver.o: /usr/include/bits/byteswap.h /usr/include/sys/socket.h
resolver.o: /usr/include/sys/uio.h /usr/include/bits/uio.h
resolver.o: /usr/include/arpa/inet.h /usr/include/pthread.h
resolver.o: /usr/include/sched.h /usr/include/bits/sched.h
resolver.o: /usr/include/signal.h /usr/include/bits/sigset.h
resolver.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/initspin.h
resolver.o: /usr/include/bits/sigthread.h /usr/include/stdio.h
resolver.o: /usr/include/libio.h /usr/include/_G_config.h
resolver.o: /usr/include/wchar.h /usr/include/gconv.h
resolver.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
resolver.o: /usr/include/bits/stdio_lim.h /usr/include/stdlib.h
resolver.o: /usr/include/netdb.h /usr/include/bits/netdb.h
resolver.o: /usr/include/errno.h /usr/include/bits/errno.h
resolver.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
resolver.o: /usr/include/string.h ns_hash.h hash.h iftop.h
ui.o: /usr/include/ctype.h /usr/include/features.h /usr/include/sys/cdefs.h
ui.o: /usr/include/gnu/stubs.h /usr/include/bits/types.h
ui.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
ui.o: /usr/include/endian.h /usr/include/bits/endian.h /usr/include/curses.h
ui.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
ui.o: /usr/include/wchar.h /usr/include/bits/wchar.h /usr/include/gconv.h
ui.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
ui.o: /usr/include/bits/stdio_lim.h /usr/include/ncurses/unctrl.h
ui.o: /usr/include/ncurses/curses.h /usr/include/string.h /usr/include/math.h
ui.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
ui.o: /usr/include/bits/mathcalls.h /usr/include/pthread.h
ui.o: /usr/include/sched.h /usr/include/time.h /usr/include/bits/sched.h
ui.o: /usr/include/signal.h /usr/include/bits/sigset.h
ui.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/initspin.h
ui.o: /usr/include/bits/sigthread.h /usr/include/stdlib.h addr_hash.h
ui.o: /usr/include/sys/socket.h /usr/include/sys/uio.h
ui.o: /usr/include/sys/types.h /usr/include/bits/uio.h
ui.o: /usr/include/bits/socket.h /usr/include/limits.h
ui.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
ui.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
ui.o: /usr/include/asm/sockios.h /usr/include/netinet/in.h
ui.o: /usr/include/stdint.h /usr/include/bits/wordsize.h
ui.o: /usr/include/bits/in.h /usr/include/bits/byteswap.h
ui.o: /usr/include/arpa/inet.h hash.h iftop.h resolver.h sorted_list.h
util.o: /usr/include/sys/types.h /usr/include/features.h
util.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
util.o: /usr/include/bits/types.h
util.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
util.o: /usr/include/time.h /usr/include/errno.h /usr/include/bits/errno.h
util.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
util.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
util.o: /usr/include/wchar.h /usr/include/bits/wchar.h /usr/include/gconv.h
util.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
util.o: /usr/include/bits/stdio_lim.h /usr/include/stdlib.h
util.o: /usr/include/string.h /usr/include/unistd.h
util.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h iftop.h
sorted_list.o: /usr/include/stdlib.h /usr/include/features.h
sorted_list.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
sorted_list.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
sorted_list.o: /usr/include/stdio.h /usr/include/bits/types.h
sorted_list.o: /usr/include/libio.h /usr/include/_G_config.h
sorted_list.o: /usr/include/wchar.h /usr/include/bits/wchar.h
sorted_list.o: /usr/include/gconv.h
sorted_list.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
sorted_list.o: /usr/include/bits/stdio_lim.h sorted_list.h iftop.h
