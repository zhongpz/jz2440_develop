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
#include <linux/poll.h>

static struct class *sixthdrv_class;  
static struct class_device	*sixthdrv_class_dev;


/* 休眠队列初始化 */
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志, 中断服务程序将它置1，third_drv_read将它清0 */
static volatile int ev_press = 0;

static struct fasync_struct *button_async;

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

	/*发送信号给应用程序*/
	kill_fasync (&button_async, SIGIO, POLL_IN);
	
	/* 接收到了准确的中断信号,并且作了相应正确的处理 */
	return IRQ_RETVAL(IRQ_HANDLED);
}



static int sixth_drv_open(struct inode *inode, struct file *file)
{
	/* 注册中断 */
	request_irq(IRQ_EINT0, button_irq, IRQT_BOTHEDGE, "s2", &pins_desc[0]);
	request_irq(IRQ_EINT2, button_irq, IRQT_BOTHEDGE, "s3", &pins_desc[1]);
	request_irq(IRQ_EINT11, button_irq, IRQT_BOTHEDGE, "s4", &pins_desc[2]);
	request_irq(IRQ_EINT19, button_irq, IRQT_BOTHEDGE, "s5", &pins_desc[3]);
	
	return 0;
}

static int sixth_drv_read(struct file *filp, char __user *buf, size_t size, loff_t *offp)
{
	if (size != 1)
		return -EINVAL;

	/* 如果没有按键动作, 休眠 */
	wait_event_interruptible(button_waitq, ev_press);

	/* 如果有按键动作, 返回键值 */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;
}

int sixth_drv_close(struct inode *inode, struct file *file)
{
	/* 清除中断 */
	free_irq(IRQ_EINT0, &pins_desc[0]);
	free_irq(IRQ_EINT2, &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
}

static unsigned sixth_drv_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait); // 不会立即休眠

	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}


static int sixth_drv_fasync (int fd, struct file *filp, int on)
{
	printk("driver: sixth_drv_fasync\n");
	return fasync_helper (fd, filp, on, &button_async);
}


/*file_operation结构体*/
static struct file_operations sixthdrv_fops = {
    .owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =  sixth_drv_open,     
	.read	 =	sixth_drv_read,
	.release =  sixth_drv_close,
	.poll    =  sixth_drv_poll,
	.fasync  =  sixth_drv_fasync,
};

int major;
/*入口函数，注册时调用*/
static int sixth_drv_init(void)
{	
	major = register_chrdev(0, "sixth_drv", &sixthdrv_fops);

	sixthdrv_class = (struct class *)class_create(THIS_MODULE, "sixth_drv");
	sixthdrv_class_dev = (struct class_device *)class_device_create(sixthdrv_class, NULL, MKDEV(major, 0), NULL, "button");

	return 0;
}

/*出口函数，卸载驱动是调用*/
static void sixth_drv_exit(void)
{
	unregister_chrdev(major, "sixth_drv");

	class_destroy(sixthdrv_class);
	class_device_unregister(sixthdrv_class_dev);	
}


/*修饰出口入口函数，告诉内核*/
module_init(sixth_drv_init);
module_exit(sixth_drv_exit);

/*认证*/
MODULE_LICENSE("GPL");



























