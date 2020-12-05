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

#include <asm/gpio.h>

#include <asm/io.h>
#include <asm/arch/regs-gpio.h>

/*���������ṹ��*/
struct pin_desc{
		int irq;                //�жϺ�
		char *name;             //����
		unsigned int pin;       //����
		unsigned int key_val;   //����ֵ
};

struct pin_desc pins_desc[4] = {
	{IRQ_EINT0,  "S2", S3C2410_GPF0,  KEY_L},
	{IRQ_EINT2,  "S3", S3C2410_GPF2,  KEY_S},
	{IRQ_EINT11, "S4", S3C2410_GPG3,  KEY_ENTER},
	{IRQ_EINT19, "S5", S3C2410_GPG11, KEY_LEFTSHIFT},
};

static struct pin_desc *irq_pd;          //
static struct timer_list buttons_timer;  //�趨��ʱ��
static struct input_dev *buttons_dev;    //����input_dev�ṹ

/*�жϴ�����*/
static irqreturn_t button_irq(int irq, void *dev_id)
{
	irq_pd = (struct pin_desc *)dev_id;
	/* �趨��ʱ��ʱʱ��Ϊ10ms */
	mod_timer(&buttons_timer, jiffies+HZ/100);
	return IRQ_RETVAL(IRQ_HANDLED);
}

/*��ʱ��������*/
static void buttons_timer_funtion(unsigned long data)
{
	struct pin_desc *pindesc = irq_pd;
	unsigned int pinval;
	if(!pindesc)
		return;

	pinval = s3c2410_gpio_getpin(pindesc->pin);

	if(pinval)
	{
		/*�ɿ�,�ϱ��¼�,input_dev�ṹ,�¼����ͣ������¼���0��ʾ�ɿ�	*/
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);	
		input_sync(buttons_dev); //�ϱ�ͬ���¼�
	}
	else
	{
		/*����*/
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);	
		input_sync(buttons_dev); //�ϱ�ͬ���¼�
	}
}
	

/* 1.��д��ں��� */
static int buttons_init(void)
{
	/* 1.1����һ��input_dev�ṹ�� */
	buttons_dev = input_allocate_device();		 //����
	
	/* 1.2����input_dev */
	/* 1.2.1 �ܲ��������¼� */
	set_bit(EV_KEY, buttons_dev->evbit);   //�����¼�
	set_bit(EV_REP, buttons_dev->evbit);   //�ظ����¼�

	/* 1.2.2 �ܲ����������¼��е������¼������ĸ�����������L,S,ENTER,LEFTSHIFT */
	set_bit(KEY_L, buttons_dev->keybit); 
	set_bit(KEY_S, buttons_dev->keybit); 
	set_bit(KEY_ENTER, buttons_dev->keybit); 
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit); 
	
	/* 1.3ע��input_dev */
	input_register_device(buttons_dev);

	/* 1.4Ӳ����صĲ��� */	
	/* 1.4.1 ע���ж� */
	int i;
	for(i = 0; i < 4; i++)
	{
		request_irq(pins_desc[i].irq, button_irq, IRQT_BOTHEDGE, pins_desc[i].name, &pins_desc[i]);
	}

	/* 1.4.2ʹ�ö�ʱ�� */

	/* 1.4.2.1��ʼ����ʱ�� */
	init_timer(&buttons_timer);

	/* 1.4.2.2��ʱ�������� */
	buttons_timer.function = buttons_timer_funtion;
	
	/* 1.4.2.3��Ӷ�ʱ��*/
	add_timer(&buttons_timer);

	
	return 0;
}

/* 2. ��д���ں��� */
static void buttons_exit(void)
{
	int i ;
	/*����ж�*/
	for(i = 0; i < 4; i++)
	{
		free_irq(pins_desc[i].irq, &pins_desc[i]);
	}

	/*�����ʱ��*/
	del_timer(&buttons_timer);

	/*ж��input_dev�ṹ*/
	input_unregister_device(buttons_dev);

	/*�ͷ�input_dev�ṹ�ռ�*/
	input_free_device(buttons_dev);	
}

/* 3.������ںͳ��ں��� */
module_init(buttons_init);
module_exit(buttons_exit);

MODULE_LICENSE("GPL");





