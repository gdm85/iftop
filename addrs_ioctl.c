/*
 * addrs_ioctl.c:
 *
 * Provides the get_addrs_ioctl() function for use on systems that
 * support a simple socket ioctl for acquiring low-level ethernet
 * information about interfaces.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>

#include "iftop.h"

/*
 * This function identifies the IP address and ethernet address for the requested
 * interface
 *
 * This function returns -1 on catastrophic failure, or a bitwise OR of the
 * following values:
 *
 * 1 - Was able to get the ethernet address
 * 2 - Was able to get the IP address
 *
 * This function should return 3 if all information was found
 */

int
get_addrs_ioctl(char *interface, char if_hw_addr[], struct in_addr *if_ip_addr)
{
  int s;
  struct ifreq ifr = {};
  int got_hw_addr = 0;
  int got_ip_addr = 0;

  /* -- */

  s = socket(PF_INET, SOCK_DGRAM, 0); /* any sort of IP socket will do */

  if (s == -1) {
    perror("socket");
    return -1;
  }

  fprintf(stderr,"interface: %s\n", interface);

  memset(if_hw_addr, 0, 6);
  strncpy(ifr.ifr_name, interface, IFNAMSIZ);

#ifdef SIOCGIFHWADDR
  if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
    fprintf(stderr, "Error getting hardware address for interface: %s\n", interface); 
    perror("ioctl(SIOCGIFHWADDR)");
  }
  else {
    memcpy(if_hw_addr, ifr.ifr_hwaddr.sa_data, 6);
    got_hw_addr = 1;
  }
#else
  fprintf(stderr, "Cannot obtain hardware address on this platform\n");
#endif
  
  /* Get the IP address of the interface */
#ifdef SIOCGIFADDR
  (*(struct sockaddr_in *) &ifr.ifr_addr).sin_family = AF_INET;
  if (ioctl(s, SIOCGIFADDR, &ifr) < 0) {
    fprintf(stderr, "Unable to get IP address for interface: %s\n", interface); 
    perror("ioctl(SIOCGIFADDR)");
  }
  else {
    memcpy(if_ip_addr, &((*(struct sockaddr_in *) &ifr.ifr_addr).sin_addr), sizeof(struct in_addr));
    got_ip_addr = 2;
  }
#else
  fprintf(stderr, "Cannot obtain IP address on this platform\n");
#endif
  
  close(s);

  return got_hw_addr + got_ip_addr;
}
