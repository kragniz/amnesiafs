// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AMNESIAFS_CONFIG_H
#define AMNESIAFS_CONFIG_H

#include <linux/key.h>

struct amnesiafs_config {
	key_serial_t key_id;
};

int amnesiafs_parse_options(char *options, struct amnesiafs_config *config);

#endif