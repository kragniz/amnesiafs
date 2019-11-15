// SPDX-License-Identifier: GPL-2.0-or-later

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <amnesiafs.h>

static int write_superblock(int fd)
{
	ssize_t written;
	struct amnesiafs_super_block sb = {
		.version = 1,
		.magic = AMNESIAFS_MAGIC,
	};

	written = write(fd, &sb, sizeof(sb));
	if (written != AMNESIAFS_BLOCKSIZE) {
		printf("Error: wrote the wrong number of bytes (%zd instead of %d)\n",
		       written, AMNESIAFS_BLOCKSIZE);
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int fd;
	int err = 0;

	if (argc != 2) {
		printf("Usage: %s device\n", argv[0]);
		return 1;
	}

	fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		perror("Error opening device");
		return 1;
	}

	err = write_superblock(fd);
	if (err != 0)
		goto out;

out:
	close(fd);
	return err;
}
