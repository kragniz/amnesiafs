// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AMNESIAFS_H
#define AMNESIAFS_H

#define AMNESIAFS_MAGIC 0xdec0ded;

struct amnesiafs_super_block {
	uint64_t magic;
	uint64_t version;

	char padding[4073];
};

_Static_assert(sizeof(struct amnesiafs_super_block) == 4096,
	       "amnesiafs_super_block must be a constent size");

#endif
