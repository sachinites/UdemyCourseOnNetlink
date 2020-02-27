obj-m += RtmNetlinkLKM.o
RtmNetlinkLKM-objs += gluethread/glthread.o rt_kern.o
all:
	make -C /lib/modules/`uname -r`/build M=$(PWD) modules
clean:
	make -C /lib/modules/`uname -r`/build M=$(PWD) clean
	rm -f rt_kern.o 
	rm -f gluethread/glthread.o
