all: amnesiafs.ko

KDIR = /lib/modules/`uname -r`/build

amnesiafs.ko: Kbuild *.c *.h
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean

fmt:
	clang-format -style=file -i *.c *.h mkfs/*.c

mkfs.amnesiafs: mkfs/*
	make -C mkfs
	cp mkfs/mkfs.amnesiafs .

test: amnesiafs.ko mkfs.amnesiafs
	./tests/run-qemu.sh
