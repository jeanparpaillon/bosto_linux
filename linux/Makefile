obj-m += hid-bosto.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)
VERSION := $(shell sed -n 's/^## \(.*\)/\1/p' ../CHANGELOG.md | head -1)
ccflags-y += -DMODULE_VER=\"$(VERSION)\"

all:
	make -C $(KDIR) M=$(PWD) modules

install:
	make -C $(KDIR) M=$(PWD) modules_install
	depmod -a

uninstall:
	rm -f /lib/modules/$(shell uname -r)/kernel/drivers/hid/hid-bosto.ko
	depmod -a

clean:
	make -C $(KDIR) M=$(PWD) clean

.PHONY: all install uninstall clean