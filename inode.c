#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include "hypervfs.h"

/**
 * v9fs_alloc_inode - helper function to allocate an inode
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
 * Helper function to setup an inode
 */
struct inode *hypervfs_get_inode(struct super_block *sb, umode_t mode, dev_t rdev)
{
	struct inode *inode = NULL;
	
	// TODO: init inode

	return inode;
}

/**
 * Release an inode
 */
void hypervfs_evict_inode(struct inode *inode)
{
	
}