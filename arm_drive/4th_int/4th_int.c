#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

static struct class *fourthdrv_class;  
static struct class_device *fourthdrv_class_dev;


/* 休眠队列初始化 */
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志, 中断服务程序将它置1，third_drv_read将它清0 */
static volatile int ev_press = 0;

/* 引脚描述结构体 */
struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};

static unsigned char key_val;


/* 引脚描述数组 */
struct pin_desc pins_desc[4] = {
	{S3C2410_GPF0, 0x01},
	{S3C2410_GPF2, 0x02},
	{S3C2410_GPG3, 0x03},
	{S3C2410_GPG11, 0x04},
};

/* 中断处理函数，获取按键值 */
static irqreturn_t button_irq(int irq, void *dev_id)
{
	struct pin_desc *pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;

	//获取中断引脚的电平
	pinval = s3c2410_gpio_getpin(pindesc->pin);

	if(pinval)
	{	
		/* 松开 */
		key_val = 0x80 | pindesc->key_val;
	}
	else
	{
		/* 按下 */
		key_val = pindesc->key_val;
	}

	ev_press = 1;                  /* 表示中断发生了 */
    wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */

	/* 接收到了准确的中断信号,并且作了相应正确的处理 */
	return IRQ_RETVAL(IRQ_HANDLED);
}



static int fourth_drv_open(struct inode *inode, struct file *file)
{
	/* 注册中断 */
	request_irq(IRQ_EINT0, button_irq, IRQT_BOTHEDGE, "s2", &pins_desc[0]);
	request_irq(IRQ_EINT2, button_irq, IRQT_BOTHEDGE, "s3", &pins_desc[1]);
	request_irq(IRQ_EINT11, button_irq, IRQT_BOTHEDGE, "s4", &pins_desc[2]);
	request_irq(IRQ_EINT19, button_irq, IRQT_BOTHEDGE, "s5", &pins_desc[3]);
	
	return 0;
}

static int fourth_drv_read(struct file *filp, char __user *buf, size_t size, loff_t *offp)
{
	if (size != 1)
		return -EINVAL;

	/* 如果没有按键动作, 休眠应用程序 */
	wait_event_interruptible(button_waitq, ev_press);

	/* 如果有按键动作, 返回键值 */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;
}

int fourth_drv_close(struct inode *inode, struct file *file)
{
	/* 清除中断 */
	free_irq(IRQ_EINT0, &pins_desc[0]);
	free_irq(IRQ_EINT2, &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
}

static struct file_operations fourthdrv_fops = {
    .owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =  fourth_drv_open,     
	.read	 =	fourth_drv_read,
	.release =  fourth_drv_close,
};

int major;
static int fourth_drv_init(void)
{	
	major = register_chrdev(0, "fourth_drv", &fourthdrv_fops);

	fourthdrv_class = (struct class *)class_create(THIS_MODULE, "fourth_drv");
	fourthdrv_class_dev = (struct class_device *)class_device_create(fourthdrv_class, NULL, MKDEV(major, 0), NULL, "int");

	return 0;
}

static void fourth_drv_exit(void)
{
	unregister_chrdev(major, "fourth_drv");

	class_destroy(fourthdrv_class);
	class_device_unregister(fourthdrv_class_dev);	

}



module_init(fourth_drv_init);
module_exit(fourth_drv_exit);


MODULE_LICENSE("GPL");



























