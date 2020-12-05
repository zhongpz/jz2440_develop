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

static struct class *seventhdrv_class;  
static struct class_device	*seventhdrv_class_dev;


/* ���߶��г�ʼ�� */
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* �ж��¼���־, �жϷ����������1��third_drv_read������0 */
static volatile int ev_press = 0;

/* �첽֪ͨ�ṹ�� */
static struct fasync_struct *button_async;

/* ����ԭ�ӱ�������ʼ��Ϊ1 */
static atomic_t canopen = ATOMIC_INIT(1);

/* �����ź���(������) */
static DECLARE_MUTEX(button_lock);


/* ���������ṹ�� */
struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};

static unsigned char key_val;


/* ������������ */
struct pin_desc pins_desc[4] = {
	{S3C2410_GPF0, 0x01},
	{S3C2410_GPF2, 0x02},
	{S3C2410_GPG3, 0x03},
	{S3C2410_GPG11, 0x04},
};

/* �жϴ���������ȡ����ֵ */
static irqreturn_t button_irq(int irq, void *dev_id)
{
	struct pin_desc *pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;
	pinval = s3c2410_gpio_getpin(pindesc->pin);

	if(pinval)
	{	
		/* �ɿ� */
		key_val = 0x80 | pindesc->key_val;
	}
	else
	{
		/* ���� */
		key_val = pindesc->key_val;
	}

	ev_press = 1;                  /* ��ʾ�жϷ����� */
    wake_up_interruptible(&button_waitq);   /* �������ߵĽ��� */

	/*�����źŸ�Ӧ�ó���*/
	kill_fasync (&button_async, SIGIO, POLL_IN);
	
	/* ���յ���׼ȷ���ж��ź�,����������Ӧ��ȷ�Ĵ��� */
	return IRQ_RETVAL(IRQ_HANDLED);
}



static int seventh_drv_open(struct inode *inode, struct file *file)
{
	#if 0
	/*ԭ�ӱ����Լ��������������Ƿ�Ϊ0,0����true*/
	if(!atomic_dec_and_test(&canopen))
	{
		atomic_inc(&canopen); //ԭ�ӱ�������
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
		/*��ȡ�ź���*/
		down(&button_lock);
	}
	
	/* ע���ж� */
	request_irq(IRQ_EINT0, button_irq, IRQT_BOTHEDGE, "s2", &pins_desc[0]);
	request_irq(IRQ_EINT2, button_irq, IRQT_BOTHEDGE, "s3", &pins_desc[1]);
	request_irq(IRQ_EINT11, button_irq, IRQT_BOTHEDGE, "s4", &pins_desc[2]);
	request_irq(IRQ_EINT19, button_irq, IRQT_BOTHEDGE, "s5", &pins_desc[3]);
	
	return 0;
}

static int seventh_drv_read(struct file *file, char __user *buf, size_t size, loff_t *offp)
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
		/* ���û�а�������, ���� */
		wait_event_interruptible(button_waitq, ev_press);
	}

	/* ����а�������, ���ؼ�ֵ */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;
}

int seventh_drv_close(struct inode *inode, struct file *file)
{	
	#if 0
	atomic_inc(&canopen); //ԭ�ӱ�������
	#endif
	/*�ͷ��ź���*/
	up(&button_lock);
	
	/* ����ж� */
	free_irq(IRQ_EINT0, &pins_desc[0]);
	free_irq(IRQ_EINT2, &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
}

static unsigned seventh_drv_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait); // ������������

	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}


static int seventh_drv_fasync (int fd, struct file *filp, int on)
{
	printk("driver: seventh_drv_fasync\n");
	return fasync_helper (fd, filp, on, &button_async);
}


/*file_operation�ṹ��*/
static struct file_operations seventhdrv_fops = {
    .owner   =  THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open    =  seventh_drv_open,     
	.read	 =	seventh_drv_read,
	.release =  seventh_drv_close,
	.poll    =  seventh_drv_poll,
	.fasync  =  seventh_drv_fasync,
};

int major;
/*��ں�����ע��ʱ����*/
static int seventh_drv_init(void)
{	
	major = register_chrdev(0, "seventh_drv", &seventhdrv_fops);

	seventhdrv_class = (struct class *)class_create(THIS_MODULE, "seventh_drv");
	seventhdrv_class_dev = (struct class_device *)class_device_create(seventhdrv_class, NULL, MKDEV(major, 0), NULL, "button");

	return 0;
}

/*���ں�����ж�������ǵ���*/
static void seventh_drv_exit(void)
{
	unregister_chrdev(major, "seventh_drv");

	class_destroy(seventhdrv_class);
	class_device_unregister(seventhdrv_class_dev);	
}


/*���γ�����ں����������ں�*/
module_init(seventh_drv_init);
module_exit(seventh_drv_exit);

/*��֤*/
MODULE_LICENSE("GPL");



























