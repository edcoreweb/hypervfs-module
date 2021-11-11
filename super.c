#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/backing-dev.h>
#include "hypervfs.h"

static const struct super_operations hypervfs_super_ops;

static int hypervfs_set_super(struct super_block *s, void *data)
{
	s->s_fs_info = data;
	return set_anon_super(s, data);
}

static int hypervfs_fill_super(struct super_block *sb, void *session, int flags)
{
	int ret;
	sb->s_maxbytes = MAX_LFS_FILESIZE;
	sb->s_blocksize_bits = fls(4096);
	sb->s_blocksize = 1 << sb->s_blocksize_bits;
	sb->s_magic = HYPERVFS_MAGIC;
	sb->s_op = &hypervfs_super_ops;
	// sb->s_xattr = hypervfs_xattr_handlers;
	sb->s_time_min = 0;

	ret = super_setup_bdi(sb);
	if (ret)
		return ret;

	sb->s_bdi->ra_pages = VM_READAHEAD_PAGES;
	sb->s_flags |= SB_ACTIVE | SB_DIRSYNC;

	return 0;
}

/**
 * Mount a superblock
 */
static struct dentry *hypervfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data)
{
	struct super_block *sb = NULL;
	struct inode *inode = NULL;
	struct dentry *root = NULL;
	umode_t mode = S_IRWXUGO | S_ISVTX;
	int retval = 0;

	// TODO: connect to server here, add private data
	retval = hypervfs_op_connect();
	if (retval)
		goto out;

	sb = sget(fs_type, NULL, hypervfs_set_super, flags, NULL);
	if (IS_ERR(sb)) {
		retval = PTR_ERR(sb);
		goto out;
	}

	retval = hypervfs_fill_super(sb, NULL, flags);
	if (retval)
		goto release_sb;

	sb->s_d_op = &hypervfs_dentry_operations;

	inode = hypervfs_get_inode(sb, S_IFDIR | mode, 0);
	if (IS_ERR(inode)) {
		retval = PTR_ERR(inode);
		goto release_sb;
	}

	root = d_make_root(inode);
	if (!root) {
		retval = -ENOMEM;
		goto release_sb;
	}

	sb->s_root = root;
	// TODO: stat the root path and fill the inode
	// retval = stat_inode(d_inode(root));

	return dget(sb->s_root);

out:
	hypervfs_op_close();
	return ERR_PTR(retval);

release_sb:
	hypervfs_op_close();
	deactivate_locked_super(sb);
	return ERR_PTR(retval);
}

static void hypervfs_kill_sb(struct super_block *sb)
{
	hypervfs_op_close();
	kill_anon_super(sb);
}


static const struct super_operations hypervfs_super_ops = {
	.alloc_inode = hypervfs_alloc_inode,
	.free_inode = hypervfs_free_inode,
	.statfs = simple_statfs,
	.drop_inode = generic_drop_inode,
	.evict_inode = hypervfs_evict_inode,
};

struct file_system_type hypervfs_fs_type = {
	.name = "hyperv",
	.mount = hypervfs_mount,
	.kill_sb = hypervfs_kill_sb,
	.owner = THIS_MODULE,
	.fs_flags = FS_RENAME_DOES_D_MOVE,
};

MODULE_ALIAS_FS("hyperv");
