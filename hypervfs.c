#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eduard Tofan");
MODULE_ALIAS_FS("hypervfs");

static int hypervfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *inode = new_inode(sb);
	if (inode) {
		inode->i_ino = get_next_ino();
		inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
		inode->i_mode = S_IFDIR | 0750;
		inode->i_op = &simple_dir_inode_operations;
		inode->i_fop = &simple_dir_operations;
		inc_nlink(inode);
	}


	sb->s_root = d_make_root(inode);
	if (!sb->s_root)
		return -ENOMEM;

	return 0;
}

static struct dentry *hypervfs_mount(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	return mount_single(fs_type, flags, data, hypervfs_fill_super);
}

static struct file_system_type hypervfs_fs_type = {
	.owner			= THIS_MODULE,
	.name			= "hypervfs",
	.kill_sb		= kill_anon_super,
	.mount			= hypervfs_mount
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
