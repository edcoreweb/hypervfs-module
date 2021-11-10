#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include "hypervfs.h"

struct kmem_cache *hypervfs_inode_cache;
static struct kobject *hypervfs_kobj;
static struct attribute *hypervfs_attrs[] = {
	NULL,
};
static struct attribute_group hypervfs_attr_group = {
	.attrs = hypervfs_attrs,
};

/**
 * Initialize the hypervfs sysfs interface
 *
 */
static int __init hypervfs_sysfs_init(void)
{
	hypervfs_kobj = kobject_create_and_add("hyperv", fs_kobj);
	if (!hypervfs_kobj)
		return -ENOMEM;

	if (sysfs_create_group(hypervfs_kobj, &hypervfs_attr_group)) {
		kobject_put(hypervfs_kobj);
		return -ENOMEM;
	}

	return 0;
}

/**
 * Unregister the hypervfs sysfs interface
 *
 */
static void hypervfs_sysfs_cleanup(void)
{
	sysfs_remove_group(hypervfs_kobj, &hypervfs_attr_group);
	kobject_put(hypervfs_kobj);
}

static void hypervfs_inode_init_once(void *data)
{
	struct inode *inode = (struct inode *)data;
	inode_init_once(inode);
}

static int hypervfs_init_inode_cache(void)
{
	hypervfs_inode_cache = kmem_cache_create(
		"hypervfs_inode_cache",
		sizeof(struct inode),
		0,
		(SLAB_RECLAIM_ACCOUNT|SLAB_MEM_SPREAD|SLAB_ACCOUNT),
		hypervfs_inode_init_once
	);

	if (!hypervfs_inode_cache)
		return -ENOMEM;

	return 0;
}

static void hypervfs_destroy_inode_cache(void)
{
	/*
	 * Make sure all delayed rcu free inodes are flushed before we
	 * destroy cache.
	 */
	rcu_barrier();
	kmem_cache_destroy(hypervfs_inode_cache);
}

/**
 * init_hypervfs - Initialize module
 *
 */
static int __init hypervfs_init(void)
{
	int err;
	pr_info("Installing hypervfs file system support\n");

	err = hypervfs_init_inode_cache();
	if (err < 0) {
		pr_err("Failed to register hypervfs for caching\n");
		return err;
	}

	err = hypervfs_sysfs_init();
	if (err < 0) {
		pr_err("Failed to register with sysfs\n");
		goto out_cache;
	}

	err = register_filesystem(&hypervfs_fs_type);
	if (err < 0) {
		pr_err("Failed to register filesystem\n");
		goto out_sysfs_cleanup;
	}

	return 0;

out_sysfs_cleanup:
	hypervfs_sysfs_cleanup();

out_cache:
	hypervfs_destroy_inode_cache();

	return err;
}

/**
 * Shutdown module
 */
static void __exit hypervfs_exit(void)
{
	hypervfs_sysfs_cleanup();
	hypervfs_destroy_inode_cache();
	unregister_filesystem(&hypervfs_fs_type);
}

module_init(hypervfs_init);
module_exit(hypervfs_exit);

MODULE_AUTHOR("Eduard Tofan");
MODULE_LICENSE("GPL");
