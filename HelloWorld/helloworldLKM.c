/*
* helloworldLKM.c − The simplest kernel module.
*/
#include <linux/module.h> 	     /* Needed by all modules */

/* This function shall be invoked as soon as this LKM is loaded.
 * Note the function prototype */
int hello_world_init_module(void){

    printk(KERN_INFO "Hello world Module Loaded Successfully\n");
    /*
     * A non 0 return means init_module failed; module can't be loaded.
     */
    return 0;
}


/* Visit my Webiste : https://csepracticals.wixsite.com/csepracticals
 * for other courses for free or on discounted price. Take a look !! */



/* This function shall be invoked as soon as this LKM is unloaded.
 * Note the function prototype */
void hello_world_cleanup_module(void){

    printk(KERN_INFO "Goodbye hello world LKM\n");
}

/* Tell the kernel which are init and cleanup functions for
 * this module. If you do not do this registration, kernel would
 * try to use 'init_module' and 'cleanup_module' instead */
module_init(hello_world_init_module);
module_exit(hello_world_cleanup_module);

/*Module Information*/
#define AUTHOR_NAME "ABHISHEK_SAGAR"
#define MODULE_DESC "HELLO_WORLD_LINUX_KERNEL_MODULE"

MODULE_AUTHOR(AUTHOR_NAME);         /* Who wrote this module? */
MODULE_DESCRIPTION(MODULE_DESC);    /* What does this module do ?*/
MODULE_LICENSE("GPL");

