#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include "hypervfs.h"

const struct dentry_operations hypervfs_dentry_operations = {
	// .d_revalidate = hypervfs_lookup_revalidate,
	// .d_weak_revalidate = hypervfs_lookup_revalidate,
	// .d_delete = hypervfs_cached_dentry_delete,
	// .d_release = hypervfs_dentry_release,
};