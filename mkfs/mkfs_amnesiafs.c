#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <amnesiafs.h>

int main(int argc, char *argv[])
{
	int fd;

	if (argc != 2) {
		printf("Usage: %s device\n", argv[0]);
		return -1;
	}

	fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		perror("Error opening device");
		return -1;
	}

	close(fd);
	return 0;
}
