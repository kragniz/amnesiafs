// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AMNESIAFS_CONFIG_H
#define AMNESIAFS_CONFIG_H

struct amnesiafs_config {
	/* name of the user's passphrase key */
	char *key_desc;

	/* probably not the best place to store this */
	char *passphrase;
};

int amnesiafs_parse_options(char *options, struct amnesiafs_config *config);

#endif