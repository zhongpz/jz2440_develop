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
#include <linux/clk.h>	
#include <linux/gpio_keys.h>
#include <asm/irq.h>
#include <asm/plat-s3c24xx/ts.h>
#include <asm/arch/regs-adc.h>
#include <asm/gpio.h>

#include <asm/io.h>
#include <asm/arch/regs-gpio.h>


static struct input_dev *s3c_ts_dev;   //����һ��input_dev����
static struct clk *adc_clk;            //����һ��ʱ��

/*������Ҫ�����ļĴ����ṹ��*/
struct s3c_ts_regs {
	unsigned long adccon;
	unsigned long adctsc;
	unsigned long adcdly;
	unsigned long adcdat0;
	unsigned long adcdat1;
	unsigned long adcupdn;
};

static volatile struct s3c_ts_regs *s3c_ts_regs;

/*�ȴ����밴���жϺ���*/
static void enter_wait_pen_down_mode(void)
{
	//���˼Ĵ����� adctsc �Ĵ�������Ϊ 0xd3 �ͽ��롰�ȴ������ж�ģʽ�� ��
	s3c_ts_regs->adctsc = 0xd3;
}

/*�ȴ������ɿ��жϺ���*/
static void enter_wait_pen_up_mode(void)
{
	//���˼Ĵ����� adctsc �Ĵ�������Ϊ 0x1d3 �ͽ��롰�ȴ��ɿ��ж�ģʽ�� ��
	s3c_ts_regs->adctsc = 0x1d3;
}

/*�жϴ�����*/
static irqreturn_t pen_down_up_irq(int irq, void *dev_id)
{
	if(s3c_ts_regs->adcdat0 & (1<<15))
	{
		/*1�ɿ�״̬*/
		printk("pen up\n");
		enter_wait_pen_down_mode();   //�ȴ�����
	}
	else
	{
		/*0����״̬*/	
		printk("pen down\n");
		enter_wait_pen_up_mode();   //�ȴ��ɿ�
	}

	return IRQ_HANDLED;
}
/*1.��ں���*/
static int s3c_ts_init(void)
{
	/*1.1����һ��input_dev�ṹ��*/
	s3c_ts_dev = input_allocate_device();
	
	/*1.2����input_dev�ṹ��*/
	/*1.2.1���ò��������¼�*/
	set_bit(EV_KEY, s3c_ts_dev->evbit);    //�������¼�
	set_bit(EV_ABS, s3c_ts_dev->evbit);   //����������λ���¼�
	
	/*1.2.2*���������¼��е������¼�*/
	set_bit(BTN_TOUCH, s3c_ts_dev->keybit);    //�������¼�

	/*1.2.3�������¼���������*/
	input_set_abs_params(s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);     //x�����������Ϊ��10λADC���������Ϊ0x3FF
	input_set_abs_params(s3c_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);     //y�������
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);  //��ѹ����

	/*1.3ע��input_dev�ṹ��*/
	input_register_device(s3c_ts_dev);
	
	
	/*1.4Ӳ����صĲ���*/
	/*1.4.1ʹ����ʱ�ӣ�����ϵͳADC����ģ�飬����CLKCON[15]Ϊ1*/
	adc_clk = clk_get(NULL, "adc");
	clk_enable(adc_clk);

	/*1.4.2ioremap�Ĵ���*/
	s3c_ts_regs = ioremap(0x58000000, sizeof(s3c_ts_regs));

	/*1.4.3���üĴ���*/

	/*  adccon����
	 *	PRSCEN [14]��Ԥ��Ƶʹ�ܡ�Ҫ����Ϊ��1��
	 *  PRSCVL [13:6]��Ԥ��Ƶϵ��������Ϊ49������ ADC ʱ�ӣ�
	 *                 ADCCLK=PCLK/(49+1)=50MHz/(49+1)=1MHz
	 *	STDBM [2]:����ģʽѡ������Ϊ0����������ģʽ
	 *  READ_ START [1]������Ϊ0,��ʹ�ö�����������ADת����
	 */
	 s3c_ts_regs->adccon |= (1<<14) | (49<<6) | (0<<2);

	
	/*ע�ᴥ�����ж�*/
	 enter_wait_pen_down_mode();   //����ȴ������ж�
	 request_irq(IRQ_TC, pen_down_up_irq, IRQF_SAMPLE_RANDOM, "ts_pen", NULL);
	 
	return 0;
}



/*2.���ں���*/
static void s3c_ts_exit(void)
{
	input_unregister_device(s3c_ts_dev);  /*ж��input_dev�ṹ*/
	input_free_device(s3c_ts_dev);  
	free_irq(IRQ_TC, NULL);             //�ͷ��ж�
	iounmap(s3c_ts_regs);      
}

/*3.����*/
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);

MODULE_LICENSE("GPL");
