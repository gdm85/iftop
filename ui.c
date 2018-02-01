/*
 * ui.c:
 *
 */

#include <sys/types.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

#include <sys/wait.h>

#include "addr_hash.h"
#include "iftop.h"
#include "sorted_list.h"
#include "options.h"
#include "screenfilter.h"

typedef struct {
    int port;
    int protocol;
} ip_service;

#define HOSTNAME_LENGTH 256

#define HELP_MESSAGE \
"Host display:                          General:\n"\
" n - toggle DNS host resolution         P - pause display\n"\
" s - toggle show source host            h - toggle this help display\n"\
" d - toggle show destination host       b - toggle bar graph display\n"\
" t - cycle line display mode            B - cycle bar graph average\n"\
"                                        T - toggle cummulative line totals\n"\
"Port display:                           j/k - scroll display\n"\
" N - toggle service resolution          f - edit filter code\n"\
" S - toggle show source port            l - set screen filter\n"\
" D - toggle show destination port       L - lin/log scales\n"\
" p - toggle port display                ! - shell command\n"\
"                                        q - quit\n"\
"Sorting:\n"\
" 1/2/3 - sort by 1st/2nd/3rd column\n"\
" < - sort by source name\n"\
" > - sort by dest name\n"\
" o - freeze current order\n"\
"\n"\
"iftop, version " IFTOP_VERSION


typedef struct host_pair_line_tag {
    addr_pair ap;
    double long total_recv;
    double long total_sent;
} host_pair_line;

extern options_t options ;

sorted_list_type screen_list;


/* Barchart scales. */
static struct {
    int max, interval;
} scale[] = {
        {      64000,     10 },     /* 64 kbit/s */
        {     128000,     10 },
        {     256000,     10 },
        {    1000000,     10 },     /* 1 Mbit/s */
        {   10000000,     10 },
        {  100000000,    100 },
        { 1000000000,    100 }      /* 1 Gbit/s */
    };
static int rateidx = 0, wantbiggerrate;

static int get_bar_interval(float bandwidth) {
    int i = 10;
    if(bandwidth > 100000000) {
        i = 100;
    }
    return i;
}

static float get_max_bandwidth() {
    float max;
    if(options.max_bandwidth > 0) {
        max = options.max_bandwidth;
    }
    else {
        max = scale[rateidx].max;
    }
    return max;
}

extern history_type history_totals;

void sprint_host(char * line, struct in_addr* addr, unsigned int port, unsigned int protocol, int L) {
    char hostname[HOSTNAME_LENGTH];
    char service[HOSTNAME_LENGTH];
    char* s_name;
    union {
        char **ch_pp;
        void **void_pp;
    } u_s_name = { &s_name };

    ip_service skey;
    int left;
    if(addr->s_addr == 0) {
        sprintf(hostname, " * ");
    }
    else {
       strcpy(hostname, inet_ntoa(*addr));
    }
    left = strlen(hostname);

    if(port != 0) {
      skey.port = port;
      skey.protocol = protocol;
      snprintf(service, HOSTNAME_LENGTH, ":%d", port);
    }
    else {
      service[0] = '\0';
    }


    sprintf(line, "%-*s", L, hostname);
    if(left > (L - strlen(service))) {
        left = L - strlen(service);
        if(left < 0) {
           left = 0;
        }
    }
    sprintf(line + left, "%-*s", L-left, service);
}

void main_print() {
	//TODO: print some stats
	printf("here stats would be printed\n");
}

void main_loop() {
	extern sig_atomic_t foad;

    while(foad == 0) {

		printf("waiting 1 second...\n");
		sleep(1);
		
		main_print();
	}
}
