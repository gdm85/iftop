/*
 * options.c:
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>

#include "iftop.h"
#include "options.h"

options_t options;

char optstr[] = "+i:f:n:NhpbBP";

/* Global options. */

/* Selecting an interface on which to listen: */

/* This is a list of interface name prefixes which are `bad' in the sense
 * that they don't refer to interfaces of external type on which we are
 * likely to want to listen. We also compare candidate interfaces to lo. */
static char *bad_interface_names[] = {
            "lo:",
            "dummy",
            "vmnet",
            NULL        /* last entry must be NULL */
        };

static int is_bad_interface_name(char *i) {
    char **p;
    for (p = bad_interface_names; *p; ++p)
        if (strncmp(i, *p, strlen(*p)) == 0)
            return 1;
    return 0;
}

/* This finds the first interface which is up and is not the loopback
 * interface or one of the interface types listed in bad_interface_names. */
static char *get_first_interface(void) {
    int s, size = 1;
    struct ifreq *ifr;
    struct ifconf ifc = {0};
    char *i = NULL;
    /* Use if_nameindex(3) instead? */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        return NULL;
    ifc.ifc_len = sizeof *ifr;
    do {
        ++size;
        ifc.ifc_req = xrealloc(ifc.ifc_req, size * sizeof *ifc.ifc_req);
        ifc.ifc_len = size * sizeof *ifc.ifc_req;
        if (ioctl(s, SIOCGIFCONF, &ifc) == -1) {
            perror("SIOCGIFCONF");
            return NULL;
        }
    } while (size * sizeof *ifc.ifc_req <= ifc.ifc_len);
    /* Ugly. */
    for (ifr = ifc.ifc_req; (char*)ifr < (char*)ifc.ifc_req + ifc.ifc_len; ++ifr) {
        if (strcmp(ifr->ifr_name, "lo") != 0 && !is_bad_interface_name(ifr->ifr_name)
            && ioctl(s, SIOCGIFFLAGS, ifr) == 0 && ifr->ifr_flags & IFF_UP) {
            i = xstrdup(ifr->ifr_name);
            break;
        }
    }
    xfree(ifc.ifc_req);
    return i;
}

static void set_defaults() {
    /* Should go through the list of interfaces, and find the first one which
     * is up and is not lo or dummy*. */
    options.interface = get_first_interface();
    if (!options.interface)
        options.interface = "eth0";

    options.filtercode = NULL;
    options.netfilter = 0;
    inet_aton("10.0.1.0", &options.netfilternet);
    inet_aton("255.255.255.0", &options.netfiltermask);
    options.dnsresolution = 1;
    options.portresolution = 1;
    options.promiscuous = 0;
    options.showbars = 1;
    options.showports = OPTION_PORTS_OFF;
    options.aggregate_src = 0;
    options.aggregate_dest = 0;
    options.paused = 0;
    options.showhelp = 0;
    options.bandwidth_in_bytes = 0;
    options.sort = OPTION_SORT_DIV2;
    options.screenfilter = NULL;
    options.freezeorder = 0;
}

static void die(char *msg) {
    fprintf(stderr, msg);
    exit(1);
}

static void set_net_filter(char* arg) {
    char* mask;

    mask = strchr(arg, '/');
    if (mask == NULL) {
        die("Could not parse net/mask\n");
    }
    *mask = '\0';
    mask++;
    if (inet_aton(arg, &options.netfilternet) == 0)
        die("Invalid network address\n");
    /* Accept a netmask like /24 or /255.255.255.0. */
    if (mask[strspn(mask, "0123456789")] == '\0') {
        /* Whole string is numeric */
        int n;
        n = atoi(mask);
        if (n > 32) {
            die("Invalid netmask");
        }
        else {
            if(n == 32) {
              /* This needs to be special cased, although I don't fully 
               * understand why -pdw 
               */
              options.netfiltermask.s_addr = htonl(0xffffffffl);
            }
            else {
              uint32_t mm = 0xffffffffl;
              mm >>= n;
              options.netfiltermask.s_addr = htonl(~mm);
            }
        }
    } 
    else if (inet_aton(mask, &options.netfiltermask) == 0) {
        die("Invalid netmask\n");
    }
    options.netfilternet.s_addr = options.netfilternet.s_addr & options.netfiltermask.s_addr;

    options.netfilter = 1;

}

/* usage:
 * Print usage information. */
static void usage(FILE *fp) {
    fprintf(fp,
"iftop: display bandwidth usage on an interface by host\n"
"\n"
"Synopsis: iftop -h | [-npbBP] [-i interface] [-f filter code] [-N net/mask]\n"
"\n"
"   -h                  display this message\n"
"   -n                  don't do hostname lookups\n"
"   -p                  run in promiscuous mode (show traffic between other\n"
"                       hosts on the same network segment)\n"
"   -b                  don't display a bar graph of traffic\n"
"   -B                  Display bandwidth in bytes\n"
"   -i interface        listen on named interface\n"
"   -f filter code      use filter code to select packets to count\n"
"                       (default: none, but only IP packets are counted)\n"
"   -N net/mask         show traffic flows in/out of network\n"
"   -P                  show ports as well as hosts\n"
"\n"
"iftop, version " IFTOP_VERSION "\n"
"copyright (c) 2002 Paul Warren <pdw@ex-parrot.com> and contributors\n"
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

            case 'n':
                options.dnsresolution = 0;
                break;

            case 'i':
                options.interface = optarg;
                break;

            case 'f':
                options.filtercode = xstrdup(optarg);
                break;

            case 'p':
                options.promiscuous = 1;
                break;

            case 'P':
                options.showports = OPTION_PORTS_ON;
                break;

            case 'N':
                set_net_filter(optarg);
                break;

            case 'b':
                options.showbars = 0;
                break;

            case 'B':
                options.bandwidth_in_bytes = 1;
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

    if (optind != argc)
        fprintf(stderr, "iftop: warning: ignored arguments following options\n");
}
