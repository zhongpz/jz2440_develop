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


static int major;  //�������豸��

static struct class *led_class;  
static struct class_device	*led_class_dev;

static volatile unsigned long *gpio_con;
static volatile unsigned long *gpio_dat;
static int pin;

static int led_drv_open(struct inode *inode, struct file *file)
{
	/*�����������*/
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
		/* �� */
		*gpio_dat &= ~(0x1 << pin);
	}
	else
	{
		/* �� */
		*gpio_dat |=  (0x1 << pin);
	}
	
	return 0;
}

/*����file_operation�ṹ*/
static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open  = led_drv_open,     
	.write = led_drv_write,	
};

/*probe����*/
static int led_probe(struct platform_device *pdev)
{
	/*����ƽ̨�豸����Դ����ioremap*/
	struct resource *res;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gpio_con = ioremap(res->start, res->end - res->start + 1);
	gpio_dat = gpio_con + 1;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 1);   // 1��ʾ��һ��IORESOURCE_IRQ��Դ
	pin = res->start;
	
	/*ע���ַ��豸��������*/
	major = register_chrdev(0, "myled", &led_fops);
	led_class = (struct class *)class_create(THIS_MODULE, "led_drv");
	led_class_dev = (struct class_device *)class_device_create(led_class, NULL, MKDEV(major, 0), NULL, "led");

	printk("led_probe, found myled\n");
	return 0;
	
}

/*remove����*/
static int  led_remove(struct platform_device *pdev)
{
	/*����ƽ̨�豸��Դ����iounmap*/
	
	
	/*ж������*/
	unregister_chrdev(major, "myled");
	class_destroy(led_class);
	class_device_unregister(led_class_dev);

	
	printk("led_remove, remove myled\n");
	return 0;
}

/*1.����һ��ƽ̨����platform_driver�ṹ��*/
struct platform_driver led_drv = {
	.probe		= led_probe,
	.remove		= led_remove,
	.driver		= {
		.name	= "myled",  //����Ҫ��ƽ̨�豸�ṹ���е�����һ��
	}
};


/*2.��ں���*/
static int led_drv_init(void)
{
	/*2.1ע��platform_driver�ṹ��*/
	platform_driver_register(&led_drv);
	return 0;
}


/*3.���ں���*/
static void led_drv_exit(void)
{
	platform_driver_unregister(&led_drv);
}


/*4.����*/
module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");

