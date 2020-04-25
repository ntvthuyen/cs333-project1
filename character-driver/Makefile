#cd /home/ubuntu/ldd/phan_1/bai_1_3
#vim Makefile

KDIR = /lib/modules/`uname -r`/build

all:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean