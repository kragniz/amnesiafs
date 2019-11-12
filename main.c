// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#include "amnesiafs.h"

static void amnesiafs_put_super(struct super_block *sb)
{
	pr_debug("amnesiafs super block destroyed\n");
}

static struct super_operations const amnesiafs_super_ops = {
	.put_super = amnesiafs_put_super,
};

static int amnesiafs_fill_sb(struct super_block *sb, void *data, int silent)
{
	struct inode *root = NULL;

	sb->s_magic = AMNESIAFS_MAGIC;
	sb->s_op = &amnesiafs_super_ops;

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
}

static struct dentry *amnesiafs_mount(struct file_system_type *type, int flags,
				      char const *dev, void *data)
{
	struct dentry *const entry =
		mount_bdev(type, flags, dev, data, amnesiafs_fill_sb);
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
		pr_err("Failed to register filesystem\n");
	}
	return 0;
}

static void __exit amnesiafs_exit(void)
{
	int err = unregister_filesystem(&amnesiafs_fs_type);
	if (err < 0) {
		pr_err("Failed to unregister filesystem\n");
	}
}

module_init(amnesiafs_init);
module_exit(amnesiafs_exit);

MODULE_DESCRIPTION("amnesiafs");
MODULE_AUTHOR("Louis Taylor");
MODULE_LICENSE("GPL");
