all: amnesiafs.ko

KDIR = /lib/modules/`uname -r`/build

amnesiafs.ko: Kbuild *.c *.h
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean

fmt:
	clang-format -style=file -i *.c

test: amnesiafs.ko
	./tests/run-qemu.sh
