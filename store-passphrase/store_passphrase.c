// SPDX-License-Identifier: GPL-2.0-or-later

#include <keyutils.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	key_serial_t key;

	if (argc != 2) {
		printf("Usage: %s amnesiafs:key\n", argv[0]);
		return 1;
	}

	char *payload = "hello";

	key = add_key("logon", argv[1], payload, strlen(payload),
		      KEY_SPEC_USER_KEYRING);
	if (key == -1) {
		perror("add_key");
		return 1;
	}

	return 0;
}
