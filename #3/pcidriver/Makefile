KERNEL_DIR = /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m += hw3_pci_driver.o

default:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
