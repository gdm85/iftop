/*
 * options.h:
 *
 */

#ifndef __OPTIONS_H_ /* include guard */
#define __OPTIONS_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


typedef enum {
  OPTION_PORTS_OFF,
  OPTION_PORTS_SRC,
  OPTION_PORTS_DEST,
  OPTION_PORTS_ON
} option_port_t;

typedef enum {
  OPTION_SORT_DIV1,
  OPTION_SORT_DIV2,
  OPTION_SORT_DIV3,
  OPTION_SORT_SRC,
  OPTION_SORT_DEST
} option_sort_t;

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
    int portresolution;
    int promiscuous;
    int showbars;
    option_port_t showports;
    int aggregate_src;
    int aggregate_dest;
    int paused;
    int showhelp;
    int bandwidth_in_bytes;
    option_sort_t sort;

    char* screenfilter;
    int freezeorder;

} options_t;

#endif /* __OPTIONS_H_ */
