CC = gcc
CFLAGS = -g -Wall -fsanitize=address,undefined

all: mkfs.amnesiafs

mkfs.amnesiafs: mkfs_amnesiafs.c
	$(CC) $(CFLAGS) -I.. -o mkfs.amnesiafs mkfs_amnesiafs.c

clean:
	$(RM) mkfs.amnesiafs
