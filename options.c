/*
 * options.c:
 *
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "options.h"

options_t options;

char optstr[] = "+i:f:n:dhp";

/* Global options. */

void set_defaults() {
    options.interface = "eth0";
    options.filtercode = NULL;
    options.netfilter = 0;
    inet_aton("10.0.1.0", &options.netfilternet);
    inet_aton("255.255.255.0", &options.netfiltermask);
    options.dnsresolution = 1;
    options.promiscuous = 0;
    options.showbars = 1;
}

void die(char *msg) {
    fprintf(stderr, msg);
    exit(1);
}

void set_net_filter(char* arg) {
    char* mask;

    mask = strstr(arg, "/");
    if(mask == NULL) {
        die("Could not parse net/mask\n");
    }
    *mask = '\0';
    mask++;
    if(inet_aton(arg, &options.netfilternet) == 0) {
        die("Invalid network address\n");
    }
    if(inet_aton(mask, &options.netfiltermask) == 0) {
        die("Invalid network mask\n");
    }

    options.netfilter = 1;

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
"   -n network/netmask  show traffic flows in/out of network\n"
"\n"
"iftop, version " IFTOP_VERSION "copyright (c) 2002 Paul Warren <pdw@ex-parrot.com>\n"
            );
}

void options_read(int argc, char **argv) {
    int opt;

    set_defaults();

    opterr = 0;
    while ((opt = getopt(argc, argv, optstr)) != -1) {
        switch (opt) {
            case 'h':
                usage(stdout);
                exit(0);

            case 'd':
                options.dnsresolution = 0;
                break;

            case 'i':
                options.interface = optarg;
                break;

            case 'f':
                options.filtercode = optarg;
                break;

            case 'p':
                options.promiscuous = 1;
                break;

            case 'n':
                set_net_filter(optarg);
                break;

            case '?':
                fprintf(stderr, "iftop: unknown option -%c\n", optopt);
                usage(stderr);
                exit(1);

            case ':':
                fprintf(stderr, "iftop: option -%c requires an argument\n", optopt);
                usage(stderr);
                exit(1);
        }
    }

}
