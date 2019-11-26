// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AMNESIAFS_DIR_H
#define AMNESIAFS_DIR_H

#include <linux/fs.h>

extern const struct file_operations amnesiafs_dir_operations;

int amnesiafs_iterate(struct file *filp, struct dir_context *ctx);

#endif
