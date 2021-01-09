#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/gpio_keys.h>

#include <linux/kernel.h>

#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>


static int major;  //定义主设备号

static struct class *led_class;  
static struct class_device	*led_class_dev;

static volatile unsigned long *gpio_con;
static volatile unsigned long *gpio_dat;
static int pin;

static int led_drv_open(struct inode *inode, struct file *file)
{
	/*配置输出引脚*/
	*gpio_con &= ~(0x3 << (2*pin));
	*gpio_con |= (0x1 << (2*pin));
	return 0;
}

static ssize_t led_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;
	copy_from_user(&val, buf, count);
	if(val == 1)
	{
		/* 开 */
		*gpio_dat &= ~(0x1 << pin);
	}
	else
	{
		/* 关 */
		*gpio_dat |=  (0x1 << pin);
	}
	
	return 0;
}

/*定义file_operation结构*/
static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open  = led_drv_open,     
	.write = led_drv_write,	
};

/*probe函数*/
static int led_probe(struct platform_device *pdev)
{
	/*根据平台设备的资源进行ioremap*/
	struct resource *res;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gpio_con = ioremap(res->start, res->end - res->start + 1);
	gpio_dat = gpio_con + 1;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 1);   // 1表示第一个IORESOURCE_IRQ资源
	pin = res->start;
	
	/*注册字符设备驱动程序*/
	major = register_chrdev(0, "myled", &led_fops);
	led_class = (struct class *)class_create(THIS_MODULE, "led_drv");
	led_class_dev = (struct class_device *)class_device_create(led_class, NULL, MKDEV(major, 0), NULL, "led");

	printk("led_probe, found myled\n");
	return 0;
	
}

/*remove函数*/
static int  led_remove(struct platform_device *pdev)
{
	/* 根据资源进行iounmap */
	iounmap(gpio_con);
	
	/* 卸载驱动 */
	unregister_chrdev(major, "myled");
	class_destroy(led_class);
	class_device_unregister(led_class_dev);

	
	printk("led_remove, remove myled\n");
	return 0;
}

/*1.定义一个平台驱动platform_driver结构体*/
struct platform_driver led_drv = {
	.probe		= led_probe,
	.remove		= led_remove,
	.driver		= {
		.name	= "myled",  //名字要与平台设备结构体中的名字一样
	}
};


/* 2.入口函数 */
static int led_drv_init(void)
{
	/*2.1注册平台结构体*/
	platform_driver_register(&led_drv);
	return 0;
}


/*3.出口函数*/
static void led_drv_exit(void)
{
	platform_driver_unregister(&led_drv);
}


/*4.修饰*/
module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");

