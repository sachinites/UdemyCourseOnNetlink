
/* Find my other courses on : http://csepracticals.wixsite.com/csepracticals
 * at great discounts and offers */

/* Demo Greetings Linux Kernel Module written for 
 * kernel 4.4.0-31-generic and 5.0.0-36-generic*/

/* #include Minimum required header files to write a kernel
 * Module*/

#include <linux/module.h>   /*Include this for any kernel Module you write*/
#include <linux/netlink.h>  /*Use it for Netlink functionality*/
#include <net/sock.h>       /*Network namespace and socket Based APIs*/
#include <linux/string.h>   /*for memset/memcpy etc..., do not use <string.h>, that is for user space*/
#include <linux/kernel.h>   /*for scnprintf*/
#define __KERNEL_CODE__
#include "netLinkKernelUtils.h" 

/*Global variables of this LKM*/
static struct sock *nl_sk = NULL;       /*Kernel space Netlink socket ptr*/

/* Reciever function for Data received over netlink
 * socket from user space
 * skb - socket buffer, a unified data structiure for 
 * packaging the data type being transported from one kernel
 * subsystem to another or from kernel space to user-space
 * or from user-space to kernel space. The internal member of this
 * socket buffer is not accessed directly, but rather kernel APIs
 * provide setters/getters for this purpose. We will use them as 
 * we would need along the way*/

/* When the Data/msg comes over netlink socket from userspace, kernel
 * packages the data in sk_buff data structures and invokes the below
 * function with pointer to that skb*/

static void netlink_recv_msg_fn(struct sk_buff *skb_in){

    struct nlmsghdr *nlh_recv, *nlh_reply;
    char *user_space_data;
    int user_space_data_len;
    struct sk_buff *skb_out;
    char kernel_reply[256];
    int user_space_process_port_id;
    int res;

    printk(KERN_INFO "%s() invoked", __FUNCTION__);

    /*skb carries Netlink Msg which starts with Netlink Header*/
    nlh_recv = (struct nlmsghdr*)(skb_in->data);

    nlmsg_dump(nlh_recv);

    user_space_process_port_id = nlh_recv->nlmsg_pid;

    printk(KERN_INFO "%s(%d) : port id of the sending user space process = %u\n", 
            __FUNCTION__, __LINE__, user_space_process_port_id);

    user_space_data = (char*)nlmsg_data(nlh_recv);
    user_space_data_len = skb_in->len;

    printk(KERN_INFO "%s(%d) : msg recvd from user space= %s, skb_in->len = %d, nlh->nlmsg_len = %d\n", 
            __FUNCTION__, __LINE__, user_space_data, user_space_data_len, nlh_recv->nlmsg_len);


    if(nlh_recv->nlmsg_flags & NLM_F_ACK){

        /*Sending reply back to user space process*/
        memset(kernel_reply, 0 , sizeof(kernel_reply));

        /*defined in linux/kernel.h */
        snprintf(kernel_reply, sizeof(kernel_reply), 
                "Msg from Process %d has been processed by kernel", nlh_recv->nlmsg_pid);

        /*Get a new sk_buff with empty Netlink hdr already appended before payload space
         * i.e skb_out->data will be pointer to below msg : 
         *
         * +----------+---------------+
         * |Netlink Hdr|   payload    |
         * ++---------+---------------+
         *
         * */

        skb_out = nlmsg_new(sizeof(kernel_reply), 0/*Related to memory allocation, skip...*/);

        /*Add a TLV*/ 
        nlh_reply = nlmsg_put(skb_out,
                0,                  /*Sender is kernel, hence, port-id = 0*/
                nlh_recv->nlmsg_seq,        /*reply with same Sequence no*/
                NLMSG_DONE,                 /*Metlink Msg type*/
                sizeof(kernel_reply),       /*Payload size*/
                0);                         /*Flags*/

        /* copy the paylod now. In userspace, use NLMSG_DATA, in kernel space
         * use nlmsg_data*/
        strncpy(nlmsg_data(nlh_reply), kernel_reply, sizeof(kernel_reply));

        /*Finaly Send the  msg to user space space process*/
        res = nlmsg_unicast(nl_sk, skb_out, user_space_process_port_id);

        if(res < 0){     
            printk(KERN_INFO "Error while sending the data back to user-space\n");
            kfree_skb(skb_out); /*free the internal skb_data also*/
        }                
    }
}
                     
                     
                     
static struct netlink_kernel_cfg cfg = {
    .input = netlink_recv_msg_fn, /*This fn would recieve msgs from userspace for
                                    Netlink protocol no 31*/
    /* There are other parameters of this structure, for now let us
     * not use them as we are just kid !!*/
};                   
                     
/*Init function of this kernel Module*/
static int __init NetlinkProject_init(void) {
    
    /* All printk output would appear in /var/log/kern.log file
     * use cmd ->  tail -f /var/log/kern.log in separate terminal 
     * window to see output*/
	printk(KERN_INFO "Hello Kernel, I am kernel Module NetlinkProjectLKM.ko\n");
   
    /* Now Create a Netlink Socket in kernel space*/
    /* Arguments : 
     * Network Namespace : Read here : 
     *                     https://blogs.igalia.com/dpino/2016/04/10/network-namespaces
     * Netlink Protocol ID : NETLINK_TEST_PROTOCOL 
     * Netlink Socket Configuration Data 
     * */ 
     
     /*Now create a Netlink socket*/
     nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST_PROTOCOL, &cfg);
     
     if(!nl_sk){
         printk(KERN_INFO "Kernel Netlink Socket for Netlink protocol %u failed.\n", NETLINK_TEST_PROTOCOL);
         return -ENOMEM; /*All errors are defined in ENOMEM for kernel space, and in stdio.h for user space*/
     }
     
     printk(KERN_INFO "Netlink Socket Created Successfully");
    /*This fn must return 0 for module to successfully make its way into kernel*/
	return 0;
}

/*Exit function of this kernel Module*/
static void __exit NetlinkProject_exit(void) {

	printk(KERN_INFO "Bye Bye. Exiting kernel Module NetlinkProjectLKM.ko \n");
    /*Release any kernel resources held by this module in this fn*/
    netlink_kernel_release(nl_sk);
    nl_sk = NULL;
}


/*Every Linux Kernel Module has Init and Exit functions - just
 * like normal C program has main() fn.*/

/* Registration of Kernel Module Init Function.
 * Whenever the Module is inserted into kernel using
 * insmod cmd, below function is triggered. You can do
 * all initializations here*/
module_init(NetlinkProject_init); 

/* Registration of Kernel Module Exit Function.
 * Whenever the Module is removed from kernel using
 * rmmod cmd, below function is triggered. You can do
 * cleanup in this function.*/
module_exit(NetlinkProject_exit);

/*Module Information*/
#define AUTHOR_NAME "ABHISHEK_SAGAR"
#define MODULE_DESC "HELLO_WORLD_LINUX_KERNEL_MODULE"

MODULE_AUTHOR(AUTHOR_NAME);         /* Who wrote this module? */
MODULE_DESCRIPTION(MODULE_DESC);    /* What does this module do ?*/

MODULE_LICENSE("GPL");
