obj-m := hyperv.o

hyperv-objs := \
	hypervfs.o \
	super.o \
	inode.o \
	dentry.o \
	file.o \
	dir.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
