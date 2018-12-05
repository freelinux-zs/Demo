# Makefile 

# Comment/uncomment the following line to disable/enable debuging
# DEBUG = y


# Add your debuging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DSCULL_DEBUG # "-O" is needed to expand inlines
else
	debuging = -O2
endif

ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m += test_chain_0.o test_chain_1.o test_chain_2.o

else


KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD	      := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules


endif


clean:
	
	rm -rf *.o *~core .depend .*.cmd *.ko *.mod.c .tmp_versions

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include .depend
endif
