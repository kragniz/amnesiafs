// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/types.h>

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/parser.h>
#include <linux/gfp.h>

#include "config.h"

enum { OPT_KEY_ID,
       OPT_ERR,
};

static const match_table_t tokens = {
	{ OPT_KEY_ID, "key_id=%s" },
	{ OPT_ERR, NULL },
};

int amnesiafs_parse_options(char *options, struct amnesiafs_config *config)
{
	int err;
	char *p;
	substring_t args[MAX_OPT_ARGS];

	if (!options)
		return 0;

	while ((p = strsep(&options, ","))) {
		int token;

		if (!*p)
			continue;

		token = match_token(p, tokens, args);
		switch (token) {
		case OPT_KEY_ID:
			err = kstrtos32(args[0].from, 10, &config->key_id);
			if (err)
				return err;

			pr_debug("key_id: %d", config->key_id);

			break;
		default: {
			pr_err("unrecognized mount option \"%s\" or missing value",
			       p);
			return -EINVAL;
			break;
		}
		}
	}
	return 0;
}