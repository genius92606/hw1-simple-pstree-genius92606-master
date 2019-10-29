MODULE_NAME := ksimple_pstree
obj-m := $(MODULE_NAME).o
KERN_VER := $(shell uname -r)
KERN_DIR ?= /usr/src/linux-headers-$(KERN_VER)/
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERN_DIR) M=$(PWD) modules
clean: 
	$(MAKE) -C $(KERN_DIR) M=$(PWD) clean
ins:
	sudo insmod $(MODULE_NAME).ko
rm:
	sudo rmmod $(MODULE_NAME)
