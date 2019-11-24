// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <argon2.h>
#include <keyutils.h>

#include <amnesiafs.h>

#define KEY_LEN 64

ssize_t get_passphrase(char *prompt, char *line, size_t n, FILE *stream)
{
	struct termios old, new;
	int read;
	bool tty = isatty(fileno(stream));

	if (tty) {
		if (tcgetattr(fileno(stream), &old) != 0)
			return -1;
		new = old;
		new.c_lflag &= ~ECHO;
		if (tcsetattr(fileno(stream), TCSAFLUSH, &new) != 0)
			return -1;
	}

	if (prompt)
		printf("%s", prompt);

	read = getline(&line, &n, stream);

	if (read >= 1 && line[read - 1] == '\n') {
		line[read - 1] = 0;
		read--;
	}
	printf("\n");

	if (tty) {
		tcsetattr(fileno(stream), TCSAFLUSH, &old);
	}

	return read;
}

int get_key_from_passphrase(char *passphrase, uint8_t *key, uint8_t *salt,
			    size_t salt_len)
{
	const uint32_t t_cost = 2;
	const uint32_t m_cost = 1 << 16;
	const uint32_t parallelism = 1;

	return argon2d_hash_raw(t_cost, m_cost, parallelism, passphrase,
				strlen(passphrase), salt, salt_len, key,
				KEY_LEN);
}

struct amnesiafs_super_block read_superblock(char *device)
{
	FILE *devicef;
	size_t read;
	struct amnesiafs_super_block sb;

	devicef = fopen(device, "r");
	if (devicef == NULL) {
		perror("Error opening device");
		exit(1);
	}

	read = fread(&sb, sizeof(sb), 1, devicef);
	if (read != 1) {
		fprintf(stderr,
			"Error: read the wrong number of bytes (%zd instead of %ld)\n",
			read, sizeof(sb));
		exit(1);
	}

	if (sb.magic != AMNESIAFS_MAGIC) {
		fprintf(stderr,
			"Error: magic did not match, are you sure that's an amnesiafs filesystem (0x%lx instead of 0x%x)\n",
			sb.magic, AMNESIAFS_MAGIC);
		exit(1);
	}

	fclose(devicef);
	return sb;
}

int main(int argc, char *argv[])
{
	struct amnesiafs_super_block sb;
	key_serial_t key;
	size_t passphrase_max = 255;
	char passphrase[passphrase_max];
	ssize_t passphrase_len;
	size_t salt_len;
	uint8_t key_value[KEY_LEN];

	if (argc != 3) {
		printf("Usage: %s <key> <device>\n", argv[0]);
		return 1;
	}

	sb = read_superblock(argv[2]);

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

	salt_len = sizeof(sb.salt) / sizeof(sb.salt[0]);
	int err = get_key_from_passphrase(passphrase, key_value, sb.salt,
					  salt_len);
	if (err != 0) {
		perror("Error deriving key");
		return 1;
	}

	key = add_key("amnesiafs", argv[1], key_value, KEY_LEN,
		      KEY_SPEC_USER_KEYRING);
	if (key == -1) {
		perror("Error adding key");
		return 1;
	}

	printf("%d\n", key);

	return 0;
}
