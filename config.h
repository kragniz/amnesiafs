// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AMNESIAFS_CONFIG_H
#define AMNESIAFS_CONFIG_H

struct amnesiafs_config {
	char *key_id;
};

int amnesiafs_parse_options(char *options, struct amnesiafs_config *config);

#endif