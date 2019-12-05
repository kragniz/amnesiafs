// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/stat.h>

#include "amnesiafs.h"
#include "config.h"
#include "dir.h"
#include "inode.h"
#include "keys.h"
#include "log.h"

struct amnesiafs_super_block *amnesiafs_get_super(struct super_block *sb)
{
	return sb->s_fs_info;
}

static void amnesiafs_put_super(struct super_block *sb)
{
	amnesiafs_debug("amnesiafs super block destroyed");
}

const struct super_operations amnesiafs_super_operations = {
	.put_super = amnesiafs_put_super,
	.statfs = simple_statfs,
	.destroy_inode = amnesiafs_destroy_inode,
};

int amnesiafs_fill_super(struct super_block *sb, void *data, int silent)
{
	int err = 0;
	struct inode *root = NULL;
	struct buffer_head *bh = NULL;
	struct amnesiafs_super_block *sb_disk;

	struct amnesiafs_config *config =
		kzalloc(sizeof(struct amnesiafs_config), GFP_KERNEL);
	if (!config)
		return -ENOMEM;

	err = amnesiafs_parse_options((char *)data, config);
	if (err)
		goto out_err;

	err = -EINVAL;
	if (!config->key_desc) {
		amnesiafs_err("missing key_id");
		goto out_err;
	}

	err = amnesiafs_get_passphrase(&config->passphrase, config->key_desc);
	if (err)
		goto out_key_err;

	/* read the block at 0 */
	bh = sb_bread(sb, 0);
	BUG_ON(!bh);

	sb_disk = (struct amnesiafs_super_block *)bh->b_data;

	/* make sure the magic number is what we're expecting */
	if (sb_disk->magic != AMNESIAFS_MAGIC) {
		amnesiafs_info("magic mismatch: wanted 0x%x, read 0x%llx",
			       AMNESIAFS_MAGIC, sb_disk->magic);
		return -EINVAL;
	}

	amnesiafs_debug(
		"loaded super: version: %lld, inodes_count: %lld, blocks_available: %lld",
		sb_disk->version, sb_disk->inodes_count,
		sb_disk->blocks_available);

	sb->s_blocksize = AMNESIAFS_BLOCKSIZE;
	/* number of bits the blocksize requires, I think ??? */
	sb->s_blocksize_bits = PAGE_SHIFT;
	sb->s_magic = AMNESIAFS_MAGIC;
	sb->s_fs_info = sb_disk;
	sb->s_op = &amnesiafs_super_operations;
	sb->s_time_gran = 1;

	root = new_inode(sb);
	if (!root) {
		amnesiafs_err("inode allocation failed\n");
		return -ENOMEM;
	}

	root->i_ino = 1;
	root->i_sb = sb;
	root->i_op = &amnesiafs_inode_operations;
	root->i_fop = &amnesiafs_dir_operations;
	root->i_atime = root->i_mtime = root->i_ctime = current_time(root);
	inode_init_owner(root, NULL, S_IFDIR);

	root->i_private = amnesiafs_get_inode(sb, 1);

	sb->s_root = d_make_root(root);
	if (!sb->s_root) {
		amnesiafs_err("root creation failed\n");
		return -ENOMEM;
	}

	return 0;

out_key_err:
	if (config->passphrase)
		kfree(config->passphrase);
out_err:
	if (config->key_desc)
		kfree(config->key_desc);
	kfree(config);
	return err;
}

void amnesiafs_sync_super(struct super_block *vsb)
{
	struct buffer_head *bh = NULL;
	struct amnesiafs_super_block *sb = amnesiafs_get_super(vsb);

	bh = sb_bread(vsb, 4);
	BUG_ON(!bh);

	bh->b_data = (char *)sb;
	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);
}