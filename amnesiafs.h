// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AMNESIAFS_H
#define AMNESIAFS_H

#define AMNESIAFS_MAGIC 0xdec0ded

#define AMNESIAFS_BLOCKSIZE 4096

#define AMNESIAFS_FILENAME_MAX 255

struct amnesiafs_super_block {
	uint64_t magic;
	uint64_t version;

	uint8_t salt[16];

	uint64_t inodes_count;
	uint64_t blocks_available;

	uint8_t padding[4048];
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
	char filename[AMNESIAFS_FILENAME_MAX];
	uint64_t inode_no;
};

_Static_assert(sizeof(struct amnesiafs_super_block) == AMNESIAFS_BLOCKSIZE,
	       "amnesiafs_super_block must remain the same size");

#endif
