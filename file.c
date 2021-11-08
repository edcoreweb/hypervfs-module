#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include "hypervfs.h"

/**
 * Open a file (or directory)
 */
int hypervfs_file_open(struct inode *inode, struct file *file)
{
	if (file->f_flags & O_APPEND)
		generic_file_llseek(file, 0, SEEK_END);

	return 0;
}

const struct file_operations hypervfs_file_operations = {
	.llseek = generic_file_llseek,
	.read_iter = generic_file_read_iter,
	.write_iter = generic_file_write_iter,
	.open = hypervfs_file_open,
	// .release = hypervfs_dir_release,
	// .lock = hypervfs_file_lock,
	// .flock = hypervfs_file_flock,
	// .mmap = hypervfs_file_mmap,
	// .fsync = hypervfs_file_fsync,
};
