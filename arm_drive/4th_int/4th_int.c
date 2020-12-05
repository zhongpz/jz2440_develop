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


/* ���߶��г�ʼ�� */
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* �ж��¼���־, �жϷ����������1��third_drv_read������0 */
static volatile int ev_press = 0;

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

	//��ȡ�ж����ŵĵ�ƽ
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

	/* ���յ���׼ȷ���ж��ź�,����������Ӧ��ȷ�Ĵ��� */
	return IRQ_RETVAL(IRQ_HANDLED);
}



static int fourth_drv_open(struct inode *inode, struct file *file)
{
	/* ע���ж� */
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

	/* ���û�а�������, ����Ӧ�ó��� */
	wait_event_interruptible(button_waitq, ev_press);

	/* ����а�������, ���ؼ�ֵ */
	copy_to_user(buf, &key_val, 1);
	ev_press = 0;
	
	return 1;
}

int fourth_drv_close(struct inode *inode, struct file *file)
{
	/* ����ж� */
	free_irq(IRQ_EINT0, &pins_desc[0]);
	free_irq(IRQ_EINT2, &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);
}

static struct file_operations fourthdrv_fops = {
    .owner   =  THIS_MODULE,    /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
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



























