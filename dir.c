// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/fs.h>
#include <linux/buffer_head.h>

#include "amnesiafs.h"

#include "dir.h"
#include "log.h"
#include "inode.h"

int amnesiafs_iterate(struct file *filp, struct dir_context *ctx)
{
	loff_t pos;
	struct inode *inode;
	struct super_block *sb;
	struct buffer_head *bh;
	struct amnesiafs_inode *sfs_inode;
	struct amnesiafs_dir_record *record;
	int i;

	amnesiafs_debug("iterating over %s", filp->f_path.dentry->d_name.name);

	pos = ctx->pos;
	inode = filp->f_path.dentry->d_inode;
	sb = inode->i_sb;

	if (pos) {
		amnesiafs_debug("exiting early");
		return 0;
	}

	sfs_inode = inode->i_private;

	if (!S_ISDIR(sfs_inode->mode)) {
		amnesiafs_err(
			"inode [%llu][%lu] for fs object %s is not a directory",
			sfs_inode->inode_no, inode->i_ino,
			filp->f_path.dentry->d_name.name);
		return -ENOTDIR;
	}

	bh = sb_bread(sb, sfs_inode->data_block_number);
	BUG_ON(!bh);

	record = (struct amnesiafs_dir_record *)bh->b_data;
	for (i = 0; i < sfs_inode->dir_children_count; i++) {
		dir_emit(ctx, record->filename, 255, record->inode_no,
			 DT_UNKNOWN);
		ctx->pos += sizeof(struct amnesiafs_dir_record);

		pos += sizeof(struct amnesiafs_dir_record);
		record++;
	}
	brelse(bh);

	return 0;
}

const struct file_operations amnesiafs_dir_operations = {
	.owner = THIS_MODULE,
	.iterate = amnesiafs_iterate,
};