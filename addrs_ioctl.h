/*
 * iftop.h:
 *
 */

#ifndef __ADDRS_IOCTL_H_ /* include guard */
#define __ADDRS_IOCTL_H_

int
get_addrs_ioctl(char *interface, u_int8_t if_hw_addr[], struct in_addr *if_ip_addr, struct in6_addr *if_ip6_addr);

#endif /* __ADDRS_IOCTL_H_ */
