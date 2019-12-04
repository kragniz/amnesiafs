// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/buffer_head.h>
#include <linux/uio.h>

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
	struct amnesiafs_super_block *sb = amnesiafs_get_super(inode->i_sb);
	struct amnesiafs_inode *amnesiafs_inode =
		amnesiafs_get_inode_from_generic(inode);
	struct buffer_head *bh;
	ssize_t written = 0;
	ssize_t err;
	char *buffer;

	amnesiafs_debug("amnesiafs_read_iter %s",
			iocb->ki_filp->f_path.dentry->d_iname);

	err = file_update_time(file);
	if (err) {
		amnesiafs_err("file_update_time failed");
		goto out;
	}

	bh = sb_bread(inode->i_sb, amnesiafs_inode->data_block_number);
	if (!bh) {
		amnesiafs_err("reading the block number [%llu] failed.",
			      amnesiafs_inode->data_block_number);
		err = -EIO;
		goto out;
	}

	buffer = (char *)bh->b_data;
	buffer += iocb->ki_pos;

	if (copy_from_iter(buffer, 4, from) != 4) {
		brelse(bh);
		amnesiafs_err("copy_from_iter failed");
		return -EFAULT;
	}

	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

	amnesiafs_inode->file_size = 4;
	err = amnesiafs_inode_save(inode->i_sb, amnesiafs_inode);

out:
	return err;
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