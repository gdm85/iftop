/*
 * ui.c:
 *
 */

#include <curses.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "addr_hash.h"
#include "iftop.h"
#include "resolver.h"
#include "sorted_list.h"

#define HOSTNAME_LENGTH 20

#define HISTORY_DIVISIONS 3

int history_divs[HISTORY_DIVISIONS] = {3, 20, 40};


typedef struct host_pair_line_tag {
    addr_pair* ap;
    long recv[HISTORY_DIVISIONS];
    long sent[HISTORY_DIVISIONS];
} host_pair_line;


extern hash_type* history;
extern int history_pos;
extern int history_len;

int screen_line_compare(void* a, void* b) {
    host_pair_line* aa = (host_pair_line*)a;
    host_pair_line* bb = (host_pair_line*)b;
    return(aa->recv[0] + aa->sent[0] < bb->recv[0] + bb->sent[0]);
}

void readable_size(float n, char* buf, int bsize) {
    if(n >= 102400) {
       snprintf(buf, bsize, " %4.1fM", n / (1024 * 1024)); 
    }
    else if(n >= 1024) {
       snprintf(buf, bsize, " %4.1fK", n / 1024); 
    }
    else {
       snprintf(buf, bsize, " %4.0fb", n ); 
    }
}

void ui_print() {
    hash_node_type* n = NULL;
    sorted_list_node* nn = NULL;
    char hostname[HOSTNAME_LENGTH];
    char line[80]; // FIXME
    int y = 2;
    sorted_list_type screen_list;

    screen_list.compare = &screen_line_compare;
    sorted_list_initialise(&screen_list);

    erase();

    while(hash_next_item(history, &n) == HASH_STATUS_OK) {
        history_type* d = (history_type*)n->rec;
        host_pair_line* screen_line;
        int i;
        
        screen_line = (host_pair_line*)calloc(1,sizeof(host_pair_line));
        screen_line->ap = (addr_pair*)n->key;
        
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

        sorted_list_insert(&screen_list, screen_line);
    }

    while((nn = sorted_list_next_item(&screen_list, nn)) != NULL) {
        int x = 0;
        int j;
        host_pair_line* screen_line = (host_pair_line*)nn->data;

        resolve(&screen_line->ap->src, hostname, HOSTNAME_LENGTH);
        sprintf(line, "%s ", hostname);
        mvaddstr(y, x, line);
        x += 20;

        resolve(&screen_line->ap->dst, hostname, HOSTNAME_LENGTH);
        sprintf(line ,"=> %10s ", hostname);
        mvaddstr(y, x, line);
        x += 24;

        for(j = 0; j < HISTORY_DIVISIONS; j++) {
            int t;
            if(history_len < history_divs[j]) {
                t = history_len * RESOLUTION; 
            }
            else {
                t = history_divs[j] * RESOLUTION;
            }
            readable_size(screen_line->sent[j] / t, line, 10);
            mvaddstr(y, x, line);
            x += strlen(line);

            readable_size(screen_line->recv[j] / t, line, 10);
            mvaddstr(y, x, line);
            x += strlen(line);
        }
        y++;
        free(screen_line);
    }
    refresh();

    sorted_list_destroy(&screen_list);
}

void ui_loop() {
    pthread_mutex_t tick_wait_mutex;
    pthread_cond_t tick_wait_cond;


    (void) initscr();      /* initialize the curses library */
    keypad(stdscr, TRUE);  /* enable keyboard mapping */
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) noecho();       /* don't echo input */


    pthread_mutex_init(&tick_wait_mutex, NULL);
    pthread_cond_init(&tick_wait_cond, NULL);
    while(1) {
        struct timespec t;
        t.tv_sec = time(NULL) + 1;
        t.tv_nsec = 0;

        pthread_cond_timedwait(&tick_wait_cond, &tick_wait_mutex, &t);
        //fprintf(stderr,"timeout tick\n");
        tick();
    }

}

void ui_finish() {
    endwin();
}
