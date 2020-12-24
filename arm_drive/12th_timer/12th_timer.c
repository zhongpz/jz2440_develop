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

static struct class * twelfthdrv_class;  
static struct class_device	* twelfthdrv_class_dev;


/* 休眠队列初始化 */
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志, 中断服务程序将它置1，third_drv_read将它清0 */
static volatile int ev_press = 0;

/* 异步通知结构体 */
static struct fasync_struct *button_async;

/* 定义原子变量并初始化为1 */
static atomic_t canopen = ATOMIC_INIT(1);

/* 定义互斥锁*/
static DECLARE_MUTEX(button_lock);

/*定义一个timer结构体*/
static struct timer_list buttons_timer;


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


static struct pin_desc *irq_pd = NULL;

/* 中断处理函数，获取按键值 */
static irqreturn_t button_irq(int irq, void *dev_id)
{
	irq_pd = (struct pin_desc *)dev_id;
	/*修改定时器的超时时间，10MS后启动定时器*/
	mod_timer(&buttons_timer, jiffies+HZ/100);  //jiffies是系统滴答时钟
	
	/* 接收到了准确的中断信号,并且作了相应正确的处理 */
	return IRQ_RETVAL(IRQ_HANDLED);
}


/*定时器处理函数*/
static void buttons_timer_function(unsigned long data)
{
	struct pin_desc *pindesc = irq_pd;
	unsigned int pinval;
	if(!pindesc)
		return ;
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

}

static int  twelfth_drv_open(struct inode *inode, struct file *file)
{
	#if 0
	/*原子变量自减操作，并测试是否为0,0返回true*/
	if(!atomic_dec_and_test(&canopen))
	{
		atomic_inc(&canopen); //原子变量自增
		return -EBUSY;
	}
	#endif
	
	if(file->f_flags & O_NONBLOCK)
	{
		if(down_trylock(&button_lock))
			return -EBUSY;
	}
	else
	{
		/*获取信号量*/
		down(&button_lock);
	}
	
	/* 注册中断 */
	request_irq(IRQ_EINT0, button_irq, IRQT_BOTHEDGE, "s2", &pins_desc[0]);
	request_irq(IRQ_EINT2, button_irq, IRQT_BOTHEDGE, "s3", &pins_desc[1]);
	request_irq(IRQ_EINT11, button_irq, IRQT_BOTHEDGE, "s4", &pins_desc[2]);
	request_irq(IRQ_EINT19, button_irq, IRQT_BOTHEDGE, "s5", &pins_desc[3]);
	
	return 0;
}

static int  twelfth_drv_read(struct file *file, char __user *buf, size_t size, loff_t *offp)
{
	if (size != 1)
		return -EINVAL;

	if(file->f_flags & O_NONBLOCK)
	{
		if(!ev_press)
			return -EAGAIN;
	}
	else
	{
		/* 如果没有按键动作, 休眠 */
		wait_event_interruptible(button_waitq, ev_press);
	}

	/* 如果有按键动作, 返回键值 */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;
}

int  twelfth_drv_close(struct inode *inode, struct file *file)
{	
	#if 0
	atomic_inc(&canopen); //原子变量自增
	#endif
	/*释放信号量*/
	up(&button_lock);
	
	/* 清除中断 */
	free_irq(IRQ_EINT0, &pins_desc[0]);
	free_irq(IRQ_EINT2, &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
}

static unsigned  twelfth_drv_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait); // 不会立即休眠

	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}


static int  twelfth_drv_fasync (int fd, struct file *filp, int on)
{
	printk("driver:  twelfth_drv_fasync\n");
	return fasync_helper (fd, filp, on, &button_async);
}


/*file_operation结构体*/
static struct file_operations  twelfthdrv_fops = {
    .owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =   twelfth_drv_open,     
	.read	 =	 twelfth_drv_read,
	.release =   twelfth_drv_close,
	.poll    =   twelfth_drv_poll,
	.fasync  =   twelfth_drv_fasync,
};

int major;
/*入口函数，注册时调用*/
static int  twelfth_drv_init(void)
{	
	major = register_chrdev(0, " twelfth_drv", & twelfthdrv_fops);

	twelfthdrv_class = (struct class *)class_create(THIS_MODULE, " twelfth_drv");
	twelfthdrv_class_dev = (struct class_device *)class_device_create( twelfthdrv_class, NULL, MKDEV(major, 0), NULL, "button");

	/*初始化定时器*/
	init_timer(&buttons_timer);
	
	/*设置*/
	//buttons_timer.data     =    //给定时器处理函数的参数
	//buttons_timer.expires  = jiffiers + 100*HZ;    //超时时间
	buttons_timer.function = buttons_timer_function; //定时器处理函数

	/*使用*/
	add_timer(&buttons_timer);  //把定时器告诉内核,这时候就会根据超时时间执行定时器处理函数
	return 0;
}

/*出口函数，卸载驱动是调用*/
static void  twelfth_drv_exit(void)
{
	unregister_chrdev(major, " twelfth_drv");

	class_destroy( twelfthdrv_class);
	class_device_unregister( twelfthdrv_class_dev);	
}


/*修饰出口入口函数，告诉内核*/
module_init( twelfth_drv_init);
module_exit( twelfth_drv_exit);

/*认证*/
MODULE_LICENSE("GPL");



























