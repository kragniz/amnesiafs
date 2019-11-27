// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AMNESIAFS_INODE_H
#define AMNESIAFS_INODE_H

#include <linux/fs.h>

extern struct kmem_cache *amnesiafs_inode_cache;

struct amnesiafs_inode *amnesiafs_get_inode(struct super_block *sb,
					    uint64_t inode_no);

#endif
