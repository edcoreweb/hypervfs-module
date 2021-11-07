#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include "hypervfs.h"

static const struct inode_operations hypervfs_dir_inode_operations;
static const struct inode_operations hypervfs_file_inode_operations;
static const struct inode_operations hypervfs_symlink_inode_operations;

/**
 * hypervfs_alloc_inode - helper function to allocate an inode
 */
struct inode *hypervfs_alloc_inode(struct super_block *sb)
{
	struct inode *inode;
	inode = (struct inode *)kmem_cache_alloc(hypervfs_inode_cache, GFP_KERNEL);
	if (!inode)
		return NULL;

	return inode;
}

/**
 * Destroy an inode
 */
void hypervfs_free_inode(struct inode *inode)
{
	kmem_cache_free(hypervfs_inode_cache, inode);
}

/**
 * Make an inode
 */
int hypervfs_init_inode(struct inode *inode, umode_t mode, dev_t rdev)
{
	int err = 0;

	inode_init_owner(inode, NULL, mode);
	inode->i_blocks = 0;
	inode->i_rdev = rdev;
	inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
	// TODO: see if we need this
	// inode->i_mapping->a_ops = &hypervfs_addr_operations;

	switch (mode & S_IFMT) {
	case S_IFIFO:
	case S_IFBLK:
	case S_IFCHR:
	case S_IFSOCK:
		inode->i_op = &hypervfs_file_inode_operations;
		init_special_inode(inode, inode->i_mode, inode->i_rdev);
		break;
	case S_IFREG:
		inode->i_op = &hypervfs_file_inode_operations;
		inode->i_fop = &hypervfs_file_operations;
		break;
	case S_IFLNK:
		inode->i_op = &hypervfs_symlink_inode_operations;
		break;
	case S_IFDIR:
		inc_nlink(inode);
		inode->i_op = &hypervfs_dir_inode_operations;
		inode->i_fop = &hypervfs_dir_operations;
		break;
	default:
		err = -EINVAL;
		goto error;
	}

error:
	return err;
}

/**
 * Helper function to setup an inode
 */
struct inode *hypervfs_get_inode(struct super_block *sb, umode_t mode, dev_t rdev)
{
	int err;
	struct inode *inode;

	inode = new_inode(sb);
	if (!inode) {
		pr_warn("%s (%d): Problem allocating inode\n", __func__, task_pid_nr(current));
		return ERR_PTR(-ENOMEM);
	}

	err = hypervfs_init_inode(inode, mode, rdev);
	if (err) {
		iput(inode);
		return ERR_PTR(err);
	}

	return inode;
}

/**
 * Release an inode
 */
void hypervfs_evict_inode(struct inode *inode)
{
	truncate_inode_pages_final(&inode->i_data);
	clear_inode(inode);
	filemap_fdatawrite(&inode->i_data);
}

static const struct inode_operations hypervfs_dir_inode_operations = {
	// .create = hypervfs_vfs_create,
	// .atomic_open = hypervfs_vfs_atomic_open,
	// .lookup = hypervfs_vfs_lookup,
	// .link = hypervfs_vfs_link,
	// .symlink = hypervfs_vfs_symlink,
	// .unlink = hypervfs_vfs_unlink,
	// .mkdir = hypervfs_vfs_mkdir,
	// .rmdir = hypervfs_vfs_rmdir,
	// .mknod = hypervfs_vfs_mknod,
	// .rename = hypervfs_vfs_rename,
	// .getattr = hypervfs_vfs_getattr,
	// .setattr = hypervfs_vfs_setattr,
	// .listxattr = hypervfs_listxattr,
	// .get_acl = hypervfs_iop_get_acl,
};

static const struct inode_operations hypervfs_file_inode_operations = {
	// .getattr = hypervfs_vfs_getattr,
	// .setattr = hypervfs_vfs_setattr,
	// .listxattr = hypervfs_listxattr,
	// .get_acl = hypervfs_iop_get_acl,
};

static const struct inode_operations hypervfs_symlink_inode_operations = {
	// .get_link = hypervfs_vfs_get_link,
	// .getattr = hypervfs_vfs_getattr,
	// .setattr = hypervfs_vfs_setattr,
	// .listxattr = hypervfs_listxattr,
};