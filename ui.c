/*
 * ui.c:
 *
 */

#include <curses.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "addr_hash.h"
#include "iftop.h"
#include "resolver.h"
#include "sorted_list.h"

#define HOSTNAME_LENGTH 256

#define HISTORY_DIVISIONS 3

/* 3, 15 and 60 seconds */
int history_divs[HISTORY_DIVISIONS] = {1, 5, 20};


typedef struct host_pair_line_tag {
    addr_pair* ap;
    long recv[HISTORY_DIVISIONS];
    long sent[HISTORY_DIVISIONS];
} host_pair_line;


extern hash_type* history;
extern int history_pos;
extern int history_len;

void ui_finish();

int screen_line_compare(void* a, void* b) {
    host_pair_line* aa = (host_pair_line*)a;
    host_pair_line* bb = (host_pair_line*)b;
    return(aa->recv[0] + aa->sent[0] < bb->recv[0] + bb->sent[0]);
}

void readable_size(float n, char* buf, int bsize, int ksize) {
    if(n >= 100000) {
       snprintf(buf, bsize, " %4.1fM", n / (ksize * ksize)); 
    }
    else if(n >= 1000) {
       snprintf(buf, bsize, " %4.1fK", n / ksize); 
    }
    else {
       snprintf(buf, bsize, " %4.0fb", n ); 
    }
}

/* Maximum and minimum rate which we plot on the bar chart. */
static int min_rate = 1;                /* 1 byte/s */
static int max_rate = (10000000);   /* 10 Mbit/s */

static int get_bar_length(const int rate) {
    float l;
    if (rate <= 0)
        return 0;
    l = (log(rate) - log(min_rate)) / (log(max_rate) - log(min_rate));
    return (l * COLS);
}

static void draw_bar_scale(void) {
    int i;
    /* Draw bar graph scale on top of the window. */
    mvhline(1, 0, 0, COLS);
    for (i = min_rate; i <= max_rate; i *= 10) {
        char s[40], *p;
        int x;
        readable_size(i, s, sizeof s, 1000);
        p = s + strspn(s, " ");
        x = get_bar_length(i);
        mvaddch(1, x, ACS_BTEE);
        if (x + strlen(p) >= COLS)
            x = COLS - strlen(p);
        mvaddstr(0, x, p);
    }
    mvaddch(1, 0, ACS_LLCORNER);
}

void ui_print() {
    hash_node_type* n = NULL;
    sorted_list_node* nn = NULL;
    char hostname[HOSTNAME_LENGTH];
    static char *line;
    static int lcols;
    int y = 2;
    sorted_list_type screen_list;

    if (!line || lcols != COLS) {
        xfree(line);
        line = calloc(COLS + 1, 1);
    }

    screen_list.compare = &screen_line_compare;
    sorted_list_initialise(&screen_list);

    erase();
    draw_bar_scale();

    while(hash_next_item(history, &n) == HASH_STATUS_OK) {
        history_type* d = (history_type*)n->rec;
        host_pair_line* screen_line;
        int i;
        
        screen_line = xcalloc(1, sizeof *screen_line);
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

    /* Screen layout: we have 2 * HISTORY_DIVISIONS 6-character wide history
     * items, and so can use COLS - 12 * HISTORY_DIVISIONS to print the two
     * host names. */

    while((nn = sorted_list_next_item(&screen_list, nn)) != NULL) {
        int x = 0, j, L, t;
        host_pair_line* screen_line = (host_pair_line*)nn->data;

        L = (COLS - 8 * HISTORY_DIVISIONS - 4) / 2;
        if(L > sizeof hostname) {
            L = sizeof hostname;
        }

        resolve(&screen_line->ap->src, hostname, L);
        sprintf(line, "%-*s", L, hostname);
        mvaddstr(y, x, line);
        x += L;

        mvaddstr(y, x, " => ");
        mvaddstr(y+1, x, " <= ");

        x += 4;
        resolve(&screen_line->ap->dst, hostname, L);
        sprintf(line, "%-*s", L, hostname);
        mvaddstr(y, x, line);
        x += L;

        for(j = 0; j < HISTORY_DIVISIONS; j++) {
            if(history_len < history_divs[j]) {
                t = history_len * RESOLUTION; 
            } else {
                t = history_divs[j] * RESOLUTION;
            }
            readable_size(8 * screen_line->sent[j] / t, line, 10, 1024);
            mvaddstr(y, x, line);

            readable_size(8 * screen_line->recv[j] / t, line, 10, 1024);
            mvaddstr(y+1, x, line);
            x += 8;

        }

        /* Do some sort of primitive bar graph thing. */
        mvchgat(y, 0, -1, A_NORMAL, 0, NULL);
        L = get_bar_length(8 * screen_line->sent[0] / history_divs[0]);
        if (L > 0)
            mvchgat(y, 0, L, A_REVERSE, 0, NULL);

        mvchgat(y+1, 0, -1, A_NORMAL, 0, NULL);
        L = get_bar_length(8 * screen_line->recv[0] / history_divs[0]);
        if (L > 0)
            mvchgat(y+1, 0, L, A_REVERSE, 0, NULL);
        
        y+=2;
        free(screen_line);
    }
    refresh();

    sorted_list_destroy(&screen_list);
}

void ui_loop() {
    pthread_mutex_t tick_wait_mutex;
    pthread_cond_t tick_wait_cond;
    extern sig_atomic_t foad;

    (void) initscr();      /* initialize the curses library */
    keypad(stdscr, TRUE);  /* enable keyboard mapping */
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) noecho();       /* don't echo input */
    halfdelay(2);

    erase();
    
    pthread_mutex_init(&tick_wait_mutex, NULL);
    pthread_cond_init(&tick_wait_cond, NULL);
    while(foad == 0) {
        if(getch() == 'q') {
            foad = 1;

        }
        tick();
    }
}

void ui_finish() {
    endwin();
}
