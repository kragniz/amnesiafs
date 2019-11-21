// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/slab.h>
#include <linux/key.h>
#include <linux/key-type.h>
#include <keys/user-type.h>

#include "log.h"

struct key_type amnesiafs_key_type = {
	.name = "amnesiafs",
	.preparse = user_preparse,
	.free_preparse = user_free_preparse,
	.instantiate = generic_key_instantiate,
	.revoke = user_revoke,
	.destroy = user_destroy,
	.describe = user_describe,
};

int amnesiafs_get_passphrase(char **passphrase, const char *key_desc)
{
	int err = 0;
	struct key *user_key;
	const struct user_key_payload *upayload;

	user_key = request_key(&amnesiafs_key_type, key_desc, NULL);

	if (IS_ERR(user_key)) {
		amnesiafs_msg(KERN_ERR, "Failed to request key: %ld",
			      PTR_ERR(user_key));
		err = PTR_ERR(user_key);
		goto out_err;
	}

	*passphrase = kzalloc(user_key->datalen + 1, GFP_KERNEL);
	if (!passphrase) {
		err = -ENOMEM;
		goto out_err;
	}

	down_read(&user_key->sem);
	upayload = user_key_payload_locked(user_key);
	if (IS_ERR_OR_NULL(upayload)) {
		err = upayload ? PTR_ERR(upayload) : -EINVAL;
		goto out_key_err;
	}

	strlcpy(*passphrase, upayload->data, user_key->datalen);
	pr_debug("passphrase: '%s'", *passphrase);

	up_read(&user_key->sem);

out_key_err:
	/* make sure the passphrase doesn't stay around longer than necessary */
	key_revoke(user_key);
out_err:
	return err;
}