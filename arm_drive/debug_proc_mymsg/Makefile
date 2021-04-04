KERN_DIR = /home/zpz/work/Linux_jz2440/kernel/linux-2.6.22.6

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	#$(CROSS_COMPILE)gcc -o ledtest ledtest.c 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	#rm -f ledtest

obj-m	+= mymsg.o
