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

#define DBG_PRINTK printk      //开启打印
//#define DBG_PRINTK(x...)     //关闭打印

static struct class *seconddrv_exit;  
static struct class_device	*seconddrv_exit_dev;

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;

/*1.写出open,write,read等函数 */
static int second_drv_open(struct inode *inode, struct file *file)
{
	DBG_PRINTK("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	*gpfcon &= ~((0x3<<(2*4))|(0x3<<(2*5))|(0x3<<(2*6)));
	
	DBG_PRINTK("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	*gpfcon |= ((0x1<<(2*4))|(0x1<<(2*5))|(0x1<<(2*6)));
	return 0;
}


static ssize_t second_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;
	
	DBG_PRINTK("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
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

/*2.创建file_operations结构体并填充 */

static struct file_operations seconddrv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   second_drv_open,     
	.write	=	second_drv_write,	   
};

/*3.入口函数注册file_operations结构体，创建设备节点 */

int major;
static int second_drv_init(void)
{
	
	DBG_PRINTK("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	major = register_chrdev(0, "second_drv", &seconddrv_fops);
	seconddrv_exit = (struct class *)class_create(THIS_MODULE, "second_drv");
	seconddrv_exit_dev = (struct class_device *)class_device_create(seconddrv_exit, NULL, MKDEV(major, 0), NULL, "led");
	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;
	
	return 0;
}

/*4. 出口函数卸载*/
static void second_drv_exit(void)
{
	
	DBG_PRINTK("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	unregister_chrdev(major, "second_drv");
	class_destroy(seconddrv_exit);
	class_device_unregister(seconddrv_exit_dev);
	iounmap(gpfcon);
}


/*5. 修饰*/
module_init(second_drv_init);
module_exit(second_drv_exit);


MODULE_LICENSE("GPL");



































