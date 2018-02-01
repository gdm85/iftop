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
#include "serv_hash.h"
#include "iftop.h"
#include "resolver.h"
#include "sorted_list.h"
#include "options.h"
#include "screenfilter.h"

#define HOSTNAME_LENGTH 256

#define HISTORY_DIVISIONS   3

#define HELP_TIME 2

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


/* 2, 10 and 40 seconds */
int history_divs[HISTORY_DIVISIONS] = {1, 5, 20};

#define UNIT_DIVISIONS 4
char* unit_bits[UNIT_DIVISIONS] =  { "b", "Kb", "Mb", "Gb"};
char* unit_bytes[UNIT_DIVISIONS] =  { "B", "KB", "MB", "GB"};

typedef struct host_pair_line_tag {
    addr_pair ap;
    double long total_recv;
    double long total_sent;
    double long recv[HISTORY_DIVISIONS];
    double long sent[HISTORY_DIVISIONS];
} host_pair_line;


extern hash_type* history;
extern int history_pos;
extern int history_len;

extern options_t options ;

void ui_finish();

hash_type* screen_hash;
hash_type* service_hash;
sorted_list_type screen_list;
host_pair_line totals;
int peaksent, peakrecv, peaktotal;

#define HELP_MSG_SIZE 80
int showhelphint = 0;
int persistenthelp = 0;
time_t helptimer = 0;
char helpmsg[HELP_MSG_SIZE];
int dontshowdisplay = 0;

/*
 * Compare two screen lines based on bandwidth.  Start comparing from the
 * specified column
 */
int screen_line_bandwidth_compare(host_pair_line* aa, host_pair_line* bb, int start_div) {
    int i;
    for(i = start_div; i < HISTORY_DIVISIONS; i++) {
        if(aa->recv[i] + aa->sent[i] != bb->recv[i] + bb->sent[i]) {
            return(aa->recv[i] + aa->sent[i] < bb->recv[i] + bb->sent[i]);
        }
    }
    return 1;
}

/*
 * Compare two screen lines based on hostname / IP.  Fall over to compare by
 * bandwidth.
 */
int screen_line_host_compare(struct in_addr* a, struct in_addr* b, host_pair_line* aa, host_pair_line* bb) {
    char hosta[HOSTNAME_LENGTH], hostb[HOSTNAME_LENGTH];
    int r;

    /* This isn't overly efficient because we resolve again before
       display. */
    if (options.dnsresolution) {
        resolve(a, hosta, HOSTNAME_LENGTH);
        resolve(b, hostb, HOSTNAME_LENGTH);
    }
    else {
        strcpy(hosta, inet_ntoa(*a));
        strcpy(hostb, inet_ntoa(*b));
    }

    r = strcmp(hosta, hostb);

    if(r == 0) {
        return screen_line_bandwidth_compare(aa, bb, 2);
    }
    else {
        return (r > 0);
    }


}

int screen_line_compare(void* a, void* b) {
    host_pair_line* aa = (host_pair_line*)a;
    host_pair_line* bb = (host_pair_line*)b;
    if(options.sort == OPTION_SORT_DIV1) {
      return screen_line_bandwidth_compare(aa, bb, 0);
    }
    else if(options.sort == OPTION_SORT_DIV2) {
      return screen_line_bandwidth_compare(aa, bb, 1);
    }
    else if(options.sort == OPTION_SORT_DIV3) {
      return screen_line_bandwidth_compare(aa, bb, 2);
    }
    else if(options.sort == OPTION_SORT_SRC) {
      return screen_line_host_compare(&(aa->ap.src), &(bb->ap.src), aa, bb);
    }
    else if(options.sort == OPTION_SORT_DEST) {
      return screen_line_host_compare(&(aa->ap.dst), &(bb->ap.dst), aa, bb);
    }

    return 1;
}

void readable_size(float n, char* buf, int bsize, int ksize, int bytes) {

    int i = 0;
    float size = 1;

    /* Convert to bits? */
    if(bytes == 0) {
      n *= 8;
    }

    while(1) {
      if(n < size * 1000 || i >= UNIT_DIVISIONS - 1) {
        snprintf(buf, bsize, " %4.0f%s", n / size, bytes ? unit_bytes[i] : unit_bits[i]);
        break;
      }
      i++;
      size *= ksize;
      if(n < size * 10) {
        snprintf(buf, bsize, " %4.2f%s", n / size, bytes ? unit_bytes[i] : unit_bits[i]);
        break;
      }
      else if(n < size * 100) {
        snprintf(buf, bsize, " %4.1f%s", n / size, bytes ? unit_bytes[i] : unit_bits[i]);
        break;
      }
  }
}


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

int history_length(const int d) {
    if (history_len < history_divs[d])
        return history_len * RESOLUTION;
    else
        return history_divs[d] * RESOLUTION;
}

extern history_type history_totals;

void screen_list_init() {
    screen_list.compare = &screen_line_compare;
    sorted_list_initialise(&screen_list);
}

void screen_list_clear() {
    sorted_list_node* nn = NULL;
    peaksent = peakrecv = peaktotal = 0;
    while((nn = sorted_list_next_item(&screen_list, nn)) != NULL) {
        free(nn->data);
    }
    sorted_list_destroy(&screen_list);
}

void calculate_totals() {
    int i;

    /**
     * Calculate peaks and totals
     */
    for(i = 0; i < HISTORY_LENGTH; i++) {
        int j;
        int ii = (HISTORY_LENGTH + history_pos - i) % HISTORY_LENGTH;

        for(j = 0; j < HISTORY_DIVISIONS; j++) {
            if(i < history_divs[j]) {
                totals.recv[j] += history_totals.recv[ii];
                totals.sent[j] += history_totals.sent[ii];
            }
        }

        if(history_totals.recv[i] > peakrecv) {
            peakrecv = history_totals.recv[i];
        }
        if(history_totals.sent[i] > peaksent) {
            peaksent = history_totals.sent[i];
        }
        if(history_totals.recv[i] + history_totals.sent[i] > peaktotal) {
            peaktotal = history_totals.recv[i] + history_totals.sent[i];
        }
    }
    for(i = 0; i < HISTORY_DIVISIONS; i++) {
      int t = history_length(i);
      totals.recv[i] /= t;
      totals.sent[i] /= t;
    }
}

void make_screen_list() {
    hash_node_type* n = NULL;
    while(hash_next_item(screen_hash, &n) == HASH_STATUS_OK) {
        host_pair_line* line = (host_pair_line*)n->rec;
        int i;
        for(i = 0; i < HISTORY_DIVISIONS; i++) {
          line->recv[i] /= history_length(i);
          line->sent[i] /= history_length(i);
        }

        /* Don't make a new, sorted screen list if order is frozen
         */
        if(!options.freezeorder) {
            sorted_list_insert(&screen_list, line);
        }

    }
}

/*
 * Zeros all data in the screen hash, but does not remove items.
 */
void screen_hash_clear() {
    hash_node_type* n = NULL;
    while(hash_next_item(screen_hash, &n) == HASH_STATUS_OK) {
        host_pair_line* hpl = (host_pair_line*)n->rec;
        memset(hpl->recv, 0, sizeof(hpl->recv));
        memset(hpl->sent, 0, sizeof(hpl->sent));
    }
}

void analyse_data() {
    hash_node_type* n = NULL;

    if(options.paused == 1) {
      return;
    }

    // Zero totals
    memset(&totals, 0, sizeof totals);

    if(options.freezeorder) {
      screen_hash_clear();
    }
    else {
      screen_list_clear();
      hash_delete_all(screen_hash);
    }

    while(hash_next_item(history, &n) == HASH_STATUS_OK) {
        history_type* d = (history_type*)n->rec;
        host_pair_line* screen_line;
	union {
	    host_pair_line **h_p_l_pp;
	    void **void_pp;
	} u_screen_line = { &screen_line };
        addr_pair ap;
        int i;
        int tsent, trecv;
        tsent = trecv = 0;


        ap = *(addr_pair*)n->key;

        /* Aggregate hosts, if required */
        if(options.aggregate_src) {
            ap.src.s_addr = 0;
        }
        if(options.aggregate_dest) {
            ap.dst.s_addr = 0;
        }

        /* Aggregate ports, if required */
        if(options.showports == OPTION_PORTS_DEST || options.showports == OPTION_PORTS_OFF) {
            ap.src_port = 0;
        }
        if(options.showports == OPTION_PORTS_SRC || options.showports == OPTION_PORTS_OFF) {
            ap.dst_port = 0;
        }
        if(options.showports == OPTION_PORTS_OFF) {
            ap.protocol = 0;
        }


        if(hash_find(screen_hash, &ap, u_screen_line.void_pp) == HASH_STATUS_KEY_NOT_FOUND) {
            screen_line = xcalloc(1, sizeof *screen_line);
            hash_insert(screen_hash, &ap, screen_line);
            screen_line->ap = ap;
        }

	screen_line->total_sent += d->total_sent;
	screen_line->total_recv += d->total_recv;

        for(i = 0; i < HISTORY_LENGTH; i++) {
            int j;
            int ii = (HISTORY_LENGTH + history_pos - i) % HISTORY_LENGTH;

            for(j = 0; j < HISTORY_DIVISIONS; j++) {
                if(i < history_divs[j]) {
                    screen_line->recv[j] += d->recv[ii];
                    screen_line->sent[j] += d->sent[ii];
                }
            }
        }

    }

    make_screen_list();


    calculate_totals();

}

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
        if (options.dnsresolution)
            resolve(addr, hostname, L);
        else
            strcpy(hostname, inet_ntoa(*addr));
    }
    left = strlen(hostname);

    if(port != 0) {
      skey.port = port;
      skey.protocol = protocol;
      if(options.portresolution && hash_find(service_hash, &skey, u_s_name.void_pp) == HASH_STATUS_OK) {
        snprintf(service, HOSTNAME_LENGTH, ":%s", s_name);
      }
      else {
        snprintf(service, HOSTNAME_LENGTH, ":%d", port);
      }
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
	while(1) {

		printf("waiting 1 second...\n");
		sleep(1);
		
		main_print();
		
		tick(0);
	}
}
