obj-m += hid-bosto.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)
CHANGELOG := $(firstword $(wildcard ../CHANGELOG.md CHANGELOG.md))
VERSION := $(if $(CHANGELOG),$(shell sed -n 's/^## \(.*\)/\1/p' $(CHANGELOG) | head -1),unknown)
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