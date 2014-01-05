/*
 * tui.c:
 *
 * Based on ui.c from the original iftop sources.
 *
 * This user interface does not make use of curses. Instead, it prints its
 * output to STDOUT. This output is activated by providing the '-t' flag.
 *
 */

#include "config.h"

#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(HAVE_TERMIOS_H)
#include <termios.h>
#elif defined(HAVE_SGTTY_H) && defined(TIOCSETP)
#include <sys/ioctl.h>
#include <sgtty.h>
#elif defined(HAVE_TERMIO_H)
#include <sys/ioctl.h>
#include <termio.h>
#else
#include <stdlib.h>
#endif

#include "sorted_list.h"
#include "options.h"
#include "ui_common.h"

/* Width of the host column in the output */
#define PRINT_WIDTH 40


/*
 * UI print function
 */
void tui_print() {
  sorted_list_node* nn = NULL;
  char host1[HOSTNAME_LENGTH], host2[HOSTNAME_LENGTH];
  char buf0_10[10], buf1_10[10], buf2_10[10];
  int j;
  static char *label;
  static char *labellong;
  int l = 0;

  if (!label) {
    xfree(label);
    label = (char *)calloc(PRINT_WIDTH + 1, 1);
  }

  if (!labellong) {
    xfree(labellong);
    labellong = (char *)calloc(PRINT_WIDTH + 1 + 9, 1);
  }

  if (options.paused ) {
    return;
  }

  /* Headings */
  snprintf(label, PRINT_WIDTH, "%-*s", PRINT_WIDTH, "Host name (port/service if enabled)");
  printf("%s %s     %10s %10s %10s %10s\n", "   #", label, "last 2s", "last 10s", "last 40s", "cumulative");

  /* Divider line */
  for (j = 0; j < PRINT_WIDTH + 52; j++) {
    printf("-");
  }
  printf("\n");

  /* Traverse the list of all connections */
  while((nn = sorted_list_next_item(&screen_list, nn)) != NULL && l < options.num_lines) {
    /* Increment the line counter */
    l++;

    /* Get the connection information */
    host_pair_line* screen_line = (host_pair_line*)nn->data;

    /* Assemble host information */
    sprint_host(host1, screen_line->ap.af, &(screen_line->ap.src6), screen_line->ap.src_port, screen_line->ap.protocol, PRINT_WIDTH, options.aggregate_src);
    sprint_host(host2, screen_line->ap.af, &(screen_line->ap.dst6), screen_line->ap.dst_port, screen_line->ap.protocol, PRINT_WIDTH, options.aggregate_dest);

    /* Send rate per connection */
    printf("%4d %s%s", l, host1, " =>");
    for(j = 0; j < HISTORY_DIVISIONS; j++) {
      readable_size(screen_line->sent[j], buf0_10, 10, 1024, options.bandwidth_in_bytes);
      printf(" %10s", buf0_10);
    }
    /* Cumulative sent data per connection */
    readable_size(screen_line->total_sent, buf0_10, 10, 1024, 1);
    printf(" %10s\n", buf0_10);

    /* Receive rate per connection */
    printf("     %s%s", host2, " <=");
    for(j = 0; j < HISTORY_DIVISIONS; j++) {
      readable_size(screen_line->recv[j], buf0_10, 10, 1024, options.bandwidth_in_bytes);
      printf(" %10s", buf0_10);
    }
    /* Cumulative received data per connection */
    readable_size(screen_line->total_recv, buf0_10, 10, 1024, 1);
    printf(" %10s\n", buf0_10);
  }

  /* Divider line */
  for (j = 0; j < PRINT_WIDTH + 52; j++) {
    printf("-");
  }
  printf("\n");

  /* Rate totals */
  snprintf(labellong, PRINT_WIDTH + 9, "%-*s", PRINT_WIDTH + 9, "Total send rate:");
  printf("%s ", labellong);
  for(j = 0; j < HISTORY_DIVISIONS; j++) {
    readable_size((((host_pair_line *)&totals)->sent[j]) , buf0_10, 10, 1024, options.bandwidth_in_bytes);
    printf("%10s%c", buf0_10, j == HISTORY_DIVISIONS - 1 ? '\n' : ' ');
  }

  snprintf(labellong, PRINT_WIDTH + 9, "%-*s", PRINT_WIDTH + 9, "Total receive rate:");
  printf("%s ", labellong);
  for(j = 0; j < HISTORY_DIVISIONS; j++) {
    readable_size((((host_pair_line *)&totals)->recv[j]) , buf0_10, 10, 1024, options.bandwidth_in_bytes);
    printf("%10s%c", buf0_10, j == HISTORY_DIVISIONS - 1 ? '\n' : ' ');
  }

  snprintf(labellong, PRINT_WIDTH + 9, "%-*s", PRINT_WIDTH + 9, "Total send and receive rate:");
  printf("%s ", labellong);
  for(j = 0; j < HISTORY_DIVISIONS; j++) {
    readable_size((((host_pair_line *)&totals)->sent[j] + ((host_pair_line *)&totals)->recv[j]) , buf0_10, 10, 1024, options.bandwidth_in_bytes);
    printf("%10s%c", buf0_10, j == HISTORY_DIVISIONS - 1 ? '\n' : ' ');
  }

  /* Divider line */
  for (j = 0; j < PRINT_WIDTH + 52; j++) {
    printf("-");
  }
  printf("\n");

  /* Peak traffic */
  snprintf(labellong, PRINT_WIDTH + 9, "%-*s", PRINT_WIDTH + 9, "Peak rate (sent/received/total):");
  readable_size(peaksent / RESOLUTION, buf0_10, 10, 1024, options.bandwidth_in_bytes);
  readable_size(peakrecv / RESOLUTION, buf1_10, 10, 1024, options.bandwidth_in_bytes);
  readable_size(peaktotal / RESOLUTION, buf2_10, 10, 1024, options.bandwidth_in_bytes);
  printf("%s %10s %10s %10s\n", labellong, buf0_10, buf1_10, buf2_10);

  /* Cumulative totals */
  snprintf(labellong, PRINT_WIDTH + 9, "%-*s", PRINT_WIDTH + 9, "Cumulative (sent/received/total):");
  readable_size(history_totals.total_sent, buf0_10, 10, 1024, 1);
  readable_size(history_totals.total_recv, buf1_10, 10, 1024, 1);
  readable_size(history_totals.total_recv + history_totals.total_sent, buf2_10, 10, 1024, 1);
  printf("%s %10s %10s %10s\n", labellong, buf0_10, buf1_10, buf2_10);

  /* Double divider line */
  for (j = 0; j < PRINT_WIDTH + 52; j++) {
    printf("=");
  }
  printf("\n\n");
}


/*
 * Text interface data structure initializations.
 */
void tui_init() {
  screen_list_init();
  screen_hash = addr_hash_create();
  service_hash = serv_hash_create();
  serv_hash_initialise(service_hash);

  printf("Listening on %s\n", options.interface);
}


/*
 * Tick function indicating screen refresh
 */
void tui_tick(int print) {
  if (print) {
    tui_print();
  }
}


/*
 * Main UI loop. Code any interactive character inputs here.
 */
void tui_loop() {
  int i;
  extern sig_atomic_t foad;

#if defined(HAVE_TERMIOS_H)
  struct termios new_termios, old_termios;

  tcgetattr(STDIN_FILENO, &old_termios);
  new_termios = old_termios;
  new_termios.c_lflag &= ~(ICANON|ECHO);
  new_termios.c_cc[VMIN] = 1;
  new_termios.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
#elif defined(HAVE_SGTTY_H) && defined(TIOCSETP)
  struct sgttyb new_tty, old_tty;

  ioctl(STDIN_FILENO, TIOCGETP, &old_tty);
  new_tty = old_tty;
  new_tty.sg_flags &= ~(ICANON|ECHO);
  ioctl(STDIN_FILENO, TIOCSETP, &new_tty);
#elif defined(HAVE_TERMIO_H)
  struct termio new_termio, old_termio;

  ioctl(0, TCGETA, &old_termio);
  new_termio = old_termio;
  new_termio.c_lflag &= ~(ICANON|ECHO);
  new_termio.c_cc[VMIN] = 1;
  new_termio.c_cc[VTIME] = 0;
  ioctl(0, TCSETA, &new_termio);
#else
  system("/bin/stty cbreak -echo >/dev/null 2>&1");
#endif

  while ((i = getchar()) != 'q' && foad == 0) {
    switch (i) {
      case 'u':
        tick(1);
        break;
      case 'n':
        options.dnsresolution ^= 1;
        printf("DNS resolution is %s.\n\n", options.dnsresolution ? "ON" : "OFF");
        tick(1);
        break;
      case 'N':
        options.portresolution ^= 1;
        printf("Port resolution is %s.\n\n", options.portresolution ? "ON" : "OFF");
        tick(1);
        break;
      case 's':
        options.aggregate_src ^= 1;
        printf("%s source host\n\n", options.aggregate_src ? "Hide" : "Show");
        tick(1);
        break;
      case 'd':
        options.aggregate_dest ^= 1;
        printf("%s destination host\n\n", options.aggregate_dest ? "Hide" : "Show");
        tick(1);
        break;
      case 'S':
        if (options.showports == OPTION_PORTS_OFF) {
          options.showports = OPTION_PORTS_SRC;
        }
        else if (options.showports == OPTION_PORTS_DEST) {
          options.showports = OPTION_PORTS_ON;
        }
        else if(options.showports == OPTION_PORTS_ON) {
          options.showports = OPTION_PORTS_DEST;
        }
        else {
          options.showports = OPTION_PORTS_OFF;
        }
        printf("Showing ports:%s%s%s%s.\n\n",
          options.showports == OPTION_PORTS_SRC ? " src" : "",
          options.showports == OPTION_PORTS_DEST ? " dest" : "",
          options.showports == OPTION_PORTS_ON ? " both" : "",
          options.showports == OPTION_PORTS_OFF ? " none" : "");
        tick(1);
        break;
      case 'D':
        if (options.showports == OPTION_PORTS_OFF) {
          options.showports = OPTION_PORTS_DEST;
        }
        else if (options.showports == OPTION_PORTS_SRC) {
          options.showports = OPTION_PORTS_ON;
        }
        else if(options.showports == OPTION_PORTS_ON) {
          options.showports = OPTION_PORTS_SRC;
        }
        else {
          options.showports = OPTION_PORTS_OFF;
        }
        printf("Showing ports:%s%s%s%s.\n\n",
          options.showports == OPTION_PORTS_SRC ? " src" : "",
          options.showports == OPTION_PORTS_DEST ? " dest" : "",
          options.showports == OPTION_PORTS_ON ? " both" : "",
          options.showports == OPTION_PORTS_OFF ? " none" : "");
        tick(1);
        break;
      case 'p':
        options.showports =
         (options.showports == OPTION_PORTS_OFF) ?
          OPTION_PORTS_ON :
          OPTION_PORTS_OFF;
        printf("Showing ports:%s%s%s%s.\n\n",
          options.showports == OPTION_PORTS_SRC ? " src" : "",
          options.showports == OPTION_PORTS_DEST ? " dest" : "",
          options.showports == OPTION_PORTS_ON ? " both" : "",
          options.showports == OPTION_PORTS_OFF ? " none" : "");
        tick(1);
        break;
      case 'P':
        options.paused ^= 1;
        if (options.paused) {
          printf("Pausing... press 'P' again to continue.\n");
        }
	else {
	  printf("Continuing.\n\n");
	  tick(1);
	}
	break;
      case 'o':
        options.freezeorder ^= 1;
        printf("Order %s.\n\n", options.freezeorder ? "frozen" : "unfrozen");
	tick(1);
	break;
      case '1':
        options.sort = OPTION_SORT_DIV1;
        printf("Sorting by column 1.\n\n");
	tick(1);
        break;
      case '2':
        options.sort = OPTION_SORT_DIV2;
        printf("Sorting by column 2.\n\n");
        tick(1);
        break;
      case '3':
        options.sort = OPTION_SORT_DIV3;
        printf("Sorting by column 3.\n\n");
        tick(1);
        break;
      case '<':
        options.sort = OPTION_SORT_SRC;
        printf("Sorting by column source.\n\n");
        tick(1);
        break;
      case '>':
        options.sort = OPTION_SORT_DEST;
        printf("Sorting by column destination.\n\n");
        tick(1);
        break;
      default:
        break;
    }
  }

#if defined(HAVE_TERMIOS_H)
  tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
#elif defined(HAVE_SGTTY_H) && defined(TIOCSETP)
  ioctl(0, TIOCSETP, &old_tty);
#elif defined(HAVE_TERMIO_H)
  ioctl(0, TCSETA, &old_termio);
#else
  system("/bin/stty -cbreak echo >/dev/null 2>&1");
#endif
}

