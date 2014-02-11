# EMGD Backlight Module Makefile
# Date : May.06.2011
# Author : Adrian Cheater
# Version : v0.1beta1

obj-m := emgdbl.o

KDIR := /lib/modules/$(shell uname -r)
ifdef SYSSRC
 KERNEL_SOURCES := $(SYSSRC)
else
 ifdef SYSINCLUDE
 KERNEL_SOURCES := $(SYSINCLUDE)/..
 else
 KERNEL_SOURCES := $(shell test -d $(KDIR)/source && echo $(KDIR)/source || echo $(KDIR)/build)
 endif
endif
PWD  := $(shell pwd)

all: default

default:
	$(MAKE) -C $(KERNEL_SOURCES) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_SOURCES) SUBDIRS=$(PWD) clean
	@rm -f modules.order
	@rm -f Module.markers
	@rm -f *~

install:
	mkdir -p $(KDIR)/updates
	cp emgdbl.ko $(KDIR)/updates/
	depmod -a

uninstall:
	rm $(KDIR)/updates/dkms/emgdbl.ko
	rmmod emgdbl
	depmod -a
	modprobe emgdbl
