obj-m := dts_test.o platf_test.o
PWD   := $(shell pwd)
K_DIR := /works/linux

all:
	$(MAKE) -C $(K_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(K_DIR) M=$(PWD) clean
