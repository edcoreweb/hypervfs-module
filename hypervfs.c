#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eduard Tofan");
MODULE_ALIAS_FS("hypervfs");

static struct file_system_type hypervfs_fs_type = {
	.owner			= THIS_MODULE,
	.name			= "hypervfs",
	.kill_sb		= kill_anon_super
};

static int __init hypervfs_init(void)
{
	return register_filesystem(&hypervfs_fs_type);
}

static void __exit hypervfs_exit(void)
{
	unregister_filesystem(&hypervfs_fs_type);
}

module_init(hypervfs_init);
module_exit(hypervfs_exit);
