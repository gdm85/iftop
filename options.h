/*
 * options.h:
 *
 */

#ifndef __OPTIONS_H_ /* include guard */
#define __OPTIONS_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

enum {
  OPTION_AGGREGATE_OFF,
  OPTION_AGGREGATE_SOURCE,
  OPTION_AGGREGATE_DEST
} option_aggregate_t;

typedef struct {
    /* interface on which to listen */
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
    option_aggregate_t aggregate;

} options_t;

#endif /* __OPTIONS_H_ */
