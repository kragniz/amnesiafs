// SPDX-License-Identifier: GPL-2.0-or-later

#include "inode.h"
#include "log.h"
#include "super.h"

ssize_t amnesiafs_read_iter(struct kiocb *iocb, struct iov_iter *to)
{
	amnesiafs_debug("amnesiafs_read_iter");
	return generic_file_read_iter(iocb, to);
}

ssize_t amnesiafs_write_iter(struct kiocb *iocb, struct iov_iter *from)
{
	struct file *file = iocb->ki_filp;
	struct inode *inode = file->f_mapping->host;

	amnesiafs_debug("amnesiafs_read_iter %s",
			iocb->ki_filp->f_path.dentry->d_iname);

	return generic_file_write_iter(iocb, from);
}

int amnesiafs_fsync(struct file *file, loff_t start, loff_t end, int datasync)
{
	int ret;
	struct super_block *sb = file->f_mapping->host->i_sb;

	ret = generic_file_fsync(file, start, end, datasync);
	if (ret == -EIO)
		amnesiafs_err(
			"detected IO error when writing metadata buffers. 0x%lx",
			sb->s_magic);
	return ret;
}

const struct file_operations amnesiafs_file_operations = {
	.owner = THIS_MODULE,
	.read_iter = amnesiafs_read_iter,
	.write_iter = amnesiafs_write_iter,
	.fsync = amnesiafs_fsync,
};