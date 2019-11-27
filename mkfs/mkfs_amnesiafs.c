// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <amnesiafs.h>

static int ensure_random_salt(uint8_t *buf)
{
	int n = sizeof(buf);
	int r = getrandom(buf, n, GRND_RANDOM);

	if (r > 0) {
		if (r != n) {
			printf("Error: got the wrong number of random bytes (%d instead of %d)\n",
			       r, n);
			return -EAGAIN; /* not sure if this will ever happen */
		}
		return 0;
	} else {
		return -errno;
	}
}

static int write_root_inode(int fd)
{
	ssize_t ret;

	struct amnesiafs_inode root_inode;

	root_inode.mode = S_IFDIR;
	root_inode.inode_no = 1;
	root_inode.data_block_number = 4;
	root_inode.dir_children_count = 1;

	ret = write(fd, &root_inode, sizeof(root_inode));

	if (ret != sizeof(root_inode)) {
		printf("Error: wrote the wrong number of bytes (%zd instead of %ld)\n",
		       ret, sizeof(root_inode));
		return 1;
	}

	return 0;
}

static int write_superblock(int fd)
{
	int err = 0;
	ssize_t written;
	uint8_t salt[16];

	err = ensure_random_salt(salt);
	if (err < 0) {
		return err;
	}

	struct amnesiafs_super_block sb = {
		.version = 1,
		.magic = AMNESIAFS_MAGIC,
	};

	/* copy salt */
	memcpy(&sb.salt, salt, sizeof(sb.salt));

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
	if (err != 0) {
		perror("Error writing superblock");
		goto out;
	}

	err = write_root_inode(fd);
	if (err != 0) {
		perror("Error writing root inode");
		goto out;
	}

out:
	close(fd);
	return err;
}
