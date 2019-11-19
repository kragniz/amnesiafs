all: amnesiafs.ko mkfs.amnesiafs amnesiafs-store-passphrase

KDIR = /lib/modules/`uname -r`/build

amnesiafs.ko: Kbuild *.c *.h
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
	make -C mkfs clean
	rm -f mkfs.amnesiafs
	make -C store-passphrase clean
	rm -f amnesiafs-store-passphrase

fmt:
	clang-format -style=file -i *.c *.h mkfs/*.c store-passphrase/*.c

mkfs.amnesiafs: mkfs/*
	make -C mkfs
	cp mkfs/mkfs.amnesiafs .

amnesiafs-store-passphrase: store-passphrase/*
	make -C store-passphrase
	cp store-passphrase/amnesiafs-store-passphrase .

test: amnesiafs.ko mkfs.amnesiafs
	./tests/run-qemu.sh
