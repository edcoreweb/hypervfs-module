#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eduard Tofan");

static int __init hypervfs_init(void)
{
	printk(KERN_INFO "Hello world!\n");
	return 0; 
}

static void __exit hypervfs_exit(void)
{
	printk(KERN_INFO "Cleaning up module.\n");
}

module_init(hypervfs_init);
module_exit(hypervfs_exit);
