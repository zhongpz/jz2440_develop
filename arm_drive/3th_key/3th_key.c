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

static struct class *thirddrv_class;  
static struct class_device	*thirddrv_class_dev;

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;

volatile unsigned long *gpgcon = NULL;
volatile unsigned long *gpgdat = NULL;

static int third_drv_open(struct inode *inode, struct file *file)
{
	/*配置GPF0,2和GPG3,11为输出引脚*/
	*gpfcon &= ~((0x3<<(0*2))|(0x3<<(2*2)));
	*gpgcon &= ~((0x3<<(3*2))|(0x3<<(11*2)));
	
	return 0;
}

static int third_drv_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	char key_val[4];
	char regval = 0;

	if(count != sizeof(key_val))
	{
		return -EINVAL;
	}
	
	/*读GPF0,2*/
	regval = *gpfdat;
	key_val[0] = (regval & (1<<0)) ? 1 : 0;
	key_val[1] = (regval & (1<<2)) ? 1 : 0;
	
	/*读GPG3,11*/
	regval = *gpgdat;
	key_val[2] = (regval & (1<<3)) ? 1 : 0;
	key_val[3] = (regval & (1<<11)) ? 1 : 0;

	copy_to_user(buff, key_val, sizeof(key_val));
	
	return sizeof(key_val);
}


static struct file_operations thirddrv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   third_drv_open,     
	.read	=	third_drv_read,	   
};

int major;
static int third_drv_init(void)
{	
	major = register_chrdev(0, "third_drv", &thirddrv_fops);

	thirddrv_class = (struct class *)class_create(THIS_MODULE, "third_drv");
	thirddrv_class_dev = (struct class_device *)class_device_create(thirddrv_class, NULL, MKDEV(major, 0), NULL, "button");

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;
	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;

	return 0;
}

static void third_drv_exit(void)
{
	unregister_chrdev(major, "third_drv");

	class_destroy(thirddrv_class);
	class_device_unregister(thirddrv_class_dev);

	iounmap(gpfcon);
	iounmap(gpgcon);
}



module_init(third_drv_init);
module_exit(third_drv_exit);


MODULE_LICENSE("GPL");



























