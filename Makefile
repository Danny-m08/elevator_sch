obj-y := export_sys_calls.o
obj-m := elevator.o

PWD := $(shell pwd)
KDIR := /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -f *.o *.ko  Module.* modules.* *~
