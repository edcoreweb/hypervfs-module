#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include "hypervfs.h"

const struct file_operations hypervfs_file_operations = {
	// .llseek = generic_file_llseek,
	// .read_iter = generic_file_read_iter,
	// .write_iter = generic_file_write_iter,
	// .open = hypervfs_file,
	// .release = hypervfs_dir_release,
	// .lock = hypervfs_file_lock,
	// .flock = hypervfs_file_flock,
	// .mmap = hypervfs_file_mmap,
	// .fsync = hypervfs_file_fsync,
};
