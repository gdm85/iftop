/*
 * util.h:
 * Utility functions
 *
 * $Id$
 *
 */

#ifndef __UTIL_H_ /* include guard */
#define __UTIL_H_

/* util.c */
void *xmalloc(size_t n);
void *xcalloc(size_t n, size_t m);
void *xrealloc(void *w, size_t n);
char *xstrdup(const char *s);
void xfree(void *v);

#endif /* __UTIL_H_ */
