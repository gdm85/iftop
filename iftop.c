/*
 * iftop.c:
 *
 */

#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <curses.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "iftop.h"
#include "addr_hash.h"
#include "resolver.h"
#include "ui.h"

/* Global options. */
char *interface = "eth0";
char *filtercode = NULL;

unsigned char if_hw_addr[6];    /* ethernet address of interface. */

hash_type* history;
time_t last_timestamp;
int history_pos = 0;
int history_len = 1;
pthread_mutex_t tick_mutex;

/* Open non-promiscuous by default since this is intended to be run on a
 * router. */
int promiscuous = 0;

sig_atomic_t foad;

static void finish(int sig) {
    foad = sig;
}



/* Only need ethernet and IP headers. */
#define CAPTURE_LENGTH 48

void init_history() {
    history = addr_hash_create();
    last_timestamp = time(NULL);
}

history_type* history_create() {
    history_type* h;
    h = xcalloc(1, sizeof *h);
    return h;
}

void history_rotate() {
    hash_node_type* n = NULL;
    history_pos = (history_pos + 1) % HISTORY_LENGTH;
    hash_next_item(history, &n);
    while(n != NULL) {
        hash_node_type* next = n;
        history_type* d = (history_type*)n->rec;
        hash_next_item(history, &next);

        if(d->last_write == history_pos) {
            addr_pair key = *(addr_pair*)(n->key);
            hash_delete(history, &key);
	    free(d);
        }
        else {
            d->recv[history_pos] = 0;
            d->sent[history_pos] = 0;
        }
        n = next; 
    }
    if(history_len < HISTORY_LENGTH) {
        history_len++;
    }
}


void tick() {
    time_t t;

    pthread_mutex_lock(&tick_mutex);
   
    t = time(NULL);
    if(t - last_timestamp >= RESOLUTION) {
        //printf("TICKING\n");
        ui_print();
        history_rotate();
        last_timestamp = t;
    }

    pthread_mutex_unlock(&tick_mutex);
}

static void handle_packet(char* args, const struct pcap_pkthdr* pkthdr,const char* packet)
{
    struct ether_header *eptr;
    eptr = (struct ether_header*)packet;
	
    tick();
    
    if(ntohs(eptr->ether_type) == ETHERTYPE_IP) {
        struct ip* iptr;
        history_type* ht;
        addr_pair ap;
	int promisc = 0;

        iptr = (struct ip*)(packet + sizeof(struct ether_header)); /* alignment? */

        if(memcmp(eptr->ether_shost, if_hw_addr, 6) == 0 ) {
            /* Packet leaving this interface. */
            ap.src = iptr->ip_src;
            ap.dst = iptr->ip_dst;
        } 
        else if(memcmp(eptr->ether_dhost, if_hw_addr, 6) == 0 || memcmp("\xFF\xFF\xFF\xFF\xFF\xFF", eptr->ether_dhost, 6) == 0) {
            ap.src = iptr->ip_dst;
            ap.dst = iptr->ip_src;
        }
	/*
	 * This packet is not from or to this interface.  Therefore assume
	 * it was picked up in promisc mode.
	 */
        else if(iptr->ip_src.s_addr < iptr->ip_dst.s_addr) {
            ap.src = iptr->ip_src;
            ap.dst = iptr->ip_dst;
	    promisc = 1;
        }
        else {
            ap.src = iptr->ip_dst;
            ap.dst = iptr->ip_src;
	    promisc = 1;
        }


	/* Add the address to be resolved */
        resolve(&iptr->ip_dst, NULL, 0);

        if(hash_find(history, &ap, (void**)&ht) == HASH_STATUS_KEY_NOT_FOUND) {
            ht = history_create();
	    ht->promisc = promisc;
            hash_insert(history, &ap, ht);
        }

        /* Update record */
        ht->last_write = history_pos;
        if(iptr->ip_src.s_addr == ap.src.s_addr) {
            ht->sent[history_pos] += ntohs(iptr->ip_len);
        }
        else {
            ht->recv[history_pos] += ntohs(iptr->ip_len);
        }

    }
}

/* packet_loop:
 * Worker function for packet capture thread. */
void packet_loop(void* ptr) {
    char errbuf[PCAP_ERRBUF_SIZE];
    char* str = "ip";
    pcap_t* pd;
    struct bpf_program F;
    int s;
    struct ifreq ifr = {0};

    /* First, get the address of the interface. If it isn't an ethernet
     * interface whose address we can obtain, there's not a lot we can do. */
    s = socket(PF_INET, SOCK_DGRAM, 0); /* any sort of IP socket will do */
    if (s == -1) {
        perror("socket");
        foad = 1;
        return;
    }
    strncpy(ifr.ifr_name, interface, IFNAMSIZ);
    ifr.ifr_hwaddr.sa_family = AF_UNSPEC;
    if (ioctl(s, SIOCGIFHWADDR, &ifr) == -1) {
        perror("ioctl(SIOCGIFHWADDR)");
        foad = 1;
        return;
    }
    close(s);
    memcpy(if_hw_addr, ifr.ifr_hwaddr.sa_data, 6);
    fprintf(stderr, "MAC address is:");
    for (s = 0; s < 6; ++s)
        fprintf(stderr, "%c%02x", s ? ':' : ' ', (unsigned int)if_hw_addr[s]);
    fprintf(stderr, "\n");
    
    resolver_initialise();

    pd = pcap_open_live(interface, CAPTURE_LENGTH, promiscuous, 1000, errbuf);
    if(pd == NULL) { 
        fprintf(stderr, "pcap_open_live(%s): %s\n", interface, errbuf); 
        foad = 1;
        return;
    }
    if (filtercode) {
        str = xmalloc(strlen(filtercode) + sizeof "() and ip");
        sprintf(str, "(%s) and ip", filtercode);
    }
    if (pcap_compile(pd, &F, str, 1, 0) == -1) {
        fprintf(stderr, "pcap_compile(%s): %s\n", str, pcap_geterr(pd));
        foad = 1;
        return;
    }
    if (pcap_setfilter(pd, &F) == -1) {
        fprintf(stderr, "pcap_setfilter: %s\n", pcap_geterr(pd));
        foad = 1;
        return;
    }
    if (filtercode)
        xfree(str);
    printf("Begin loop\n");
    pcap_loop(pd,0,(pcap_handler)handle_packet,NULL);
    printf("end loop\n");
}

/* usage:
 * Print usage information. */
void usage(FILE *fp) {
    fprintf(fp,
"iftop: display bandwidth usage on an interface by host\n"
"\n"
"Synopsis: iftop -h | [-d] [-p] [-i interface] [-f filter code]\n"
"\n"
"   -h                  display this message\n"
"   -d                  don't do hostname lookups\n"
"   -p                  run in promiscuous mode (show traffic between other\n"
"                       hosts on the same network segment)\n"
"   -i interface        listen on named interface (default: eth0)\n"
"   -f filter code      use filter code to select packets to count\n"
"                       (default: none, but only IP packets are counted)\n"
"\n"
"iftop, version " IFTOP_VERSION "copyright (c) 2002 Paul Warren <pdw@ex-parrot.com>\n"
            );
}

/* main:
 * Entry point. See usage(). */
char optstr[] = "+i:f:dhp";
int main(int argc, char **argv) {
    pthread_t thread;
    struct sigaction sa = {0};
    extern int dnsresolution;   /* in ui.c */
    int opt;

    opterr = 0;
    while ((opt = getopt(argc, argv, optstr)) != -1) {
        switch (opt) {
            case 'h':
                usage(stdout);
                return 0;

            case 'd':
                dnsresolution = 0;
                break;

            case 'i':
                interface = optarg;
                break;

            case 'f':
                filtercode = optarg;
                break;

            case 'p':
                promiscuous = 1;
                break;

            case '?':
                fprintf(stderr, "iftop: unknown option -%c\n", optopt);
                usage(stderr);
                return 1;

            case ':':
                fprintf(stderr, "iftop: option -%c requires an argument\n", optopt);
                usage(stderr);
                return 1;
        }
    }
    
    sa.sa_handler = finish;
    sigaction(SIGINT, &sa, NULL);

    pthread_mutex_init(&tick_mutex, NULL);

    init_history();

    pthread_create(&thread, NULL, (void*)&packet_loop, NULL);

    ui_loop();

    pthread_cancel(thread);

    ui_finish();
    
    return 0;
}
