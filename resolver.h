/*
 * resolver.h:
 *
 */

#ifndef __RESOLVER_H_ /* include guard */
#define __RESOLVER_H_

#include <netinet/in.h>

void resolver_initialise();

void resolve(struct in_addr* addr, char* result, int buflen);

#endif /* __RESOLVER_H_ */
