// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/types.h>

#include <linux/string.h>
#include <linux/parser.h>
#include <linux/gfp.h>

#include "config.h"

enum { OPT_KEY_NAME,
       OPT_ERR,
};

static const match_table_t tokens = {
	{ OPT_KEY_NAME, "key_name=%s" },
	{ OPT_ERR, NULL },
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
		case OPT_KEY_NAME:
			config->key_desc = kstrdup(args[0].from, GFP_KERNEL);
			pr_debug("key_name: %s", config->key_desc);
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