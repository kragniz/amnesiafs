// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/buffer_head.h>
#include <linux/time.h>
#include <linux/stat.h>

#include "amnesiafs.h"

#include "dir.h"
#include "inode.h"
#include "log.h"

struct kmem_cache *amnesiafs_inode_cache = NULL;

void amnesiafs_fill_inode(struct super_block *sb, struct inode *inode,
			  struct amnesiafs_inode *amnesiafs_inode)
{
	inode->i_mode = amnesiafs_inode->mode;
	inode->i_sb = sb;
	inode->i_ino = amnesiafs_inode->inode_no;
	//inode->i_op = &amnesiafs_inode_ops;
	inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
	inode->i_private = amnesiafs_inode;

	if (S_ISDIR(amnesiafs_inode->mode)) {
		inode->i_fop = &amnesiafs_dir_operations;
	} else if (S_ISREG(amnesiafs_inode->mode)) {
		//inode->i_fop = &amnesiafs_file_operations;
	} else {
		printk(KERN_WARNING
		       "Inode %lu is neither a directory nor a regular file",
		       inode->i_ino);
		inode->i_fop = NULL;
	}
}

// struct amnesiafs_inode *amnesiafs_get_inode(struct super_block *sb,
// 					    uint64_t inode_no)
// {
// 	struct buffer_head *bh;
// 	struct amnesiafs_inode *inode;
// 	struct amnesiafs_inode *inode_buf;

// 	bh = sb_bread(sb, 2);
// 	BUG_ON(!bh);

// 	inode = (struct amnesiafs_inode *)(bh->b_data + (
// 							      sb, inode_no));
// 	inode_buf = kmem_cache_alloc(amnesiafs_inode_cache, GFP_KERNEL);
// 	memcpy(inode_buf, inode, sizeof(*inode_buf));

// 	brelse(bh);
// 	return inode_buf;
// }
