CC = gcc
LINK = $(shell pkg-config --libs --cflags libkeyutils) $(shell pkg-config --libs --cflags libargon2)
CFLAGS = -g -Wall -fsanitize=address,undefined

all: amnesiafs-store-passphrase

amnesiafs-store-passphrase: store_passphrase.c
	$(CC) $(CFLAGS) $(LINK) -I.. -o amnesiafs-store-passphrase store_passphrase.c

clean:
	$(RM) amnesiafs-store-passphrase
