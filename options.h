/*
 * options.h:
 *
 */

#ifndef __OPTIONS_H_ /* include guard */
#define __OPTIONS_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
    /* interface to listen on */
    char *interface;

    /* pcap filter code */
    char *filtercode;

    /* Cross network filter */
    int netfilter;
    struct in_addr netfilternet;
    struct in_addr netfiltermask;
    int dnsresolution;
    int promiscuous;
    int showbars;

} options_t;

#endif /* __OPTIONS_H_ */
