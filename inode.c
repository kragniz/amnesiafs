// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/time.h>

#include "amnesiafs.h"

#include "dir.h"
#include "file.h"
#include "inode.h"
#include "log.h"

struct kmem_cache *amnesiafs_inode_cache = NULL;

void amnesiafs_destroy_inode(struct inode *inode)
{
	struct simplefs_inode *amnesiafs_inode = inode->i_private;

	amnesiafs_debug("freeing inode %p (%lu)\n", amnesiafs_inode,
			inode->i_ino);
	kmem_cache_free(amnesiafs_inode_cache, amnesiafs_inode);
}

struct dentry *amnesiafs_lookup(struct inode *parent_inode,
				struct dentry *child_dentry, unsigned int flags)
{
	struct amnesiafs_inode *parent = parent_inode->i_private;
	struct super_block *sb = parent_inode->i_sb;
	struct buffer_head *bh;
	struct amnesiafs_dir_record *record;
	int i;

	amnesiafs_debug("lookup in: inode=%llu, b=%llu", parent->inode_no,
			parent->data_block_number);

	bh = sb_bread(sb, parent->data_block_number);
	BUG_ON(!bh);
	record = (struct amnesiafs_dir_record *)bh->b_data;
	for (i = 0; i < parent->dir_children_count; i++) {
		amnesiafs_debug("have file: '%s' (ino=%llu)", record->filename,
				record->inode_no);

		if (!strcmp(record->filename, child_dentry->d_name.name)) {
			struct inode *inode =
				amnesiafs_iget(sb, record->inode_no);
			inode_init_owner(
				inode, parent_inode,
				((struct amnesiafs_inode *)inode->i_private)
					->mode);
			d_add(child_dentry, inode);
			return NULL;
		}
		record++;
	}

	amnesiafs_err("No inode found for the filename '%s'",
		      child_dentry->d_name.name);
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

struct inode_operations amnesiafs_inode_operations = {
	.create = amnesiafs_create,
	.lookup = amnesiafs_lookup,
	.mkdir = amnesiafs_mkdir,
};

void amnesiafs_fill_inode(struct super_block *sb, struct inode *inode,
			  struct amnesiafs_inode *amnesiafs_inode)
{
	amnesiafs_debug("filling inode %ld", inode->i_ino);

	inode->i_mode = amnesiafs_inode->mode;
	inode->i_sb = sb;
	inode->i_ino = amnesiafs_inode->inode_no;
	inode->i_op = &amnesiafs_inode_operations;
	inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
	inode->i_private = amnesiafs_inode;

	if (S_ISDIR(amnesiafs_inode->mode)) {
		inode->i_fop = &amnesiafs_dir_operations;
	} else if (S_ISREG(amnesiafs_inode->mode)) {
		inode->i_fop = &amnesiafs_file_operations;
	} else {
		amnesiafs_err(
			"inode %lu is neither a directory nor a regular file",
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

	amnesiafs_debug("gettign sb_read");

	bh = sb_bread(sb, 1);
	BUG_ON(!bh);

	amnesiafs_debug("got sb_read");

	inode = (struct amnesiafs_inode *)(bh->b_data);
	inode_buf = kmem_cache_alloc(amnesiafs_inode_cache, GFP_KERNEL);
	memcpy(inode_buf, inode, sizeof(*inode_buf));
	amnesiafs_debug("inode: dir_children_count: %lld mode: %d",
			inode->dir_children_count, inode->mode);

	brelse(bh);
	return inode_buf;
}

struct inode *amnesiafs_iget(struct super_block *sb, int ino)
{
	struct inode *inode;
	struct amnesiafs_inode *amnesiafs_inode = amnesiafs_get_inode(sb, ino);

	inode = new_inode(sb);
	inode->i_ino = ino;
	inode->i_sb = sb;
	inode->i_op = &amnesiafs_inode_operations;

	if (S_ISDIR(amnesiafs_inode->mode))
		inode->i_fop = &amnesiafs_dir_operations;
	else if (S_ISREG(amnesiafs_inode->mode))
		inode->i_fop = &amnesiafs_file_operations;
	else
		amnesiafs_err("unknown inode type");

	inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);

	inode->i_private = amnesiafs_inode;

	return inode;
}