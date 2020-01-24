/*
 * =====================================================================================
 *
 *       Filename:  userspace.c
 *
 *    Description:  This file represents the userspace application for Netlink socket based Communication
 *
 *        Version:  1.0
 *        Created:  11/21/2019 09:37:51 PM
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
/*For kernel space, all errors are defined in 
 *usr/include/asm-generic/errno.h*/
#include <errno.h>   /* To use 'errno' as explained in code*/
#include <unistd.h>  /* for getpid() to get process id */
#include <memory.h>
#include <stdint.h>  /*for using uint32_t*/
#include <pthread.h>
#undef __KERNEL__
#include "common.h"
#include "nlrt.h"

int
send_netlink_msg_to_kernel(int sock_fd, 
                           char *msg, 
                           uint32_t msg_size,
                           int nlmsg_type);

static void
greet_kernel(int sock_fd, char *msg, uint32_t msg_len){

    send_netlink_msg_to_kernel(sock_fd, msg, msg_len, NLMSG_GREET);
}

static void
exit_userspace(int sock_fd){
    close(sock_fd);
    exit(0);
}

uint32_t new_seq_no(){

    static uint32_t seq_no = 0 ;
    return seq_no++;
}

/*Return the number of bytes send to kernel*/
int
send_netlink_msg_to_kernel(int sock_fd, 
                           char *msg, 
                           uint32_t msg_size,
                           int nlmsg_type){

    /* While sending msg to kernel, we will have 
     * to specify src address and dest address in
     *  struct sockaddr_nl structure. Src address 
     *  shall be this application (msg originator),
     *  and Destination adddress shall be kernel's */

    struct sockaddr_nl src_addr, dest_addr;

    /* Netlink msghdr for sending Netlink msgs*/
    struct nlmsghdr *nlh = NULL;

    memset(&src_addr, 0, sizeof(src_addr));
    
    /* specify who is the sender of the msg (i.e. this application),
     * kenel uses this info to reply back*/
    src_addr.nl_family = AF_NETLINK;
    /* ID of the application, it should be unique
     * to a process, good pratice to use process-id*/
    src_addr.nl_pid = getpid(); 
    
    /* Binding means: here, appln is telling the OS/Kernel that 
     * this application (identified using port-id by OS) is interested 
     * in receiving the msgs for Netlink protocol# NETLINK_TEST_PROTOCOL.
     * You can see we have specified two arguments in bind(). Kernel will 
     * use sock_fd (a handle) to handover the msgs coming from kernel subsystem 
     * (the kernel module we wrote) to deliver to the application whose port-id is
     * specified in src_address (means, this application itself).
     * */

    if(bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) == -1){
        printf("Error : Bind has failed\n");
        exit(1);
    }

    /* The application needs to specify whom it is sending the msg over 
     * Netlink Protocol.
     * In this case, our application is interested in sending msg to kernel 
     * (Other options are : Other userspace applications).
     * Our application needs to specify the destination address - 
     * The kernel's address using dest-pid = 0.
     * In kernel, any kernel subsystem/module which has
     * opened the Netlink socket for protocol NETLINK_TEST_PROTOCOL 
     * as Netlink protocol will going to recieve this msg. */
    
     memset(&dest_addr, 0, sizeof(dest_addr));
     dest_addr.nl_family = AF_NETLINK;
     dest_addr.nl_pid = 0;    /* For Linux Kernel this is always Zero*/

    /* now, we need to send a Netlink msg to Linux kernel Module.
     * We need to take a memory space to accomodate 
     * Netlink Msg Hdr followed by payload msg.
     * */

     /* Always use the macro NLMSG_SPACE to calculate the size of Payload data. 
      * This macro will take care to do all necessary alignment*/
     nlh=(struct nlmsghdr *)calloc(1,
                        NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD));

     /* Fill the netlink message header fields*/
     /* size of the payload + padding + netlink header*/
     nlh->nlmsg_len = NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD);
     nlh->nlmsg_pid = getpid();
     nlh->nlmsg_flags |= NLM_F_REQUEST; /*We want a reply from Kernel*/
     nlh->nlmsg_type = nlmsg_type;
     nlh->nlmsg_seq = new_seq_no();

     /* Fill in the netlink message payload */
     /* Copy the application data to Netlink payload space.
      * Use macro NLMSG_DATA to get ptr to netlink payload data
      * space*/
     strncpy(NLMSG_DATA(nlh), msg, msg_size);
    
     /*Now, wrap the data to be send inside iovec*/
     /* iovector - It is a conatiner of netlink msg*/
     struct iovec iov; 

     iov.iov_base = (void *)nlh;
     iov.iov_len = nlh->nlmsg_len;

    /* Outermost msg sturucture which will be a container of iovec. 
     * This Outermost msg structure is required to support unified
     * interface of message exchange between kernel and user-space*/
     static struct msghdr outermsghdr;

    /*Now wrap the iovec inside the msghdr*/
     memset(&outermsghdr, 0, sizeof(struct msghdr));
     outermsghdr.msg_name = (void *)&dest_addr; /*Whom you are sending this msg to*/
     outermsghdr.msg_namelen = sizeof(dest_addr);
     outermsghdr.msg_iov = &iov;
     outermsghdr.msg_iovlen = 1;

     int rc = sendmsg(sock_fd, &outermsghdr, 0);
     if(rc < 0){
        printf("Msg Sending Failed, error no = %d\n", errno);
     }    
     return rc;
}

int
create_netlink_socket(int protocol_number){

     /* Create a net link socket using usual socket() system call
      * When SOCK_RAW is used, Application has to pepare the struct msghdr
      * structure and send msg using sendmsg().
      * When SOCK_DGRAM is used, socket layer will take care to prepare struct
      * msghdr for you. You have to use sendto() in this case.
      * In this file, I have demonstrated SOCK_RAW case
      * */

    int sock_fd = socket(PF_NETLINK, 
                         SOCK_RAW, 
                         protocol_number);

    return sock_fd;
}

typedef struct thread_arg_{

    int sock_fd;
} thread_arg_t;

static void *
_start_kernel_data_receiver_thread(void *arg){

    int rc = 0;
    struct iovec iov;
    struct nlmsghdr *nlh_recv = NULL;
    static struct msghdr outermsghdr;
    int sock_fd = 0;

    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    sock_fd = thread_arg->sock_fd;

    /*Take a new buffer to recv data from kernel*/
    nlh_recv = (struct nlmsghdr *)calloc(1,
            NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD));
    
    do{
        /* Since, USA is receiving the msg from KS, so, just leave all
         * fields of nlmsghdr empty. they shall be filled by kernel
         * while delivering the msg to USA*/
        memset(nlh_recv, 0, NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD));
        
        iov.iov_base = (void *)nlh_recv;
        iov.iov_len = NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD);

        memset(&outermsghdr, 0, sizeof(struct msghdr));

        outermsghdr.msg_iov     = &iov;
        outermsghdr.msg_name    = NULL;
        outermsghdr.msg_iovlen  = 1;
        outermsghdr.msg_namelen = 0;

        /* Read message from kernel. Its a blocking system call 
         * Application execuation is suspended at this point 
         * and would not resume until it receives linux kernel
         * msg. We can configure recvmsg() to not to block, 
         * but lets use it in blocking mode for now */

        rc = recvmsg(sock_fd, &outermsghdr, 0);

        /* We have successfully received msg from linux kernel*/
        /* print the msg from kernel. kernel msg shall be stored 
         * in outermsghdr.msg_iov->iov_base
         * in same format : that is Netlink hdr followed by payload data*/

        printf("Received Netlink msg from kernel, bytes recvd = %d\n", rc);
        nlmsg_dump(outermsghdr.msg_iov->iov_base);
    } while(1);
}


void
start_kernel_data_receiver_thread(thread_arg_t *thread_arg){

    pthread_attr_t attr;
    pthread_t recv_pkt_thread;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&recv_pkt_thread, &attr,
            _start_kernel_data_receiver_thread,
            (void *)thread_arg);
}

/*Insert the LKM before starting the userspace program*/
int
main(int argc, char **argv){

    int choice;
    int sock_fd;

    sock_fd = create_netlink_socket(NETLINK_TEST_PROTOCOL);
    
    if(sock_fd == -1){
        printf("Error : Netlink socket creation failed"
        ": error = %d\n", errno);
        exit(EXIT_FAILURE);
    }

    thread_arg_t thread_arg;
    thread_arg.sock_fd = sock_fd;

    start_kernel_data_receiver_thread(&thread_arg);

    while(1){
        /*Main - Menu*/
        printf("Main-Menu\n");
        printf("\t1. Greet Kernel\n");
        printf("\t2. Route Add\n");
        printf("\t3. Exit\n");
        printf("choice ? ");
        scanf("%d", &choice);

        switch(choice){
            case 1:
                {
                    char user_msg[MAX_PAYLOAD];
                    memset(user_msg, 0, MAX_PAYLOAD);

                    if((fgets((char *)user_msg, MAX_PAYLOAD - 1, stdin) == NULL)){
                        printf("error in reading from stdin\n");
                        exit(EXIT_SUCCESS);
                    }
                    greet_kernel(sock_fd, user_msg, strlen(user_msg));
                }
            break;
            case 2:
                nl_route_add_from_user_input(sock_fd);
                break;
            case 3:
                exit_userspace(sock_fd);
            break;
            default:
                ;
        }
    }
    return 0;
}
