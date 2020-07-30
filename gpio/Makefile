
KERVER := $(shell uname -r)

KDIR := /lib/modules/$(KERVER)/build

obj-m := gpio_module.o

default:
	$(MAKE) -C $(KDIR) M=$$PWD modules

clean:
	$(make) -C $(KDIR) M=$$PWD clean
