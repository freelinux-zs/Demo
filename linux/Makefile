KVERS = $(shell uname -r)

#kernel modules
#obj-m += hello.o
#obj-m += exported_symbols.o
obj-m += globalmem.o
#Specify flags for the module compilation
#EXTRA_CFLAGS=-g -O0

build: kernel_modules

kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
