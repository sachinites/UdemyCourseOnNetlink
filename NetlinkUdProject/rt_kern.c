/*
 * =====================================================================================
 *
 *       Filename:  rt.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/22/2020 06:14:33 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Juniper Networks (https://csepracticals.wixsite.com/csepracticals), sachinites@gmail.com
 *        Company:  Juniper Networks
 *
 *        This file is part of the Netlink Sockets distribution (https://github.com/sachinites) 
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

#include "rt.h"
#include <linux/slab.h> /*kmalloc/kfree*/

void
rt_init_rt_table(rt_table_t *rt_table){

    init_glthread(&rt_table->head);
}

rt_bool_t
rt_add_new_rt_entry(rt_table_t *rt_table,
    char *dest_ip, char mask, char *gw_ip, char *oif){

    rt_entry_t *rt_entry = NULL;

    rt_entry = kmalloc(sizeof(rt_entry_t), GFP_KERNEL); 

    if(!rt_entry)
        return RT_FALSE;

    strncpy(rt_entry->dest_ip, dest_ip, sizeof(rt_entry->dest_ip));
    rt_entry->mask = mask;
    strncpy(rt_entry->gw_ip, gw_ip, sizeof(rt_entry->gw_ip));
    strncpy(rt_entry->oif, oif, sizeof(rt_entry->oif));

    init_glthread(&rt_entry->rt_entry_glue);

    glthread_add_next(&rt_table->head, &rt_entry->rt_entry_glue);
    return RT_TRUE;
}

rt_bool_t
rt_delete_rt_entry(rt_table_t *rt_table,
    char *dest_ip, char mask){

    glthread_t *curr;
    rt_entry_t *rt_entry = NULL;

    ITERATE_GLTHREAD_BEGIN(&rt_table->head, curr){
    
        rt_entry = rt_entry_glue_to_rt_entry(curr);

        if(strncmp(rt_entry->dest_ip, dest_ip, sizeof(rt_entry->dest_ip)) == 0 &&
            rt_entry->mask == mask){

            remove_glthread(&rt_entry->rt_entry_glue);
            kfree(rt_entry);
            return RT_TRUE;
        }
    } ITERATE_GLTHREAD_END(&rt_table->head, curr);

    return RT_FALSE;
}

rt_bool_t
rt_update_rt_entry(rt_table_t *rt_table,
    char *dest_ip, char mask, 
    char *new_gw_ip, char *new_oif){

    return RT_TRUE;
}

void
rt_clear_rt_table(rt_table_t *rt_table){


}

void
rt_free_rt_table(rt_table_t *rt_table){


}

void
rt_dump_rt_table(rt_table_t *rt_table){

    glthread_t *curr;
    rt_entry_t *rt_entry = NULL;

    ITERATE_GLTHREAD_BEGIN(&rt_table->head, curr){

        rt_entry = rt_entry_glue_to_rt_entry(curr);

        printk(KERN_INFO "%-20s %-4d %-20s %s\n",
                rt_entry->dest_ip,
                rt_entry->mask,
                rt_entry->gw_ip,
                rt_entry->oif);
    } ITERATE_GLTHREAD_END(&rt_table->head, curr);
}
