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

static struct class *firstdrv_class;  
static struct class_device	*firstdrv_class_dev;


/* 1.д��open,read��write�Ⱥ��� */
static int first_drv_open(struct inode *inode, struct file *file)
{
	printk("first_drv_open\n");
	return 0;
}

static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	printk("first_drv_write\n");
	return 0;
}

/* 2.����file_operations �ṹ�岢���*/
static struct file_operations firstdrv_fops = {
    .owner  =   THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open   =   first_drv_open,     
	.write	=	first_drv_write,	   
};


int major;  //register_chrdev��д0��ϵͳ�Զ������豸��

/* 3.����ں����е���register_chrdev����ע��file_operations�ṹ�� */
static int first_drv_init(void)
{
	major = register_chrdev(0, "first_drv", &firstdrv_fops);
	firstdrv_class = (struct class *)class_create(THIS_MODULE, "first_drv");
	firstdrv_class_dev = (struct class_device *)class_device_create(firstdrv_class, NULL, MKDEV(major, 0), NULL, "xyz"); /* /dev/xyz */
	return 0;
}

/* 4.���ں���ж�� */
static void first_drv_exit(void)
{
	unregister_chrdev(major, "first_drv"); // ж��
	class_destroy(firstdrv_class);
	class_device_unregister(firstdrv_class_dev);
}

/* 5.������ں����ͳ��ں���,���ں˵����� */
module_init(first_drv_init);
module_exit(first_drv_exit);


MODULE_LICENSE("GPL");






