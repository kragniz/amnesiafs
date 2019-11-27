// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AMNESIAFS_H
#define AMNESIAFS_H

#define AMNESIAFS_MAGIC 0xdec0ded

#define AMNESIAFS_BLOCKSIZE 4096

struct amnesiafs_super_block {
	uint64_t magic;
	uint64_t version;

	uint8_t salt[16];

	uint8_t padding[4057];
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

_Static_assert(sizeof(struct amnesiafs_super_block) == AMNESIAFS_BLOCKSIZE,
	       "amnesiafs_super_block must remain the same size");

#endif
