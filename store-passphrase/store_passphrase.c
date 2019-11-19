// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>

#include <argon2.h>
#include <keyutils.h>

#define KEY_LEN 64

ssize_t get_passphrase(char *prompt, char *line, size_t n, FILE *stream)
{
	struct termios old, new;
	int read;

	if (tcgetattr(fileno(stream), &old) != 0)
		return -1;
	new = old;
	new.c_lflag &= ~ECHO;
	if (tcsetattr(fileno(stream), TCSAFLUSH, &new) != 0)
		return -1;

	if (prompt)
		printf("%s", prompt);

	read = getline(&line, &n, stream);

	if (read >= 1 && line[read - 1] == '\n') {
		line[read - 1] = 0;
		read--;
	}
	printf("\n");

	tcsetattr(fileno(stream), TCSAFLUSH, &old);

	return read;
}

int get_key_from_passphrase(char *passphrase, uint8_t *key)
{
	const uint32_t t_cost = 2;
	const uint32_t m_cost = 1 << 16;
	const uint32_t parallelism = 1;
	const char *salt = "todo: move me to the fs superblock and randomize";

	return argon2d_hash_raw(t_cost, m_cost, parallelism, passphrase,
				strlen(passphrase), salt, strlen(salt), key,
				KEY_LEN);
}

int main(int argc, char *argv[])
{
	key_serial_t key;
	size_t passphrase_max = 255;
	char passphrase[passphrase_max];
	ssize_t passphrase_len;
	uint8_t key_value[KEY_LEN];

	if (argc != 2) {
		printf("Usage: %s amnesiafs:key\n", argv[0]);
		return 1;
	}

	passphrase_len = get_passphrase("Passphrase: ", passphrase,
					passphrase_max, stdin);
	if (passphrase_len < 0) {
		perror("Error reading passphrase");
		return 1;
	} else if (passphrase_len == 0) {
		fprintf(stderr, "Invalid passphrase length (%ld)\n",
			passphrase_len);
		return 1;
	}

	int err = get_key_from_passphrase(passphrase, key_value);
	if (err != 0) {
		perror("Error deriving key");
		return 1;
	}

	key = add_key("logon", argv[1], key_value, KEY_LEN,
		      KEY_SPEC_USER_KEYRING);
	if (key == -1) {
		perror("Error adding key");
		return 1;
	}

	printf("%d\n", key);

	return 0;
}
