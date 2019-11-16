// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/types.h>

#include <linux/string.h>
#include <linux/parser.h>
#include <linux/gfp.h>

#include "config.h"

enum { OPT_KEY_ID,
};

static const match_table_t tokens = {
	{ OPT_KEY_ID, "key_id=%s" },
};

int amnesiafs_parse_options(char *options, struct amnesiafs_config *config)
{
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
			config->key_id = kstrdup(args[0].from, GFP_KERNEL);
			pr_debug("key_id: %s", config->key_id);
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