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


addr_hash.o: addr_hash.h hash.h iftop.h /usr/include/alloca.h
addr_hash.o: /usr/include/arpa/inet.h /usr/include/asm/socket.h
addr_hash.o: /usr/include/asm/sockios.h /usr/include/bits/byteswap.h
addr_hash.o: /usr/include/bits/endian.h /usr/include/bits/in.h
addr_hash.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
addr_hash.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
addr_hash.o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
addr_hash.o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
addr_hash.o: /usr/include/bits/types.h /usr/include/bits/uio.h
addr_hash.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
addr_hash.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
addr_hash.o: /usr/include/endian.h /usr/include/features.h
addr_hash.o: /usr/include/_G_config.h /usr/include/gconv.h
addr_hash.o: /usr/include/gnu/stubs.h /usr/include/libio.h
addr_hash.o: /usr/include/netinet/in.h /usr/include/stdint.h
addr_hash.o: /usr/include/stdio.h /usr/include/stdlib.h
addr_hash.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
addr_hash.o: /usr/include/sys/socket.h /usr/include/sys/sysmacros.h
addr_hash.o: /usr/include/sys/types.h /usr/include/sys/uio.h
addr_hash.o: /usr/include/time.h /usr/include/wchar.h /usr/include/xlocale.h
addr_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
addr_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
addr_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
addr_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
hash.o: hash.h iftop.h /usr/include/alloca.h /usr/include/bits/endian.h
hash.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
hash.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
hash.o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
hash.o: /usr/include/bits/types.h /usr/include/bits/waitflags.h
hash.o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
hash.o: /usr/include/endian.h /usr/include/features.h /usr/include/_G_config.h
hash.o: /usr/include/gconv.h /usr/include/gnu/stubs.h /usr/include/libio.h
hash.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/sys/cdefs.h
hash.o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
hash.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/wchar.h
hash.o: /usr/include/xlocale.h
hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
iftop.o: addr_hash.h hash.h iftop.h resolver.h ui.h /usr/include/alloca.h
iftop.o: /usr/include/arpa/inet.h /usr/include/asm/sigcontext.h
iftop.o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
iftop.o: /usr/include/bits/byteswap.h /usr/include/bits/confname.h
iftop.o: /usr/include/bits/endian.h /usr/include/bits/environments.h
iftop.o: /usr/include/bits/in.h /usr/include/bits/initspin.h
iftop.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
iftop.o: /usr/include/bits/sched.h /usr/include/bits/select.h
iftop.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
iftop.o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
iftop.o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
iftop.o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
iftop.o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
iftop.o: /usr/include/bits/time.h /usr/include/bits/types.h
iftop.o: /usr/include/bits/uio.h /usr/include/bits/waitflags.h
iftop.o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
iftop.o: /usr/include/bits/wordsize.h /usr/include/curses.h
iftop.o: /usr/include/endian.h /usr/include/features.h
iftop.o: /usr/include/_G_config.h /usr/include/gconv.h /usr/include/getopt.h
iftop.o: /usr/include/gnu/stubs.h /usr/include/libio.h
iftop.o: /usr/include/linux/if_ether.h /usr/include/ncurses/curses.h
iftop.o: /usr/include/ncurses/unctrl.h /usr/include/net/ethernet.h
iftop.o: /usr/include/netinet/in.h /usr/include/netinet/ip.h
iftop.o: /usr/include/pthread.h /usr/include/sched.h /usr/include/signal.h
iftop.o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/stdlib.h
iftop.o: /usr/include/string.h /usr/include/sys/cdefs.h
iftop.o: /usr/include/sys/select.h /usr/include/sys/socket.h
iftop.o: /usr/include/sys/sysmacros.h /usr/include/sys/time.h
iftop.o: /usr/include/sys/types.h /usr/include/sys/ucontext.h
iftop.o: /usr/include/sys/uio.h /usr/include/time.h /usr/include/ucontext.h
iftop.o: /usr/include/unistd.h /usr/include/wchar.h /usr/include/xlocale.h
iftop.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
iftop.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
iftop.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
iftop.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
iftop.o: /usr/local/include/net/bpf.h /usr/local/include/pcap.h
ns_hash.o: hash.h iftop.h ns_hash.h /usr/include/alloca.h
ns_hash.o: /usr/include/arpa/inet.h /usr/include/asm/socket.h
ns_hash.o: /usr/include/asm/sockios.h /usr/include/bits/byteswap.h
ns_hash.o: /usr/include/bits/endian.h /usr/include/bits/in.h
ns_hash.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
ns_hash.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
ns_hash.o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
ns_hash.o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
ns_hash.o: /usr/include/bits/types.h /usr/include/bits/uio.h
ns_hash.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
ns_hash.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
ns_hash.o: /usr/include/endian.h /usr/include/features.h
ns_hash.o: /usr/include/_G_config.h /usr/include/gconv.h
ns_hash.o: /usr/include/gnu/stubs.h /usr/include/libio.h
ns_hash.o: /usr/include/netinet/in.h /usr/include/stdint.h
ns_hash.o: /usr/include/stdio.h /usr/include/stdlib.h /usr/include/sys/cdefs.h
ns_hash.o: /usr/include/sys/select.h /usr/include/sys/socket.h
ns_hash.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
ns_hash.o: /usr/include/sys/uio.h /usr/include/time.h /usr/include/wchar.h
ns_hash.o: /usr/include/xlocale.h
ns_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
ns_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
ns_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
ns_hash.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
resolver.o: hash.h iftop.h ns_hash.h /usr/include/alloca.h
resolver.o: /usr/include/arpa/inet.h /usr/include/asm/errno.h
resolver.o: /usr/include/asm/socket.h /usr/include/asm/sockios.h
resolver.o: /usr/include/bits/byteswap.h /usr/include/bits/endian.h
resolver.o: /usr/include/bits/errno.h /usr/include/bits/in.h
resolver.o: /usr/include/bits/initspin.h /usr/include/bits/netdb.h
resolver.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sched.h
resolver.o: /usr/include/bits/select.h /usr/include/bits/siginfo.h
resolver.o: /usr/include/bits/sigset.h /usr/include/bits/sigthread.h
resolver.o: /usr/include/bits/sockaddr.h /usr/include/bits/socket.h
resolver.o: /usr/include/bits/stdio_lim.h /usr/include/bits/time.h
resolver.o: /usr/include/bits/types.h /usr/include/bits/uio.h
resolver.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
resolver.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
resolver.o: /usr/include/endian.h /usr/include/errno.h /usr/include/features.h
resolver.o: /usr/include/_G_config.h /usr/include/gconv.h
resolver.o: /usr/include/gnu/stubs.h /usr/include/libio.h
resolver.o: /usr/include/linux/errno.h /usr/include/netdb.h
resolver.o: /usr/include/netinet/in.h /usr/include/pthread.h
resolver.o: /usr/include/rpc/netdb.h /usr/include/sched.h
resolver.o: /usr/include/signal.h /usr/include/stdint.h /usr/include/stdio.h
resolver.o: /usr/include/stdlib.h /usr/include/string.h
resolver.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
resolver.o: /usr/include/sys/socket.h /usr/include/sys/sysmacros.h
resolver.o: /usr/include/sys/types.h /usr/include/sys/uio.h
resolver.o: /usr/include/time.h /usr/include/wchar.h /usr/include/xlocale.h
resolver.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
resolver.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
resolver.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
resolver.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
sorted_list.o: iftop.h sorted_list.h /usr/include/alloca.h
sorted_list.o: /usr/include/bits/endian.h /usr/include/bits/pthreadtypes.h
sorted_list.o: /usr/include/bits/sched.h /usr/include/bits/select.h
sorted_list.o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
sorted_list.o: /usr/include/bits/time.h /usr/include/bits/types.h
sorted_list.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
sorted_list.o: /usr/include/bits/wchar.h /usr/include/endian.h
sorted_list.o: /usr/include/features.h /usr/include/_G_config.h
sorted_list.o: /usr/include/gconv.h /usr/include/gnu/stubs.h
sorted_list.o: /usr/include/libio.h /usr/include/stdio.h /usr/include/stdlib.h
sorted_list.o: /usr/include/sys/cdefs.h /usr/include/sys/select.h
sorted_list.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
sorted_list.o: /usr/include/time.h /usr/include/wchar.h /usr/include/xlocale.h
sorted_list.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
sorted_list.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
ui.o: addr_hash.h hash.h iftop.h resolver.h sorted_list.h
ui.o: /usr/include/alloca.h /usr/include/arpa/inet.h
ui.o: /usr/include/asm/sigcontext.h /usr/include/asm/socket.h
ui.o: /usr/include/asm/sockios.h /usr/include/bits/byteswap.h
ui.o: /usr/include/bits/endian.h /usr/include/bits/huge_val.h
ui.o: /usr/include/bits/in.h /usr/include/bits/initspin.h
ui.o: /usr/include/bits/mathcalls.h /usr/include/bits/mathdef.h
ui.o: /usr/include/bits/nan.h /usr/include/bits/pthreadtypes.h
ui.o: /usr/include/bits/sched.h /usr/include/bits/select.h
ui.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
ui.o: /usr/include/bits/siginfo.h /usr/include/bits/signum.h
ui.o: /usr/include/bits/sigset.h /usr/include/bits/sigstack.h
ui.o: /usr/include/bits/sigthread.h /usr/include/bits/sockaddr.h
ui.o: /usr/include/bits/socket.h /usr/include/bits/stdio_lim.h
ui.o: /usr/include/bits/time.h /usr/include/bits/types.h
ui.o: /usr/include/bits/uio.h /usr/include/bits/waitflags.h
ui.o: /usr/include/bits/waitstatus.h /usr/include/bits/wchar.h
ui.o: /usr/include/bits/wordsize.h /usr/include/curses.h /usr/include/endian.h
ui.o: /usr/include/features.h /usr/include/_G_config.h /usr/include/gconv.h
ui.o: /usr/include/gnu/stubs.h /usr/include/libio.h /usr/include/math.h
ui.o: /usr/include/ncurses/curses.h /usr/include/ncurses/unctrl.h
ui.o: /usr/include/netinet/in.h /usr/include/pthread.h /usr/include/sched.h
ui.o: /usr/include/signal.h /usr/include/stdint.h /usr/include/stdio.h
ui.o: /usr/include/stdlib.h /usr/include/string.h /usr/include/sys/cdefs.h
ui.o: /usr/include/sys/select.h /usr/include/sys/socket.h
ui.o: /usr/include/sys/sysmacros.h /usr/include/sys/types.h
ui.o: /usr/include/sys/ucontext.h /usr/include/sys/uio.h /usr/include/time.h
ui.o: /usr/include/ucontext.h /usr/include/wchar.h /usr/include/xlocale.h
ui.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/limits.h
ui.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
ui.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
ui.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/syslimits.h
util.o: iftop.h /usr/include/alloca.h /usr/include/asm/errno.h
util.o: /usr/include/bits/confname.h /usr/include/bits/endian.h
util.o: /usr/include/bits/environments.h /usr/include/bits/errno.h
util.o: /usr/include/bits/posix_opt.h /usr/include/bits/pthreadtypes.h
util.o: /usr/include/bits/sched.h /usr/include/bits/select.h
util.o: /usr/include/bits/sigset.h /usr/include/bits/stdio_lim.h
util.o: /usr/include/bits/time.h /usr/include/bits/types.h
util.o: /usr/include/bits/waitflags.h /usr/include/bits/waitstatus.h
util.o: /usr/include/bits/wchar.h /usr/include/bits/wordsize.h
util.o: /usr/include/endian.h /usr/include/errno.h /usr/include/features.h
util.o: /usr/include/_G_config.h /usr/include/gconv.h /usr/include/getopt.h
util.o: /usr/include/gnu/stubs.h /usr/include/libio.h
util.o: /usr/include/linux/errno.h /usr/include/stdio.h /usr/include/stdlib.h
util.o: /usr/include/string.h /usr/include/sys/cdefs.h
util.o: /usr/include/sys/select.h /usr/include/sys/sysmacros.h
util.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/unistd.h
util.o: /usr/include/wchar.h /usr/include/xlocale.h
util.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stdarg.h
util.o: /usr/lib/gcc-lib/i386-redhat-linux/2.96/include/stddef.h
