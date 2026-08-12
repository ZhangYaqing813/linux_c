//
// Created by zhangyq on 2026
//新增加模块参数的实验
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

static char *msg="hello this is default kernel msg";
module_param(msg,charp,0444);
MODULE_PARM_DESC(msg,"A message to print module load");


static int __init  hellow_init(void) {
    //printk(KERN_ALERT "hellow_kernel my first kernel module\n");
    printk(KERN_INFO "msg : %s\n",msg);
    return 0;
}

static void __exit hello_exit(void) {

    printk(KERN_ALERT "exit my first kernel module\n");

}

module_init (hellow_init);
module_exit (hello_exit);
MODULE_LICENSE ("GPL");
MODULE_DESCRIPTION("A simple hello world kernel module");
