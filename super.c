// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "amnesiafs.h"
#include "config.h"
#include "log.h"
#include "keys.h"

static void amnesiafs_put_super(struct super_block *sb)
{
	pr_debug("amnesiafs super block destroyed\n");
}

static struct super_operations const amnesiafs_super_ops = {
	.put_super = amnesiafs_put_super,
	.statfs = simple_statfs,
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
		amnesiafs_msg(KERN_ERR, "missing key_id");
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
		amnesiafs_msg(KERN_INFO,
			      "magic mismatch: wanted 0x%x, read 0x%llx",
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
		pr_err("inode allocation failed\n");
		return -ENOMEM;
	}

	root->i_ino = 0;
	root->i_sb = sb;
	root->i_atime = root->i_mtime = root->i_ctime = current_time(root);
	inode_init_owner(root, NULL, S_IFDIR);

	sb->s_root = d_make_root(root);
	if (!sb->s_root) {
		pr_err("root creation failed\n");
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
		pr_err("amnesiafs mounting failed\n");
	else
		pr_debug("amnesiafs mounted\n");
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
		pr_err("failed to register filesystem\n");
	}
	return 0;
}

static void __exit amnesiafs_exit(void)
{
	int err = unregister_filesystem(&amnesiafs_fs_type);
	if (err < 0) {
		pr_err("failed to unregister filesystem\n");
	}
}

module_init(amnesiafs_init);
module_exit(amnesiafs_exit);

MODULE_DESCRIPTION("amnesiafs");
MODULE_AUTHOR("Louis Taylor");
MODULE_LICENSE("GPL");
