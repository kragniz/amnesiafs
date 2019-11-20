// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/slab.h>
#include <linux/key.h>
#include <keys/user-type.h>

#include "log.h"

int amnesiafs_get_passphrase(char **passphrase, const char *key_desc)
{
	int err = 0;
	struct key *user_key;
	const struct user_key_payload *upayload;

	user_key = request_key(&key_type_logon, key_desc, NULL);

	if (IS_ERR(user_key)) {
		amnesiafs_msg(KERN_ERR, "Failed to request key: %ld",
			      PTR_ERR(user_key));
		err = PTR_ERR(user_key);
		goto out_err;
	}

	*passphrase = kzalloc(user_key->datalen + 1, GFP_KERNEL);
	if (!passphrase)
		return -ENOMEM;

	down_read(&user_key->sem);
	upayload = user_key_payload_locked(user_key);
	if (IS_ERR_OR_NULL(upayload)) {
		err = upayload ? PTR_ERR(upayload) : -EINVAL;
		goto out_err;
	}

	strncpy(*passphrase, upayload->data, user_key->datalen);
	pr_debug("passphrase: '%s'", *passphrase);

	up_read(&user_key->sem);

	/* make sure the passphrase doesn't stay around longer than necessary */
	key_revoke(user_key);

out_err:
	return err;
}