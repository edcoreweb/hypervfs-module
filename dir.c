#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include "hypervfs.h"

const struct file_operations hypervfs_dir_operations = {
	.read = generic_read_dir,
	.llseek = generic_file_llseek,
	// .iterate_shared = hypervfs_dir_readdir,
	// .open = hypervfs_file_open,
	// .release = hypervfs_dir_release,
	// .fsync = hypervfs_file_fsync,
};
