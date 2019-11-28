// SPDX-License-Identifier: GPL-2.0-or-later

#include "log.h"
#include "inode.h"

ssize_t amnesiafs_read(struct file *filp, char __user *buf, size_t len,
		       loff_t *ppos)
{
	amnesiafs_err("amnesiafs_read not implemented");
	return 0;
}

ssize_t amnesiafs_write(struct file *filp, const char __user *buf, size_t len,
			loff_t *ppos)
{
	amnesiafs_err("amnesiafs_write not implemented");
	return 0;
}

const struct file_operations amnesiafs_file_operations = {
	.owner = THIS_MODULE,
	.read = amnesiafs_read,
	.write = amnesiafs_write,
};