obj-m := gpio_int.o
PWD   := $(shell pwd)
K_DIR := /works/linux

all:
	$(MAKE) -C $(K_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(K_DIR) M=$(PWD) clean
