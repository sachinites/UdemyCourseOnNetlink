/*
 * =====================================================================================
 *
 *       Filename:  common.h
 *
 *    Description:  This file contains common routines and definitions to be used in kernel and user space
 *
 *        Version:  1.0
 *        Created:  11/22/2019 12:16:25 PM
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

#ifndef __NL_COMMON__
#define __NL_COMMON__

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#ifndef __KERNEL__
#include <stdio.h>
#include <memory.h>
#endif

/* maximum payload size in Bytes exchanged between kernel and userspace
 * in either directions*/
#define MAX_PAYLOAD 1024

/*User defined NL MSG TYPES, should be > 16 */
#define NLMSG_GREET     20

/* user define netlink protocol.
 * It should match with Netlink Socket protocol opened in kernel space.*/

/* Define a New Netlink protocol ID
 * In file linux/netlink.h, you can see in line no [8-33]
 * lists the reserved IDs used for Netlink protocols already implemented
 * in Linux. We can use upto max 31th protocol number for Netlink. Let
 * us just pick the unused one - 31 below */

//#define NETLINK_TEST_PROTOCOL   31
#define NETLINK_TEST_PROTOCOL NETLINK_ROUTE


static inline char *
netlink_get_msg_type(__u16 nlmsg_type){

    switch(nlmsg_type){
        case NLMSG_NOOP:
            return "NLMSG_NOOP";
        case NLMSG_ERROR:
            return "NLMSG_ERROR";
        case NLMSG_DONE:
            return "NLMSG_DONE";
        case NLMSG_OVERRUN:
            return "NLMSG_OVERRUN";
        case NLMSG_GREET:
            return "NLMSG_GREET";
        case RTM_NEWROUTE:
            return "RTM_NEWROUTE";
        default:
            return "NLMSG_UNKNOWN";
    }
}

static inline void
nlmsg_dump(struct nlmsghdr *nlh){
#ifdef __KERNEL__
    printk(KERN_INFO "Dumping Netlink Msgs Hdr");
#else
    printf("Dumping Netlink Msgs Hdr");
#endif
#ifdef __KERNEL__
    printk(KERN_INFO "  Netlink Msg Type = %s", 
        netlink_get_msg_type(nlh->nlmsg_type));
#else
    printf("  Netlink Msg Type = %s",
        netlink_get_msg_type(nlh->nlmsg_type));
#endif
#ifdef __KERNEL__
    printk(KERN_INFO "  Netlink Msg len  = %d", nlh->nlmsg_len);
#else
    printf("  Netlink Msg len  = %d", nlh->nlmsg_len);
#endif
#ifdef __KERNEL__
    printk(KERN_INFO "  Netlink Msg flags  = %d", nlh->nlmsg_flags);
#else
    printf("  Netlink Msg flags  = %d", nlh->nlmsg_flags);
#endif
#ifdef __KERNEL__
    printk(KERN_INFO "  Netlink Msg Seq#  = %d", nlh->nlmsg_seq);
#else
    printf("  Netlink Msg Seq#  = %d", nlh->nlmsg_seq);
#endif
#ifdef __KERNEL__
    printk(KERN_INFO "  Netlink Msg Pid#  = %d", nlh->nlmsg_pid);
#else
    printf("  Netlink Msg Pid#  = %d", nlh->nlmsg_pid);
#endif

    if(nlh->nlmsg_type == NLMSG_ERROR){
#ifdef __KERNEL__
        printk(KERN_INFO "  Error code = %d\n", *(int *)(nlh + 1));
#else
        printf("    Error code = %d\n", *(int *)(nlh + 1));
#endif
    nlmsg_dump((struct nlmsghdr *)((char *)(nlh + 1) + sizeof(int)));    
    }
}

#define NLMSG_TAIL(nlh_ptr) \
    ((char *)nlh_ptr + nlh_ptr->nlmsg_len)

/*return -1 on failure, 0 on success*/
static inline uint32_t
nl_attr_add(struct nlmsghdr *nlh, 
            int maxlen,
            int attr_type, 
            int attr_len,
            char *attr_data){

    int len = 0;
    struct rtattr *new_rta = NULL;
    
    /* RTA_LENGTH - Macro to align attribute length.
     * It implicitely count the length of attribute hdr
     * struct rtattr*/
    len = RTA_ALIGN(sizeof(struct rtattr)) + RTA_ALIGN(attr_len);

    if (nlh->nlmsg_len + len > maxlen){
#ifndef __KERNEL__
        printf("Error : %s() attribute add error : attr_type = %d\n",
            __FUNCTION__, attr_type);
#else
        printk(KERN_INFO "Error : %s() attribute add error : attr_type = %d\n",
            __FUNCTION__, attr_type);
#endif
        return -1;
    }

    new_rta = (struct rtattr *)NLMSG_TAIL(nlh);
    new_rta->rta_type = attr_type;
    new_rta->rta_len = attr_len;
    memcpy(RTA_DATA(new_rta), attr_data, attr_len);
    nlh->nlmsg_len += len;
    return 0;
}

#endif /* __NL_COMMON__ */
