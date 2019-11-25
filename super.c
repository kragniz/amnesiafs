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
#include "log.h"
#include "keys.h"

static void amnesiafs_put_super(struct super_block *sb)
{
	amnesiafs_debug("amnesiafs super block destroyed");
}

static struct super_operations const amnesiafs_super_ops = {
	.put_super = amnesiafs_put_super,
	.statfs = simple_statfs,
};

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

static int amnesiafs_iterate(struct file *filp, struct dir_context *ctx)
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

static int amnesiafs_fill_super(struct super_block *sb, void *data, int silent)
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

	sb->s_blocksize = PAGE_SIZE;
	sb->s_blocksize_bits = PAGE_SHIFT;
	sb->s_magic = AMNESIAFS_MAGIC;
	sb->s_op = &amnesiafs_super_ops;
	sb->s_time_gran = 1;

	root = new_inode(sb);
	if (!root) {
		amnesiafs_err("inode allocation failed\n");
		return -ENOMEM;
	}

	root->i_ino = 0;
	root->i_sb = sb;
	root->i_fop = &amnesiafs_dir_operations;
	root->i_atime = root->i_mtime = root->i_ctime = current_time(root);
	inode_init_owner(root, NULL, S_IFDIR);

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

static struct dentry *amnesiafs_mount(struct file_system_type *type, int flags,
				      char const *dev, void *data)
{
	struct dentry *const entry =
		mount_bdev(type, flags, dev, data, amnesiafs_fill_super);
	if (IS_ERR(entry))
		amnesiafs_err("amnesiafs mounting failed\n");
	else
		amnesiafs_debug("mounted");
	return entry;
}

struct file_system_type amnesiafs_fs_type = {
	.owner = THIS_MODULE,
	.name = "amnesiafs",
	.mount = amnesiafs_mount,
	.kill_sb = kill_block_super,
	.fs_flags = FS_REQUIRES_DEV,
};

static int __init amnesiafs_init(void)
{
	int err = register_filesystem(&amnesiafs_fs_type);
	if (err < 0) {
		amnesiafs_err("failed to register filesystem\n");
	}
	err = register_key_type(&amnesiafs_key_type);
	if (err < 0)
		amnesiafs_err("failed to register key type\n");
	return err;
}

static void __exit amnesiafs_exit(void)
{
	int err = unregister_filesystem(&amnesiafs_fs_type);
	if (err < 0) {
		amnesiafs_err("failed to unregister filesystem\n");
	}
	unregister_key_type(&amnesiafs_key_type);
}

module_init(amnesiafs_init);
module_exit(amnesiafs_exit);

MODULE_DESCRIPTION("amnesiafs");
MODULE_AUTHOR("Louis Taylor");
MODULE_LICENSE("GPL");
