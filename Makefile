KDIR = /lib/modules/`uname -r`/build

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean

fmt:
	clang-format -style=file -i *.c

test:
	./tests/run-qemu.sh
