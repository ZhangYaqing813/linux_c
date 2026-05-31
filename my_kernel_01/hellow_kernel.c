//
// Created by zhangyq on 2026/5/31.
//
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

static int __init  hellow_init(void) {
    printk(KERN_ALERT "hellow_kernel my first kernel module\n");
    return 0;
}

static void __exit hello_exit(void) {

    printk(KERN_ALERT "exit my first kernel module\n");

}

module_init (hellow_init);
module_exit (hello_exit);
MODULE_LICENSE ("GPL");
MODULE_DESCRIPTION("A simple hello world kernel module");
