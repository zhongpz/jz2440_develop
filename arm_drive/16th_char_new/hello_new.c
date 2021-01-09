
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/poll.h>
#include <linux/cdev.h>


#define HELLO_CNT 2    //次设备号范围
static struct cdev hello_cdev;
static struct class *hello_class;

static int hello_open (struct inode *inode, struct file *file)
{
	printk("hello_open\n");
	return 0;
}


/* 1.确定主设备号 */
static int major;

/* 2.构造file_operations结构体 */
static struct file_operations hello_fops = {
	.owner = THIS_MODULE,
	.open  = hello_open,
};


static int hello_init(void)
{
	dev_t devid;
	/* 3.告诉内核,分成3步 */
	/* 3.1规定次设备号的范围 */
#if 0
	major = register_chrdev(0, "hello", &hello_fops);
	/* (major,  0), (major, 1), ..., (major, 255)都对应hello_fops */
#else
	if(major)
	{
		/* 已知主设备号 */
		devid = MKDEV(major, 0); //将主设备号和次设备号转为dev_t类型
		register_chrdev_region(devid, HELLO_CNT, "hello");/* (major,0~1) 对应 hello_fops, (major, 2~255)都不对应hello_fops */
	}
	else
	{
		/* 不知道主设备号 */
		alloc_chrdev_region(&devid, 0, HELLO_CNT, "hello");/* (major,0~1) 对应 hello_fops, (major, 2~255)都不对应hello_fops */
		major = MAJOR(devid);  //获取主设备号
	}
	
	/* 3.2初始化cdev结构，把file_operations结构放到cdev结构中*/
	cdev_init(&hello_cdev, &hello_fops);
	/* 3.3把cdev添加进内核 */
	cdev_add(&hello_cdev, devid, HELLO_CNT);
#endif

	hello_class = class_create(THIS_MODULE, "hello"); //创建类
	class_device_create(hello_class, NULL, MKDEV(major, 0), NULL, "hello0");/* 创建设备结点/dev/hello0 */
	class_device_create(hello_class, NULL, MKDEV(major, 1), NULL, "hello1");/* 创建设备结点/dev/hello1 */

	/* 创建设备结点/dev/hello2,因为次设备号的范围是0-1，所以应用程序打开这个设备就会出错 */
	class_device_create(hello_class, NULL, MKDEV(major, 2), NULL, "hello2");

	return 0;
}

static void hello_exit(void)
{	
	class_device_destroy(hello_class, MKDEV(major, 0));
	class_device_destroy(hello_class, MKDEV(major, 1));
	class_device_destroy(hello_class, MKDEV(major, 2));
	class_destroy(hello_class);

	cdev_del(&hello_cdev);
	unregister_chrdev_region(MKDEV(major, 0), HELLO_CNT);

}
module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");













