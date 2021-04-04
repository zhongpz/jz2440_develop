#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

static struct class *seconddrv_exit;  
static struct class_device	*seconddrv_exit_dev;

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;


static int second_drv_open(struct inode *inode, struct file *file)
{
	*gpfcon &= ~((0x3<<(2*4))|(0x3<<(2*5))|(0x3<<(2*6)));
	*gpfcon |= ((0x1<<(2*4))|(0x1<<(2*5))|(0x1<<(2*6)));
	return 0;
}


static ssize_t second_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;
	copy_from_user(&val, buf, count);
	if(val == 1)
	{
		*gpfdat &= ~((0x1<<4)|(0x1<<5)|(0x1<<6));
	}
	else
	{
		*gpfdat |=  ((0x1<<4)|(0x1<<5)|(0x1<<6));
	}
	
	return 0;
}



static struct file_operations seconddrv_fops = {
    .owner  =   THIS_MODULE,    
    .open   =   second_drv_open,     
	.write	=	second_drv_write,	   
};



int major;
static int second_drv_init(void)
{
	major = register_chrdev(0, "second_drv", &seconddrv_fops);
	seconddrv_exit = (struct class *)class_create(THIS_MODULE, "second_drv");
	seconddrv_exit_dev = (struct class_device *)class_device_create(seconddrv_exit, NULL, MKDEV(major, 0), NULL, "led");
	gpfcon = (volatile unsigned long *)0x56000050;//ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;
	
	return 0;
}


static void second_drv_exit(void)
{
	unregister_chrdev(major, "second_drv");
	class_destroy(seconddrv_exit);
	class_device_unregister(seconddrv_exit_dev);
	iounmap(gpfcon);
}


module_init(second_drv_init);
module_exit(second_drv_exit);


MODULE_LICENSE("GPL");

















