CC = gcc
CFLAGS = -g -Wall -fsanitize=address

all: mkfs.amnesiafs

mkfs.amnesiafs: mkfs_amnesiafs.c
	$(CC) $(CFLAGS) -o mkfs.amnesiafs mkfs_amnesiafs.c

clean:
	$(RM) mkfs.amnesiafs
