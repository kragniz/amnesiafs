// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef AMNESIAFS_KEYS_H
#define AMNESIAFS_KEYS_H

#include <linux/key.h>
#include <linux/key-type.h>
#include <keys/user-type.h>

extern struct key_type amnesiafs_key_type;

int amnesiafs_get_passphrase(char **passphrase, const char *key_desc);

#endif
