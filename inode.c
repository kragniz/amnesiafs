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
#include "super.h"

struct kmem_cache *amnesiafs_inode_cache = NULL;

struct amnesiafs_inode *amnesiafs_get_inode_from_generic(struct inode *inode)
{
	return inode->i_private;
}

void amnesiafs_destroy_inode(struct inode *inode)
{
	struct amnesiafs_inode *amnesiafs_inode = inode->i_private;

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

static struct amnesiafs_inode *
amnesiafs_inode_search(struct super_block *sb, struct amnesiafs_inode *start,
		       struct amnesiafs_inode *search)
{
	uint64_t count = 0;
	while (start->inode_no != search->inode_no &&
	       count < amnesiafs_get_super(sb)->inodes_count) {
		count++;
		start++;
	}

	if (start->inode_no == search->inode_no) {
		return start;
	}

	return NULL;
}

static void amnesiafs_inode_add(struct super_block *vsb,
				struct amnesiafs_inode *inode)
{
	struct amnesiafs_super_block *sb = amnesiafs_get_super(vsb);
	struct buffer_head *bh = NULL;
	struct amnesiafs_inode *inode_iterator = NULL;

	amnesiafs_debug("adding new inode");

	amnesiafs_debug("inode count: %lld", sb->inodes_count);
	bh = sb_bread(vsb, 1); /* TODO: is this the right block to use? */
	BUG_ON(!bh);

	inode_iterator = (struct amnesiafs_inode *)bh->b_data;

	inode_iterator += sb->inodes_count;

	memcpy(inode_iterator, inode, sizeof(struct amnesiafs_inode));
	sb->inodes_count++;

	mark_buffer_dirty(bh);
	amnesiafs_sync_super(vsb);
	brelse(bh);
}

int amnesiafs_inode_save(struct super_block *sb,
			 struct amnesiafs_inode *amnesiafs_inode)
{
	struct amnesiafs_inode *inode_iterator;
	struct buffer_head *bh;

	bh = sb_bread(sb, 4);
	BUG_ON(!bh);

	inode_iterator = amnesiafs_inode_search(
		sb, (struct amnesiafs_inode *)bh->b_data, amnesiafs_inode);

	if (inode_iterator) {
		memcpy(inode_iterator, amnesiafs_inode,
		       sizeof(*inode_iterator));
		amnesiafs_debug("updated inode");

		mark_buffer_dirty(bh);
		sync_dirty_buffer(bh);
	} else {
		amnesiafs_err("couldn't update inode");
		return -EIO;
	}

	brelse(bh);

	return 0;
}

static int amnesiafs_create_fs_object(struct inode *dir, struct dentry *dentry,
				      umode_t mode)
{
	struct inode *inode;
	struct amnesiafs_inode *amnesiafs_inode;
	struct super_block *sb;
	struct amnesiafs_inode *parent_dir_inode;
	struct buffer_head *bh;
	struct amnesiafs_dir_record *dir_contents_datablock;
	int err;

	sb = dir->i_sb;

	if (!S_ISDIR(mode) && !S_ISREG(mode)) {
		amnesiafs_err("neither a file nor a directory");
		return -EINVAL;
	}

	inode = new_inode(sb);
	if (!inode) {
		return -ENOMEM;
	}

	inode->i_sb = sb;
	inode->i_op = &amnesiafs_inode_operations;
	inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
	inode->i_ino = 100;

	amnesiafs_inode = kmem_cache_alloc(amnesiafs_inode_cache, GFP_KERNEL);
	amnesiafs_inode->inode_no = inode->i_ino;
	inode->i_private = amnesiafs_inode;
	amnesiafs_inode->mode = mode;

	if (S_ISDIR(mode)) {
		amnesiafs_debug("new directory creation");
		amnesiafs_inode->dir_children_count = 0;
		inode->i_fop = &amnesiafs_dir_operations;
	} else if (S_ISREG(mode)) {
		amnesiafs_debug("new file creation request");
		amnesiafs_inode->file_size = 0;
		inode->i_fop = &amnesiafs_file_operations;
	}

	/* TODO: actually choose a block */
	amnesiafs_inode->data_block_number = 100;

	amnesiafs_inode_add(sb, amnesiafs_inode);

	parent_dir_inode = dir->i_private;
	bh = sb_bread(sb, parent_dir_inode->data_block_number);
	BUG_ON(!bh);

	dir_contents_datablock = (struct amnesiafs_dir_record *)bh->b_data;

	dir_contents_datablock += parent_dir_inode->dir_children_count;

	dir_contents_datablock->inode_no = amnesiafs_inode->inode_no;
	strcpy(dir_contents_datablock->filename, dentry->d_name.name);

	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

	parent_dir_inode->dir_children_count++;
	err = amnesiafs_inode_save(sb, parent_dir_inode);
	if (err) {
		return err;
	}

	inode_init_owner(inode, dir, mode);
	d_add(dentry, inode);

	return 0;
}

static int amnesiafs_create(struct inode *dir, struct dentry *dentry,
			    umode_t mode, bool excl)
{
	amnesiafs_err("amnesiafs_create not implemented");
	amnesiafs_debug("trying '%s'", dentry->d_name.name);
	return amnesiafs_create_fs_object(dir, dentry, mode);
}

static int amnesiafs_mkdir(struct inode *dir, struct dentry *dentry,
			   umode_t mode)
{
	amnesiafs_err("amnesiafs_mkdir not implemented");
	return amnesiafs_create_fs_object(dir, dentry, S_IFDIR | mode);
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