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


/* ���߶��г�ʼ�� */
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* �ж��¼���־, �жϷ����������1��third_drv_read������0 */
static volatile int ev_press = 0;

static struct fasync_struct *button_async;

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



static int sixth_drv_open(struct inode *inode, struct file *file)
{
	/* ע���ж� */
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

	/* ���û�а�������, ���� */
	wait_event_interruptible(button_waitq, ev_press);

	/* ����а�������, ���ؼ�ֵ */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;
}

int sixth_drv_close(struct inode *inode, struct file *file)
{
	/* ����ж� */
	free_irq(IRQ_EINT0, &pins_desc[0]);
	free_irq(IRQ_EINT2, &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
}

static unsigned sixth_drv_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait); // ������������

	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

	return mask;
}


static int sixth_drv_fasync (int fd, struct file *filp, int on)
{
	printk("driver: sixth_drv_fasync\n");
	return fasync_helper (fd, filp, on, &button_async);
}


/*file_operation�ṹ��*/
static struct file_operations sixthdrv_fops = {
    .owner   =  THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
    .open    =  sixth_drv_open,     
	.read	 =	sixth_drv_read,
	.release =  sixth_drv_close,
	.poll    =  sixth_drv_poll,
	.fasync  =  sixth_drv_fasync,
};

int major;
/*��ں�����ע��ʱ����*/
static int sixth_drv_init(void)
{	
	major = register_chrdev(0, "sixth_drv", &sixthdrv_fops);

	sixthdrv_class = (struct class *)class_create(THIS_MODULE, "sixth_drv");
	sixthdrv_class_dev = (struct class_device *)class_device_create(sixthdrv_class, NULL, MKDEV(major, 0), NULL, "button");

	return 0;
}

/*���ں�����ж�������ǵ���*/
static void sixth_drv_exit(void)
{
	unregister_chrdev(major, "sixth_drv");

	class_destroy(sixthdrv_class);
	class_device_unregister(sixthdrv_class_dev);	
}


/*���γ�����ں����������ں�*/
module_init(sixth_drv_init);
module_exit(sixth_drv_exit);

/*��֤*/
MODULE_LICENSE("GPL");



























