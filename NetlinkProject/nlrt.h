/*
 * =====================================================================================
 *
 *       Filename:  nlrt.h
 *
 *    Description:  This file implements the routines for routing table management
 *
 *        Version:  1.0
 *        Created:  11/22/2019 09:08:14 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Juniper Networks (https://csepracticals.wixsite.com/csepracticals), sachinites@gmail.com
 *        Company:  Juniper Networks
 *
 *        This file is part of the Netlink Sockets Course distribution (https://github.com/sachinites) 
 *        Copyright (c) 2019 Abhishek Sagar.
 *        This program is free software: you can redistribute it and/or modify it under the terms of the GNU General 
 *        Public License as published by the Free Software Foundation, version 3.
 *        
 *        This program is distributed in the hope that it will be useful, but
 *        WITHOUT ANY WARRANTY; without even the implied warranty of
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *        General Public License for more details.
 *
 *        visit website : https://csepracticals.wixsite.com/csepracticals for more courses and projects
 *                                  
 * =====================================================================================
 */

#ifndef __NLRT__
#define __NLRT__

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <stdint.h>  /*for using uint32_t*/
#undef __KERNEL__
#include "common.h"

/*A structure to send Netlink Request to Routing
 * Kernel Subsystem. We need to format the request from
 * user-space in the format as prescribed by this structure*/
typedef struct nl_rt_request_{

    struct nlmsghdr nlh;
    struct rtmsg r;
    char buf[MAX_PAYLOAD];
} nl_rt_request_t;

#if 0
/* The IP Address need to be specified in this format to be
 * sent to kernel subsystems*/
typedef struct addr_{

    uint8_t family; /*AF_INET for IPV4. AF_INET6 for ipv6*/
    uint8_t mask;  /*mask value (0-32) for ipv4. (0-128 for ipv6)*/
    /* To store the address, addr could be IPV4 (=4B) or IPV6
     * ( = 16B). Let us take 16B as it could store IPV4 as well*/
    uint8_t addr[sizeof(struct in6_addr)];
} addr_t;
#endif

void
nl_route_add_from_user_input(int sock_fd);

nl_rt_request_t *
nl_route_add(int sock_fd, uint32_t dest,
          uint8_t mask, uint32_t gw_ip,
          uint32_t ifindex);

#endif /* __NLRT__ */
