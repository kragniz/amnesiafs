// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/buffer_head.h>
#include <linux/uio.h>

#include "inode.h"
#include "log.h"
#include "super.h"

ssize_t amnesiafs_read_iter(struct kiocb *iocb, struct iov_iter *to)
{
	struct file *file = iocb->ki_filp;
	struct inode *inode = file->f_mapping->host;
	struct amnesiafs_inode *amnesiafs_inode =
		amnesiafs_get_inode_from_generic(inode);
	struct buffer_head *bh;
	ssize_t err;
	char *buffer;
	ssize_t read = 0;

	amnesiafs_debug("amnesiafs_read_iter");

	if (iocb->ki_pos >= amnesiafs_inode->file_size) {
		return 0;
	}

	bh = sb_bread(inode->i_sb, amnesiafs_inode->data_block_number);
	if (!bh) {
		amnesiafs_err("reading the block number [%llu] failed.",
			      amnesiafs_inode->data_block_number);
		err = -EIO;
		goto out;
	}

	buffer = (char *)bh->b_data;
	int n = min(amnesiafs_inode->file_size, iov_iter_iovec(to).iov_len);
	read = copy_to_iter(buffer, n, to);
	amnesiafs_debug("want %d bytes, got %ld", n, read);
	if (read <= 0) {
		brelse(bh);
		amnesiafs_err("copy_to_iter failed");
		err = -EFAULT;
		goto out;
	}

	brelse(bh);
	amnesiafs_debug("returning %d", read);
	return read;

out:
	return err;
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

	amnesiafs_debug("amnesiafs_write_iter %s",
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

	written = copy_from_iter(buffer,
				 min((size_t)AMNESIAFS_BLOCKSIZE,
				     iov_iter_iovec(from).iov_len),
				 from);
	if (written <= 0) {
		brelse(bh);
		amnesiafs_err("copy_from_iter failed");
		err = -EFAULT;
		goto out;
	}

	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

	amnesiafs_inode->file_size = written;
	err = amnesiafs_inode_save(inode->i_sb, amnesiafs_inode);
	if (err < 0)
		goto out;

	return written;

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