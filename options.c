/*
 * options.c:
 *
 *
 */

#include "config.h"

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "iftop.h"
#include "options.h"
#include "integers.h"

#if !defined(HAVE_INET_ATON) && defined(HAVE_INET_PTON)
#   define inet_aton(a, b)  inet_pton(AF_INET, (a), (b))
#endif

options_t options;

char optstr[] = "+i:f:nNF:hpbBPm:c:";

/* Global options. */

/* Selecting an interface on which to listen: */

/* This is a list of interface name prefixes which are `bad' in the sense
 * that they don't refer to interfaces of external type on which we are
 * likely to want to listen. We also compare candidate interfaces to lo. */
static char *bad_interface_names[] = {
            "lo:",
            "lo",
            "stf",     /* pseudo-device 6to4 tunnel interface */
            "gif",     /* psuedo-device generic tunnel interface */
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
    struct if_nameindex * nameindex;
    char *i = NULL;
    int j = 0;
    /* Use if_nameindex(3) instead? */

    nameindex = if_nameindex();
    if(nameindex == NULL) {
        return NULL;
    }

    while(nameindex[j].if_index != 0) {
        if (strcmp(nameindex[j].if_name, "lo") != 0 && !is_bad_interface_name(nameindex[j].if_name)) {
            i = xstrdup(nameindex[j].if_name);
            break;
        }
        j++;
    }
    if_freenameindex(nameindex);
    return i;
}

void options_set_defaults() {
    char *s;
    /* Should go through the list of interfaces, and find the first one which
     * is up and is not lo or dummy*. */
    options.interface = get_first_interface();
    if (!options.interface)
        options.interface = "eth0";

    options.filtercode = NULL;
    options.netfilter = 0;
    inet_aton("10.0.1.0", &options.netfilternet);
    inet_aton("255.255.255.0", &options.netfiltermask);

#ifdef NEED_PROMISCUOUS_FOR_OUTGOING
    options.promiscuous = 1;
    options.promiscuous_but_choosy = 1;
#else
    options.promiscuous = 0;
    options.promiscuous_but_choosy = 0;
#endif
    options.showbars = 1;

    options.bandwidth_in_bytes = 0;
    options.sort = OPTION_SORT_DIV2;
    options.screenfilter = NULL;
    options.freezeorder = 0;
    options.show_totals = 0;
    options.max_bandwidth = 0; /* auto */
    options.bar_interval = 1;
}

static void die(char *msg) {
    fprintf(stderr, "%s", msg);
    exit(1);
}

static void set_max_bandwidth(char* arg) {
    char* units;
    long long mult = 1;
    long long value;
    units = arg + strspn(arg, "0123456789");
    if(strlen(units) > 1) {
        die("Invalid units\n");
    }
    if(strlen(units) == 1) {
        if(*units == 'k' || *units == 'K') {
            mult = 1024;
        }
        else if(*units == 'm' || *units == 'M') {
            mult = 1024 * 1024;
        }
        else if(*units == 'g' || *units == 'G') {
            mult = 1024 * 1024 * 1024;
        }
    }
    *units = '\0';
    if(sscanf(arg, "%lld", &value) != 1) {
        die("Error reading max bandwidth\n");
    }
    options.max_bandwidth = value * mult;
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
              u_int32_t mm = 0xffffffffl;
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
"   -N                  don't convert port numbers to services\n"
"   -p                  run in promiscuous mode (show traffic between other\n"
"                       hosts on the same network segment)\n"
"   -b                  don't display a bar graph of traffic\n"
"   -B                  Display bandwidth in bytes\n"
"   -i interface        listen on named interface\n"
"   -f filter code      use filter code to select packets to count\n"
"                       (default: none, but only IP packets are counted)\n"
"   -F net/mask         show traffic flows in/out of network\n"
"   -P                  show ports as well as hosts\n"
"   -m limit            sets the upper limit for the bandwidth scale\n"
"   -c config file      specifies an alternative configuration file\n"
"\n"
"iftop, version " IFTOP_VERSION "\n"
"copyright (c) 2002 Paul Warren <pdw@ex-parrot.com> and contributors\n"
            );
}
