#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include "hypervfs.h"

static int hypervfs_lookup_revalidate(struct dentry *dentry, unsigned int flags)
{
	struct inode *inode;

	if (flags & LOOKUP_RCU)
		return -ECHILD;

	inode = d_inode(dentry);
	if (!inode)
		goto out_valid;

	// TODO: make a way to invalidate the inode
	// and refresh if invalid
	if (false) {
		int retval;
		// retval = hypervfs_refresh_inode(inode);

		if (retval == -ENOENT)
			return 0;

		if (retval < 0)
			return retval;
		
	}

out_valid:
	return 1;
}

/**
 * Called when dentry refcount equals 0
 */
static int hypervfs_cached_dentry_delete(const struct dentry *dentry)
{
	/* Don't cache negative dentries */
	if (d_really_is_negative(dentry))
		return 1;

	return 0;
}

/**
 * Called when dentry is going to be freed
 *
 */
static void hypervfs_dentry_release(struct dentry *dentry)
{
	// TODO: see if we need this
	dentry->d_fsdata = NULL;
}

const struct dentry_operations hypervfs_dentry_operations = {
	.d_revalidate = hypervfs_lookup_revalidate,
	.d_weak_revalidate = hypervfs_lookup_revalidate,
	.d_delete = hypervfs_cached_dentry_delete,
	.d_release = hypervfs_dentry_release,
};
