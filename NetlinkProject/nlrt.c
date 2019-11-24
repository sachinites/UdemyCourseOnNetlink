/*
 * =====================================================================================
 *
 *       Filename:  nlrt.c
 *
 *    Description:  This file implements the routines for Routing Table Management through Netlink Sockets
 *
 *        Version:  1.0
 *        Created:  11/22/2019 09:12:12 PM
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

#include "nlrt.h"
#include <stdio.h>
#include <stdint.h>     /*for uint32_t*/
#include <memory.h>
#include <arpa/inet.h>  /*For inet_pton*/
#include <net/if.h>     /*for if_nametoindex*/
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>     /*for getpid*/

static void
prompt_user_for_rt_params(uint32_t *dest, 
                          uint32_t  *mask, 
                          uint32_t *gw_ip,
                          uint32_t *ifindex){

   char ip_addr[16];
   memset(ip_addr, 0, 16);
   printf("\t\tEnter Destination Address : ");
   scanf("%s", ip_addr);

   /*Convert address from A.B.C.D format to integer equivalent*/
   inet_pton(AF_INET, ip_addr, dest);
   *dest = htonl(*dest);

   printf("\t\tEnter mask [0-32]: ");
   scanf("%d", mask);

   memset(ip_addr, 0, 16);
   printf("\t\tEnter GateWay Address : ");
   scanf("%s", ip_addr);

   /*Convert address from A.B.C.D format to integer equivalent*/
   inet_pton(AF_INET, ip_addr, gw_ip);
   *gw_ip = htonl(*gw_ip);

   char if_name[32];
   memset(if_name, 0, sizeof(if_name));
   printf("\t\tEnter Interface Name : ");
   scanf("%s", if_name);
   *ifindex = if_nametoindex(if_name);   
}

void
nl_route_add_from_user_input(int sock_fd){

    /*Let's first ask the Route parameters from the user*/
    /*We need to ask four things : 
     * 1. Route
     * 2. Mask
     * 3. Gateway IP
        * 4. Outgoing interface name*/
     uint32_t dest = 0;
     uint32_t mask = 0;
     uint32_t gw_ip = 0;
     uint32_t ifindex = 0;
     prompt_user_for_rt_params(&dest, &mask, &gw_ip, &ifindex);

     nl_rt_request_t *nl_rt_request = 
        nl_route_add(sock_fd, dest, mask, gw_ip, ifindex);
     
     if(!nl_rt_request){
        return;
     }

     nlmsg_dump(&nl_rt_request->nlh);
     int rc = send(sock_fd, &nl_rt_request->nlh, 
            nl_rt_request->nlh.nlmsg_len, 0);
     printf("Error : Bytes sent = %d, errno = %d\n", rc, errno);
     free(nl_rt_request);
     nl_rt_request = NULL;
}

/*import the function from userspace.c*/
extern 
uint32_t new_seq_no();

/* Routine to prepare a Netlink msg to add a route 
 * to Routing Table.
 * Return a pointer to the msg to be send to kernel, 
 * else return NULL on failure.*/

nl_rt_request_t *
nl_route_add(int sock_fd, uint32_t dest, 
          uint8_t mask, uint32_t gw_ip, 
          uint32_t ifindex){

    int rc = 0;
    nl_rt_request_t *nl_rt_request = calloc(1, 
                NLMSG_HDRLEN + NLMSG_ALIGN(sizeof((nl_rt_request_t *)0)->r) +
                NLMSG_ALIGN(sizeof((nl_rt_request_t *)0)->buf));

    /* Initialize request structure */
    
    /*Set the flags in Netlink hdr*/
    nl_rt_request->nlh.nlmsg_len = NLMSG_HDRLEN + 
        NLMSG_ALIGN(sizeof(nl_rt_request->r));

    nl_rt_request->nlh.nlmsg_flags = 0;
    nl_rt_request->nlh.nlmsg_flags = 
    /*US is requesting kernel subsystem to
     * perform some operation*/ 
                    NLM_F_REQUEST | 
    /*US is asking kernel subsystem to create 
     * the requested resource(a route)*/
                    NLM_F_CREATE  | 
    /*US is telling kernel to not to create 
     * a resource if already exist*/
                    NLM_F_EXCL    |
    /*US is telling kernel to send back ACK*/
                    NLM_F_ACK;
    
    /* Defined in include/uapi/linux/rtnetlink.h
     * RTM_NEWROUTE is the Netlink msg type used to
     * tell the routing subsystem in kernel space to
     * create a new route or update the existing one*/
     nl_rt_request->nlh.nlmsg_type = RTM_NEWROUTE;
     
     /*Get the new sequence number*/
     nl_rt_request->nlh.nlmsg_seq = new_seq_no();
     nl_rt_request->nlh.nlmsg_pid = getpid();

#if 0
    /* Now initialize the Dest/mask IP addresses into formatted
     * structure*/
     addr_t ipv4_dest;
     ipv4_dest.family = AF_INET; /*For IPV4*/
     ipv4_dest.mask = mask;
     memcpy(ipv4_dest.addr, (uint8_t *)&dest, sizeof(struct in_addr));
#endif

     /*Now let us fill struct rtmsg fields*/
    
     nl_rt_request->r.rtm_family = AF_INET;
     nl_rt_request->r.rtm_dst_len = mask;

     /* We want to add a route to kernel's main routing table.
      * Kernel can be confnigured to have multiple routing
      * tables*/
     nl_rt_request->r.rtm_table    = RT_TABLE_MAIN;
     nl_rt_request->r.rtm_protocol = RTPROT_BOOT;
     nl_rt_request->r.rtm_type     = RTN_UNICAST;

#if 0
     if(nl_rt_request->r.rtm_family == AF_INET6)
        nl_rt_request->r.rtm_scope = RT_SCOPE_UNIVERSE;
     else if(nl_rt_request->r.rtm_family == AF_INET4)
        nl_rt_request->r.rtm_scope = RT_SCOPE_LINK;
     else
        nl_rt_request->r.rtm_scope = RT_SCOPE_NOWHERE;
#endif
     
     nl_rt_request->r.rtm_scope = RT_SCOPE_UNIVERSE;

    nl_rt_request->nlh.nlmsg_len = NLMSG_HDRLEN + 
        NLMSG_ALIGN(sizeof(struct rtmsg)) ;

    /* Alternaively, in above line, You can also use 
     * NLMSG_LENGTH which also include the size of 
     * NLMSG_HDRLEN implicitely
     *  nl_rt_request->nlh.nlmsg_len = 
     *      NLMSG_LENGTH(sizeof(nl_rt_request_t));*/
    
 
    /*We have prepared the nlhdr and rtmsg, now add the 
     * attributes (TLVs)*/

     rc = nl_attr_add(&nl_rt_request->nlh, MAX_PAYLOAD, 
                RTA_DST, sizeof(uint32_t), (char *)&dest);

     if (rc < 0){
        printf("Error : %s(%d) : Adding Attribute\n", __FUNCTION__, __LINE__);
        return 0;
     }
     
     rc = nl_attr_add(&nl_rt_request->nlh, MAX_PAYLOAD,
                RTA_OIF, sizeof(uint32_t), (char *)&ifindex);

     if (rc < 0){
        printf("Error : %s(%d) : Adding Attribute\n", __FUNCTION__, __LINE__);
        return 0;
     }

     rc = nl_attr_add(&nl_rt_request->nlh, MAX_PAYLOAD,
                RTA_GATEWAY, sizeof(uint32_t), (char *)&gw_ip);

     if (rc < 0){
        printf("Error : %s(%d) : Adding Attribute\n", __FUNCTION__, __LINE__);
        return 0;
     }

     return nl_rt_request;
}



