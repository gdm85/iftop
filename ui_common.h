/*
 * ui_common.h
 *
 *
 */

#ifndef __UI_COMMON_H_ /* include guard */
#define __UI_COMMON_H_

#include <string.h>
#include <stdio.h>

#include "addr_hash.h"
#include "serv_hash.h"
#include "iftop.h"
#include "resolver.h"
#include "sorted_list.h"
#include "options.h"

#define HISTORY_DIVISIONS 3

#define UNIT_DIVISIONS 4

#define HOSTNAME_LENGTH 256

typedef struct host_pair_line_tag {
  addr_pair ap;
  double long total_recv;
  double long total_sent;
  double long recv[HISTORY_DIVISIONS];
  double long sent[HISTORY_DIVISIONS];
} host_pair_line;

extern options_t options;

sorted_list_type screen_list;
host_pair_line totals;
int peaksent, peakrecv, peaktotal;
extern history_type history_totals;
hash_type* screen_hash;
hash_type* service_hash;

void analyse_data(void);
void screen_list_init(void);
void sprint_host(char * line, int af, struct in6_addr* addr, unsigned int port, unsigned int protocol, int L, int unspecified_as_star);
void readable_size(float, char*, int, int, int);

#endif /* __UI_COMMON_H_ */
