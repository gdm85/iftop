/*
 * ui.c:
 *
 */

#include <ctype.h>
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
#include "options.h"

#define HOSTNAME_LENGTH 256

#define HISTORY_DIVISIONS   3
#define BARGRAPH_INTERVAL   1   /* which division used for bars. */

/* 1, 15 and 60 seconds */
int history_divs[HISTORY_DIVISIONS] = {1, 5, 20};


typedef struct host_pair_line_tag {
    addr_pair* ap;
    long recv[HISTORY_DIVISIONS];
    long sent[HISTORY_DIVISIONS];
} host_pair_line;


extern hash_type* history;
extern int history_pos;
extern int history_len;

extern options_t options ;

void ui_finish();

int screen_line_compare(void* a, void* b) {
    int i;
    host_pair_line* aa = (host_pair_line*)a;
    host_pair_line* bb = (host_pair_line*)b;
    /* Ignore the first division so that stuff doesn't jump around too much */
    for(i = 1; i < HISTORY_DIVISIONS; i++) {
        if(aa->recv[i] + aa->sent[i] != bb->recv[i] + bb->sent[i]) {
            return(aa->recv[i] + aa->sent[i] < bb->recv[i] + bb->sent[i]);
        }
    }
    return 1;
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

static void draw_bar_scale(int* y) {
    int i;
    if(options.showbars) {
        /* Draw bar graph scale on top of the window. */
        mvhline(*y + 1, 0, 0, COLS);
        for (i = min_rate; i <= max_rate; i *= 10) {
            char s[40], *p;
            int x;
            readable_size(i, s, sizeof s, 1000);
            p = s + strspn(s, " ");
            x = get_bar_length(i);
            mvaddch(*y + 1, x, ACS_BTEE);
            if (x + strlen(p) >= COLS)
                x = COLS - strlen(p);
            mvaddstr(*y, x, p);
        }
        mvaddch(*y + 1, 0, ACS_LLCORNER);
        *y += 2;
    }
    else {
        mvhline(*y, 0, 0, COLS);
        *y += 1;
    }
}

int history_length(const int d) {
    if (history_len < history_divs[d])
        return history_len * RESOLUTION;
    else
        return history_divs[d] * RESOLUTION;
}

void draw_line_totals(int y, host_pair_line* line) {
    int j, t, L;
    char buf[10];
    int x = (COLS - 8 * HISTORY_DIVISIONS);

    for(j = 0; j < HISTORY_DIVISIONS; j++) {
        t = history_length(j);
        readable_size(8 * line->sent[j] / t, buf, 10, 1024);
        mvaddstr(y, x, buf);

        readable_size(8 * line->recv[j] / t, buf, 10, 1024);
        mvaddstr(y+1, x, buf);
        x += 8;
    }
    
    if(options.showbars) {
        t = history_length(BARGRAPH_INTERVAL);
        mvchgat(y, 0, -1, A_NORMAL, 0, NULL);
        L = get_bar_length(8 * line->sent[BARGRAPH_INTERVAL] / t);
        if (L > 0)
            mvchgat(y, 0, L + 1, A_REVERSE, 0, NULL);

        mvchgat(y+1, 0, -1, A_NORMAL, 0, NULL);
        L = get_bar_length(8 * line->recv[BARGRAPH_INTERVAL] / t);
        if (L > 0)
            mvchgat(y+1, 0, L + 1, A_REVERSE, 0, NULL);
    }
}

void draw_totals(host_pair_line* totals) {
    /* Draw rule */
    int y = LINES - 3;
    mvhline(y, 0, 0, COLS);
    y++;
    draw_line_totals(y, totals);
}

extern history_type history_totals;


void ui_print() {
    hash_node_type* n = NULL;
    sorted_list_node* nn = NULL;
    char hostname[HOSTNAME_LENGTH];
    static char *line;
    static int lcols;
    int peaksent = 0, peakrecv = 0, peaktotal = 0;
    int y = 1;
    int i;
    sorted_list_type screen_list;
    host_pair_line totals;

    if (!line || lcols != COLS) {
        xfree(line);
        line = calloc(COLS + 1, 1);
    }

    screen_list.compare = &screen_line_compare;
    sorted_list_initialise(&screen_list);

    clear();
    //erase();
    move(0, 0);
    attron(A_REVERSE);
    addstr(" Q ");
    attroff(A_REVERSE);
    addstr(" quit ");
    attron(A_REVERSE);
    addstr(" R ");
    attroff(A_REVERSE);
    addstr(options.dnsresolution ? " name resolution off "
                         : " name resolution on  ");
    draw_bar_scale(&y);

    memset(&totals, 0, sizeof totals);

    while(hash_next_item(history, &n) == HASH_STATUS_OK) {
        history_type* d = (history_type*)n->rec;
        host_pair_line* screen_line;
        int i;
        int tsent, trecv;
        tsent = trecv = 0;
        
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


    /* Screen layout: we have 2 * HISTORY_DIVISIONS 6-character wide history
     * items, and so can use COLS - 12 * HISTORY_DIVISIONS to print the two
     * host names. */

    while((nn = sorted_list_next_item(&screen_list, nn)) != NULL) {
        int x = 0, L;
        host_pair_line* screen_line = (host_pair_line*)nn->data;

        if(y < LINES - 4) {
            
            L = (COLS - 8 * HISTORY_DIVISIONS - 4) / 2;
            if(L > sizeof hostname) {
                L = sizeof hostname;
            }

            if (options.dnsresolution)
                resolve(&screen_line->ap->src, hostname, L);
            else
                strcpy(hostname, inet_ntoa(screen_line->ap->src));
            sprintf(line, "%-*s", L, hostname);
            mvaddstr(y, x, line);
            x += L;

            mvaddstr(y, x, " => ");
            mvaddstr(y+1, x, " <= ");

            x += 4;
            if (options.dnsresolution)
                resolve(&screen_line->ap->dst, hostname, L);
            else
                strcpy(hostname, inet_ntoa(screen_line->ap->dst));
            sprintf(line, "%-*s", L, hostname);
            mvaddstr(y, x, line);
            
            draw_line_totals(y, screen_line);

        }
        y += 2;
        free(screen_line);
    }


    y = LINES - 2;

    mvaddstr(y, 0, "total: ");
    mvaddstr(y+1, 0, " peak: ");

    readable_size((totals.recv[0] + totals.sent[0]) * 8 / RESOLUTION, line, 10, 1024);
    mvaddstr(y, 8, line);

    readable_size(peaktotal * 8 / RESOLUTION, line, 10, 1024);
    mvaddstr(y+1, 8, line);

    readable_size((peakrecv + peaksent) * 8 / RESOLUTION, line, 10, 1024);
    mvaddstr(y+1, 8, line);

    mvaddstr(y, 18, "TX: ");
    mvaddstr(y+1, 18, "RX: ");

    readable_size(history_totals.total_sent / RESOLUTION, line, 10, 1024);
    mvaddstr(y, 22, line);

    readable_size(history_totals.total_recv / RESOLUTION, line, 10, 1024);
    mvaddstr(y+1, 22, line);

    mvaddstr(y, 33, "peaks: ");
    /* mvaddstr(y+1, 24, "recv:       "); */

    readable_size(peaksent * 8 / RESOLUTION, line, 10, 1024);
    mvaddstr(y, 39, line);

    readable_size(peakrecv * 8 / RESOLUTION, line, 10, 1024);
    mvaddstr(y+1, 39, line);

    mvaddstr(y, COLS - 8 * HISTORY_DIVISIONS - 8, "totals:");

    draw_totals(&totals);
    
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
        int i;
        i = toupper(getch());
        switch (i) {
            case 'Q':
                foad = 1;
                break;

            case 'R':
                options.dnsresolution = !options.dnsresolution;
                break;

	        case 'B':
                options.showbars = !options.showbars; 
                break;
        }
        tick();
    }
}

void ui_finish() {
    endwin();
}
