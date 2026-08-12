#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>


static int count = 1;
module_param(count, int, 0644);
MODULE_PARM_DESC(count, "print hello times");


static int __init  hello_kernel(void)
{
    int i =0;
    for (i = 0; i < count; i++)
        printk(KERN_INFO "Hello kernel %d\n", i);
    return 0;
}



static void __exit  exit_kernel(void)
{
    //printk(KERN_INFO "Goodbye kernel\n");
    printk(KERN_ALERT "Goodbye kernel\n");

}


module_init(hello_kernel);
module_exit(exit_kernel);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Hello kernel");
MODULE_AUTHOR("zhang");