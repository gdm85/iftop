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
#include "options.h"


unsigned char if_hw_addr[6];    /* ethernet address of interface. */

extern options_t options;

hash_type* history;
history_type history_totals;
time_t last_timestamp;
int history_pos = 0;
int history_len = 1;
pthread_mutex_t tick_mutex;

pcap_t* pd; /* pcap descriptor */
pcap_handler packet_handler;

sig_atomic_t foad;

static void finish(int sig) {
    foad = sig;
}



/* Only need ethernet and IP headers. */
#define CAPTURE_LENGTH 48

void init_history() {
    history = addr_hash_create();
    last_timestamp = time(NULL);
    memset(&history_totals, 0, sizeof history_totals);
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

    history_totals.sent[history_pos] = 0;
    history_totals.recv[history_pos] = 0;

    if(history_len < HISTORY_LENGTH) {
        history_len++;
    }
}


void tick(int print) {
    time_t t;

    pthread_mutex_lock(&tick_mutex);
   
    t = time(NULL);
    if(t - last_timestamp >= RESOLUTION) {
        //printf("TICKING\n");
        analyse_data();
        ui_print();
        history_rotate();
        last_timestamp = t;
    }
    else if(print) {
        ui_print();
    }

    pthread_mutex_unlock(&tick_mutex);
}

int in_filter_net(struct in_addr addr) {
    int ret;
    ret = ((addr.s_addr & options.netfiltermask.s_addr) == options.netfilternet.s_addr);
    return ret;
}

static void handle_ip_packet(struct ip* iptr, int hw_dir)
{
    int direction = 0; /* incoming */
    history_type* ht;
    addr_pair ap;
    int len;

    if(options.netfilter == 0) { 
        /*
         * Net filter is off, so assign direction based on MAC address
         */
        if(hw_dir == 1) {
            /* Packet leaving this interface. */
            ap.src = iptr->ip_src;
            ap.dst = iptr->ip_dst;
            direction = 1;
        }
        else if(hw_dir == 0) {
            /* Packet incoming */
            ap.src = iptr->ip_dst;
            ap.dst = iptr->ip_src;
        }

       /*
        * This packet is not from or to this interface, or the h/ware 
        * layer did not give the direction away.  Therefore assume
        * it was picked up in promisc mode, and account it as incoming.
        */
        else if(iptr->ip_src.s_addr < iptr->ip_dst.s_addr) {
            ap.src = iptr->ip_src;
            ap.dst = iptr->ip_dst;
        }
        else {
            ap.src = iptr->ip_dst;
            ap.dst = iptr->ip_src;
        }
    }
    else {
        /* 
         * Net filter on, assign direction according to netmask 
         */ 
        if(in_filter_net(iptr->ip_src) & !in_filter_net(iptr->ip_dst)) {
            /* out of network */
            ap.src = iptr->ip_src;
            ap.dst = iptr->ip_dst;
            direction = 1;
        }
        else if(in_filter_net(iptr->ip_dst) & !in_filter_net(iptr->ip_src)) {
            /* into network */
            ap.src = iptr->ip_dst;
            ap.dst = iptr->ip_src;
        }
        else {
            /* drop packet */
            return ;
        }
    }

    /* Add the addresses to be resolved */
    resolve(&iptr->ip_dst, NULL, 0);
    resolve(&iptr->ip_src, NULL, 0);

    if(hash_find(history, &ap, (void**)&ht) == HASH_STATUS_KEY_NOT_FOUND) {
        ht = history_create();
        hash_insert(history, &ap, ht);
    }

    len = ntohs(iptr->ip_len);

    /* Update record */
    ht->last_write = history_pos;
    if(iptr->ip_src.s_addr == ap.src.s_addr) {
        ht->sent[history_pos] += len;
    ht->total_sent += len;
    }
    else {
        ht->recv[history_pos] += len;
    ht->total_recv += len;
    }

    if(direction == 0) {
        /* incoming */
        history_totals.recv[history_pos] += ntohs(iptr->ip_len);
    history_totals.total_recv += len;
    }
    else {
        history_totals.sent[history_pos] += ntohs(iptr->ip_len);
    history_totals.total_sent += len;
    }
    
}

static void handle_raw_packet(unsigned char* args, const struct pcap_pkthdr* pkthdr, const unsigned char* packet)
{
    handle_ip_packet((struct ip*)packet, -1);
}

static void handle_eth_packet(unsigned char* args, const struct pcap_pkthdr* pkthdr, const unsigned char* packet)
{
    struct ether_header *eptr;
    eptr = (struct ether_header*)packet;
       
    tick(0);
    
    if(ntohs(eptr->ether_type) == ETHERTYPE_IP) {
        struct ip* iptr;
        int dir = -1;
        
        /*
         * Is a direction implied by the MAC addresses?
         */
        if(memcmp(eptr->ether_shost, if_hw_addr, 6) == 0 ) {
            /* packet leaving this i/f */
            dir = 1;
        } 
        else if(memcmp(eptr->ether_dhost, if_hw_addr, 6) == 0 || memcmp("\xFF\xFF\xFF\xFF\xFF\xFF", eptr->ether_dhost, 6) == 0) {
            /* packet entering this i/f */
            dir = 0;
        }

        iptr = (struct ip*)(packet + sizeof(struct ether_header) ); /* alignment? */
        handle_ip_packet(iptr, dir);
    }
}


/*
 * packet_init:
 *
 * performs pcap initialisation, called before ui is initialised
 */
void packet_init() {
    char errbuf[PCAP_ERRBUF_SIZE];
    char* str = "ip";
    struct bpf_program F;
    int s;
    struct ifreq ifr = {0};
    int dlt;

    /* First, get the address of the interface. If it isn't an ethernet
     * interface whose address we can obtain, there's not a lot we can do. */
    s = socket(PF_INET, SOCK_DGRAM, 0); /* any sort of IP socket will do */
    if (s == -1) {
        perror("socket");
        exit(1);
    }
    strncpy(ifr.ifr_name, options.interface, IFNAMSIZ);
    ifr.ifr_hwaddr.sa_family = AF_UNSPEC;
    if (ioctl(s, SIOCGIFHWADDR, &ifr) == -1) {
        perror("ioctl(SIOCGIFHWADDR)");
        exit(1);
    }
    close(s);
    memcpy(if_hw_addr, ifr.ifr_hwaddr.sa_data, 6);
    fprintf(stderr, "MAC address is:");
    for (s = 0; s < 6; ++s)
        fprintf(stderr, "%c%02x", s ? ':' : ' ', (unsigned int)if_hw_addr[s]);
    fprintf(stderr, "\n");
    
    resolver_initialise();

    pd = pcap_open_live(options.interface, CAPTURE_LENGTH, options.promiscuous, 1000, errbuf);
    if(pd == NULL) { 
        fprintf(stderr, "pcap_open_live(%s): %s\n", options.interface, errbuf); 
        exit(1);
    }
    dlt = pcap_datalink(pd);
    if(dlt == DLT_EN10MB) {
        packet_handler = handle_eth_packet;
    }
    else if(dlt == DLT_RAW) {
        packet_handler = handle_raw_packet;
    } 
    else {
        fprintf(stderr, "Unsupported datalink type: %d\n"
                "Please email pdw@ex-parrot.com, quoting the datalink type and what you were\n"
                "trying to do at the time\n.", dlt);
        exit(1);
    }

    if (options.filtercode) {
        str = xmalloc(strlen(options.filtercode) + sizeof "() and ip");
        sprintf(str, "(%s) and ip", options.filtercode);
    }
    if (pcap_compile(pd, &F, str, 1, 0) == -1) {
        fprintf(stderr, "pcap_compile(%s): %s\n", str, pcap_geterr(pd));
        exit(1);
        return;
    }
    if (pcap_setfilter(pd, &F) == -1) {
        fprintf(stderr, "pcap_setfilter: %s\n", pcap_geterr(pd));
        exit(1);
        return;
    }
    if (options.filtercode)
        xfree(str);

}

/* packet_loop:
 * Worker function for packet capture thread. */
void packet_loop(void* ptr) {
    pcap_loop(pd,0,(pcap_handler)packet_handler,NULL);
}


/* main:
 * Entry point. See usage(). */
int main(int argc, char **argv) {
    pthread_t thread;
    struct sigaction sa = {0};

    options_read(argc, argv);
    
    sa.sa_handler = finish;
    sigaction(SIGINT, &sa, NULL);

    pthread_mutex_init(&tick_mutex, NULL);

    packet_init();

    init_history();

    ui_init();

    pthread_create(&thread, NULL, (void*)&packet_loop, NULL);

    ui_loop();

    pthread_cancel(thread);

    ui_finish();
    
    return 0;
}
