// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/stat.h>

#include "amnesiafs.h"
#include "super.h"
#include "inode.h"
#include "keys.h"
#include "log.h"

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
	int err;

	amnesiafs_inode_cache = kmem_cache_create(
		"amnesiafs_inode_cache", sizeof(struct amnesiafs_inode), 0,
		(SLAB_RECLAIM_ACCOUNT | SLAB_MEM_SPREAD), NULL);
	if (!amnesiafs_inode_cache)
		return -ENOMEM;

	err = register_filesystem(&amnesiafs_fs_type);
	if (err < 0)
		amnesiafs_err("failed to register filesystem\n");

	err = register_key_type(&amnesiafs_key_type);
	if (err < 0)
		amnesiafs_err("failed to register key type\n");

	return err;
}

static void __exit amnesiafs_exit(void)
{
	int err = unregister_filesystem(&amnesiafs_fs_type);
	if (err < 0)
		amnesiafs_err("failed to unregister filesystem\n");

	kmem_cache_destroy(amnesiafs_inode_cache);
	unregister_key_type(&amnesiafs_key_type);
}

module_init(amnesiafs_init);
module_exit(amnesiafs_exit);

MODULE_DESCRIPTION("amnesiafs");
MODULE_AUTHOR("Louis Taylor");
MODULE_LICENSE("GPL");
