/*
 * iftop.c:
 *
 */

#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <curses.h>
#include <signal.h>
#include <string.h>

#include "iftop.h"
#include "addr_hash.h"
#include "resolver.h"
#include "ui.h"



hash_type* history;
time_t last_timestamp;
int history_pos = 0;
int history_len = 1;
pthread_mutex_t tick_mutex;



#define CAPTURE_LENGTH 512

void init_history() {
    history = addr_hash_create();
    last_timestamp = time(NULL);
}

history_type* history_create() {
    history_type* h;
    if ((h = calloc(1, sizeof(history_type))) == 0) {
        fprintf (stderr, "out of memory (history_type)\n");
        exit(1);
    }
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
        struct ip* iptr = (struct ip*)(packet + sizeof(struct ether_header));
        history_type* ht;
        addr_pair ap;

        if(iptr->ip_src.s_addr < iptr->ip_dst.s_addr) {
            ap.src = iptr->ip_src;
            ap.dst = iptr->ip_dst;
        }
        else {
            ap.src = iptr->ip_dst;
            ap.dst = iptr->ip_src;
        }

	/* Add the address to be resolved */
        resolve(&iptr->ip_dst, NULL, 0);

        if(hash_find(history, &ap, (void**)&ht) == HASH_STATUS_KEY_NOT_FOUND) {
            ht = history_create();
            hash_insert(history, &ap, ht);
        }

        /* Update record */
        ht->last_write = history_pos;
        if(iptr->ip_src.s_addr < iptr->ip_dst.s_addr) {
            ht->recv[history_pos] += ntohs(iptr->ip_len);
        }
        else {
            ht->sent[history_pos] += ntohs(iptr->ip_len);
        }

    }
    fflush(stdout);
}

/*
 * packet capture thread
 */
void packet_loop(void* ptr) {
    char errbuf[PCAP_ERRBUF_SIZE];
    char* device;
    pcap_t* pd;

    resolver_initialise();

    device = pcap_lookupdev(errbuf);
    printf("Device: %s\n",device);
    pd = pcap_open_live(device,CAPTURE_LENGTH,1,1000,errbuf);
    if(pd == NULL) { 
        printf("pcap_open_live(): %s\n",errbuf); 
        exit(1); 
    }
    printf("Begin loop\n");
    init_history();
    pcap_loop(pd,0,(pcap_handler)handle_packet,NULL);
    printf("end loop\n");
}

static void finish(int sig)
{
    ui_finish();
    exit(0);
}

int main(int argc, char **argv) {
    pthread_t thread;

    (void) signal(SIGINT, finish);      /* arrange interrupts to terminate */

    pthread_mutex_init(&tick_mutex, NULL);

    pthread_create(&thread, NULL, (void*)&packet_loop, NULL);

    ui_loop();

    return 0;
}
