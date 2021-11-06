#define HYPERVFS_MAGIC 0x10a486

extern struct file_system_type hypervfs_fs_type;
extern const struct dentry_operations hypervfs_dentry_operations;
extern struct kmem_cache *hypervfs_inode_cache;

struct inode *hypervfs_alloc_inode(struct super_block *sb);
void hypervfs_free_inode(struct inode *inode);
struct inode *hypervfs_get_inode(struct super_block *sb, umode_t mode, dev_t);
void hypervfs_evict_inode(struct inode *inode);
