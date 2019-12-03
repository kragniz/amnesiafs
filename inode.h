// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AMNESIAFS_INODE_H
#define AMNESIAFS_INODE_H

#include <linux/fs.h>

extern struct kmem_cache *amnesiafs_inode_cache;

extern struct inode_operations amnesiafs_inode_operations;

struct dentry *amnesiafs_lookup(struct inode *parent_inode,
				struct dentry *child_dentry,
				unsigned int flags);

struct amnesiafs_inode *amnesiafs_get_inode(struct super_block *sb,
					    uint64_t inode_no);

struct amnesiafs_inode *amnesiafs_get_inode_from_generic(struct inode *inode);

void amnesiafs_destroy_inode(struct inode *inode);

struct inode *amnesiafs_iget(struct super_block *sb, int ino);

#endif
