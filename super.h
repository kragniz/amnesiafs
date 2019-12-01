// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AMNESIAFS_SUPER_H
#define AMNESIAFS_SUPER_H

#include <linux/fs.h>

#include "amnesiafs.h"

extern const struct super_operations amnesiafs_super_operations;

int amnesiafs_fill_super(struct super_block *sb, void *data, int silent);

void amnesiafs_sync_super(struct super_block *vsb);

struct amnesiafs_super_block *amnesiafs_get_super(struct super_block *sb);

#endif
