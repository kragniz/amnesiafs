// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/time.h>

#include "amnesiafs.h"

#include "dir.h"
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

struct amnesiafs_inode *amnesiafs_get_inode(struct super_block *sb,
					    uint64_t inode_no)
{
	struct buffer_head *bh;
	struct amnesiafs_inode *inode;
	struct amnesiafs_inode *inode_buf;

	bh = sb_bread(sb, 1);
	BUG_ON(!bh);

	inode = (struct amnesiafs_inode *)(bh->b_data);
	inode_buf = kmem_cache_alloc(amnesiafs_inode_cache, GFP_KERNEL);
	memcpy(inode_buf, inode, sizeof(*inode_buf));
	amnesiafs_debug("inode: dir_children_count: %lld mode: %d",
			inode->dir_children_count, inode->mode);

	brelse(bh);
	return inode_buf;
}

struct dentry *amnesiafs_lookup(struct inode *parent_inode,
				struct dentry *child_dentry, unsigned int flags)
{
	amnesiafs_err("amnesiafs_lookup not implemented");
	return NULL;
}

static int amnesiafs_create(struct inode *dir, struct dentry *dentry,
			    umode_t mode, bool excl)
{
	amnesiafs_err("amnesiafs_create not implemented");
	return -ENOTRECOVERABLE;
}

static int amnesiafs_mkdir(struct inode *dir, struct dentry *dentry,
			   umode_t mode)
{
	amnesiafs_err("amnesiafs_mkdir not implemented");
	return -ENOTRECOVERABLE;
}

struct inode_operations amnesiafs_inode_ops = {
	.create = amnesiafs_create,
	.lookup = amnesiafs_lookup,
	.mkdir = amnesiafs_mkdir,
};