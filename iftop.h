/*
 * iftop.h:
 *
 */

#ifndef __IFTOP_H_ /* include guard */
#define __IFTOP_H_

#include "config.h"

/* 40 / 2  */
#define HISTORY_LENGTH  20
#define RESOLUTION 2
#define DUMP_RESOLUTION 300

/* At least OpenBSD and NexentaCore do not
 * define s6_addr32 for user land settings.
 */
#if !defined s6_addr32 && defined __sun__
#	define s6_addr32 _S6_un._S6_u32
#elif !defined s6_addr32 && \
		( defined __OpenBSD__ || defined __FreeBSD__ )
#	define s6_addr32 __u6_addr.__u6_addr32
#endif	/* !defined s6_addr32 */

typedef struct {
    long recv[HISTORY_LENGTH];
    long sent[HISTORY_LENGTH];
    double long total_sent;
    double long total_recv;
    int last_write;
} history_type;

void tick(int print);

void *xmalloc(size_t n);
void *xcalloc(size_t n, size_t m);
void *xrealloc(void *w, size_t n);
char *xstrdup(const char *s);
void xfree(void *v);

/* options.c */
void options_read(int argc, char **argv);

struct pfloghdr {
      unsigned char		length;
      unsigned char		af;
      unsigned char		action;
      unsigned char		reason;
      char				ifname[16];
      char				ruleset[16];
      unsigned int		rulenr;
      unsigned int		subrulenr;
      unsigned char		dir;
      unsigned char		pad[3];
};

#endif /* __IFTOP_H_ */
