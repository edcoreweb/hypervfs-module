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

static int hypervfs_mknod(struct inode *dir, struct dentry *dentry, umode_t omode, dev_t rdev)
{
	int err;
	const unsigned char *name;
	umode_t mode;
	struct inode *inode;
	// struct posix_acl *dacl = NULL, *pacl = NULL;

	// err = hypervfs_acl_mode(dir, &mode, &dacl, &pacl);
	// if (err) {
	// 	goto error;
	// }

	name = dentry->d_name.name;

	// err = hypervfs_op_mknod(name, mode);
	if (err)
		goto error;

	// hypervfs_invalidate_inode_attr(dir);

	// inode = hypervfs_get_new_inode(ino);
	// if (IS_ERR(inode)) {
	// 	err = PTR_ERR(inode);
	// 	goto error;
	// }

	// hypervfs_set_create_acl(inode, dacl, pacl);
	// d_instantiate(dentry, inode);
	err = 0;
error:
	// posix_acl_release(dacl);
	// posix_acl_release(pacl);
	return err;
}

static int hypervfs_create(struct inode *dir, struct dentry *dentry, umode_t omode, bool excl)
{
	return hypervfs_mknod(dir, dentry, omode, 0);
}

static struct dentry *hypervfs_lookup(struct inode *dir, struct dentry *entry, unsigned int flags)
{
	int err;
	struct inode *inode;
	struct dentry *newent;

	if (entry->d_name.len > NAME_MAX)
		return ERR_PTR(-ENAMETOOLONG);

	// TODO: fetch the data from the server

	newent = d_splice_alias(inode, entry);
	err = PTR_ERR(newent);
	if (IS_ERR(newent))
		goto out_err;

	return newent;

 out_iput:
	iput(inode);
 out_err:
	return ERR_PTR(err);
}

static int hypervfs_atomic_open(struct inode *dir, struct dentry *entry, struct file *file, unsigned flags, umode_t mode)
{
	int err;
	struct dentry *res = NULL;

	if (d_in_lookup(entry)) {
		res = hypervfs_lookup(dir, entry, 0);
		if (IS_ERR(res))
			return PTR_ERR(res);

		if (res)
			entry = res;
	}

	if (!(flags & O_CREAT) || d_really_is_positive(entry))
		goto out;

	/* Only creates */
	file->f_mode |= FMODE_CREATED;

	err = hypervfs_mknod(dir, entry, mode, 0);
	if (err) {
		dput(res);
		return err;
	}

out:
	return finish_no_open(file, res);
}

static int hypervfs_getattr(const struct path *path, struct kstat *stat, u32 request_mask, unsigned int flags)
{
	struct dentry *dentry = path->dentry;

	generic_fillattr(d_inode(dentry), stat);

	return 0;

}

static const struct inode_operations hypervfs_dir_inode_operations = {
	.create = hypervfs_create,
	.atomic_open = hypervfs_atomic_open,
	.lookup = hypervfs_lookup,
	// .link = hypervfs_vfs_link,
	// .symlink = hypervfs_vfs_symlink,
	// .unlink = hypervfs_vfs_unlink,
	// .mkdir = hypervfs_vfs_mkdir,
	// .rmdir = hypervfs_vfs_rmdir,
	.mknod = hypervfs_mknod,
	// .rename = hypervfs_vfs_rename,
	.getattr = hypervfs_getattr,
	// .setattr = hypervfs_vfs_setattr,
	// .listxattr = hypervfs_listxattr,
	// .get_acl = hypervfs_iop_get_acl,
};

static const struct inode_operations hypervfs_file_inode_operations = {
	.getattr = hypervfs_getattr,
	// .setattr = hypervfs_vfs_setattr,
	// .listxattr = hypervfs_listxattr,
	// .get_acl = hypervfs_iop_get_acl,
};

static const struct inode_operations hypervfs_symlink_inode_operations = {
	// .get_link = hypervfs_vfs_get_link,
	.getattr = hypervfs_getattr,
	// .setattr = hypervfs_vfs_setattr,
	// .listxattr = hypervfs_listxattr,
};
