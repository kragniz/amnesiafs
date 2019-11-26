// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/fs.h>
#include <linux/buffer_head.h>

#include "dir.h"
#include "log.h"

struct amnesiafs_inode {
	mode_t mode;
	uint64_t inode_no;
	uint64_t data_block_number;

	union {
		uint64_t file_size;
		uint64_t dir_children_count;
	};
};

struct amnesiafs_dir_record {
	char filename[255];
	uint64_t inode_no;
};

int amnesiafs_iterate(struct file *filp, struct dir_context *ctx)
{
	loff_t pos;
	struct inode *inode;
	struct super_block *sb;
	struct buffer_head *bh;
	struct amnesiafs_inode *sfs_inode;
	struct amnesiafs_dir_record *record;
	int i;

	pos = ctx->pos;
	inode = filp->f_path.dentry->d_inode;
	sb = inode->i_sb;

	if (pos) {
		return 0;
	}

	sfs_inode = inode->i_private;

	if (!S_ISDIR(sfs_inode->mode)) {
		amnesiafs_err(
			"inode [%llu][%lu] for fs object [%s] not a directory",
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