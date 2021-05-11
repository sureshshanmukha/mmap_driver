obj-m +=char.o
all:
	gcc userspace_app.c -o userspace
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean 
